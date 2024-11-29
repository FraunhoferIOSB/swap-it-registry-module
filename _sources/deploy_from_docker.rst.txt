..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

=================================
Deploy pre-build Docker container
=================================

The `SWAP-IT Demo Scenario <https://github.com/swap-it/demo-scenario>`_ Repository provides pre-build docker container for SWAP-IT software components. From there, a pre-build container of the Registry Module can be deployed with:

.. code-block:: c

    //pull the image
    docker pull ghcr.io/swap-it/demo-scenario/device_registry:latest

    //start the image
    docker run -p 8000:8000 --add-host host.docker.internal:host-gateway ghcr.io/swap-it/demo-scenario/device_registry:latest