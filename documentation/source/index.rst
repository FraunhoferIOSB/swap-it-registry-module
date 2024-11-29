..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

.. toctree::
   :maxdepth: 4


.. _SWAP_IT_REGISTRY_MODULE:

=======================
SWAP-IT Registry Module
=======================

The SWAP-IT Registry Module is a base module of the `SWAP-IT Architecture <https://github.com/swap-it/demo-scenario>`_. The module is supposed to map server of SWAP-Assets and thus,
make them available for process execution steps. Doing so, the Registry Module provides three main functionalities,
which can be accessed through OPC UA Method calls:

1. **Register Asset:** The addition of an Asset to the Registry Module. The Data Values are mapped to the registry module and the
asset can be considered for process execution steps within the SWAP-IT Architecture.

2. **Unregister Asset:** The removal of an Asset from the Registry Module. The Data Values are no longer mapped to the registry module and the
asset cannot be considered for process execution steps within the SWAP-IT Architecture anymore.

3. **Filter Asset:** Provision of assets to the swap-it-execution-engine from a set of registered assets. The Filter Asset Method can
evaluate product capabilities and match them against asset capabilities.


.. figure:: /images/registry_overview.png
   :alt: SWAP-IT Overview
   :width: 100%

.. toctree::
   :maxdepth: 4
   :caption: Contents:

   swap-it-architecture
   getting_started
   related_projects
   glossary
   contact



