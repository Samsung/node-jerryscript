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
#include "ecma-exceptions.h"
#include "ecma-gc.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-function-object.h"
#include "ecma-objects.h"
#include "jcontext.h"
#include "jrt-bit-fields.h"

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 */

static void ecma_instantiate_builtin (ecma_builtin_id_t id);

/**
 * Helper definition for ecma_builtin_property_list_references.
 */
typedef const ecma_builtin_property_descriptor_t *ecma_builtin_property_list_reference_t;

/**
 * Definition of built-in dispatch routine function pointer.
 */
typedef ecma_value_t (*ecma_builtin_dispatch_routine_t) (uint16_t builtin_routine_id,
                                                         ecma_value_t this_arg,
                                                         const ecma_value_t arguments_list[],
                                                         uint32_t arguments_number);
/**
 * Definition of built-in dispatch call function pointer.
 */
typedef ecma_value_t (*ecma_builtin_dispatch_call_t) (const ecma_value_t arguments_list[],
                                                      uint32_t arguments_number);
/**
 * Definition of a builtin descriptor which contains the builtin object's:
 * - prototype objects's id (13-bits)
 * - type (3-bits)
 *
 * Layout:
 *
 * |----------------------|---------------|
 *     prototype_id(13)      obj_type(3)
 */
typedef uint16_t ecma_builtin_descriptor_t;

/**
 * Bitshift index for get the prototype object's id from a builtin descriptor
 */
#define ECMA_BUILTIN_PROTOTYPE_ID_SHIFT 3

/**
 * Bitmask for get the object's type from a builtin descriptor
 */
#define ECMA_BUILTIN_OBJECT_TYPE_MASK ((1 << ECMA_BUILTIN_PROTOTYPE_ID_SHIFT) - 1)

/**
 * Create a builtin descriptor value
 */
#define ECMA_MAKE_BUILTIN_DESCRIPTOR(type, proto_id) \
  (((proto_id) << ECMA_BUILTIN_PROTOTYPE_ID_SHIFT) | (type))

/**
 * List of the built-in descriptors.
 */
static const ecma_builtin_descriptor_t ecma_builtin_descriptors[] =
{
/** @cond doxygen_suppress */
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  ECMA_MAKE_BUILTIN_DESCRIPTOR (object_type, object_prototype_builtin_id),
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
#define BUILTIN_ROUTINE(a, b, c, d, e)
#define BUILTIN(builtin_id, \
                object_type, \
                object_prototype_builtin_id, \
                is_extensible, \
                lowercase_name) \
  ECMA_MAKE_BUILTIN_DESCRIPTOR (object_type, object_prototype_builtin_id),
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
/** @endcond */
};

#ifndef JERRY_NDEBUG
/** @cond doxygen_suppress */
enum
{
  ECMA_BUILTIN_EXTENSIBLE_CHECK =
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  (is_extensible != 0 || builtin_id == ECMA_BUILTIN_ID_TYPE_ERROR_THROWER) &&
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
#define BUILTIN_ROUTINE(a, b, c, d, e)
#define BUILTIN(builtin_id, \
                object_type, \
                object_prototype_builtin_id, \
                is_extensible, \
                lowercase_name) \
  (is_extensible != 0 || builtin_id == ECMA_BUILTIN_ID_TYPE_ERROR_THROWER) &&
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
  true
};
/** @endcond */

/**
 * All the builtin object must be extensible except the ThrowTypeError object.
 */
JERRY_STATIC_ASSERT (ECMA_BUILTIN_EXTENSIBLE_CHECK == true,
                     ecma_builtin_must_be_extensible_except_the_builtin_thorw_type_error_object);
#endif /* !JERRY_NDEBUG */

/**
 * List of the built-in routines.
 */
static const ecma_builtin_dispatch_routine_t ecma_builtin_routines[] =
{
/** @cond doxygen_suppress */
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _dispatch_routine,
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
#define BUILTIN_ROUTINE(a, b, c, d, e)
#define BUILTIN(builtin_id, \
                object_type, \
                object_prototype_builtin_id, \
                is_extensible, \
                lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _dispatch_routine,
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
/** @endcond */
};

/**
 * List of the built-in call functions.
 */
static const ecma_builtin_dispatch_call_t ecma_builtin_call_functions[] =
{
/** @cond doxygen_suppress */
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _dispatch_call,
#include "ecma-builtins.inc.h"
#undef BUILTIN_ROUTINE
#undef BUILTIN
/** @endcond */
};

/**
 * List of the built-in construct functions.
 */
static const ecma_builtin_dispatch_call_t ecma_builtin_construct_functions[] =
{
/** @cond doxygen_suppress */
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _dispatch_construct,
#include "ecma-builtins.inc.h"
#undef BUILTIN_ROUTINE
#undef BUILTIN
/** @endcond */
};

/**
 * Property descriptor lists for all built-ins.
 */
static const ecma_builtin_property_list_reference_t ecma_builtin_property_list_references[] =
{
/** @cond doxygen_suppress */
#define BUILTIN(a, b, c, d, e)
#define BUILTIN_ROUTINE(builtin_id, \
                        object_type, \
                        object_prototype_builtin_id, \
                        is_extensible, \
                        lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _property_descriptor_list,
#include "ecma-builtins.inc.h"
#undef BUILTIN
#undef BUILTIN_ROUTINE
#define BUILTIN_ROUTINE(a, b, c, d, e)
#define BUILTIN(builtin_id, \
                object_type, \
                object_prototype_builtin_id, \
                is_extensible, \
                lowercase_name) \
  ecma_builtin_ ## lowercase_name ## _property_descriptor_list,
#include "ecma-builtins.inc.h"
#undef BUILTIN_ROUTINE
#undef BUILTIN
/** @endcond */
};

/**
 * Get the number of properties of a built-in object.
 *
 * @return the number of properties
 */
static size_t
ecma_builtin_get_property_count (ecma_builtin_id_t builtin_id) /**< built-in ID */
{
  JERRY_ASSERT (builtin_id < ECMA_BUILTIN_ID__COUNT);
  const ecma_builtin_property_descriptor_t *property_list_p = ecma_builtin_property_list_references[builtin_id];

  const ecma_builtin_property_descriptor_t *curr_property_p = property_list_p;

  while (curr_property_p->magic_string_id != LIT_MAGIC_STRING__COUNT)
  {
    curr_property_p++;
  }

  return (size_t) (curr_property_p - property_list_p);
} /* ecma_builtin_get_property_count */

/**
 * Check if passed object is the instance of specified built-in.
 *
 * @return true  - if the object is instance of the specified built-in
 *         false - otherwise
 */
bool
ecma_builtin_is (ecma_object_t *obj_p, /**< pointer to an object */
                 ecma_builtin_id_t builtin_id) /**< id of built-in to check on */
{
  JERRY_ASSERT (obj_p != NULL && !ecma_is_lexical_environment (obj_p));
  JERRY_ASSERT (builtin_id < ECMA_BUILTIN_ID__COUNT);

  /* If a built-in object is not instantiated, its value is NULL,
     hence it cannot be equal to a valid object. */
  jmem_cpointer_t builtin_cp = JERRY_CONTEXT (ecma_builtin_objects)[builtin_id];

  return (builtin_cp != JMEM_CP_NULL && (obj_p == ECMA_GET_NON_NULL_POINTER (ecma_object_t, builtin_cp)));
} /* ecma_builtin_is */

/**
 * Get reference to specified built-in object
 *
 * Note:
 *   Does not increase the reference counter.
 *
 * @return pointer to the object's instance
 */
ecma_object_t *
ecma_builtin_get (ecma_builtin_id_t builtin_id) /**< id of built-in to check on */
{
  JERRY_ASSERT (builtin_id < ECMA_BUILTIN_ID__COUNT);

  if (JERRY_UNLIKELY (JERRY_CONTEXT (ecma_builtin_objects)[builtin_id] == JMEM_CP_NULL))
  {
    ecma_instantiate_builtin (builtin_id);
  }

  return ECMA_GET_NON_NULL_POINTER (ecma_object_t, JERRY_CONTEXT (ecma_builtin_objects)[builtin_id]);
} /* ecma_builtin_get */

/**
 * Get reference to the global object
 *
 * Note:
 *   Does not increase the reference counter.
 *
 * @return pointer to the global object
 */
inline ecma_object_t * JERRY_ATTR_ALWAYS_INLINE
ecma_builtin_get_global (void)
{
  JERRY_ASSERT (JERRY_CONTEXT (ecma_builtin_objects)[ECMA_BUILTIN_ID_GLOBAL] != JMEM_CP_NULL);

  return ECMA_GET_NON_NULL_POINTER (ecma_object_t, JERRY_CONTEXT (ecma_builtin_objects)[ECMA_BUILTIN_ID_GLOBAL]);
} /* ecma_builtin_get_global */

/**
 * Checks whether the given function is a built-in routine
 *
 * @return true - if the function object is a built-in routine
 *         false - otherwise
 */
inline bool JERRY_ATTR_ALWAYS_INLINE
ecma_builtin_function_is_routine (ecma_object_t *func_obj_p) /**< function object */
{
  JERRY_ASSERT (ecma_get_object_type (func_obj_p) == ECMA_OBJECT_TYPE_FUNCTION);
  JERRY_ASSERT (ecma_get_object_is_builtin (func_obj_p));

  ecma_extended_object_t *ext_func_obj_p = (ecma_extended_object_t *) func_obj_p;
  return (ext_func_obj_p->u.built_in.routine_id >= ECMA_BUILTIN_ID__COUNT);
} /* ecma_builtin_function_is_routine */

/**
 * Instantiate specified ECMA built-in object
 */
static void
ecma_instantiate_builtin (ecma_builtin_id_t obj_builtin_id) /**< built-in id */
{
  JERRY_ASSERT (obj_builtin_id < ECMA_BUILTIN_ID__COUNT);
  JERRY_ASSERT (JERRY_CONTEXT (ecma_builtin_objects)[obj_builtin_id] == JMEM_CP_NULL);

  ecma_builtin_descriptor_t builtin_desc = ecma_builtin_descriptors[obj_builtin_id];
  ecma_builtin_id_t object_prototype_builtin_id = (ecma_builtin_id_t) (builtin_desc >> ECMA_BUILTIN_PROTOTYPE_ID_SHIFT);

  ecma_object_t *prototype_obj_p;

  /* cppcheck-suppress arrayIndexOutOfBoundsCond */
  if (JERRY_UNLIKELY (object_prototype_builtin_id == ECMA_BUILTIN_ID__COUNT))
  {
    prototype_obj_p = NULL;
  }
  else
  {
    if (JERRY_CONTEXT (ecma_builtin_objects)[object_prototype_builtin_id] == JMEM_CP_NULL)
    {
      ecma_instantiate_builtin (object_prototype_builtin_id);
    }
    prototype_obj_p = ECMA_GET_NON_NULL_POINTER (ecma_object_t,
                                                 JERRY_CONTEXT (ecma_builtin_objects)[object_prototype_builtin_id]);
    JERRY_ASSERT (prototype_obj_p != NULL);
  }

  ecma_object_type_t obj_type = (ecma_object_type_t) (builtin_desc & ECMA_BUILTIN_OBJECT_TYPE_MASK);

  bool is_extended_built_in = (obj_type == ECMA_OBJECT_TYPE_CLASS
                               || obj_type == ECMA_OBJECT_TYPE_ARRAY);

  size_t ext_object_size = (is_extended_built_in ? sizeof (ecma_extended_built_in_object_t)
                                                 : sizeof (ecma_extended_object_t));

  size_t property_count = ecma_builtin_get_property_count (obj_builtin_id);

  if (property_count > 32)
  {
    /* Only 64 extra properties supported at the moment.
     * This can be extended to 256 later. */
    JERRY_ASSERT (property_count <= (32 + 64));

    ext_object_size += sizeof (uint32_t) * 2;
  }

  ecma_object_t *obj_p = ecma_create_object (prototype_obj_p, ext_object_size, obj_type);

  if (JERRY_UNLIKELY (obj_builtin_id == ECMA_BUILTIN_ID_TYPE_ERROR_THROWER))
  {
    ecma_op_ordinary_object_prevent_extensions (obj_p);
  }
  else
  {
    ecma_op_ordinary_object_set_extensible (obj_p);
  }

  /*
   * [[Class]] property of built-in object is not stored explicitly.
   *
   * See also: ecma_object_get_class_name
   */

  ecma_set_object_is_builtin (obj_p);
  ecma_built_in_props_t *built_in_props_p;

  if (is_extended_built_in)
  {
    built_in_props_p = &((ecma_extended_built_in_object_t *) obj_p)->built_in;
  }
  else
  {
    built_in_props_p = &((ecma_extended_object_t *) obj_p)->u.built_in;
  }

  built_in_props_p->id = (uint8_t) obj_builtin_id;
  built_in_props_p->routine_id = (uint16_t) obj_builtin_id;
  built_in_props_p->u.instantiated_bitset[0] = 0;

  if (property_count > 32)
  {
    built_in_props_p->length_and_bitset_size = 1 << ECMA_BUILT_IN_BITSET_SHIFT;

    uint32_t *instantiated_bitset_p = built_in_props_p->u.instantiated_bitset;
    instantiated_bitset_p[1] = 0;
    instantiated_bitset_p[2] = 0;
  }
  else
  {
    built_in_props_p->length_and_bitset_size = 0;
  }

  /** Initializing [[PrimitiveValue]] properties of built-in prototype objects */
  switch (obj_builtin_id)
  {
#if ENABLED (JERRY_BUILTIN_ARRAY)
    case ECMA_BUILTIN_ID_ARRAY_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_ARRAY);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.array.length = 0;
      ext_object_p->u.array.u.length_prop = ECMA_PROPERTY_FLAG_WRITABLE | ECMA_PROPERTY_TYPE_VIRTUAL;
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_ARRAY) */

#if ENABLED (JERRY_BUILTIN_STRING)
    case ECMA_BUILTIN_ID_STRING_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_CLASS);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_STRING_UL;
      ext_object_p->u.class_prop.u.value = ecma_make_magic_string_value (LIT_MAGIC_STRING__EMPTY);
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_STRING) */

#if ENABLED (JERRY_BUILTIN_NUMBER)
    case ECMA_BUILTIN_ID_NUMBER_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_CLASS);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_NUMBER_UL;
      ext_object_p->u.class_prop.u.value = ecma_make_integer_value (0);
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_NUMBER) */

#if ENABLED (JERRY_BUILTIN_BOOLEAN)
    case ECMA_BUILTIN_ID_BOOLEAN_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_CLASS);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_BOOLEAN_UL;
      ext_object_p->u.class_prop.u.value = ECMA_VALUE_FALSE;
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_BOOLEAN) */

#if !ENABLED (JERRY_ESNEXT)
#if ENABLED (JERRY_BUILTIN_DATE)
    case ECMA_BUILTIN_ID_DATE_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_CLASS);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_DATE_UL;

      ecma_number_t *prim_prop_num_value_p = ecma_alloc_number ();
      *prim_prop_num_value_p = ecma_number_make_nan ();
      ECMA_SET_INTERNAL_VALUE_POINTER (ext_object_p->u.class_prop.u.value, prim_prop_num_value_p);
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_DATE) */

#if ENABLED (JERRY_BUILTIN_REGEXP)
    case ECMA_BUILTIN_ID_REGEXP_PROTOTYPE:
    {
      JERRY_ASSERT (obj_type == ECMA_OBJECT_TYPE_CLASS);
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) obj_p;

      ext_object_p->u.class_prop.class_id = LIT_MAGIC_STRING_REGEXP_UL;

      re_compiled_code_t *bc_p = re_compile_bytecode (ecma_get_magic_string (LIT_MAGIC_STRING_EMPTY_NON_CAPTURE_GROUP),
                                                      RE_FLAG_EMPTY);

      JERRY_ASSERT (bc_p != NULL);

      ECMA_SET_INTERNAL_VALUE_POINTER (ext_object_p->u.class_prop.u.value, bc_p);

      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
#endif /* !ENABLED (JERRY_ESNEXT) */
    default:
    {
      JERRY_ASSERT (obj_type != ECMA_OBJECT_TYPE_CLASS);
      break;
    }
  }

  ECMA_SET_NON_NULL_POINTER (JERRY_CONTEXT (ecma_builtin_objects)[obj_builtin_id], obj_p);
} /* ecma_instantiate_builtin */

/**
 * Finalize ECMA built-in objects
 */
void
ecma_finalize_builtins (void)
{
  for (ecma_builtin_id_t id = (ecma_builtin_id_t) 0;
       id < ECMA_BUILTIN_ID__COUNT;
       id = (ecma_builtin_id_t) (id + 1))
  {
    if (JERRY_CONTEXT (ecma_builtin_objects)[id] != JMEM_CP_NULL)
    {
      ecma_object_t *obj_p = ECMA_GET_NON_NULL_POINTER (ecma_object_t, JERRY_CONTEXT (ecma_builtin_objects)[id]);
      ecma_deref_object (obj_p);

#if ENABLED (JERRY_ESNEXT)
      /* Note: In ES2015 a function object may contain tagged template literal collection. Whenever
         this function is assigned to a builtin function or function routine during the GC it may cause unresolvable
         circle since one part of the circle is a weak reference (marked by GC) and the other part is hard reference
         (reference count). In this case when the function which contains the tagged template literal collection
         is getting GC marked the arrays in the collection are still holding weak references to properties/prototypes
         which prevents these objects from getting freed. Releasing the property list and the prototype reference
         manually eliminates the existence of the unresolvable circle described above. */
      ecma_gc_free_properties (obj_p);
      obj_p->u1.property_list_cp = JMEM_CP_NULL;
      obj_p->u2.prototype_cp = JMEM_CP_NULL;
#endif /* ENABLED (JERRY_ESNEXT) */

      JERRY_CONTEXT (ecma_builtin_objects)[id] = JMEM_CP_NULL;
    }
  }
} /* ecma_finalize_builtins */

/**
 * Construct a Function object for specified built-in routine
 *
 * See also: ECMA-262 v5, 15
 *
 * @return pointer to constructed Function object
 */
static ecma_object_t *
ecma_builtin_make_function_object_for_routine (ecma_builtin_id_t builtin_id, /**< identifier of built-in object */
                                               uint16_t routine_id, /**< builtin-wide identifier of the built-in
                                                                     *   object's routine property */
                                               uint16_t name_id, /**< magic string id of 'name' property */
                                               uint16_t flags, /**< see also: ecma_builtin_routine_flags */
                                               uint8_t length_prop_value) /**< value of 'length' property */
{
  JERRY_ASSERT (length_prop_value < (1 << ECMA_BUILT_IN_BITSET_SHIFT));

  ecma_object_t *prototype_obj_p = ecma_builtin_get (ECMA_BUILTIN_ID_FUNCTION_PROTOTYPE);

  size_t ext_object_size = sizeof (ecma_extended_object_t);

  ecma_object_t *func_obj_p = ecma_create_object (prototype_obj_p,
                                                  ext_object_size,
                                                  ECMA_OBJECT_TYPE_FUNCTION);

  ecma_set_object_is_builtin (func_obj_p);

  JERRY_ASSERT (routine_id >= ECMA_BUILTIN_ID__COUNT);

  ecma_extended_object_t *ext_func_obj_p = (ecma_extended_object_t *) func_obj_p;
  ext_func_obj_p->u.built_in.id = (uint8_t) builtin_id;
  ext_func_obj_p->u.built_in.routine_id = routine_id;
  ext_func_obj_p->u.built_in.u.builtin_routine.name = name_id;
  ext_func_obj_p->u.built_in.u.builtin_routine.bitset = flags;

  ext_func_obj_p->u.built_in.length_and_bitset_size = length_prop_value;

  return func_obj_p;
} /* ecma_builtin_make_function_object_for_routine */

/**
 * Construct a Function object for specified built-in accessor getter
 *
 * @return pointer to constructed accessor getter Function object
 */
static ecma_object_t *
ecma_builtin_make_function_object_for_getter_accessor (ecma_builtin_id_t builtin_id, /**< id of built-in object */
                                                       uint16_t routine_id, /**< builtin-wide id of the built-in
                                                                            *   object's routine property */
                                                       uint16_t name_id) /**< magic string id of 'name' property */
{
  return ecma_builtin_make_function_object_for_routine (builtin_id,
                                                        routine_id,
                                                        name_id,
                                                        ECMA_BUILTIN_ROUTINE_GETTER,
                                                        0);
} /* ecma_builtin_make_function_object_for_getter_accessor */

/**
 * Construct a Function object for specified built-in accessor setter
 *
 * @return pointer to constructed accessor getter Function object
 */
static ecma_object_t *
ecma_builtin_make_function_object_for_setter_accessor (ecma_builtin_id_t builtin_id, /**< id of built-in object */
                                                       uint16_t routine_id, /**< builtin-wide id of the built-in
                                                                            *   object's routine property */
                                                       uint16_t name_id) /**< magic string id of 'name' property */
{
  return ecma_builtin_make_function_object_for_routine (builtin_id,
                                                        routine_id,
                                                        name_id,
                                                        ECMA_BUILTIN_ROUTINE_SETTER,
                                                        1);
} /* ecma_builtin_make_function_object_for_setter_accessor */

/**
 * Lazy instantiation of builtin routine property of builtin object
 *
 * If the property is not instantiated yet, instantiate the property and
 * return pointer to the instantiated property.
 *
 * @return pointer property, if one was instantiated,
 *         NULL - otherwise.
 */
ecma_property_t *
ecma_builtin_routine_try_to_instantiate_property (ecma_object_t *object_p, /**< object */
                                                  ecma_string_t *string_p) /**< property's name */
{
  JERRY_ASSERT (ecma_get_object_is_builtin (object_p));
  JERRY_ASSERT (ecma_get_object_type (object_p) == ECMA_OBJECT_TYPE_FUNCTION);
  JERRY_ASSERT (ecma_builtin_function_is_routine (object_p));

  if (ecma_string_is_length (string_p))
  {
    /*
     * Lazy instantiation of 'length' property
     */

    ecma_property_t *len_prop_p;
    ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;
#if ENABLED (JERRY_ESNEXT)
    uint16_t *bitset_p = &ext_func_p->u.built_in.u.builtin_routine.bitset;
    if (*bitset_p & ECMA_BUILTIN_ROUTINE_LENGTH_INITIALIZED)
    {
      /* length property was already instantiated */
      return NULL;
    }
    /* We mark that the property was lazily instantiated,
     * as it is configurable and so can be deleted (ECMA-262 v6, 19.2.4.1) */
    *bitset_p |= ECMA_BUILTIN_ROUTINE_LENGTH_INITIALIZED;
    ecma_property_value_t *len_prop_value_p = ecma_create_named_data_property (object_p,
                                                                               string_p,
                                                                               ECMA_PROPERTY_FLAG_CONFIGURABLE,
                                                                               &len_prop_p);
#else /* !ENABLED (JERRY_ESNEXT) */
    /* We don't need to mark that the property was already lazy instantiated,
     * as it is non-configurable and so can't be deleted (ECMA-262 v5, 13.2.5) */
    ecma_property_value_t *len_prop_value_p = ecma_create_named_data_property (object_p,
                                                                               string_p,
                                                                               ECMA_PROPERTY_FIXED,
                                                                               &len_prop_p);
#endif /* ENABLED (JERRY_ESNEXT) */

    uint8_t length = ext_func_p->u.built_in.length_and_bitset_size;
    JERRY_ASSERT (length < (1 << ECMA_BUILT_IN_BITSET_SHIFT));

    len_prop_value_p->value = ecma_make_integer_value (length);

    return len_prop_p;
  }

#if ENABLED (JERRY_ESNEXT)
  /*
   * Lazy instantiation of 'name' property
   */
  if (ecma_compare_ecma_string_to_magic_id (string_p, LIT_MAGIC_STRING_NAME))
  {
    ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;
    uint16_t *bitset_p = &ext_func_p->u.built_in.u.builtin_routine.bitset;

    if (*bitset_p & ECMA_BUILTIN_ROUTINE_NAME_INITIALIZED)
    {
      /* name property was already instantiated */
      return NULL;
    }

    /* We mark that the property was lazily instantiated */
    *bitset_p |= ECMA_BUILTIN_ROUTINE_NAME_INITIALIZED;
    ecma_property_t *name_prop_p;
    ecma_property_value_t *name_prop_value_p = ecma_create_named_data_property (object_p,
                                                                                string_p,
                                                                                ECMA_PROPERTY_FLAG_CONFIGURABLE,
                                                                                &name_prop_p);

    ecma_string_t *name_p;
    lit_magic_string_id_t name_id = ext_func_p->u.built_in.u.builtin_routine.name;

    if (JERRY_UNLIKELY (name_id > LIT_NON_INTERNAL_MAGIC_STRING__COUNT))
    {
      /* Note: Whenever new intrinsic routine is being added this mapping should be updated as well! */
      if (JERRY_UNLIKELY (name_id == LIT_INTERNAL_MAGIC_STRING_ARRAY_PROTOTYPE_VALUES))
      {
        name_p = ecma_get_magic_string (LIT_MAGIC_STRING_VALUES);
      }
      else if (JERRY_UNLIKELY (name_id == LIT_INTERNAL_MAGIC_STRING_SET_PROTOTYPE_KEYS))
      {
        name_p = ecma_get_magic_string (LIT_MAGIC_STRING_VALUES);
      }
      else
      {
        JERRY_ASSERT (LIT_IS_GLOBAL_SYMBOL (name_id));
        name_p = ecma_op_get_global_symbol (name_id);
      }
    }
    else
    {
      name_p = ecma_get_magic_string (name_id);
    }

    char *prefix_p = NULL;
    lit_utf8_size_t prefix_size = 0;

    if (*bitset_p & (ECMA_BUILTIN_ROUTINE_GETTER | ECMA_BUILTIN_ROUTINE_SETTER))
    {
      prefix_size = 4;
      prefix_p = (*bitset_p & ECMA_BUILTIN_ROUTINE_GETTER) ? "get " : "set ";
    }

    name_prop_value_p->value = ecma_op_function_form_name (name_p, prefix_p, prefix_size);

    if (JERRY_UNLIKELY (name_id > LIT_NON_INTERNAL_MAGIC_STRING__COUNT))
    {
      ecma_deref_ecma_string (name_p);
    }

    return name_prop_p;
  }
#endif /* ENABLED (JERRY_ESNEXT) */

  return NULL;
} /* ecma_builtin_routine_try_to_instantiate_property */

/**
 * If the property's name is one of built-in properties of the object
 * that is not instantiated yet, instantiate the property and
 * return pointer to the instantiated property.
 *
 * @return pointer property, if one was instantiated,
 *         NULL - otherwise.
 */
ecma_property_t *
ecma_builtin_try_to_instantiate_property (ecma_object_t *object_p, /**< object */
                                          ecma_string_t *string_p) /**< property's name */
{
  JERRY_ASSERT (ecma_get_object_is_builtin (object_p));

  lit_magic_string_id_t magic_string_id = ecma_get_string_magic (string_p);

#if ENABLED (JERRY_ESNEXT)
  if (JERRY_UNLIKELY (ecma_prop_name_is_symbol (string_p)))
  {
    if (string_p->u.hash & ECMA_GLOBAL_SYMBOL_FLAG)
    {
      magic_string_id = (string_p->u.hash >> ECMA_GLOBAL_SYMBOL_SHIFT);
    }
  }
#endif /* ENABLED (JERRY_ESNEXT) */

  if (magic_string_id == LIT_MAGIC_STRING__COUNT)
  {
    return NULL;
  }

  ecma_built_in_props_t *built_in_props_p;
  ecma_object_type_t object_type = ecma_get_object_type (object_p);
  JERRY_ASSERT (object_type != ECMA_OBJECT_TYPE_FUNCTION || !ecma_builtin_function_is_routine (object_p));

  if (object_type == ECMA_OBJECT_TYPE_CLASS || object_type == ECMA_OBJECT_TYPE_ARRAY)
  {
    built_in_props_p = &((ecma_extended_built_in_object_t *) object_p)->built_in;
  }
  else
  {
    built_in_props_p = &((ecma_extended_object_t *) object_p)->u.built_in;
  }

  ecma_builtin_id_t builtin_id = (ecma_builtin_id_t) built_in_props_p->id;

  JERRY_ASSERT (builtin_id < ECMA_BUILTIN_ID__COUNT);
  JERRY_ASSERT (ecma_builtin_is (object_p, builtin_id));

  const ecma_builtin_property_descriptor_t *property_list_p = ecma_builtin_property_list_references[builtin_id];

  const ecma_builtin_property_descriptor_t *curr_property_p = property_list_p;

  while (curr_property_p->magic_string_id != magic_string_id)
  {
    if (curr_property_p->magic_string_id == LIT_MAGIC_STRING__COUNT)
    {
      return NULL;
    }
    curr_property_p++;
  }

  uint32_t index = (uint32_t) (curr_property_p - property_list_p);

  uint32_t *bitset_p = built_in_props_p->u.instantiated_bitset + (index >> 5);

  uint32_t bit_for_index = (uint32_t) (1u << (index & 0x1f));

  if (*bitset_p & bit_for_index)
  {
    /* This property was instantiated before. */
    return NULL;
  }

  *bitset_p |= bit_for_index;

  ecma_value_t value = ECMA_VALUE_EMPTY;
  bool is_accessor = false;
  ecma_object_t *getter_p = NULL;
  ecma_object_t *setter_p = NULL;

  switch (curr_property_p->type)
  {
    case ECMA_BUILTIN_PROPERTY_SIMPLE:
    {
      value = curr_property_p->value;
      break;
    }
    case ECMA_BUILTIN_PROPERTY_NUMBER:
    {
      ecma_number_t num = 0.0;

      if (curr_property_p->value < ECMA_BUILTIN_NUMBER_MAX)
      {
        num = curr_property_p->value;
      }
      else if (curr_property_p->value < ECMA_BUILTIN_NUMBER_NAN)
      {
        static const ecma_number_t builtin_number_list[] =
        {
          ECMA_NUMBER_MAX_VALUE,
          ECMA_NUMBER_MIN_VALUE,
#if ENABLED (JERRY_ESNEXT)
          ECMA_NUMBER_EPSILON,
          ECMA_NUMBER_MAX_SAFE_INTEGER,
          ECMA_NUMBER_MIN_SAFE_INTEGER,
#endif /* ENABLED (JERRY_ESNEXT) */
          ECMA_NUMBER_E,
          ECMA_NUMBER_PI,
          ECMA_NUMBER_LN10,
          ECMA_NUMBER_LN2,
          ECMA_NUMBER_LOG2E,
          ECMA_NUMBER_LOG10E,
          ECMA_NUMBER_SQRT2,
          ECMA_NUMBER_SQRT_1_2,
        };

        num = builtin_number_list[curr_property_p->value - ECMA_BUILTIN_NUMBER_MAX];
      }
      else
      {
        switch (curr_property_p->value)
        {
          case ECMA_BUILTIN_NUMBER_POSITIVE_INFINITY:
          {
            num = ecma_number_make_infinity (false);
            break;
          }
          case ECMA_BUILTIN_NUMBER_NEGATIVE_INFINITY:
          {
            num = ecma_number_make_infinity (true);
            break;
          }
          default:
          {
            JERRY_ASSERT (curr_property_p->value == ECMA_BUILTIN_NUMBER_NAN);

            num = ecma_number_make_nan ();
            break;
          }
        }
      }

      value = ecma_make_number_value (num);
      break;
    }
    case ECMA_BUILTIN_PROPERTY_STRING:
    {
      value = ecma_make_magic_string_value ((lit_magic_string_id_t) curr_property_p->value);
      break;
    }
#if ENABLED (JERRY_ESNEXT)
    case ECMA_BUILTIN_PROPERTY_SYMBOL:
    {
      ecma_stringbuilder_t builder = ecma_stringbuilder_create_raw ((lit_utf8_byte_t *) "Symbol.", 7);

      lit_magic_string_id_t symbol_desc_id = (lit_magic_string_id_t) curr_property_p->value;

      ecma_stringbuilder_append_magic (&builder, symbol_desc_id);

      ecma_value_t symbol_desc_value = ecma_make_string_value (ecma_stringbuilder_finalize (&builder));

      ecma_string_t *symbol_p = ecma_new_symbol_from_descriptor_string (symbol_desc_value);
      lit_magic_string_id_t symbol_id = (lit_magic_string_id_t) curr_property_p->magic_string_id;
      symbol_p->u.hash = (uint16_t) ((symbol_id << ECMA_GLOBAL_SYMBOL_SHIFT) | ECMA_GLOBAL_SYMBOL_FLAG);

      value = ecma_make_symbol_value (symbol_p);
      break;
    }
    case ECMA_BUILTIN_PROPERTY_INTRINSIC_PROPERTY:
    {
      value = ecma_op_object_get_by_magic_id (ecma_builtin_get (ECMA_BUILTIN_ID_INTRINSIC_OBJECT),
                                              (lit_magic_string_id_t) curr_property_p->value);
      break;
    }
    case ECMA_BUILTIN_PROPERTY_ACCESSOR_BUILTIN_FUNCTION:
    {
      is_accessor = true;
      uint16_t getter_id = ECMA_ACCESSOR_READ_WRITE_GET_GETTER_ID (curr_property_p->value);
      uint16_t setter_id = ECMA_ACCESSOR_READ_WRITE_GET_SETTER_ID (curr_property_p->value);
      getter_p = ecma_builtin_get (getter_id);
      setter_p = ecma_builtin_get (setter_id);
      ecma_ref_object (getter_p);
      ecma_ref_object (setter_p);
      break;
    }
#endif /* ENABLED (JERRY_ESNEXT) */
    case ECMA_BUILTIN_PROPERTY_OBJECT:
    {
      ecma_object_t *builtin_object_p = ecma_builtin_get ((ecma_builtin_id_t) curr_property_p->value);
      ecma_ref_object (builtin_object_p);
      value = ecma_make_object_value (builtin_object_p);
      break;
    }
    case ECMA_BUILTIN_PROPERTY_ROUTINE:
    {
      ecma_object_t *func_obj_p;
      func_obj_p = ecma_builtin_make_function_object_for_routine (builtin_id,
                                                                  ECMA_GET_ROUTINE_ID (curr_property_p->value),
                                                                  curr_property_p->magic_string_id,
                                                                  ECMA_BUILTIN_ROUTINE_NO_OPTS,
                                                                  ECMA_GET_ROUTINE_LENGTH (curr_property_p->value));
      value = ecma_make_object_value (func_obj_p);
      break;
    }
    case ECMA_BUILTIN_PROPERTY_ACCESSOR_READ_WRITE:
    {
      is_accessor = true;
      uint16_t getter_id = ECMA_ACCESSOR_READ_WRITE_GET_GETTER_ID (curr_property_p->value);
      uint16_t setter_id = ECMA_ACCESSOR_READ_WRITE_GET_SETTER_ID (curr_property_p->value);
      getter_p = ecma_builtin_make_function_object_for_getter_accessor (builtin_id,
                                                                        getter_id,
                                                                        curr_property_p->magic_string_id);
      setter_p = ecma_builtin_make_function_object_for_setter_accessor (builtin_id,
                                                                        setter_id,
                                                                        curr_property_p->magic_string_id);
      break;
    }
    default:
    {
      JERRY_ASSERT (curr_property_p->type == ECMA_BUILTIN_PROPERTY_ACCESSOR_READ_ONLY);

      is_accessor = true;
      getter_p = ecma_builtin_make_function_object_for_getter_accessor (builtin_id,
                                                                        curr_property_p->value,
                                                                        curr_property_p->magic_string_id);
      break;
    }
  }

  ecma_property_t *prop_p;

  if (is_accessor)
  {
    ecma_create_named_accessor_property (object_p,
                                         string_p,
                                         getter_p,
                                         setter_p,
                                         curr_property_p->attributes,
                                         &prop_p);

    if (setter_p)
    {
      ecma_deref_object (setter_p);
    }
    if (getter_p)
    {
      ecma_deref_object (getter_p);
    }
  }
  else
  {
    ecma_property_value_t *prop_value_p = ecma_create_named_data_property (object_p,
                                                                           string_p,
                                                                           curr_property_p->attributes,
                                                                           &prop_p);
    prop_value_p->value = value;

    /* Reference count of objects must be decreased. */
    if (ecma_is_value_object (value))
    {
      ecma_free_value (value);
    }
  }

  return prop_p;
} /* ecma_builtin_try_to_instantiate_property */

/**
 * List names of a built-in function's lazy instantiated properties
 *
 * See also:
 *          ecma_builtin_routine_try_to_instantiate_property
 */
void
ecma_builtin_routine_list_lazy_property_names (ecma_object_t *object_p, /**< a built-in object */
                                               uint32_t opts, /**< listing options using flags
                                                               *   from ecma_list_properties_options_t */
                                               ecma_collection_t *main_collection_p, /**< 'main' collection */
                                               ecma_collection_t *non_enum_collection_p) /**< skipped 'non-enumerable'
                                                                                          *   collection */
{
  JERRY_ASSERT (ecma_get_object_is_builtin (object_p));
  JERRY_ASSERT (ecma_get_object_type (object_p) == ECMA_OBJECT_TYPE_FUNCTION);
  JERRY_ASSERT (ecma_builtin_function_is_routine (object_p));

  const bool separate_enumerable = (opts & ECMA_LIST_ENUMERABLE) != 0;
  const bool is_array_indices_only = (opts & ECMA_LIST_ARRAY_INDICES) != 0;

  ecma_collection_t *for_enumerable_p = main_collection_p;
  JERRY_UNUSED (for_enumerable_p);

  ecma_collection_t *for_non_enumerable_p = separate_enumerable ? non_enum_collection_p : main_collection_p;

  if (!is_array_indices_only)
  {
#if ENABLED (JERRY_ESNEXT)
    ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;
    if (!(ext_func_p->u.built_in.u.builtin_routine.bitset & ECMA_BUILTIN_ROUTINE_LENGTH_INITIALIZED))
    {
      /* Unintialized 'length' property is non-enumerable (ECMA-262 v6, 19.2.4.1) */
      ecma_collection_push_back (for_non_enumerable_p, ecma_make_magic_string_value (LIT_MAGIC_STRING_LENGTH));
    }
    if (!(ext_func_p->u.built_in.u.builtin_routine.bitset & ECMA_BUILTIN_ROUTINE_NAME_INITIALIZED))
    {
      /* Unintialized 'name' property is non-enumerable (ECMA-262 v6, 19.2.4.2) */
      ecma_collection_push_back (for_non_enumerable_p, ecma_make_magic_string_value (LIT_MAGIC_STRING_NAME));
    }
#else /* !ENABLED (JERRY_ESNEXT) */
    /* 'length' property is non-enumerable (ECMA-262 v5, 15) */
    ecma_collection_push_back (for_non_enumerable_p, ecma_make_magic_string_value (LIT_MAGIC_STRING_LENGTH));
#endif /* ENABLED (JERRY_ESNEXT) */
  }
} /* ecma_builtin_routine_list_lazy_property_names */

/**
 * List names of a built-in object's lazy instantiated properties
 *
 * See also:
 *          ecma_builtin_try_to_instantiate_property
 */
void
ecma_builtin_list_lazy_property_names (ecma_object_t *object_p, /**< a built-in object */
                                       uint32_t opts, /**< listing options using flags
                                                       *   from ecma_list_properties_options_t */
                                       ecma_collection_t *main_collection_p, /**< 'main' collection */
                                       ecma_collection_t *non_enum_collection_p) /**< skipped 'non-enumerable'
                                                                                  *   collection */
{
  JERRY_ASSERT (ecma_get_object_is_builtin (object_p));
  JERRY_ASSERT (ecma_get_object_type (object_p) != ECMA_OBJECT_TYPE_FUNCTION
                || !ecma_builtin_function_is_routine (object_p));

  const bool separate_enumerable = (opts & ECMA_LIST_ENUMERABLE) != 0;
  const bool is_array_indices_only = (opts & ECMA_LIST_ARRAY_INDICES) != 0;

  ecma_built_in_props_t *built_in_props_p;
  ecma_object_type_t object_type = ecma_get_object_type (object_p);

  if (object_type == ECMA_OBJECT_TYPE_CLASS || object_type == ECMA_OBJECT_TYPE_ARRAY)
  {
    built_in_props_p = &((ecma_extended_built_in_object_t *) object_p)->built_in;
  }
  else
  {
    built_in_props_p = &((ecma_extended_object_t *) object_p)->u.built_in;
  }

  ecma_builtin_id_t builtin_id = (ecma_builtin_id_t) built_in_props_p->id;

  JERRY_ASSERT (builtin_id < ECMA_BUILTIN_ID__COUNT);
  JERRY_ASSERT (ecma_builtin_is (object_p, builtin_id));

  const ecma_builtin_property_descriptor_t *curr_property_p = ecma_builtin_property_list_references[builtin_id];

  uint32_t index = 0;
  uint32_t *bitset_p = built_in_props_p->u.instantiated_bitset;

  ecma_collection_t *for_non_enumerable_p = (separate_enumerable ? non_enum_collection_p
                                                                 : main_collection_p);

  while (curr_property_p->magic_string_id != LIT_MAGIC_STRING__COUNT)
  {
    if (index == 32)
    {
      bitset_p++;
      index = 0;
    }

#if ENABLED (JERRY_ESNEXT)
    /* Builtin symbol properties are internal magic strings which must not be listed */
    if (curr_property_p->magic_string_id > LIT_NON_INTERNAL_MAGIC_STRING__COUNT)
    {
      curr_property_p++;
      continue;
    }
#endif /* ENABLED (JERRY_ESNEXT) */

    ecma_string_t *name_p = ecma_get_magic_string ((lit_magic_string_id_t) curr_property_p->magic_string_id);

    if (is_array_indices_only && ecma_string_get_array_index (name_p) == ECMA_STRING_NOT_ARRAY_INDEX)
    {
      curr_property_p++;
      continue;
    }

    uint32_t bit_for_index = (uint32_t) 1u << index;

    if (!(*bitset_p & bit_for_index) || ecma_op_ordinary_object_has_own_property (object_p, name_p))
    {
      ecma_value_t name = ecma_make_magic_string_value ((lit_magic_string_id_t) curr_property_p->magic_string_id);

#if ENABLED (JERRY_ESNEXT)
      if (curr_property_p->attributes & ECMA_PROPERTY_FLAG_ENUMERABLE)
      {
        ecma_collection_push_back (main_collection_p, name);
      }
      else
#endif /* ENABLED (JERRY_ESNEXT) */
      {
        ecma_collection_push_back (for_non_enumerable_p, name);
      }
    }

    curr_property_p++;
    index++;
  }
} /* ecma_builtin_list_lazy_property_names */

/**
 * Dispatcher of built-in routines
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_dispatch_routine (ecma_extended_object_t *func_obj_p, /**< builtin object */
                               ecma_value_t this_arg_value, /**< 'this' argument value */
                               const ecma_value_t *arguments_list_p, /**< list of arguments passed to routine */
                               uint32_t arguments_list_len) /**< length of arguments' list */
{
  JERRY_ASSERT (ecma_builtin_function_is_routine ((ecma_object_t *) func_obj_p));

  ecma_value_t padded_arguments_list_p[3] = { ECMA_VALUE_UNDEFINED, ECMA_VALUE_UNDEFINED, ECMA_VALUE_UNDEFINED };

  if (arguments_list_len <= 2)
  {
    switch (arguments_list_len)
    {
      case 2:
      {
        padded_arguments_list_p[1] = arguments_list_p[1];
        /* FALLTHRU */
      }
      case 1:
      {
        padded_arguments_list_p[0] = arguments_list_p[0];
        break;
      }
      default:
      {
        JERRY_ASSERT (arguments_list_len == 0);
      }
    }

    arguments_list_p = padded_arguments_list_p;
  }

  return ecma_builtin_routines[func_obj_p->u.built_in.id] (func_obj_p->u.built_in.routine_id,
                                                           this_arg_value,
                                                           arguments_list_p,
                                                           arguments_list_len);
} /* ecma_builtin_dispatch_routine */

/**
 * Handle calling [[Call]] of built-in object
 *
 * @return ecma value
 */
ecma_value_t
ecma_builtin_dispatch_call (ecma_object_t *obj_p, /**< built-in object */
                            ecma_value_t this_arg_value, /**< 'this' argument value */
                            const ecma_value_t *arguments_list_p, /**< arguments list */
                            uint32_t arguments_list_len) /**< arguments list length */
{
  JERRY_ASSERT (ecma_get_object_type (obj_p) == ECMA_OBJECT_TYPE_FUNCTION);
  JERRY_ASSERT (ecma_get_object_is_builtin (obj_p));

  ecma_extended_object_t *ext_obj_p = (ecma_extended_object_t *) obj_p;

  if (ecma_builtin_function_is_routine (obj_p))
  {
    return ecma_builtin_dispatch_routine (ext_obj_p,
                                          this_arg_value,
                                          arguments_list_p,
                                          arguments_list_len);
  }

  ecma_builtin_id_t builtin_object_id = ext_obj_p->u.built_in.id;
  JERRY_ASSERT (builtin_object_id < sizeof (ecma_builtin_call_functions) / sizeof (ecma_builtin_dispatch_call_t));
  return ecma_builtin_call_functions[builtin_object_id] (arguments_list_p, arguments_list_len);
} /* ecma_builtin_dispatch_call */

/**
 * Handle calling [[Construct]] of built-in object
 *
 * @return ecma value
 */
ecma_value_t
ecma_builtin_dispatch_construct (ecma_object_t *obj_p, /**< built-in object */
                                 ecma_object_t *new_target_p, /**< new target */
                                 const ecma_value_t *arguments_list_p, /**< arguments list */
                                 uint32_t arguments_list_len) /**< arguments list length */
{
  JERRY_ASSERT (ecma_get_object_type (obj_p) == ECMA_OBJECT_TYPE_FUNCTION);
  JERRY_ASSERT (ecma_get_object_is_builtin (obj_p));

  if (ecma_builtin_function_is_routine (obj_p))
  {
    return ecma_raise_type_error (ECMA_ERR_MSG ("Built-in routines have no constructor."));
  }

  ecma_extended_object_t *ext_obj_p = (ecma_extended_object_t *) obj_p;
  ecma_builtin_id_t builtin_object_id = ext_obj_p->u.built_in.id;
  JERRY_ASSERT (builtin_object_id < sizeof (ecma_builtin_construct_functions) / sizeof (ecma_builtin_dispatch_call_t));

#if ENABLED (JERRY_ESNEXT)
  ecma_object_t *old_new_target = JERRY_CONTEXT (current_new_target);
  JERRY_CONTEXT (current_new_target) = new_target_p;
#else /* !ENABLED (JERRY_ESNEXT) */
  JERRY_UNUSED (new_target_p);
#endif /* ENABLED (JERRY_ESNEXT) */

  ecma_value_t ret_value = ecma_builtin_construct_functions[builtin_object_id] (arguments_list_p, arguments_list_len);

#if ENABLED (JERRY_ESNEXT)
  JERRY_CONTEXT (current_new_target) = old_new_target;
#endif /* ENABLED (JERRY_ESNEXT) */

  return ret_value;
} /* ecma_builtin_dispatch_construct */

/**
 * @}
 * @}
 */
