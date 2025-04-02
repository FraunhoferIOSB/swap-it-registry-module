..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

====================
Register SWAP Assets
====================

There are several ways to register SWAP Assets within the Registry Module:
- Add Agent Method within the Registry Module
- Register Method from the `Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_ (the method callback is provided within the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_)
- Json Configuration of the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_

For all registering approaches, the Registry Module must be started first:

.. code-block:: c

    /*build and start the registry module*/
    git clone https://github.com/FraunhoferIOSB/swap-it-registry-module
    cd swap-it-registry-module
    mkdir build && cd build
    cmake ..
    make
    ./swap-it-registry-module

Manual Registration
====================
The repository provides a dummy server that can be deployed to test the manual registration. The server utilizes the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_, however, in the first steps,
tha manual registration, namely without the json configuration, are illustrated. The dummy server corresponds to the `SWAP-IT Demo Scenario Beginner Tutorial <https://swap-it.github.io/demo-scenario/beginner.html>`_ so that the construction of the server is not explained here in detail.

The code for the server can be found within *tutorials/register/register_manually.c* in this repository. The executable can be started with

.. _register_manually:
.. code-block:: c

   cd build
   ./tutorials/register_manually

On the code basis, the SWAP Server is configured based on a JSON:

.. _JSON:

.. code-block:: c

    {
      application_name: "warehouse",
      resource_ip: "localhost",
      port: "4840",
      module_type: "WarehouseModuleType",
      module_name: "WarehouseModule",
      service_name: "GetPartsFromWarehouse"
    }

and build with the open62541-server-template:


.. code-block:: c

    UA_ByteString conf = UA_String_fromChars("{\n"
                                                 "  application_name: \"warehouse_dr1\",\n"
                                                 "  resource_ip: \"localhost\",\n"
                                                 "  port: \"4840\",\n"
                                                 "  module_type: \"WarehouseModuleType\",\n"
                                                 "  module_name: \"WarehouseModule\",\n"
                                                 "  service_name: \"GetPartsFromWarehouse\",\n"
                                                 "}");

    /* the structure UA_service_server_interpreter will return the pre-defined information about the server
    * from the json configuration*/
    UA_service_server_interpreter swap_server;
    memset(&swap_server, 0, sizeof(UA_service_server_interpreter));
    /* with the function UA_server_swap_it from the open62541 server template,
    * it is possible to configure the OPC UA server with a single function call*/
    UA_server_swap_it(server, conf, warehousemethodCallback, UA_FALSE, &running, UA_FALSE, &swap_server);
    UA_ByteString_clear(&conf);

The server can now be registered within the registry module, either with the *Add Agent Method* provided by the registry module, or with the *register method* inside the server. Both can be called with the UA Expert Client.

Registering with the Add Agent Method
--------------------------------------
First, the server can now be registered from the registry:

.. figure:: /images/add_agent.png
   :alt: SWAP-IT Overview
   :width: 100%

The Method requires four arguments:

    - service_name: Name of the Service offered from the server
    - address: the web-address of the server
    - port: the port of the server
    - moduleType: the name of the SWAP-Module (see `Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_) that is offered by the server

Now, its possible to set the values based on the `JSON`_ Config:

    - service_name: GetPartsFromWarehouse
    - address: localhost
    - port: 4840
    - moduleType: WarehouseModule

.. figure:: /images/add_agent_parameterized.png
   :alt: SWAP-IT Overview
   :width: 100%

Now we can check the registered server in the registry module. When changing some of the mapped values within the swap server (opc.tcp://localhost:4840), they are now mapped to and synchronized with the registry.

.. figure:: /images/registered_agent.png
   :alt: SWAP-IT Overview
   :width: 100%

Registering with the register Method
------------------------------------

The second manual registering approach arises from the server itself. The `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_ includes the corresponding callback, so that,
when utilizing the template to build the server, the method can be called directly. The server to be registers is the same compared to the Add Agent registration, however, the server, as well as the registry module,
must be restarted or the server must be unregistered from the registry module, since a single server can only be registered once.

The register method requires different arguments, compared to the add agent method:

.. figure:: /images/register.png
   :alt: SWAP-IT Overview
   :width: 100%

The Method requires two arguments:

    - Registry URL: URL of the registry module
    - Resource URL: URL of the server

Both arguments can be set with:

    - Registry URL: localhost:8000
    - Resource URL: localhost:4840

.. figure:: /images/register_parameterized.png
   :alt: SWAP-IT Overview
   :width: 100%

Now we can check the registered server in the registry module. When changing some of the mapped values within the swap server (opc.tcp://localhost:4840), they are now mapped to and synchronized with the registry.

.. figure:: /images/registered_agent.png
   :alt: SWAP-IT Overview
   :width: 100%


.. _Registration:

Registration with the open62541-server-template
===============================================

For the registration with the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_, the JSON config has to be appended with the argument *device_registry:"opc.tcp://localhost:8000"*

.. code-block:: c

    {
      application_name: "warehouse",
      resource_ip: "localhost",
      port: "4840",
      module_type: "WarehouseModuleType",
      module_name: "WarehouseModule",
      service_name: "GetPartsFromWarehouse",
      device_registry:"opc.tcp://localhost:8000"
    }

Besides, the argument  *UA_Boolean register_agent_in_registry* of the utility function *UA_server_swap_it()* has to be set to true.

.. code-block:: c

    UA_ByteString conf = UA_String_fromChars("{\n"
                                                 "  application_name: \"warehouse_dr1\",\n"
                                                 "  resource_ip: \"localhost\",\n"
                                                 "  port: \"4840\",\n"
                                                 "  module_type: \"WarehouseModuleType\",\n"
                                                 "  module_name: \"WarehouseModule\",\n"
                                                 "  service_name: \"GetPartsFromWarehouse\",\n"
                                                 "  device_registry:\"opc.tcp://localhost:8000\"\n"
                                                 "}");

    /* the structure UA_service_server_interpreter will return the pre-defined information about the server
    * from the json configuration*/
    UA_service_server_interpreter swap_server;
    memset(&swap_server, 0, sizeof(UA_service_server_interpreter));
    /* with the function UA_server_swap_it from the open62541 server template,
    * it is possible to configure the OPC UA server with a single function call*/
    UA_server_swap_it(server, conf, warehousemethodCallback, UA_FALSE, &running, UA_TRUE, &swap_server);
    UA_ByteString_clear(&conf);


Now, th asset can now be registered by simply staring the executable. However, it must be considered that the registry module must be running to register an agent

.. code-block:: c

   cd build
   ./tutorials/register