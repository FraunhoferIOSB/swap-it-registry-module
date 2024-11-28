/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_node_iterator.h"

static UA_NodeId
findSingleChildNode(UA_Client *client, UA_QualifiedName targetName,
                    UA_NodeId referenceTypeId, UA_NodeId startingNode);
static UA_NodeId
findSingleChildNode(UA_Client *client, UA_QualifiedName targetName,
                    UA_NodeId referenceTypeId, UA_NodeId startingNode){
    UA_NodeId resultNodeId;
    UA_RelativePathElement rpe;
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = referenceTypeId;
    rpe.isInverse = false;
    rpe.includeSubtypes = true;
    rpe.targetName = targetName;
    UA_BrowsePath bp;
    UA_BrowsePath_init(&bp);
    bp.startingNode = startingNode;
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;
    UA_TranslateBrowsePathsToNodeIdsRequest translateBrowsePathsToNodeIdsRequest;
    UA_TranslateBrowsePathsToNodeIdsRequest_init(&translateBrowsePathsToNodeIdsRequest);
    translateBrowsePathsToNodeIdsRequest.browsePathsSize = 1;
    translateBrowsePathsToNodeIdsRequest.browsePaths = &bp;
    UA_TranslateBrowsePathsToNodeIdsResponse tbr =
            UA_Client_Service_translateBrowsePathsToNodeIds(client, translateBrowsePathsToNodeIdsRequest);
    if((tbr.resultsSize > 0 && tbr.results->statusCode != UA_STATUSCODE_GOOD) ||
        tbr.resultsSize < 1){
        UA_TranslateBrowsePathsToNodeIdsResponse_clear(&tbr);
        return UA_NODEID_NULL;
    }
    UA_StatusCode res = UA_NodeId_copy(&tbr.results->targets->targetId.nodeId, &resultNodeId);
    if(res != UA_STATUSCODE_GOOD){
        UA_TranslateBrowsePathsToNodeIdsResponse_clear(&tbr);
        return UA_NODEID_NULL;
    }
    UA_TranslateBrowsePathsToNodeIdsResponse_clear(&tbr);
    return resultNodeId;
}

static UA_StatusCode clientgetObjectType(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId tdef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    if(UA_NodeId_equal(&referenceTypeId, &tdef) == true){
        UA_ext_node_info *handler = (UA_ext_node_info*) handle;
        UA_QualifiedName qname;
        UA_QualifiedName_init(&qname);
        UA_Client_readBrowseNameAttribute(handler->client, childId, &qname);
        UA_QualifiedName_copy(&qname, &handler->name);
        UA_QualifiedName_clear(&qname);
        UA_NodeId_clear(&tdef);
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&tdef);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    if(isInverse)
        return UA_STATUSCODE_GOOD;

    internalNodeIteratorHandle *handle_str = (internalNodeIteratorHandle *) handle;
    //Read the NodeClass of the current child node
    UA_NodeClass nodeClass;
    UA_Client_readNodeClassAttribute(handle_str->client, childId, &nodeClass);
    if(nodeClass == UA_NODECLASS_OBJECT || nodeClass == UA_NODECLASS_VARIABLE || nodeClass == UA_NODECLASS_METHOD){
        UA_LocalizedText localizedText;
        UA_Client_readDisplayNameAttribute(handle_str->client, childId, &localizedText);
        UA_QualifiedName qualifiedName;
        UA_Client_readBrowseNameAttribute(handle_str->client, childId, &qualifiedName);
        if(nodeClass == UA_NODECLASS_OBJECT){
            UA_NodeId objTypeId;
            UA_NodeId_init(&objTypeId);
            /*browse the object type and provide it to UA_Server_addObjectNode*/
            /*get the data type from the aggregate and get the browsename */
            UA_ext_node_info handle;
            handle.client = handle_str->client;
            UA_QualifiedName_init(&handle.name);
            UA_Client_forEachChildNodeCall(handle_str->client, childId, clientgetObjectType, &handle);
            if(handle.name.namespaceIndex != 0)
                handle.name.namespaceIndex = map_namespace_idx(handle_str->server, handle_str->client, handle.name.namespaceIndex);
            UA_getObjectTypeNode handler;
            handler.server = handle_str->server;
            handler.name = handle.name;
            UA_NodeId_init(&handler.id);
            UA_Server_forEachChildNodeCall(handle_str->server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTTYPESFOLDER), get_ObjectType_NodeId, &handler);

            UA_ObjectAttributes objectAttributes = UA_ObjectAttributes_default;
            objectAttributes.displayName = localizedText;
            UA_NodeId assignedNode;

            UA_StatusCode retval = UA_Server_addObjectNode(handle_str->server,
                                    UA_NODEID_NUMERIC(1, 0),
                                    handle_str->parent,
                                    referenceTypeId,
                                    qualifiedName,
                                    /*todo currently, when using the correct object type, each child of an object type will occur 2 times,
                                      1. from the aggregation meachnism that adds each child of the object from the source server,
                                      2. from the object type when instatiating in the aggregation server */
                                    handler.id,
                                    objectAttributes,
                                    NULL,
                                    &assignedNode);
            if(retval!=UA_STATUSCODE_GOOD)
                printf("failed to add the Object node with StatusCode %s \n", UA_StatusCode_name(retval));
            UA_QualifiedName_clear(&handle.name);
            UA_NodeId_clear(&handler.id);

            internalNodeIteratorHandle recursiveHandle;
            recursiveHandle.client = handle_str->client;
            recursiveHandle.server = handle_str->server;
            recursiveHandle.parent = assignedNode;
            recursiveHandle.aggregateConfig = handle_str->aggregateConfig;
            recursiveHandle.currentMappingIndex = handle_str->currentMappingIndex;
            recursiveHandle.remoteParentObject = childId; //if the object contains a method, we later need the parent for the call context
            UA_Client_forEachChildNodeCall(recursiveHandle.client, childId,
                                           nodeIter, (void *) &recursiveHandle);
        }
        if(nodeClass == UA_NODECLASS_METHOD){
            UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
            methodAttr.displayName = localizedText;
            methodAttr.executable = true;
            methodAttr.userExecutable = true;
            UA_QualifiedName input_name = UA_QUALIFIEDNAME(0, "InputArguments");
            UA_QualifiedName output_name = UA_QUALIFIEDNAME(0, "OutputArguments");
            UA_NodeId references_id = UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCES);
            UA_NodeId inputArgsNodeId =
                    findSingleChildNode(handle_str->client, input_name, references_id, childId);
            //TODO check if nodeId is NULL
            UA_Variant inputArgs;
            UA_Variant_init(&inputArgs);
            UA_Client_readValueAttribute(handle_str->client, inputArgsNodeId, &inputArgs);
            UA_NodeId outputArgsNodeId =
                    findSingleChildNode(handle_str->client, output_name, references_id, childId);
            //TODO check if nodeId is NULL
            UA_Variant outputArgs;
            UA_Variant_init(&outputArgs);
            UA_Client_readValueAttribute(handle_str->client, outputArgsNodeId, &outputArgs);

            UA_Argument * input = NULL;
            if(inputArgs.arrayLength > 0)
                input = (UA_Argument *) inputArgs.data;
            UA_Argument * output = NULL;
            if(outputArgs.arrayLength > 0)
                output = (UA_Argument *) outputArgs.data;

            UA_NodeId assignedNode;
            UA_StatusCode retval = UA_Server_addMethodNode(handle_str->server,
                                    UA_NODEID_NUMERIC(1, 0),
                                    handle_str->parent,
                                    referenceTypeId,
                                    qualifiedName,
                                    methodAttr,
                                    methodCallback,
                                    inputArgs.arrayLength,
                                    input,
                                    outputArgs.arrayLength,
                                    output,
                                    NULL,
                                    &assignedNode);
            if (retval != UA_STATUSCODE_GOOD){
                printf("failed to add the method node with StatusCode %s \n", UA_StatusCode_name(retval));
            }
            internalNodeContext *nodeIdContext = (internalNodeContext *) UA_calloc(1, sizeof(internalNodeContext));
            UA_NodeId_copy(&childId, &nodeIdContext->remote_node_id);
            UA_NodeId_copy(&handle_str->remoteParentObject, &nodeIdContext->remote_parent_object_node_id);
            UA_NodeId_copy(&assignedNode, &nodeIdContext->local_node_id);
            nodeIdContext->aggregateConfig = handle_str->aggregateConfig;
            UA_Server_setNodeContext(handle_str->server, assignedNode, nodeIdContext);
        }
        if(nodeClass == UA_NODECLASS_VARIABLE){
            /*todo browse the variable type and provide it to UA_Server_addVariableNode*/
            UA_Variant content;
            UA_Variant_init(&content);
            UA_Client_readValueAttribute(handle_str->client, childId, &content);
            UA_NodeId data_type_id;
            UA_NodeId_init(&data_type_id);
            UA_StatusCode retval = UA_Client_readDataTypeAttribute(handle_str->client, childId, &data_type_id);
            if(retval != UA_STATUSCODE_GOOD)
                printf("failed to read the data type attribute with error %s \n", UA_StatusCode_name(retval));
            if(data_type_id.namespaceIndex != 0)
                data_type_id.namespaceIndex = map_namespace_idx(handle_str->server, handle_str->client, data_type_id.namespaceIndex);
            if(qualifiedName.namespaceIndex != 0)
                qualifiedName.namespaceIndex = map_namespace_idx(handle_str->server, handle_str->client, qualifiedName.namespaceIndex);
            content.type = UA_Server_findDataType(handle_str->server, &data_type_id);
            UA_QualifiedName qn;
            UA_Client_readBrowseNameAttribute(handle_str->client, childId, &qn);
            UA_Byte accessLevel;
            UA_Client_readAccessLevelAttribute(handle_str->client, childId, &accessLevel);
            UA_NodeId newNode;
            UA_VariableAttributes variableAttributes = UA_VariableAttributes_default;
            variableAttributes.displayName = localizedText;
            variableAttributes.dataType = data_type_id;
            variableAttributes.accessLevel = accessLevel;
            /*the access level is now mapped from the node in the aggregated server, independent of the aggregation server config*/
            /*if(handle_str->aggregateConfig->mappingEntrys[handle_str->currentMappingIndex].synchronization == UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ_WRITE) {
                variableAttributes.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
            }*/
            UA_Int32 outValueRank;
            UA_Client_readValueRankAttribute(handle_str->client, childId, &outValueRank);
            variableAttributes.valueRank = outValueRank;
            UA_Client_readArrayDimensionsAttribute(handle_str->client, childId, &variableAttributes.arrayDimensionsSize, &variableAttributes.arrayDimensions);
            UA_StatusCode ret_val = UA_Server_addVariableNode(handle_str->server,
                                                              UA_NODEID_NUMERIC(1, 0),
                                                              handle_str->parent,
                                                              referenceTypeId,
                                                              qualifiedName,
                                                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                                              variableAttributes,
                                                              NULL,
                                                              &newNode);
            if(ret_val != UA_STATUSCODE_GOOD){
                printf("cannot add the variable node with error %s\n", UA_StatusCode_name(ret_val));
            }
            write_variable_value(handle_str->server, &content, data_type_id.namespaceIndex, newNode, data_type_id);
            if(handle_str->aggregateConfig->mappingEntrys[handle_str->currentMappingIndex].synchronization == UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ ||
               handle_str->aggregateConfig->mappingEntrys[handle_str->currentMappingIndex].synchronization == UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ_WRITE){
                internalNodeContext *nodeIdContext = (internalNodeContext *) UA_calloc(1, sizeof(internalNodeContext));
                UA_NodeId_copy(&childId, &nodeIdContext->remote_node_id);
                UA_NodeId_copy(&newNode, &nodeIdContext->local_node_id);
                nodeIdContext->aggregateConfig = handle_str->aggregateConfig;
                UA_Server_setNodeContext(handle_str->server, newNode, nodeIdContext);

                UA_ValueCallback valueCallback;
                valueCallback.onRead = onRead;
                if(handle_str->aggregateConfig->mappingEntrys[handle_str->currentMappingIndex].synchronization == UA_AGGREGATE_SYNCHRONIZATION_CALLBACK_READ){
                    valueCallback.onWrite = NULL;
                } else {
                    valueCallback.onWrite = onWrite;
                }
                //ToDo free on server delete the context
                UA_Server_setVariableNode_valueCallback(handle_str->server, newNode, valueCallback);
            } else if(handle_str->aggregateConfig->mappingEntrys[handle_str->currentMappingIndex].synchronization == UA_AGGREGATE_SYNCHRONIZATION_SUBSCRIPTION){
                //add new monitored item to the subscription
                //ToDo free and check alloc
                UA_AggregateNodeId *aggregateNodeId = (UA_AggregateNodeId *) UA_calloc(1, sizeof(UA_AggregateNodeId));
                UA_NodeId_copy(&childId, &aggregateNodeId->nodeId);
                UA_NodeId_copy(&newNode, &aggregateNodeId->nodeIdOnAggregationServer);
                TAILQ_INSERT_HEAD(&handle_str->aggregateConfig->subscriptionNodes, aggregateNodeId, listEntry);
            }
        UA_NodeId_clear(&data_type_id);
        UA_QualifiedName_clear(&qualifiedName);
        UA_LocalizedText_clear(&localizedText);
        }
    }
    return UA_STATUSCODE_GOOD;
}
