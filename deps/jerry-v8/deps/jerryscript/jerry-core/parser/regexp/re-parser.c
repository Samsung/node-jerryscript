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

#include "ecma-exceptions.h"
#include "ecma-globals.h"
#include "ecma-try-catch-macro.h"
#include "jcontext.h"
#include "jrt-libc-includes.h"
#include "lit-char-helpers.h"
#include "re-compiler.h"
#include "re-parser.h"

#if ENABLED (JERRY_BUILTIN_REGEXP)

/** \addtogroup parser Parser
 * @{
 *
 * \addtogroup regexparser Regular expression
 * @{
 *
 * \addtogroup regexparser_parser Parser
 * @{
 */

/**
 * Lookup a character in the input string.
 *
 * @return true - if lookup number of characters ahead are hex digits
 *         false - otherwise
 */
bool
re_hex_lookup (re_parser_ctx_t *parser_ctx_p, /**< RegExp parser context */
               uint32_t lookup) /**< size of lookup */
{
  const lit_utf8_byte_t *curr_p = parser_ctx_p->input_curr_p;

  if (JERRY_UNLIKELY (curr_p + lookup > parser_ctx_p->input_end_p))
  {
    return false;
  }

  for (uint32_t i = 0; i < lookup; i++)
  {
    if (!lit_char_is_hex_digit (*curr_p++))
    {
      return false;
    }
  }

  return true;
} /* re_hex_lookup */

/**
 * Consume non greedy (question mark) character if present.
 *
 * @return true - if non-greedy character found
 *         false - otherwise
 */
static inline bool JERRY_ATTR_ALWAYS_INLINE
re_parse_non_greedy_char (re_parser_ctx_t *parser_ctx_p) /**< RegExp parser context */
{
  if (parser_ctx_p->input_curr_p < parser_ctx_p->input_end_p
      && *parser_ctx_p->input_curr_p == LIT_CHAR_QUESTION)
  {
    parser_ctx_p->input_curr_p++;
    return true;
  }

  return false;
} /* re_parse_non_greedy_char */

/**
 * Parse a max 3 digit long octal number from input string iterator.
 *
 * @return uint32_t - parsed octal number
 */
uint32_t
re_parse_octal (re_parser_ctx_t *parser_ctx_p) /**< RegExp parser context */
{
  uint32_t number = 0;
  for (int index = 0;
       index < 3
       && parser_ctx_p->input_curr_p < parser_ctx_p->input_end_p
       && lit_char_is_octal_digit (*parser_ctx_p->input_curr_p);
       index++)
  {
    number = number * 8 + lit_char_hex_to_int (*parser_ctx_p->input_curr_p++);
  }

  return number;
} /* re_parse_octal */

/**
 * Parse RegExp iterators
 *
 * @return empty ecma value - if parsed successfully
 *         error ecma value - otherwise
 *
 *         Returned value must be freed with ecma_free_value
 */
ecma_value_t
re_parse_iterator (re_parser_ctx_t *parser_ctx_p, /**< RegExp parser context */
                   re_token_t *re_token_p) /**< [out] output token */
{
  ecma_value_t ret_value = ECMA_VALUE_EMPTY;

  re_token_p->qmin = 1;
  re_token_p->qmax = 1;
  re_token_p->greedy = true;

  if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
  {
    return ret_value;
  }

  ecma_char_t ch = *parser_ctx_p->input_curr_p;

  switch (ch)
  {
    case LIT_CHAR_QUESTION:
    {
      parser_ctx_p->input_curr_p++;
      re_token_p->qmin = 0;
      re_token_p->qmax = 1;
      re_token_p->greedy = !re_parse_non_greedy_char (parser_ctx_p);
      break;
    }
    case LIT_CHAR_ASTERISK:
    {
      parser_ctx_p->input_curr_p++;
      re_token_p->qmin = 0;
      re_token_p->qmax = RE_ITERATOR_INFINITE;
      re_token_p->greedy = !re_parse_non_greedy_char (parser_ctx_p);
      break;
    }
    case LIT_CHAR_PLUS:
    {
      parser_ctx_p->input_curr_p++;
      re_token_p->qmin = 1;
      re_token_p->qmax = RE_ITERATOR_INFINITE;
      re_token_p->greedy = !re_parse_non_greedy_char (parser_ctx_p);
      break;
    }
    case LIT_CHAR_LEFT_BRACE:
    {
      parser_ctx_p->input_curr_p++;
      uint32_t qmin = 0;
      uint32_t qmax = RE_ITERATOR_INFINITE;
      uint32_t digits = 0;

      while (true)
      {
        if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("invalid quantifier"));
        }

        ch = *parser_ctx_p->input_curr_p++;

        if (lit_char_is_decimal_digit (ch))
        {
          if (digits >= ECMA_NUMBER_MAX_DIGITS)
          {
            return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp quantifier error: too many digits."));
          }
          digits++;
          qmin = qmin * 10 + lit_char_hex_to_int (ch);
        }
        else if (ch == LIT_CHAR_COMMA)
        {
          if (qmax != RE_ITERATOR_INFINITE)
          {
            return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp quantifier error: double comma."));
          }

          if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
          {
            return ecma_raise_syntax_error (ECMA_ERR_MSG ("invalid quantifier"));
          }

          if (*parser_ctx_p->input_curr_p == LIT_CHAR_RIGHT_BRACE)
          {
            if (digits == 0)
            {
              return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp quantifier error: missing digits."));
            }

            parser_ctx_p->input_curr_p++;
            re_token_p->qmin = qmin;
            re_token_p->qmax = RE_ITERATOR_INFINITE;
            break;
          }
          qmax = qmin;
          qmin = 0;
          digits = 0;
        }
        else if (ch == LIT_CHAR_RIGHT_BRACE)
        {
          if (digits == 0)
          {
            return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp quantifier error: missing digits."));
          }

          if (qmax != RE_ITERATOR_INFINITE)
          {
            re_token_p->qmin = qmax;
          }
          else
          {
            re_token_p->qmin = qmin;
          }

          re_token_p->qmax = qmin;

          break;
        }
        else
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp quantifier error: unknown char."));
        }
      }

      re_token_p->greedy = !re_parse_non_greedy_char (parser_ctx_p);
      break;
    }
    default:
    {
      break;
    }
  }

  JERRY_ASSERT (ecma_is_value_empty (ret_value));

  return ret_value;
} /* re_parse_iterator */

/**
 * Count the number of groups in pattern
 */
static void
re_count_num_of_groups (re_parser_ctx_t *parser_ctx_p) /**< RegExp parser context */
{
  int char_class_in = 0;
  parser_ctx_p->groups_count = 0;
  const lit_utf8_byte_t *curr_p = parser_ctx_p->input_start_p;

  while (curr_p < parser_ctx_p->input_end_p)
  {
    switch (*curr_p++)
    {
      case LIT_CHAR_BACKSLASH:
      {
        if (curr_p < parser_ctx_p->input_end_p)
        {
          lit_utf8_incr (&curr_p);
        }
        break;
      }
      case LIT_CHAR_LEFT_SQUARE:
      {
        char_class_in++;
        break;
      }
      case LIT_CHAR_RIGHT_SQUARE:
      {
        if (char_class_in)
        {
          char_class_in--;
        }
        break;
      }
      case LIT_CHAR_LEFT_PAREN:
      {
        if (curr_p < parser_ctx_p->input_end_p
            && *curr_p != LIT_CHAR_QUESTION
            && !char_class_in)
        {
          parser_ctx_p->groups_count++;
        }
        break;
      }
    }
  }
} /* re_count_num_of_groups */

/**
 * Read the input pattern and parse the next token for the RegExp compiler
 *
 * @return empty ecma value - if parsed successfully
 *         error ecma value - otherwise
 *
 *         Returned value must be freed with ecma_free_value
 */
ecma_value_t
re_parse_next_token (re_parser_ctx_t *parser_ctx_p, /**< RegExp parser context */
                     re_token_t *out_token_p) /**< [out] output token */
{
  ecma_value_t ret_value = ECMA_VALUE_EMPTY;

  if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
  {
    out_token_p->type = RE_TOK_EOF;
    return ret_value;
  }

  ecma_char_t ch = lit_cesu8_read_next (&parser_ctx_p->input_curr_p);

  switch (ch)
  {
    case LIT_CHAR_VLINE:
    {
      out_token_p->type = RE_TOK_ALTERNATIVE;
      break;
    }
    case LIT_CHAR_CIRCUMFLEX:
    {
      out_token_p->type = RE_TOK_ASSERT_START;
      break;
    }
    case LIT_CHAR_DOLLAR_SIGN:
    {
      out_token_p->type = RE_TOK_ASSERT_END;
      break;
    }
    case LIT_CHAR_DOT:
    {
      out_token_p->type = RE_TOK_PERIOD;
      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);
      break;
    }
    case LIT_CHAR_BACKSLASH:
    {
      if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
      {
        return ecma_raise_syntax_error (ECMA_ERR_MSG ("invalid regular experssion"));
      }

      out_token_p->type = RE_TOK_CHAR;
      ch = lit_cesu8_read_next (&parser_ctx_p->input_curr_p);

      if (ch == LIT_CHAR_LOWERCASE_B)
      {
        out_token_p->type = RE_TOK_ASSERT_WORD_BOUNDARY;
      }
      else if (ch == LIT_CHAR_UPPERCASE_B)
      {
        out_token_p->type = RE_TOK_ASSERT_NOT_WORD_BOUNDARY;
      }
      else if (ch == LIT_CHAR_LOWERCASE_F)
      {
        out_token_p->value = LIT_CHAR_FF;
      }
      else if (ch == LIT_CHAR_LOWERCASE_N)
      {
        out_token_p->value = LIT_CHAR_LF;
      }
      else if (ch == LIT_CHAR_LOWERCASE_T)
      {
        out_token_p->value = LIT_CHAR_TAB;
      }
      else if (ch == LIT_CHAR_LOWERCASE_R)
      {
        out_token_p->value = LIT_CHAR_CR;
      }
      else if (ch == LIT_CHAR_LOWERCASE_V)
      {
        out_token_p->value = LIT_CHAR_VTAB;
      }
      else if (ch == LIT_CHAR_LOWERCASE_C)
      {
        if (parser_ctx_p->input_curr_p < parser_ctx_p->input_end_p)
        {
          ch = *parser_ctx_p->input_curr_p;

          if ((ch >= LIT_CHAR_ASCII_UPPERCASE_LETTERS_BEGIN && ch <= LIT_CHAR_ASCII_UPPERCASE_LETTERS_END)
              || (ch >= LIT_CHAR_ASCII_LOWERCASE_LETTERS_BEGIN && ch <= LIT_CHAR_ASCII_LOWERCASE_LETTERS_END))
          {
            out_token_p->value = (ch % 32);
            parser_ctx_p->input_curr_p++;
          }
          else
          {
            out_token_p->value = LIT_CHAR_BACKSLASH;
            parser_ctx_p->input_curr_p--;
          }
        }
        else
        {
          out_token_p->value = LIT_CHAR_BACKSLASH;
          parser_ctx_p->input_curr_p--;
        }
      }
      else if (ch == LIT_CHAR_LOWERCASE_X
               && re_hex_lookup (parser_ctx_p, 2))
      {
        ecma_char_t code_unit;

        if (!lit_read_code_unit_from_hex (parser_ctx_p->input_curr_p, 2, &code_unit))
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("decode error"));
        }

        parser_ctx_p->input_curr_p += 2;
        out_token_p->value = code_unit;
      }
      else if (ch == LIT_CHAR_LOWERCASE_U
               && re_hex_lookup (parser_ctx_p, 4))
      {
        ecma_char_t code_unit;

        if (!lit_read_code_unit_from_hex (parser_ctx_p->input_curr_p, 4, &code_unit))
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("decode error"));
        }

        parser_ctx_p->input_curr_p += 4;
        out_token_p->value = code_unit;
      }
      else if (ch == LIT_CHAR_LOWERCASE_D)
      {
        out_token_p->type = RE_TOK_DIGIT;
        break;
      }
      else if (ch == LIT_CHAR_UPPERCASE_D)
      {
        out_token_p->type = RE_TOK_NOT_DIGIT;
        break;
      }
      else if (ch == LIT_CHAR_LOWERCASE_S)
      {
        out_token_p->type = RE_TOK_WHITE;
        break;
      }
      else if (ch == LIT_CHAR_UPPERCASE_S)
      {
        out_token_p->type = RE_TOK_NOT_WHITE;
        break;
      }
      else if (ch == LIT_CHAR_LOWERCASE_W)
      {
        out_token_p->type = RE_TOK_WORD_CHAR;
        break;
      }
      else if (ch == LIT_CHAR_UPPERCASE_W)
      {
        out_token_p->type = RE_TOK_NOT_WORD_CHAR;
        break;
      }
      else if (lit_char_is_decimal_digit (ch))
      {
        if (ch == LIT_CHAR_0)
        {
          if (parser_ctx_p->input_curr_p < parser_ctx_p->input_end_p
              && lit_char_is_decimal_digit (*parser_ctx_p->input_curr_p))
          {
            return ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp escape pattern error."));
          }

          out_token_p->value = LIT_UNICODE_CODE_POINT_NULL;
        }
        else
        {
          if (parser_ctx_p->groups_count == -1)
          {
            re_count_num_of_groups (parser_ctx_p);
          }

          if (parser_ctx_p->groups_count)
          {
            parser_ctx_p->input_curr_p--;
            uint32_t number = 0;
            int index = 0;

            do
            {
              if (index >= RE_MAX_RE_DECESC_DIGITS)
              {
                ret_value = ecma_raise_syntax_error (ECMA_ERR_MSG ("RegExp escape error: decimal escape too long."));
                return ret_value;
              }
              if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
              {
                break;
              }

              ecma_char_t digit = *parser_ctx_p->input_curr_p++;

              if (!lit_char_is_decimal_digit (digit))
              {
                parser_ctx_p->input_curr_p--;
                break;
              }
              number = number * 10 + lit_char_hex_to_int (digit);
              index++;
            }
            while (true);

            if ((int) number <= parser_ctx_p->groups_count)
            {
              out_token_p->type = RE_TOK_BACKREFERENCE;
            }
            else
            /* Invalid backreference, fallback to octal */
            {
              /* Rewind to start of number. */
              parser_ctx_p->input_curr_p -= index;

              /* Try to reparse as octal. */
              ecma_char_t digit = *parser_ctx_p->input_curr_p;

              if (!lit_char_is_octal_digit (digit))
              {
                /* Not octal, keep digit character value. */
                number = digit;
                parser_ctx_p->input_curr_p++;
              }
              else
              {
                number = re_parse_octal (parser_ctx_p);
              }
            }
            out_token_p->value = number;
          }
          else
          /* Invalid backreference, fallback to octal if possible */
          {
            if (!lit_char_is_octal_digit (ch))
            {
              /* Not octal, keep character value. */
              out_token_p->value = ch;
            }
            else
            {
              parser_ctx_p->input_curr_p--;
              out_token_p->value = re_parse_octal (parser_ctx_p);
            }
          }
        }
      }
      else
      {
        out_token_p->value = ch;
      }

      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);
      break;
    }
    case LIT_CHAR_LEFT_PAREN:
    {
      if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
      {
        return ecma_raise_syntax_error (ECMA_ERR_MSG ("Unterminated group"));
      }

      if (*parser_ctx_p->input_curr_p == LIT_CHAR_QUESTION)
      {
        parser_ctx_p->input_curr_p++;
        if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("Invalid group"));
        }

        ch = *parser_ctx_p->input_curr_p++;

        if (ch == LIT_CHAR_EQUALS)
        {
          /* (?= */
          out_token_p->type = RE_TOK_ASSERT_START_POS_LOOKAHEAD;
        }
        else if (ch == LIT_CHAR_EXCLAMATION)
        {
          /* (?! */
          out_token_p->type = RE_TOK_ASSERT_START_NEG_LOOKAHEAD;
        }
        else if (ch == LIT_CHAR_COLON)
        {
          /* (?: */
          out_token_p->type = RE_TOK_START_NON_CAPTURE_GROUP;
        }
        else
        {
          return ecma_raise_syntax_error (ECMA_ERR_MSG ("Invalid group"));
        }
      }
      else
      {
        /* ( */
        out_token_p->type = RE_TOK_START_CAPTURE_GROUP;
      }
      break;
    }
    case LIT_CHAR_RIGHT_PAREN:
    {
      out_token_p->type = RE_TOK_END_GROUP;
      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);
      break;
    }
    case LIT_CHAR_LEFT_SQUARE:
    {
      out_token_p->type = RE_TOK_START_CHAR_CLASS;

      if (parser_ctx_p->input_curr_p >= parser_ctx_p->input_end_p)
      {
        return ecma_raise_syntax_error (ECMA_ERR_MSG ("invalid character class"));
      }

      if (*parser_ctx_p->input_curr_p == LIT_CHAR_CIRCUMFLEX)
      {
        out_token_p->type = RE_TOK_START_INV_CHAR_CLASS;
        parser_ctx_p->input_curr_p++;
      }

      break;
    }
    case LIT_CHAR_QUESTION:
    case LIT_CHAR_ASTERISK:
    case LIT_CHAR_PLUS:
    {
      return ecma_raise_syntax_error (ECMA_ERR_MSG ("Invalid RegExp token."));
    }
    case LIT_CHAR_NULL:
    {
      out_token_p->type = RE_TOK_EOF;
      break;
    }
    case LIT_CHAR_LEFT_BRACE:
    {
#if ENABLED (JERRY_REGEXP_STRICT_MODE)
      return ecma_raise_syntax_error (ECMA_ERR_MSG ("Invalid RegExp token."));
#else /* !ENABLED (JERRY_REGEXP_STRICT_MODE) */

      /* Make sure that the current '{' does not start an iterator.
       *
       * E.g: /\s+{3,4}/ should fail as there is nothing to iterate.
       *     However /\s+{3,4/ should be valid in web compatibility mode.
       */
      const lit_utf8_byte_t *input_curr_p = parser_ctx_p->input_curr_p;

      lit_utf8_decr (&parser_ctx_p->input_curr_p);
      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);
      if (ecma_is_value_empty (ret_value))
      {
        return ecma_raise_syntax_error (ECMA_ERR_MSG ("Invalid RegExp token."));
      }

      JERRY_ASSERT (ECMA_IS_VALUE_ERROR (ret_value));
      jcontext_release_exception ();

      parser_ctx_p->input_curr_p = input_curr_p;
      /* It was not an iterator, continue the parsing. */
#endif /* ENABLED (JERRY_REGEXP_STRICT_MODE) */
      /* FALLTHRU */
    }
    default:
    {
      out_token_p->type = RE_TOK_CHAR;
      out_token_p->value = ch;
#if ENABLED (JERRY_REGEXP_STRICT_MODE)
      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);
#else
      /* In case of compatiblity mode try the following:
       * 1. Try parsing an iterator after the character.
       * 2.a. If no error is reported: it was an iterator so return an empty value.
       * 2.b. If there was an error: it was not an iterator thus return the current position
       *      to the start of the iterator parsing and set the return value to the empty value.
       * 3. The next 're_parse_next_token' call will handle the further parsing of characters.
       */
      const lit_utf8_byte_t *input_curr_p = parser_ctx_p->input_curr_p;
      ret_value = re_parse_iterator (parser_ctx_p, out_token_p);

      if (!ecma_is_value_empty (ret_value))
      {
        jcontext_release_exception ();
        parser_ctx_p->input_curr_p = input_curr_p;
        ret_value = ECMA_VALUE_EMPTY;
      }
#endif
      break;
    }
  }

  return ret_value;
} /* re_parse_next_token */

/**
 * @}
 * @}
 * @}
 */

#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
