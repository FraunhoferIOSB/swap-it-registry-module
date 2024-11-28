/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_config_parse.h"
#include "assert.h"

UA_StatusCode search_service_object(UA_NodeId childId, UA_Boolean reference, UA_NodeId referenceTypeId, void *handle){
    service_node *s_node = (service_node *) handle;
    UA_Server *server = s_node->server;
    UA_QualifiedName child_name;
    UA_Server_readBrowseName(server, childId, &child_name);
    if(UA_String_equal(&child_name.name, &s_node->service_name.name)){
        s_node->service_node_found = UA_TRUE;
        s_node->service_object_nodeId = childId;
    }
    UA_QualifiedName_clear(&child_name);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode get_variable(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse){
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId has_component_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId organizes_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    if(!UA_NodeId_equal(&referenceTypeId, &organizes_nodeId)){
        if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
            UA_NodeId_clear(&has_component_nodeId);
            UA_NodeId_clear(&organizes_nodeId);
            return UA_STATUSCODE_GOOD;
        }
    }
    UA_NodeId server_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER);
    UA_NodeId aliases_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ALIASES);
    if(UA_NodeId_equal(&childId, &server_nodeId) || UA_NodeId_equal(&childId, &aliases_nodeId)){
        UA_NodeId_clear(&has_component_nodeId);
        UA_NodeId_clear(&organizes_nodeId);
        UA_NodeId_clear(&server_nodeId);
        UA_NodeId_clear(&aliases_nodeId);
        /*UA_NodeId_clear(&location_nodeId);*/
        return UA_STATUSCODE_GOOD;
    }
    UA_get_variable *node_handler = (UA_get_variable*) handle;
    UA_QualifiedName bn;
    UA_Server_readBrowseName(node_handler->server, childId, &bn);
    if(UA_String_equal(&bn.name, &node_handler->variable_name.name)){
        UA_NodeId_copy(&childId, &node_handler->variable_nodeId);
    }
    else{
        UA_Server_forEachChildNodeCall(node_handler->server, childId, get_variable, (void*) node_handler);
    }
    UA_QualifiedName_clear(&bn);
    UA_NodeId_clear(&has_component_nodeId);
    UA_NodeId_clear(&organizes_nodeId);
    UA_NodeId_clear(&server_nodeId);
    UA_NodeId_clear(&aliases_nodeId);
    /*UA_NodeId_clear(&location_nodeId);*/
    return UA_STATUSCODE_GOOD;
}


UA_StatusCode get_single_variable(UA_Server *server, UA_QualifiedName variable_name, UA_NodeId *variable_nodeId){
    UA_get_variable handle;
    memset(&handle,0, sizeof(UA_get_variable));
    handle.server = server;
    UA_QualifiedName_copy(&variable_name, &handle.variable_name);
    UA_Server_forEachChildNodeCall(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), get_variable, (void*) &handle);
    UA_NodeId_copy(&handle.variable_nodeId, variable_nodeId);
    UA_QualifiedName_clear(&handle.variable_name);
    UA_NodeId_clear(&handle.variable_nodeId);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode find_method(UA_Server *server, char *methodname, UA_NodeId *service_method_nodeId){
    return get_single_variable(server, UA_QUALIFIEDNAME(0, methodname), service_method_nodeId);
}


UA_StatusCode write_assignment_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_Server *server = (UA_Server*) handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName(server, childId, &qn);
    UA_Variant value;
    UA_String temp = UA_String_fromChars("Default Assignment Agent");
    UA_Variant_setScalarCopy(&value, &temp, &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_writeValue(server, childId, value);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode find_assignment_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_Server *server = (UA_Server*) handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName(server, childId, &qn);
    UA_Server_forEachChildNodeCall(server, childId, write_assignment_node, server);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode loadRegisterAgentConfig(UA_Server *server, UA_ByteString json, UA_AggregationServerConfig *asc){
    service_node s_node;
    memset(&s_node, 0, sizeof(service_node));
    cj5_token tokens[256];
    cj5_result r = cj5_parse((const char *)json.data, (unsigned int)json.length, tokens, 256, NULL);
    const char *g_token_types[] = {"CJ5_TOKEN_OBJECT", "CJ5_TOKEN_ARRAY", "CJ5_TOKEN_NUMBER",
                                   "CJ5_TOKEN_STRING", "CJ5_TOKEN_BOOL", "CJ5_TOKEN_NULL"};


    // print complete parsed JSON for test and debug purpose
    if(!r.error) {
        for(unsigned int i = 0; i < r.num_tokens; i++) {
            char content[1024];
            unsigned int num = tokens[i].end - tokens[i].start + 1;
            memcpy(content, (const char *)&json.data[tokens[i].start], num);
            content[num] = '\0';
            assert(tokens[i].type <= CJ5_TOKEN_NULL);
            printf("%d: { type = %s, size = %d, parent = %d, content = '%s'\n", i,
                   g_token_types[tokens[i].type], tokens[i].size, tokens[i].parent_id, content);
        }
    } else {
        printf("ERROR: %d - line:%d (%d)\n", r.error, r.error_line, r.error_col);
        if(r.error == CJ5_ERROR_OVERFLOW) {
            printf("COUNT: %d\n", r.num_tokens);    // actual number of tokens needed
        }
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    unsigned int idx = 0;
    cj5_find(&r, &idx, "aggregates");
    if(tokens[idx].type != CJ5_TOKEN_ARRAY){
        printf("error during parse!");
    }
    cj5_error_code err;
    char buf_address[256];
    char buf_port[256];
    char buf_module[256];
    char buf_name[256];
    //ToDO remove outer loop since the configs will only contain a single server
    unsigned int idx_array = idx;
    for (unsigned int i = 0; i < tokens[idx_array].size; ++i) {
        idx +=1;
        unsigned int local_object_index = idx;
        asc->aggregateConfigSize++;
        asc->aggregateConfig = (UA_AggregateConfig *) UA_realloc(asc->aggregateConfig, sizeof(UA_AggregateConfig) * asc->aggregateConfigSize);
        memset(&asc->aggregateConfig[asc->aggregateConfigSize-1], 0, sizeof(UA_AggregateConfig));
        TAILQ_INIT(&asc->aggregateConfig[asc->aggregateConfigSize-1].aggregateValueRequests);
        TAILQ_INIT(&asc->aggregateConfig[asc->aggregateConfigSize-1].aggregateValueResponses);
        TAILQ_INIT(&asc->aggregateConfig[asc->aggregateConfigSize-1].subscriptionNodes);

        cj5_find(&r, &idx, "service_name");
        err = cj5_get_str(&r, idx, buf_name, NULL);
        if(err != CJ5_ERROR_NONE) {
            return UA_STATUSCODE_BADCONFIGURATIONERROR;
        }
        asc->aggregateConfig[asc->aggregateConfigSize-1].name = UA_String_fromChars(buf_name);
        UA_NodeId pfdl_service_agent_object_nodeId;
        UA_NodeId_init(&pfdl_service_agent_object_nodeId);
        UA_QualifiedName qn = UA_QUALIFIEDNAME(0, "PFDLServiceAgents");
        get_single_variable(server, qn, &pfdl_service_agent_object_nodeId);
        s_node.service_node_found = UA_FALSE;
        s_node.service_name.name = UA_String_fromChars(buf_name);
        s_node.server = server;
        UA_Server_forEachChildNodeCall(server, pfdl_service_agent_object_nodeId,
                                       (UA_NodeIteratorCallback) search_service_object, &s_node);

        if(s_node.service_node_found == UA_FALSE){
            UA_ObjectAttributes attr = UA_ObjectAttributes_default;
            UA_StatusCode retval = UA_Server_addObjectNode(server, UA_NODEID_NULL, pfdl_service_agent_object_nodeId,
                                                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                                           UA_QUALIFIEDNAME(0, buf_name),
                                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                                           attr, NULL, &s_node.service_object_nodeId);
            if(retval !=UA_STATUSCODE_GOOD){
                printf("failed to add the node\n");
            }
        }
        UA_NodeId_clear(&pfdl_service_agent_object_nodeId);
        UA_Server_forEachChildNodeCall(server, s_node.service_object_nodeId, find_assignment_node, server);

        idx = local_object_index;
        cj5_find(&r, &idx, "address");
        err = cj5_get_str(&r, idx, buf_address, NULL);
        if(err != CJ5_ERROR_NONE) {
            return UA_STATUSCODE_BADCONFIGURATIONERROR;
        }
        asc->aggregateConfig[asc->aggregateConfigSize-1].ip = UA_String_fromChars(buf_address);

        idx = local_object_index;
        cj5_find(&r, &idx, "port");
        err = cj5_get_str(&r, idx, buf_port, NULL);
        if(err != CJ5_ERROR_NONE) {
            return UA_STATUSCODE_BADCONFIGURATIONERROR;
        }
        asc->aggregateConfig[asc->aggregateConfigSize-1].port = UA_String_fromChars(buf_port);

        //get the starting Node to browse the target server
        idx = local_object_index;
        cj5_find(&r, &idx, "moduleType");
        err = cj5_get_str(&r, idx, buf_module, NULL);
        if(err != CJ5_ERROR_NONE) {
            return UA_STATUSCODE_BADCONFIGURATIONERROR;
        }
        char *object = "Objects/";
        char *path = (char *) calloc(strlen(object)+strlen(buf_module)+2, sizeof(char));
        strcpy(path, object);
        strcat(path, buf_module);
        path[strlen(object)+strlen(buf_module)] = * (char *)"/";
        path[strlen(object)+strlen(buf_module)+1] = '\0';
        printf("the path variable has the value %s\n", path);

        //resolve mapping array
        idx = local_object_index;
        cj5_find(&r, &idx, "mapping");
        if(tokens[idx].type != CJ5_TOKEN_ARRAY){
            printf("error during parse!");
        }
        unsigned int inner_local_mapping_index = idx;
        for (unsigned int j = 0; j < tokens[inner_local_mapping_index].size; ++j) {
            idx += 1;

            // adjust the aggregation config
            asc->aggregateConfig[asc->aggregateConfigSize - 1].mappingEntrysSize++;
            asc->aggregateConfig[asc->aggregateConfigSize - 1].mappingEntrys = (UA_AggregateMappingConfig *)
                    UA_realloc(asc->aggregateConfig[asc->aggregateConfigSize - 1].mappingEntrys,
                               asc->aggregateConfig[asc->aggregateConfigSize - 1].mappingEntrysSize *
                               sizeof(UA_AggregateMappingConfig));

            unsigned int inner_local_object_index = idx;
            cj5_find(&r, &idx, "entryNode");
            char buf_browse_path[256];
            err = cj5_get_str(&r, idx, buf_browse_path, NULL);
            if(err != CJ5_ERROR_NONE) {
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            char *bpath = (char*) calloc(strlen(buf_browse_path)+strlen(path)+1, sizeof(char));
            strcpy(bpath, path);
            strcat(bpath, buf_browse_path);
            asc->aggregateConfig[asc->aggregateConfigSize - 1].mappingEntrys[j].nodePath = UA_String_fromChars(bpath);
            idx = inner_local_object_index;
            char buf_sync[128];
            cj5_find(&r, &idx, "synchronization");
            err = cj5_get_str(&r, idx, buf_sync, NULL);
            if (err != CJ5_ERROR_NONE){
                return UA_STATUSCODE_BADCONFIGURATIONERROR;
            }
            if(memcmp(buf_sync, "subs", 4) == 0){
                asc->aggregateConfig[asc->aggregateConfigSize-1].mappingEntrys[j].synchronization = UA_AGGREGATE_SYNCHRONIZATION_SUBSCRIPTION;
            } else if(memcmp(buf_sync, "on-demand-read-write", 20) == 0){
                asc->aggregateConfig[asc->aggregateConfigSize-1].mappingEntrys[j].synchronization = UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ_WRITE;
            } else if(memcmp(buf_sync, "on-demand-read", 14) == 0){
                asc->aggregateConfig[asc->aggregateConfigSize-1].mappingEntrys[j].synchronization = UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ;
            }
            free(bpath);
        }
        free(path);

    }
    idx = 0;
    cj5_find(&r, &idx, "defaults");
    if(tokens[idx].type != CJ5_TOKEN_OBJECT) {
        printf("error during parse!");
    }
    unsigned int object_default_index = idx;
    int64_t subscription_interval = 0;
    cj5_find(&r, &idx, "subscription_interval");
    err = cj5_get_int(&r, idx, &subscription_interval);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    asc->subscription_interval = (UA_Duration)subscription_interval;

    idx = object_default_index;
    cj5_find(&r, &idx, "unsecure");
    char buf_unsecure[64];
    err = cj5_get_str(&r, idx, buf_unsecure, NULL);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    if(memcmp(buf_unsecure, "unsec", 4) == 0){
        asc->unsecureAllowed = true;
    } else {
        asc->unsecureAllowed = false;
    }

    idx = object_default_index;
    cj5_find(&r, &idx, "timeout");
    int64_t timeout = 0;
    err = cj5_get_int(&r, idx, &timeout);
    if (err != CJ5_ERROR_NONE){
        return UA_STATUSCODE_BADCONFIGURATIONERROR;
    }
    asc->aggregateTimeout = (UA_Duration)timeout;

    UA_NodeId parent_id;
    UA_NodeId_init(&parent_id);
    UA_NodeId_copy(&s_node.service_object_nodeId, &parent_id);
    UA_QualifiedName_clear(&s_node.service_name);
    UA_NodeId_clear(&s_node.service_object_nodeId);

    //init aggregation server namespace
    initRegisteredAgentNodes(server, asc, parent_id, buf_module);
    /*return UA_STATUSCODE_GOOD;*/
}

