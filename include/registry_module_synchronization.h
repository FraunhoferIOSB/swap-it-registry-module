/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_SYNCHRONIZATION_H_
#define REGISTRY_MODULE_SYNCHRONIZATION_H_

#include <pthread.h>
#include "../deps/open62541_queue.h"
#include "registry_module_namespace_solver.h"


typedef struct UA_AggregateValueRequest{
    UA_NodeId const *remote_node_id;
    UA_NodeId const *local_node_id;
    UA_DataValue *value;
    TAILQ_ENTRY(UA_AggregateValueRequest) listEntry;
} UA_AggregateValueRequest;

typedef enum {
    UA_AGGREGATE_SYNCHRONIZATION_NONE,
    UA_AGGREGATE_SYNCHRONIZATION_SUBSCRIPTION,
    UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ,
    UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ_WRITE
} UA_AggregateSynchronization;

typedef struct UA_AggregateMappingConfig{
    UA_String nodePath;
    UA_AggregateSynchronization synchronization;
} UA_AggregateMappingConfig;

typedef struct {
    UA_String name;
    UA_String ip;
    UA_String port;
    //internal nodes, will be moved outside later
    UA_NodeId aggregateEntryNode;
    UA_NodeId statusEntryNode;
    UA_NodeId nodesEntryNode;
    //fields for threaded aggregate synchronization
    pthread_t threadId;
    pthread_mutex_t threadMutex;
    bool volatile running;
    size_t openValueJobsSize;
    UA_Client *client;
    UA_Server *server;
    //Request List
    TAILQ_HEAD(, UA_AggregateValueRequest) aggregateValueRequests;
    //Response List
    TAILQ_HEAD(, UA_AggregateValueResponse) aggregateValueResponses;
    //Subscription NodeId List
    TAILQ_HEAD(, UA_AggregateNodeId) subscriptionNodes;
    size_t mappingEntrysSize;
    UA_AggregateMappingConfig * mappingEntrys;
} UA_AggregateConfig;

typedef struct {
    UA_NodeId remote_node_id;
    UA_NodeId remote_parent_object_node_id; //needed for method calls
    UA_NodeId local_node_id;
    UA_AggregateConfig *aggregateConfig;
} internalNodeContext;

typedef struct UA_AggregateValueResponse{
    const UA_NodeId *local_node_id;
    UA_DataValue value;
    TAILQ_ENTRY(UA_AggregateValueResponse) listEntry;
} UA_AggregateValueResponse;

typedef struct{
    UA_AggregateConfig *config;
    UA_NodeId nodeIdOnAggregationServer;
} UA_AggregationSubscriptionNodeContext;

void
onWrite(UA_Server *server, const UA_NodeId *sessionId,
        void *sessionContext, const UA_NodeId *nodeId,
        void *nodeContext, const UA_NumericRange *range,
        const UA_DataValue *data);

void
onRead(UA_Server *server, const UA_NodeId *sessionId,
       void *sessionContext, const UA_NodeId *nodeid,
       void *nodeContext, const UA_NumericRange *range,
       const UA_DataValue *value);

UA_StatusCode
methodCallback(UA_Server *server,
                         const UA_NodeId *sessionId, void *sessionHandle,
                         const UA_NodeId *methodId, void *methodContext,
                         const UA_NodeId *objectId, void *objectContext,
                         size_t inputSize, const UA_Variant *input,
                         size_t outputSize, UA_Variant *output);

void handler_subscriptionSynchronization(UA_Client *client, UA_UInt32 subId, void *subContext,
                         UA_UInt32 monId, void *monContext, UA_DataValue *value);

#endif //REGISTRY_MODULE_SYNCHRONIZATION_H_
