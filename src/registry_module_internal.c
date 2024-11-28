/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_internal.h"

UA_StatusCode
readRegisteredAgentNS(UA_Server *server, UA_AggregationServerConfig *asc, size_t aggregate_index){
    UA_AggregateConfig *aggregateConfig = &asc->aggregateConfig[aggregate_index];
    UA_Client *client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    cc->customDataTypes = UA_Server_getConfig(server)->customDataTypes;

    //generate a const char * connection url
    char port[128];
    memcpy(port, aggregateConfig->port.data, aggregateConfig->port.length);
    port[aggregateConfig->port.length] = '\0';
    char address[256];
    memcpy(address, aggregateConfig->ip.data, aggregateConfig->ip.length);
    address[aggregateConfig->ip.length] = '\0';
    char connectionURL[512];
    snprintf (connectionURL, 512, "%s%s%s%s", "opc.tcp://", address, ":", port);

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Aggregation, client connect with: %s", connectionURL);

    UA_StatusCode retval = UA_Client_connect(client, connectionURL);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return UA_STATUSCODE_BADNOTCONNECTED;
    }

    for (int i = 0; i < aggregateConfig->mappingEntrysSize; ++i)
    {
        char mapping_browse_path[aggregateConfig->mappingEntrys[i].nodePath.length +1];
        memcpy(mapping_browse_path, aggregateConfig->mappingEntrys[i].nodePath.data, aggregateConfig->mappingEntrys[i].nodePath.length);
        mapping_browse_path[aggregateConfig->mappingEntrys[i].nodePath.length] = '\0';
        //source of string split https://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings
        char ** res  = NULL;
        char *  p    = strtok (mapping_browse_path, "/");
        int n_spaces = 0, g;
        /* split string and append tokens to 'res' */
        while (p) {
            res = realloc (res, sizeof (char*) * ++n_spaces);
            if (res == NULL)
                printf("error not enough memory");
            res[n_spaces-1] = p;
            p = strtok (NULL, "/");
        }
        /* realloc one extra element for the last NULL */
        res = realloc (res, sizeof (char*) * (n_spaces+1));
        res[n_spaces] = 0;
        /* print the result */
        /*for (g = 0; g < (n_spaces+1); ++g)
            printf ("res[%d] = %s\n", g, res[g]);*/
        g = n_spaces+1;

        UA_BrowsePath browsePath;
        UA_BrowsePath_init(&browsePath);
        browsePath.startingNode = UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER);
        browsePath.relativePath.elements = (UA_RelativePathElement*) UA_Array_new(g-1, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]);
        browsePath.relativePath.elementsSize = g-1;

        UA_NodeId parent_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER);
        for(size_t k = 0; k < (size_t)(g-1); k++) {
            /*get the node's namespaceidx from the aggregate to create the correct browsepath*/
            UA_getBrowseName handler;
            handler.client = client;
            handler.name = UA_QUALIFIEDNAME_ALLOC(0, res[k]);
            UA_Client_forEachChildNodeCall(client, parent_id, get_namespace_idx_from_config_node, &handler);
            UA_NodeId_clear(&parent_id);
            UA_NodeId_copy(&handler.id, &parent_id);

            UA_RelativePathElement *elem = &browsePath.relativePath.elements[k];
            elem->referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCES);
            UA_QualifiedName_copy(&handler.name, &elem->targetName);
            elem->includeSubtypes = true;
            UA_QualifiedName_clear(&handler.name);
            UA_NodeId_clear(&handler.id);
        }
        free(res);
        UA_TranslateBrowsePathsToNodeIdsRequest request;
        UA_TranslateBrowsePathsToNodeIdsRequest_init(&request);
        request.browsePaths = UA_calloc(1, sizeof(UA_BrowsePath));
        UA_BrowsePath_copy(&browsePath, request.browsePaths);
        UA_BrowsePath_clear(&browsePath);
        request.browsePathsSize = 1;

        UA_TranslateBrowsePathsToNodeIdsResponse response = UA_Client_Service_translateBrowsePathsToNodeIds(client, request);
        UA_TranslateBrowsePathsToNodeIdsRequest_clear(&request);

        for (int j = 0; j < response.resultsSize; ++j) {
            UA_String out = UA_STRING_NULL;
            UA_print(&response.results[j].targets->targetId.nodeId, &UA_TYPES[UA_TYPES_NODEID], &out);
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "NodeId of translate %.*s ", (int)out.length, out.data);
            UA_String_clear(&out);
        }

        UA_LocalizedText dname;
        UA_Client_readDisplayNameAttribute(client, response.results->targets->targetId.nodeId, &dname);
        UA_QualifiedName bn;
        UA_QualifiedName_init(&bn);
        UA_Client_readBrowseNameAttribute(client, response.results->targets->targetId.nodeId, &bn);
        UA_ObjectAttributes attr = UA_ObjectAttributes_default;
        attr.displayName = dname;

        UA_NodeId new_parent_id;
        UA_Server_addObjectNode(server,UA_NODEID_NULL, asc->aggregateConfig[0].nodesEntryNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                bn, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                attr,NULL, &new_parent_id);

        internalNodeIteratorHandle handle = {0};
        //memset(&handle, 1, sizeof(internalNodeIteratorHandle));
        handle.client = client;
        handle.server = server;
        handle.parent = new_parent_id;
        //memset(&handle.aggregateConfig, 1, sizeof(UA_AggregateConfig));
        handle.aggregateConfig = &asc->aggregateConfig[aggregate_index];
        handle.currentMappingIndex = i;
        handle.remoteParentObject = response.results->targets->targetId.nodeId;

        //ToDo ensure the start node is of object type
        UA_Client_forEachChildNodeCall(client, response.results->targets->targetId.nodeId,
                                       nodeIter, (void *) &handle);
        UA_NodeId_clear(&response.results->targets->targetId.nodeId);
    }
    aggregateConfig->client = client;
    return UA_STATUSCODE_GOOD;
}

void syncRegisteredAgents(UA_Server * server, UA_AggregationServerConfig *asc){
    for(size_t i = 0; i < asc->aggregateConfigSize; ++i) {
        UA_AggregateValueResponse *avr1, *avr2;
        pthread_mutex_lock(&asc->aggregateConfig[i].threadMutex);
        TAILQ_FOREACH_SAFE(avr1, &asc->aggregateConfig[i].aggregateValueResponses, listEntry, avr2) {
            //ToDo check return value
            UA_Server_writeValue(server, *avr1->local_node_id, avr1->value.value);
            TAILQ_REMOVE(&asc->aggregateConfig[i].aggregateValueResponses, avr1, listEntry);
            UA_DataValue_clear(&avr1->value);
            free(avr1);
        }
        pthread_mutex_unlock(&asc->aggregateConfig[i].threadMutex);
    }
}