/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/start_client.h"
#include <check.h>
#include <stdio.h>
#include <types_common_generated_handling.h>
#include<unistd.h>

UA_Client *start_client_connection(UA_Server *server, const char* server_url){
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    UA_StatusCode retval = UA_ClientConfig_setDefault(cc);
    ck_assert(retval == UA_STATUSCODE_GOOD);
    cc->customDataTypes = UA_Server_getConfig(server)->customDataTypes;
    retval = UA_Client_connect(client, server_url);
    if(retval != UA_STATUSCODE_GOOD)
        printf("client failed to connect to server %s  with Statuscode %s \n", server_url, UA_StatusCode_name(retval));
    ck_assert(retval == UA_STATUSCODE_GOOD);
    return client;
}

UA_StatusCode
find_single_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId tdef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    if(UA_NodeId_equal(&referenceTypeId, &tdef)){
      UA_NodeId_clear(&tdef);
      return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&tdef);
    UA_NodeId mod = UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE);
    if(UA_NodeId_equal(&referenceTypeId, &mod)){
        UA_NodeId_clear(&mod);
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&mod);
    UA_Client_get_node *handler = (UA_Client_get_node*) handle;
    UA_QualifiedName qn;
    UA_QualifiedName_init(&qn);
    UA_StatusCode retval = UA_Client_readBrowseNameAttribute(handler->client, childId, &qn);
    if (retval != UA_STATUSCODE_GOOD)
        return retval;
    if(UA_QualifiedName_equal(&qn, &handler->names[0])){
      retval = UA_NodeId_copy(&childId, &handler->target_id[handler->ctr]);
      if(retval != UA_STATUSCODE_GOOD)
            return retval;
      handler->ctr++;
      UA_QualifiedName_clear(&qn);
      return UA_STATUSCODE_GOOD;
    }
    retval = UA_Client_forEachChildNodeCall(handler->client, childId, find_single_node, handler);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    UA_QualifiedName_clear(&qn);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode browse_aggregated_nodes(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId tdef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    if(UA_NodeId_equal(&referenceTypeId, &tdef)){
        UA_NodeId_clear(&tdef);
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&tdef);
    UA_NodeId mod = UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE);
    if(UA_NodeId_equal(&referenceTypeId, &mod)){
        UA_NodeId_clear(&mod);
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&mod);
    UA_Client_get_aggregated_nodes *handler = (UA_Client_get_aggregated_nodes*) handle;
    UA_QualifiedName qn;
    UA_QualifiedName_init(&qn);
    UA_StatusCode retval = UA_Client_readBrowseNameAttribute(handler->client, childId, &qn);
    if (retval != UA_STATUSCODE_GOOD){
        UA_QualifiedName_clear(&qn);
        return retval;
    }
    retval = UA_Array_append((void**)&handler->names, &handler->ctr, (void*) &qn, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    UA_QualifiedName_clear(&qn);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    retval = UA_Client_forEachChildNodeCall(handler->client, childId, browse_aggregated_nodes, handler);
    if (retval != UA_STATUSCODE_GOOD)
        return retval;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode browse_single_variable(UA_Client_get_node *aggregation, UA_NodeId *tar_var_id, UA_NodeId start_node_id, UA_QualifiedName qname){
    aggregation->ctr = 0;
    UA_StatusCode retval = UA_QualifiedName_copy(&qname, aggregation->names);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    retval = UA_Client_forEachChildNodeCall(aggregation->client, start_node_id, find_single_node, aggregation);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    retval = UA_NodeId_copy(aggregation->target_id, tar_var_id);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    UA_QualifiedName_clear(aggregation->names);
    UA_NodeId_clear(&aggregation->target_id[0]);
    return UA_STATUSCODE_GOOD;
}