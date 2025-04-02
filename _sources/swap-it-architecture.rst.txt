..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

====================
SWAP-IT Architecture
====================

The SWAP-IT architecture introduces a modular, scalable and lightweight architecture utilizing simplified semantic information
models. In general, components of the architecture are split into two types of modules. On the one hand, the *Basic Modules* are the core components of the architecture
by enabling the interaction with the loosely coupled components. All basic components will be published as open source software projects for a simple adaption to enable an automate integration of new processes, based on the
`Production Flow Description Language (PFDL) <https://github.com/iml130/pfdl>`_. On the other hand, *Use Case specific Modules* integrate existing shop floor assets, either physical assets, such as production resources, or non physical assets,
such as optimization approaches to assign jobs to resources. Since these components can vary from production environment to production environment,
the SWAP-IT architecture provides an easy integration mechanism for existing and new assets. As a result, the SWAP-IT architecture enables industrial production environments with a high degrees
of flexibility, easy process integration and in addition, addresses further challenges of production environments, such as an automated
resource parameterization for individual products, or an easy integration of new assets into a shop floor.

.. figure:: /images/swap_it_general.PNG
   :alt: SWAP-IT Overview
   :width: 100%

For this to be achieved, one process module executes exactly one process that is specified in the PFDL format.
The Enterprise Resource Planning (ERP) module is utilized to submit orders and the registry module makes resources available for process agents. Each resource on a shop floor is defined as a SWAP-IT Asset (*Use-Case Specific Modules*). Such assets can easily
be defined by utilizing the SWAP-IT `Common Information Model <https://github.com/FraunhoferIOSB/swap-it-common-information-model>`_. As a result, new assets can easily be integrated into the architecture.
An example interaction between the SWAP-IT assets is shown in the sequence diagram below. As starting point, each shop floor *Asset* that should be available for the execution of process steps has to register itself in the *Registry Module*. Here, an arbitrary number of assets, that
are able to execute any given process step, can register themself within the *Registry Module*.
An Process execution starts with the submission of an order from a *Customer*. Afterwards, the *ERP Module* generates the process description (*PFDL*) for the order and transmits it to a *Process Agent*. Within the *Process Agent* the *PFDL Scheduler* interprets
the the PFDL and starts to schedule specified services. Each scheduled service is then transmitted to the *Execution Engine*, which in turn, requests all suitable *Assets* to execute the service from the *Registry Module*. Afterwards, the *Execution Engine* interacts
with an *Assignment Asset* to find the best asset in the context of the specified optimization approach. While these interaction steps are the recommended way to assign a service to an *Asset*, the architecture incorporates additional assignment approaches. Further details
about these approaches can be found in the  `Process Agent Project <https://fraunhoferiosb.github.io/swap-it-execution-engine/ResourceAssignment.html>`_. After an *Asset* is found, the *Execution Engine* initiates the service execution on this particular asset and then, returns the service result to the *PFDL Scheduler*. Although the
sequence diagram only displays a single service execution, the *Process Agent* can execute an arbitrary number of process steps until the production process of a product is completed. After the completion of the production process, the *ERP Module* receives a notification from
the *Process Agent* and can inform the customer that the order is completed.

.. figure:: /images/SWAPSequence.PNG
   :alt: Sequence Diagram SWAP-It Components
   :width: 100%


SWAP-IT Software
================

The application consists of a set of loosely coupled software components. The figure below depicts all software components that are currently available as open source projects, or will be published as open source projects within 2024.
The development status and the interaction of these components is shown in the figure below.

.. figure:: /images/img_1.png
   :alt: Development Status SWAP-IT Software Components
   :width: 100%