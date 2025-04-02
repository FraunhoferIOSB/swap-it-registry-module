..
    Licensed under the MIT License.
    For details on the licensing terms, see the LICENSE file.
    SPDX-License-Identifier: MIT

   Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

==========================================
Build the Registry Module from the Scratch
==========================================
To run the SWAP-IT registry module, a local installed version of the C-based OPC UA SDK `open62541 <https://github.com/open62541/open62541>`_, version 1.4.6, is required, as well as a local installed version of the `open62541-server-template <https://github.com/FraunhoferIOSB/swap-it-open62541-server-template>`_.

Installation Requirements
=========================

.. code-block:: c

    /*install dependencies check and open62541*/
    apt-get -y update
    apt-get -y install git build-essential gcc pkg-config cmake python3 check
    git clone https://github.com/open62541/open62541
    cd open62541

    /*switch to open62541 version 1.4.6*/
    git fetch --all --tags
    git checkout tags/v1.4.6 -b v1.4.6-branch

    /* init submodules, build and install open62541*/
    git submodule update --init --recursive
    mkdir build && cd build
    cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUA_NAMESPACE_ZERO=FULL -DUA_ENABLE_JSON_ENCODING=ON -DUA_MULTITHREADING=200 ..
    make install

    /*install the server template*/
    git clone https://github.com/FraunhoferIOSB/swap-it-open62541-server-template
    cd swap-it-open62541-server-template
    mkdir build && cd build
    cmake ..
    make install

Build and Deploy the Registry Module
====================================

After installing the dependencies, the registry module can be deployed with

.. code-block:: c

    /*build and start the registry module*/
    git clone https://github.com/FraunhoferIOSB/swap-it-registry-module
    cd swap-it-registry-module
    mkdir build && cd build
    cmake ..
    make
    ./swap-it-registry-module
