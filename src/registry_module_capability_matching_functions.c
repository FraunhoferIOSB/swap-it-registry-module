/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_capability_matching_functions.h"
#include <stdio.h>

static void string_matching(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = UA_String_equal((UA_String*) resource_value.data, (UA_String*) product_value.data);
}

static void number_smaller(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = *(UA_Double*) resource_value.data > *(UA_Double*) product_value.data ? 1 : 0;
}

static void number_smaller_or_equal(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = *(UA_Double*) resource_value.data >= *(UA_Double*) product_value.data ? 1 : 0;
}

static void number_equal(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = *(UA_Double*) resource_value.data == *(UA_Double*) product_value.data ? 1 : 0;
}

static void number_greater_or_equal(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = *(UA_Double*) resource_value.data <= *(UA_Double*) product_value.data ? true : false;
}

static void number_greater(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    *agent_is_suitable = *(UA_Double*) resource_value.data < *(UA_Double*) product_value.data ? true : false;
}

static void Is_true(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    UA_Boolean res_value = *(UA_Boolean*) resource_value.data;
    if(res_value == UA_TRUE){
        UA_Boolean prod_value = *(UA_Boolean*) product_value.data;
        if( prod_value == true){
            *agent_is_suitable = true;
        }
        else{
            *agent_is_suitable = false;
        }
    }
    else{
        *agent_is_suitable = false;
    }

}

static void Is_false(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable){
    UA_Boolean res_value = *(UA_Boolean*) resource_value.data;
    UA_Boolean prod_value = *(UA_Boolean*) product_value.data;
    if(res_value == UA_FALSE) {
        if (prod_value == false) {
            *agent_is_suitable = true;
        } else {
            *agent_is_suitable = false;
        }
    }
    else{
        *agent_is_suitable = false;
    }
}

void(* matching_functions[])(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable) = {
        number_equal,
        number_greater,
        number_smaller,
        number_greater_or_equal,
        number_smaller_or_equal,
        Is_true,
        Is_false,
        string_matching};

