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
#include "ecma-builtin-helpers.h"
#include "ecma-builtins.h"
#include "ecma-builtin-object.h"
#include "ecma-conversion.h"
#include "ecma-exceptions.h"
#include "ecma-function-object.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"
#include "ecma-proxy-object.h"
#include "ecma-string-object.h"
#include "ecma-try-catch-macro.h"
#include "jrt.h"

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
  /* Note: these 6 routines must be in this order */
  ECMA_OBJECT_PROTOTYPE_ROUTINE_START = ECMA_BUILTIN_ID__COUNT - 1,
  ECMA_OBJECT_PROTOTYPE_TO_STRING,
  ECMA_OBJECT_PROTOTYPE_VALUE_OF,
  ECMA_OBJECT_PROTOTYPE_TO_LOCALE_STRING,
  ECMA_OBJECT_PROTOTYPE_GET_PROTO,
  ECMA_OBJECT_PROTOTYPE_IS_PROTOTYPE_OF,
  ECMA_OBJECT_PROTOTYPE_HAS_OWN_PROPERTY,
  ECMA_OBJECT_PROTOTYPE_PROPERTY_IS_ENUMERABLE,
  ECMA_OBJECT_PROTOTYPE_SET_PROTO
};

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-object-prototype.inc.h"
#define BUILTIN_UNDERSCORED_ID object_prototype
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup objectprototype ECMA Object.prototype object built-in
 * @{
 */

/**
 * The Object.prototype object's 'toString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.2
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_to_string (ecma_value_t this_arg) /**< this argument */
{
  return ecma_builtin_helper_object_to_string (this_arg);
} /* ecma_builtin_object_prototype_object_to_string */

/**
 * The Object.prototype object's 'valueOf' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.4
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_value_of (ecma_value_t this_arg) /**< this argument */
{
  return ecma_op_to_object (this_arg);
} /* ecma_builtin_object_prototype_object_value_of */

/**
 * The Object.prototype object's 'toLocaleString' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.3
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_to_locale_string (ecma_value_t this_arg) /**< this argument */
{
  return ecma_op_invoke_by_magic_id (this_arg, LIT_MAGIC_STRING_TO_STRING_UL, &this_arg, 1);
} /* ecma_builtin_object_prototype_object_to_locale_string */

/**
 * The Object.prototype object's 'hasOwnProperty' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.5
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_has_own_property (ecma_object_t *obj_p, /**< this argument */
                                                       ecma_string_t *prop_name_p) /**< first argument */
{
#if ENABLED (JERRY_BUILTIN_PROXY)
  if (ECMA_OBJECT_IS_PROXY (obj_p))
  {
    ecma_property_descriptor_t prop_desc;

    ecma_value_t status = ecma_proxy_object_get_own_property_descriptor (obj_p, prop_name_p, &prop_desc);

    if (ecma_is_value_true (status))
    {
      ecma_free_property_descriptor (&prop_desc);
    }

    return status;
  }
#endif /* ENABLED (JERRY_BUILTIN_PROXY) */

  return ecma_make_boolean_value (ecma_op_ordinary_object_has_own_property (obj_p, prop_name_p));
} /* ecma_builtin_object_prototype_object_has_own_property */

/**
 * The Object.prototype object's 'isPrototypeOf' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.6
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_is_prototype_of (ecma_object_t *obj_p, /**< this argument */
                                                      ecma_value_t arg) /**< routine's first argument */
{
  /* 3. Compare prototype to object */
  ecma_value_t v_obj_value = ecma_op_to_object (arg);

  if (ECMA_IS_VALUE_ERROR (v_obj_value))
  {
    return v_obj_value;
  }

  ecma_object_t *v_obj_p = ecma_get_object_from_value (v_obj_value);

  ecma_value_t ret_value = ecma_op_object_is_prototype_of (obj_p, v_obj_p);

  ecma_deref_object (v_obj_p);

  return ret_value;
} /* ecma_builtin_object_prototype_object_is_prototype_of */

/**
 * The Object.prototype object's 'propertyIsEnumerable' routine
 *
 * See also:
 *          ECMA-262 v5, 15.2.4.7
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_object_prototype_object_property_is_enumerable (ecma_object_t *obj_p, /**< this argument */
                                                             ecma_string_t *prop_name_p) /**< first argument */
{
  ecma_property_descriptor_t prop_desc;
  ecma_value_t status = ecma_op_object_get_own_property_descriptor (obj_p, prop_name_p, &prop_desc);

  if (!ecma_is_value_true (status))
  {
    return status;
  }

  bool is_enumerable = (prop_desc.flags & ECMA_PROP_IS_ENUMERABLE);

  ecma_free_property_descriptor (&prop_desc);

  return ecma_make_boolean_value (is_enumerable);
} /* ecma_builtin_object_prototype_object_property_is_enumerable */

/**
 * Dispatcher of the built-in's routines
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
ecma_value_t
ecma_builtin_object_prototype_dispatch_routine (uint16_t builtin_routine_id, /**< built-in wide routine
                                                                              *   identifier */
                                                ecma_value_t this_arg, /**< 'this' argument value */
                                                const ecma_value_t arguments_list_p[], /**< list of arguments
                                                                                      *   passed to routine */
                                                uint32_t arguments_number) /**< length of arguments' list */
{
  JERRY_UNUSED (arguments_number);

  /* no specialization */
  if (builtin_routine_id <= ECMA_OBJECT_PROTOTYPE_VALUE_OF)
  {
    if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_TO_STRING)
    {
      return ecma_builtin_object_prototype_object_to_string (this_arg);
    }

    JERRY_ASSERT (builtin_routine_id <= ECMA_OBJECT_PROTOTYPE_VALUE_OF);

    return ecma_builtin_object_prototype_object_value_of (this_arg);
  }

  if (builtin_routine_id <= ECMA_OBJECT_PROTOTYPE_IS_PROTOTYPE_OF)
  {
    if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_IS_PROTOTYPE_OF)
    {
      /* 15.2.4.6.1. */
      if (!ecma_is_value_object (arguments_list_p[0]))
      {
        return ECMA_VALUE_FALSE;
      }
    }

    if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_TO_LOCALE_STRING)
    {
      return ecma_builtin_object_prototype_object_to_locale_string (this_arg);
    }

    ecma_value_t to_object = ecma_op_to_object (this_arg);

    if (ECMA_IS_VALUE_ERROR (to_object))
    {
      return to_object;
    }

    ecma_object_t *obj_p = ecma_get_object_from_value (to_object);

    ecma_value_t ret_value;

#if ENABLED (JERRY_ESNEXT)
    if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_GET_PROTO)
    {
      ret_value = ecma_builtin_object_object_get_prototype_of (obj_p);
    }
    else
#endif /* ENABLED (JERRY_ESNEXT)*/
    {
      ret_value = ecma_builtin_object_prototype_object_is_prototype_of (obj_p, arguments_list_p[0]);
    }

    ecma_deref_object (obj_p);

    return ret_value;
  }

  JERRY_ASSERT (builtin_routine_id >= ECMA_OBJECT_PROTOTYPE_HAS_OWN_PROPERTY);

#if ENABLED (JERRY_ESNEXT)
  if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_SET_PROTO)
  {
    return ecma_builtin_object_object_set_proto (this_arg, arguments_list_p[0]);
  }
#endif /* ENABLED (JERRY_ESNEXT)*/

  ecma_string_t *prop_name_p = ecma_op_to_property_key (arguments_list_p[0]);

  if (prop_name_p == NULL)
  {
    return ECMA_VALUE_ERROR;
  }

  ecma_value_t to_object = ecma_op_to_object (this_arg);

  if (ECMA_IS_VALUE_ERROR (to_object))
  {
    ecma_deref_ecma_string (prop_name_p);
    return to_object;
  }

  ecma_object_t *obj_p = ecma_get_object_from_value (to_object);

  ecma_value_t ret_value;

  if (builtin_routine_id == ECMA_OBJECT_PROTOTYPE_HAS_OWN_PROPERTY)
  {
    ret_value = ecma_builtin_object_prototype_object_has_own_property (obj_p, prop_name_p);
  }
  else
  {
    ret_value = ecma_builtin_object_prototype_object_property_is_enumerable (obj_p, prop_name_p);
  }

  ecma_deref_ecma_string (prop_name_p);
  ecma_deref_object (obj_p);

  return ret_value;
} /* ecma_builtin_object_prototype_dispatch_routine */

/**
 * @}
 * @}
 * @}
 */
