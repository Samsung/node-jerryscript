/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecma-globals.h"
#include "re-bytecode.h"
#include "ecma-regexp-object.h"

#if ENABLED (JERRY_BUILTIN_REGEXP)

/** \addtogroup parser Parser
 * @{
 *
 * \addtogroup regexparser Regular expression
 * @{
 *
 * \addtogroup regexparser_bytecode Bytecode
 * @{
 */

/**
 * Size of block of RegExp bytecode. Used for allocation
 *
 * @return pointer to the RegExp compiled code header
 */
#define REGEXP_BYTECODE_BLOCK_SIZE 8UL

void
re_initialize_regexp_bytecode (re_bytecode_ctx_t *bc_ctx_p) /**< RegExp bytecode context */
{
  const size_t initial_size = JERRY_ALIGNUP (REGEXP_BYTECODE_BLOCK_SIZE + sizeof (re_compiled_code_t), JMEM_ALIGNMENT);
  bc_ctx_p->block_start_p = jmem_heap_alloc_block (initial_size);
  bc_ctx_p->block_end_p = bc_ctx_p->block_start_p + initial_size;
  bc_ctx_p->current_p = bc_ctx_p->block_start_p + sizeof (re_compiled_code_t);
} /* re_initialize_regexp_bytecode */

/**
 * Realloc the bytecode container
 *
 * @return current position in RegExp bytecode
 */
static uint8_t *
re_realloc_regexp_bytecode_block (re_bytecode_ctx_t *bc_ctx_p) /**< RegExp bytecode context */
{
  JERRY_ASSERT (bc_ctx_p->block_end_p >= bc_ctx_p->block_start_p);
  const size_t old_size = (size_t) (bc_ctx_p->block_end_p - bc_ctx_p->block_start_p);

  /* If one of the members of RegExp bytecode context is NULL, then all member should be NULL
   * (it means first allocation), otherwise all of the members should be a non NULL pointer. */
  JERRY_ASSERT ((!bc_ctx_p->current_p && !bc_ctx_p->block_end_p && !bc_ctx_p->block_start_p)
                || (bc_ctx_p->current_p && bc_ctx_p->block_end_p && bc_ctx_p->block_start_p));

  const size_t new_size = old_size + REGEXP_BYTECODE_BLOCK_SIZE;
  JERRY_ASSERT (bc_ctx_p->current_p >= bc_ctx_p->block_start_p);
  const size_t current_ptr_offset = (size_t) (bc_ctx_p->current_p - bc_ctx_p->block_start_p);

  bc_ctx_p->block_start_p = jmem_heap_realloc_block (bc_ctx_p->block_start_p,
                                                     old_size,
                                                     new_size);
  bc_ctx_p->block_end_p = bc_ctx_p->block_start_p + new_size;
  bc_ctx_p->current_p = bc_ctx_p->block_start_p + current_ptr_offset;

  return bc_ctx_p->current_p;
} /* re_realloc_regexp_bytecode_block */

/**
 * Append a new bytecode to the and of the bytecode container
 */
static uint8_t *
re_bytecode_reserve (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                     const size_t size) /**< size */
{
  JERRY_ASSERT (size <= REGEXP_BYTECODE_BLOCK_SIZE);

  uint8_t *current_p = bc_ctx_p->current_p;
  if (current_p + size > bc_ctx_p->block_end_p)
  {
    current_p = re_realloc_regexp_bytecode_block (bc_ctx_p);
  }

  bc_ctx_p->current_p += size;
  return current_p;
} /* re_bytecode_reserve */

/**
 * Insert a new bytecode to the bytecode container
 */
static void
re_bytecode_insert (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                    const size_t offset, /**< distance from the start of the container */
                    const size_t size) /**< size */
{
  JERRY_ASSERT (size <= REGEXP_BYTECODE_BLOCK_SIZE);

  uint8_t *current_p = bc_ctx_p->current_p;
  if (current_p + size > bc_ctx_p->block_end_p)
  {
    re_realloc_regexp_bytecode_block (bc_ctx_p);
  }

  uint8_t *dest_p = bc_ctx_p->block_start_p + offset;
  const size_t bytecode_length = re_get_bytecode_length (bc_ctx_p);
  if (bytecode_length - offset > 0)
  {
    memmove (dest_p + size, dest_p, bytecode_length - offset);
  }

  bc_ctx_p->current_p += size;
} /* re_bytecode_insert */

/**
 * Encode ecma_char_t into bytecode
 */
static void
re_encode_char (uint8_t *dest_p, /**< destination */
                const ecma_char_t c) /**< character */
{
  *dest_p++ = (uint8_t) ((c >> 8) & 0xFF);
  *dest_p = (uint8_t) (c & 0xFF);
} /* re_encode_char */

/**
 * Encode uint32_t into bytecode
 */
static void
re_encode_u32 (uint8_t *dest_p, /**< destination */
               const uint32_t u) /**< uint32 value */
{
  *dest_p++ = (uint8_t) ((u >> 24) & 0xFF);
  *dest_p++ = (uint8_t) ((u >> 16) & 0xFF);
  *dest_p++ = (uint8_t) ((u >> 8) & 0xFF);
  *dest_p = (uint8_t) (u & 0xFF);
} /* re_encode_u32 */

/**
 * Get a character from the RegExp bytecode and increase the bytecode position
 *
 * @return ecma character
 */
inline ecma_char_t JERRY_ATTR_ALWAYS_INLINE
re_get_char (const uint8_t **bc_p) /**< pointer to bytecode start */
{
  const uint8_t *src_p = *bc_p;
  ecma_char_t chr = (ecma_char_t) *src_p++;
  chr = (ecma_char_t) (chr << 8);
  chr = (ecma_char_t) (chr | *src_p);
  (*bc_p) += sizeof (ecma_char_t);
  return chr;
} /* re_get_char */

/**
 * Get a RegExp opcode and increase the bytecode position
 *
 * @return current RegExp opcode
 */
inline re_opcode_t JERRY_ATTR_ALWAYS_INLINE
re_get_opcode (const uint8_t **bc_p) /**< pointer to bytecode start */
{
  return (re_opcode_t) *((*bc_p)++);
} /* re_get_opcode */

/**
 * Get a parameter of a RegExp opcode and increase the bytecode position
 *
 * @return opcode parameter
 */
inline uint32_t JERRY_ATTR_ALWAYS_INLINE
re_get_value (const uint8_t **bc_p) /**< pointer to bytecode start */
{
  const uint8_t *src_p = *bc_p;
  uint32_t value = (uint32_t) (*src_p++);
  value <<= 8;
  value |= ((uint32_t) (*src_p++));
  value <<= 8;
  value |= ((uint32_t) (*src_p++));
  value <<= 8;
  value |= ((uint32_t) (*src_p++));

  (*bc_p) += sizeof (uint32_t);
  return value;
} /* re_get_value */

/**
 * Get length of bytecode
 *
 * @return bytecode length (unsigned integer)
 */
inline uint32_t JERRY_ATTR_PURE JERRY_ATTR_ALWAYS_INLINE
re_get_bytecode_length (re_bytecode_ctx_t *bc_ctx_p) /**< RegExp bytecode context */
{
  return ((uint32_t) (bc_ctx_p->current_p - bc_ctx_p->block_start_p));
} /* re_get_bytecode_length */

/**
 * Append a RegExp opcode
 */
void
re_append_opcode (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                  const re_opcode_t opcode) /**< input opcode */
{
  uint8_t *dest_p = re_bytecode_reserve (bc_ctx_p, sizeof (uint8_t));
  *dest_p = (uint8_t) opcode;
} /* re_append_opcode */

/**
 * Append a parameter of a RegExp opcode
 */
void
re_append_u32 (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
               const uint32_t value) /**< input value */
{
  uint8_t *dest_p = re_bytecode_reserve (bc_ctx_p, sizeof (uint32_t));
  re_encode_u32 (dest_p, value);
} /* re_append_u32 */

/**
 * Append a character to the RegExp bytecode
 */
void
re_append_char (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                const ecma_char_t input_char) /**< input char */
{
  uint8_t *dest_p = re_bytecode_reserve (bc_ctx_p, sizeof (ecma_char_t));
  re_encode_char (dest_p, input_char);
} /* re_append_char */

/**
 * Append a jump offset parameter of a RegExp opcode
 */
void
re_append_jump_offset (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                       uint32_t value) /**< input value */
{
  value += (uint32_t) (sizeof (uint32_t));
  re_append_u32 (bc_ctx_p, value);
} /* re_append_jump_offset */

/**
 * Insert a RegExp opcode
 */
void
re_insert_opcode (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
                  const uint32_t offset, /**< distance from the start of the container */
                  const re_opcode_t opcode) /**< input opcode */
{
  re_bytecode_insert (bc_ctx_p, offset, sizeof (uint8_t));
  *(bc_ctx_p->block_start_p + offset) = (uint8_t) opcode;
} /* re_insert_opcode */

/**
 * Insert a parameter of a RegExp opcode
 */
void
re_insert_u32 (re_bytecode_ctx_t *bc_ctx_p, /**< RegExp bytecode context */
               uint32_t offset, /**< distance from the start of the container */
               uint32_t value) /**< input value */
{
  re_bytecode_insert (bc_ctx_p, offset, sizeof (uint32_t));
  re_encode_u32 (bc_ctx_p->block_start_p + offset, value);
} /* re_insert_u32 */

#if ENABLED (JERRY_REGEXP_DUMP_BYTE_CODE)
/**
 * RegExp bytecode dumper
 */
void
re_dump_bytecode (re_bytecode_ctx_t *bc_ctx_p) /**< RegExp bytecode context */
{
  re_compiled_code_t *compiled_code_p = (re_compiled_code_t *) bc_ctx_p->block_start_p;
  JERRY_DEBUG_MSG ("%d ", compiled_code_p->header.status_flags);
  JERRY_DEBUG_MSG ("%d ", compiled_code_p->captures_count);
  JERRY_DEBUG_MSG ("%d | ", compiled_code_p->non_captures_count);

  const uint8_t *bytecode_p = (const uint8_t *) (compiled_code_p + 1);

  re_opcode_t op;
  while ((op = re_get_opcode (&bytecode_p)))
  {
    switch (op)
    {
      case RE_OP_MATCH:
      {
        JERRY_DEBUG_MSG ("MATCH, ");
        break;
      }
      case RE_OP_CHAR:
      {
        JERRY_DEBUG_MSG ("CHAR ");
        JERRY_DEBUG_MSG ("%c, ", (char) re_get_char (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_NON_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DEBUG_MSG ("N");
        /* FALLTHRU */
      }
      case RE_OP_CAPTURE_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DEBUG_MSG ("GZ_START ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_GROUP_START:
      {
        JERRY_DEBUG_MSG ("START ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_CAPTURE_NON_GREEDY_GROUP_END:
      {
        JERRY_DEBUG_MSG ("N");
        /* FALLTHRU */
      }
      case RE_OP_CAPTURE_GREEDY_GROUP_END:
      {
        JERRY_DEBUG_MSG ("G_END ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_NON_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DEBUG_MSG ("N");
        /* FALLTHRU */
      }
      case RE_OP_NON_CAPTURE_GREEDY_ZERO_GROUP_START:
      {
        JERRY_DEBUG_MSG ("GZ_NC_START ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_GROUP_START:
      {
        JERRY_DEBUG_MSG ("NC_START ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_CAPTURE_NON_GREEDY_GROUP_END:
      {
        JERRY_DEBUG_MSG ("N");
        /* FALLTHRU */
      }
      case RE_OP_NON_CAPTURE_GREEDY_GROUP_END:
      {
        JERRY_DEBUG_MSG ("G_NC_END ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_SAVE_AT_START:
      {
        JERRY_DEBUG_MSG ("RE_START ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_SAVE_AND_MATCH:
      {
        JERRY_DEBUG_MSG ("RE_END, ");
        break;
      }
      case RE_OP_GREEDY_ITERATOR:
      {
        JERRY_DEBUG_MSG ("GREEDY_ITERATOR ");
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_NON_GREEDY_ITERATOR:
      {
        JERRY_DEBUG_MSG ("NON_GREEDY_ITERATOR ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_PERIOD:
      {
        JERRY_DEBUG_MSG ("PERIOD ");
        break;
      }
      case RE_OP_ALTERNATIVE:
      {
        JERRY_DEBUG_MSG ("ALTERNATIVE ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_ASSERT_START:
      {
        JERRY_DEBUG_MSG ("ASSERT_START ");
        break;
      }
      case RE_OP_ASSERT_END:
      {
        JERRY_DEBUG_MSG ("ASSERT_END ");
        break;
      }
      case RE_OP_ASSERT_WORD_BOUNDARY:
      {
        JERRY_DEBUG_MSG ("ASSERT_WORD_BOUNDARY ");
        break;
      }
      case RE_OP_ASSERT_NOT_WORD_BOUNDARY:
      {
        JERRY_DEBUG_MSG ("ASSERT_NOT_WORD_BOUNDARY ");
        break;
      }
      case RE_OP_LOOKAHEAD_POS:
      {
        JERRY_DEBUG_MSG ("LOOKAHEAD_POS ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_LOOKAHEAD_NEG:
      {
        JERRY_DEBUG_MSG ("LOOKAHEAD_NEG ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_BACKREFERENCE:
      {
        JERRY_DEBUG_MSG ("BACKREFERENCE ");
        JERRY_DEBUG_MSG ("%d, ", re_get_value (&bytecode_p));
        break;
      }
      case RE_OP_INV_CHAR_CLASS:
      {
        JERRY_DEBUG_MSG ("INV_");
        /* FALLTHRU */
      }
      case RE_OP_CHAR_CLASS:
      {
        JERRY_DEBUG_MSG ("CHAR_CLASS ");
        uint32_t num_of_class = re_get_value (&bytecode_p);
        JERRY_DEBUG_MSG ("%d", num_of_class);
        while (num_of_class)
        {
          if ((compiled_code_p->header.status_flags & RE_FLAG_UNICODE) != 0)
          {
            JERRY_DEBUG_MSG (" %u", re_get_value (&bytecode_p));
            JERRY_DEBUG_MSG ("-%u", re_get_value (&bytecode_p));
          }
          else
          {
            JERRY_DEBUG_MSG (" %u", re_get_char (&bytecode_p));
            JERRY_DEBUG_MSG ("-%u", re_get_char (&bytecode_p));
          }
          num_of_class--;
        }
        JERRY_DEBUG_MSG (", ");
        break;
      }
      default:
      {
        JERRY_DEBUG_MSG ("UNKNOWN(%d), ", (uint32_t) op);
        break;
      }
    }
  }
  JERRY_DEBUG_MSG ("EOF\n");
} /* re_dump_bytecode */
#endif /* ENABLED (JERRY_REGEXP_DUMP_BYTE_CODE) */

/**
 * @}
 * @}
 * @}
 */

#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
