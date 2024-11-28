/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_client_interface.h"
#include "../deps/common.h"

void *aggregate_client_connection( void *ptr)
{
    UA_AggregateConfig *aggregateConfig = (UA_AggregateConfig *) ptr;
    //generate a const char * connection url
    char port[128];
    memcpy(port, aggregateConfig->port.data, aggregateConfig->port.length);
    port[aggregateConfig->port.length] = '\0';
    char address[256];
    memcpy(address, aggregateConfig->ip.data, aggregateConfig->ip.length);
    address[aggregateConfig->ip.length] = '\0';
    char connectionURL[512];
    snprintf (connectionURL, 512, "%s%s%s%s", "opc.tcp://", address, ":", port);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Thread: Aggregation, client connect with: %s", connectionURL);
    //setup synchronization subscription
    UA_CreateSubscriptionRequest createSubscriptionRequest = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse createSubscriptionResponse = UA_Client_Subscriptions_create(aggregateConfig->client, createSubscriptionRequest, NULL, NULL, NULL);
    if(createSubscriptionResponse.responseHeader.serviceResult == UA_STATUSCODE_GOOD){
        UA_String out = UA_STRING_NULL;
        UA_print(&aggregateConfig->name, &UA_TYPES[UA_TYPES_STRING], &out);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Subscription on server %.*s added.", (int)out.length, out.data);
        UA_String_clear(&out);
    }
    if(createSubscriptionResponse.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        printf("Add Subscription Item error %u\n", createSubscriptionResponse.responseHeader.serviceResult);

    size_t counter = 0;
    UA_AggregateNodeId *aggregateNodeId;
    TAILQ_FOREACH(aggregateNodeId, &aggregateConfig->subscriptionNodes, listEntry) {
        counter++;
    }
    void *contexts[counter];
    UA_MonitoredItemCreateRequest *items = (UA_MonitoredItemCreateRequest *) UA_calloc(counter, sizeof(UA_MonitoredItemCreateRequest));
    UA_Client_DataChangeNotificationCallback *callbacks = (UA_Client_DataChangeNotificationCallback *) UA_calloc(counter, sizeof(UA_Client_DataChangeNotificationCallback));
    UA_Client_DeleteMonitoredItemCallback *deleteCallbacks = (UA_Client_DeleteMonitoredItemCallback *) UA_calloc(counter, sizeof(UA_Client_DeleteMonitoredItemCallback));
    if(counter > 0){
        size_t loopCount = 0;
        TAILQ_FOREACH(aggregateNodeId, &aggregateConfig->subscriptionNodes, listEntry) {
            items[loopCount] = UA_MonitoredItemCreateRequest_default(aggregateNodeId->nodeId);
            callbacks[loopCount] = handler_subscriptionSynchronization;
            deleteCallbacks[loopCount] = NULL;
            UA_AggregationSubscriptionNodeContext *aggregationSubscriptionNodeContext =
                (UA_AggregationSubscriptionNodeContext *) UA_calloc(1, sizeof(UA_AggregationSubscriptionNodeContext));
            aggregationSubscriptionNodeContext->config = aggregateConfig;
            aggregationSubscriptionNodeContext->nodeIdOnAggregationServer = aggregateNodeId->nodeIdOnAggregationServer;
            //contexts[loopCount] = &aggregateNodeId->nodeIdOnAggregationServer;
            contexts[loopCount] = aggregationSubscriptionNodeContext;
            loopCount++;
        }

        UA_CreateMonitoredItemsRequest createRequest;
        UA_CreateMonitoredItemsRequest_init(&createRequest);
        createRequest.subscriptionId = createSubscriptionResponse.subscriptionId;
        createRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        createRequest.itemsToCreate = items;
        createRequest.itemsToCreateSize = counter;
        UA_CreateMonitoredItemsResponse createResponse =
            UA_Client_MonitoredItems_createDataChanges(aggregateConfig->client, createRequest, contexts,
                                                       callbacks, deleteCallbacks);
        for(size_t i = 0; i < createResponse.resultsSize; ++i) {
            if(createResponse.results[i].statusCode != UA_STATUSCODE_GOOD)
                printf("Add Monitor Item error %u\n", createResponse.results[i].statusCode);
        }
    }
    while(aggregateConfig->running) {
        /* if already connected, this will return GOOD and do nothing */
        /* if the connection is closed/errored, the connection will be reset and then reconnected */
        /* Alternatively you can also use UA_Client_getState to get the current state */
        UA_StatusCode connectionState;
        UA_Client_getState(aggregateConfig->client, NULL, NULL, &connectionState);
        if(connectionState != UA_STATUSCODE_GOOD){
            /*todo the if condition checks whether the client is already disconnected or not. Find a better solution/synchronization*/
            if(UA_Client_getConfig(aggregateConfig->client)->clientDescription.applicationUri.data != NULL){
                UA_StatusCode retval = UA_Client_connect(aggregateConfig->client, connectionURL);
                if(retval != UA_STATUSCODE_GOOD) {
                    UA_String out = UA_STRING_NULL;
                    UA_print(&aggregateConfig->name, &UA_TYPES[UA_TYPES_STRING], &out);
                    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_CLIENT, "Server %.*s not connected. Retrying to connect in 10 seconds.", (int)out.length, out.data);
                    UA_String_clear(&out);
                    /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
                    /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
                    sleep(1);
                    continue;
                }
            }
        }
        UA_Client_run_iterate(aggregateConfig->client, 0);
        //check if there are jobs in the request list
        UA_AggregateValueRequest *avr1, *avr2;
        pthread_mutex_lock(&aggregateConfig->threadMutex);
        TAILQ_FOREACH_SAFE(avr1, &aggregateConfig->aggregateValueRequests, listEntry, avr2) {
            UA_Variant variantContent;
            UA_Variant_init(&variantContent);
            UA_Client_readValueAttribute(aggregateConfig->client, *avr1->remote_node_id, &variantContent);
            UA_AggregateValueResponse *avr = (UA_AggregateValueResponse *)UA_calloc(1, sizeof(UA_AggregateValueResponse));
            avr->value.value = variantContent;
            avr->local_node_id = avr1->local_node_id;
            TAILQ_INSERT_HEAD(&aggregateConfig->aggregateValueResponses, avr, listEntry);
            TAILQ_REMOVE(&aggregateConfig->aggregateValueRequests, avr1, listEntry);
            free(avr1);
        }
        pthread_mutex_unlock(&aggregateConfig->threadMutex);
        sleep_ms(10);
    }
    /*clear the allocated memory */
    for(size_t i=0; i < counter; i++){
        free(contexts[i]);
    }
    free(items);
    free(callbacks);
    free(deleteCallbacks);
    return ptr;
}