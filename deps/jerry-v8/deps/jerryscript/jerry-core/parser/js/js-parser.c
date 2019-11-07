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

#include "debugger.h"
#include "ecma-exceptions.h"
#include "ecma-helpers.h"
#include "ecma-literal-storage.h"
#include "ecma-module.h"
#include "jcontext.h"
#include "js-parser-internal.h"

#if ENABLED (JERRY_PARSER)

JERRY_STATIC_ASSERT ((int) ECMA_PARSE_STRICT_MODE == (int) PARSER_IS_STRICT,
                     ecma_parse_strict_mode_must_be_equal_to_parser_is_strict);

#if ENABLED (JERRY_ES2015)
JERRY_STATIC_ASSERT ((ECMA_PARSE_CLASS_CONSTRUCTOR << PARSER_CLASS_PARSE_OPTS_OFFSET) == PARSER_CLASS_CONSTRUCTOR,
                     ecma_class_parse_options_must_be_able_to_be_shifted_to_ecma_general_flags);
#endif /* ENABLED (JERRY_ES2015) */

/** \addtogroup parser Parser
 * @{
 *
 * \addtogroup jsparser JavaScript
 * @{
 *
 * \addtogroup jsparser_parser Parser
 * @{
 */

/**
 * Compute real literal indicies.
 *
 * @return length of the prefix opcodes
 */
static void
parser_compute_indicies (parser_context_t *context_p, /**< context */
                         uint16_t *ident_end, /**< end of the identifier group */
                         uint16_t *const_literal_end) /**< end of the const literal group */
{
  parser_list_iterator_t literal_iterator;
  lexer_literal_t *literal_p;
  uint32_t status_flags = context_p->status_flags;

  uint16_t ident_count = 0;
  uint16_t const_literal_count = 0;

  uint16_t ident_index;
  uint16_t const_literal_index;
  uint16_t literal_index;

  if (status_flags & PARSER_ARGUMENTS_NOT_NEEDED)
  {
    status_flags &= (uint32_t) ~PARSER_ARGUMENTS_NEEDED;
    context_p->status_flags = status_flags;
  }

  /* First phase: count the number of items in each group. */
  parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);
  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)))
  {
    switch (literal_p->type)
    {
      case LEXER_IDENT_LITERAL:
      {
        if (literal_p->status_flags & LEXER_FLAG_USED)
        {
          ident_count++;
          break;
        }
#if !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
        else if (!(literal_p->status_flags & LEXER_FLAG_SOURCE_PTR))
        {
          jmem_heap_free_block ((void *) literal_p->u.char_p, literal_p->prop.length);
          /* This literal should not be freed even if an error is encountered later. */
          literal_p->status_flags |= LEXER_FLAG_SOURCE_PTR;
        }
#endif /* !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
        continue;
      }
      case LEXER_STRING_LITERAL:
      {
        const_literal_count++;
        break;
      }
      case LEXER_NUMBER_LITERAL:
      {
        const_literal_count++;
        continue;
      }
      case LEXER_FUNCTION_LITERAL:
      case LEXER_REGEXP_LITERAL:
      {
        continue;
      }
      default:
      {
        JERRY_ASSERT (literal_p->type == LEXER_UNUSED_LITERAL);
        continue;
      }
    }

#if !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
    const uint8_t *char_p = literal_p->u.char_p;

    if ((literal_p->status_flags & LEXER_FLAG_SOURCE_PTR)
        && literal_p->prop.length < 0xfff)
    {
      size_t bytes_to_end = (size_t) (context_p->source_end_p - char_p);

      if (bytes_to_end < 0xfffff)
      {
        literal_p->u.source_data = ((uint32_t) bytes_to_end) | (((uint32_t) literal_p->prop.length) << 20);
        literal_p->status_flags |= LEXER_FLAG_LATE_INIT;
        status_flags |= PARSER_HAS_LATE_LIT_INIT;
        context_p->status_flags = status_flags;
        char_p = NULL;
      }
    }

    if (char_p != NULL)
    {
      literal_p->u.value = ecma_find_or_create_literal_string (char_p,
                                                               literal_p->prop.length);

      if (!(literal_p->status_flags & LEXER_FLAG_SOURCE_PTR))
      {
        jmem_heap_free_block ((void *) char_p, literal_p->prop.length);
        /* This literal should not be freed even if an error is encountered later. */
        literal_p->status_flags |= LEXER_FLAG_SOURCE_PTR;
      }
    }
#endif /* !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
  }

  ident_index = context_p->register_count;
  const_literal_index = (uint16_t) (ident_index + ident_count);
  literal_index = (uint16_t) (const_literal_index + const_literal_count);

  /* Second phase: Assign an index to each literal. */
  parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);

  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)))
  {
    switch (literal_p->type)
    {
      case LEXER_IDENT_LITERAL:
      {
        if (literal_p->status_flags & LEXER_FLAG_USED)
        {
          literal_p->prop.index = ident_index;
          ident_index++;
        }
        break;
      }
      case LEXER_STRING_LITERAL:
      case LEXER_NUMBER_LITERAL:
      {
        JERRY_ASSERT ((literal_p->status_flags & ~(LEXER_FLAG_SOURCE_PTR | LEXER_FLAG_LATE_INIT)) == 0);
        literal_p->prop.index = const_literal_index;
        const_literal_index++;
        break;
      }
      case LEXER_FUNCTION_LITERAL:
      case LEXER_REGEXP_LITERAL:
      {
        JERRY_ASSERT (literal_p->status_flags == 0);

        literal_p->prop.index = literal_index;
        literal_index++;
        break;
      }
      default:
      {
        JERRY_ASSERT (literal_p->type == LEXER_UNUSED_LITERAL
                      && literal_p->status_flags == LEXER_FLAG_FUNCTION_ARGUMENT);
        break;
      }
    }
  }

  JERRY_ASSERT (ident_index == context_p->register_count + ident_count);
  JERRY_ASSERT (const_literal_index == ident_index + const_literal_count);
  JERRY_ASSERT (literal_index <= context_p->register_count + context_p->literal_count);

  context_p->literal_count = literal_index;

  *ident_end = ident_index;
  *const_literal_end = const_literal_index;
} /* parser_compute_indicies */

/**
 * Initialize literal pool.
 */
static void
parser_init_literal_pool (parser_context_t *context_p, /**< context */
                          ecma_value_t *literal_pool_p) /**< start of literal pool */
{
  parser_list_iterator_t literal_iterator;
  lexer_literal_t *literal_p;

  parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);

  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)))
  {
    switch (literal_p->type)
    {
      case LEXER_IDENT_LITERAL:
      {
        if (!(literal_p->status_flags & LEXER_FLAG_USED))
        {
          break;
        }
        /* FALLTHRU */
      }
      case LEXER_STRING_LITERAL:
      {
        ecma_value_t lit_value;
#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
        lit_value = ecma_find_or_create_literal_string (literal_p->u.char_p,
                                                        literal_p->prop.length);
#else /* !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
        lit_value = literal_p->u.value;
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

        JERRY_ASSERT (literal_p->prop.index >= context_p->register_count);
        literal_pool_p[literal_p->prop.index] = lit_value;

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
        if (!context_p->is_show_opcodes
            && !(literal_p->status_flags & LEXER_FLAG_SOURCE_PTR))
        {
          jmem_heap_free_block ((void *) literal_p->u.char_p, literal_p->prop.length);
        }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
        break;
      }
      case LEXER_NUMBER_LITERAL:
      {
        JERRY_ASSERT (literal_p->prop.index >= context_p->register_count);

        literal_pool_p[literal_p->prop.index] = literal_p->u.value;
        break;
      }
      case LEXER_FUNCTION_LITERAL:
      case LEXER_REGEXP_LITERAL:
      {
        JERRY_ASSERT (literal_p->prop.index >= context_p->register_count);

        ECMA_SET_INTERNAL_VALUE_POINTER (literal_pool_p[literal_p->prop.index],
                                         literal_p->u.bytecode_p);
        break;
      }
      default:
      {
        JERRY_ASSERT (literal_p->type == LEXER_UNUSED_LITERAL);
        break;
      }
    }
  }
} /* parser_init_literal_pool */

/*
 * During byte code post processing certain bytes are not
 * copied into the final byte code buffer. For example, if
 * one byte is enough for encoding a literal index, the
 * second byte is not copied. However, when a byte is skipped,
 * the offsets of those branches which crosses (jumps over)
 * that byte code should also be decreased by one. Instead
 * of finding these jumps every time when a byte is skipped,
 * all branch offset updates are computed in one step.
 *
 * Branch offset mapping example:
 *
 * Let's assume that each parser_mem_page of the byte_code
 * buffer is 8 bytes long and only 4 bytes are kept for a
 * given page:
 *
 * +---+---+---+---+---+---+---+---+
 * | X | 1 | 2 | 3 | X | 4 | X | X |
 * +---+---+---+---+---+---+---+---+
 *
 * X marks those bytes which are removed. The resulting
 * offset mapping is the following:
 *
 * +---+---+---+---+---+---+---+---+
 * | 0 | 1 | 2 | 3 | 3 | 4 | 4 | 4 |
 * +---+---+---+---+---+---+---+---+
 *
 * Each X is simply replaced by the index of the previous
 * index starting from zero. This shows the number of
 * copied bytes before a given byte including the byte
 * itself. The last byte always shows the number of bytes
 * copied from this page.
 *
 * This mapping allows recomputing all branch targets,
 * since mapping[to] - mapping[from] is the new argument
 * for forward branches. As for backward branches, the
 * equation is reversed to mapping[from] - mapping[to].
 *
 * The mapping is relative to one page, so distance
 * computation affecting multiple pages requires a loop.
 * We should also note that only argument bytes can
 * be skipped, so removed bytes cannot be targeted by
 * branches. Valid branches always target instruction
 * starts only.
 */

/**
 * Recompute the argument of a forward branch.
 *
 * @return the new distance
 */
static size_t
parser_update_forward_branch (parser_mem_page_t *page_p, /**< current page */
                              size_t full_distance, /**< full distance */
                              uint8_t bytes_copied_before_jump) /**< bytes copied before jump */
{
  size_t new_distance = 0;

  while (full_distance > PARSER_CBC_STREAM_PAGE_SIZE)
  {
    new_distance += page_p->bytes[PARSER_CBC_STREAM_PAGE_SIZE - 1] & CBC_LOWER_SEVEN_BIT_MASK;
    full_distance -= PARSER_CBC_STREAM_PAGE_SIZE;
    page_p = page_p->next_p;
  }

  new_distance += page_p->bytes[full_distance - 1] & CBC_LOWER_SEVEN_BIT_MASK;
  return new_distance - bytes_copied_before_jump;
} /* parser_update_forward_branch */

/**
 * Recompute the argument of a backward branch.
 *
 * @return the new distance
 */
static size_t
parser_update_backward_branch (parser_mem_page_t *page_p, /**< current page */
                               size_t full_distance, /**< full distance */
                               uint8_t bytes_copied_before_jump) /**< bytes copied before jump */
{
  size_t new_distance = bytes_copied_before_jump;

  while (full_distance >= PARSER_CBC_STREAM_PAGE_SIZE)
  {
    JERRY_ASSERT (page_p != NULL);
    new_distance += page_p->bytes[PARSER_CBC_STREAM_PAGE_SIZE - 1] & CBC_LOWER_SEVEN_BIT_MASK;
    full_distance -= PARSER_CBC_STREAM_PAGE_SIZE;
    page_p = page_p->next_p;
  }

  if (full_distance > 0)
  {
    size_t offset = PARSER_CBC_STREAM_PAGE_SIZE - full_distance;

    JERRY_ASSERT (page_p != NULL);

    new_distance += page_p->bytes[PARSER_CBC_STREAM_PAGE_SIZE - 1] & CBC_LOWER_SEVEN_BIT_MASK;
    new_distance -= page_p->bytes[offset - 1] & CBC_LOWER_SEVEN_BIT_MASK;
  }

  return new_distance;
} /* parser_update_backward_branch */

/**
 * Update targets of all branches in one step.
 */
static void
parse_update_branches (parser_context_t *context_p, /**< context */
                       uint8_t *byte_code_p) /**< byte code */
{
  parser_mem_page_t *page_p = context_p->byte_code.first_p;
  parser_mem_page_t *prev_page_p = NULL;
  parser_mem_page_t *last_page_p = context_p->byte_code.last_p;
  size_t last_position = context_p->byte_code.last_position;
  size_t offset = 0;
  size_t bytes_copied = 0;

  if (last_position >= PARSER_CBC_STREAM_PAGE_SIZE)
  {
    last_page_p = NULL;
    last_position = 0;
  }

  while (page_p != last_page_p || offset < last_position)
  {
    /* Branch instructions are marked to improve search speed. */
    if (page_p->bytes[offset] & CBC_HIGHEST_BIT_MASK)
    {
      uint8_t *bytes_p = byte_code_p + bytes_copied;
      uint8_t flags;
      uint8_t bytes_copied_before_jump = 0;
      size_t branch_argument_length;
      size_t target_distance;
      size_t length;

      if (offset > 0)
      {
        bytes_copied_before_jump = page_p->bytes[offset - 1] & CBC_LOWER_SEVEN_BIT_MASK;
      }
      bytes_p += bytes_copied_before_jump;

      if (*bytes_p == CBC_EXT_OPCODE)
      {
        bytes_p++;
        flags = cbc_ext_flags[*bytes_p];
      }
      else
      {
        flags = cbc_flags[*bytes_p];
      }

      JERRY_ASSERT (flags & CBC_HAS_BRANCH_ARG);
      branch_argument_length = CBC_BRANCH_OFFSET_LENGTH (*bytes_p);
      bytes_p++;

      /* Decoding target. */
      length = branch_argument_length;
      target_distance = 0;
      do
      {
        target_distance = (target_distance << 8) | *bytes_p;
        bytes_p++;
      }
      while (--length > 0);

      if (CBC_BRANCH_IS_FORWARD (flags))
      {
        /* Branch target was not set. */
        JERRY_ASSERT (target_distance > 0);

        target_distance = parser_update_forward_branch (page_p,
                                                        offset + target_distance,
                                                        bytes_copied_before_jump);
      }
      else
      {
        if (target_distance < offset)
        {
          uint8_t bytes_copied_before_target = page_p->bytes[offset - target_distance - 1];
          bytes_copied_before_target = bytes_copied_before_target & CBC_LOWER_SEVEN_BIT_MASK;

          target_distance = (size_t) (bytes_copied_before_jump - bytes_copied_before_target);
        }
        else if (target_distance == offset)
        {
          target_distance = bytes_copied_before_jump;
        }
        else
        {
          target_distance = parser_update_backward_branch (prev_page_p,
                                                           target_distance - offset,
                                                           bytes_copied_before_jump);
        }
      }

      /* Encoding target again. */
      do
      {
        bytes_p--;
        *bytes_p = (uint8_t) (target_distance & 0xff);
        target_distance >>= 8;
      }
      while (--branch_argument_length > 0);
    }

    offset++;
    if (offset >= PARSER_CBC_STREAM_PAGE_SIZE)
    {
      parser_mem_page_t *next_p = page_p->next_p;

      /* We reverse the pages before the current page. */
      page_p->next_p = prev_page_p;
      prev_page_p = page_p;

      bytes_copied += page_p->bytes[PARSER_CBC_STREAM_PAGE_SIZE - 1] & CBC_LOWER_SEVEN_BIT_MASK;
      page_p = next_p;
      offset = 0;
    }
  }

  /* After this point the pages of the byte code stream are
   * not used anymore. However, they needs to be freed during
   * cleanup, so the first and last pointers of the stream
   * descriptor are reversed as well. */
  if (last_page_p != NULL)
  {
    JERRY_ASSERT (last_page_p == context_p->byte_code.last_p);
    last_page_p->next_p = prev_page_p;
  }
  else
  {
    last_page_p = context_p->byte_code.last_p;
  }

  context_p->byte_code.last_p = context_p->byte_code.first_p;
  context_p->byte_code.first_p = last_page_p;
} /* parse_update_branches */

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)

/**
 * Print literal.
 */
static void
parse_print_literal (ecma_compiled_code_t *compiled_code_p, /**< compiled code */
                     uint16_t literal_index, /**< literal index */
                     parser_list_t *literal_pool_p) /**< literal pool */
{
  parser_list_iterator_t literal_iterator;
  uint16_t argument_end;
  uint16_t register_end;
  uint16_t ident_end;

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) compiled_code_p;
    argument_end = args_p->argument_end;
    register_end = args_p->register_end;
    ident_end = args_p->ident_end;
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) compiled_code_p;
    argument_end = args_p->argument_end;
    register_end = args_p->register_end;
    ident_end = args_p->ident_end;
  }

#if ENABLED (JERRY_ES2015)
  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_REST_PARAMETER)
  {
    argument_end++;
  }
#endif /* ENABLED (JERRY_ES2015) */

  if (literal_index < argument_end)
  {
    JERRY_DEBUG_MSG (" arg:%d", literal_index);
    return;
  }

  if (literal_index < register_end)
  {
    JERRY_DEBUG_MSG (" reg:%d", literal_index);
    return;
  }

  parser_list_iterator_init (literal_pool_p, &literal_iterator);

  while (true)
  {
    lexer_literal_t *literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator);

    JERRY_ASSERT (literal_p != NULL);

    if (literal_p->prop.index == literal_index)
    {
      if (literal_index < ident_end)
      {
        JERRY_DEBUG_MSG (" ident:%d->", literal_index);
      }
      else
      {
        JERRY_DEBUG_MSG (" lit:%d->", literal_index);
      }

      util_print_literal (literal_p);
      return;
    }
  }
} /* parse_print_literal */

#define PARSER_READ_IDENTIFIER_INDEX(name) \
  name = *byte_code_p++; \
  if (name >= encoding_limit) \
  { \
    name = (uint16_t) (((name << 8) | byte_code_p[0]) - encoding_delta); \
    byte_code_p++; \
  }

/**
 * Print byte code.
 */
static void
parse_print_final_cbc (ecma_compiled_code_t *compiled_code_p, /**< compiled code */
                       parser_list_t *literal_pool_p, /**< literal pool */
                       size_t length) /**< length of byte code */
{
  uint8_t flags;
  uint8_t *byte_code_start_p;
  uint8_t *byte_code_end_p;
  uint8_t *byte_code_p;
  uint16_t encoding_limit;
  uint16_t encoding_delta;
  uint16_t stack_limit;
  uint16_t argument_end;
  uint16_t register_end;
  uint16_t ident_end;
  uint16_t const_literal_end;
  uint16_t literal_end;

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args = (cbc_uint16_arguments_t *) compiled_code_p;
    stack_limit = args->stack_limit;
    argument_end = args->argument_end;
    register_end = args->register_end;
    ident_end = args->ident_end;
    const_literal_end = args->const_literal_end;
    literal_end = args->literal_end;
  }
  else
  {
    cbc_uint8_arguments_t *args = (cbc_uint8_arguments_t *) compiled_code_p;
    stack_limit = args->stack_limit;
    argument_end = args->argument_end;
    register_end = args->register_end;
    ident_end = args->ident_end;
    const_literal_end = args->const_literal_end;
    literal_end = args->literal_end;
  }

  JERRY_DEBUG_MSG ("\nFinal byte code dump:\n\n  Maximum stack depth: %d\n  Flags: [",
                   (int) (stack_limit + register_end));

  if (!(compiled_code_p->status_flags & CBC_CODE_FLAGS_FULL_LITERAL_ENCODING))
  {
    JERRY_DEBUG_MSG ("small_lit_enc");
    encoding_limit = CBC_SMALL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_SMALL_LITERAL_ENCODING_DELTA;
  }
  else
  {
    JERRY_DEBUG_MSG ("full_lit_enc");
    encoding_limit = CBC_FULL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_FULL_LITERAL_ENCODING_DELTA;
  }

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    JERRY_DEBUG_MSG (",uint16_arguments");
  }

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_STRICT_MODE)
  {
    JERRY_DEBUG_MSG (",strict_mode");
  }

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_ARGUMENTS_NEEDED)
  {
    JERRY_DEBUG_MSG (",arguments_needed");
  }

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_LEXICAL_ENV_NOT_NEEDED)
  {
    JERRY_DEBUG_MSG (",no_lexical_env");
  }

#if ENABLED (JERRY_ES2015)
  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_ARROW_FUNCTION)
  {
    JERRY_DEBUG_MSG (",arrow");
  }

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_CONSTRUCTOR)
  {
    JERRY_DEBUG_MSG (",constructor");
  }
#endif /* ENABLED (JERRY_ES2015) */

  JERRY_DEBUG_MSG ("]\n");

  JERRY_DEBUG_MSG ("  Argument range end: %d\n", (int) argument_end);
  JERRY_DEBUG_MSG ("  Register range end: %d\n", (int) register_end);
  JERRY_DEBUG_MSG ("  Identifier range end: %d\n", (int) ident_end);
  JERRY_DEBUG_MSG ("  Const literal range end: %d\n", (int) const_literal_end);
  JERRY_DEBUG_MSG ("  Literal range end: %d\n\n", (int) literal_end);

  byte_code_start_p = (uint8_t *) compiled_code_p;

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    byte_code_start_p += sizeof (cbc_uint16_arguments_t);
  }
  else
  {
    byte_code_start_p += sizeof (cbc_uint8_arguments_t);
  }

  byte_code_start_p += (unsigned int) (literal_end - register_end) * sizeof (ecma_value_t);

  byte_code_end_p = byte_code_start_p + length;
  byte_code_p = byte_code_start_p;

  while (byte_code_p < byte_code_end_p)
  {
    cbc_opcode_t opcode = (cbc_opcode_t) *byte_code_p;
    cbc_ext_opcode_t ext_opcode = CBC_EXT_NOP;
    size_t cbc_offset = (size_t) (byte_code_p - byte_code_start_p);

    if (opcode != CBC_EXT_OPCODE)
    {
      flags = cbc_flags[opcode];
      JERRY_DEBUG_MSG (" %3d : %s", (int) cbc_offset, cbc_names[opcode]);
      byte_code_p++;
    }
    else
    {
      ext_opcode = (cbc_ext_opcode_t) byte_code_p[1];
      flags = cbc_ext_flags[ext_opcode];
      JERRY_DEBUG_MSG (" %3d : %s", (int) cbc_offset, cbc_ext_names[ext_opcode]);
      byte_code_p += 2;

#if ENABLED (JERRY_LINE_INFO)
      if (ext_opcode == CBC_EXT_LINE)
      {
        uint32_t value = 0;
        uint8_t byte;

        do
        {
          byte = *byte_code_p++;
          value = (value << 7) | (byte & CBC_LOWER_SEVEN_BIT_MASK);
        }
        while (byte & CBC_HIGHEST_BIT_MASK);

        JERRY_DEBUG_MSG (" %d\n", (int) value);
        continue;
      }
#endif /* ENABLED (JERRY_LINE_INFO) */
    }

    if (flags & (CBC_HAS_LITERAL_ARG | CBC_HAS_LITERAL_ARG2))
    {
      uint16_t literal_index;

      PARSER_READ_IDENTIFIER_INDEX (literal_index);
      parse_print_literal (compiled_code_p, literal_index, literal_pool_p);
    }

    if (flags & CBC_HAS_LITERAL_ARG2)
    {
      uint16_t literal_index;

      PARSER_READ_IDENTIFIER_INDEX (literal_index);
      parse_print_literal (compiled_code_p, literal_index, literal_pool_p);

      if (!(flags & CBC_HAS_LITERAL_ARG))
      {
        PARSER_READ_IDENTIFIER_INDEX (literal_index);
        parse_print_literal (compiled_code_p, literal_index, literal_pool_p);
      }
    }

    if (flags & CBC_HAS_BYTE_ARG)
    {
      if (opcode == CBC_PUSH_NUMBER_POS_BYTE
          || ext_opcode == CBC_EXT_PUSH_LITERAL_PUSH_NUMBER_POS_BYTE)
      {
        JERRY_DEBUG_MSG (" number:%d", (int) *byte_code_p + 1);
      }
      else if (opcode == CBC_PUSH_NUMBER_NEG_BYTE
               || ext_opcode == CBC_EXT_PUSH_LITERAL_PUSH_NUMBER_NEG_BYTE)
      {
        JERRY_DEBUG_MSG (" number:%d", -((int) *byte_code_p + 1));
      }
      else
      {
        JERRY_DEBUG_MSG (" byte_arg:%d", *byte_code_p);
      }
      byte_code_p++;
    }

    if (flags & CBC_HAS_BRANCH_ARG)
    {
      size_t branch_offset_length = (opcode != CBC_EXT_OPCODE ? CBC_BRANCH_OFFSET_LENGTH (opcode)
                                                              : CBC_BRANCH_OFFSET_LENGTH (ext_opcode));
      size_t offset = 0;

      do
      {
        offset = (offset << 8) | *byte_code_p++;
      }
      while (--branch_offset_length > 0);

      JERRY_DEBUG_MSG (" offset:%d(->%d)",
                       (int) offset,
                       (int) (cbc_offset + (CBC_BRANCH_IS_FORWARD (flags) ? offset : -offset)));
    }

    JERRY_DEBUG_MSG ("\n");
  }
} /* parse_print_final_cbc */

#undef PARSER_READ_IDENTIFIER_INDEX

#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

#if ENABLED (JERRY_DEBUGGER)

/**
 * Send current breakpoint list.
 */
static void
parser_send_breakpoints (parser_context_t *context_p, /**< context */
                         jerry_debugger_header_type_t type) /**< message type */
{
  JERRY_ASSERT (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED);
  JERRY_ASSERT (context_p->breakpoint_info_count > 0);

  jerry_debugger_send_data (type,
                            context_p->breakpoint_info,
                            context_p->breakpoint_info_count * sizeof (parser_breakpoint_info_t));

  context_p->breakpoint_info_count = 0;
} /* parser_send_breakpoints */

/**
 * Append a breakpoint info.
 */
void
parser_append_breakpoint_info (parser_context_t *context_p, /**< context */
                               jerry_debugger_header_type_t type, /**< message type */
                               uint32_t value) /**< line or offset of the breakpoint */
{
  JERRY_ASSERT (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED);

  context_p->status_flags |= PARSER_DEBUGGER_BREAKPOINT_APPENDED;

  if (context_p->breakpoint_info_count >= JERRY_DEBUGGER_SEND_MAX (parser_breakpoint_info_t))
  {
    parser_send_breakpoints (context_p, type);
  }

  context_p->breakpoint_info[context_p->breakpoint_info_count].value = value;
  context_p->breakpoint_info_count = (uint16_t) (context_p->breakpoint_info_count + 1);
} /* parser_append_breakpoint_info */

#endif /* ENABLED (JERRY_DEBUGGER) */

/**
 * Forward iterator: move to the next byte code
 *
 * @param page_p page
 * @param offset offset
 */
#define PARSER_NEXT_BYTE(page_p, offset) \
  do { \
    if (++(offset) >= PARSER_CBC_STREAM_PAGE_SIZE) \
    { \
      offset = 0; \
      page_p = page_p->next_p; \
    } \
  } while (0)

/**
 * Forward iterator: move to the next byte code. Also updates the offset of the previous byte code.
 *
 * @param page_p page
 * @param offset offset
 * @param real_offset real offset
 */
#define PARSER_NEXT_BYTE_UPDATE(page_p, offset, real_offset) \
  do { \
    page_p->bytes[offset] = real_offset; \
    if (++(offset) >= PARSER_CBC_STREAM_PAGE_SIZE) \
    { \
      offset = 0; \
      real_offset = 0; \
      page_p = page_p->next_p; \
    } \
  } while (0)

/**
 * Post processing main function.
 *
 * @return compiled code
 */
static ecma_compiled_code_t *
parser_post_processing (parser_context_t *context_p) /**< context */
{
  uint16_t literal_one_byte_limit;
  uint16_t ident_end;
  uint16_t const_literal_end;
  parser_mem_page_t *page_p;
  parser_mem_page_t *last_page_p;
  size_t last_position;
  size_t offset;
  size_t length;
  size_t literal_length;
  size_t total_size;
#if ENABLED (JERRY_SNAPSHOT_SAVE)
  size_t total_size_used;
#endif /* ENABLED (JERRY_SNAPSHOT_SAVE) */
  uint8_t real_offset;
  uint8_t *byte_code_p;
  bool needs_uint16_arguments;
  cbc_opcode_t last_opcode = CBC_EXT_OPCODE;
  ecma_compiled_code_t *compiled_code_p;
  ecma_value_t *literal_pool_p;
  uint8_t *dst_p;

  if ((size_t) context_p->stack_limit + (size_t) context_p->register_count > PARSER_MAXIMUM_STACK_LIMIT)
  {
    parser_raise_error (context_p, PARSER_ERR_STACK_LIMIT_REACHED);
  }

  JERRY_ASSERT (context_p->literal_count <= PARSER_MAXIMUM_NUMBER_OF_LITERALS);

#if ENABLED (JERRY_DEBUGGER)
  if ((JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
      && !(context_p->status_flags & PARSER_DEBUGGER_BREAKPOINT_APPENDED))
  {
    /* Always provide at least one breakpoint. */
    parser_emit_cbc (context_p, CBC_BREAKPOINT_DISABLED);
    parser_flush_cbc (context_p);

    parser_append_breakpoint_info (context_p, JERRY_DEBUGGER_BREAKPOINT_LIST, context_p->token.line);

    context_p->last_breakpoint_line = context_p->token.line;
  }

  if ((JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
      && context_p->breakpoint_info_count > 0)
  {
    parser_send_breakpoints (context_p, JERRY_DEBUGGER_BREAKPOINT_LIST);
    JERRY_ASSERT (context_p->breakpoint_info_count == 0);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  parser_compute_indicies (context_p, &ident_end, &const_literal_end);

  if (context_p->literal_count <= CBC_MAXIMUM_SMALL_VALUE)
  {
    literal_one_byte_limit = CBC_MAXIMUM_BYTE_VALUE - 1;
  }
  else
  {
    literal_one_byte_limit = CBC_LOWER_SEVEN_BIT_MASK;
  }

  last_page_p = context_p->byte_code.last_p;
  last_position = context_p->byte_code.last_position;

  if (last_position >= PARSER_CBC_STREAM_PAGE_SIZE)
  {
    last_page_p = NULL;
    last_position = 0;
  }

  page_p = context_p->byte_code.first_p;
  offset = 0;
  length = 0;

  while (page_p != last_page_p || offset < last_position)
  {
    uint8_t *opcode_p;
    uint8_t flags;
    size_t branch_offset_length;

    opcode_p = page_p->bytes + offset;
    last_opcode = (cbc_opcode_t) (*opcode_p);
    PARSER_NEXT_BYTE (page_p, offset);
    branch_offset_length = CBC_BRANCH_OFFSET_LENGTH (last_opcode);
    flags = cbc_flags[last_opcode];
    length++;

    if (last_opcode == CBC_EXT_OPCODE)
    {
      cbc_ext_opcode_t ext_opcode;

      ext_opcode = (cbc_ext_opcode_t) page_p->bytes[offset];
      branch_offset_length = CBC_BRANCH_OFFSET_LENGTH (ext_opcode);
      flags = cbc_ext_flags[ext_opcode];
      PARSER_NEXT_BYTE (page_p, offset);
      length++;

#if ENABLED (JERRY_ES2015)
      if (ext_opcode == CBC_EXT_CONSTRUCTOR_RETURN)
      {
        last_opcode = CBC_RETURN;
      }
#endif /* ENABLED (JERRY_ES2015) */

#if ENABLED (JERRY_LINE_INFO)
      if (ext_opcode == CBC_EXT_LINE)
      {
        uint8_t last_byte = 0;

        do
        {
          last_byte = page_p->bytes[offset];
          PARSER_NEXT_BYTE (page_p, offset);
          length++;
        }
        while (last_byte & CBC_HIGHEST_BIT_MASK);

        continue;
      }
#endif /* ENABLED (JERRY_LINE_INFO) */
    }

    while (flags & (CBC_HAS_LITERAL_ARG | CBC_HAS_LITERAL_ARG2))
    {
      uint8_t *first_byte = page_p->bytes + offset;
      uint32_t literal_index = *first_byte;

      PARSER_NEXT_BYTE (page_p, offset);
      length++;

      literal_index |= ((uint32_t) page_p->bytes[offset]) << 8;

      if (literal_index >= PARSER_REGISTER_START)
      {
        literal_index -= PARSER_REGISTER_START;
      }
      else
      {
        literal_index = (PARSER_GET_LITERAL (literal_index))->prop.index;
      }

      if (literal_index <= literal_one_byte_limit)
      {
        *first_byte = (uint8_t) literal_index;
      }
      else
      {
        if (context_p->literal_count <= CBC_MAXIMUM_SMALL_VALUE)
        {
          JERRY_ASSERT (literal_index <= CBC_MAXIMUM_SMALL_VALUE);
          *first_byte = CBC_MAXIMUM_BYTE_VALUE;
          page_p->bytes[offset] = (uint8_t) (literal_index - CBC_MAXIMUM_BYTE_VALUE);
          length++;
        }
        else
        {
          JERRY_ASSERT (literal_index <= CBC_MAXIMUM_FULL_VALUE);
          *first_byte = (uint8_t) ((literal_index >> 8) | CBC_HIGHEST_BIT_MASK);
          page_p->bytes[offset] = (uint8_t) (literal_index & 0xff);
          length++;
        }
      }
      PARSER_NEXT_BYTE (page_p, offset);

      if (flags & CBC_HAS_LITERAL_ARG2)
      {
        if (flags & CBC_HAS_LITERAL_ARG)
        {
          flags = CBC_HAS_LITERAL_ARG;
        }
        else
        {
          flags = CBC_HAS_LITERAL_ARG | CBC_HAS_LITERAL_ARG2;
        }
      }
      else
      {
        break;
      }
    }

    if (flags & CBC_HAS_BYTE_ARG)
    {
      /* This argument will be copied without modification. */
      PARSER_NEXT_BYTE (page_p, offset);
      length++;
    }

    if (flags & CBC_HAS_BRANCH_ARG)
    {
      bool prefix_zero = true;
#if PARSER_MAXIMUM_CODE_SIZE <= 65535
      cbc_opcode_t jump_forward = CBC_JUMP_FORWARD_2;
#else /* PARSER_MAXIMUM_CODE_SIZE > 65535 */
      cbc_opcode_t jump_forward = CBC_JUMP_FORWARD_3;
#endif /* PARSER_MAXIMUM_CODE_SIZE <= 65535 */

      /* The leading zeroes are dropped from the stream.
       * Although dropping these zeroes for backward
       * branches are unnecessary, we use the same
       * code path for simplicity. */
      JERRY_ASSERT (branch_offset_length > 0 && branch_offset_length <= 3);

      while (--branch_offset_length > 0)
      {
        uint8_t byte = page_p->bytes[offset];
        if (byte > 0 || !prefix_zero)
        {
          prefix_zero = false;
          length++;
        }
        else
        {
          JERRY_ASSERT (CBC_BRANCH_IS_FORWARD (flags));
        }
        PARSER_NEXT_BYTE (page_p, offset);
      }

      if (last_opcode == jump_forward
          && prefix_zero
          && page_p->bytes[offset] == CBC_BRANCH_OFFSET_LENGTH (jump_forward) + 1)
      {
        /* Uncoditional jumps which jump right after the instruction
         * are effectively NOPs. These jumps are removed from the
         * stream. The 1 byte long CBC_JUMP_FORWARD form marks these
         * instructions, since this form is constructed during post
         * processing and cannot be emitted directly. */
        *opcode_p = CBC_JUMP_FORWARD;
        length--;
      }
      else
      {
        /* Other last bytes are always copied. */
        length++;
      }

      PARSER_NEXT_BYTE (page_p, offset);
    }
  }

  if (!(context_p->status_flags & PARSER_NO_END_LABEL)
      || !(PARSER_OPCODE_IS_RETURN (last_opcode)))
  {
    context_p->status_flags &= (uint32_t) ~PARSER_NO_END_LABEL;
    length++;
  }

  needs_uint16_arguments = false;
  total_size = sizeof (cbc_uint8_arguments_t);

  if (context_p->stack_limit > CBC_MAXIMUM_BYTE_VALUE
      || context_p->register_count > CBC_MAXIMUM_BYTE_VALUE
      || context_p->literal_count > CBC_MAXIMUM_BYTE_VALUE)
  {
    needs_uint16_arguments = true;
    total_size = sizeof (cbc_uint16_arguments_t);
  }

  literal_length = (size_t) (context_p->literal_count - context_p->register_count) * sizeof (ecma_value_t);

  total_size += literal_length + length;

  if ((context_p->status_flags & PARSER_ARGUMENTS_NEEDED)
      && !(context_p->status_flags & PARSER_IS_STRICT))
  {
    total_size += context_p->argument_count * sizeof (ecma_value_t);
  }

#if ENABLED (JERRY_LINE_INFO) || ENABLED (JERRY_ES2015_MODULE_SYSTEM)
  if (JERRY_CONTEXT (resource_name) != ECMA_VALUE_UNDEFINED)
  {
    total_size += sizeof (ecma_value_t);
  }
#endif /* ENABLED (JERRY_LINE_INFO) || ENABLED (JERRY_ES2015_MODULE_SYSTEM) */

#if ENABLED (JERRY_SNAPSHOT_SAVE)
  total_size_used = total_size;
#endif /* ENABLED (JERRY_SNAPSHOT_SAVE) */
  total_size = JERRY_ALIGNUP (total_size, JMEM_ALIGNMENT);

  compiled_code_p = (ecma_compiled_code_t *) parser_malloc (context_p, total_size);

#if ENABLED (JERRY_SNAPSHOT_SAVE)
  // Avoid getting junk bytes at the end when bytes at the end remain unused:
  if (total_size_used < total_size)
  {
    memset (((uint8_t *) compiled_code_p) + total_size_used, 0, total_size - total_size_used);
  }
#endif /* ENABLED (JERRY_SNAPSHOT_SAVE) */

#if ENABLED (JERRY_MEM_STATS)
  jmem_stats_allocate_byte_code_bytes (total_size);
#endif /* ENABLED (JERRY_MEM_STATS) */

  byte_code_p = (uint8_t *) compiled_code_p;
  compiled_code_p->size = (uint16_t) (total_size >> JMEM_ALIGNMENT_LOG);
  compiled_code_p->refs = 1;
  compiled_code_p->status_flags = CBC_CODE_FLAGS_FUNCTION;

#if ENABLED (JERRY_ES2015)
  if (context_p->status_flags & PARSER_FUNCTION_HAS_REST_PARAM)
  {
    JERRY_ASSERT (context_p->argument_count > 0);
    context_p->argument_count--;
  }
#endif /* ENABLED (JERRY_ES2015) */

  if (needs_uint16_arguments)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) compiled_code_p;

    args_p->stack_limit = context_p->stack_limit;
    args_p->argument_end = context_p->argument_count;
    args_p->register_end = context_p->register_count;
    args_p->ident_end = ident_end;
    args_p->const_literal_end = const_literal_end;
    args_p->literal_end = context_p->literal_count;

    compiled_code_p->status_flags |= CBC_CODE_FLAGS_UINT16_ARGUMENTS;
    byte_code_p += sizeof (cbc_uint16_arguments_t);
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) compiled_code_p;

    args_p->stack_limit = (uint8_t) context_p->stack_limit;
    args_p->argument_end = (uint8_t) context_p->argument_count;
    args_p->register_end = (uint8_t) context_p->register_count;
    args_p->ident_end = (uint8_t) ident_end;
    args_p->const_literal_end = (uint8_t) const_literal_end;
    args_p->literal_end = (uint8_t) context_p->literal_count;

    byte_code_p += sizeof (cbc_uint8_arguments_t);
  }

  uint16_t encoding_limit;
  uint16_t encoding_delta;

  if (context_p->literal_count > CBC_MAXIMUM_SMALL_VALUE)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_FULL_LITERAL_ENCODING;
    encoding_limit = CBC_FULL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_FULL_LITERAL_ENCODING_DELTA;
  }
  else
  {
    encoding_limit = CBC_SMALL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_SMALL_LITERAL_ENCODING_DELTA;
  }

  if (context_p->status_flags & PARSER_IS_STRICT)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_STRICT_MODE;
  }

  if (context_p->status_flags & PARSER_ARGUMENTS_NEEDED)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_ARGUMENTS_NEEDED;

    /* Arguments is stored in the lexical environment. */
    context_p->status_flags |= PARSER_LEXICAL_ENV_NEEDED;
  }

  if (!(context_p->status_flags & PARSER_LEXICAL_ENV_NEEDED))
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_LEXICAL_ENV_NOT_NEEDED;
  }

#if ENABLED (JERRY_ES2015)
  if (context_p->status_flags & PARSER_IS_ARROW_FUNCTION)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_ARROW_FUNCTION;
  }

  if (context_p->status_flags & PARSER_CLASS_CONSTRUCTOR)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_CONSTRUCTOR;
  }

  if (context_p->status_flags & PARSER_FUNCTION_HAS_REST_PARAM)
  {
    compiled_code_p->status_flags |= CBC_CODE_FLAGS_REST_PARAMETER;
  }
#endif /* ENABLED (JERRY_ES2015) */

  literal_pool_p = ((ecma_value_t *) byte_code_p) - context_p->register_count;
  byte_code_p += literal_length;
  dst_p = byte_code_p;

  parser_init_literal_pool (context_p, literal_pool_p);

  page_p = context_p->byte_code.first_p;
  offset = 0;
  real_offset = 0;
  uint8_t last_register_index = (uint8_t) JERRY_MIN (context_p->register_count,
                                                     (PARSER_MAXIMUM_NUMBER_OF_REGISTERS - 1));

  while (page_p != last_page_p || offset < last_position)
  {
    uint8_t flags;
    uint8_t *opcode_p;
    uint8_t *branch_mark_p;
    cbc_opcode_t opcode;
    size_t branch_offset_length;

    opcode_p = dst_p;
    branch_mark_p = page_p->bytes + offset;
    opcode = (cbc_opcode_t) (*branch_mark_p);
    branch_offset_length = CBC_BRANCH_OFFSET_LENGTH (opcode);

    if (opcode == CBC_JUMP_FORWARD)
    {
      /* These opcodes are deleted from the stream. */
#if PARSER_MAXIMUM_CODE_SIZE <= 65535
      size_t counter = 3;
#else /* PARSER_MAXIMUM_CODE_SIZE > 65535 */
      size_t counter = 4;
#endif /* PARSER_MAXIMUM_CODE_SIZE <= 65535 */

      do
      {
        PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
      }
      while (--counter > 0);

      continue;
    }

    /* Storing the opcode */
    *dst_p++ = (uint8_t) opcode;
    real_offset++;
    PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
    flags = cbc_flags[opcode];

#if ENABLED (JERRY_DEBUGGER)
    if (opcode == CBC_BREAKPOINT_DISABLED)
    {
      uint32_t bp_offset = (uint32_t) (((uint8_t *) dst_p) - ((uint8_t *) compiled_code_p) - 1);
      parser_append_breakpoint_info (context_p, JERRY_DEBUGGER_BREAKPOINT_OFFSET_LIST, bp_offset);
    }
#endif /* ENABLED (JERRY_DEBUGGER) */

    if (opcode == CBC_EXT_OPCODE)
    {
      cbc_ext_opcode_t ext_opcode;

      ext_opcode = (cbc_ext_opcode_t) page_p->bytes[offset];
      flags = cbc_ext_flags[ext_opcode];
      branch_offset_length = CBC_BRANCH_OFFSET_LENGTH (ext_opcode);

      /* Storing the extended opcode */
      *dst_p++ = (uint8_t) ext_opcode;
      opcode_p++;
      real_offset++;
      PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);

#if ENABLED (JERRY_LINE_INFO)
      if (ext_opcode == CBC_EXT_LINE)
      {
        uint8_t last_byte = 0;

        do
        {
          last_byte = page_p->bytes[offset];
          *dst_p++ = last_byte;

          real_offset++;
          PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
        }
        while (last_byte & CBC_HIGHEST_BIT_MASK);

        continue;
      }
#endif /* ENABLED (JERRY_LINE_INFO) */
    }

    /* Only literal and call arguments can be combined. */
    JERRY_ASSERT (!(flags & CBC_HAS_BRANCH_ARG)
                   || !(flags & (CBC_HAS_BYTE_ARG | CBC_HAS_LITERAL_ARG)));

    while (flags & (CBC_HAS_LITERAL_ARG | CBC_HAS_LITERAL_ARG2))
    {
      uint16_t first_byte = page_p->bytes[offset];

      uint8_t *opcode_pos_p = dst_p - 1;
      *dst_p++ = (uint8_t) first_byte;
      real_offset++;
      PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);

      if (first_byte > literal_one_byte_limit)
      {
        *dst_p++ = page_p->bytes[offset];

        if (first_byte > encoding_limit)
        {
          first_byte = (uint16_t) (((first_byte << 8) | dst_p[-1]) - encoding_delta);
        }
        real_offset++;
      }
      PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);

      if (flags & CBC_HAS_LITERAL_ARG2)
      {
        if (flags & CBC_HAS_LITERAL_ARG)
        {
          flags = CBC_HAS_LITERAL_ARG;
        }
        else
        {
          flags = CBC_HAS_LITERAL_ARG | CBC_HAS_LITERAL_ARG2;
        }
      }
      else
      {
        if (opcode == CBC_ASSIGN_SET_IDENT && JERRY_LIKELY (first_byte < last_register_index))
        {
          *opcode_pos_p = CBC_MOV_IDENT;
        }

        break;
      }
    }

    if (flags & CBC_HAS_BYTE_ARG)
    {
      /* This argument will be copied without modification. */
      *dst_p++ = page_p->bytes[offset];
      real_offset++;
      PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
      continue;
    }

    if (flags & CBC_HAS_BRANCH_ARG)
    {
      *branch_mark_p |= CBC_HIGHEST_BIT_MASK;
      bool prefix_zero = true;

      /* The leading zeroes are dropped from the stream. */
      JERRY_ASSERT (branch_offset_length > 0 && branch_offset_length <= 3);

      while (--branch_offset_length > 0)
      {
        uint8_t byte = page_p->bytes[offset];
        if (byte > 0 || !prefix_zero)
        {
          prefix_zero = false;
          *dst_p++ = page_p->bytes[offset];
          real_offset++;
        }
        else
        {
          /* When a leading zero is dropped, the branch
           * offset length must be decreased as well. */
          (*opcode_p)--;
        }
        PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
      }

      *dst_p++ = page_p->bytes[offset];
      real_offset++;
      PARSER_NEXT_BYTE_UPDATE (page_p, offset, real_offset);
      continue;
    }
  }

#if ENABLED (JERRY_DEBUGGER)
  if ((JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
      && context_p->breakpoint_info_count > 0)
  {
    parser_send_breakpoints (context_p, JERRY_DEBUGGER_BREAKPOINT_OFFSET_LIST);
    JERRY_ASSERT (context_p->breakpoint_info_count == 0);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  if (!(context_p->status_flags & PARSER_NO_END_LABEL))
  {
    *dst_p++ = CBC_RETURN_WITH_BLOCK;
  }
  JERRY_ASSERT (dst_p == byte_code_p + length);

  parse_update_branches (context_p, byte_code_p);

  parser_cbc_stream_free (&context_p->byte_code);

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes)
  {
    parser_list_iterator_t literal_iterator;
    lexer_literal_t *literal_p;

    parse_print_final_cbc (compiled_code_p, &context_p->literal_pool, length);
    JERRY_DEBUG_MSG ("\nByte code size: %d bytes\n", (int) length);
    context_p->total_byte_code_size += (uint32_t) length;

    parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);
    while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)))
    {
      if ((literal_p->type == LEXER_IDENT_LITERAL || literal_p->type == LEXER_STRING_LITERAL)
          && !(literal_p->status_flags & LEXER_FLAG_SOURCE_PTR))
      {
        jmem_heap_free_block ((void *) literal_p->u.char_p, literal_p->prop.length);
      }
    }
  }
#else /* !ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
  if (context_p->status_flags & PARSER_HAS_LATE_LIT_INIT)
  {
    parser_list_iterator_t literal_iterator;
    lexer_literal_t *literal_p;
    uint16_t register_count = context_p->register_count;

    parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);
    while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)))
    {
      if ((literal_p->status_flags & LEXER_FLAG_LATE_INIT)
          && literal_p->prop.index >= register_count)
      {
        uint32_t source_data = literal_p->u.source_data;
        const uint8_t *char_p = context_p->source_end_p - (source_data & 0xfffff);
        ecma_value_t lit_value = ecma_find_or_create_literal_string (char_p,
                                                                     source_data >> 20);
        literal_pool_p[literal_p->prop.index] = lit_value;
      }
    }
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

  if ((context_p->status_flags & PARSER_ARGUMENTS_NEEDED)
      && !(context_p->status_flags & PARSER_IS_STRICT))
  {
    parser_list_iterator_t literal_iterator;
    uint16_t argument_count = 0;
    uint16_t register_count = context_p->register_count;
    ecma_value_t *argument_base_p = (ecma_value_t *) (((uint8_t *) compiled_code_p) + total_size);
    argument_base_p -= context_p->argument_count;

    parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);
    while (argument_count < context_p->argument_count)
    {
      lexer_literal_t *literal_p;
      literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator);

      JERRY_ASSERT (literal_p != NULL);

      if (!(literal_p->status_flags & LEXER_FLAG_FUNCTION_ARGUMENT))
      {
        continue;
      }

      /* All arguments must be moved to initialized registers. */
      if (literal_p->type == LEXER_UNUSED_LITERAL)
      {
        argument_base_p[argument_count] = ECMA_VALUE_EMPTY;
        argument_count++;
        continue;
      }

      JERRY_ASSERT (literal_p->type == LEXER_IDENT_LITERAL);

      JERRY_ASSERT (literal_p->prop.index >= register_count);

      argument_base_p[argument_count] = literal_pool_p[literal_p->prop.index];
      argument_count++;
    }
  }

#if ENABLED (JERRY_LINE_INFO) || ENABLED (JERRY_ES2015_MODULE_SYSTEM)
  if (JERRY_CONTEXT (resource_name) != ECMA_VALUE_UNDEFINED)
  {
    ecma_value_t *resource_name_p = (ecma_value_t *) (((uint8_t *) compiled_code_p) + total_size);

    if ((context_p->status_flags & PARSER_ARGUMENTS_NEEDED)
        && !(context_p->status_flags & PARSER_IS_STRICT))
    {
      resource_name_p -= context_p->argument_count;
    }

    resource_name_p[-1] = JERRY_CONTEXT (resource_name);
  }
#endif /* ENABLED (JERRY_LINE_INFO) || ENABLED (JERRY_ES2015_MODULE_SYSTEM) */

#if ENABLED (JERRY_DEBUGGER)
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    jerry_debugger_send_function_cp (JERRY_DEBUGGER_BYTE_CODE_CP, compiled_code_p);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  return compiled_code_p;
} /* parser_post_processing */

#undef PARSER_NEXT_BYTE
#undef PARSER_NEXT_BYTE_UPDATE

/**
 * Free identifiers and literals.
 */
static void
parser_free_literals (parser_list_t *literal_pool_p) /**< literals */
{
  parser_list_iterator_t literal_iterator;
  lexer_literal_t *literal_p;

  parser_list_iterator_init (literal_pool_p, &literal_iterator);
  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)) != NULL)
  {
    util_free_literal (literal_p);
  }

  parser_list_free (literal_pool_p);
} /* parser_free_literals */

/**
 * Parse function arguments
 */
static void
parser_parse_function_arguments (parser_context_t *context_p, /**< context */
                                 lexer_token_type_t end_type) /**< expected end type */
{
#if ENABLED (JERRY_ES2015)
  bool duplicated_argument_names = false;
  bool initializer_found = false;
#endif /* ENABLED (JERRY_ES2015) */

  JERRY_ASSERT (context_p->next_scanner_info_p->type == SCANNER_TYPE_FUNCTION);
  scanner_create_variables (context_p, SCANNER_CREATE_VARS_NO_OPTS);

  if (context_p->token.type == end_type)
  {
    return;
  }

  while (true)
  {
#if ENABLED (JERRY_ES2015)
    if (context_p->status_flags & PARSER_FUNCTION_HAS_REST_PARAM)
    {
      parser_raise_error (context_p, PARSER_ERR_FORMAL_PARAM_AFTER_REST_PARAMETER);
    }
    else if (context_p->token.type == LEXER_THREE_DOTS)
    {
      lexer_expect_identifier (context_p, LEXER_IDENT_LITERAL);

      if (duplicated_argument_names)
      {
        parser_raise_error (context_p, PARSER_ERR_DUPLICATED_ARGUMENT_NAMES);
      }

      context_p->status_flags |= PARSER_FUNCTION_HAS_REST_PARAM;
    }
#endif /* ENABLED (JERRY_ES2015) */

    if (context_p->token.type != LEXER_LITERAL
        || context_p->token.lit_location.type != LEXER_IDENT_LITERAL)
    {
      parser_raise_error (context_p, PARSER_ERR_IDENTIFIER_EXPECTED);
    }

    lexer_construct_literal_object (context_p,
                                    &context_p->token.lit_location,
                                    LEXER_IDENT_LITERAL);

    if (context_p->token.literal_is_reserved
        || context_p->lit_object.type != LEXER_LITERAL_OBJECT_ANY)
    {
      context_p->status_flags |= PARSER_HAS_NON_STRICT_ARG;
    }

    if (JERRY_UNLIKELY (context_p->lit_object.literal_p->status_flags & LEXER_FLAG_FUNCTION_ARGUMENT))
    {
#if ENABLED (JERRY_ES2015)
      if (initializer_found)
      {
        parser_raise_error (context_p, PARSER_ERR_DUPLICATED_ARGUMENT_NAMES);
      }
#endif /* ENABLED (JERRY_ES2015) */
#if ENABLED (JERRY_ES2015)
      duplicated_argument_names = true;
#endif /* ENABLED (JERRY_ES2015) */

      context_p->status_flags |= PARSER_HAS_NON_STRICT_ARG;
    }
    else
    {
      context_p->lit_object.literal_p->status_flags |= LEXER_FLAG_FUNCTION_ARGUMENT;
    }

    context_p->argument_count++;
    if (context_p->argument_count >= PARSER_MAXIMUM_NUMBER_OF_REGISTERS)
    {
      parser_raise_error (context_p, PARSER_ERR_ARGUMENT_LIMIT_REACHED);
    }

    lexer_next_token (context_p);

#if ENABLED (JERRY_ES2015)
    if (context_p->token.type == LEXER_ASSIGN)
    {
      if (context_p->status_flags & PARSER_FUNCTION_HAS_REST_PARAM)
      {
        parser_raise_error (context_p, PARSER_ERR_REST_PARAMETER_DEFAULT_INITIALIZER);
      }

      parser_branch_t skip_init;

      if (duplicated_argument_names)
      {
        parser_raise_error (context_p, PARSER_ERR_DUPLICATED_ARGUMENT_NAMES);
      }
      initializer_found = true;

      /* LEXER_ASSIGN does not overwrite lit_object. */
      parser_emit_cbc (context_p, CBC_PUSH_UNDEFINED);
      parser_emit_cbc_literal (context_p, CBC_STRICT_EQUAL_RIGHT_LITERAL, context_p->lit_object.index);
      parser_emit_cbc_forward_branch (context_p, CBC_BRANCH_IF_FALSE_FORWARD, &skip_init);

      parser_emit_cbc_literal_from_token (context_p, CBC_PUSH_LITERAL);
      parser_parse_expression_statement (context_p, PARSE_EXPR_NO_COMMA | PARSE_EXPR_HAS_LITERAL);

      parser_set_branch_to_current_position (context_p, &skip_init);
    }
#endif /* ENABLED (JERRY_ES2015) */

    if (context_p->token.type != LEXER_COMMA)
    {
      break;
    }

    lexer_next_token (context_p);
  }

  if (context_p->token.type != end_type)
  {
    parser_error_t error = ((end_type == LEXER_RIGHT_PAREN) ? PARSER_ERR_RIGHT_PAREN_EXPECTED
                                                            : PARSER_ERR_IDENTIFIER_EXPECTED);

    parser_raise_error (context_p, error);
  }
} /* parser_parse_function_arguments */

/**
 * Parse and compile EcmaScript source code
 *
 * Note: source must be a valid UTF-8 string
 *
 * @return compiled code
 */
static ecma_compiled_code_t *
parser_parse_source (const uint8_t *arg_list_p, /**< function argument list */
                     size_t arg_list_size, /**< size of function argument list */
                     const uint8_t *source_p, /**< valid UTF-8 source code */
                     size_t source_size, /**< size of the source code */
                     uint32_t parse_opts, /**< ecma_parse_opts_t option bits */
                     parser_error_location_t *error_location_p) /**< error location */
{
  parser_context_t context;
  ecma_compiled_code_t *compiled_code_p;

  context.error = PARSER_ERR_NO_ERROR;

  if (error_location_p != NULL)
  {
    error_location_p->error = PARSER_ERR_NO_ERROR;
  }

  if (arg_list_p == NULL)
  {
    context.status_flags = PARSER_ARGUMENTS_NOT_NEEDED;
  }
  else
  {
    context.status_flags = PARSER_IS_FUNCTION;
  }

#if ENABLED (JERRY_ES2015)
  context.status_flags |= PARSER_GET_CLASS_PARSER_OPTS (parse_opts);
#endif /* ENABLED (JERRY_ES2015) */

  context.stack_depth = 0;
  context.stack_limit = 0;
  context.last_context_p = NULL;
  context.last_statement.current_p = NULL;
  context.status_flags |= parse_opts & PARSER_STRICT_MODE_MASK;

  context.token.flags = 0;
  context.line = 1;
  context.column = 1;

  scanner_info_t scanner_info_end;
  scanner_info_end.next_p = NULL;
  scanner_info_end.source_p = NULL;
  scanner_info_end.type = SCANNER_TYPE_END;
  context.next_scanner_info_p = &scanner_info_end;
  context.active_scanner_info_p = NULL;
  context.skipped_scanner_info_p = NULL;
  context.skipped_scanner_info_end_p = NULL;

  context.last_cbc_opcode = PARSER_CBC_UNAVAILABLE;

  context.argument_count = 0;
  context.register_count = 0;
  context.literal_count = 0;

  parser_cbc_stream_init (&context.byte_code);
  context.byte_code_size = 0;
  parser_list_init (&context.literal_pool,
                    sizeof (lexer_literal_t),
                    (uint32_t) ((128 - sizeof (void *)) / sizeof (lexer_literal_t)));
  context.scope_stack_p = NULL;
  context.scope_stack_size = 0;
  context.scope_stack_top = 0;
  context.scope_stack_reg_top = 0;
#if ENABLED (JERRY_ES2015)
  context.scope_stack_global_end = 0;
#endif /* ENABLED (JERRY_ES2015) */

#ifndef JERRY_NDEBUG
  context.context_stack_depth = 0;
#endif /* !JERRY_NDEBUG */

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  context.is_show_opcodes = (JERRY_CONTEXT (jerry_init_flags) & ECMA_INIT_SHOW_OPCODES);
  context.total_byte_code_size = 0;

  if (context.is_show_opcodes)
  {
    JERRY_DEBUG_MSG ("\n--- %s parsing start ---\n\n",
                     (arg_list_p == NULL) ? "Script"
                                          : "Function");
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

#if ENABLED (JERRY_ES2015)
  if (parse_opts & ECMA_PARSE_DIRECT_EVAL)
  {
    context.status_flags |= PARSER_IS_EVAL;
  }
#endif /* ENABLED (JERRY_ES2015) */

  scanner_scan_all (&context,
                    arg_list_p,
                    arg_list_p + arg_list_size,
                    source_p,
                    source_p + source_size);

  if (JERRY_UNLIKELY (context.error != PARSER_ERR_NO_ERROR))
  {
    JERRY_ASSERT (context.error == PARSER_ERR_OUT_OF_MEMORY);

    if (error_location_p != NULL)
    {
      error_location_p->error = context.error;
      error_location_p->line = context.token.line;
      error_location_p->column = context.token.column;
    }
    return NULL;
  }

  if (arg_list_p == NULL)
  {
    context.source_p = source_p;
    context.source_end_p = source_p + source_size;
  }
  else
  {
    context.source_p = arg_list_p;
    context.source_end_p = arg_list_p + arg_list_size;
  }

  context.u.allocated_buffer_p = NULL;
  context.line = 1;
  context.column = 1;
  context.token.flags = 0;

  parser_stack_init (&context);

#if ENABLED (JERRY_DEBUGGER)
  context.breakpoint_info_count = 0;
#endif /* ENABLED (JERRY_DEBUGGER) */

#if ENABLED (JERRY_ES2015_MODULE_SYSTEM)
  if (context.status_flags & PARSER_IS_MODULE)
  {
    context.status_flags |= PARSER_IS_STRICT;
  }

  if (parse_opts & ECMA_PARSE_EVAL)
  {
    /* After this point this flag is set for non-direct evals as well. */
    context.status_flags |= PARSER_IS_EVAL;
  }

  context.module_current_node_p = NULL;
#endif /* ENABLED (JERRY_ES2015_MODULE_SYSTEM) */

  PARSER_TRY (context.try_buffer)
  {
    /* Pushing a dummy value ensures the stack is never empty.
     * This simplifies the stack management routines. */
    parser_stack_push_uint8 (&context, CBC_MAXIMUM_BYTE_VALUE);
    /* The next token must always be present to make decisions
     * in the parser. Therefore when a token is consumed, the
     * lexer_next_token() must be immediately called. */
    lexer_next_token (&context);

    if (arg_list_p != NULL)
    {
      parser_parse_function_arguments (&context, LEXER_EOS);

      JERRY_ASSERT (context.next_scanner_info_p->type == SCANNER_TYPE_END_ARGUMENTS);
      scanner_release_next (&context, sizeof (scanner_info_t));

      context.source_p = source_p;
      context.source_end_p = source_p + source_size;
      context.line = 1;
      context.column = 1;

      lexer_next_token (&context);
    }
#if ENABLED (JERRY_ES2015_MODULE_SYSTEM)
    else if (parse_opts & ECMA_PARSE_MODULE)
    {
      scanner_create_variables (&context, SCANNER_CREATE_VARS_NO_OPTS);
    }
#endif /* ENABLED (JERRY_ES2015_MODULE_SYSTEM) */
    else
    {
      JERRY_ASSERT (context.next_scanner_info_p->source_p == source_p
                    && context.next_scanner_info_p->type == SCANNER_TYPE_FUNCTION);

#if ENABLED (JERRY_ES2015)
      if (scanner_is_global_context_needed (&context))
      {
        parser_branch_t branch;

#ifndef JERRY_NDEBUG
        PARSER_PLUS_EQUAL_U16 (context.context_stack_depth, PARSER_BLOCK_CONTEXT_STACK_ALLOCATION);
#endif /* !JERRY_NDEBUG */

        parser_emit_cbc_forward_branch (&context,
                                        CBC_BLOCK_CREATE_CONTEXT,
                                        &branch);

        parser_stack_push (&context, &branch, sizeof (parser_branch_t));
        context.status_flags |= PARSER_INSIDE_BLOCK;
      }
#endif /* ENABLED (JERRY_ES2015) */

      scanner_create_variables (&context, SCANNER_CREATE_VARS_IS_EVAL);
    }

    parser_parse_statements (&context);

#if ENABLED (JERRY_ES2015)
    if (context.status_flags & PARSER_INSIDE_BLOCK)
    {
      parser_branch_t branch;
      parser_stack_pop (&context, &branch, sizeof (parser_branch_t));

      parser_emit_cbc (&context, CBC_CONTEXT_END);
      parser_set_branch_to_current_position (&context, &branch);
      parser_flush_cbc (&context);
    }
#endif /* ENABLED (JERRY_ES2015) */

    /* When the parsing is successful, only the
     * dummy value can be remained on the stack. */
    JERRY_ASSERT (context.stack_top_uint8 == CBC_MAXIMUM_BYTE_VALUE
                  && context.stack.last_position == 1
                  && context.stack.first_p != NULL
                  && context.stack.first_p->next_p == NULL
                  && context.stack.last_p == NULL);
    JERRY_ASSERT (context.last_statement.current_p == NULL);

    JERRY_ASSERT (context.last_cbc_opcode == PARSER_CBC_UNAVAILABLE);
    JERRY_ASSERT (context.u.allocated_buffer_p == NULL);

    compiled_code_p = parser_post_processing (&context);
    parser_list_free (&context.literal_pool);

#ifndef JERRY_NDEBUG
    JERRY_ASSERT (context.status_flags & PARSER_SCANNING_SUCCESSFUL);
#endif /* !JERRY_NDEBUG */

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
    if (context.is_show_opcodes)
    {
      JERRY_DEBUG_MSG ("\n%s parsing successfully completed. Total byte code size: %d bytes\n",
                       (arg_list_p == NULL) ? "Script"
                                            : "Function",
                       (int) context.total_byte_code_size);
    }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */
  }
  PARSER_CATCH
  {
    if (context.last_statement.current_p != NULL)
    {
      parser_free_jumps (context.last_statement);
    }

    if (context.u.allocated_buffer_p != NULL)
    {
      parser_free_local (context.u.allocated_buffer_p,
                         context.allocated_buffer_size);
    }

    scanner_cleanup (&context);

#if ENABLED (JERRY_ES2015_MODULE_SYSTEM)
    if (context.module_current_node_p != NULL)
    {
      ecma_module_release_module_nodes (context.module_current_node_p);
    }
#endif

    if (error_location_p != NULL)
    {
      error_location_p->error = context.error;
      error_location_p->line = context.token.line;
      error_location_p->column = context.token.column;
    }

    compiled_code_p = NULL;
    parser_free_literals (&context.literal_pool);
    parser_cbc_stream_free (&context.byte_code);
  }
  PARSER_TRY_END

  if (context.scope_stack_p != NULL)
  {
    parser_free (context.scope_stack_p, context.scope_stack_size * sizeof (parser_scope_stack));
  }

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context.is_show_opcodes)
  {
    JERRY_DEBUG_MSG ("\n--- %s parsing end ---\n\n",
                     (arg_list_p == NULL) ? "Script"
                                          : "Function");
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

  parser_stack_free (&context);

  return compiled_code_p;
} /* parser_parse_source */

/**
 * Save parser context before function parsing.
 */
static void
parser_save_context (parser_context_t *context_p, /**< context */
                     parser_saved_context_t *saved_context_p) /**< target for saving the context */
{
  JERRY_ASSERT (context_p->last_cbc_opcode == PARSER_CBC_UNAVAILABLE);

#if ENABLED (JERRY_DEBUGGER)
  if ((JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
      && context_p->breakpoint_info_count > 0)
  {
    parser_send_breakpoints (context_p, JERRY_DEBUGGER_BREAKPOINT_LIST);
    context_p->breakpoint_info_count = 0;
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  /* Save private part of the context. */

  saved_context_p->status_flags = context_p->status_flags;
  saved_context_p->stack_depth = context_p->stack_depth;
  saved_context_p->stack_limit = context_p->stack_limit;
  saved_context_p->prev_context_p = context_p->last_context_p;
  saved_context_p->last_statement = context_p->last_statement;

  saved_context_p->argument_count = context_p->argument_count;
  saved_context_p->register_count = context_p->register_count;
  saved_context_p->literal_count = context_p->literal_count;

  saved_context_p->byte_code = context_p->byte_code;
  saved_context_p->byte_code_size = context_p->byte_code_size;
  saved_context_p->literal_pool_data = context_p->literal_pool.data;
  saved_context_p->scope_stack_p = context_p->scope_stack_p;
  saved_context_p->scope_stack_size = context_p->scope_stack_size;
  saved_context_p->scope_stack_top = context_p->scope_stack_top;
  saved_context_p->scope_stack_reg_top = context_p->scope_stack_reg_top;
#if ENABLED (JERRY_ES2015)
  saved_context_p->scope_stack_global_end = context_p->scope_stack_global_end;
#endif /* ENABLED (JERRY_ES2015) */

#ifndef JERRY_NDEBUG
  saved_context_p->context_stack_depth = context_p->context_stack_depth;
#endif /* !JERRY_NDEBUG */

  /* Reset private part of the context. */

  context_p->status_flags &= PARSER_IS_STRICT;
  context_p->stack_depth = 0;
  context_p->stack_limit = 0;
  context_p->last_context_p = saved_context_p;
  context_p->last_statement.current_p = NULL;

  context_p->argument_count = 0;
  context_p->register_count = 0;
  context_p->literal_count = 0;

  parser_cbc_stream_init (&context_p->byte_code);
  context_p->byte_code_size = 0;
  parser_list_reset (&context_p->literal_pool);
  context_p->scope_stack_p = NULL;
  context_p->scope_stack_size = 0;
  context_p->scope_stack_top = 0;
  context_p->scope_stack_reg_top = 0;
#if ENABLED (JERRY_ES2015)
  context_p->scope_stack_global_end = 0;
#endif /* ENABLED (JERRY_ES2015) */

#ifndef JERRY_NDEBUG
  context_p->context_stack_depth = 0;
#endif /* !JERRY_NDEBUG */
} /* parser_save_context */

/**
 * Restore parser context after function parsing.
 */
static void
parser_restore_context (parser_context_t *context_p, /**< context */
                        parser_saved_context_t *saved_context_p) /**< target for saving the context */
{
  parser_list_free (&context_p->literal_pool);

  if (context_p->scope_stack_p != NULL)
  {
    parser_free (context_p->scope_stack_p, context_p->scope_stack_size * sizeof (parser_scope_stack));
  }

  /* Restore private part of the context. */

  JERRY_ASSERT (context_p->last_cbc_opcode == PARSER_CBC_UNAVAILABLE);

  context_p->status_flags = saved_context_p->status_flags;
  context_p->stack_depth = saved_context_p->stack_depth;
  context_p->stack_limit = saved_context_p->stack_limit;
  context_p->last_context_p = saved_context_p->prev_context_p;
  context_p->last_statement = saved_context_p->last_statement;

  context_p->argument_count = saved_context_p->argument_count;
  context_p->register_count = saved_context_p->register_count;
  context_p->literal_count = saved_context_p->literal_count;

  context_p->byte_code = saved_context_p->byte_code;
  context_p->byte_code_size = saved_context_p->byte_code_size;
  context_p->literal_pool.data = saved_context_p->literal_pool_data;
  context_p->scope_stack_p = saved_context_p->scope_stack_p;
  context_p->scope_stack_size = saved_context_p->scope_stack_size;
  context_p->scope_stack_top = saved_context_p->scope_stack_top;
  context_p->scope_stack_reg_top = saved_context_p->scope_stack_reg_top;
#if ENABLED (JERRY_ES2015)
  context_p->scope_stack_global_end = saved_context_p->scope_stack_global_end;
#endif /* ENABLED (JERRY_ES2015) */

#ifndef JERRY_NDEBUG
  context_p->context_stack_depth = saved_context_p->context_stack_depth;
#endif /* !JERRY_NDEBUG */
} /* parser_restore_context */

/**
 * Parse function code
 *
 * @return compiled code
 */
ecma_compiled_code_t *
parser_parse_function (parser_context_t *context_p, /**< context */
                       uint32_t status_flags) /**< extra status flags */
{
  parser_saved_context_t saved_context;
  ecma_compiled_code_t *compiled_code_p;

  JERRY_ASSERT (status_flags & PARSER_IS_FUNCTION);
  parser_save_context (context_p, &saved_context);
  context_p->status_flags |= status_flags;

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes)
  {
#if ENABLED (JERRY_ES2015)
    JERRY_DEBUG_MSG ("\n--- %s parsing start ---\n\n",
                     (context_p->status_flags & PARSER_CLASS_CONSTRUCTOR) ? "Class constructor"
                                                                          : "Function");
#else /* !ENABLED (JERRY_ES2015) */
    JERRY_DEBUG_MSG ("\n--- Function parsing start ---\n\n");
#endif /* ENABLED (JERRY_ES2015) */
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

#if ENABLED (JERRY_DEBUGGER)
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    jerry_debugger_send_parse_function (context_p->token.line, context_p->token.column);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  lexer_next_token (context_p);

  if (context_p->token.type != LEXER_LEFT_PAREN)
  {
    parser_raise_error (context_p, PARSER_ERR_ARGUMENT_LIST_EXPECTED);
  }

  lexer_next_token (context_p);

  parser_parse_function_arguments (context_p, LEXER_RIGHT_PAREN);
  lexer_next_token (context_p);

  if ((context_p->status_flags & PARSER_IS_PROPERTY_GETTER)
      && context_p->argument_count != 0)
  {
    parser_raise_error (context_p, PARSER_ERR_NO_ARGUMENTS_EXPECTED);
  }

  if ((context_p->status_flags & PARSER_IS_PROPERTY_SETTER)
      && context_p->argument_count != 1)
  {
    parser_raise_error (context_p, PARSER_ERR_ONE_ARGUMENT_EXPECTED);
  }

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes
      && (context_p->status_flags & PARSER_HAS_NON_STRICT_ARG))
  {
    JERRY_DEBUG_MSG ("  Note: legacy (non-strict) argument definition\n\n");
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

  if (context_p->token.type != LEXER_LEFT_BRACE)
  {
    parser_raise_error (context_p, PARSER_ERR_LEFT_BRACE_EXPECTED);
  }

  lexer_next_token (context_p);

#if ENABLED (JERRY_ES2015)
  if ((context_p->status_flags & PARSER_CLASS_CONSTRUCTOR_SUPER) == PARSER_CLASS_CONSTRUCTOR_SUPER)
  {
    context_p->status_flags |= PARSER_LEXICAL_ENV_NEEDED;
  }
#endif /* ENABLED (JERRY_ES2015) */
  parser_parse_statements (context_p);
  compiled_code_p = parser_post_processing (context_p);

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes)
  {
#if ENABLED (JERRY_ES2015)
    JERRY_DEBUG_MSG ("\n--- %s parsing end ---\n\n",
                     (context_p->status_flags & PARSER_CLASS_CONSTRUCTOR) ? "Class constructor"
                                                                          : "Function");
#else /* !ENABLED (JERRY_ES2015) */
    JERRY_DEBUG_MSG ("\n--- Function parsing end ---\n\n");
#endif /* ENABLED (JERRY_ES2015) */
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

  parser_restore_context (context_p, &saved_context);

  return compiled_code_p;
} /* parser_parse_function */

#if ENABLED (JERRY_ES2015)

/**
 * Parse arrow function code
 *
 * @return compiled code
 */
ecma_compiled_code_t *
parser_parse_arrow_function (parser_context_t *context_p, /**< context */
                             uint32_t status_flags) /**< extra status flags */
{
  parser_saved_context_t saved_context;
  ecma_compiled_code_t *compiled_code_p;

  JERRY_ASSERT ((status_flags & PARSER_IS_FUNCTION)
                 && (status_flags & PARSER_IS_ARROW_FUNCTION));
  parser_save_context (context_p, &saved_context);
  context_p->status_flags |= status_flags;
#if ENABLED (JERRY_ES2015)
  context_p->status_flags |= saved_context.status_flags & PARSER_CLASS_HAS_SUPER;
#endif /* ENABLED (JERRY_ES2015) */

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes)
  {
    JERRY_DEBUG_MSG ("\n--- Arrow function parsing start ---\n\n");
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

#if ENABLED (JERRY_DEBUGGER)
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    jerry_debugger_send_parse_function (context_p->token.line, context_p->token.column);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  if (status_flags & PARSER_ARROW_PARSE_ARGS)
  {
    parser_parse_function_arguments (context_p, LEXER_RIGHT_PAREN);
    lexer_next_token (context_p);
  }
  else
  {
    parser_parse_function_arguments (context_p, LEXER_ARROW);
  }

  JERRY_ASSERT (context_p->token.type == LEXER_ARROW);

  lexer_next_token (context_p);

  if (context_p->token.type == LEXER_LEFT_BRACE)
  {
    lexer_next_token (context_p);

    context_p->status_flags |= PARSER_IS_CLOSURE;
    parser_parse_statements (context_p);

    /* Unlike normal function, arrow functions consume their close brace. */
    JERRY_ASSERT (context_p->token.type == LEXER_RIGHT_BRACE);
    lexer_next_token (context_p);
  }
  else
  {
    if (context_p->status_flags & PARSER_IS_STRICT
        && context_p->status_flags & PARSER_HAS_NON_STRICT_ARG)
    {
      parser_raise_error (context_p, PARSER_ERR_NON_STRICT_ARG_DEFINITION);
    }

    parser_parse_expression (context_p, PARSE_EXPR_NO_COMMA);

    if (context_p->last_cbc_opcode == CBC_PUSH_LITERAL)
    {
      context_p->last_cbc_opcode = CBC_RETURN_WITH_LITERAL;
    }
    else
    {
      parser_emit_cbc (context_p, CBC_RETURN);
    }
    parser_flush_cbc (context_p);
  }

  compiled_code_p = parser_post_processing (context_p);

#if ENABLED (JERRY_PARSER_DUMP_BYTE_CODE)
  if (context_p->is_show_opcodes)
  {
    JERRY_DEBUG_MSG ("\n--- Arrow function parsing end ---\n\n");
  }
#endif /* ENABLED (JERRY_PARSER_DUMP_BYTE_CODE) */

  parser_restore_context (context_p, &saved_context);

  return compiled_code_p;
} /* parser_parse_arrow_function */

#endif /* ENABLED (JERRY_ES2015) */

/**
 * Raise a parse error.
 */
void
parser_raise_error (parser_context_t *context_p, /**< context */
                    parser_error_t error) /**< error code */
{
  /* Must be compatible with the scanner because
   * the lexer might throws errors during prescanning. */
  parser_saved_context_t *saved_context_p = context_p->last_context_p;

  while (saved_context_p != NULL)
  {
    parser_cbc_stream_free (&saved_context_p->byte_code);

    /* First the current literal pool is freed, and then it is replaced
     * by the literal pool coming from the saved context. Since literals
     * are not used anymore, this is a valid replacement. The last pool
     * is freed by parser_parse_source. */

    parser_free_literals (&context_p->literal_pool);
    context_p->literal_pool.data = saved_context_p->literal_pool_data;

    if (context_p->scope_stack_p != NULL)
    {
      parser_free (context_p->scope_stack_p, context_p->scope_stack_size * sizeof (parser_scope_stack));
    }
    context_p->scope_stack_p = saved_context_p->scope_stack_p;
    context_p->scope_stack_size = saved_context_p->scope_stack_size;

    if (saved_context_p->last_statement.current_p != NULL)
    {
      parser_free_jumps (saved_context_p->last_statement);
    }

    saved_context_p = saved_context_p->prev_context_p;
  }

  context_p->error = error;
  PARSER_THROW (context_p->try_buffer);
  /* Should never been reached. */
  JERRY_ASSERT (0);
} /* parser_raise_error */

#endif /* ENABLED (JERRY_PARSER) */

/**
 * Parse EcmaScript source code
 *
 * Note:
 *      if arg_list_p is not NULL, a function body is parsed
 *      returned value must be freed with ecma_free_value
 *
 * @return true - if success
 *         syntax error - otherwise
 */
ecma_value_t
parser_parse_script (const uint8_t *arg_list_p, /**< function argument list */
                     size_t arg_list_size, /**< size of function argument list */
                     const uint8_t *source_p, /**< source code */
                     size_t source_size, /**< size of the source code */
                     uint32_t parse_opts, /**< ecma_parse_opts_t option bits */
                     ecma_compiled_code_t **bytecode_data_p) /**< [out] JS bytecode */
{
#if ENABLED (JERRY_PARSER)
  parser_error_location_t parser_error;

#if ENABLED (JERRY_DEBUGGER)
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    jerry_debugger_send_string (JERRY_DEBUGGER_SOURCE_CODE,
                                JERRY_DEBUGGER_NO_SUBTYPE,
                                source_p,
                                source_size);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  *bytecode_data_p = parser_parse_source (arg_list_p,
                                          arg_list_size,
                                          source_p,
                                          source_size,
                                          parse_opts,
                                          &parser_error);

  if (!*bytecode_data_p)
  {
#if ENABLED (JERRY_ES2015_MODULE_SYSTEM)
    if (JERRY_CONTEXT (module_top_context_p) != NULL)
    {
      ecma_module_cleanup ();
    }
#endif
#if ENABLED (JERRY_DEBUGGER)
    if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
    {
      jerry_debugger_send_type (JERRY_DEBUGGER_PARSE_ERROR);
    }
#endif /* ENABLED (JERRY_DEBUGGER) */

    if (parser_error.error == PARSER_ERR_OUT_OF_MEMORY)
    {
      /* It is unlikely that memory can be allocated in an out-of-memory
       * situation. However, a simple value can still be thrown. */
      JERRY_CONTEXT (error_value) = ECMA_VALUE_NULL;
      JERRY_CONTEXT (status_flags) |= ECMA_STATUS_EXCEPTION;
      return ECMA_VALUE_ERROR;
    }
#if ENABLED (JERRY_ERROR_MESSAGES)
    const lit_utf8_byte_t *err_bytes_p = (const lit_utf8_byte_t *) parser_error_to_string (parser_error.error);
    lit_utf8_size_t err_bytes_size = lit_zt_utf8_string_size (err_bytes_p);

    ecma_string_t *err_str_p = ecma_new_ecma_string_from_utf8 (err_bytes_p, err_bytes_size);
    ecma_value_t err_str_val = ecma_make_string_value (err_str_p);
    ecma_value_t line_str_val = ecma_make_uint32_value (parser_error.line);
    ecma_value_t col_str_val = ecma_make_uint32_value (parser_error.column);

    ecma_value_t error_value = ecma_raise_standard_error_with_format (ECMA_ERROR_SYNTAX,
                                                                      "% [%:%:%]",
                                                                      err_str_val,
                                                                      JERRY_CONTEXT (resource_name),
                                                                      line_str_val,
                                                                      col_str_val);

    ecma_free_value (col_str_val);
    ecma_free_value (line_str_val);
    ecma_free_value (err_str_val);

    return error_value;
#else /* !ENABLED (JERRY_ERROR_MESSAGES) */
    return ecma_raise_syntax_error ("");
#endif /* ENABLED (JERRY_ERROR_MESSAGES) */
  }

#if ENABLED (JERRY_ES2015_MODULE_SYSTEM)
  if (JERRY_CONTEXT (module_top_context_p) != NULL)
  {
    ecma_value_t ret_value = ecma_module_parse_modules ();

    if (ECMA_IS_VALUE_ERROR (ret_value))
    {
      ecma_bytecode_deref (*bytecode_data_p);
      *bytecode_data_p = NULL;
      ecma_module_cleanup ();

      return ret_value;
    }
  }
#endif /* ENABLED (JERRY_ES2015_MODULE_SYSTEM) */

#if ENABLED (JERRY_DEBUGGER)
  if ((JERRY_CONTEXT (debugger_flags) & (JERRY_DEBUGGER_CONNECTED | JERRY_DEBUGGER_PARSER_WAIT))
      == (JERRY_DEBUGGER_CONNECTED | JERRY_DEBUGGER_PARSER_WAIT))
  {
    JERRY_DEBUGGER_SET_FLAGS (JERRY_DEBUGGER_PARSER_WAIT_MODE);
    jerry_debugger_send_type (JERRY_DEBUGGER_WAITING_AFTER_PARSE);

    while (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_PARSER_WAIT_MODE)
    {
      jerry_debugger_receive (NULL);

      if (!(JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED))
      {
        break;
      }

      jerry_debugger_transport_sleep ();
    }
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  return ECMA_VALUE_TRUE;
#else /* !ENABLED (JERRY_PARSER) */
  JERRY_UNUSED (arg_list_p);
  JERRY_UNUSED (arg_list_size);
  JERRY_UNUSED (source_p);
  JERRY_UNUSED (source_size);
  JERRY_UNUSED (parse_opts);
  JERRY_UNUSED (bytecode_data_p);

  return ecma_raise_syntax_error (ECMA_ERR_MSG ("The parser has been disabled."));
#endif /* ENABLED (JERRY_PARSER) */
} /* parser_parse_script */

/**
 * @}
 * @}
 * @}
 */
