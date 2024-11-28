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
#include "include/evaluate_test_results.h"
#include "include/start_swap_server.h"
#include "types_common_generated.h"
#include "types_common_generated_handling.h"
#include <stdlib.h>
#include <check.h>

START_TEST(check_registry_module_read_write)
{
    UA_Boolean running = UA_TRUE;
    UA_StatusCode retval;
    //run aggregation server thread
    test_registry_module_config aggr_config;
    aggr_config.running = &running;
    start_registry_module_server_thread(&aggr_config);
    sleep(20);
    /*run a swap server that registers itself in the registry module*/
    test_swap_server_config aggregate_conf;
    aggregate_conf.register_server = true;
    aggregate_conf.running = &running;
    aggregate_conf.conf = "../../../tests/configs/warehouse.json5";
    start_swap_server_thread(&aggregate_conf);
    sleep(20);

    /*get target nodeIds from the swap server 1 inside the registry module*/
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
    UA_NodeId get_parts_obj, state_var, test_num_var, loc_var;
    retval = UA_NodeId_copy(aggregation->target_id ,&get_parts_obj);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    UA_QualifiedName_clear(aggregation->names);
    UA_NodeId_clear(&aggregation->target_id[0]);
    /*get the nodeid of the state variable*/
    retval = browse_single_variable(aggregation, &state_var, get_parts_obj, UA_QUALIFIEDNAME(2, "AssetState"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the location variable*/
    retval = browse_single_variable(aggregation, &loc_var, get_parts_obj, UA_QUALIFIEDNAME(2, "Location"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    /*get the nodeid of the test_numeric variable*/
    retval = browse_single_variable(aggregation, &test_num_var, get_parts_obj, UA_QUALIFIEDNAME(0, "test_numeric"));
    ck_assert(retval == UA_STATUSCODE_GOOD);

    /*get swap server nodes*/
    UA_Client_get_node *swap_server = (UA_Client_get_node*) UA_calloc(1, sizeof(UA_Client_get_node));
    swap_server->client = start_client_connection(aggr_config.server, "opc.tcp://localhost:4080");
    //get aggregate1_server_nodes
    swap_server->size = 1;
    swap_server->ctr = 0;
    swap_server->names = UA_QualifiedName_new();
    swap_server->target_id = UA_NodeId_new();
    UA_NodeId swap_state_var, swap_test_num_var, swap_loc_var;
    retval = browse_single_variable(swap_server, &swap_test_num_var, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(0, "test_numeric"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = browse_single_variable(swap_server, &swap_loc_var, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "Location"));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    retval = browse_single_variable(swap_server, &swap_state_var, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), UA_QUALIFIEDNAME(2, "AssetState"));
    ck_assert(retval == UA_STATUSCODE_GOOD);

    /*Write the location variable in the swap server*/
    UA_String val_1 = UA_String_fromChars("Test_Value_swap_server_1");
    UA_String val_2 = UA_String_fromChars("Test_Value_registry_module_1");
    UA_Variant val_test1, val_test2;
    UA_Variant_init(&val_test1);
    UA_Variant_init(&val_test2);
    UA_Variant_setScalarCopy(&val_test1, &val_1, &UA_TYPES[UA_TYPES_STRING]);
    UA_Variant_setScalarCopy(&val_test2, &val_2, &UA_TYPES[UA_TYPES_STRING]);
    printf("test result %d \n", check_string(swap_server->client, aggregation->client, val_test1,swap_loc_var,loc_var, false));
    ck_assert_int_eq(check_string(swap_server->client, aggregation->client, val_test1,swap_loc_var,loc_var, false), 1);
    sleep(20);
    printf("test result %d \n", check_string(swap_server->client, aggregation->client, val_test2,swap_loc_var,loc_var, true));
    ck_assert_int_eq(check_string(swap_server->client, aggregation->client, val_test2,swap_loc_var,loc_var, true), 0);
    sleep(20);
    printf("test result %d \n", check_string(swap_server->client, aggregation->client, val_test1,swap_loc_var,loc_var, false));
    ck_assert_int_eq(check_string(swap_server->client, aggregation->client, val_test1,swap_loc_var,loc_var, false), 0);
    sleep(20);
    UA_String_clear(&val_1);
    UA_String_clear(&val_2);
    UA_Variant_clear(&val_test1);
    UA_Variant_clear(&val_test2);

    /*Write the state variable in the registry module server*/
    UA_AssetStateType val_1_state = UA_ASSETSTATETYPE_ASSET_STATE_ERROR;
    UA_AssetStateType val_2_state = UA_ASSETSTATETYPE_ASSET_STATE_OPERATIONAL;
    UA_Variant_init(&val_test1);
    UA_Variant_init(&val_test2);
    UA_Variant_setScalarCopy(&val_test1, &val_1_state, &UA_TYPES_COMMON[UA_TYPES_COMMON_ASSETSTATETYPE]);
    UA_Variant_setScalarCopy(&val_test2, &val_2_state, &UA_TYPES_COMMON[UA_TYPES_COMMON_ASSETSTATETYPE]);
    printf("test result %d \n", check_state(swap_server->client, aggregation->client, val_test1,swap_state_var,state_var, true));
    ck_assert_int_eq(check_state(swap_server->client, aggregation->client, val_test1,swap_state_var,state_var, true), 0);
    sleep(20);
    printf("test result %d \n", check_state(swap_server->client, aggregation->client, val_test2,swap_state_var,state_var, false));
    ck_assert_int_eq(check_state(swap_server->client, aggregation->client, val_test2,swap_state_var,state_var, false), 1);
    sleep(20);
    printf("test result %d \n", check_state(swap_server->client, aggregation->client, val_test1,swap_state_var,state_var, true));
    ck_assert_int_eq(check_state(swap_server->client, aggregation->client, val_test1,swap_state_var,state_var, true), 0);
    sleep(20);
    UA_Variant_clear(&val_test1);
    UA_Variant_clear(&val_test2);

    /*Write the test_num variable in the swap server*/
    UA_Capability_Struct_Number val_1_capa;
    UA_Capability_Struct_Number val_2_capa;
    val_1_capa.value = 62541;
    val_1_capa.relational_Operator = 1;
    val_2_capa.value = 14526;
    val_2_capa.relational_Operator = 6;
    UA_Variant_init(&val_test1);
    UA_Variant_init(&val_test2);
    UA_Variant_setScalarCopy(&val_test1, &val_1_capa, &UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_NUMBER]);
    UA_Variant_setScalarCopy(&val_test2, &val_2_capa, &UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_NUMBER]);
    printf("test result %d \n", check_num_capa(swap_server->client, aggregation->client, val_test1,swap_test_num_var,test_num_var, false));
    ck_assert_int_eq(check_num_capa(swap_server->client, aggregation->client, val_test1,swap_test_num_var,test_num_var, false), 1);
    sleep(20);
    printf("test result %d \n", check_num_capa(swap_server->client, aggregation->client, val_test2,swap_test_num_var,test_num_var, true));
    ck_assert_int_eq(check_num_capa(swap_server->client, aggregation->client, val_test2,swap_test_num_var,test_num_var, true), 0);
    sleep(20);
    printf("test result %d \n", check_num_capa(swap_server->client, aggregation->client, val_test1,swap_test_num_var,test_num_var, false));
    ck_assert_int_eq(check_num_capa(swap_server->client, aggregation->client, val_test1,swap_test_num_var,test_num_var, false), 0);
    sleep(20);
    UA_Variant_clear(&val_test1);
    UA_Variant_clear(&val_test2);
    running = UA_FALSE;
    sleep(20);
    free(aggregation);
}

Suite * aggregation_server_suite(void){
    Suite *s;
    TCase *tc_core;
    s = suite_create("Registry Module");
    /* Core test case */
    tc_core = tcase_create("Registry Module");
    tcase_set_timeout(tc_core, 300);
    tcase_add_test(tc_core, check_registry_module_read_write);
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

