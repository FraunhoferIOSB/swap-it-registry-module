..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

======================
Unregister SWAP Assets
======================

There are several ways to unregister SWAP Assets within the Registry Module:
- Remove Agent Method within the Registry Module
- Unregister Method from the `Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_ (the method callback is provided within the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_)
- Json Configuration of the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_. However, this approach automatically unregisters the agent when shutting down the server, so that no code example will be provided. The code example is available at `Registration`. The server must just be shut down for the un-registration.

First, the Registry Module must be started:

.. code-block:: c

    /*build and start the registry module*/
    git clone https://github.com/FraunhoferIOSB/swap-it-registry-module
    cd swap-it-registry-module
    mkdir build && cd build
    cmake ..
    make
    ./swap-it-registry-module

as well as a swap-asset, which registers itself:


.. code-block:: c

    cd build
    ./tutorials/unregister

Unregistering with the Remove Agent Method
============================================

First the asset can be removed from the registry module with the remove agent method inside the registry module:

The Method requires three arguments:

    - agent_url: the web-address of the server
    - agent_port: the port of the server
    - service_name: Name of the Service offered from the server

.. figure:: /images/remove_agent.png
   :alt: SWAP-IT Overview
   :width: 100%

Now, its possible to set the values and execute the method:

    - agent_url: localhost
    - agent_port: 4840
    - service_name: GetPartsFromWarehouse

.. figure:: /images/remove_agent_parameterized.png
   :alt: SWAP-IT Overview
   :width: 100%


Finally, the server is removed from the registry module:

.. figure:: /images/removed_agent.png
   :alt: SWAP-IT Overview
   :width: 100%

Unregistering with the unregister Method
========================================

Similar to the registering approach, assets can be removed as well from the server side. Here, the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_ provides a corresponding unregister method callback,

.. figure:: /images/unregister.png
   :alt: SWAP-IT Overview
   :width: 100%

The method requires the following arguments:

The Method requires two arguments:

    - Registry URL: URL of the registry module
    - Resource URL: URL of the server

Both arguments can be set with:

    - Registry URL: localhost:8000
    - Resource URL: localhost:4840

.. figure:: /images/unregister_parameterized.png
   :alt: SWAP-IT Overview
   :width: 100%

Finally, the server is removed from the registry module:

.. figure:: /images/removed_agent.png
   :alt: SWAP-IT Overview
   :width: 100%