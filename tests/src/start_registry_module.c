/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/start_registry_module.h"
#include "../../deps/common.h"

void *start_registry_module_server(void *data) {
    test_registry_module_config *aggr_config = (test_registry_module_config *) data;
    UA_agent_list asc;
    aggr_config->server = start_registry_module(&asc);
    while(*(aggr_config->running) == UA_TRUE){
        UA_Server_run_iterate(aggr_config->server, false);
        for (size_t i =0; i < asc.number_agents; i++){
            syncRegisteredAgents(aggr_config->server, &asc.agents[i]);
        }
    }
    clear_registry_module(aggr_config->server, &asc);
    return UA_STATUSCODE_GOOD;
}

void start_registry_module_server_thread(test_registry_module_config *aggr_config){
    printf("run aggregation server thread\n");
    pthread_create(&aggr_config->threadId, NULL, start_registry_module_server, aggr_config);
}

