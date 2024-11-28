/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include <stdlib.h>
#include <check.h>
#include "../include/registry_module_init.h"
#include "../deps/common.h"
#include "../include/registry_module_clear.h"

START_TEST(parse_empty_config)
{
    UA_ByteString aggregationServerConfiguration = UA_BYTESTRING_NULL;
    aggregationServerConfiguration = loadFile("config_empty.json5");
    UA_agent_list asc;
    UA_Server *server = start_registry_module(&asc);
    *asc.agent_url = UA_String_fromChars("test");
    asc.number_agents = 1;
    UA_StatusCode retVal = loadRegisterAgentConfig(server, aggregationServerConfiguration, asc.agents);
    ck_assert_int_eq(UA_STATUSCODE_BADCONFIGURATIONERROR, retVal);
    clear_registry_module(server, &asc);
    UA_ByteString_clear(&aggregationServerConfiguration);
} END_TEST

START_TEST(parse_incomplete_config){
    UA_ByteString aggregationServerConfiguration = UA_BYTESTRING_NULL;
    aggregationServerConfiguration = loadFile("config_incomplete.json5");
    UA_agent_list asc;
    UA_Server *server = start_registry_module(&asc);
    *asc.agent_url = UA_String_fromChars("test");
    asc.number_agents = 0;
    UA_StatusCode retVal = loadRegisterAgentConfig(server, aggregationServerConfiguration, asc.agents);
    printf("retval = %s \n", UA_StatusCode_name(retVal));
    printf("retval = %s \n", UA_StatusCode_name(UA_STATUSCODE_BADCONFIGURATIONERROR));
    ck_assert_int_eq(UA_STATUSCODE_BADCONFIGURATIONERROR, retVal);
    clear_registry_module(server, &asc);
    UA_ByteString_clear(&aggregationServerConfiguration);
} END_TEST

START_TEST(parse_valid_config){
    UA_ByteString aggregationServerConfiguration = UA_BYTESTRING_NULL;
    aggregationServerConfiguration = loadFile("config_valid.json5");
    UA_agent_list asc;
    UA_Server *server = start_registry_module(&asc);
    *asc.agent_url = UA_String_fromChars("test");
    asc.number_agents = 0;
    UA_StatusCode retVal = loadRegisterAgentConfig(server, aggregationServerConfiguration, asc.agents);
    printf("retval = %s \n", UA_StatusCode_name(retVal));
    ck_assert(UA_STATUSCODE_GOOD == retVal);
    clear_registry_module(server, &asc);
    UA_ByteString_clear(&aggregationServerConfiguration);
} END_TEST

Suite * aggregation_server_suite(void){
    Suite *s;
    TCase *tc_core;
    s = suite_create("Aggregation Server");
    /* Core test case */
    tc_core = tcase_create("Config Parse");
    tcase_set_timeout(tc_core, 300);
    tcase_add_test(tc_core, parse_empty_config);
    tcase_add_test(tc_core, parse_incomplete_config);
    //tcase_add_test(tc_core, parse_valid_config);
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
