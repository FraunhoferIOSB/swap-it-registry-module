/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_register_agent.h"
#include "stdio.h"

void create_server_address(char *address, char *port, UA_String *agent_list_url){
    size_t size = 2+strlen(address)+strlen(port);
    char *agent_url = (char*) UA_calloc(size+1, sizeof(char));
    memcpy(agent_url, address, strlen(address));
    strcat(agent_url, ":");
    strcat(agent_url, port);
    agent_url[size] = '\0';
    *agent_list_url = UA_String_fromChars(agent_url);
    free(agent_url);
}

UA_StatusCode initRegisteredAgentNodes(UA_Server * server, UA_AggregationServerConfig *asc, UA_NodeId parent_nodeId, char *ModuleTypeName){
    //create the url of the server and use it as browsename for the aggregate
    ////generate a const char * connection url
    char port[128];
    memcpy(port, asc->aggregateConfig->port.data, asc->aggregateConfig->port.length);
    port[asc->aggregateConfig->port.length] = '\0';
    char address[256];
    memcpy(address, asc->aggregateConfig->ip.data, asc->aggregateConfig->ip.length);
    address[asc->aggregateConfig->ip.length] = '\0';
    char connectionURL[512];
    snprintf (connectionURL, 512, "%s%s%s%s", "opc.tcp://", address, ":", port);

    UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName.locale = UA_STRING("en-US");
    oAttr.displayName.text = UA_String_fromChars(connectionURL);
    UA_QualifiedName bn;
    bn.namespaceIndex = 0;
    bn.name = UA_String_fromChars(connectionURL);
    UA_StatusCode retval = UA_Server_addObjectNode(server, UA_NODEID_NULL, parent_nodeId,
                                                       UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                                       bn,UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                                       oAttr, NULL, &asc->aggregateConfig[asc->aggregateConfigSize-1].aggregateEntryNode);
    UA_String_clear(&oAttr.displayName.text);
    UA_QualifiedName_clear(&bn);

    if(retval != UA_STATUSCODE_GOOD)
        printf("cannot add variable 1 inside the  initAggregationServerNS_modified function with error %s\n", UA_StatusCode_name(retval));

    //Add aggregation status Object
    oAttr = UA_ObjectAttributes_default;
    oAttr.displayName.locale = UA_STRING("en-US");
    oAttr.displayName.text = UA_STRING("Status");
    retval = UA_Server_addObjectNode(server, UA_NODEID_NULL, asc->aggregateConfig[asc->aggregateConfigSize-1].aggregateEntryNode,
                                         UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                         UA_QUALIFIEDNAME(0, "Status"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                         oAttr, NULL, &asc->aggregateConfig[asc->aggregateConfigSize-1].statusEntryNode);

    if(retval != UA_STATUSCODE_GOOD)
        printf("cannot add variable 2 inside the  initAggregationServerNS_modified function with error %s\n", UA_StatusCode_name(retval));

    //Add information model entry point object
    oAttr = UA_ObjectAttributes_default;
    oAttr.displayName.locale = UA_STRING("en-US");
    oAttr.displayName.text = UA_STRING(ModuleTypeName);
    retval = UA_Server_addObjectNode(server, UA_NODEID_NULL, asc->aggregateConfig[asc->aggregateConfigSize-1].aggregateEntryNode,
                                         UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                         UA_QUALIFIEDNAME(0, ModuleTypeName), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                         oAttr, NULL, &asc->aggregateConfig[asc->aggregateConfigSize-1].nodesEntryNode);

    if(retval != UA_STATUSCODE_GOOD)
        printf("cannot add variable 3 inside the  initAggregationServerNS_modified function with error %s\n", UA_StatusCode_name(retval));

    //Add IP-address information
    UA_VariableAttributes vAttr = UA_VariableAttributes_default;
    vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Address");
    UA_Variant_setScalar(&vAttr.value, &asc->aggregateConfig[asc->aggregateConfigSize-1].ip, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Server_addVariableNode(server, UA_NODEID_NULL, asc->aggregateConfig[asc->aggregateConfigSize-1].statusEntryNode,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                           UA_QUALIFIEDNAME(0, "Address"),
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                           vAttr, NULL, NULL);

    if(retval != UA_STATUSCODE_GOOD)
        printf("cannot add variable 4 inside the  initAggregationServerNS_modified function with error %s\n", UA_StatusCode_name(retval));

    //Add port information
    vAttr = UA_VariableAttributes_default;
    vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Port");
    UA_Variant_setScalar(&vAttr.value, &asc->aggregateConfig[asc->aggregateConfigSize-1].port, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Server_addVariableNode(server, UA_NODEID_NULL, asc->aggregateConfig[asc->aggregateConfigSize-1].statusEntryNode,
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                           UA_QUALIFIEDNAME(0, "Port"),
                                           UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                           vAttr, NULL, NULL);

    if(retval != UA_STATUSCODE_GOOD)
        printf("cannot add variable 5 inside the  initAggregationServerNS_modified function with error %s\n", UA_StatusCode_name(retval));


    readRegisteredAgentNS(server, asc, asc->aggregateConfigSize-1);
    //start a thread to map the agent to the registry module
    asc->aggregateConfig[asc->aggregateConfigSize-1].running = true;
    asc->aggregateConfig->server = server;
    pthread_create(&asc->aggregateConfig[asc->aggregateConfigSize-1].threadId, NULL, aggregate_client_connection, &asc->aggregateConfig[asc->aggregateConfigSize-1]);
    return UA_STATUSCODE_GOOD;
}

void add_agent_server(UA_Server *server, char *service_name, char *address, char *port, char *moduleType, UA_agent_list *asc){
    printf("execute the callback function\n");
    char *key_address = "address:\"";
    char *key_port = "port:\"";
    char *key_moduleType = "moduleType:\"";
    char *quotation_mark = "\",";
    //char *terminate = (char*)"\0";
    char *first_part = "{ aggregates: \n [{service_name:\"";
    char *last_part = "mapping: [\n{entryNode: \"Capabilities\" ,synchronization: \"subscription\"}\n"
                      ",{entryNode: \"State\", synchronization: \"subscription\"}\n ,{entryNode: \"Queue/ServiceQueue\", synchronization: \"subscription\"}\n]}],\n defaults:{subscription_interval: 500,unsecure: \"allowed\",timeout: 2000}}";

    //todo add an array for optional nodes to be subscribed
    size_t curr_size = (1+strlen(quotation_mark)+strlen(first_part)+ strlen(service_name)
                        +strlen(quotation_mark)+strlen(address)+strlen(key_address)
                        +strlen(quotation_mark)+strlen(port)+strlen(key_port)
                        +strlen(quotation_mark)+strlen(moduleType)+strlen(key_moduleType)
                        +strlen(last_part))*sizeof(char);
    char *json_string = (char*) UA_calloc(curr_size,sizeof(char));
    strcpy(json_string, first_part);
    strcat(json_string, service_name);
    strcat(json_string, quotation_mark);
    strcat(json_string, key_address);
    strcat(json_string, address);
    strcat(json_string, quotation_mark);
    strcat(json_string, key_port);
    strcat(json_string, port);
    strcat(json_string, quotation_mark);
    strcat(json_string, key_moduleType);
    strcat(json_string, moduleType);
    strcat(json_string, quotation_mark);
    strcat(json_string, last_part);

    UA_ByteString json;
    UA_ByteString_init(&json);
    json.length = strlen(json_string);
    json.data = (UA_Byte*) json_string;

    asc->agent_url = (UA_String *) UA_realloc(asc->agent_url, (asc->number_agents+1) *sizeof(UA_String));
    memset(&asc->agent_url[asc->number_agents], 0, sizeof(UA_String));
    asc->agents = (UA_AggregationServerConfig *)UA_realloc(asc->agents ,(asc->number_agents+1) *sizeof(UA_AggregationServerConfig));
    memset(&asc->agents[asc->number_agents], 0, sizeof(UA_AggregationServerConfig));
    asc->agents[asc->number_agents].aggregateConfig = (UA_AggregateConfig*) UA_calloc(0, sizeof(UA_AggregateConfig));

    loadRegisterAgentConfig(server, json,  &asc->agents[asc->number_agents]);
    create_server_address(address, port, &asc->agent_url[asc->number_agents]);
    asc->number_agents +=1;

    free(json_string);
    free(moduleType);
    free(service_name);
    free(port);
    free(address);
}
