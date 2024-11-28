/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_clear.h"

/*iterates through all aggregated nodes and clears the node context (for methods and variables)
 *entry node is asc->nodesEntryNode
 *
 */
UA_StatusCode clear_node_context(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId m_rule = UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE);
    UA_NodeId t_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    if(UA_NodeId_equal(&m_rule, &referenceTypeId) || UA_NodeId_equal(&referenceTypeId, &t_definition)){
        UA_NodeId_clear(&m_rule);
        UA_NodeId_clear(&t_definition);
        return UA_STATUSCODE_GOOD;
    }
    UA_Server *server = (UA_Server *) handle;
    //Read the NodeClass of the current child node
    UA_NodeClass nodeClass;
    UA_Server_readNodeClass(server, childId, &nodeClass);
    if(nodeClass == UA_NODECLASS_VARIABLE || nodeClass == UA_NODECLASS_METHOD){
        internalNodeContext *nodeIdContext;
        /*UA_StatusCode retval = */UA_Server_getNodeContext(server, childId, (void**) &nodeIdContext);
        if(nodeIdContext){
            UA_NodeId_clear(&nodeIdContext->local_node_id);
            UA_NodeId_clear(&nodeIdContext->remote_node_id);
            UA_NodeId_clear(&nodeIdContext->remote_parent_object_node_id);
            free(nodeIdContext);
        }
    }
    /*recursively check all children*/
    UA_Server_forEachChildNodeCall(server, childId, clear_node_context, (void*) server);
    UA_NodeId_clear(&m_rule);
    UA_NodeId_clear(&t_definition);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode clear_registered_modules_config(UA_AggregateConfig *aggregateConfig){
    aggregateConfig->running = false;
    UA_NodeId_clear(&aggregateConfig->aggregateEntryNode);
    UA_NodeId_clear(&aggregateConfig->statusEntryNode);
    UA_NodeId_clear(&aggregateConfig->nodesEntryNode);
    UA_String_clear(&aggregateConfig->name);
    UA_String_clear(&aggregateConfig->ip);
    UA_String_clear(&aggregateConfig->port);
    /*clear the linked lists*/
    UA_AggregateValueRequest *val3;
    TAILQ_FOREACH(val3, &aggregateConfig->aggregateValueRequests, listEntry){
        //UA_NodeId_clear(val3->remote_node_id);
        //UA_NodeId_clear(val3->local_node_id);
        UA_DataValue_clear(val3->value);
        TAILQ_REMOVE(&aggregateConfig->aggregateValueRequests, val3, listEntry);
        free(val3);
        val3 = NULL;
    }
    UA_AggregateValueResponse *val5;
    TAILQ_FOREACH(val5, &aggregateConfig->aggregateValueResponses, listEntry){
        //UA_NodeId_clear(val5->local_node_id);
        UA_DataValue_clear(&val5->value);
        TAILQ_REMOVE(&aggregateConfig->aggregateValueResponses, val5, listEntry);
        free(val5);
        val5 =NULL;;
    }
    UA_AggregateNodeId *val1, *val2;
    TAILQ_FOREACH_SAFE(val1, &aggregateConfig->subscriptionNodes, listEntry, val2){
        UA_NodeId_clear(&val1->nodeId);
        UA_NodeId_clear(&val1->nodeIdOnAggregationServer);
        TAILQ_REMOVE(&aggregateConfig->subscriptionNodes, val1, listEntry);
        free(val1);
        val1 = NULL;
    }
    /*clear the mappingEntrys*/
    for(size_t i=0; i<aggregateConfig->mappingEntrysSize; i++){
        UA_String_clear(&aggregateConfig->mappingEntrys[i].nodePath);
    }
    //UA_Client_disconnect(aggregateConfig->client);
    UA_Client_delete(aggregateConfig->client);
    free(aggregateConfig->mappingEntrys);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode clear_registry_module(UA_Server *server, UA_agent_list *asc){
    //todo unregister all agents
    for(size_t j=0; j< asc->number_agents; j++)
    {
        for(size_t i=0; i< asc->agents[j].aggregateConfigSize; i++){
            UA_Server_forEachChildNodeCall(server, asc->agents[j].aggregateConfig[i].nodesEntryNode, clear_node_context, (void*) server);
            clear_registered_modules_config(&asc->agents[j].aggregateConfig[i]);
        }
        UA_String_clear(&asc->agent_url[j]);
    }
    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    free(asc->agent_url);
    free(asc->agents);
    return UA_STATUSCODE_GOOD;
}