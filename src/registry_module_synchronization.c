/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_synchronization.h"
#include <stdio.h>
#include <open62541/plugin/log_stdout.h>

void
handler_subscriptionSynchronization(UA_Client *client, UA_UInt32 subId, void *subContext,
                         UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    UA_AggregationSubscriptionNodeContext *aggrgationSubscriptionNodeContext = (UA_AggregationSubscriptionNodeContext *) monContext;
    UA_String out = UA_STRING_NULL;
    UA_print(&aggrgationSubscriptionNodeContext->nodeIdOnAggregationServer, &UA_TYPES[UA_TYPES_NODEID], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Value on underlying server changed! NodeId on server: %.*s", (int)out.length, out.data);
    UA_String_clear(&out);
    pthread_mutex_lock(&aggrgationSubscriptionNodeContext->config->threadMutex);
    UA_AggregateValueResponse *avr = (UA_AggregateValueResponse *)UA_calloc(1,
                                                                            sizeof(UA_AggregateValueResponse));
    UA_Variant_copy(&value->value, &avr->value.value);
    avr->local_node_id = &aggrgationSubscriptionNodeContext->nodeIdOnAggregationServer;
    TAILQ_INSERT_HEAD(&aggrgationSubscriptionNodeContext->config->aggregateValueResponses, avr, listEntry);
    pthread_mutex_unlock(&aggrgationSubscriptionNodeContext->config->threadMutex);
}

void
onRead(UA_Server *server, const UA_NodeId *sessionId,
       void *sessionContext, const UA_NodeId *nodeid,
       void *nodeContext, const UA_NumericRange *range,
       const UA_DataValue *value){
    internalNodeContext *internalNodeContext1 = (internalNodeContext *) nodeContext;
    //add entry to the value request queue
    UA_AggregateValueRequest *avr = (UA_AggregateValueRequest *) UA_calloc(1, sizeof(UA_AggregateValueRequest));
    avr->remote_node_id = &internalNodeContext1->remote_node_id;
    avr->local_node_id = &internalNodeContext1->local_node_id;
    pthread_mutex_lock(&internalNodeContext1->aggregateConfig->threadMutex);
    TAILQ_INSERT_HEAD(&internalNodeContext1->aggregateConfig->aggregateValueRequests, avr, listEntry);
    pthread_mutex_unlock(&internalNodeContext1->aggregateConfig->threadMutex);
}

void
onWrite(UA_Server *server, const UA_NodeId *sessionId,
        void *sessionContext, const UA_NodeId *nodeId,
        void *nodeContext, const UA_NumericRange *range,
        const UA_DataValue *data){
    internalNodeContext *internalNodeContext1 = (internalNodeContext *) nodeContext;
    if(data->value.type->typeId.namespaceIndex != 0){
        /*get the NodeId of the data type from the underlying server#2#*/
        UA_Client_get_Id handler = {0};
        handler.client = internalNodeContext1->aggregateConfig->client;
        UA_NodeId_init(&handler.id);
        UA_QualifiedName_init(&handler.qname);
        UA_Server_readBrowseName(server, data->value.type->typeId, &handler.qname);
        UA_Client_forEachChildNodeCall(internalNodeContext1->aggregateConfig->client, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATATYPE), client_get_data_type_nodeId, &handler);

        UA_DataType temp;
        temp.members = data->value.type->members;
        temp.overlayable = data->value.type->overlayable;
        temp.membersSize = data->value.type->membersSize;
        temp.pointerFree = data->value.type->pointerFree;
        temp.typeId = handler.id;
        temp.typeKind = data->value.type->typeKind;
        temp.typeName = data->value.type->typeName;
        temp.binaryEncodingId = data->value.type->binaryEncodingId;
        UA_Variant new;
        UA_Variant_init(&new);
        new.type = &temp;
        new.data = data->value.data;
        new.arrayDimensions = data->value.arrayDimensions;
        new.arrayLength = data->value.arrayLength;
        new.arrayDimensionsSize = data->value.arrayDimensionsSize;
        new.storageType = data->value.storageType;

        UA_StatusCode result = UA_Client_writeValueAttribute(internalNodeContext1->aggregateConfig->client, internalNodeContext1->remote_node_id, &new);
        printf("Write result %s\n", UA_StatusCode_name(result));
        UA_NodeId_clear(&handler.id);
        UA_QualifiedName_clear(&handler.qname);
    }
    else{
        UA_StatusCode result = UA_Client_writeValueAttribute(internalNodeContext1->aggregateConfig->client, internalNodeContext1->remote_node_id, &data->value);
        printf("Write result %s\n", UA_StatusCode_name(result));
    }
}

UA_StatusCode
methodCallback(UA_Server *server,
                         const UA_NodeId *sessionId, void *sessionHandle,
                         const UA_NodeId *methodId, void *methodContext,
                         const UA_NodeId *objectId, void *objectContext,
                         size_t inputSize, const UA_Variant *input,
                         size_t outputSize, UA_Variant *output) {

    internalNodeContext *internalNodeContext1 = (internalNodeContext *) methodContext;
    //generate a const char * connection url
    char port[128];
    memcpy(port, internalNodeContext1->aggregateConfig->port.data, internalNodeContext1->aggregateConfig->port.length);
    port[internalNodeContext1->aggregateConfig->port.length] = '\0';
    char address[256];
    memcpy(address, internalNodeContext1->aggregateConfig->ip.data, internalNodeContext1->aggregateConfig->ip.length);
    address[internalNodeContext1->aggregateConfig->ip.length] = '\0';
    char connectionURL[512];
    snprintf (connectionURL, 512, "%s%s%s%s", "opc.tcp://", address, ":", port);


    size_t outputSizeTmp;
    UA_Variant *outputTmp = UA_Variant_new();
    UA_StatusCode result = UA_Client_call(internalNodeContext1->aggregateConfig->client,
                                          internalNodeContext1->remote_parent_object_node_id,
                                          internalNodeContext1->remote_node_id,
                                          inputSize, input, &outputSizeTmp, &outputTmp);

    for (size_t i = 0; i < outputSizeTmp; ++i) {
        if(outputTmp[i].arrayLength > 0){
            UA_Variant_setArrayCopy(&output[i], outputTmp[i].data, outputTmp[i].arrayLength, outputTmp[i].type);
        } else{
            UA_Variant_setScalarCopy(&output[i], outputTmp[i].data, outputTmp[i].type);
        }
    }
    return result;
}