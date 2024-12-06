..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

====================
Filter SWAP Assets
====================

In this tutorial, two approaches are introduced on how to filter assets from the registry module:

    - Filtering with the `UA-Expert Client <https://www.unified-automation.com/de/downloads/opc-ua-clients.html>`_
    - Implementing an individual `open62541-based <https://github.com/open62541/open62541>`_ client application for the filtering

Starting the Environment
========================

For the filtering, a set of resources with different capabilities is required. Here, the Warehouse resource configs from the unit test are re-used for simplicity.
In addition, it is possible to re-use a utility function from the unit tests *start_swap_server_thread()* for a simplified start of resources. Since the resources utilize the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_
they are automatically registered within the registry module. However, it is required to start the swap-it-registry-module first, so that the resources are registered successfully:

.. code-block:: c

    /*start the registry module*/
    cd swap-it-registry-module
    mkdir build && cd build
    cmake ..
    make
    ./swap-it-registry-module

Next, the resources are configured. The code is also available in the `Filter Tutorial <https://github.com/FraunhoferIOSB/swap-it-registry-module/tree/main/tutorials/filter/filter.c>`_. Here, the structure *test_swap_server_config*
is set for each resource, so that five resources are configured. Since each resource runs in a separate thread, the loop in the main function executes only a sleep.

.. code-block:: c

    #include "../../tests/include/start_swap_server.h"

    UA_Boolean running = true;
    static void stopHandler(int sign) {
      UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
      running = false;
    }

    int main(void) {

      /*run a swap server that registers itself in the registry module*/
      test_swap_server_config aggregate_conf_1;
      aggregate_conf_1.register_server = true;
      aggregate_conf_1.running = &running;
      aggregate_conf_1.conf = "../../tests/configs/warehouse.json5";
      start_swap_server_thread(&aggregate_conf_1);

      test_swap_server_config aggregate_conf_2;
      aggregate_conf_2.register_server = true;
      aggregate_conf_2.running = &running;
      aggregate_conf_2.conf = "../../tests/configs/warehouse_1.json5";
      start_swap_server_thread(&aggregate_conf_2);

      test_swap_server_config aggregate_conf_3;
      aggregate_conf_3.register_server = true;
      aggregate_conf_3.running = &running;
      aggregate_conf_3.conf = "../../tests/configs/warehouse_2.json5";
      start_swap_server_thread(&aggregate_conf_3);

      test_swap_server_config aggregate_conf_4;
      aggregate_conf_4.register_server = true;
      aggregate_conf_4.running = &running;
      aggregate_conf_4.conf = "../../tests/configs/warehouse_3.json5";
      start_swap_server_thread(&aggregate_conf_4);

      test_swap_server_config aggregate_conf_5;
      aggregate_conf_5.register_server = true;
      aggregate_conf_5.running = &running;
      aggregate_conf_5.conf = "../../tests/configs/warehouse_4.json5";
      start_swap_server_thread(&aggregate_conf_5);

      while(running)
        sleep(1);

      return EXIT_SUCCESS;
    }

With the code, it is now possible to execute the script:

.. code-block:: c

    cd build
    cmake ..
    make filter
    ./filter

The resources run on the following urls:

    - opc.tcp://localhost:4080
    - opc.tcp://localhost:4081
    - opc.tcp://localhost:4082
    - opc.tcp://localhost:4083
    - opc.tcp://localhost:4084

The ports can be changed in the `JSON configuration files <https://github.com/FraunhoferIOSB/swap-it-registry-module/tree/main/tests/configs>`_ of the resources.
The registered resources can now be check when connecting to the registry module, which runs on the url *opc.tcp://localhost:8000*

Filtering with the UA-Expert Client
===================================

For a simple testing, the resources can be filtered with only setting the *service_name* argument of the *Filter Agents* method:

    - service_name:GetPartsFromWarehouse

and the invoking the method.

.. figure:: /images/call_empty_filter.png
   :alt: SWAP-IT Overview
   :width: 100%

The registry module then returns a list with the NodeIds, of the corresponding objects within the registry module, of all suitable resources. However, since no capabilities are set, all resources are returned:

.. figure:: /images/result_empty_filter.png
   :alt: SWAP-IT Overview
   :width: 100%

In the next step, it is now possible to set some capabilities to restrict the number of returned resources, evaluated by the registry module. Each of the resources provides a numeric, a boolean and a string capability:

    - test_numeric
    - test_boolean
    - test_string

For this, the arguments of the method call must be adjusted with additional arguments, for example when filtering based on the test_string, the method arguments must be set as following:

    - service_name:GetPartsFromWarehouse
    - Capability_Names:{test_string}
    - Capability_Items:{test string}

.. figure:: /images/filter_ua_capa.png
   :alt: SWAP-IT Overview
   :width: 100%

**Important Note:**

Both, the Capability_Names and the Capability_Items are Arrays, so that, even for single capability values, the arguments must be provided as arrays. In addition, the length of both arrays have to be equal.

Different to the method call with no capabilities, the method returns with these arguments only a single resource:

.. figure:: /images/call_capa_results.png
   :alt: SWAP-IT Overview
   :width: 100%

Filtering with the open62541-Client
===================================

For the open62541 client application, first a stop handler must be defined, as well as a custom structure type which is deployed to search the method node within the registry module address space:

.. code-block:: c

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

Next, a callback function to search the method node is required:

.. code-block:: c

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
     /*continue the search from the current child Node*/
     UA_Client_forEachChildNodeCall(handler->client, childId, search_method_node, handle);
     UA_QualifiedName_clear(&qname);
     return UA_STATUSCODE_GOOD;
    }

Lastly, it is now possible to define the main loop of the client, which includes the creation of the client, the browsing of the registry module address space for the Filter Agents Method,
the definition of input arguments, as well as the method call with the client. For the first `approach (service_filtering) <https://github.com/FraunhoferIOSB/swap-it-registry-module/blob/main/tutorials/filter/service_filtering.c>`_,
no capabilities are specified, so that the NodeId from each of the five available assets is returned:

.. code-block:: c

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

The application then returns the following results from the Filter Agents Method:

    -  i=54820
    -  i=55006
    -  i=55024
    -  i=54986
    -  i=54948

**Important Note**

Since the NodeIds of the registered assets are generated during the registration process, the returned NodeIds from individual executions might differ to the above listed.


For the second approach, a String and a numeric capability is set as input argument, so that only a subset of the five assets is returned from the Filter Agents Method.
The Code from the first approach can be re-used, however beginning at the section *create the input arguments*, the code must be adjusted as followed:

.. code-block:: c

    /*create the input arguments*/
     UA_Variant *inp = (UA_Variant*) UA_Array_new(3, &UA_TYPES[UA_TYPES_VARIANT]);
     UA_String service_name = UA_String_fromChars("GetPartsFromWarehouse");

     UA_String *capability_names = (UA_String*) UA_Array_new(2, &UA_TYPES[UA_TYPES_STRING]);
     UA_String *capability_values = (UA_String*) UA_Array_new(2, &UA_TYPES[UA_TYPES_STRING]);
     capability_names[0] = UA_String_fromChars("test_string");
     capability_names[1] = UA_String_fromChars("test_numeric");
     capability_values[0] = UA_String_fromChars("test string");
     capability_values[1] = UA_String_fromChars("60");

     UA_Variant_setScalarCopy(&inp[0], &service_name, &UA_TYPES[UA_TYPES_STRING]);
     UA_Variant_setArrayCopy(&inp[1], capability_names,2, &UA_TYPES[UA_TYPES_STRING]);
     UA_Variant_setArrayCopy(&inp[2], capability_values,2, &UA_TYPES[UA_TYPES_STRING]);
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
      UA_Array_delete(capability_names, 2, &UA_TYPES[UA_TYPES_STRING]);
      UA_Array_delete(capability_values, 2, &UA_TYPES[UA_TYPES_STRING]);
      UA_Variant_delete(outp);
      UA_String_clear(&service_name);
      return retval;

The returned results from the Filter Agents Method is a single asset with the NodeId:

    -  i=54892

**Important Note**

Since the NodeIds of the registered assets are generated during the registration process, the returned NodeIds from individual executions might differ to the above listed.





