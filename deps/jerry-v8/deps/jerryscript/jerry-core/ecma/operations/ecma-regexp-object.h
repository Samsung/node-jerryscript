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

#ifndef ECMA_REGEXP_OBJECT_H
#define ECMA_REGEXP_OBJECT_H

#if ENABLED (JERRY_BUILTIN_REGEXP)

#include "ecma-globals.h"
#include "re-compiler.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmaregexpobject ECMA RegExp object related routines
 * @{
 */

/**
 * RegExp flags
 * Note:
 *      This enum has to be kept in sync with jerry_regexp_flags_t.
 */
typedef enum
{
  RE_FLAG_EMPTY = 0u,              /* Empty RegExp flags */
  RE_FLAG_GLOBAL = (1u << 1),      /**< ECMA-262 v5, 15.10.7.2 */
  RE_FLAG_IGNORE_CASE = (1u << 2), /**< ECMA-262 v5, 15.10.7.3 */
  RE_FLAG_MULTILINE = (1u << 3),   /**< ECMA-262 v5, 15.10.7.4 */
  RE_FLAG_STICKY = (1u << 4),      /**< ECMA-262 v6, 21.2.5.12 */
  RE_FLAG_UNICODE = (1u << 5)      /**< ECMA-262 v6, 21.2.5.15 */
} ecma_regexp_flags_t;

/**
 * Structure for storing capturing group results
 */
typedef struct
{
  const lit_utf8_byte_t *begin_p; /**< substring start pointer */
  const lit_utf8_byte_t *end_p;   /**< substring end pointer */
} ecma_regexp_capture_t;

/**
 * Check if an ecma_regexp_capture_t contains a defined capture
 */
#define ECMA_RE_IS_CAPTURE_DEFINED(c) ((c)->begin_p != NULL && (c)->end_p >= (c)->begin_p)

ecma_value_t
ecma_regexp_get_capture_value (const ecma_regexp_capture_t *const capture_p);

/**
 * Structure for storing non-capturing group results
 */
typedef struct
{
  const lit_utf8_byte_t *str_p; /**< string pointer */
} ecma_regexp_non_capture_t;

#if (JERRY_STACK_LIMIT != 0)
/**
 * Value used ase result when stack limit is reached
 */
#define ECMA_RE_OUT_OF_STACK ((const lit_utf8_byte_t *) UINTPTR_MAX)

/**
 * Checks if the stack limit has been reached during regexp matching
 */
#define ECMA_RE_STACK_LIMIT_REACHED(p) (JERRY_UNLIKELY (p == ECMA_RE_OUT_OF_STACK))
#else /* JERRY_STACK_LIMIT == 0 */
#define ECMA_RE_STACK_LIMIT_REACHED(p) (false)
#endif /* JERRY_STACK_LIMIT != 0 */

/**
 * RegExp executor context
 */
typedef struct
{
  const lit_utf8_byte_t *input_end_p;          /**< end of input string */
  const lit_utf8_byte_t *input_start_p;        /**< start of input string */
  uint32_t captures_count;                     /**< number of capture groups */
  ecma_regexp_capture_t *captures_p;           /**< capturing groups */
  uint32_t non_captures_count;                 /**< number of non-capture groups */
  ecma_regexp_non_capture_t *non_captures_p;   /**< non-capturing groups */
  uint32_t *iterations_p;                      /**< number of iterations */
  uint16_t flags;                              /**< RegExp flags */
} ecma_regexp_ctx_t;

ecma_value_t ecma_op_create_regexp_object_from_bytecode (re_compiled_code_t *bytecode_p);
ecma_value_t ecma_op_create_regexp_object (ecma_string_t *pattern_p, uint16_t flags);
ecma_value_t ecma_regexp_exec_helper (ecma_object_t *regexp_object_p,
                                      ecma_string_t *input_string_p);
ecma_string_t *ecma_regexp_read_pattern_str_helper (ecma_value_t pattern_arg);
lit_code_point_t ecma_regexp_canonicalize (lit_code_point_t ch, bool is_ignorecase);
lit_code_point_t ecma_regexp_canonicalize_char (lit_code_point_t ch);
ecma_value_t ecma_regexp_parse_flags (ecma_string_t *flags_str_p, uint16_t *flags_p);
void ecma_regexp_initialize_props (ecma_object_t *re_obj_p, ecma_string_t *source_p, uint16_t flags);

ecma_value_t ecma_regexp_replace_helper (ecma_value_t this_arg, ecma_value_t string_arg, ecma_value_t replace_arg);
ecma_value_t ecma_regexp_search_helper (ecma_value_t regexp_arg, ecma_value_t string_arg);
ecma_value_t ecma_regexp_split_helper (ecma_value_t this_arg, ecma_value_t string_arg, ecma_value_t limit_arg);
ecma_value_t ecma_regexp_match_helper (ecma_value_t this_arg, ecma_value_t string_arg);

ecma_value_t ecma_op_regexp_exec (ecma_value_t this_arg, ecma_string_t *str_p);
/**
 * @}
 * @}
 */

#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
#endif /* !ECMA_REGEXP_OBJECT_H */
