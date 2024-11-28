/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_INTERNAL_H_
#define REGISTRY_MODULE_INTERNAL_H_

#include <open62541/plugin/log_stdout.h>
#include <open62541/client_config_default.h>
#include "registry_module_synchronization.h"
#include "registry_module_node_iterator.h"

typedef struct {
    UA_AggregateConfig *aggregateConfig;
    size_t aggregateConfigSize;
    UA_Duration subscription_interval;
    UA_Boolean unsecureAllowed;
    UA_Duration aggregateTimeout;
} UA_AggregationServerConfig;

typedef struct{
    UA_AggregationServerConfig *agents;
    UA_String *agent_url;
    size_t number_agents;
}UA_agent_list;

typedef struct{
    UA_Boolean service_node_found;
    UA_QualifiedName service_name;
    UA_NodeId service_object_nodeId;
    UA_Server *server;
}service_node;

typedef struct{
    UA_QualifiedName variable_name;
    UA_NodeId variable_nodeId;
    UA_Server *server;
}UA_get_variable;

void syncRegisteredAgents(UA_Server * server, UA_AggregationServerConfig *asc);
UA_StatusCode find_method(UA_Server *server, char *methodname, UA_NodeId *service_method_nodeId);
UA_StatusCode readRegisteredAgentNS(UA_Server *server, UA_AggregationServerConfig *asc, size_t aggregate_index);

#endif //REGISTRY_MODULE_INTERNAL_H_
