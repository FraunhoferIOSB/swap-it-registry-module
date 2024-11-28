/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "include/start_registry_module.h"
#include "include/start_client.h"
#include "include/start_swap_server.h"
#include "include/evaluate_test_results.h"
#include <stdlib.h>
#include <check.h>

START_TEST(check_simple_method_call)
{
    UA_StatusCode retval;
    UA_Boolean running = UA_TRUE;
    //run aggregation server thread
    test_registry_module_config aggr_config;
    aggr_config.running = &running;
    start_registry_module_server_thread(&aggr_config);
    sleep(20);
    /*run a swap server that registers itself in the registry module*/
    test_swap_server_config aggregate_conf;
    aggregate_conf.conf = "../../../tests/configs/warehouse.json5";
    aggregate_conf.register_server = true;
    aggregate_conf.running = &running;
    start_swap_server_thread(&aggregate_conf);
    sleep(20);

    //get aggregation_server_nodes
    UA_Client_get_node *aggregation = (UA_Client_get_node*) UA_calloc(1, sizeof(UA_Client_get_node));
    aggregation->client = start_client_connection(aggr_config.server, "opc.tcp://localhost:8000");

    //get registry module nodes
    aggregation->size = 1;
    aggregation->ctr = 0;
    aggregation->names = UA_QualifiedName_new();
    UA_QualifiedName qname = UA_QUALIFIEDNAME(0, "GetPartsFromWarehouse");
    retval = UA_QualifiedName_copy(&qname, aggregation->names);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    aggregation->target_id = UA_Array_new(aggregation->size, &UA_TYPES[UA_TYPES_NODEID]);
    retval = UA_Client_forEachChildNodeCall(aggregation->client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), find_single_node, aggregation);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_NodeId service_object, add_queue, remove_queue, queue_var, get_parts_obj;
    retval = UA_NodeId_copy(aggregation->target_id ,&get_parts_obj);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_QualifiedName_clear(aggregation->names);
    UA_NodeId_clear(&aggregation->target_id[0]);
    /*get the nodeid of the state variable*/
    retval = browse_single_variable(aggregation, &service_object, get_parts_obj, UA_QUALIFIEDNAME(2, "ServiceQueue"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the location variable*/
    retval = browse_single_variable(aggregation, &add_queue, get_parts_obj, UA_QUALIFIEDNAME(2, "add_queue_element"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the test_numeric variable*/
    retval = browse_single_variable(aggregation, &remove_queue, get_parts_obj, UA_QUALIFIEDNAME(2, "remove_queue_element"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = browse_single_variable(aggregation, &queue_var, get_parts_obj, UA_QUALIFIEDNAME(2, "queue_variable"));
    ck_assert(retval == UA_STATUSCODE_GOOD);

    /*get swap server nodes*/
    UA_Client_get_node *swap_server = (UA_Client_get_node*) UA_calloc(1, sizeof(UA_Client_get_node));
    swap_server->client = start_client_connection(aggr_config.server, "opc.tcp://localhost:4080");
    //get aggregate1_server_nodes
    swap_server->size = 1;
    swap_server->ctr = 0;
    swap_server->names = UA_QualifiedName_new();
    swap_server->target_id = UA_NodeId_new();
    UA_NodeId swap_service_object, swap_add_queue, swap_remove_queue, swap_queue_var;
    retval = browse_single_variable(swap_server, &swap_service_object, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "ServiceQueue"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the location variable*/
    retval = browse_single_variable(swap_server, &swap_add_queue, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "add_queue_element"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the test_numeric variable*/
    retval = browse_single_variable(swap_server, &swap_remove_queue, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "remove_queue_element"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = browse_single_variable(swap_server, &swap_queue_var, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "queue_variable"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    sleep(20);

    /*check the method calls from the queue variables*/
    UA_Queue_Data_Type val_1, val_2;
    UA_Queue_Data_Type_init(&val_1);
    UA_Queue_Data_Type_init(&val_2);
    val_1.client_Identifier = UA_String_fromChars("1");
    val_1.service_UUID = UA_String_fromChars("1");
    val_1.queue_Element_State = UA_QUEUE_STATE_VARIABLE_TYPE_READY_FOR_EXECUTION;
    val_1.serviceParameterSize = 0;
    val_1.serviceParameter = UA_DataValue_new();

    val_2.client_Identifier = UA_String_fromChars("2");
    val_2.service_UUID = UA_String_fromChars("2");
    val_2.queue_Element_State = UA_QUEUE_STATE_VARIABLE_TYPE_WAITING_FOR_EXECUTION;
    val_2.serviceParameterSize = 0;
    val_2.serviceParameter = UA_DataValue_new();

    UA_Variant val_test1, val_test2;
    UA_Variant_init(&val_test1);
    UA_Variant_init(&val_test2);
    UA_Variant_setScalarCopy(&val_test1, &val_1, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);
    UA_Variant_setScalarCopy(&val_test2, &val_2, &UA_TYPES_COMMON[UA_TYPES_COMMON_QUEUE_DATA_TYPE]);

    /*1: add a queue element from the registry and read new values in both servers*/
    retval = UA_Client_call(aggregation->client, service_object, add_queue, 1, &val_test1, 0, NULL);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    sleep(20);
    UA_Variant out_val_swap, out_val_reg;
    UA_Variant_init(&out_val_swap);
    UA_Variant_init(&out_val_reg);
    retval = UA_Client_readValueAttribute(swap_server->client, swap_queue_var, &out_val_swap);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = UA_Client_readValueAttribute(aggregation->client, queue_var, &out_val_reg);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    check_queue(val_1, *(UA_Queue_Data_Type*) out_val_swap.data, 1, true);
    check_queue(*(UA_Queue_Data_Type*) out_val_reg.data, *(UA_Queue_Data_Type*) out_val_swap.data, 1, false);

    /*2: add a queue element from the swap_server read new values in both servers*/
    UA_Variant_clear(&out_val_swap);
    UA_Variant_clear(&out_val_reg);
    UA_Variant_init(&out_val_reg);
    UA_Variant_init(&out_val_reg);
    retval = UA_Client_call(swap_server->client, swap_service_object, swap_add_queue, 1, &val_test2, 0, NULL);
    printf("Check 2 \n");
    ck_assert(retval == UA_STATUSCODE_GOOD);
    sleep(20);
    retval = UA_Client_readValueAttribute(swap_server->client, swap_queue_var, &out_val_swap);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = UA_Client_readValueAttribute(aggregation->client, queue_var, &out_val_reg);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_Queue_Data_Type *reg_queue = (UA_Queue_Data_Type*) out_val_reg.data;
    UA_Queue_Data_Type *swap_queue = (UA_Queue_Data_Type*) out_val_swap.data;

    check_queue(val_1,  reg_queue[0], 1, true);
    check_queue(val_2,  reg_queue[1], 2, true);
    check_queue(reg_queue[0], swap_queue[0], 1, false);
    check_queue(reg_queue[1], swap_queue[1], 2, false);

    /*3: remove a queue element from the registry read new values in both servers*/
    retval = UA_Client_call(swap_server->client, swap_service_object, swap_remove_queue, 1, &val_test2, 0, NULL);
    printf("Check 3 \n");
    ck_assert(retval == UA_STATUSCODE_GOOD);
    sleep(20);
    UA_Variant_clear(&out_val_swap);
    UA_Variant_clear(&out_val_reg);
    retval = UA_Client_readValueAttribute(swap_server->client, swap_queue_var, &out_val_swap);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = UA_Client_readValueAttribute(aggregation->client, queue_var, &out_val_reg);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    check_queue(val_1, *(UA_Queue_Data_Type*) out_val_swap.data, 1, true);
    check_queue(*(UA_Queue_Data_Type*) out_val_reg.data, *(UA_Queue_Data_Type*) out_val_swap.data, 1, false);

    /*4: remove a queue element from the swap_server read new values in both servers*/
    retval = UA_Client_call(aggregation->client, service_object, remove_queue, 1, &val_test1, 0, NULL);
    printf("Check 4 \n");
    ck_assert(retval == UA_STATUSCODE_GOOD);
    sleep(20);
    UA_Variant_clear(&out_val_swap);
    UA_Variant_clear(&out_val_reg);
    retval = UA_Client_readValueAttribute(swap_server->client, swap_queue_var, &out_val_swap);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = UA_Client_readValueAttribute(aggregation->client, queue_var, &out_val_reg);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    reg_queue = (UA_Queue_Data_Type*) out_val_reg.data;
    swap_queue = (UA_Queue_Data_Type*) out_val_swap.data;
    ck_assert(UA_String_equal(&reg_queue->client_Identifier, &swap_queue->client_Identifier) == true);
    ck_assert(UA_String_equal(&reg_queue->service_UUID, &swap_queue->service_UUID) == true);
    ck_assert(UA_String_equal(&reg_queue->productId, &swap_queue->productId) == true);
    ck_assert(reg_queue->entry_Number == 0);
    ck_assert(reg_queue->queue_Element_State == 0);
    ck_assert(swap_queue->queue_Element_State == 0);
    ck_assert(swap_queue->entry_Number == 0);
    sleep(20);
    running = false;
    free(aggregation);
    sleep(20);

}

Suite * aggregation_server_suite(void){
    Suite *s;
    TCase *tc_core;
    s = suite_create("Aggregation Server");
    /* Core test case */
    tc_core = tcase_create("Check Aggregation Server Structure");
    tcase_set_timeout(tc_core, 300);
    tcase_add_test(tc_core, check_simple_method_call);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;
    s = aggregation_server_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    //check();
}

