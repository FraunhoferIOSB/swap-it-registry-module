/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_callback_functions.h"
#include "stdio.h"

//handler struct to search for an agent in the server
typedef struct{
    char *target_node_bn[4];
    UA_Server *server;
    size_t ctr;
    size_t max_length;
    UA_NodeId service_object_node;
    UA_Boolean existing_service_agents;
}UA_NodeHandler;

typedef struct{
    UA_Server *server;
    char *target_node_bn[3];
    size_t ctr;
    size_t max_length;
    UA_NodeId *target_node_nodeid;
    size_t number_of_agents;
}UA_Capability_finder;

typedef enum{
    Equal = 0,
    Greater = 1,
    Smaller = 2,
    GreaterOrEqual = 3,
    SmallerOrEqual = 4,
    IsTrue = 5,
    IsFalse = 6,
    EqualString = 7
}UA_operator_enum;

typedef struct{
    UA_String name;
    UA_DataType type;
    UA_Variant value;
    UA_operator_enum oper;
}UA_capability;

typedef struct{
    UA_capability *capability_items;
    UA_Server *server;
    size_t number_capability_items;
}Agent_capabilities;

typedef struct{
    Agent_capabilities *agents;
    UA_NodeId *agent_node;
    size_t number_agents;
}UA_All_Agent_Capabilities;


UA_StatusCode add_agent_to_registry(UA_Server *server,
                                           const UA_NodeId *sessionId, void *sessionHandle,
                                           const UA_NodeId *methodId, void *methodContext,
                                           const UA_NodeId *objectId, void *objectContext,
                                           size_t inputSize, const UA_Variant *input,
                                           size_t outputSize, UA_Variant *output){

    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_agent_list *asc = (UA_agent_list *) methodContext;

    UA_String temp = *(UA_String*) input[0].data;
    char *name = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(name, temp.data, temp.length);
    name[temp.length] = '\0';

    temp = *(UA_String*) input[1].data;
    char *add = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(add, temp.data, temp.length);
    add[temp.length] = '\0';

    temp = *(UA_String*) input[2].data;
    char *prt = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(prt, temp.data, temp.length);
    prt[temp.length] = '\0';

    temp = *(UA_String*) input[3].data;
    char *module = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(module, temp.data, temp.length);
    module[temp.length] = '\0';

    add_agent_server(server, name, add, prt, module, asc);

    return UA_STATUSCODE_GOOD;

}

static UA_StatusCode check_remaining_agents(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId, &ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeHandler *node = (UA_NodeHandler*) handle;
    //check if the browsnema begins with opc.tcp
    UA_QualifiedName qn;
    UA_Server_readBrowseName((*node).server, childId, &qn);
    char *name = (char*) qn.name.data;
    if(memcmp(name, "opc.tcp://", 10*sizeof(char)) == 0){
        node->existing_service_agents = true;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode add_agent_to_filter_list(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_Capability_finder *node = (UA_Capability_finder*) handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName((*node).server, childId, &qn);
    //ToDo find better solution to ignore Assignment and Avaliable_Agents node
    UA_QualifiedName assign = UA_QUALIFIEDNAME(2, "Assignment");
    UA_QualifiedName available = UA_QUALIFIEDNAME(2, "Available_Agents");
    if(UA_QualifiedName_equal(&qn, &assign)){
        return UA_STATUSCODE_GOOD;
    }
    if(UA_QualifiedName_equal(&qn, &available)){
        return UA_STATUSCODE_GOOD;
    }
    if(node->number_of_agents == 0){
        node->target_node_nodeid = (UA_NodeId*) UA_Array_new(1, &UA_TYPES[UA_TYPES_NODEID]);
        node->target_node_nodeid[0] = childId;
        node->number_of_agents=1;
    }else{
        UA_StatusCode retval = UA_Array_resize((void **) &node->target_node_nodeid, &node->number_of_agents,  node->number_of_agents+1, &UA_TYPES[UA_TYPES_NODEID]);
        node->target_node_nodeid[node->number_of_agents-1] = childId;
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to append the array \n");
        }
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode find_agents_for_capabilities(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_Capability_finder *node = (UA_Capability_finder*) handle;
    //check if the ctr has the size of the max length -> if so node was not found
    if((*node).ctr == (*node).max_length)
        return UA_STATUSCODE_GOOD;
    UA_QualifiedName qn;
    UA_Server_readBrowseName((*node).server, childId, &qn);
    UA_String target_node_bn = UA_String_fromChars((*node).target_node_bn[(*node).ctr]);
    if(UA_String_equal(&qn.name, &target_node_bn)){
        if(node->ctr == node->max_length-1){
            UA_Server_forEachChildNodeCall(node->server, childId, add_agent_to_filter_list, node);
        }
        else{
            (*node).ctr++;
            UA_Server_forEachChildNodeCall(node->server, childId, find_agents_for_capabilities, node);
        }
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode find_agent(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeHandler *node = (UA_NodeHandler*) handle;
    //check if the ctr has the size of the max length -> if so node was not found
    if((*node).ctr == (*node).max_length)
        return UA_STATUSCODE_GOOD;
    UA_QualifiedName qn;
    UA_Server_readBrowseName((*node).server, childId, &qn);
    UA_String target_node_bn = UA_String_fromChars((*node).target_node_bn[(*node).ctr]);
    if(UA_String_equal(&qn.name, &target_node_bn)){
        if(node->ctr == node->max_length-1){
            //delete the node of the agent
            UA_Server_deleteNode(node->server,childId, true);
            //check if the service node has no more agents registered -> if not remove the service node
            node->existing_service_agents = false;
            UA_Server_forEachChildNodeCall((*node).server, node->service_object_node, check_remaining_agents, node);
            //remove the service object from the server
            if(node->existing_service_agents == false){
                UA_Server_deleteNode(node->server,node->service_object_node, true);
            }
        }
        else{
            if(node->ctr == node->max_length-2){
                node->service_object_node = childId;
            }
            (*node).ctr++;
            UA_Server_forEachChildNodeCall(node->server, childId, find_agent, node);
        }
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode remove_agent_from_registry(UA_Server *server,
                                                const UA_NodeId *sessionId, void *sessionHandle,
                                                const UA_NodeId *methodId, void *methodContext,
                                                const UA_NodeId *objectId, void *objectContext,
                                                size_t inputSize, const UA_Variant *input,
                                                size_t outputSize, UA_Variant *output){

    printf("unregister agents with arguments: \n");
    //get the agent list from the method context
    UA_Server_getNodeContext(server, *methodId, &methodContext);
    UA_agent_list *asc = (UA_agent_list *) methodContext;

    UA_String temp = *(UA_String*) input[0].data;
    char *address = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(address, temp.data, temp.length);
    address[temp.length] = '\0';

    UA_String out = UA_STRING_NULL;
    UA_print(&temp, &UA_TYPES[UA_TYPES_STRING], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "server address: %.*s", (int)out.length, out.data);
    UA_String_clear(&out);

    temp = *(UA_String*) input[1].data;
    char *port = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(port, temp.data, temp.length);
    port[temp.length] = '\0';

    UA_print(&temp, &UA_TYPES[UA_TYPES_STRING], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "server port: %.*s", (int)out.length, out.data);
    UA_String_clear(&out);

    temp = *(UA_String*) input[2].data;
    char *service = (char*) calloc(temp.length+1, sizeof(char));
    memcpy(service, temp.data, temp.length);
    service[temp.length] = '\0';

    UA_print(&temp, &UA_TYPES[UA_TYPES_STRING], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "server service: %.*s", (int)out.length, out.data);
    UA_String_clear(&out);

    create_server_address(address, port, &temp);

    UA_print(&temp, &UA_TYPES[UA_TYPES_STRING], &out);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "complete server address: %.*s", (int)out.length, out.data);
    UA_String_clear(&out);

    //resize the agent list
    for (size_t i=0; i< asc->number_agents; i++){
        if(UA_String_equal(&asc->agent_url[i], &temp)){
            asc->agents[i] = asc->agents[i+1];
            asc->agent_url[i] = asc->agent_url[i+1];
        }
    }
    asc->agents = (UA_AggregationServerConfig*) UA_realloc(asc->agents, (asc->number_agents-1) * sizeof(UA_AggregationServerConfig));
    asc->agent_url = (UA_String*) UA_realloc(asc->agent_url, (asc->number_agents-1) * sizeof(UA_String));


    //todo remove the node from the server, if no other resource with the corresponding service exists
    //ToDo remove array
    UA_NodeHandler *node_handler = (UA_NodeHandler*) UA_calloc(1, sizeof(UA_NodeHandler));
    node_handler->ctr = 0;
    node_handler->max_length = 4;
    node_handler->server = server;

    char *first_element = "opc.tcp://";
    size_t size = strlen(first_element)+1+temp.length;
    char *complete_url = (char*) UA_calloc(size, sizeof(char));
    memcpy(complete_url, first_element, strlen(first_element));
    strcat(complete_url, (char*) temp.data);
    complete_url[size] = '\0';

    node_handler->target_node_bn[0] = "AgentList";
    node_handler->target_node_bn[1] = "PFDLServiceAgents";
    node_handler->target_node_bn[2] = service;
    node_handler->target_node_bn[3] = complete_url;
    UA_Server_forEachChildNodeCall(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), find_agent, node_handler);
    asc->number_agents--;
    free(service);
    free(address);
    free(port);
    UA_String_clear(&temp);

    return UA_STATUSCODE_GOOD;

}

static UA_StatusCode get_capability_item(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId, &ref_id)) {
        return UA_STATUSCODE_GOOD;
    }
    Agent_capabilities *node = (Agent_capabilities *)handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName(node->server, childId, &qn);
    UA_NodeId data_type_nodeId;
    UA_Server_readDataType(node->server, childId, &data_type_nodeId);
    UA_Variant var;
    UA_Server_readValue(node->server, childId, &var);
    UA_NodeId capa_bool_Id = UA_NODEID_NUMERIC(2, UA_COMMONID_CAPABILITY_STRUCT_BOOLEAN);
    UA_NodeId capa_number_Id = UA_NODEID_NUMERIC(2, UA_COMMONID_CAPABILITY_STRUCT_NUMBER);
    UA_NodeId capa_string_Id = UA_NODEID_NUMERIC(2, UA_COMMONID_CAPABILITY_STRUCT_STRING);

    node->number_capability_items++;
    node->capability_items = (UA_capability *) UA_realloc( node->capability_items, node->number_capability_items * sizeof(UA_capability));

    if(UA_NodeId_equal(&data_type_nodeId, &capa_bool_Id)){
        UA_Capability_Struct_Boolean bo = *(UA_Capability_Struct_Boolean*) var.data;
        UA_Variant_setScalarCopy(&node->capability_items[node->number_capability_items-1].value, &bo.value, &UA_TYPES[UA_TYPES_BOOLEAN]);
        node->capability_items[node->number_capability_items-1].type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_BOOLEAN];
        node->capability_items[node->number_capability_items-1].oper = (UA_operator_enum)bo.relational_Operator;
        node->capability_items[node->number_capability_items-1].name = qn.name;
    }
    else if(UA_NodeId_equal(&data_type_nodeId, &capa_number_Id)){
        UA_Capability_Struct_Number bo = *(UA_Capability_Struct_Number*) var.data;
        UA_Variant_setScalarCopy(&node->capability_items[node->number_capability_items-1].value, &bo.value, &UA_TYPES[UA_TYPES_DOUBLE]);
        node->capability_items[node->number_capability_items-1].type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_NUMBER];
        node->capability_items[node->number_capability_items-1].oper = (UA_operator_enum)bo.relational_Operator;
        node->capability_items[node->number_capability_items-1].name = qn.name;
    }
    else if(UA_NodeId_equal(&data_type_nodeId, &capa_string_Id)){
        UA_Capability_Struct_String bo = *(UA_Capability_Struct_String *) var.data;
        UA_Variant_setScalarCopy(&node->capability_items[node->number_capability_items-1].value, &bo.value, &UA_TYPES[UA_TYPES_STRING]);
        node->capability_items[node->number_capability_items-1].type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_STRING];
        node->capability_items[node->number_capability_items-1].oper = (UA_operator_enum)bo.relational_Operator;
        node->capability_items[node->number_capability_items-1].name = qn.name;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode get_capability_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id)){
        return UA_STATUSCODE_GOOD;
    }
    Agent_capabilities *node = (Agent_capabilities*) handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName((*node).server, childId, &qn);

    UA_String capa_name = UA_String_fromChars("Capabilities");
    if(UA_String_equal(&qn.name, &capa_name)){
        UA_Server_forEachChildNodeCall(node->server, childId, get_capability_item,
                                       node);
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_capability_values(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId ref_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    if(!UA_NodeId_equal(&referenceTypeId,&ref_id))
        return UA_STATUSCODE_GOOD;
    Agent_capabilities *node = (Agent_capabilities*) handle;
    UA_QualifiedName qn;
    UA_Server_readBrowseName(node->server, childId, &qn);
    UA_QualifiedName sta = UA_QUALIFIEDNAME(0, "Status");
    if(UA_QualifiedName_equal(&qn, &sta))
        return UA_STATUSCODE_GOOD;
    //initalize the array
    node->number_capability_items = 0;
    node->capability_items = (UA_capability*) UA_calloc(0, sizeof(UA_capability));
    UA_Server_forEachChildNodeCall(node->server, childId, get_capability_node,
                                   node);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode filter_agents(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionHandle,
                                   const UA_NodeId *methodId, void *methodContext,
                                   const UA_NodeId *objectId, void *objectContext,
                                   size_t inputSize, const UA_Variant *input,
                                   size_t outputSize, UA_Variant *output){

    UA_String service_name = *(UA_String*) input[0].data;
    if(input[1].arrayLength != input[2].arrayLength){
        return UA_STATUSCODE_BAD;
    }
    //iterate over all Capability Nodes
    UA_Capability_finder node_handler;
    memset(&node_handler, 0, sizeof(UA_Capability_finder));
    node_handler.target_node_bn[0] = "AgentList";
    node_handler.target_node_bn[1] = "PFDLServiceAgents";
    node_handler.target_node_bn[2] = (char*)service_name.data;
    node_handler.ctr = 0;
    node_handler.max_length = 3;
    node_handler.server= server;
    node_handler.number_of_agents = 0;
    UA_Server_forEachChildNodeCall(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), find_agents_for_capabilities, &node_handler);
    UA_String temp = UA_String_fromChars("None");
    UA_String capa_node = *(UA_String*) input[1].data;
    if(UA_String_equal(&capa_node, &temp) == 1){
        UA_Variant_setArrayCopy(output, node_handler.target_node_nodeid, node_handler.number_of_agents, &UA_TYPES[UA_TYPES_NODEID]);
        UA_String_clear(&temp);
        return UA_STATUSCODE_GOOD;
    }
    else{
        UA_String *item_list = (UA_String*) UA_Array_new(input[1].arrayLength, &UA_TYPES[UA_TYPES_STRING]);
        //process the input array on pos 1
        UA_StatusCode retval = UA_Array_copy(input[1].data, input[1].arrayLength, (void **) &item_list, &UA_TYPES[UA_TYPES_STRING]);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to copy the capability name array with error %s\n", UA_StatusCode_name(retval));
        }

        UA_String *value_list = (UA_String*) UA_Array_new(input[2].arrayLength, &UA_TYPES[UA_TYPES_STRING]);
        retval = UA_Array_copy(input[2].data, input[2].arrayLength, (void **) &value_list, &UA_TYPES[UA_TYPES_STRING]);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to copy the capability value array with error %s\n", UA_StatusCode_name(retval));
        }
        UA_All_Agent_Capabilities handler;
        handler.agents = (Agent_capabilities *) UA_calloc(node_handler.number_of_agents, sizeof(Agent_capabilities));
        handler.number_agents = node_handler.number_of_agents;
        handler.agent_node = (UA_NodeId*) UA_calloc(node_handler.number_of_agents, sizeof(UA_NodeId));

        //get the capability information from all agents
        for(size_t i=0; i<node_handler.number_of_agents; i++){
            //resize the array
            handler.agents[i].server = server;
            handler.agent_node[i] = node_handler.target_node_nodeid[i];
            //Check get the capability Items of each Agent
            UA_Server_forEachChildNodeCall(server, handler.agent_node[i], get_capability_values, &handler.agents[i]);
        }
        UA_DataType string_type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_STRING];
        UA_DataType bool_type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_BOOLEAN];
        UA_DataType number_type = UA_TYPES_COMMON[UA_TYPES_COMMON_CAPABILITY_STRUCT_NUMBER];
        UA_NodeId *agent_result_list = (UA_NodeId*) UA_calloc(0, sizeof(UA_NodeId));
        size_t number_result_agents = 0;
        for(size_t i=0; i<handler.number_agents; i++){
            UA_Boolean agent_is_suitable = true;
            for(size_t j=0; j< input[1].arrayLength; j++){
                for (size_t k=0; k<handler.agents->number_capability_items; k++){
                    if(UA_String_equal(&item_list[j], &handler.agents->capability_items[k].name)){
                        UA_Variant product_value;
                        if(UA_NodeId_equal(&handler.agents[i].capability_items[k].type.typeId, &string_type.typeId)/*memcmp(&handler.agents[i].capability_items[k].type, &string_type, sizeof(UA_DataType))==0*/){
                            UA_String product_value_str; // = UA_String_fromChars(capability_item_values[j]);
                            UA_String_copy(&value_list[j], &product_value_str);
                            UA_Variant_setScalarCopy(&product_value, &product_value_str, &UA_TYPES[UA_TYPES_STRING]);
                            void (*matching_function)(UA_Variant, UA_Variant, UA_Boolean *) = matching_functions[handler.agents[i].capability_items[k].oper];
                            matching_function(handler.agents[i].capability_items[k].value, product_value, &agent_is_suitable);
                            if(agent_is_suitable == false){
                                break;
                            }
                        }
                        else if(UA_NodeId_equal(&handler.agents[i].capability_items[k].type.typeId, &bool_type.typeId)/*memcmp(&handler.agents[i].capability_items[k].type, &bool_type, sizeof(UA_DataType))==0*/){
                            UA_Boolean product_value_str;
                            UA_String true_one = UA_String_fromChars("true");
                            UA_String true_two = UA_String_fromChars("True");
                            if(UA_String_equal(&true_one, &value_list[j]) || UA_String_equal(&true_two, &value_list[j])){
                                product_value_str = true;
                            }
                            else{
                                product_value_str = false;
                            }
                            UA_Variant_setScalarCopy(&product_value, &product_value_str, &UA_TYPES[UA_TYPES_BOOLEAN]);
                            void (*matching_function)(UA_Variant, UA_Variant, UA_Boolean *) = matching_functions[handler.agents[i].capability_items[k].oper];
                            matching_function(handler.agents[i].capability_items[k].value, product_value, &agent_is_suitable);
                            if(agent_is_suitable == false){
                                break;
                            }
                        }
                        else if(UA_NodeId_equal(&handler.agents[i].capability_items[k].type.typeId, &number_type.typeId))/*memcmp(&handler.agents[i].capability_items[k].type, &number_type, sizeof(UA_DataType))==0)*/{
                            UA_Double product_value_str = atof((char*) value_list[j].data);
                            UA_Variant_setScalarCopy(&product_value, &product_value_str, &UA_TYPES[UA_TYPES_DOUBLE]);
                            void (*matching_function)(UA_Variant, UA_Variant, UA_Boolean *) = matching_functions[handler.agents[i].capability_items[k].oper];
                            matching_function(handler.agents[i].capability_items[k].value, product_value, &agent_is_suitable);
                            if(agent_is_suitable == false){
                                break;
                            }
                        }
                    }
                    if(agent_is_suitable == false){
                        break;
                    }
                }
                if(agent_is_suitable == false){
                    break;
                }
            }
            if(agent_is_suitable == true){
                agent_result_list = (UA_NodeId*) UA_realloc(agent_result_list, (number_result_agents+1)*sizeof(UA_NodeId));
                agent_result_list[number_result_agents] = handler.agent_node[i];
                number_result_agents++;
            }
        }
        UA_Variant_setArrayCopy(output, agent_result_list, number_result_agents, &UA_TYPES[UA_TYPES_NODEID]);
        UA_Array_delete(item_list, input[1].arrayLength, &UA_TYPES[UA_TYPES_STRING]);
        UA_Array_delete(value_list, input[2].arrayLength, &UA_TYPES[UA_TYPES_STRING]);
        free(handler.agents);
        free(handler.agent_node);
        free(agent_result_list);
        return UA_STATUSCODE_GOOD;
    }
}
