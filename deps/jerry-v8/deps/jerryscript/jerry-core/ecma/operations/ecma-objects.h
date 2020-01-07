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

#ifndef ECMA_OBJECTS_H
#define ECMA_OBJECTS_H

#include "ecma-builtins.h"
#include "ecma-conversion.h"
#include "ecma-globals.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmaobjectsinternalops ECMA objects' operations
 * @{
 */

ecma_property_t ecma_op_object_get_own_property (ecma_object_t *object_p, ecma_string_t *property_name_p,
                                                 ecma_property_ref_t *property_ref_p, uint32_t options);
bool ecma_op_object_has_own_property (ecma_object_t *object_p, ecma_string_t *property_name_p);
bool ecma_op_object_has_property (ecma_object_t *object_p, ecma_string_t *property_name_p);
ecma_value_t ecma_op_object_find_own (ecma_value_t base_value, ecma_object_t *object_p, ecma_string_t *property_name_p);
ecma_value_t ecma_op_object_find (ecma_object_t *object_p, ecma_string_t *property_name_p);
ecma_value_t ecma_op_object_find_by_uint32_index (ecma_object_t *object_p, uint32_t index);
ecma_value_t ecma_op_object_find_by_number_index (ecma_object_t *object_p, ecma_number_t index);
ecma_value_t ecma_op_object_get_own_data_prop (ecma_object_t *object_p, ecma_string_t *property_name_p);
ecma_value_t ecma_op_object_get (ecma_object_t *object_p, ecma_string_t *property_name_p);
ecma_value_t ecma_op_object_get_with_receiver (ecma_object_t *object_p, ecma_string_t *property_name_p,
                                               ecma_value_t receiver);
ecma_value_t ecma_op_object_get_length (ecma_object_t *object_p, uint32_t *length_p);
ecma_value_t ecma_op_object_get_by_uint32_index (ecma_object_t *object_p, uint32_t index);
ecma_value_t ecma_op_object_get_by_magic_id (ecma_object_t *object_p, lit_magic_string_id_t property_id);
#if ENABLED (JERRY_ES2015)
ecma_value_t ecma_op_object_get_by_symbol_id (ecma_object_t *object_p, lit_magic_string_id_t property_id);
ecma_value_t ecma_op_get_method_by_symbol_id (ecma_value_t value, lit_magic_string_id_t symbol_id);
ecma_value_t ecma_op_get_method_by_magic_id (ecma_value_t value, lit_magic_string_id_t magic_id);
#endif /* ENABLED (JERRY_ES2015) */
ecma_value_t ecma_op_object_put (ecma_object_t *object_p, ecma_string_t *property_name_p, ecma_value_t value,
                                 bool is_throw);
ecma_value_t ecma_op_object_put_by_uint32_index (ecma_object_t *object_p, uint32_t index,
                                                 ecma_value_t value, bool is_throw);
ecma_value_t ecma_op_object_put_by_number_index (ecma_object_t *object_p, ecma_number_t index,
                                                 ecma_value_t value, bool is_throw);
ecma_value_t ecma_op_object_delete (ecma_object_t *obj_p, ecma_string_t *property_name_p, bool is_throw);
ecma_value_t ecma_op_object_delete_by_uint32_index (ecma_object_t *obj_p, uint32_t index, bool is_throw);
ecma_value_t ecma_op_object_delete_by_number_index (ecma_object_t *obj_p, ecma_number_t index, bool is_throw);
ecma_value_t ecma_op_object_default_value (ecma_object_t *obj_p, ecma_preferred_type_hint_t hint);
ecma_value_t ecma_op_object_define_own_property (ecma_object_t *obj_p, ecma_string_t *property_name_p,
                                                 const ecma_property_descriptor_t *property_desc_p);
bool ecma_op_object_get_own_property_descriptor (ecma_object_t *object_p, ecma_string_t *property_name_p,
                                                 ecma_property_descriptor_t *prop_desc_p);
ecma_value_t ecma_op_object_has_instance (ecma_object_t *obj_p, ecma_value_t value);
bool ecma_op_object_is_prototype_of (ecma_object_t *base_p, ecma_object_t *target_p);
ecma_collection_t * ecma_op_object_get_property_names (ecma_object_t *obj_p, uint32_t opts);

lit_magic_string_id_t ecma_object_get_class_name (ecma_object_t *obj_p);
bool ecma_object_class_is (ecma_object_t *object_p, uint32_t class_id);
bool ecma_object_is_regexp_object (ecma_value_t arg);
#if ENABLED (JERRY_ES2015)
ecma_value_t ecma_op_is_concat_spreadable (ecma_value_t arg);
ecma_value_t ecma_op_is_regexp (ecma_value_t arg);
ecma_value_t ecma_op_species_constructor (ecma_object_t *this_value, ecma_builtin_id_t default_constructor_id);
#endif /* ENABLED (JERRY_ES2015) */

/**
 * @}
 * @}
 */

#endif /* !ECMA_OBJECTS_H */
