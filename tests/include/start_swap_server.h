/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef START_SWAP_SERVER_H
#define START_SWAP_SERVER_H

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <signal.h>

/*typedef struct{
    UA_NodeId id;
    UA_Server *server;
    UA_QualifiedName name;
}UA_Server_find_Node;*/

typedef struct{
    UA_Boolean register_server;
    UA_Boolean *running;
    pthread_t threadId;
    UA_Server *server;
    char *conf;
}test_swap_server_config;

void start_swap_server_thread(test_swap_server_config *aggregate_conf);

#endif //START_SWAP_SERVER_H
