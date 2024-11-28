/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef OPEN62541_REGISTER_AGENT_H_
#define OPEN62541_REGISTER_AGENT_H_

#include <open62541/plugin/log_stdout.h>
#include <open62541/client_config_default.h>
#include "registry_module_node_iterator.h"
#include "registry_module_internal.h"
#include "registry_module_config_parse.h"
#include "registry_module_client_interface.h"

UA_StatusCode search_service_object(UA_NodeId childId, UA_Boolean reference, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode write_assignment_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode find_assignment_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
void create_server_address(char *address, char *port, UA_String *agent_list_url);
void add_agent_server(UA_Server *server, char *service_name, char *address, char *port, char *moduleType, UA_agent_list *asc);
UA_StatusCode initRegisteredAgentNodes(UA_Server * server, UA_AggregationServerConfig *asc, UA_NodeId parent_nodeId, char *ModuleTypeName);

#endif //OPEN62541_REGISTER_AGENT_H_
