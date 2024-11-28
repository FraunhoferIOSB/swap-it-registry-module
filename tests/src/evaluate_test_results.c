/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/evaluate_test_results.h"
#include <check.h>
#include <stdio.h>
#include <types_common_generated_handling.h>
#include <unistd.h>

int check_name_list(UA_QualifiedName *sources, UA_QualifiedName target, size_t num_sources){
    for(size_t i=0; i<num_sources; i++)
        if(UA_QualifiedName_equal(&target, &sources[i]))
            return 1;
    return 0;
}

UA_Boolean check_string(UA_Client *aggregate_client, UA_Client *aggregation_client,
                        UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server){
    UA_Variant read_value_v;
    UA_StatusCode retval;
    UA_Variant_init(&read_value_v);
    if(write_aggregation_server){
        retval = UA_Client_writeValueAttribute(aggregation_client, aggregation_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        sleep(1);
        retval = UA_Client_readValueAttribute(aggregate_client, aggregate_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    else{
        retval = UA_Client_writeValueAttribute(aggregate_client, aggregate_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        /*todo check why the value sync requires that much time*/
        sleep(5/*20*/);
        retval = UA_Client_readValueAttribute(aggregation_client, aggregation_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    return UA_String_equal((UA_String*) value.data, (UA_String*) read_value_v.data);
}

UA_Boolean check_state(UA_Client *aggregate_client, UA_Client *aggregation_client,
                       UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server){
    UA_Variant read_value_v;
    UA_StatusCode retval;
    UA_Variant_init(&read_value_v);
    if(write_aggregation_server){
        retval = UA_Client_writeValueAttribute(aggregation_client, aggregation_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        sleep(1);
        retval = UA_Client_readValueAttribute(aggregate_client, aggregate_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    else{
        retval = UA_Client_writeValueAttribute(aggregate_client, aggregate_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        /*todo check why the value sync requires that much time*/
        sleep(5/*20*/);
        retval = UA_Client_readValueAttribute(aggregation_client, aggregation_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    return UA_AssetStateType_equal((UA_AssetStateType*) value.data, (UA_AssetStateType*) read_value_v.data);
}


UA_Boolean check_num_capa(UA_Client *aggregate_client, UA_Client *aggregation_client,
    UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server){
    UA_Variant read_value_v;
    UA_StatusCode retval;
    UA_Variant_init(&read_value_v);
    if(write_aggregation_server){
        retval = UA_Client_writeValueAttribute(aggregation_client, aggregation_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        sleep(1);
        retval = UA_Client_readValueAttribute(aggregate_client, aggregate_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    else{
        retval = UA_Client_writeValueAttribute(aggregate_client, aggregate_nodeId, &value);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
        /*todo check why the value sync requires that much time*/
        sleep(5/*20*/);
        retval = UA_Client_readValueAttribute(aggregation_client, aggregation_nodeId, &read_value_v);
        if(retval != UA_STATUSCODE_GOOD)
        {
            printf("failed to write the node with statuscode %s \n", UA_StatusCode_name(retval));
            return false;
        }
    }
    return UA_Capability_Struct_Number_equal((UA_Capability_Struct_Number*) value.data, (UA_Capability_Struct_Number*) read_value_v.data);
}
/*val_1 must be the initial value*/
void check_queue(UA_Queue_Data_Type val_1, UA_Queue_Data_Type val_2, UA_Int16 entry_number, UA_Boolean initial_value){
    ck_assert(UA_String_equal(&val_1.client_Identifier, &val_2.client_Identifier) == true);
    ck_assert(UA_String_equal(&val_1.service_UUID, &val_2.service_UUID) == true);
    ck_assert(UA_String_equal(&val_1.productId, &val_2.productId) == true);
    if(initial_value)
        ck_assert(val_2.entry_Number == entry_number);
    else
    {
        ck_assert(val_1.queue_Element_State == val_2.queue_Element_State);
        ck_assert(val_1.entry_Number == val_2.entry_Number);
    }
}

UA_Boolean check_registered_agents(UA_QualifiedName *nameList, size_t nameListSize, UA_QualifiedName targetName, size_t target_nbr){
    size_t ctr =0;
    for(size_t i = 0; i < nameListSize; i++){
        if(UA_QualifiedName_equal(&nameList[i], &targetName))
            ctr++;
        if (ctr == target_nbr)
            return true;
    }
    return false;
}