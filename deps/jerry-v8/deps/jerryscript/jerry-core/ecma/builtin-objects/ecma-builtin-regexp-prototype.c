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
#include "ecma-array-object.h"
#include "ecma-builtins.h"
#include "ecma-builtin-helpers.h"
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-globals.h"
#include "ecma-gc.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"
#include "ecma-regexp-object.h"
#include "ecma-try-catch-macro.h"
#include "lit-char-helpers.h"

#if ENABLED (JERRY_BUILTIN_REGEXP)
#include "ecma-regexp-object.h"
#include "re-compiler.h"

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-regexp-prototype.inc.h"
#define BUILTIN_UNDERSCORED_ID regexp_prototype
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup regexpprototype ECMA RegExp.prototype object built-in
 * @{
 */

#if ENABLED (JERRY_ES2015)
/**
 * Helper function to retrieve the flags associated with a RegExp object
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_EMPTY - otherwise
 */
static ecma_value_t
ecma_builtin_regexp_prototype_flags_helper (ecma_value_t this, /**< this value */
                                            uint16_t *flags_p) /**< [out] flags */
{
  if (!ecma_object_is_regexp_object (this))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("Incompatible type"));
  }

  ecma_extended_object_t *re_obj_p = (ecma_extended_object_t *) ecma_get_object_from_value (this);
  re_compiled_code_t *bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t,
                                                                  re_obj_p->u.class_prop.u.value);

  if (bc_p != NULL)
  {
    *flags_p = bc_p->header.status_flags;
  }

  return ECMA_VALUE_EMPTY;
} /* ecma_builtin_regexp_prototype_flags_helper */

/**
 * The RegExp.prototype object's 'flags' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.3
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         string value     - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_flags (ecma_value_t this_arg) /**< this argument */
{
  static const lit_magic_string_id_t flag_lit_ids[] =
  {
    LIT_MAGIC_STRING_GLOBAL,
    LIT_MAGIC_STRING_IGNORECASE_UL,
    LIT_MAGIC_STRING_MULTILINE,
    LIT_MAGIC_STRING_UNICODE,
    LIT_MAGIC_STRING_STICKY
  };

  static const lit_utf8_byte_t flag_chars[] =
  {
    LIT_CHAR_LOWERCASE_G,
    LIT_CHAR_LOWERCASE_I,
    LIT_CHAR_LOWERCASE_M,
    LIT_CHAR_LOWERCASE_U,
    LIT_CHAR_LOWERCASE_Y
  };

  if (!ecma_is_value_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' value is not an object."));
  }

  ecma_object_t *object_p = ecma_get_object_from_value (this_arg);

  ecma_stringbuilder_t builder = ecma_stringbuilder_create ();
  for (uint32_t i = 0; i < sizeof (flag_lit_ids) / sizeof (lit_magic_string_id_t); i++)
  {
    ecma_value_t result = ecma_op_object_get_by_magic_id (object_p, flag_lit_ids[i]);
    if (ECMA_IS_VALUE_ERROR (result))
    {
      ecma_stringbuilder_destroy (&builder);
      return result;
    }

    if (ecma_op_to_boolean (result))
    {
      ecma_stringbuilder_append_byte (&builder, flag_chars[i]);
    }

    ecma_free_value (result);
  }

  return ecma_make_string_value (ecma_stringbuilder_finalize (&builder));
} /* ecma_builtin_regexp_prototype_get_flags */

/**
 * The RegExp.prototype object's 'source' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.10
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         string value     - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_source (ecma_value_t this_arg) /**< this argument */
{
  if (!ecma_object_is_regexp_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("Incompatible type"));
  }

  ecma_extended_object_t *re_obj_p = (ecma_extended_object_t *) ecma_get_object_from_value (this_arg);
  re_compiled_code_t *bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t,
                                                                  re_obj_p->u.class_prop.u.value);

  if (bc_p != NULL)
  {
    ecma_ref_ecma_string (ecma_get_string_from_value (bc_p->source));
    return bc_p->source;
  }

  return ecma_make_string_value (ecma_get_magic_string (LIT_MAGIC_STRING_EMPTY_NON_CAPTURE_GROUP));
} /* ecma_builtin_regexp_prototype_get_source */

/**
 * The RegExp.prototype object's 'global' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.4
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_TRUE  - if 'global' flag is set
 *         ECMA_VALUE_FALSE - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_global (ecma_value_t this_arg) /**< this argument */
{
  uint16_t flags = RE_FLAG_EMPTY;
  ecma_value_t ret_value = ecma_builtin_regexp_prototype_flags_helper (this_arg, &flags);
  if (ECMA_IS_VALUE_ERROR (ret_value))
  {
    return ret_value;
  }

  return ecma_make_boolean_value (flags & RE_FLAG_GLOBAL);
} /* ecma_builtin_regexp_prototype_get_global */

/**
 * The RegExp.prototype object's 'ignoreCase' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.5
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_TRUE  - if 'ignoreCase' flag is set
 *         ECMA_VALUE_FALSE - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_ignorecase (ecma_value_t this_arg) /**< this argument */
{
  uint16_t flags = RE_FLAG_EMPTY;
  ecma_value_t ret_value = ecma_builtin_regexp_prototype_flags_helper (this_arg, &flags);
  if (ECMA_IS_VALUE_ERROR (ret_value))
  {
    return ret_value;
  }

  return ecma_make_boolean_value (flags & RE_FLAG_IGNORE_CASE);
} /* ecma_builtin_regexp_prototype_get_ignorecase */

/**
 * The RegExp.prototype object's 'multiline' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.7
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_TRUE  - if 'multiline' flag is set
 *         ECMA_VALUE_FALSE - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_multiline (ecma_value_t this_arg) /**< this argument */
{
  uint16_t flags = RE_FLAG_EMPTY;
  ecma_value_t ret_value = ecma_builtin_regexp_prototype_flags_helper (this_arg, &flags);
  if (ECMA_IS_VALUE_ERROR (ret_value))
  {
    return ret_value;
  }

  return ecma_make_boolean_value (flags & RE_FLAG_MULTILINE);
} /* ecma_builtin_regexp_prototype_get_multiline */

/**
 * The RegExp.prototype object's 'sticky' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.12
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_TRUE  - if 'sticky' flag is set
 *         ECMA_VALUE_FALSE - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_sticky (ecma_value_t this_arg) /**< this argument */
{
  uint16_t flags = RE_FLAG_EMPTY;
  ecma_value_t ret_value = ecma_builtin_regexp_prototype_flags_helper (this_arg, &flags);
  if (ECMA_IS_VALUE_ERROR (ret_value))
  {
    return ret_value;
  }

  return ecma_make_boolean_value (flags & RE_FLAG_STICKY);
} /* ecma_builtin_regexp_prototype_get_sticky */

/**
 * The RegExp.prototype object's 'unicode' accessor property
 *
 * See also:
 *          ECMA-262 v6, 21.2.5.15
 *
 * @return ECMA_VALUE_ERROR - if 'this' is not a RegExp object
 *         ECMA_VALUE_TRUE  - if 'unicode' flag is set
 *         ECMA_VALUE_FALSE - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_get_unicode (ecma_value_t this_arg) /**< this argument */
{
  uint16_t flags = RE_FLAG_EMPTY;
  ecma_value_t ret_value = ecma_builtin_regexp_prototype_flags_helper (this_arg, &flags);
  if (ECMA_IS_VALUE_ERROR (ret_value))
  {
    return ret_value;
  }

  return ecma_make_boolean_value (flags & RE_FLAG_UNICODE);
} /* ecma_builtin_regexp_prototype_get_unicode */
#endif /* ENABLED (JERRY_ES2015) */

#if ENABLED (JERRY_BUILTIN_ANNEXB)

/**
 * The RegExp.prototype object's 'compile' routine
 *
 * See also:
 *          ECMA-262 v5, B.2.5.1
 *
 * @return undefined        - if compiled successfully
 *         error ecma value - otherwise
 *
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_compile (ecma_value_t this_arg, /**< this argument */
                                       ecma_value_t pattern_arg, /**< pattern or RegExp object */
                                       ecma_value_t flags_arg) /**< flags */
{
  if (!ecma_object_is_regexp_object (this_arg)
      /* The builtin RegExp.prototype object does not have [[RegExpMatcher]] internal slot */
      || ecma_get_object_from_value (this_arg) == ecma_builtin_get (ECMA_BUILTIN_ID_REGEXP_PROTOTYPE))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("Incompatible type"));
  }

  uint16_t flags = 0;

  if (ecma_object_is_regexp_object (pattern_arg)
      && ecma_get_object_from_value (pattern_arg) != ecma_builtin_get (ECMA_BUILTIN_ID_REGEXP_PROTOTYPE))
  {
    if (!ecma_is_value_undefined (flags_arg))
    {
      return ecma_raise_type_error (ECMA_ERR_MSG ("Invalid argument"));
    }

    /* Compile from existing RegExp object. */
    ecma_extended_object_t *target_p = (ecma_extended_object_t *) ecma_get_object_from_value (pattern_arg);
    re_compiled_code_t *target_bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t,
                                                                           target_p->u.class_prop.u.value);

    ecma_object_t *this_object_p = ecma_get_object_from_value (this_arg);
    ecma_extended_object_t *current_p = (ecma_extended_object_t *) this_object_p;

    re_compiled_code_t *current_bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t,
                                                                            current_p->u.class_prop.u.value);

    JERRY_ASSERT (current_bc_p != NULL);
    ecma_bytecode_deref ((ecma_compiled_code_t *) current_bc_p);

    JERRY_ASSERT (target_bc_p != NULL);
    ecma_bytecode_ref ((ecma_compiled_code_t *) target_bc_p);
    ECMA_SET_INTERNAL_VALUE_POINTER (current_p->u.class_prop.u.value, target_bc_p);
    ecma_regexp_initialize_props (this_object_p,
                                  ecma_get_string_from_value (target_bc_p->source),
                                  target_bc_p->header.status_flags);
    return ecma_copy_value (this_arg);
  }

  ecma_string_t *pattern_string_p = ecma_regexp_read_pattern_str_helper (pattern_arg);

  /* Get source string. */
  if (pattern_string_p == NULL)
  {
    return ECMA_VALUE_ERROR;
  }

  /* Parse flags. */
  if (!ecma_is_value_undefined (flags_arg))
  {
    ecma_string_t *flags_str_p = ecma_op_to_string (flags_arg);
    if (JERRY_UNLIKELY (flags_str_p == NULL))
    {
      ecma_deref_ecma_string (pattern_string_p);
      return ECMA_VALUE_ERROR;
    }

    ecma_value_t parsed_flags_val = ecma_regexp_parse_flags (flags_str_p, &flags);
    ecma_deref_ecma_string (flags_str_p);

    if (ECMA_IS_VALUE_ERROR (parsed_flags_val))
    {
      ecma_deref_ecma_string (pattern_string_p);
      return parsed_flags_val;
    }
  }

  /* Try to compile bytecode from new source. */
  const re_compiled_code_t *new_bc_p = NULL;
  ecma_value_t bc_val = re_compile_bytecode (&new_bc_p, pattern_string_p, flags);
  if (ECMA_IS_VALUE_ERROR (bc_val))
  {
    ecma_deref_ecma_string (pattern_string_p);
    return bc_val;
  }

  ecma_object_t *this_obj_p = ecma_get_object_from_value (this_arg);
  ecma_value_t *bc_prop_p = &(((ecma_extended_object_t *) this_obj_p)->u.class_prop.u.value);

  re_compiled_code_t *old_bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t, *bc_prop_p);

  JERRY_ASSERT (old_bc_p != NULL);
  ecma_bytecode_deref ((ecma_compiled_code_t *) old_bc_p);

  ECMA_SET_INTERNAL_VALUE_POINTER (*bc_prop_p, new_bc_p);
  ecma_regexp_initialize_props (this_obj_p, pattern_string_p, flags);
  ecma_deref_ecma_string (pattern_string_p);

  return ecma_copy_value (this_arg);
} /* ecma_builtin_regexp_prototype_compile */

#endif /* ENABLED (JERRY_BUILTIN_ANNEXB) */

/**
 * The RegExp.prototype object's 'exec' routine
 *
 * See also:
 *          ECMA-262 v5, 15.10.6.2
 *
 * @return array object containing the results - if the matched
 *         null                                - otherwise
 *
 *         May raise error, so returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_exec (ecma_value_t this_arg, /**< this argument */
                                    ecma_value_t arg) /**< routine's argument */
{
  if (!ecma_object_is_regexp_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("Incomplete RegExp type"));
  }

  ecma_value_t obj_this = ecma_op_to_object (this_arg);
  if (ECMA_IS_VALUE_ERROR (obj_this))
  {
    return obj_this;
  }

  ecma_string_t *input_str_p = ecma_op_to_string (arg);
  if (JERRY_UNLIKELY (input_str_p == NULL))
  {
    ecma_free_value (obj_this);
    return ECMA_VALUE_ERROR;
  }

  ecma_value_t ret_value = ecma_regexp_exec_helper (obj_this, ecma_make_string_value (input_str_p), false);

  ecma_free_value (obj_this);
  ecma_deref_ecma_string (input_str_p);

  return ret_value;
} /* ecma_builtin_regexp_prototype_exec */

/**
 * The RegExp.prototype object's 'test' routine
 *
 * See also:
 *          ECMA-262 v5, 15.10.6.3
 *
 * @return true  - if match is not null
 *         false - otherwise
 *
 *         May raise error, so returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_test (ecma_value_t this_arg, /**< this argument */
                                    ecma_value_t arg) /**< routine's argument */
{
  ecma_value_t result = ecma_builtin_regexp_prototype_exec (this_arg, arg);

  if (ECMA_IS_VALUE_ERROR (result))
  {
    return result;
  }

  ecma_value_t ret_value = ecma_make_boolean_value (!ecma_is_value_null (result));
  ecma_free_value (result);

  return ret_value;
} /* ecma_builtin_regexp_prototype_test */

/**
 * The RegExp.prototype object's 'toString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.10.6.4
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_to_string (ecma_value_t this_arg) /**< this argument */
{
#if ENABLED (JERRY_ES2015)
  if (!ecma_is_value_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' value is not an object."));
  }

  ecma_object_t *object_p = ecma_get_object_from_value (this_arg);

  ecma_value_t result = ecma_op_object_get_by_magic_id (object_p, LIT_MAGIC_STRING_SOURCE);
  if (ECMA_IS_VALUE_ERROR (result))
  {
    return result;
  }

  ecma_string_t *source_p = ecma_op_to_string (result);
  ecma_free_value (result);

  if (source_p == NULL)
  {
    return ECMA_VALUE_ERROR;
  }

  result = ecma_op_object_get_by_magic_id (object_p, LIT_MAGIC_STRING_FLAGS);
  if (ECMA_IS_VALUE_ERROR (result))
  {
    ecma_deref_ecma_string (source_p);
    return result;
  }

  ecma_string_t *flags_p = ecma_op_to_string (result);
  ecma_free_value (result);

  if (flags_p == NULL)
  {
    ecma_deref_ecma_string (source_p);
    return ECMA_VALUE_ERROR;
  }

  ecma_stringbuilder_t builder = ecma_stringbuilder_create ();
  ecma_stringbuilder_append_byte (&builder, LIT_CHAR_SLASH);
  ecma_stringbuilder_append (&builder, source_p);
  ecma_stringbuilder_append_byte (&builder, LIT_CHAR_SLASH);
  ecma_stringbuilder_append (&builder, flags_p);

  ecma_deref_ecma_string (source_p);
  ecma_deref_ecma_string (flags_p);

  return ecma_make_string_value (ecma_stringbuilder_finalize (&builder));
#else /* !ENABLED (JERRY_ES2015) */
  if (!ecma_object_is_regexp_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("'this' value is not a RegExp object."));
  }

  ecma_object_t *obj_p = ecma_get_object_from_value (this_arg);
  ecma_extended_object_t *re_obj_p = (ecma_extended_object_t *) obj_p;

  re_compiled_code_t *bc_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (re_compiled_code_t,
                                                                  re_obj_p->u.class_prop.u.value);

  ecma_string_t *source_p;
  uint16_t flags;

  if (bc_p != NULL)
  {
    source_p = ecma_get_string_from_value (bc_p->source);
    flags = bc_p->header.status_flags;
  }
  else
  {
    source_p = ecma_get_magic_string (LIT_MAGIC_STRING_EMPTY_NON_CAPTURE_GROUP);
    flags = RE_FLAG_EMPTY;
  }

  ecma_stringbuilder_t result = ecma_stringbuilder_create ();
  ecma_stringbuilder_append_byte (&result, LIT_CHAR_SLASH);
  ecma_stringbuilder_append (&result, source_p);
  ecma_stringbuilder_append_byte (&result, LIT_CHAR_SLASH);

  if (flags & RE_FLAG_GLOBAL)
  {
    ecma_stringbuilder_append_byte (&result, LIT_CHAR_LOWERCASE_G);
  }

  if (flags & RE_FLAG_IGNORE_CASE)
  {
    ecma_stringbuilder_append_byte (&result, LIT_CHAR_LOWERCASE_I);
  }

  if (flags & RE_FLAG_MULTILINE)
  {
    ecma_stringbuilder_append_byte (&result, LIT_CHAR_LOWERCASE_M);
  }

  return ecma_make_string_value (ecma_stringbuilder_finalize (&result));
#endif /* ENABLED (JERRY_ES2015) */
} /* ecma_builtin_regexp_prototype_to_string */

#if ENABLED (JERRY_ES2015)
/**
 * Helper function to determine if method is the builtin exec method
 *
 * @return true, if function is the builtin exec method
 *         false, otherwise
 */
inline bool JERRY_ATTR_ALWAYS_INLINE
ecma_builtin_is_regexp_exec (ecma_extended_object_t *obj_p)
{
  return (ecma_get_object_is_builtin ((ecma_object_t *) obj_p)
          && obj_p->u.built_in.routine_id == ECMA_ROUTINE_LIT_MAGIC_STRING_EXECecma_builtin_regexp_prototype_exec);
} /* ecma_builtin_is_regexp_exec */

/**
 * The RegExp.prototype object's '@@replace' routine
 *
 * See also:
 *          ECMA-262 v6.0, 21.2.5.8
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_symbol_replace (ecma_value_t this_arg, /**< this argument */
                                              ecma_value_t string_arg, /**< source string */
                                              ecma_value_t replace_arg) /**< replace string */
{
  return ecma_regexp_replace_helper (this_arg, string_arg, replace_arg);
} /* ecma_builtin_regexp_prototype_symbol_replace */

/**
 * The RegExp.prototype object's '@@match' routine
 *
 * See also:
 *          ECMA-262 v6.0, 21.2.5.6
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_regexp_prototype_symbol_match (ecma_value_t this_arg, /**< this argument */
                                            ecma_value_t string_arg) /**< source string */
{
  if (!ecma_is_value_object (this_arg))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("TypeError"));
  }

  ecma_string_t *str_p = ecma_op_to_string (string_arg);

  if (JERRY_UNLIKELY (str_p == NULL))
  {
    return ECMA_VALUE_ERROR;
  }

  ecma_object_t *obj_p = ecma_get_object_from_value (this_arg);

  ecma_value_t global_value = ecma_op_object_get_by_magic_id (obj_p, LIT_MAGIC_STRING_GLOBAL);

  if (ECMA_IS_VALUE_ERROR (global_value))
  {
    ecma_deref_ecma_string (str_p);
    return global_value;
  }

  bool global = ecma_op_to_boolean (global_value);

  ecma_free_value (global_value);

  if (!global)
  {
    ecma_value_t result = ecma_op_regexp_exec (this_arg, str_p);
    ecma_deref_ecma_string (str_p);
    return result;
  }

  ecma_value_t full_unicode_value = ecma_op_object_get_by_magic_id (obj_p, LIT_MAGIC_STRING_UNICODE);

  if (ECMA_IS_VALUE_ERROR (full_unicode_value))
  {
    ecma_deref_ecma_string (str_p);
    return full_unicode_value;
  }

  bool full_unicode = ecma_op_to_boolean (full_unicode_value);

  ecma_free_value (full_unicode_value);

  ecma_value_t set_status = ecma_op_object_put (obj_p,
                                                ecma_get_magic_string (LIT_MAGIC_STRING_LASTINDEX_UL),
                                                ecma_make_uint32_value (0),
                                                true);

  if (ECMA_IS_VALUE_ERROR (set_status))
  {
    ecma_deref_ecma_string (str_p);
    return set_status;
  }

  ecma_value_t ret_value = ECMA_VALUE_ERROR;
  ecma_value_t result_array = ecma_op_create_array_object (0, 0, false);
  ecma_object_t *result_array_p = ecma_get_object_from_value (result_array);
  uint32_t n = 0;

  while (true)
  {
    ecma_value_t result_value = ecma_op_regexp_exec (this_arg, str_p);

    if (ECMA_IS_VALUE_ERROR (result_value))
    {
      goto result_cleanup;
    }

    if (ecma_is_value_null (result_value))
    {
      if (n == 0)
      {
        ret_value = ECMA_VALUE_NULL;
        goto result_cleanup;
      }

      ecma_deref_ecma_string (str_p);
      return result_array;
    }

    ecma_object_t *result_value_p = ecma_get_object_from_value (result_value);
    ecma_value_t match_str_value = ecma_op_object_get_by_uint32_index (result_value_p, 0);

    ecma_deref_object (result_value_p);

    if (ECMA_IS_VALUE_ERROR (match_str_value))
    {
      goto result_cleanup;
    }

    ecma_string_t *match_str_p = ecma_op_to_string (match_str_value);

    if (JERRY_UNLIKELY (match_str_p == NULL))
    {
      ecma_free_value (match_str_value);
      goto result_cleanup;
    }

    ecma_value_t new_prop = ecma_builtin_helper_def_prop_by_index (result_array_p,
                                                                   n,
                                                                   match_str_value,
                                                                   ECMA_PROPERTY_CONFIGURABLE_ENUMERABLE_WRITABLE);

    JERRY_ASSERT (!ECMA_IS_VALUE_ERROR (new_prop));

    ecma_value_t match_result = ECMA_VALUE_ERROR;
    if (ecma_string_is_empty (match_str_p))
    {
      ecma_value_t this_index = ecma_op_object_get_by_magic_id (obj_p, LIT_MAGIC_STRING_LASTINDEX_UL);

      if (ECMA_IS_VALUE_ERROR (this_index))
      {
        goto match_cleanup;
      }

      uint32_t index;
      ecma_value_t length_value = ecma_op_to_length (this_index, &index);

      ecma_free_value (this_index);

      if (ECMA_IS_VALUE_ERROR (length_value))
      {
        goto match_cleanup;
      }

      uint32_t next_index = ecma_op_advance_string_index (str_p, index, full_unicode);

      ecma_value_t next_set_status = ecma_op_object_put (obj_p,
                                                         ecma_get_magic_string (LIT_MAGIC_STRING_LASTINDEX_UL),
                                                         ecma_make_uint32_value (next_index),
                                                         true);

      if (ECMA_IS_VALUE_ERROR (next_set_status))
      {
        goto match_cleanup;
      }
    }

    match_result = ECMA_VALUE_EMPTY;

match_cleanup:
    ecma_deref_ecma_string (match_str_p);
    ecma_free_value (match_str_value);

    if (ECMA_IS_VALUE_ERROR (match_result))
    {
      goto result_cleanup;
    }

    n++;
  }

result_cleanup:
  ecma_deref_ecma_string (str_p);
  ecma_deref_object (result_array_p);
  return ret_value;
} /* ecma_builtin_regexp_prototype_symbol_match */
#endif /* ENABLED (JERRY_ES2015) */

/**
 * @}
 * @}
 * @}
 */

#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
