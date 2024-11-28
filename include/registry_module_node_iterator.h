/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_BROWSE_NAMESPACE_H_
#define REGISTRY_MODULE_BROWSE_NAMESPACE_H_
#include <stdio.h>
#include "registry_module_synchronization.h"

typedef struct{
    UA_Server *server;
    UA_NodeId objectTypeId;
    UA_QualifiedName objectName;
}UA_nodeIdFinder;

typedef struct UA_AggregateNodeId{
    UA_NodeId nodeId;
    UA_NodeId nodeIdOnAggregationServer;
    TAILQ_ENTRY(UA_AggregateNodeId) listEntry;
} UA_AggregateNodeId;

typedef struct {
    UA_Server *server;
    UA_Client *client;
    UA_NodeId parent;
    UA_NodeId remoteParentObject;
    UA_AggregateConfig *aggregateConfig;
    size_t currentMappingIndex;
} internalNodeIteratorHandle;

typedef struct{
    UA_QualifiedName name;
    UA_Client *client;
}UA_ext_node_info;

UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);

#endif //REGISTRY_MODULE_BROWSE_NAMESPACE_H_
