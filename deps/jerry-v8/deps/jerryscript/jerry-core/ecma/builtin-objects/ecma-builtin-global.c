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

#include "ecma-alloc.h"
#include "ecma-builtins.h"
#include "ecma-conversion.h"
#include "ecma-eval.h"
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-try-catch-macro.h"
#include "jrt.h"
#include "lit-char-helpers.h"
#include "lit-magic-strings.h"
#include "lit-strings.h"
#include "vm.h"
#include "jcontext.h"
#include "jrt-libc-includes.h"
#include "jrt-bit-fields.h"

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

/**
 * This object has a custom dispatch function.
 */
#define BUILTIN_CUSTOM_DISPATCH

/**
 * List of built-in routine identifiers.
 */
enum
{
  ECMA_GLOBAL_ROUTINE_START = ECMA_BUILTIN_ID__COUNT - 1,
  /* Note: these 5 routine ids must be in this order */
  ECMA_GLOBAL_IS_NAN,
  ECMA_GLOBAL_IS_FINITE,
  ECMA_GLOBAL_EVAL,
  ECMA_GLOBAL_PARSE_INT,
  ECMA_GLOBAL_PARSE_FLOAT,
  ECMA_GLOBAL_DECODE_URI,
  ECMA_GLOBAL_DECODE_URI_COMPONENT,
  ECMA_GLOBAL_ENCODE_URI,
  ECMA_GLOBAL_ENCODE_URI_COMPONENT,
  ECMA_GLOBAL_ESCAPE,
  ECMA_GLOBAL_UNESCAPE,
};

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-global.inc.h"
#define BUILTIN_UNDERSCORED_ID global
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup global ECMA Global object built-in
 * @{
 */

/**
 * The Global object's 'eval' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.1
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_eval (ecma_value_t x) /**< routine's first argument */
{
  if (JERRY_UNLIKELY (!ecma_is_value_string (x)))
  {
    /* step 1 */
    return ecma_copy_value (x);
  }

  uint32_t parse_opts = vm_is_direct_eval_form_call () ? ECMA_PARSE_DIRECT_EVAL : ECMA_PARSE_NO_OPTS;

  /* See also: ECMA-262 v5, 10.1.1 */
  if (parse_opts && vm_is_strict_mode ())
  {
    JERRY_ASSERT (parse_opts & ECMA_PARSE_DIRECT_EVAL);
    parse_opts |= ECMA_PARSE_STRICT_MODE;
  }

#if ENABLED (JERRY_ESNEXT)
  if (vm_is_direct_eval_form_call ())
  {
    parse_opts |= ECMA_GET_LOCAL_PARSE_OPTS ();
  }
#endif /* ENABLED (JERRY_ESNEXT) */

  /* steps 2 to 8 */
  return ecma_op_eval (ecma_get_string_from_value (x), parse_opts);
} /* ecma_builtin_global_object_eval */

/**
 * The Global object's 'isNaN' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.4
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_is_nan (ecma_number_t arg_num) /**< routine's first argument */
{
  return ecma_make_boolean_value (ecma_number_is_nan (arg_num));
} /* ecma_builtin_global_object_is_nan */

/**
 * The Global object's 'isFinite' routine
 *
 * See also:
 *          ECMA-262 v5, 15.1.2.5
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_is_finite (ecma_number_t arg_num) /**< routine's first argument */
{
  bool is_finite = !(ecma_number_is_nan (arg_num)
                     || ecma_number_is_infinity (arg_num));

  return ecma_make_boolean_value (is_finite);
} /* ecma_builtin_global_object_is_finite */

/**
 * Helper function to check whether a character is in a character bitset.
 *
 * @return true if the character is in the character bitset.
 */
static bool
ecma_builtin_global_object_character_is_in (uint32_t character, /**< character */
                                            const uint8_t *bitset) /**< character set */
{
  JERRY_ASSERT (character < 128);
  return (bitset[character >> 3] & (1u << (character & 0x7))) != 0;
} /* ecma_builtin_global_object_character_is_in */

/**
 * Unescaped URI characters bitset:
 *   One bit for each character between 0 - 127.
 *   Bit is set if the character is in the unescaped URI set.
 */
static const uint8_t unescaped_uri_set[16] =
{
  0x0, 0x0, 0x0, 0x0, 0xda, 0xff, 0xff, 0xaf,
  0xff, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x47
};

/**
 * Unescaped URI component characters bitset:
 *   One bit for each character between 0 - 127.
 *   Bit is set if the character is in the unescaped component URI set.
 */
static const uint8_t unescaped_uri_component_set[16] =
{
  0x0, 0x0, 0x0, 0x0, 0x82, 0x67, 0xff, 0x3,
  0xfe, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x47
};

/**
 * Format is a percent sign followed by two hex digits.
 */
#define URI_ENCODED_BYTE_SIZE (3)

/**
 * The Global object's 'decodeURI' and 'decodeURIComponent' routines
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.1
 *          ECMA-262 v5, 15.1.3.2
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_decode_uri_helper (lit_utf8_byte_t *input_start_p, /**< routine's first argument's
                                                                               *   string buffer */
                                              lit_utf8_size_t input_size, /**< routine's first argument's
                                                                           *   string buffer's size */
                                              const uint8_t *reserved_uri_bitset) /**< reserved characters bitset */
{
  lit_utf8_byte_t *input_char_p = input_start_p;
  lit_utf8_byte_t *input_end_p = input_start_p + input_size;
  ecma_stringbuilder_t builder = ecma_stringbuilder_create ();

  while (input_char_p < input_end_p)
  {
    if (*input_char_p != '%')
    {
      ecma_stringbuilder_append_byte (&builder, *input_char_p++);
      continue;
    }

    uint32_t hex_value = lit_char_hex_lookup (input_char_p + 1, input_end_p, 2);
    if (hex_value == UINT32_MAX)
    {
      ecma_stringbuilder_destroy (&builder);
      return ecma_raise_uri_error (ECMA_ERR_MSG ("Invalid hexadecimal value."));
    }

    ecma_char_t decoded_byte = (ecma_char_t) hex_value;
    input_char_p += URI_ENCODED_BYTE_SIZE;

    if (decoded_byte <= LIT_UTF8_1_BYTE_CODE_POINT_MAX)
    {
      if (ecma_builtin_global_object_character_is_in (decoded_byte, reserved_uri_bitset)
          && !ecma_builtin_global_object_character_is_in (decoded_byte, unescaped_uri_component_set))
      {
        ecma_stringbuilder_append_char (&builder, LIT_CHAR_PERCENT);
        input_char_p -= 2;
      }
      else
      {
        ecma_stringbuilder_append_byte (&builder, (lit_utf8_byte_t) decoded_byte);
      }
    }
    else
    {
      uint32_t bytes_count;

      if ((decoded_byte & LIT_UTF8_2_BYTE_MASK) == LIT_UTF8_2_BYTE_MARKER)
      {
        bytes_count = 2;
      }
      else if ((decoded_byte & LIT_UTF8_3_BYTE_MASK) == LIT_UTF8_3_BYTE_MARKER)
      {
        bytes_count = 3;
      }
      else if ((decoded_byte & LIT_UTF8_4_BYTE_MASK) == LIT_UTF8_4_BYTE_MARKER)
      {
        bytes_count = 4;
      }
      else
      {
        ecma_stringbuilder_destroy (&builder);
        return ecma_raise_uri_error (ECMA_ERR_MSG ("Invalid UTF8 character."));
      }

      lit_utf8_byte_t octets[LIT_UTF8_MAX_BYTES_IN_CODE_POINT];
      octets[0] = (lit_utf8_byte_t) decoded_byte;
      bool is_valid = true;

      for (uint32_t i = 1; i < bytes_count; i++)
      {
        if (input_char_p >= input_end_p || *input_char_p != '%')
        {
          is_valid = false;
          break;
        }
        else
        {
          hex_value = lit_char_hex_lookup (input_char_p + 1, input_end_p, 2);

          if (hex_value == UINT32_MAX || (hex_value & LIT_UTF8_EXTRA_BYTE_MASK) != LIT_UTF8_EXTRA_BYTE_MARKER)
          {
            is_valid = false;
            break;
          }

          input_char_p += URI_ENCODED_BYTE_SIZE;
          octets[i] = (lit_utf8_byte_t) hex_value;
        }
      }

      if (!is_valid
          || !lit_is_valid_utf8_string (octets, bytes_count))
      {
        ecma_stringbuilder_destroy (&builder);
        return ecma_raise_uri_error (ECMA_ERR_MSG ("Invalid UTF8 string."));
      }

      lit_code_point_t cp;
      lit_read_code_point_from_utf8 (octets, bytes_count, &cp);

      if (lit_is_code_point_utf16_high_surrogate (cp)
          || lit_is_code_point_utf16_low_surrogate (cp))
      {
        ecma_stringbuilder_destroy (&builder);
        return ecma_raise_uri_error (ECMA_ERR_MSG ("Invalid UTF8 codepoint."));
      }

      lit_utf8_byte_t result_chars[LIT_CESU8_MAX_BYTES_IN_CODE_POINT];
      lit_utf8_size_t cp_size = lit_code_point_to_cesu8 (cp, result_chars);
      ecma_stringbuilder_append_raw (&builder, result_chars, cp_size);
    }
  }

  return ecma_make_string_value (ecma_stringbuilder_finalize (&builder));
} /* ecma_builtin_global_object_decode_uri_helper */

/**
 * Helper function to encode byte as hexadecimal values.
 */
static void
ecma_builtin_global_object_byte_to_hex (lit_utf8_byte_t *dest_p, /**< destination pointer */
                                        uint32_t byte) /**< value */
{
  JERRY_ASSERT (byte < 256);

  dest_p[0] = LIT_CHAR_PERCENT;
  ecma_char_t hex_digit = (ecma_char_t) (byte >> 4);
  dest_p[1] = (lit_utf8_byte_t) ((hex_digit > 9) ? (hex_digit + ('A' - 10)) : (hex_digit + '0'));
  hex_digit = (lit_utf8_byte_t) (byte & 0xf);
  dest_p[2] = (lit_utf8_byte_t) ((hex_digit > 9) ? (hex_digit + ('A' - 10)) : (hex_digit + '0'));
} /* ecma_builtin_global_object_byte_to_hex */

/**
 * The Global object's 'encodeURI' and 'encodeURIComponent' routines
 *
 * See also:
 *          ECMA-262 v5, 15.1.3.3
 *          ECMA-262 v5, 15.1.3.4
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_encode_uri_helper (lit_utf8_byte_t *input_start_p, /**< routine's first argument's
                                                                               *   string buffer */
                                              lit_utf8_size_t input_size, /**< routine's first argument's
                                                                           *   string buffer's size */
                                              const uint8_t *unescaped_uri_bitset_p) /**< unescaped bitset */
{
  lit_utf8_byte_t *input_char_p = input_start_p;
  const lit_utf8_byte_t *input_end_p = input_start_p + input_size;
  ecma_char_t ch;
  ecma_stringbuilder_t builder = ecma_stringbuilder_create ();
  lit_utf8_byte_t octets[LIT_UTF8_MAX_BYTES_IN_CODE_POINT];
  memset (octets, LIT_BYTE_NULL, LIT_UTF8_MAX_BYTES_IN_CODE_POINT);

  while (input_char_p < input_end_p)
  {
    input_char_p += lit_read_code_unit_from_utf8 (input_char_p, &ch);

    if (lit_is_code_point_utf16_low_surrogate (ch))
    {
      ecma_stringbuilder_destroy (&builder);
      return ecma_raise_uri_error (ECMA_ERR_MSG ("Unicode surrogate pair missing."));
    }

    lit_code_point_t cp = ch;

    if (lit_is_code_point_utf16_high_surrogate (ch))
    {
      if (input_char_p == input_end_p)
      {
        ecma_stringbuilder_destroy (&builder);
        return ecma_raise_uri_error (ECMA_ERR_MSG ("Unicode surrogate pair missing."));
      }

      ecma_char_t next_ch;
      lit_utf8_size_t read_size = lit_read_code_unit_from_utf8 (input_char_p, &next_ch);

      if (lit_is_code_point_utf16_low_surrogate (next_ch))
      {
        cp = lit_convert_surrogate_pair_to_code_point (ch, next_ch);
        input_char_p += read_size;
      }
      else
      {
        ecma_stringbuilder_destroy (&builder);
        return ecma_raise_uri_error (ECMA_ERR_MSG ("Unicode surrogate pair missing."));
      }
    }

    lit_utf8_size_t utf_size = lit_code_point_to_utf8 (cp, octets);
    lit_utf8_byte_t result_chars[URI_ENCODED_BYTE_SIZE];

    if (utf_size == 1)
    {
      if (ecma_builtin_global_object_character_is_in (octets[0], unescaped_uri_bitset_p))
      {
        ecma_stringbuilder_append_byte (&builder, octets[0]);
      }
      else
      {
        ecma_builtin_global_object_byte_to_hex (result_chars, octets[0]);
        ecma_stringbuilder_append_raw (&builder, result_chars, URI_ENCODED_BYTE_SIZE);
      }
    }
    else
    {
      for (uint32_t i = 0; i < utf_size; i++)
      {
        JERRY_ASSERT (utf_size <= LIT_UTF8_MAX_BYTES_IN_CODE_POINT);
        ecma_builtin_global_object_byte_to_hex (result_chars, octets[i]);
        ecma_stringbuilder_append_raw (&builder, result_chars, URI_ENCODED_BYTE_SIZE);
      }
    }
  }

  return ecma_make_string_value (ecma_stringbuilder_finalize (&builder));
} /* ecma_builtin_global_object_encode_uri_helper */

#if ENABLED (JERRY_BUILTIN_ANNEXB)

/**
 * Maximum value of a byte.
 */
#define ECMA_ESCAPE_MAXIMUM_BYTE_VALUE (255)

/**
 * Format is a percent sign followed by lowercase u and four hex digits.
 */
#define ECMA_ESCAPE_ENCODED_UNICODE_CHARACTER_SIZE (6)

/**
 * Escape characters bitset:
 *   One bit for each character between 0 - 127.
 *   Bit is set if the character does not need to be converted to %xx form.
 *   These characters are: a-z A-Z 0-9 @ * _ + - . /
 */
static const uint8_t ecma_escape_set[16] =
{
  0x0, 0x0, 0x0, 0x0, 0x0, 0xec, 0xff, 0x3,
  0xff, 0xff, 0xff, 0x87, 0xfe, 0xff, 0xff, 0x7
};

/**
 * The Global object's 'escape' routine
 *
 * See also:
 *          ECMA-262 v5, B.2.1
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_escape (lit_utf8_byte_t *input_start_p, /**< routine's first argument's
                                                                    *   string buffer */
                                   lit_utf8_size_t input_size) /**< routine's first argument's
                                                                *   string buffer's size */
{
  const lit_utf8_byte_t *input_curr_p = input_start_p;
  const lit_utf8_byte_t *input_end_p = input_start_p + input_size;
  ecma_stringbuilder_t builder = ecma_stringbuilder_create ();
  lit_utf8_byte_t result_chars[URI_ENCODED_BYTE_SIZE];

  while (input_curr_p < input_end_p)
  {
    ecma_char_t chr = lit_cesu8_read_next (&input_curr_p);

    if (chr <= LIT_UTF8_1_BYTE_CODE_POINT_MAX)
    {
      if (ecma_builtin_global_object_character_is_in ((uint32_t) chr, ecma_escape_set))
      {
        ecma_stringbuilder_append_char (&builder, chr);
      }
      else
      {
        ecma_builtin_global_object_byte_to_hex (result_chars, chr);
        ecma_stringbuilder_append_raw (&builder, result_chars, URI_ENCODED_BYTE_SIZE);
      }
    }
    else if (chr > ECMA_ESCAPE_MAXIMUM_BYTE_VALUE)
    {
      ecma_stringbuilder_append_char (&builder, LIT_CHAR_PERCENT);
      ecma_stringbuilder_append_char (&builder, LIT_CHAR_LOWERCASE_U);

      ecma_builtin_global_object_byte_to_hex (result_chars, (chr >> JERRY_BITSINBYTE));
      ecma_stringbuilder_append_raw (&builder, result_chars + 1, 2);

      ecma_builtin_global_object_byte_to_hex (result_chars, (chr & 0xff));
      ecma_stringbuilder_append_raw (&builder, result_chars + 1, 2);
    }
    else
    {
      ecma_builtin_global_object_byte_to_hex (result_chars, chr);
      ecma_stringbuilder_append_raw (&builder, result_chars, URI_ENCODED_BYTE_SIZE);
    }
  }

  return ecma_make_string_value (ecma_stringbuilder_finalize (&builder));
} /* ecma_builtin_global_object_escape */

/**
 * The Global object's 'unescape' routine
 *
 * See also:
 *          ECMA-262 v5, B.2.2
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_global_object_unescape (lit_utf8_byte_t *input_start_p, /**< routine's first argument's
                                                                      *   string buffer */
                                     lit_utf8_size_t input_size) /**< routine's first argument's
                                                                  *   string buffer's size */
{
  const lit_utf8_byte_t *input_curr_p = input_start_p;
  const lit_utf8_byte_t *input_end_p = input_start_p + input_size;
  /* 4. */
  /* The length of input string is always greater than output string
   * so we re-use the input string buffer.
   * The %xx is three byte long, and the maximum encoded value is 0xff,
   * which maximum encoded length is two byte. Similar to this, the maximum
   * encoded length of %uxxxx is four byte. */
  lit_utf8_byte_t *output_char_p = input_start_p;

  /* The state of parsing that tells us where we are in an escape pattern.
   * 0    we are outside of pattern,
   * 1    found '%', start of pattern,
   * 2    found first hex digit of '%xy' pattern
   * 3    found valid '%xy' pattern
   * 4    found 'u', start of '%uwxyz' pattern
   * 5-7  found hex digits of '%uwxyz' pattern
   * 8    found valid '%uwxyz' pattern
   */
  uint8_t status = 0;
  ecma_char_t hex_digits = 0;
  /* 5. */
  while (input_curr_p < input_end_p)
  {
    /* 6. */
    ecma_char_t chr = lit_cesu8_read_next (&input_curr_p);

    /* 7-8. */
    if (status == 0 && chr == LIT_CHAR_PERCENT)
    {
      /* Found '%' char, start of escape sequence. */
      status = 1;
    }
    /* 9-10. */
    else if (status == 1 && chr == LIT_CHAR_LOWERCASE_U)
    {
      /* Found 'u' char after '%'. */
      status = 4;
    }
    else if (status > 0 && lit_char_is_hex_digit (chr))
    {
      /* Found hexadecimal digit in escape sequence. */
      hex_digits = (ecma_char_t) (hex_digits * 16 + (ecma_char_t) lit_char_hex_to_int (chr));
      status++;
    }
    else
    {
      /* Previously found hexadecimal digit in escape sequence but it's not valid '%xy' pattern
       * so essentially it was only a simple character. */
      status = 0;
    }

    /* 11-17. Found valid '%uwxyz' or '%xy' escape. */
    if (status == 8 || status == 3)
    {
      output_char_p -= (status == 3) ? 2 : 5;
      status = 0;
      chr = hex_digits;
      hex_digits = 0;
    }

    /* Copying character. */
    lit_utf8_size_t lit_size = lit_code_unit_to_utf8 (chr, output_char_p);
    output_char_p += lit_size;
    JERRY_ASSERT (output_char_p <= input_curr_p);
  }

  lit_utf8_size_t output_length = (lit_utf8_size_t) (output_char_p - input_start_p);
  ecma_string_t *output_string_p = ecma_new_ecma_string_from_utf8 (input_start_p, output_length);
  return ecma_make_string_value (output_string_p);
} /* ecma_builtin_global_object_unescape */

#endif /* ENABLED (JERRY_BUILTIN_ANNEXB) */

/**
 * Dispatcher of the built-in's routines
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
ecma_value_t
ecma_builtin_global_dispatch_routine (uint16_t builtin_routine_id, /**< built-in wide routine identifier */
                                      ecma_value_t this_arg, /**< 'this' argument value */
                                      const ecma_value_t arguments_list_p[], /**< list of arguments
                                                                              *   passed to routine */
                                      uint32_t arguments_number) /**< length of arguments' list */
{
  JERRY_UNUSED (this_arg);
  JERRY_UNUSED (arguments_list_p);
  JERRY_UNUSED (arguments_number);

  ecma_value_t routine_arg_1 = arguments_list_p[0];

  if (builtin_routine_id == ECMA_GLOBAL_EVAL)
  {
    return ecma_builtin_global_object_eval (routine_arg_1);
  }

  if (builtin_routine_id <= ECMA_GLOBAL_IS_FINITE)
  {
    ecma_number_t arg_num;

    routine_arg_1 = ecma_get_number (routine_arg_1, &arg_num);

    if (!ecma_is_value_empty (routine_arg_1))
    {
      return routine_arg_1;
    }

    if (builtin_routine_id == ECMA_GLOBAL_IS_NAN)
    {
      return ecma_builtin_global_object_is_nan (arg_num);
    }

    JERRY_ASSERT (builtin_routine_id == ECMA_GLOBAL_IS_FINITE);

    return ecma_builtin_global_object_is_finite (arg_num);
  }

  ecma_string_t *str_p = ecma_op_to_string (routine_arg_1);

  if (JERRY_UNLIKELY (str_p == NULL))
  {
    return ECMA_VALUE_ERROR;
  }

  ecma_value_t ret_value;

  if (builtin_routine_id <= ECMA_GLOBAL_PARSE_FLOAT)
  {
    ECMA_STRING_TO_UTF8_STRING (str_p, string_buff, string_buff_size);

    if (builtin_routine_id == ECMA_GLOBAL_PARSE_INT)
    {
      ret_value = ecma_number_parse_int (string_buff,
                                         string_buff_size,
                                         arguments_list_p[1]);
    }
    else
    {
      JERRY_ASSERT (builtin_routine_id == ECMA_GLOBAL_PARSE_FLOAT);
      ret_value = ecma_number_parse_float (string_buff,
                                           string_buff_size);
    }

    ECMA_FINALIZE_UTF8_STRING (string_buff, string_buff_size);
    ecma_deref_ecma_string (str_p);
    return ret_value;
  }

  lit_utf8_size_t input_size = ecma_string_get_size (str_p);

  JMEM_DEFINE_LOCAL_ARRAY (input_start_p,
                           input_size + 1,
                           lit_utf8_byte_t);

  ecma_string_to_utf8_bytes (str_p, input_start_p, input_size);

  input_start_p[input_size] = LIT_BYTE_NULL;

  switch (builtin_routine_id)
  {
#if ENABLED (JERRY_BUILTIN_ANNEXB)
    case ECMA_GLOBAL_ESCAPE:
    {
      ret_value = ecma_builtin_global_object_escape (input_start_p, input_size);
      break;
    }
    case ECMA_GLOBAL_UNESCAPE:
    {
      ret_value = ecma_builtin_global_object_unescape (input_start_p, input_size);
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_ANNEXB) */
    case ECMA_GLOBAL_DECODE_URI:
    case ECMA_GLOBAL_DECODE_URI_COMPONENT:
    {
      const uint8_t *uri_set = (builtin_routine_id == ECMA_GLOBAL_DECODE_URI ? unescaped_uri_set
                                                                             : unescaped_uri_component_set);

      ret_value = ecma_builtin_global_object_decode_uri_helper (input_start_p, input_size, uri_set);
      break;
    }
    default:
    {
      JERRY_ASSERT (builtin_routine_id == ECMA_GLOBAL_ENCODE_URI
                    || builtin_routine_id == ECMA_GLOBAL_ENCODE_URI_COMPONENT);

      const uint8_t *uri_set = (builtin_routine_id == ECMA_GLOBAL_ENCODE_URI ? unescaped_uri_set
                                                                             : unescaped_uri_component_set);

      ret_value = ecma_builtin_global_object_encode_uri_helper (input_start_p, input_size, uri_set);
      break;
    }
  }

  JMEM_FINALIZE_LOCAL_ARRAY (input_start_p);

  ecma_deref_ecma_string (str_p);
  return ret_value;
} /* ecma_builtin_global_dispatch_routine */

/**
 * @}
 * @}
 * @}
 */
