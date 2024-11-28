/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */

#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>
#include "stdio.h"
#include "deps/cj5.h"
#include "deps/open62541_queue.h"

#include "namespace_common_generated.h"
#include "types_common_generated.h"
#include "include/registry_module_clear.h"
#include "include/registry_module_init.h"
#include "include/registry_module_register_agent.h"

UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_agent_list *asc = UA_calloc(1, sizeof(UA_agent_list));
    UA_Server *server =  start_registry_module(asc);

    while(running){
        UA_Server_run_iterate(server, false);
        for (size_t i =0; i < asc->number_agents; i++){
            syncRegisteredAgents(server, &asc->agents[i]);
        }
    }

    UA_StatusCode retval = clear_registry_module(server, asc);
    free(asc);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}