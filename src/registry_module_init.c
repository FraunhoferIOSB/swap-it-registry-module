/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_init.h"

UA_StatusCode initRegistryModuleServerStructure(UA_Server *server){
    //List that captures all entities within the system
    UA_NodeId AgentListNodeId;
    UA_ObjectAttributes AAttr = UA_ObjectAttributes_default;
    AAttr.displayName = UA_LOCALIZEDTEXT("en-us", "AgentList");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "AgentList"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            AAttr,NULL, &AgentListNodeId);

    //List for all agents that execute PFDL services
    UA_NodeId PFDLService_ID;
    UA_ObjectAttributes PFAttr = UA_ObjectAttributes_default;
    PFAttr.displayName = UA_LOCALIZEDTEXT("en-us", "PFDLServiceAgents");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            AgentListNodeId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "PFDLServiceAgents"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            PFAttr,NULL, &PFDLService_ID);

    UA_Argument filter_agent_input_Arguments[3];
    UA_Argument_init(&filter_agent_input_Arguments[0]);
    filter_agent_input_Arguments[0].description = UA_LOCALIZEDTEXT("en-us", "the Name of the service for which agents should be filtered");
    filter_agent_input_Arguments[0].name = UA_STRING("service_name");
    filter_agent_input_Arguments[0].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    filter_agent_input_Arguments[0].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&filter_agent_input_Arguments[1]);
    filter_agent_input_Arguments[1].description = UA_LOCALIZEDTEXT("en-us", "An Array with BrowseName of the capabilities that should be matched");
    filter_agent_input_Arguments[1].name = UA_STRING("Capability_Names");
    filter_agent_input_Arguments[1].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    filter_agent_input_Arguments[1].valueRank = UA_VALUERANK_SCALAR_OR_ONE_DIMENSION;


    UA_Argument_init(&filter_agent_input_Arguments[2]);
    filter_agent_input_Arguments[2].description = UA_LOCALIZEDTEXT("en-us", "An Array of Values with the Value of each Capability Item");
    filter_agent_input_Arguments[2].name = UA_STRING("Capability_Items");
    filter_agent_input_Arguments[2].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    filter_agent_input_Arguments[2].valueRank = UA_VALUERANK_SCALAR_OR_ONE_DIMENSION;

    UA_Argument filter_agent_output_Arguments[1];
    UA_Argument_init(&filter_agent_output_Arguments[0]);
    filter_agent_output_Arguments[0].description = UA_LOCALIZEDTEXT("en-us", "The Resuklt of the Calback is a list of BrowseNames that includes all Agents which are suitable to execute the required Service");
    filter_agent_output_Arguments[0].name = UA_STRING("Agent_List");
    filter_agent_output_Arguments[0].dataType = UA_TYPES[UA_TYPES_NODEID].typeId;
    filter_agent_output_Arguments[0].valueRank = UA_VALUERANK_ANY;

    UA_Argument remove_method_inputArguments[3];
    UA_Argument_init(&remove_method_inputArguments[0]);
    remove_method_inputArguments[0].description = UA_LOCALIZEDTEXT("en-us", "agent_url");
    remove_method_inputArguments[0].name = UA_STRING("agent_url");
    remove_method_inputArguments[0].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    remove_method_inputArguments[0].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&remove_method_inputArguments[1]);
    remove_method_inputArguments[1].description = UA_LOCALIZEDTEXT("en-us", "agent_port");
    remove_method_inputArguments[1].name = UA_STRING("agent_port");
    remove_method_inputArguments[1].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    remove_method_inputArguments[1].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&remove_method_inputArguments[2]);
    remove_method_inputArguments[2].description = UA_LOCALIZEDTEXT("en-us", "agent_service");
    remove_method_inputArguments[2].name = UA_STRING("agent_service");
    remove_method_inputArguments[2].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    remove_method_inputArguments[2].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument add_method_inputArguments[4];
    UA_Argument_init(&add_method_inputArguments[0]);
    add_method_inputArguments[0].description = UA_LOCALIZEDTEXT("en-US", "service_name");
    add_method_inputArguments[0].name = UA_STRING("service_name");
    add_method_inputArguments[0].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    add_method_inputArguments[0].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&add_method_inputArguments[1]);
    add_method_inputArguments[1].description = UA_LOCALIZEDTEXT("en-US", "address");
    add_method_inputArguments[1].name = UA_STRING("address");
    add_method_inputArguments[1].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    add_method_inputArguments[1].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&add_method_inputArguments[2]);
    add_method_inputArguments[2].description = UA_LOCALIZEDTEXT("en-US", "port");
    add_method_inputArguments[2].name = UA_STRING("port");
    add_method_inputArguments[2].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    add_method_inputArguments[2].valueRank = UA_VALUERANK_SCALAR;

    UA_Argument_init(&add_method_inputArguments[3]);
    add_method_inputArguments[3].description = UA_LOCALIZEDTEXT("en-US", "moduleType");
    add_method_inputArguments[3].name = UA_STRING("moduleType");
    add_method_inputArguments[3].dataType = UA_TYPES[UA_TYPES_STRING].typeId;
    add_method_inputArguments[3].valueRank = UA_VALUERANK_SCALAR;


    UA_MethodAttributes  filter_method_attr = UA_MethodAttributes_default;
    filter_method_attr.displayName = UA_LOCALIZEDTEXT("en-us", "Filter Agents");
    filter_method_attr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_STRING(1, "Filter_Agents"),
                            PFDLService_ID, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Filter_Agents"),
                            filter_method_attr, &filter_agents,
                            3, filter_agent_input_Arguments, 1, filter_agent_output_Arguments,
                            NULL, NULL);


    UA_MethodAttributes  add_method_attr = UA_MethodAttributes_default;
    add_method_attr.displayName = UA_LOCALIZEDTEXT("en-us", "Add Agent");
    add_method_attr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_STRING(1, "Add_Agent_Server"),
                            PFDLService_ID, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Add_Agent_Server"),
                            add_method_attr, &add_agent_to_registry,
                            4, add_method_inputArguments, 0, NULL,
                            NULL, NULL);

    UA_MethodAttributes  remove_method_attr = UA_MethodAttributes_default;
    remove_method_attr.displayName = UA_LOCALIZEDTEXT("en-us", "Remove Agent");
    remove_method_attr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_STRING(1, "Remove_Agent_Server"),
                            PFDLService_ID, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(1, "Remove_Agent_Server"),
                            remove_method_attr, &remove_agent_from_registry,
                            3, remove_method_inputArguments, 0, NULL,
                            NULL, NULL);


    //List for all orchestration related Agents/functionalities
    UA_NodeId Orchestrator_Service_NodeId;
    UA_ObjectAttributes orAttr = UA_ObjectAttributes_default;
    orAttr.displayName = UA_LOCALIZEDTEXT("en-us", "OrchestratorServices");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            AgentListNodeId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "OrchestratorServices"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            orAttr,NULL, &Orchestrator_Service_NodeId);

    //List for all Assignment modules
    UA_NodeId AssignmentModuleListId;
    UA_ObjectAttributes AsAttr = UA_ObjectAttributes_default;
    AsAttr.displayName = UA_LOCALIZEDTEXT("en-us", "AssignmentAgentList");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            Orchestrator_Service_NodeId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "AssignmentAgentList"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            AsAttr,NULL, &AssignmentModuleListId);
    // List for all EE
    UA_NodeId EEListId;
    UA_ObjectAttributes EEAttr = UA_ObjectAttributes_default;
    EEAttr.displayName = UA_LOCALIZEDTEXT("en-us", "ExecutionEngineList");
    UA_Server_addObjectNode(server, UA_NODEID_NULL,
                            Orchestrator_Service_NodeId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                            UA_QUALIFIEDNAME(0, "ExecutionEngineList"),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                            EEAttr,NULL, &EEListId);

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
initAggregationServerNS(UA_Server * server, UA_AggregationServerConfig *asc){
    for(size_t i = 0; i < asc->aggregateConfigSize; ++i) {
        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName.locale = UA_STRING("en-US");
        oAttr.displayName.text = asc->aggregateConfig[i].name;
        UA_QualifiedName bn;
        bn.namespaceIndex = 0;
        bn.name = asc->aggregateConfig[i].name;
        UA_Server_addObjectNode(server, UA_NODEID_NULL, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                bn,UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, NULL, &asc->aggregateConfig[i].aggregateEntryNode);
        //Add aggregation status Object
        oAttr = UA_ObjectAttributes_default;
        oAttr.displayName.locale = UA_STRING("en-US");
        oAttr.displayName.text = UA_STRING("Status");
        UA_Server_addObjectNode(server, UA_NODEID_NULL, asc->aggregateConfig[i].aggregateEntryNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                UA_QUALIFIEDNAME(0, "Status"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, NULL, &asc->aggregateConfig[i].statusEntryNode);
        //Add information model entry point object
        oAttr = UA_ObjectAttributes_default;
        oAttr.displayName.locale = UA_STRING("en-US");
        oAttr.displayName.text = UA_STRING("Nodes");
        UA_Server_addObjectNode(server, UA_NODEID_NULL, asc->aggregateConfig[i].aggregateEntryNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                UA_QUALIFIEDNAME(0, "Nodes"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, NULL, &asc->aggregateConfig[i].nodesEntryNode);
        //Add IP-address information
        UA_VariableAttributes vAttr = UA_VariableAttributes_default;
        vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Address");
        UA_Variant_setScalar(&vAttr.value, &asc->aggregateConfig[i].ip, &UA_TYPES[UA_TYPES_STRING]);
        UA_Server_addVariableNode(server, UA_NODEID_NULL, asc->aggregateConfig[i].statusEntryNode,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                  UA_QUALIFIEDNAME(0, "Address"),
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                  vAttr, NULL, NULL);
        //Add port information
        vAttr = UA_VariableAttributes_default;
        vAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Port");
        UA_Variant_setScalar(&vAttr.value, &asc->aggregateConfig[i].port, &UA_TYPES[UA_TYPES_STRING]);
        UA_Server_addVariableNode(server, UA_NODEID_NULL, asc->aggregateConfig[i].statusEntryNode,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                  UA_QUALIFIEDNAME(0, "Port"),
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                                  vAttr, NULL, NULL);

        UA_StatusCode retval = readRegisteredAgentNS(server, asc, i);
        if(retval == UA_STATUSCODE_GOOD){
            asc->aggregateConfig[i].server = server;
            //start a thread for each aggregate
            asc->aggregateConfig[i].running = true;
            pthread_create(&asc->aggregateConfig[i].threadId, NULL, aggregate_client_connection, &asc->aggregateConfig[i]);
        }
    }
    return UA_STATUSCODE_GOOD;
}

UA_Server *start_registry_module(UA_agent_list *asc){
    UA_Server *server = UA_Server_new();
    UA_ServerConfig *config = UA_Server_getConfig(server);

    UA_StatusCode retval = UA_ServerConfig_setMinimal(config, 8000, NULL);
    UA_LocalizedText_clear(&config->applicationDescription.applicationName);
    config->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en", "SWAP-IT Registry Module");
    config->maxSessions = 10000;
    config->maxSecureChannels = 10000;
    UA_Server_run_startup(server);
    initRegistryModuleServerStructure(server);

    //load the common model for the registry
    retval = namespace_common_generated(server);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Adding the common model namespace failed. Please check previous error output.");
    }

    //initialize agent list
    asc->number_agents = 0;
    asc->agents = (UA_AggregationServerConfig*) UA_calloc(0, sizeof(UA_AggregationServerConfig));
    asc->agent_url = (UA_String*) UA_calloc(0, sizeof(UA_String));

    //set the agent list as method context for the add and remove agent method
    UA_NodeId add_agent_NodeId;
    UA_NodeId remove_agent_NodeId;
    UA_NodeId_init(&add_agent_NodeId);
    UA_NodeId_init(&remove_agent_NodeId);
    find_method(server, "Add_Agent_Server", &add_agent_NodeId);
    find_method(server, "Remove_Agent_Server", &remove_agent_NodeId);

    UA_Server_setNodeContext(server, add_agent_NodeId, (void*) asc);
    UA_Server_setNodeContext(server, remove_agent_NodeId, (void*) asc);
    UA_NodeId_clear(&add_agent_NodeId);
    UA_NodeId_clear(&remove_agent_NodeId);
    return server;
}