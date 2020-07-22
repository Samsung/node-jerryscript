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

/**
 * Garbage collector implementation
 */

#include "ecma-alloc.h"
#include "ecma-array-object.h"
#include "ecma-container-object.h"
#include "ecma-function-object.h"
#include "ecma-globals.h"
#include "ecma-gc.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"
#include "ecma-property-hashmap.h"
#include "ecma-proxy-object.h"
#include "jcontext.h"
#include "jrt.h"
#include "jrt-libc-includes.h"
#include "jrt-bit-fields.h"
#include "re-compiler.h"
#include "vm-defines.h"
#include "vm-stack.h"

#if ENABLED (JERRY_BUILTIN_TYPEDARRAY)
#include "ecma-typedarray-object.h"
#endif /* ENABLED (JERRY_BUILTIN_TYPEDARRAY) */
#if ENABLED (JERRY_BUILTIN_PROMISE)
#include "ecma-promise-object.h"
#endif /* ENABLED (JERRY_BUILTIN_PROMISE) */

/* TODO: Extract GC to a separate component */

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmagc Garbage collector
 * @{
 */

/*
 * The garbage collector uses the reference counter
 * of object: it increases the counter by one when
 * the object is marked at the first time.
 */

/**
 * Get visited flag of the object.
 *
 * @return true  - if visited
 *         false - otherwise
 */
static inline bool JERRY_ATTR_ALWAYS_INLINE
ecma_gc_is_object_visited (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  return (object_p->type_flags_refs < ECMA_OBJECT_NON_VISITED);
} /* ecma_gc_is_object_visited */

/**
 * Mark objects as visited starting from specified object as root
 */
static void ecma_gc_mark (ecma_object_t *object_p);

/**
 * Set visited flag of the object.
 */
static void
ecma_gc_set_object_visited (ecma_object_t *object_p) /**< object */
{
  if (object_p->type_flags_refs >= ECMA_OBJECT_NON_VISITED)
  {
#if (JERRY_GC_MARK_LIMIT != 0)
    if (JERRY_CONTEXT (ecma_gc_mark_recursion_limit) != 0)
    {
      JERRY_CONTEXT (ecma_gc_mark_recursion_limit)--;
      /* Set the reference count of gray object to 0 */
      object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs & (ECMA_OBJECT_REF_ONE - 1));
      ecma_gc_mark (object_p);
      JERRY_CONTEXT (ecma_gc_mark_recursion_limit)++;
    }
    else
    {
      /* Set the reference count of the non-marked gray object to 1 */
      object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs & ((ECMA_OBJECT_REF_ONE << 1) - 1));
      JERRY_ASSERT (object_p->type_flags_refs >= ECMA_OBJECT_REF_ONE);
    }
#else /* (JERRY_GC_MARK_LIMIT == 0) */
    /* Set the reference count of gray object to 0 */
    object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs & (ECMA_OBJECT_REF_ONE - 1));
#endif /* (JERRY_GC_MARK_LIMIT != 0) */
  }
} /* ecma_gc_set_object_visited */

/**
 * Initialize GC information for the object
 */
inline void
ecma_init_gc_info (ecma_object_t *object_p) /**< object */
{
  JERRY_CONTEXT (ecma_gc_objects_number)++;
  JERRY_CONTEXT (ecma_gc_new_objects)++;

  JERRY_ASSERT (object_p->type_flags_refs < ECMA_OBJECT_REF_ONE);
  object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs | ECMA_OBJECT_REF_ONE);

  object_p->gc_next_cp = JERRY_CONTEXT (ecma_gc_objects_cp);
  ECMA_SET_NON_NULL_POINTER (JERRY_CONTEXT (ecma_gc_objects_cp), object_p);
} /* ecma_init_gc_info */

/**
 * Increase reference counter of an object
 */
void
ecma_ref_object (ecma_object_t *object_p) /**< object */
{
  if (JERRY_LIKELY (object_p->type_flags_refs < ECMA_OBJECT_MAX_REF))
  {
    object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs + ECMA_OBJECT_REF_ONE);
  }
  else
  {
    jerry_fatal (ERR_REF_COUNT_LIMIT);
  }
} /* ecma_ref_object */

/**
 * Decrease reference counter of an object
 */
inline void JERRY_ATTR_ALWAYS_INLINE
ecma_deref_object (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p->type_flags_refs >= ECMA_OBJECT_REF_ONE);
  object_p->type_flags_refs = (uint16_t) (object_p->type_flags_refs - ECMA_OBJECT_REF_ONE);
} /* ecma_deref_object */

/**
 * Mark referenced object from property
 */
static inline void JERRY_ATTR_ALWAYS_INLINE
ecma_gc_mark_properties (ecma_property_pair_t *property_pair_p) /**< property pair */
{
  for (uint32_t index = 0; index < ECMA_PROPERTY_PAIR_ITEM_COUNT; index++)
  {
    uint8_t property = property_pair_p->header.types[index];

    switch (ECMA_PROPERTY_GET_TYPE (property))
    {
      case ECMA_PROPERTY_TYPE_NAMEDDATA:
      {
        ecma_value_t value = property_pair_p->values[index].value;

        if (ecma_is_value_object (value))
        {
          ecma_object_t *value_obj_p = ecma_get_object_from_value (value);

          ecma_gc_set_object_visited (value_obj_p);
        }
        break;
      }
      case ECMA_PROPERTY_TYPE_NAMEDACCESSOR:
      {
        ecma_property_value_t *accessor_objs_p = property_pair_p->values + index;

        ecma_getter_setter_pointers_t *get_set_pair_p = ecma_get_named_accessor_property (accessor_objs_p);

        if (get_set_pair_p->getter_cp != JMEM_CP_NULL)
        {
          ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER (ecma_object_t, get_set_pair_p->getter_cp));
        }

        if (get_set_pair_p->setter_cp != JMEM_CP_NULL)
        {
          ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER (ecma_object_t, get_set_pair_p->setter_cp));
        }
        break;
      }
      case ECMA_PROPERTY_TYPE_INTERNAL:
      {
        JERRY_ASSERT (ECMA_PROPERTY_GET_NAME_TYPE (property) == ECMA_DIRECT_STRING_MAGIC
                      && property_pair_p->names_cp[index] >= LIT_FIRST_INTERNAL_MAGIC_STRING);
        break;
      }
      default:
      {
        JERRY_ASSERT (ECMA_PROPERTY_GET_TYPE (property) == ECMA_PROPERTY_TYPE_SPECIAL);

        JERRY_ASSERT (property == ECMA_PROPERTY_TYPE_HASHMAP
                      || property == ECMA_PROPERTY_TYPE_DELETED);
        break;
      }
    }
  }
} /* ecma_gc_mark_properties */

/**
 * Mark objects referenced by bound function object.
 */
static void JERRY_ATTR_NOINLINE
ecma_gc_mark_bound_function_object (ecma_object_t *object_p) /**< bound function object */
{
  JERRY_ASSERT (ecma_get_object_type (object_p) == ECMA_OBJECT_TYPE_BOUND_FUNCTION);

  ecma_bound_function_t *bound_func_p = (ecma_bound_function_t *) object_p;

  ecma_object_t *target_func_p;
  target_func_p = ECMA_GET_NON_NULL_POINTER_FROM_POINTER_TAG (ecma_object_t,
                                                              bound_func_p->header.u.bound_function.target_function);

  ecma_gc_set_object_visited (target_func_p);

  ecma_value_t args_len_or_this = bound_func_p->header.u.bound_function.args_len_or_this;

  if (!ecma_is_value_integer_number (args_len_or_this))
  {
    if (ecma_is_value_object (args_len_or_this))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (args_len_or_this));
    }

    return;
  }

  ecma_integer_value_t args_length = ecma_get_integer_from_value (args_len_or_this);
  ecma_value_t *args_p = (ecma_value_t *) (bound_func_p + 1);

  JERRY_ASSERT (args_length > 0);

  for (ecma_integer_value_t i = 0; i < args_length; i++)
  {
    if (ecma_is_value_object (args_p[i]))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (args_p[i]));
    }
  }
} /* ecma_gc_mark_bound_function_object */

#if ENABLED (JERRY_BUILTIN_PROMISE)
/**
 * Mark objects referenced by Promise built-in.
 */
static void
ecma_gc_mark_promise_object (ecma_extended_object_t *ext_object_p) /**< extended object */
{
  /* Mark promise result. */
  ecma_value_t result = ext_object_p->u.class_prop.u.value;

  if (ecma_is_value_object (result))
  {
    ecma_gc_set_object_visited (ecma_get_object_from_value (result));
  }

  /* Mark all reactions. */
  ecma_promise_object_t *promise_object_p = (ecma_promise_object_t *) ext_object_p;
  ecma_collection_t *collection_p = promise_object_p->reactions;

  if (collection_p != NULL)
  {
    ecma_value_t *buffer_p = collection_p->buffer_p;
    ecma_value_t *buffer_end_p = buffer_p + collection_p->item_count;

    while (buffer_p < buffer_end_p)
    {
      ecma_value_t value = *buffer_p++;

      ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER_FROM_POINTER_TAG (ecma_object_t, value));

      if (JMEM_CP_GET_FIRST_BIT_FROM_POINTER_TAG (value))
      {
        ecma_gc_set_object_visited (ecma_get_object_from_value (*buffer_p++));
      }

      if (JMEM_CP_GET_SECOND_BIT_FROM_POINTER_TAG (value))
      {
        ecma_gc_set_object_visited (ecma_get_object_from_value (*buffer_p++));
      }
    }
  }
} /* ecma_gc_mark_promise_object */

#endif /* ENABLED (JERRY_BUILTIN_PROMISE) */

#if ENABLED (JERRY_BUILTIN_MAP)
/**
 * Mark objects referenced by Map built-in.
 */
static void
ecma_gc_mark_map_object (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  ecma_extended_object_t *map_object_p = (ecma_extended_object_t *) object_p;
  ecma_collection_t *container_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t,
                                                                    map_object_p->u.class_prop.u.value);
  ecma_value_t *start_p = ECMA_CONTAINER_START (container_p);
  uint32_t entry_count = ECMA_CONTAINER_ENTRY_COUNT (container_p);

  for (uint32_t i = 0; i < entry_count; i+= ECMA_CONTAINER_PAIR_SIZE)
  {
    ecma_container_pair_t *entry_p = (ecma_container_pair_t *) (start_p + i);

    if (ecma_is_value_empty (entry_p->key))
    {
      continue;
    }

    if (ecma_is_value_object (entry_p->key))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (entry_p->key));
    }

    if (ecma_is_value_object (entry_p->value))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (entry_p->value));
    }
  }
} /* ecma_gc_mark_map_object */
#endif /* ENABLED (JERRY_BUILTIN_MAP) */

#if ENABLED (JERRY_BUILTIN_WEAKMAP)
/**
 * Mark objects referenced by WeakMap built-in.
 */
static void
ecma_gc_mark_weakmap_object (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  ecma_extended_object_t *map_object_p = (ecma_extended_object_t *) object_p;
  ecma_collection_t *container_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t,
                                                                    map_object_p->u.class_prop.u.value);
  ecma_value_t *start_p = ECMA_CONTAINER_START (container_p);
  uint32_t entry_count = ECMA_CONTAINER_ENTRY_COUNT (container_p);

  for (uint32_t i = 0; i < entry_count; i+= ECMA_CONTAINER_PAIR_SIZE)
  {
    ecma_container_pair_t *entry_p = (ecma_container_pair_t *) (start_p + i);

    if (ecma_is_value_empty (entry_p->key))
    {
      continue;
    }

    if (ecma_is_value_object (entry_p->value))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (entry_p->value));
    }
  }
} /* ecma_gc_mark_weakmap_object */
#endif /* ENABLED (JERRY_BUILTIN_WEAKMAP) */

#if ENABLED (JERRY_BUILTIN_SET)
/**
 * Mark objects referenced by Set built-in.
 */
static void
ecma_gc_mark_set_object (ecma_object_t *object_p) /**< object */
{
  JERRY_ASSERT (object_p != NULL);

  ecma_extended_object_t *map_object_p = (ecma_extended_object_t *) object_p;
  ecma_collection_t *container_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t,
                                                                    map_object_p->u.class_prop.u.value);
  ecma_value_t *start_p = ECMA_CONTAINER_START (container_p);
  uint32_t entry_count = ECMA_CONTAINER_ENTRY_COUNT (container_p);

  for (uint32_t i = 0; i < entry_count; i+= ECMA_CONTAINER_VALUE_SIZE)
  {
    ecma_value_t *entry_p = start_p + i;

    if (ecma_is_value_empty (*entry_p))
    {
      continue;
    }

    if (ecma_is_value_object (*entry_p))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (*entry_p));
    }
  }
} /* ecma_gc_mark_set_object */
#endif /* ENABLED (JERRY_BUILTIN_SET) */

#if ENABLED (JERRY_ESNEXT)
/**
 * Mark objects referenced by inactive generator functions, async functions, etc.
 */
static void
ecma_gc_mark_executable_object (ecma_object_t *object_p) /**< object */
{
  vm_executable_object_t *executable_object_p = (vm_executable_object_t *) object_p;

  if (executable_object_p->extended_object.u.class_prop.extra_info & ECMA_ASYNC_GENERATOR_CALLED)
  {
    ecma_value_t task = executable_object_p->extended_object.u.class_prop.u.head;

    while (!ECMA_IS_INTERNAL_VALUE_NULL (task))
    {
      ecma_async_generator_task_t *task_p;
      task_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_async_generator_task_t, task);

      JERRY_ASSERT (ecma_is_value_object (task_p->promise));
      ecma_gc_set_object_visited (ecma_get_object_from_value (task_p->promise));

      if (ecma_is_value_object (task_p->operation_value))
      {
        ecma_gc_set_object_visited (ecma_get_object_from_value (task_p->operation_value));
      }

      task = task_p->next;
    }
  }

  if (!ECMA_EXECUTABLE_OBJECT_IS_SUSPENDED (executable_object_p->extended_object.u.class_prop.extra_info))
  {
    /* All objects referenced by running executable objects are strong roots,
     * and a finished executable object cannot refer to other values. */
    return;
  }

  ecma_gc_set_object_visited (executable_object_p->frame_ctx.lex_env_p);

  if (ecma_is_value_object (executable_object_p->frame_ctx.this_binding))
  {
    ecma_gc_set_object_visited (ecma_get_object_from_value (executable_object_p->frame_ctx.this_binding));
  }

  const ecma_compiled_code_t *bytecode_header_p = executable_object_p->frame_ctx.bytecode_header_p;
  size_t register_end;

  if (bytecode_header_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) bytecode_header_p;
    register_end = args_p->register_end;
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) bytecode_header_p;
    register_end = args_p->register_end;
  }

  ecma_value_t *register_p = VM_GET_REGISTERS (&executable_object_p->frame_ctx);
  ecma_value_t *register_end_p = register_p + register_end;

  while (register_p < register_end_p)
  {
    if (ecma_is_value_object (*register_p))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (*register_p));
    }

    register_p++;
  }

  if (executable_object_p->frame_ctx.context_depth > 0)
  {
    ecma_value_t *context_end_p = register_p;

    register_p += executable_object_p->frame_ctx.context_depth;

    ecma_value_t *context_top_p = register_p;

    do
    {
      uint32_t offsets = vm_get_context_value_offsets (context_top_p);

      while (VM_CONTEXT_HAS_NEXT_OFFSET (offsets))
      {
        int32_t offset = VM_CONTEXT_GET_NEXT_OFFSET (offsets);

        if (ecma_is_value_object (context_top_p[offset]))
        {
          ecma_gc_set_object_visited (ecma_get_object_from_value (context_top_p[offset]));
        }

        offsets >>= VM_CONTEXT_OFFSET_SHIFT;
      }

      JERRY_ASSERT (context_top_p >= context_end_p + offsets);
      context_top_p -= offsets;
    }
    while (context_top_p > context_end_p);
  }

  register_end_p = executable_object_p->frame_ctx.stack_top_p;

  while (register_p < register_end_p)
  {
    if (ecma_is_value_object (*register_p))
    {
      ecma_gc_set_object_visited (ecma_get_object_from_value (*register_p));
    }

    register_p++;
  }

  if (ecma_is_value_object (executable_object_p->frame_ctx.block_result))
  {
    ecma_gc_set_object_visited (ecma_get_object_from_value (executable_object_p->frame_ctx.block_result));
  }
} /* ecma_gc_mark_executable_object */

#endif /* ENABLED (JERRY_ESNEXT) */

#if ENABLED (JERRY_BUILTIN_PROXY)
/**
 * Mark the objects referenced by a proxy object
 */
static void
ecma_gc_mark_proxy_object (ecma_object_t *object_p) /**< proxy object */
{
  JERRY_ASSERT (ECMA_OBJECT_IS_PROXY (object_p));

  ecma_proxy_object_t *proxy_p = (ecma_proxy_object_t *) object_p;

  if (!ecma_is_value_null (proxy_p->target))
  {
    ecma_gc_set_object_visited (ecma_get_object_from_value (proxy_p->target));
  }

  if (!ecma_is_value_null (proxy_p->handler))
  {
    ecma_gc_set_object_visited (ecma_get_object_from_value (proxy_p->handler));
  }
} /* ecma_gc_mark_proxy_object */
#endif /* ENABLED (JERRY_BUILTIN_PROXY) */

/**
 * Mark objects as visited starting from specified object as root
 */
static void
ecma_gc_mark (ecma_object_t *object_p) /**< object to mark from */
{
  JERRY_ASSERT (object_p != NULL);
  JERRY_ASSERT (ecma_gc_is_object_visited (object_p));

  if (ecma_is_lexical_environment (object_p))
  {
    jmem_cpointer_t outer_lex_env_cp = object_p->u2.outer_reference_cp;

    if (outer_lex_env_cp != JMEM_CP_NULL)
    {
      ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER (ecma_object_t, outer_lex_env_cp));
    }

    if (ecma_get_lex_env_type (object_p) != ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
    {
      ecma_object_t *binding_object_p = ecma_get_lex_env_binding_object (object_p);
      ecma_gc_set_object_visited (binding_object_p);

      return;
    }
  }
  else
  {
    jmem_cpointer_t proto_cp = object_p->u2.prototype_cp;

    if (proto_cp != JMEM_CP_NULL)
    {
      ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER (ecma_object_t, proto_cp));
    }

    switch (ecma_get_object_type (object_p))
    {
      case ECMA_OBJECT_TYPE_CLASS:
      {
        ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

        switch (ext_object_p->u.class_prop.class_id)
        {
#if ENABLED (JERRY_BUILTIN_PROMISE)
          case LIT_MAGIC_STRING_PROMISE_UL:
          {
            ecma_gc_mark_promise_object (ext_object_p);
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_PROMISE) */
#if ENABLED (JERRY_BUILTIN_DATAVIEW)
          case LIT_MAGIC_STRING_DATAVIEW_UL:
          {
            ecma_dataview_object_t *dataview_p = (ecma_dataview_object_t *) object_p;
            ecma_gc_set_object_visited (dataview_p->buffer_p);
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_DATAVIEW) */
#if ENABLED (JERRY_BUILTIN_CONTAINER)
#if ENABLED (JERRY_BUILTIN_WEAKSET)
          case LIT_MAGIC_STRING_WEAKSET_UL:
          {
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_WEAKSET) */
#if ENABLED (JERRY_BUILTIN_SET)
          case LIT_MAGIC_STRING_SET_UL:
          {
            ecma_gc_mark_set_object (object_p);
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_SET) */
#if ENABLED (JERRY_BUILTIN_WEAKMAP)
          case LIT_MAGIC_STRING_WEAKMAP_UL:
          {
            ecma_gc_mark_weakmap_object (object_p);
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_WEAKMAP) */
#if ENABLED (JERRY_BUILTIN_MAP)
          case LIT_MAGIC_STRING_MAP_UL:
          {
            ecma_gc_mark_map_object (object_p);
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_MAP) */
#endif /* ENABLED (JERRY_BUILTIN_CONTAINER) */
#if ENABLED (JERRY_ESNEXT)
          case LIT_MAGIC_STRING_GENERATOR_UL:
          case LIT_MAGIC_STRING_ASYNC_GENERATOR_UL:
          {
            ecma_gc_mark_executable_object (object_p);
            break;
          }
          case LIT_INTERNAL_MAGIC_PROMISE_CAPABILITY:
          {
            ecma_promise_capabality_t *capability_p = (ecma_promise_capabality_t *) object_p;

            if (ecma_is_value_object (capability_p->header.u.class_prop.u.promise))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (capability_p->header.u.class_prop.u.promise));
            }
            if (ecma_is_value_object (capability_p->resolve))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (capability_p->resolve));
            }
            if (ecma_is_value_object (capability_p->reject))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (capability_p->reject));
            }
            break;
          }
#endif /* ENABLED (JERRY_ESNEXT) */
          default:
          {
            break;
          }
        }

        break;
      }
      case ECMA_OBJECT_TYPE_PSEUDO_ARRAY:
      {
        ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

        switch (ext_object_p->u.pseudo_array.type)
        {
#if ENABLED (JERRY_BUILTIN_TYPEDARRAY)
          case ECMA_PSEUDO_ARRAY_TYPEDARRAY:
          case ECMA_PSEUDO_ARRAY_TYPEDARRAY_WITH_INFO:
          {
            ecma_gc_set_object_visited (ecma_typedarray_get_arraybuffer (object_p));
            break;
          }
#endif /* ENABLED (JERRY_BUILTIN_TYPEDARRAY) */
#if ENABLED (JERRY_ESNEXT)
          case ECMA_PSEUDO_ARRAY_ITERATOR:
          case ECMA_PSEUDO_SET_ITERATOR:
          case ECMA_PSEUDO_MAP_ITERATOR:
          {
            ecma_value_t iterated_value = ext_object_p->u.pseudo_array.u2.iterated_value;
            if (!ecma_is_value_empty (iterated_value))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (iterated_value));
            }
            break;
          }
          case ECMA_PSEUDO_STRING_ITERATOR:
          {
            break;
          }
#endif /* ENABLED (JERRY_ESNEXT) */
          default:
          {
            JERRY_ASSERT (ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_ARRAY_ARGUMENTS);

            ecma_object_t *lex_env_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_object_t,
                                                                        ext_object_p->u.pseudo_array.u2.lex_env_cp);

            ecma_gc_set_object_visited (lex_env_p);
            break;
          }
        }

        break;
      }
      case ECMA_OBJECT_TYPE_ARRAY:
      {
        ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

        if (ecma_op_array_is_fast_array (ext_object_p))
        {
          if (object_p->u1.property_list_cp != JMEM_CP_NULL)
          {
            ecma_value_t *values_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, object_p->u1.property_list_cp);

            for (uint32_t i = 0; i < ext_object_p->u.array.length; i++)
            {
              if (ecma_is_value_object (values_p[i]))
              {
                ecma_gc_set_object_visited (ecma_get_object_from_value (values_p[i]));
              }
            }
          }

          return;
        }
        break;
      }
#if ENABLED (JERRY_BUILTIN_PROXY)
      case ECMA_OBJECT_TYPE_PROXY:
      {
        ecma_gc_mark_proxy_object (object_p);
        break;
      }
#endif /* ENABLED (JERRY_BUILTIN_PROXY) */
      case ECMA_OBJECT_TYPE_BOUND_FUNCTION:
      {
        ecma_gc_mark_bound_function_object (object_p);
        break;
      }
      case ECMA_OBJECT_TYPE_FUNCTION:
      {
        if (!ecma_get_object_is_builtin (object_p))
        {
          ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;
          ecma_gc_set_object_visited (ECMA_GET_NON_NULL_POINTER_FROM_POINTER_TAG (ecma_object_t,
                                                                                  ext_func_p->u.function.scope_cp));

#if ENABLED (JERRY_ESNEXT)
          const ecma_compiled_code_t *byte_code_p = ecma_op_function_get_compiled_code (ext_func_p);

          if (CBC_FUNCTION_IS_ARROW (byte_code_p->status_flags))
          {
            ecma_arrow_function_t *arrow_func_p = (ecma_arrow_function_t *) object_p;

            if (ecma_is_value_object (arrow_func_p->this_binding))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (arrow_func_p->this_binding));
            }

            if (ecma_is_value_object (arrow_func_p->new_target))
            {
              ecma_gc_set_object_visited (ecma_get_object_from_value (arrow_func_p->new_target));
            }
          }
#endif /* ENABLED (JERRY_ESNEXT) */
        }
        break;
      }
#if ENABLED (JERRY_ESNEXT)
      case ECMA_OBJECT_TYPE_EXTERNAL_FUNCTION:
      {
        ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;

        if (ext_func_p->u.external_handler_cb == ecma_proxy_revoke_cb)
        {
          ecma_revocable_proxy_object_t *rev_proxy_p = (ecma_revocable_proxy_object_t *) object_p;

          if (!ecma_is_value_null (rev_proxy_p->proxy))
          {
            ecma_gc_set_object_visited (ecma_get_object_from_value (rev_proxy_p->proxy));
          }
        }
        else if (ext_func_p->u.external_handler_cb == ecma_op_get_capabilities_executor_cb)
        {
          ecma_promise_capability_executor_t *executor_p = (ecma_promise_capability_executor_t *) object_p;
          ecma_gc_set_object_visited (ecma_get_object_from_value (executor_p->capability));
        }
        else if (ext_func_p->u.external_handler_cb == ecma_promise_all_handler_cb)
        {
          ecma_promise_all_executor_t *executor_p = (ecma_promise_all_executor_t *) object_p;
          ecma_gc_set_object_visited (ecma_get_object_from_value (executor_p->capability));
          ecma_gc_set_object_visited (ecma_get_object_from_value (executor_p->values));
          ecma_gc_set_object_visited (ecma_get_object_from_value (executor_p->remaining_elements));
        }
        break;
      }
#endif /* ENABLED (JERRY_ESNEXT) */
      default:
      {
        break;
      }
    }
  }

  jmem_cpointer_t prop_iter_cp = object_p->u1.property_list_cp;

#if ENABLED (JERRY_PROPRETY_HASHMAP)
  if (prop_iter_cp != JMEM_CP_NULL)
  {
    ecma_property_header_t *prop_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_property_header_t, prop_iter_cp);
    if (prop_iter_p->types[0] == ECMA_PROPERTY_TYPE_HASHMAP)
    {
      prop_iter_cp = prop_iter_p->next_property_cp;
    }
  }
#endif /* ENABLED (JERRY_PROPRETY_HASHMAP) */

  while (prop_iter_cp != JMEM_CP_NULL)
  {
    ecma_property_header_t *prop_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_property_header_t, prop_iter_cp);
    JERRY_ASSERT (ECMA_PROPERTY_IS_PROPERTY_PAIR (prop_iter_p));

    ecma_gc_mark_properties ((ecma_property_pair_t *) prop_iter_p);

    prop_iter_cp = prop_iter_p->next_property_cp;
  }
} /* ecma_gc_mark */

/**
 * Free the native handle/pointer by calling its free callback.
 */
static void
ecma_gc_free_native_pointer (ecma_property_t *property_p) /**< property */
{
  JERRY_ASSERT (property_p != NULL);

  ecma_property_value_t *value_p = ECMA_PROPERTY_VALUE_PTR (property_p);
  ecma_native_pointer_t *native_pointer_p;

  native_pointer_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_native_pointer_t,
                                                      value_p->value);

  while (native_pointer_p != NULL)
  {
    if (native_pointer_p->info_p != NULL)
    {
      ecma_object_native_free_callback_t free_cb = native_pointer_p->info_p->free_cb;

      if (free_cb != NULL)
      {
        free_cb (native_pointer_p->data_p);
      }
    }

    ecma_native_pointer_t *next_p = native_pointer_p->next_p;

    jmem_heap_free_block (native_pointer_p, sizeof (ecma_native_pointer_t));

    native_pointer_p = next_p;
  }
} /* ecma_gc_free_native_pointer */

/**
 * Free specified fast access mode array object.
 */
static void
ecma_free_fast_access_array (ecma_object_t *object_p) /**< fast access mode array object to free */
{
  JERRY_ASSERT (ecma_op_object_is_fast_array (object_p));

  ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;
  const uint32_t aligned_length = ECMA_FAST_ARRAY_ALIGN_LENGTH (ext_object_p->u.array.length);

  if (object_p->u1.property_list_cp != JMEM_CP_NULL)
  {
    ecma_value_t *values_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, object_p->u1.property_list_cp);

    for (uint32_t i = 0; i < aligned_length; i++)
    {
      ecma_free_value_if_not_object (values_p[i]);
    }

    jmem_heap_free_block (values_p, aligned_length * sizeof (ecma_value_t));
  }

  ecma_dealloc_extended_object (object_p, sizeof (ecma_extended_object_t));
} /* ecma_free_fast_access_array */

#if ENABLED (JERRY_ESNEXT)

/**
 * Free non-objects referenced by inactive generator functions, async functions, etc.
 *
 * @return total object size
 */
static size_t
ecma_gc_free_executable_object (ecma_object_t *object_p) /**< object */
{
  vm_executable_object_t *executable_object_p = (vm_executable_object_t *) object_p;

  const ecma_compiled_code_t *bytecode_header_p = executable_object_p->frame_ctx.bytecode_header_p;
  size_t size, register_end;

  if (bytecode_header_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) bytecode_header_p;

    register_end = args_p->register_end;
    size = (register_end + (size_t) args_p->stack_limit) * sizeof (ecma_value_t);
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) bytecode_header_p;

    register_end = args_p->register_end;
    size = (register_end + (size_t) args_p->stack_limit) * sizeof (ecma_value_t);
  }

  size = JERRY_ALIGNUP (sizeof (vm_executable_object_t) + size, sizeof (uintptr_t));

  JERRY_ASSERT (!(executable_object_p->extended_object.u.class_prop.extra_info & ECMA_EXECUTABLE_OBJECT_RUNNING));

  ecma_bytecode_deref ((ecma_compiled_code_t *) bytecode_header_p);

  if (executable_object_p->extended_object.u.class_prop.extra_info & ECMA_ASYNC_GENERATOR_CALLED)
  {
    ecma_value_t task = executable_object_p->extended_object.u.class_prop.u.head;

    while (!ECMA_IS_INTERNAL_VALUE_NULL (task))
    {
      ecma_async_generator_task_t *task_p;
      task_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_async_generator_task_t, task);

      JERRY_ASSERT (ecma_is_value_object (task_p->promise));
      ecma_free_value_if_not_object (task_p->operation_value);

      task = task_p->next;
      jmem_heap_free_block (task_p, sizeof (ecma_async_generator_task_t));
    }
  }

  if (executable_object_p->extended_object.u.class_prop.extra_info & ECMA_EXECUTABLE_OBJECT_COMPLETED)
  {
    return size;
  }

  ecma_free_value_if_not_object (executable_object_p->frame_ctx.this_binding);

  ecma_value_t *register_p = VM_GET_REGISTERS (&executable_object_p->frame_ctx);
  ecma_value_t *register_end_p = register_p + register_end;

  while (register_p < register_end_p)
  {
    ecma_free_value_if_not_object (*register_p++);
  }

  if (executable_object_p->frame_ctx.context_depth > 0)
  {
    ecma_value_t *context_end_p = register_p;

    register_p += executable_object_p->frame_ctx.context_depth;

    ecma_value_t *context_top_p = register_p;

    do
    {
      context_top_p[-1] &= (uint32_t) ~VM_CONTEXT_HAS_LEX_ENV;

      uint32_t offsets = vm_get_context_value_offsets (context_top_p);

      while (VM_CONTEXT_HAS_NEXT_OFFSET (offsets))
      {
        int32_t offset = VM_CONTEXT_GET_NEXT_OFFSET (offsets);

        if (ecma_is_value_object (context_top_p[offset]))
        {
          context_top_p[offset] = ECMA_VALUE_UNDEFINED;
        }

        offsets >>= VM_CONTEXT_OFFSET_SHIFT;
      }

      context_top_p = vm_stack_context_abort (&executable_object_p->frame_ctx, context_top_p);
    }
    while (context_top_p > context_end_p);
  }

  register_end_p = executable_object_p->frame_ctx.stack_top_p;

  while (register_p < register_end_p)
  {
    ecma_free_value_if_not_object (*register_p++);
  }

  return size;
} /* ecma_gc_free_executable_object */

#endif /* ENABLED (JERRY_ESNEXT) */

/**
 * Free properties of an object
 */
void
ecma_gc_free_properties (ecma_object_t *object_p) /**< object */
{
  jmem_cpointer_t prop_iter_cp = object_p->u1.property_list_cp;

#if ENABLED (JERRY_PROPRETY_HASHMAP)
  if (prop_iter_cp != JMEM_CP_NULL)
  {
    ecma_property_header_t *prop_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_property_header_t,
                                                                     prop_iter_cp);
    if (prop_iter_p->types[0] == ECMA_PROPERTY_TYPE_HASHMAP)
    {
      ecma_property_hashmap_free (object_p);
      prop_iter_cp = object_p->u1.property_list_cp;
    }
  }
#endif /* ENABLED (JERRY_PROPRETY_HASHMAP) */

  while (prop_iter_cp != JMEM_CP_NULL)
  {
    ecma_property_header_t *prop_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_property_header_t, prop_iter_cp);
    JERRY_ASSERT (ECMA_PROPERTY_IS_PROPERTY_PAIR (prop_iter_p));

    /* Both cannot be deleted. */
    JERRY_ASSERT (prop_iter_p->types[0] != ECMA_PROPERTY_TYPE_DELETED
                  || prop_iter_p->types[1] != ECMA_PROPERTY_TYPE_DELETED);

    ecma_property_pair_t *prop_pair_p = (ecma_property_pair_t *) prop_iter_p;

    for (int i = 0; i < ECMA_PROPERTY_PAIR_ITEM_COUNT; i++)
    {
      ecma_property_t *property_p = (ecma_property_t *) (prop_iter_p->types + i);
      jmem_cpointer_t name_cp = prop_pair_p->names_cp[i];

      if (ECMA_PROPERTY_GET_NAME_TYPE (*property_p) == ECMA_DIRECT_STRING_MAGIC)
      {
        /* Call the native's free callback. */
        if (JERRY_UNLIKELY (name_cp == LIT_INTERNAL_MAGIC_STRING_NATIVE_POINTER))
        {
          ecma_gc_free_native_pointer (property_p);
        }
#if ENABLED (JERRY_BUILTIN_WEAKMAP) || ENABLED (JERRY_BUILTIN_WEAKSET)
        else if (JERRY_UNLIKELY (name_cp == LIT_INTERNAL_MAGIC_STRING_WEAK_REFS))
        {
          ecma_collection_t *refs_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t,
                                                                       ECMA_PROPERTY_VALUE_PTR (property_p)->value);
          for (uint32_t j = 0; j < refs_p->item_count; j++)
          {
            const ecma_value_t value = refs_p->buffer_p[j];
            if (!ecma_is_value_empty (value))
            {
              ecma_object_t *container_p = ecma_get_object_from_value (value);

              ecma_op_container_remove_weak_entry (container_p,
                                                   ecma_make_object_value (object_p));
            }
          }

          ecma_collection_destroy (refs_p);
        }
#endif /* ENABLED (JERRY_BUILTIN_WEAKMAP) || ENABLED (JERRY_BUILTIN_WEAKSET) */
      }

      if (prop_iter_p->types[i] != ECMA_PROPERTY_TYPE_DELETED)
      {
        ecma_free_property (object_p, name_cp, property_p);
      }
    }

    prop_iter_cp = prop_iter_p->next_property_cp;

    ecma_dealloc_property_pair (prop_pair_p);
  }
} /* ecma_gc_free_properties */

/**
 * Free specified object.
 */
static void
ecma_gc_free_object (ecma_object_t *object_p) /**< object to free */
{
  JERRY_ASSERT (object_p != NULL
                && !ecma_gc_is_object_visited (object_p)
                && ((object_p->type_flags_refs & ECMA_OBJECT_REF_MASK) == ECMA_OBJECT_NON_VISITED));

  JERRY_ASSERT (JERRY_CONTEXT (ecma_gc_objects_number) > 0);
  JERRY_CONTEXT (ecma_gc_objects_number)--;

  if (ecma_is_lexical_environment (object_p))
  {
    if (ecma_get_lex_env_type (object_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
    {
      ecma_gc_free_properties (object_p);
    }

    ecma_dealloc_object (object_p);
    return;
  }

  ecma_object_type_t object_type = ecma_get_object_type (object_p);

  size_t ext_object_size = sizeof (ecma_extended_object_t);

  if (JERRY_UNLIKELY (ecma_get_object_is_builtin (object_p)))
  {
    uint8_t length_and_bitset_size;

    if (object_type == ECMA_OBJECT_TYPE_CLASS
        || object_type == ECMA_OBJECT_TYPE_ARRAY)
    {
      ext_object_size = sizeof (ecma_extended_built_in_object_t);
      length_and_bitset_size = ((ecma_extended_built_in_object_t *) object_p)->built_in.length_and_bitset_size;
      ext_object_size += (2 * sizeof (uint32_t)) * (length_and_bitset_size >> ECMA_BUILT_IN_BITSET_SHIFT);
    }
    else
    {
      length_and_bitset_size = ((ecma_extended_object_t *) object_p)->u.built_in.length_and_bitset_size;
      ext_object_size += (2 * sizeof (uint32_t)) * (length_and_bitset_size >> ECMA_BUILT_IN_BITSET_SHIFT);

      ecma_gc_free_properties (object_p);
      ecma_dealloc_extended_object (object_p, ext_object_size);
      return;
    }
  }

  switch (object_type)
  {
    case ECMA_OBJECT_TYPE_GENERAL:
    {
      ecma_gc_free_properties (object_p);
      ecma_dealloc_object (object_p);
      return;
    }
    case ECMA_OBJECT_TYPE_ARRAY:
    {
      if (ecma_op_array_is_fast_array ((ecma_extended_object_t *) object_p))
      {
        ecma_free_fast_access_array (object_p);
        return;
      }
      break;
    }
    case ECMA_OBJECT_TYPE_EXTERNAL_FUNCTION:
    {
#if ENABLED (JERRY_ESNEXT)
      ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;

      if (ext_func_p->u.external_handler_cb == ecma_proxy_revoke_cb)
      {
        ext_object_size = sizeof (ecma_revocable_proxy_object_t);
      }
      else if (ext_func_p->u.external_handler_cb == ecma_op_get_capabilities_executor_cb)
      {
        ext_object_size = sizeof (ecma_promise_capability_executor_t);
      }
      else if (ext_func_p->u.external_handler_cb == ecma_promise_all_handler_cb)
      {
        ext_object_size = sizeof (ecma_promise_all_executor_t);
      }
#endif /* ENABLED (JERRY_ESNEXT) */
      break;
    }
    case ECMA_OBJECT_TYPE_CLASS:
    {
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

      switch (ext_object_p->u.class_prop.class_id)
      {
#if ENABLED (JERRY_ESNEXT)
        case LIT_MAGIC_STRING_SYMBOL_UL:
#endif /* ENABLED (JERRY_ESNEXT) */
        case LIT_MAGIC_STRING_STRING_UL:
        case LIT_MAGIC_STRING_NUMBER_UL:
        {
          ecma_free_value (ext_object_p->u.class_prop.u.value);
          break;
        }

        case LIT_MAGIC_STRING_DATE_UL:
        {
          ecma_number_t *num_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_number_t,
                                                                  ext_object_p->u.class_prop.u.value);
          ecma_dealloc_number (num_p);
          break;
        }
        case LIT_MAGIC_STRING_REGEXP_UL:
        {
          ecma_compiled_code_t *bytecode_p = ECMA_GET_INTERNAL_VALUE_ANY_POINTER (ecma_compiled_code_t,
                                                                                  ext_object_p->u.class_prop.u.value);

          ecma_bytecode_deref (bytecode_p);

          break;
        }
#if ENABLED (JERRY_BUILTIN_TYPEDARRAY)
        case LIT_MAGIC_STRING_ARRAY_BUFFER_UL:
        {
          uint32_t arraybuffer_length = ext_object_p->u.class_prop.u.length;

          if (ECMA_ARRAYBUFFER_HAS_EXTERNAL_MEMORY (ext_object_p))
          {
            ext_object_size = sizeof (ecma_arraybuffer_external_info);

            /* Call external free callback if any. */
            ecma_arraybuffer_external_info *array_p = (ecma_arraybuffer_external_info *) ext_object_p;
            JERRY_ASSERT (array_p != NULL);

            if (array_p->free_cb != NULL)
            {
              (array_p->free_cb) (array_p->buffer_p);
            }
          }
          else
          {
            ext_object_size += arraybuffer_length;
          }

          break;
        }
#endif /* ENABLED (JERRY_BUILTIN_TYPEDARRAY) */
#if ENABLED (JERRY_BUILTIN_PROMISE)
        case LIT_MAGIC_STRING_PROMISE_UL:
        {
          ecma_free_value_if_not_object (ext_object_p->u.class_prop.u.value);

          /* Reactions only contains objects. */
          ecma_collection_destroy (((ecma_promise_object_t *) object_p)->reactions);

          ext_object_size = sizeof (ecma_promise_object_t);
          break;
        }
#endif /* ENABLED (JERRY_BUILTIN_PROMISE) */
#if ENABLED (JERRY_BUILTIN_CONTAINER)
#if ENABLED (JERRY_BUILTIN_MAP)
        case LIT_MAGIC_STRING_MAP_UL:
#endif /* ENABLED (JERRY_BUILTIN_MAP) */
#if ENABLED (JERRY_BUILTIN_SET)
        case LIT_MAGIC_STRING_SET_UL:
#endif /* ENABLED (JERRY_BUILTIN_SET) */
#if ENABLED (JERRY_BUILTIN_WEAKMAP)
        case LIT_MAGIC_STRING_WEAKMAP_UL:
#endif /* ENABLED (JERRY_BUILTIN_WEAKMAP) */
#if ENABLED (JERRY_BUILTIN_WEAKSET)
        case LIT_MAGIC_STRING_WEAKSET_UL:
#endif /* ENABLED (JERRY_BUILTIN_WEAKSET) */
        {
          ecma_extended_object_t *map_object_p = (ecma_extended_object_t *) object_p;
          ecma_collection_t *container_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t,
                                                                            map_object_p->u.class_prop.u.value);
          ecma_op_container_free_entries (object_p);
          ecma_collection_destroy (container_p);

          break;
        }
#endif /* ENABLED (JERRY_BUILTIN_CONTAINER) */
#if ENABLED (JERRY_BUILTIN_DATAVIEW)
        case LIT_MAGIC_STRING_DATAVIEW_UL:
        {
          ext_object_size = sizeof (ecma_dataview_object_t);
          break;
        }
#endif /* ENABLED (JERRY_BUILTIN_DATAVIEW) */
#if ENABLED (JERRY_ESNEXT)
        case LIT_MAGIC_STRING_GENERATOR_UL:
        case LIT_MAGIC_STRING_ASYNC_GENERATOR_UL:
        {
          ext_object_size = ecma_gc_free_executable_object (object_p);
          break;
        }
        case LIT_INTERNAL_MAGIC_PROMISE_CAPABILITY:
        {
          ext_object_size = sizeof (ecma_promise_capabality_t);
          break;
        }
#endif /* ENABLED (JERRY_ESNEXT) */
        default:
        {
          /* The undefined id represents an uninitialized class. */
          JERRY_ASSERT (ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_UNDEFINED
                        || ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_ARGUMENTS_UL
                        || ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_BOOLEAN_UL
                        || ext_object_p->u.class_prop.class_id == LIT_MAGIC_STRING_ERROR_UL
                        || ext_object_p->u.class_prop.class_id == LIT_INTERNAL_MAGIC_STRING_INTERNAL_OBJECT);
          break;
        }
      }

      break;
    }
#if ENABLED (JERRY_BUILTIN_PROXY)
    case ECMA_OBJECT_TYPE_PROXY:
    {
      ext_object_size = sizeof (ecma_proxy_object_t);
      break;
    }
#endif /* ENABLED (JERRY_BUILTIN_PROXY) */
    case ECMA_OBJECT_TYPE_FUNCTION:
    {
      /* Function with byte-code (not a built-in function). */
      ecma_extended_object_t *ext_func_p = (ecma_extended_object_t *) object_p;

#if ENABLED (JERRY_SNAPSHOT_EXEC)
      if (ext_func_p->u.function.bytecode_cp != ECMA_NULL_POINTER)
      {
#endif /* ENABLED (JERRY_SNAPSHOT_EXEC) */
        ecma_compiled_code_t *byte_code_p = (ECMA_GET_INTERNAL_VALUE_POINTER (ecma_compiled_code_t,
                                                                              ext_func_p->u.function.bytecode_cp));

#if ENABLED (JERRY_ESNEXT)
        if (CBC_FUNCTION_IS_ARROW (byte_code_p->status_flags))
        {
          ecma_free_value_if_not_object (((ecma_arrow_function_t *) object_p)->this_binding);
          ecma_free_value_if_not_object (((ecma_arrow_function_t *) object_p)->new_target);
          ext_object_size = sizeof (ecma_arrow_function_t);
        }
#endif /* ENABLED (JERRY_ESNEXT) */

        ecma_bytecode_deref (byte_code_p);
#if ENABLED (JERRY_SNAPSHOT_EXEC)
      }
      else
      {
        ext_object_size = sizeof (ecma_static_function_t);
      }
#endif /* ENABLED (JERRY_SNAPSHOT_EXEC) */
      break;
    }
    case ECMA_OBJECT_TYPE_PSEUDO_ARRAY:
    {
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

      switch (ext_object_p->u.pseudo_array.type)
      {
        case ECMA_PSEUDO_ARRAY_ARGUMENTS:
        {
          JERRY_ASSERT (ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_ARRAY_ARGUMENTS);

          uint32_t formal_params_number = ext_object_p->u.pseudo_array.u1.length;
          ecma_value_t *arg_literal_p = (ecma_value_t *) (ext_object_p + 1);

          for (uint32_t i = 0; i < formal_params_number; i++)
          {
            if (arg_literal_p[i] != ECMA_VALUE_EMPTY)
            {
              ecma_string_t *name_p = ecma_get_string_from_value (arg_literal_p[i]);
              ecma_deref_ecma_string (name_p);
            }
          }

          size_t formal_params_size = formal_params_number * sizeof (ecma_value_t);
          ext_object_size += formal_params_size;
          break;
        }
#if ENABLED (JERRY_BUILTIN_TYPEDARRAY)
        case ECMA_PSEUDO_ARRAY_TYPEDARRAY_WITH_INFO:
        {
          ext_object_size = sizeof (ecma_extended_typedarray_object_t);
          break;
        }
#endif /* ENABLED (JERRY_BUILTIN_TYPEDARRAY) */
#if ENABLED (JERRY_ESNEXT)
        case ECMA_PSEUDO_STRING_ITERATOR:
        {
          ecma_value_t iterated_value = ext_object_p->u.pseudo_array.u2.iterated_value;

          if (!ecma_is_value_empty (iterated_value))
          {
            ecma_deref_ecma_string (ecma_get_string_from_value (iterated_value));
          }

          break;
        }
#endif /* ENABLED (JERRY_ESNEXT) */
        default:
        {
          JERRY_ASSERT (ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_ARRAY_TYPEDARRAY
                        || ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_ARRAY_ITERATOR
                        || ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_SET_ITERATOR
                        || ext_object_p->u.pseudo_array.type == ECMA_PSEUDO_MAP_ITERATOR);
          break;
        }
      }

      break;
    }
    case ECMA_OBJECT_TYPE_BOUND_FUNCTION:
    {
      ext_object_size = sizeof (ecma_bound_function_t);
      ecma_bound_function_t *bound_func_p = (ecma_bound_function_t *) object_p;

      ecma_value_t args_len_or_this = bound_func_p->header.u.bound_function.args_len_or_this;

      if (!ecma_is_value_integer_number (args_len_or_this))
      {
        ecma_free_value_if_not_object (args_len_or_this);
        break;
      }

      ecma_integer_value_t args_length = ecma_get_integer_from_value (args_len_or_this);
      ecma_value_t *args_p = (ecma_value_t *) (bound_func_p + 1);

      for (ecma_integer_value_t i = 0; i < args_length; i++)
      {
        ecma_free_value_if_not_object (args_p[i]);
      }

      size_t args_size = ((size_t) args_length) * sizeof (ecma_value_t);
      ext_object_size += args_size;
      break;
    }
    default:
    {
      JERRY_UNREACHABLE ();
    }
  }

  ecma_gc_free_properties (object_p);
  ecma_dealloc_extended_object (object_p, ext_object_size);
} /* ecma_gc_free_object */

/**
 * Run garbage collection, freeing objects that are no longer referenced.
 */
void
ecma_gc_run (void)
{
#if (JERRY_GC_MARK_LIMIT != 0)
  JERRY_ASSERT (JERRY_CONTEXT (ecma_gc_mark_recursion_limit) == JERRY_GC_MARK_LIMIT);
#endif /* (JERRY_GC_MARK_LIMIT != 0) */

  JERRY_CONTEXT (ecma_gc_new_objects) = 0;

  ecma_object_t black_list_head;
  black_list_head.gc_next_cp = JMEM_CP_NULL;
  ecma_object_t *black_end_p = &black_list_head;

  ecma_object_t white_gray_list_head;
  white_gray_list_head.gc_next_cp = JERRY_CONTEXT (ecma_gc_objects_cp);

  ecma_object_t *obj_prev_p = &white_gray_list_head;
  jmem_cpointer_t obj_iter_cp = obj_prev_p->gc_next_cp;
  ecma_object_t *obj_iter_p;

  /* Move root objects (i.e. they have global or stack references) to the black list. */
  while (obj_iter_cp != JMEM_CP_NULL)
  {
    obj_iter_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_object_t, obj_iter_cp);
    const jmem_cpointer_t obj_next_cp = obj_iter_p->gc_next_cp;

    JERRY_ASSERT (obj_prev_p == NULL
                  || ECMA_GET_NON_NULL_POINTER (ecma_object_t, obj_prev_p->gc_next_cp) == obj_iter_p);

    if (obj_iter_p->type_flags_refs >= ECMA_OBJECT_REF_ONE)
    {
      /* Moving the object to list of marked objects. */
      obj_prev_p->gc_next_cp = obj_next_cp;

      black_end_p->gc_next_cp = obj_iter_cp;
      black_end_p = obj_iter_p;
    }
    else
    {
      obj_iter_p->type_flags_refs |= ECMA_OBJECT_NON_VISITED;
      obj_prev_p = obj_iter_p;
    }

    obj_iter_cp = obj_next_cp;
  }

  black_end_p->gc_next_cp = JMEM_CP_NULL;

  /* Mark root objects. */
  obj_iter_cp = black_list_head.gc_next_cp;
  while (obj_iter_cp != JMEM_CP_NULL)
  {
    obj_iter_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_object_t, obj_iter_cp);
    ecma_gc_mark (obj_iter_p);
    obj_iter_cp = obj_iter_p->gc_next_cp;
  }

  /* Mark non-root objects. */
  bool marked_anything_during_current_iteration;

  do
  {
#if (JERRY_GC_MARK_LIMIT != 0)
    JERRY_ASSERT (JERRY_CONTEXT (ecma_gc_mark_recursion_limit) == JERRY_GC_MARK_LIMIT);
#endif /* (JERRY_GC_MARK_LIMIT != 0) */

    marked_anything_during_current_iteration = false;

    obj_prev_p = &white_gray_list_head;
    obj_iter_cp = obj_prev_p->gc_next_cp;

    while (obj_iter_cp != JMEM_CP_NULL)
    {
      obj_iter_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_object_t, obj_iter_cp);
      const jmem_cpointer_t obj_next_cp = obj_iter_p->gc_next_cp;

      JERRY_ASSERT (obj_prev_p == NULL
                    || ECMA_GET_NON_NULL_POINTER (ecma_object_t, obj_prev_p->gc_next_cp) == obj_iter_p);

      if (ecma_gc_is_object_visited (obj_iter_p))
      {
        /* Moving the object to list of marked objects */
        obj_prev_p->gc_next_cp = obj_next_cp;

        black_end_p->gc_next_cp = obj_iter_cp;
        black_end_p = obj_iter_p;

#if (JERRY_GC_MARK_LIMIT != 0)
        if (obj_iter_p->type_flags_refs >= ECMA_OBJECT_REF_ONE)
        {
          /* Set the reference count of non-marked gray object to 0 */
          obj_iter_p->type_flags_refs = (uint16_t) (obj_iter_p->type_flags_refs & (ECMA_OBJECT_REF_ONE - 1));
          ecma_gc_mark (obj_iter_p);
          marked_anything_during_current_iteration = true;
        }
#else /* (JERRY_GC_MARK_LIMIT == 0) */
        marked_anything_during_current_iteration = true;
#endif /* (JERRY_GC_MARK_LIMIT != 0) */
      }
      else
      {
        obj_prev_p = obj_iter_p;
      }

      obj_iter_cp = obj_next_cp;
    }
  }
  while (marked_anything_during_current_iteration);

  black_end_p->gc_next_cp = JMEM_CP_NULL;
  JERRY_CONTEXT (ecma_gc_objects_cp) = black_list_head.gc_next_cp;

  /* Sweep objects that are currently unmarked. */
  obj_iter_cp = white_gray_list_head.gc_next_cp;

  while (obj_iter_cp != JMEM_CP_NULL)
  {
    obj_iter_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_object_t, obj_iter_cp);
    const jmem_cpointer_t obj_next_cp = obj_iter_p->gc_next_cp;

    JERRY_ASSERT (!ecma_gc_is_object_visited (obj_iter_p));

    ecma_gc_free_object (obj_iter_p);
    obj_iter_cp = obj_next_cp;
  }

#if ENABLED (JERRY_BUILTIN_REGEXP)
  /* Free RegExp bytecodes stored in cache */
  re_cache_gc ();
#endif /* ENABLED (JERRY_BUILTIN_REGEXP) */
} /* ecma_gc_run */

/**
 * Try to free some memory (depending on memory pressure).
 *
 * When called with JMEM_PRESSURE_FULL, the engine will be terminated with ERR_OUT_OF_MEMORY.
 */
void
ecma_free_unused_memory (jmem_pressure_t pressure) /**< current pressure */
{
#if ENABLED (JERRY_DEBUGGER)
  while ((JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
         && JERRY_CONTEXT (debugger_byte_code_free_tail) != ECMA_NULL_POINTER)
  {
    /* Wait until all byte code is freed or the connection is aborted. */
    jerry_debugger_receive (NULL);
  }
#endif /* ENABLED (JERRY_DEBUGGER) */

  if (JERRY_LIKELY (pressure == JMEM_PRESSURE_LOW))
  {
#if ENABLED (JERRY_PROPRETY_HASHMAP)
    if (JERRY_CONTEXT (ecma_prop_hashmap_alloc_state) > ECMA_PROP_HASHMAP_ALLOC_ON)
    {
      --JERRY_CONTEXT (ecma_prop_hashmap_alloc_state);
    }
    JERRY_CONTEXT (status_flags) &= (uint32_t) ~ECMA_STATUS_HIGH_PRESSURE_GC;
#endif /* ENABLED (JERRY_PROPRETY_HASHMAP) */
    /*
     * If there is enough newly allocated objects since last GC, probably it is worthwhile to start GC now.
     * Otherwise, probability to free sufficient space is considered to be low.
     */
    size_t new_objects_fraction = CONFIG_ECMA_GC_NEW_OBJECTS_FRACTION;

    if (JERRY_CONTEXT (ecma_gc_new_objects) * new_objects_fraction > JERRY_CONTEXT (ecma_gc_objects_number))
    {
      ecma_gc_run ();
    }

    return;
  }
  else if (pressure == JMEM_PRESSURE_HIGH)
  {
    /* Freeing as much memory as we currently can */
#if ENABLED (JERRY_PROPRETY_HASHMAP)
    if (JERRY_CONTEXT (status_flags) & ECMA_STATUS_HIGH_PRESSURE_GC)
    {
      JERRY_CONTEXT (ecma_prop_hashmap_alloc_state) = ECMA_PROP_HASHMAP_ALLOC_MAX;
    }
    else if (JERRY_CONTEXT (ecma_prop_hashmap_alloc_state) < ECMA_PROP_HASHMAP_ALLOC_MAX)
    {
      ++JERRY_CONTEXT (ecma_prop_hashmap_alloc_state);
      JERRY_CONTEXT (status_flags) |= ECMA_STATUS_HIGH_PRESSURE_GC;
    }
#endif /* ENABLED (JERRY_PROPRETY_HASHMAP) */

    ecma_gc_run ();

#if ENABLED (JERRY_PROPRETY_HASHMAP)
    /* Free hashmaps of remaining objects. */
    jmem_cpointer_t obj_iter_cp = JERRY_CONTEXT (ecma_gc_objects_cp);

    while (obj_iter_cp != JMEM_CP_NULL)
    {
      ecma_object_t *obj_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_object_t, obj_iter_cp);

      if (!ecma_is_lexical_environment (obj_iter_p)
          || ecma_get_lex_env_type (obj_iter_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE)
      {
        if (!ecma_is_lexical_environment (obj_iter_p)
            && ecma_op_object_is_fast_array (obj_iter_p))
        {
          obj_iter_cp = obj_iter_p->gc_next_cp;
          continue;
        }

        jmem_cpointer_t prop_iter_cp = obj_iter_p->u1.property_list_cp;

        if (prop_iter_cp != JMEM_CP_NULL)
        {
          ecma_property_header_t *prop_iter_p = ECMA_GET_NON_NULL_POINTER (ecma_property_header_t, prop_iter_cp);

          if (prop_iter_p->types[0] == ECMA_PROPERTY_TYPE_HASHMAP)
          {
            ecma_property_hashmap_free (obj_iter_p);
          }
        }

      }

      obj_iter_cp = obj_iter_p->gc_next_cp;
    }
#endif /* ENABLED (JERRY_PROPRETY_HASHMAP) */

    jmem_pools_collect_empty ();
    return;
  }
  else if (JERRY_UNLIKELY (pressure == JMEM_PRESSURE_FULL))
  {
    jerry_fatal (ERR_OUT_OF_MEMORY);
  }
  else
  {
    JERRY_ASSERT (pressure == JMEM_PRESSURE_NONE);
    JERRY_UNREACHABLE ();
  }
} /* ecma_free_unused_memory */

/**
 * @}
 * @}
 */
