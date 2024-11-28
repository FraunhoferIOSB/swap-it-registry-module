/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "include/start_registry_module.h"
#include "include/start_swap_server.h"
#include "include/start_client.h"
#include "include/evaluate_test_results.h"
#include <stdlib.h>
#include <check.h>

#define nbr_target_nodes 18
size_t nbr_target_vals = nbr_target_nodes;


START_TEST(check_structure)
{
    UA_Boolean running = UA_TRUE;
    UA_StatusCode retval;
    //run registry module thread
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
    /*Set target browse names of nodes that should be mapped to the registry module*/
    UA_QualifiedName names[nbr_target_nodes] = {
        UA_QUALIFIEDNAME(2, "Location"),
        UA_QUALIFIEDNAME(2, "AssetState"),
        UA_QUALIFIEDNAME(2, "State"),
        UA_QUALIFIEDNAME(2, "ServiceQueue"),
        UA_QUALIFIEDNAME(2, "add_queue_element"),
        UA_QUALIFIEDNAME(2, "queue_variable"),
        UA_QUALIFIEDNAME(2, "remove_queue_element"),
        UA_QUALIFIEDNAME(2, "Capabilities"),
        UA_QUALIFIEDNAME(0, "test_boolean"),
        UA_QUALIFIEDNAME(0, "test_numeric"),
        UA_QUALIFIEDNAME(0, "test_string"),
        UA_QUALIFIEDNAME(0, "WarehouseModule"),
        UA_QUALIFIEDNAME(0, "Status"),
        UA_QUALIFIEDNAME(0, "opc.tcp://localhost:4080"),
        UA_QUALIFIEDNAME(0, "Address"),
        UA_QUALIFIEDNAME(0, "Port"),
        UA_QUALIFIEDNAME(0, "InputArguments")
    };

    /*browse the registry module*/
    UA_Client_get_node *handler = (UA_Client_get_node*) UA_calloc(1, sizeof(UA_Client_get_node));
    handler->client = start_client_connection(aggr_config.server, "opc.tcp://localhost:8000");
    handler->size = 1;
    handler->ctr = 0;
    handler->names = (UA_QualifiedName*) UA_Array_new(1, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    UA_QualifiedName target_name = UA_QUALIFIEDNAME(0, "GetPartsFromWarehouse");
    retval = UA_QualifiedName_copy(&target_name, &handler->names[0]);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    handler->target_id = (UA_NodeId*) UA_Array_new(handler->size, &UA_TYPES[UA_TYPES_NODEID]);
    retval = UA_Client_forEachChildNodeCall(handler->client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), find_single_node, handler);
    ck_assert(retval == UA_STATUSCODE_GOOD);

    UA_String out = UA_STRING_NULL;
    UA_print(&handler->target_id[0], &UA_TYPES[UA_TYPES_NODEID], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "GetPartsFromWarehouse NodeId %.*s ", (int)out.length, out.data);
    UA_String_clear(&out);

    for(size_t i=0; i< handler->ctr; i++ ){
        UA_Client_get_aggregated_nodes aggregated_nodes;
        aggregated_nodes.client = handler->client;
        aggregated_nodes.ctr = 0;
        aggregated_nodes.names = (UA_QualifiedName*) UA_Array_new(handler->size, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
        retval = UA_Client_forEachChildNodeCall(handler->client, handler->target_id[i], browse_aggregated_nodes, &aggregated_nodes);
        ck_assert(retval == UA_STATUSCODE_GOOD);
        for(size_t j=0; j< aggregated_nodes.ctr; j++){
            ck_assert(check_name_list(names, aggregated_nodes.names[j], nbr_target_vals) == 1);
        }
        UA_Array_delete(aggregated_nodes.names, aggregated_nodes.ctr, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    }
    sleep(20);
    UA_Array_delete((void*) handler->names, 1, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    UA_Array_delete((void*)handler->target_id, handler->size, &UA_TYPES[UA_TYPES_NODEID]);
    running = UA_FALSE;
    sleep(20);
    free(handler);
    sleep(20);
}

Suite * aggregation_server_suite(void){
    Suite *s;
    TCase *tc_core;
    s = suite_create("Aggregation Server");
    /* Core test case */
    tc_core = tcase_create("Check Aggregation Server Structure");
    tcase_set_timeout(tc_core, 300);
    tcase_add_test(tc_core, check_structure);
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
}
