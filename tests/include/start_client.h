/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef START_CLIENT_H
#define START_CLIENT_H

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>
#include "open62541/client_highlevel.h"
#include "types_common_generated.h"
#include "types_common_generated_handling.h"

typedef struct{
    UA_Client *client;
    size_t size;
    size_t ctr;
    UA_QualifiedName *names;
    UA_NodeId *target_id;
}UA_Client_get_node;

typedef struct{
    UA_Client *client;
    size_t ctr;
    UA_QualifiedName *names;
}UA_Client_get_aggregated_nodes;

UA_Client *start_client_connection(UA_Server *server, const char* server_url);
UA_StatusCode find_single_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode browse_aggregated_nodes(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode browse_single_variable(UA_Client_get_node *aggregation, UA_NodeId *tar_var_id, UA_NodeId start_node_id, UA_QualifiedName qname);

#endif //START_CLIENT_H
