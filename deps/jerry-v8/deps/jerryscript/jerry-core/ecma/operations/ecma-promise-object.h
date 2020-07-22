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

#ifndef ECMA_PROMISE_OBJECT_H
#define ECMA_PROMISE_OBJECT_H

#if ENABLED (JERRY_BUILTIN_PROMISE)
#include "ecma-globals.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmaarraybufferobject ECMA ArrayBuffer object related routines
 * @{
 */

/**
 * The PromiseState of promise object.
 */
typedef enum
{
  ECMA_PROMISE_IS_PENDING = (1 << 0), /**< pending state */
  ECMA_PROMISE_IS_FULFILLED = (1 << 1), /**< fulfilled state */
  ECMA_PROMISE_ALREADY_RESOLVED = (1 << 2), /**< already resolved */
} ecma_promise_flags_t;

/**
 * Indicates the type of the executor in promise construct.
 */
typedef enum
{
  ECMA_PROMISE_EXECUTOR_FUNCTION, /**< the executor is a function, it is for the usual constructor */
  ECMA_PROMISE_EXECUTOR_EMPTY /**< the executor is empty, it is for external C API */
} ecma_promise_executor_type_t;

/**
 * Description of the promise resolving functions.
 */
typedef struct
{
  ecma_value_t resolve; /**< the resolve function */
  ecma_value_t reject; /**< the reject function */
} ecma_promise_resolving_functions_t;

/**
 * Description of the promise object.
 * It need more space than normal object to store builtin properties.
 */
typedef struct
{
  ecma_extended_object_t header; /**< extended object part */
  ecma_collection_t *reactions; /**< list of promise reactions */
} ecma_promise_object_t;

/* The Promise reaction is a compressed structure, where each item can
 * be a sequence of up to three ecma object values as seen below:
 *
 * [ Capability ][ Optional fullfilled callback ][ Optional rejected callback ]
 * [ Async function callback ]
 *
 * The first member is an object, which lower bits specify the type of the reaction:
 *   bit 2 is not set: callback reactions
 *     The first two objects specify the resolve/reject functions of the promise
 *     returned by the `then` operation which can be used to chain event handlers.
 *
 *     bit 0: has a fullfilled callback
 *     bit 1: has a rejected callback
 *
 *   bit 2 is set: async function callback
 */

bool ecma_is_promise (ecma_object_t *obj_p);
ecma_value_t ecma_op_create_promise_object (ecma_value_t executor, ecma_promise_executor_type_t type);
uint16_t ecma_promise_get_flags (ecma_object_t *promise_p);
ecma_value_t ecma_promise_get_result (ecma_object_t *promise_p);
void ecma_reject_promise (ecma_value_t promise, ecma_value_t reason);
void ecma_fulfill_promise (ecma_value_t promise, ecma_value_t value);
ecma_object_t *ecma_promise_new_capability (ecma_value_t constructor);
ecma_value_t ecma_promise_reject_or_resolve (ecma_value_t this_arg, ecma_value_t value, bool is_resolve);
ecma_value_t ecma_promise_then (ecma_value_t promise, ecma_value_t on_fulfilled, ecma_value_t on_rejected);
void ecma_promise_async_then (ecma_value_t promise, ecma_value_t executable_object);
ecma_value_t ecma_promise_async_await (ecma_extended_object_t *async_generator_object_p, ecma_value_t value);
void ecma_promise_create_resolving_functions (ecma_object_t *object_p, ecma_promise_resolving_functions_t *funcs,
                                              bool create_already_resolved);
void ecma_promise_free_resolving_functions (ecma_promise_resolving_functions_t *funcs);

uint32_t ecma_promise_remaining_inc_or_dec (ecma_value_t remaining, bool is_inc);
ecma_value_t ecma_promise_all_handler_cb (const ecma_value_t function_obj, const ecma_value_t this_val,
                                          const ecma_value_t args_p[], const uint32_t args_count);

ecma_value_t ecma_op_get_capabilities_executor_cb (const ecma_value_t function_obj, const ecma_value_t this_val,
                                                   const ecma_value_t args_p[], const uint32_t args_count);

/**
 * @}
 * @}
 */

#endif /* ENABLED (JERRY_BUILTIN_PROMISE) */
#endif /* !ECMA_PROMISE_OBJECT_H */
