/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#include "../include/registry_module_namespace_solver.h"

UA_StatusCode client_get_data_type_nodeId(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId tdef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);
    if(UA_NodeId_equal(&referenceTypeId, &tdef) == false){
        UA_NodeId_clear(&tdef);
        return UA_STATUSCODE_GOOD;
    }
    UA_NodeId_clear(&tdef);

    UA_Client_get_Id *handler = (UA_Client_get_Id*) handle;
    UA_QualifiedName qname;
    UA_QualifiedName_init(&qname);
    UA_Client_readBrowseNameAttribute(handler->client, childId, &qname);
    if(UA_String_equal(&qname.name, &handler->qname.name)){
        UA_QualifiedName_clear(&qname);
        UA_NodeId_copy(&childId, &handler->id);
        return UA_STATUSCODE_GOOD;
    }
    UA_Client_forEachChildNodeCall(handler->client, childId, client_get_data_type_nodeId, handler);
    UA_QualifiedName_clear(&qname);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_namespace_idx_from_config_node(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
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

    UA_getBrowseName *handler = (UA_getBrowseName*) handle;
    UA_QualifiedName qname;
    UA_QualifiedName_init(&qname);
    UA_Client_readBrowseNameAttribute(handler->client, childId, &qname);
    if(UA_String_equal(&qname.name, &handler->name.name)){
        UA_QualifiedName_clear(&handler->name);
        UA_QualifiedName_copy(&qname, &handler->name);
        UA_NodeId_copy(&childId, &handler->id);
        return UA_STATUSCODE_GOOD;
    }
    UA_Client_forEachChildNodeCall(handler->client, childId, get_namespace_idx_from_config_node, handler);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode get_ObjectType_NodeId(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
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
    UA_getObjectTypeNode *handler = (UA_getObjectTypeNode*) handle;
    UA_QualifiedName qname;
    UA_QualifiedName_init(&qname);
    UA_Server_readBrowseName(handler->server, childId, &qname);
    if(UA_QualifiedName_equal(&qname, &handler->name)){
        UA_NodeId_copy(&childId, &handler->id);
        UA_QualifiedName_clear(&qname);
        return UA_STATUSCODE_GOOD;
    }
    UA_Server_forEachChildNodeCall(handler->server, childId, get_ObjectType_NodeId, handler);
    UA_QualifiedName_clear(&qname);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode check_source_server_namespace_array(UA_UInt16 namespacidx, UA_Client *client, UA_String *namespacename){
    UA_Variant out_val;
    UA_Variant_init(&out_val);
    UA_Client_readValueAttribute(client, UA_NODEID_NUMERIC(0, 2255), &out_val);
    if(UA_Variant_hasArrayType(&out_val, &UA_TYPES[UA_TYPES_STRING])){
        UA_String *namespacearray;
        UA_StatusCode retval = UA_Array_copy(out_val.data, out_val.arrayLength, (void **) &namespacearray, &UA_TYPES[UA_TYPES_STRING]);
        if(retval != UA_STATUSCODE_GOOD){
            printf("failed to copy the namespacearray with error %s\n", UA_StatusCode_name(retval));
        }
        UA_String_copy(&namespacearray[(size_t) namespacidx], namespacename);
        UA_Array_delete(namespacearray, out_val.arrayLength, &UA_TYPES[UA_TYPES_STRING]);
    }
    UA_Variant_clear(&out_val);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode check_aggregation_server_namespace_array(UA_UInt16 *namespaceidx, UA_Server *server, UA_String namespacename){
    UA_Variant out_val;
    UA_Variant_init(&out_val);
    UA_Server_readValue(server, UA_NODEID_NUMERIC(0, 2255), &out_val);
    if(UA_Variant_hasArrayType(&out_val, &UA_TYPES[UA_TYPES_STRING])){
        UA_String *namespacearray;
        UA_StatusCode retval = UA_Array_copy(out_val.data, out_val.arrayLength, (void **) &namespacearray, &UA_TYPES[UA_TYPES_STRING]);
        if(retval != UA_STATUSCODE_GOOD){
            UA_Variant_clear(&out_val);
            printf("failed to copy the namespacearray with error %s\n", UA_StatusCode_name(retval));
            return retval;
        }
        for(size_t i=0; i<out_val.arrayLength; i++){
            if(UA_String_equal(&namespacename, &namespacearray[i])){
                UA_UInt16_copy((UA_UInt16*) &i, namespaceidx);
            }
        }
        UA_Array_delete(namespacearray, out_val.arrayLength, &UA_TYPES[UA_TYPES_STRING]);
    }
    UA_Variant_clear(&out_val);
    return UA_STATUSCODE_GOOD;
}

UA_UInt16 map_namespace_idx(UA_Server *server, UA_Client *client, UA_UInt16 currentIdentifier){
    UA_UInt16 namespaceindex = 0;
    UA_String namespacename;
    UA_String_init(&namespacename);
    check_source_server_namespace_array(currentIdentifier, client, &namespacename);
    check_aggregation_server_namespace_array(&namespaceindex, server, namespacename);
    UA_String_clear(&namespacename);
    return namespaceindex;
}

UA_StatusCode browse_target_server(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle){
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NamespaceidxMapping *node = (UA_NamespaceidxMapping*) handle;
    UA_QualifiedName qn;
    UA_QualifiedName_init(&qn);
    UA_Client_readBrowseNameAttribute(node->client, childId, &qn);
    UA_String name = UA_String_fromChars(node->target_node_name);
    if(UA_String_equal(&qn.name, &name)){
        node->curr_id = childId;
        node->namespaceIndex = qn.namespaceIndex;
    }
    UA_String_clear(&name);
    UA_QualifiedName_clear(&qn);
    return UA_STATUSCODE_GOOD;
}

UA_UInt16 check_namespace_idx(UA_NamespaceidxMapping *data){
    //check if it is the Objects folder
    UA_NamespaceidxMapping *node = (UA_NamespaceidxMapping*) data;
    UA_QualifiedName qn;
    UA_Client_readBrowseNameAttribute(node->client, node->curr_id, &qn);
    UA_String name = UA_String_fromChars(node->target_node_name);
    if(UA_String_equal(&qn.name, &name)){
        UA_String_clear(&name);
        return qn.namespaceIndex;
    }
    UA_Client_forEachChildNodeCall(node->client, node->curr_id, browse_target_server, node);
    UA_String_clear(&name);
    return node->namespaceIndex;
}

UA_StatusCode get_namespace(UA_Client *client, UA_Server *server, UA_NamespaceidxMapping *index_finder, UA_UInt16 *namespaceidx){
    *namespaceidx = check_namespace_idx(index_finder);
    UA_String namespacename;
    UA_String_init(&namespacename);
    check_source_server_namespace_array(*namespaceidx, client, &namespacename);
    check_aggregation_server_namespace_array(namespaceidx, server, namespacename);
    UA_String_clear(&namespacename);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode encode_single_extension_object(UA_Server *server, UA_ExtensionObject *old_val, UA_ExtensionObject *new_val, UA_NodeId data_type_id, UA_UInt16 namespaceindex){
    old_val->content.encoded.typeId.namespaceIndex = namespaceindex;
    const UA_DataType *typ = UA_Server_findDataType(server, &data_type_id);
    UA_DecodeBinaryOptions options;
    options.customTypes = UA_Server_getConfig(server)->customDataTypes;
    new_val->encoding = UA_EXTENSIONOBJECT_DECODED;
    new_val->content.decoded.type = typ;
    new_val->content.decoded.data = UA_malloc(typ->memSize);
    UA_StatusCode retval = UA_decodeBinary(&old_val->content.encoded.body, new_val->content.decoded.data, typ, &options);
    if(retval != UA_STATUSCODE_GOOD){
        return retval;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode create_new_extension_object_array(UA_Server *server, UA_Variant *variable_val, UA_ExtensionObject *old_val, UA_Variant content, UA_ExtensionObject *new_val, UA_NodeId data_type_id, UA_UInt16 namespaceindex){
    new_val = (UA_ExtensionObject*) UA_Array_new(content.arrayLength, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
    variable_val->arrayLength = content.arrayLength;
    for(size_t k=0; k< content.arrayLength; k++){
        //extensionobject is decoded and must be decoded
        if(old_val[k].encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING){
            encode_single_extension_object(server, &old_val[k], &new_val[k], data_type_id, namespaceindex);
            /*old_val[k].content.encoded.typeId.namespaceIndex = namespaceindex;
            const UA_DataType *typ = UA_Server_findDataType(server, &data_type_id);
            UA_DecodeBinaryOptions options;
            options.customTypes = UA_Server_getConfig(server)->customDataTypes;
            new_val[k].encoding = UA_EXTENSIONOBJECT_DECODED;
            new_val[k].content.decoded.type = typ;
            new_val[k].content.decoded.data = UA_malloc(typ->memSize);
            UA_StatusCode retval = UA_decodeBinary(&old_val[k].content.encoded.body, new_val[k].content.decoded.data, typ, &options);
            if(retval != UA_STATUSCODE_GOOD){
                printf("failed to decode the bytestring with error %s\n", UA_StatusCode_name(retval));
                return retval;
            }*/
        }
        //the extensionobject is already decoded
        else{
            UA_ExtensionObject_copy(&old_val[k], &new_val[k]);
        }
    }
    return UA_STATUSCODE_GOOD;
}

/*todo clear memory of the extension objects*/
UA_StatusCode write_variable_value(UA_Server *server, UA_Variant *content, UA_UInt16 namespaceindex, UA_NodeId newNode, UA_NodeId data_type_id){
  UA_StatusCode retval;
  if(content->type == &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]){
                //the variant into which the decoded variable value will be stored
                UA_Variant variable_val;
                UA_Variant_init(&variable_val);
                //case the variant stores an array
                if(content->arrayLength > 0){
                    UA_ExtensionObject *old_val;
                    retval = UA_Array_copy(content->data, content->arrayLength, (void **)&old_val, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
                    if(retval != UA_STATUSCODE_GOOD){
                        printf("failed to copy the capability name array with error %s\n", UA_StatusCode_name(retval));
                        return retval;
                    }
                    UA_ExtensionObject *new_val = UA_ExtensionObject_new();
                    create_new_extension_object_array(server, &variable_val, old_val, *content, new_val, data_type_id, namespaceindex);
                    UA_Variant_setArrayCopy(&variable_val, new_val, content->arrayLength, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
                    UA_Array_delete(old_val, content->arrayLength, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
                    UA_Array_delete(new_val, content->arrayLength, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
                }
                //case variant stores a single value
                else{
                    UA_ExtensionObject old_val;
                    UA_ExtensionObject_init(&old_val);
                    UA_ExtensionObject_copy((UA_ExtensionObject*) content->data, &old_val);
                    UA_ExtensionObject new_val;
                    UA_ExtensionObject_init(&new_val);
                    if(old_val.encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING){
                        encode_single_extension_object(server, &old_val, &new_val, data_type_id, namespaceindex);
                    }
                    //the extensionobject is already decoded
                    else{
                        UA_ExtensionObject_copy(&old_val, &new_val);
                    }
                    UA_Variant_setArrayCopy(&variable_val, &new_val, 1, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
                    UA_ExtensionObject_clear(&old_val);
                    UA_ExtensionObject_clear(&new_val);
                }
                retval = UA_Server_writeValue(server, newNode, variable_val);
                UA_Variant_clear(&variable_val);
                if(retval != UA_STATUSCODE_GOOD){
                    printf("cannot write the variable node error %s\n", UA_StatusCode_name(retval));
                    return retval;
                }
  }
  else{
                //case no extensionobject
                retval = UA_Server_writeValue(server, newNode, *content);
                if(retval != UA_STATUSCODE_GOOD){
                    printf("cannot write the variable node with error %s\n", UA_StatusCode_name(retval));
                    return retval;
                }
  }UA_Variant_clear(content);
  return retval;
}

