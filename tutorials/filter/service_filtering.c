/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */

#include <stdio.h>
#include <open62541/types.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>

UA_Boolean running = true;
static void stopHandler(int sign) {
 UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
 running = false;
}

typedef struct{
 UA_Client *client;
 UA_NodeId *children_ids;
 UA_QualifiedName *children_names;
 size_t nbr_children;
 size_t required_children;
}search_nodes;

UA_StatusCode search_method_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
 if(isInverse){
  return UA_STATUSCODE_GOOD;
 }
 /*the nodes of the registry module are related to each other only with the HasComponent reference,
  *so that nodes with other reference relations can be ignored*/
 UA_NodeId has_component_nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
 if(!UA_NodeId_equal(&referenceTypeId, &has_component_nodeId)){
   UA_NodeId_clear(&has_component_nodeId);
   return UA_STATUSCODE_GOOD;
 }
 search_nodes *handler = (search_nodes*) handle;
 UA_QualifiedName qname;
 UA_QualifiedName_init(&qname);
 UA_Client_readBrowseNameAttribute(handler->client, childId, &qname);
 if(UA_QualifiedName_equal(&qname, &handler->children_names[handler->nbr_children])){
  UA_QualifiedName_clear(&qname);
  /*if the current child Node is the Node with the desired browsename, copy the nodeId*/
  UA_NodeId_copy(&childId, &handler->children_ids[handler->nbr_children]);
  handler->nbr_children +=1;
  /*if all nodes are found, abort th search*/
  if(handler->nbr_children == handler->required_children){
   return UA_STATUSCODE_GOOD;
  }
  /*else restart the search from the Objects Node, since the desired node might already have been checked*/
  UA_Client_forEachChildNodeCall(handler->client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), search_method_node, handle);
  return UA_STATUSCODE_GOOD;
 }
 /*continue the serch from the current child Node*/
 UA_Client_forEachChildNodeCall(handler->client, childId, search_method_node, handle);
 UA_QualifiedName_clear(&qname);
 return UA_STATUSCODE_GOOD;
}

int main(void){
 /*create a client*/
 UA_Client *client = UA_Client_new();
 UA_ClientConfig *cc = UA_Client_getConfig(client);
 UA_ClientConfig_setDefault(cc);
 /*connect the client to the registry module*/
 char *url = "opc.tcp://localhost:8000";
 UA_StatusCode retval = UA_Client_connect(client, url);

 /*search the PFDLServiceAgents Object and the FilerAgents Method*/
 /*define the structure, 2 nodes are required*/
 search_nodes handle;
 handle.client = client;
 handle.nbr_children = 0;
 handle.required_children = 2;
 handle.children_ids = (UA_NodeId*) UA_calloc(2, sizeof(UA_NodeId));
 handle.children_names = (UA_QualifiedName*) UA_calloc(2, sizeof(UA_QualifiedName));
 /*set the browsenames of the desired nodes*/
 UA_QualifiedName object = UA_QUALIFIEDNAME(0, "PFDLServiceAgents");
 UA_QualifiedName method = UA_QUALIFIEDNAME(1, "Filter_Agents");
 UA_QualifiedName_copy(&object, &handle.children_names[0]);
 UA_QualifiedName_copy(&method, &handle.children_names[1]);
 /*browse the address space of the registry module*/
 UA_Client_forEachChildNodeCall(handle.client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), search_method_node, &handle);

 /*create the input arguments*/
 UA_Variant *inp = (UA_Variant*) UA_Array_new(3, &UA_TYPES[UA_TYPES_VARIANT]);
 UA_String service_name = UA_String_fromChars("GetPartsFromWarehouse");
 UA_String empty = UA_String_fromChars("");
 UA_Variant_setScalarCopy(&inp[0], &service_name, &UA_TYPES[UA_TYPES_STRING]);
 UA_Variant_setScalarCopy(&inp[1], &empty, &UA_TYPES[UA_TYPES_STRING]);
 UA_Variant_setScalarCopy(&inp[2], &empty, &UA_TYPES[UA_TYPES_STRING]);
 /*variant for the method results*/
 UA_Variant *outp = UA_Variant_new();
 size_t outp_size = 1;

 /*call the Filter Agents Method*/
 retval = UA_Client_call(client, handle.children_ids[0], handle.children_ids[1], 3, inp, &outp_size, &outp);
 if(retval != UA_STATUSCODE_GOOD){
  goto cleanup;
 }
 /*print the results of the service call*/
 UA_String out = UA_STRING_NULL;
 UA_print(outp, &UA_TYPES[UA_TYPES_VARIANT], &out);
 printf("Filtered Assets: %.*s\n", (int)out.length, out.data);
 UA_String_clear(&out);

 /*main loop of the client*/
 while(running)
  UA_Client_run_iterate(client, 0);

 /*clear memory*/
 cleanup:
  UA_Client_disconnect(client);
  UA_Client_delete(client);
  UA_QualifiedName_clear(&handle.children_names[0]);
  UA_QualifiedName_clear(&handle.children_names[1]);
  UA_NodeId_clear(&handle.children_ids[0]);
  UA_NodeId_clear(&handle.children_ids[1]);
  free(handle.children_names);
  free(handle.children_ids);
  UA_Array_delete(inp, 3, &UA_TYPES[UA_TYPES_VARIANT]);
  UA_Variant_delete(outp);
  UA_String_clear(&empty);
  UA_String_clear(&service_name);
  return retval;
}