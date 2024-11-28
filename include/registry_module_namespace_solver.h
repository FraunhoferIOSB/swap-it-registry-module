/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_NAMESPACE_SOLVER_H_
#define REGISTRY_MODULE_NAMESPACE_SOLVER_H_

#include <open62541/types.h>
#include <open62541/server.h>
#include <open62541/client_highlevel.h>
#include <stdio.h>

typedef struct{
    UA_Client *client;
    UA_NodeId id;
    UA_QualifiedName qname;
}UA_Client_get_Id;

typedef struct{
    UA_Client *client;
    UA_UInt16 namespaceIndex;
    char *target_node_name;
    UA_NodeId curr_id;
}UA_NamespaceidxMapping;

typedef struct{
    UA_Server *server;
    UA_NodeId id;
    UA_QualifiedName name;
}UA_getObjectTypeNode;

typedef struct{
    UA_Client *client;
    UA_QualifiedName name;
    UA_NodeId id;
}UA_getBrowseName;

UA_StatusCode get_namespace(UA_Client *client, UA_Server *server, UA_NamespaceidxMapping *index_finder, UA_UInt16 *namespaceidx);
UA_StatusCode write_variable_value(UA_Server *server, UA_Variant *content, UA_UInt16 namespaceindex, UA_NodeId newNode, UA_NodeId data_type_id);
UA_StatusCode get_ObjectType_NodeId(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode get_namespace_idx_from_config_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_UInt16 map_namespace_idx(UA_Server *server, UA_Client *client, UA_UInt16 currentIdentifier);
UA_StatusCode client_get_data_type_nodeId(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);

#endif //REGISTRY_MODULE_NAMESPACE_SOLVER_H_
