/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include <stdio.h>
#include "signal.h"
#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include "swap_it.h"
#include "namespace_common_generated.h"
#include "namespace_pfdl_parameter_generated.h"
#include "namespace_warehouse_generated.h"
#include "types_common_generated.h"
#include "types_pfdl_parameter_generated_handling.h"
#include "types_common_generated_handling.h"
#include "warehouse_nodeids.h"
#include "../include/start_swap_server.h"


static UA_ByteString loadFile(const char *const path) {
    UA_ByteString fileContents = UA_STRING_NULL;
    /* Open the file */
    FILE *fp = fopen(path, "rb");
    if(!fp) {
        //errno = 0; /* We read errno also from the tcp layer... */
        printf("failed to load the file\n");
        return fileContents;
    }
    /* Get the file length, allocate the data and read */
    fseek(fp, 0, SEEK_END);
    fileContents.length = (size_t)ftell(fp);
    fileContents.data = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
    if(fileContents.data) {
        fseek(fp, 0, SEEK_SET);
        size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
        if(read != fileContents.length)
            UA_ByteString_clear(&fileContents);
    } else {
        fileContents.length = 0;
    }
    fclose(fp);

    return fileContents;
}

UA_StatusCode warehousemethodCallback(UA_Server *server,
                                      const UA_NodeId *sessionId, void *sessionHandle,
                                      const UA_NodeId *methodId, void *methodContext,
                                      const UA_NodeId *objectId, void *objectContext,
                                      size_t inputSize, const UA_Variant *input,
                                      size_t outputSize, UA_Variant *output){

    //set output variable for the Service finished event
    UA_SWAP_Order order;
    UA_SWAP_Order_init(&order);
    order.number_light_segments = 5;
    order.order_id = 1000;
    order.segmentsSize = 2;

    UA_Light_Segment *segments = (UA_Light_Segment *) UA_calloc(2, sizeof(UA_Light_Segment));
    segments[0].diameter = 5;
    segments[0].color = UA_String_fromChars("red");
    segments[0].segment_id = UA_String_fromChars("Default");
    segments[1].diameter = 5;
    segments[1].color = UA_String_fromChars("green");
    segments[1].segment_id = UA_String_fromChars("Default");

    UA_Stand_Segment stand;
    UA_Stand_Segment_init(&stand);
    stand.stand_height = 3;
    stand.stand_shape = UA_String_fromChars("plate");
    stand.stand_id = UA_String_fromChars("Default");

    order.stand = stand;
    order.segments = segments;
    //print the specified parameter
    UA_String out = UA_STRING_NULL;
    UA_print(&order, &UA_TYPES_PFDL_PARAMETER[UA_TYPES_PFDL_PARAMETER_SWAP_ORDER], &out);
    printf("Order Variable: %.*s\n", (int)out.length, out.data);
    UA_String_clear(&out);

    UA_Variant temp;
    UA_Variant_init(&temp);
    UA_Variant_setScalarCopy(&temp, &order, &UA_TYPES_PFDL_PARAMETER[UA_TYPES_PFDL_PARAMETER_SWAP_ORDER]);
    //fire the service result event
    UA_NodeId eventOutNodeId;
    UA_NodeId_init(&eventOutNodeId);
    UA_Server_createEvent(server, UA_NODEID_NUMERIC(4, UA_WAREHOUSEID_GETPARTSFROMWAREHOUSE), &eventOutNodeId);
    UA_Server_writeObjectProperty_scalar(server, eventOutNodeId,
                                         UA_QUALIFIEDNAME(4, "order"), &order, &UA_TYPES_PFDL_PARAMETER[UA_TYPES_PFDL_PARAMETER_SWAP_ORDER]);

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Trigger the event");
    UA_Server_triggerEvent(server, eventOutNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), NULL, UA_TRUE);
    UA_NodeId_clear(&eventOutNodeId);

    //create the ServiceExecutionAsyncResultDataType variable
    UA_ServiceExecutionAsyncResultDataType res;
    UA_ServiceExecutionAsyncResultDataType_init(&res);
    res.serviceResultCode = (UA_UInt32) 1;
    res.serviceResultMessage = UA_String_fromChars("Message");
    res.expectedServiceExecutionDuration = (UA_Double) 7;
    res.serviceTriggerResult = UA_SERVICETRIGGERRESULT_SERVICE_RESULT_ACCEPTED;
    //set the variable as method output
    UA_Variant_setScalarCopy(output, &res, &UA_TYPES_COMMON[UA_TYPES_COMMON_SERVICEEXECUTIONASYNCRESULTDATATYPE]);
    UA_String_clear(&res.serviceResultMessage);
    //free allocated memory
    free(order.segments);
    return UA_STATUSCODE_GOOD;
}

void *start_swap_server(void *data){
    test_swap_server_config *aggregate_conf = (test_swap_server_config*) data;
    aggregate_conf->server = UA_Server_new();
    /* load the required namespace with autogenerated functions from the nodeset-compiler*/
    UA_StatusCode retval = namespace_common_generated(aggregate_conf->server);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_WARNING(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding the common namespace failed. Please check previous error output.");
        UA_Server_delete(aggregate_conf->server);
    }
    retval = namespace_pfdl_parameter_generated(aggregate_conf->server);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding the pfdl types namespace failed. Please check previous error output.");
        UA_Server_delete(aggregate_conf->server);
    }
    retval = namespace_warehouse_generated(aggregate_conf->server);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding the pfdl types namespace failed. Please check previous error output.");
        UA_Server_delete(aggregate_conf->server);
    }
    UA_ByteString json = loadFile(aggregate_conf->conf);
    UA_service_server_interpreter swap_server;
    memset(&swap_server, 0, sizeof(UA_service_server_interpreter));
    retval = UA_server_swap_it(aggregate_conf->server, json, warehousemethodCallback, UA_FALSE, aggregate_conf->running, aggregate_conf->register_server == true ? true:false, &swap_server);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding the pfdl types namespace failed. Please check previous error output.");
        UA_Server_delete(aggregate_conf->server);
    }
    UA_ByteString_clear(&json);
    while(aggregate_conf->running) {
        UA_Server_run_iterate(aggregate_conf->server, true);
    }
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,"Shutting down server %s ", swap_server.server_name);
    /*unregister the agent and clear the config information*/
    clear_swap_server(&swap_server, UA_TRUE, aggregate_conf->server);
    UA_Server_run_shutdown(aggregate_conf->server);
    UA_Server_delete(aggregate_conf->server);
    return UA_STATUSCODE_GOOD;
}

void start_swap_server_thread(test_swap_server_config *aggregate_conf){
    pthread_create(&aggregate_conf->threadId, NULL, start_swap_server, aggregate_conf);
};