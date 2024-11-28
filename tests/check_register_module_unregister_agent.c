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

#define nbr_server 4;
size_t ctr = nbr_server

START_TEST(check_unregister_agent)
{
    UA_StatusCode retval;
    UA_Boolean running = UA_TRUE;
    //run aggregation server thread
    test_registry_module_config aggr_config;
    aggr_config.running = &running;
    start_registry_module_server_thread(&aggr_config);
    sleep(20);

    //get aggregation_server_nodes
    UA_Client_get_node *aggregation = (UA_Client_get_node*) UA_calloc(1, sizeof(UA_Client_get_node));
    aggregation->client = start_client_connection(aggr_config.server, "opc.tcp://localhost:8000");

    //get registry module nodes
    aggregation->size = 1;
    aggregation->ctr = 0;
    aggregation->names = UA_QualifiedName_new();
    UA_QualifiedName qname = UA_QUALIFIEDNAME(0, "PFDLServiceAgents");
    retval = UA_QualifiedName_copy(&qname, aggregation->names);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    aggregation->target_id = UA_Array_new(aggregation->size, &UA_TYPES[UA_TYPES_NODEID]);
    retval = UA_Client_forEachChildNodeCall(aggregation->client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), find_single_node, aggregation);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_NodeId service_object, add_agent, remove_agent;
    retval = UA_NodeId_copy(aggregation->target_id ,&service_object);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_QualifiedName_clear(aggregation->names);
    UA_NodeId_clear(&aggregation->target_id[0]);
    retval = browse_single_variable(aggregation, &add_agent, service_object, UA_QUALIFIEDNAME(1, "Add_Agent_Server"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_QualifiedName_clear(aggregation->names);
    UA_NodeId_clear(&aggregation->target_id[0]);
    retval = browse_single_variable(aggregation, &remove_agent, service_object, UA_QUALIFIEDNAME(1, "Remove_Agent_Server"));
    ck_assert(retval == UA_STATUSCODE_GOOD);

    /*run a swap server that registers itself in the registry module*/
    test_swap_server_config aggregate_conf_1;
    aggregate_conf_1.register_server = false;
    aggregate_conf_1.running = &running;
    aggregate_conf_1.conf = "../../../tests/configs/warehouse.json5";
    start_swap_server_thread(&aggregate_conf_1);

    test_swap_server_config aggregate_conf_2;
    aggregate_conf_2.register_server = false;
    aggregate_conf_2.running = &running;
    aggregate_conf_2.conf = "../../../tests/configs/warehouse_1.json5";
    start_swap_server_thread(&aggregate_conf_2);

    test_swap_server_config aggregate_conf_3;
    aggregate_conf_3.register_server = false;
    aggregate_conf_3.running = &running;
    aggregate_conf_3.conf = "../../../tests/configs/warehouse_2.json5";
    start_swap_server_thread(&aggregate_conf_3);

    test_swap_server_config aggregate_conf_4;
    aggregate_conf_4.register_server = false;
    aggregate_conf_4.running = &running;
    aggregate_conf_4.conf = "../../../tests/configs/warehouse_3.json5";
    start_swap_server_thread(&aggregate_conf_4);
    sleep(25);
    for(size_t i = 0; i < ctr; i++){
        /*create input values*/
        UA_Variant *inp = (UA_Variant*) UA_Array_new(4, &UA_TYPES[UA_TYPES_VARIANT]);
        UA_String address = UA_String_fromChars("localhost");
        UA_String service_name = UA_String_fromChars("GetPartsFromWarehouse");
        UA_String moduleType  =  UA_String_fromChars("WarehouseModule");
        int port_nbr = 4080 +(int) i;
        char server_port[5];
        sprintf(server_port, "%d" , port_nbr);
        server_port[4] = '\0';
        UA_String port = UA_String_fromChars(server_port);
        UA_Variant_setScalarCopy(&inp[0], &service_name, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&inp[1], &address, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&inp[2], &port, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&inp[3], &moduleType, &UA_TYPES[UA_TYPES_STRING]);
        retval = UA_Client_call(aggregation->client, service_object, add_agent, 4, inp, NULL, NULL);
        printf("called the service method with statuscode %s \n", UA_StatusCode_name(retval));
        ck_assert(retval == UA_STATUSCODE_GOOD);
        UA_String_clear(&service_name);
        UA_String_clear(&address);
        UA_String_clear(&port);
        UA_String_clear(&moduleType);
        UA_Array_delete(inp, 4, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    /*check if all agents are added*/
    UA_Client_get_aggregated_nodes aggregated_nodes;
    aggregated_nodes.client = aggregation->client;
    aggregated_nodes.ctr = 0;
    aggregated_nodes.names = (UA_QualifiedName*) UA_Array_new(0, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    retval = UA_Client_forEachChildNodeCall(aggregated_nodes.client, service_object, browse_aggregated_nodes, &aggregated_nodes);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    ck_assert(true == check_registered_agents(aggregated_nodes.names, aggregated_nodes.ctr, UA_QUALIFIEDNAME(2, "AssetState"), 4));
    /*unregister the agents*/
    for(size_t i = 0; i < ctr; i++){
        UA_Variant *inp = (UA_Variant*) UA_Array_new(3, &UA_TYPES[UA_TYPES_VARIANT]);
        UA_String address = UA_String_fromChars("localhost");
        UA_String service_name = UA_String_fromChars("GetPartsFromWarehouse");
        int port_nbr = 4080 +(int) i;
        char server_port[5];
        sprintf(server_port, "%d" , port_nbr);
        server_port[4] = '\0';
        UA_String port = UA_String_fromChars(server_port);
        UA_Variant_setScalarCopy(&inp[2], &service_name, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&inp[0], &address, &UA_TYPES[UA_TYPES_STRING]);
        UA_Variant_setScalarCopy(&inp[1], &port, &UA_TYPES[UA_TYPES_STRING]);
        retval = UA_Client_call(aggregation->client, service_object, remove_agent, 3, inp, NULL, NULL);
        printf("called the service method with statuscode %s \n", UA_StatusCode_name(retval));
        ck_assert(retval == UA_STATUSCODE_GOOD);
        UA_String_clear(&service_name);
        UA_String_clear(&address);
        UA_String_clear(&port);
        UA_Array_delete(inp, 3, &UA_TYPES[UA_TYPES_VARIANT]);
    }
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
    tcase_add_test(tc_core, check_unregister_agent);
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
