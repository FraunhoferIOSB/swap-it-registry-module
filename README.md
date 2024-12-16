# SWAP-IT Registry Module
The SWAP-IT Registry Module is a base module of the SWAP-IT Architecture (https://github.com/swap-it/demo-scenario). The module is supposed to map server of SWAP-Assets and thus,
make them available for process execution steps. Doing so, the Registry Module provides three main functionalities,
which can be accessed through OPC UA Method calls:

1. **Register Asset:** The addition of an Asset to the Registry Module. The Data Values are mapped to the registry module and the
   asset can be considered for process execution steps within the SWAP-IT Architecture.

2. **Unregister Asset:** The removal of an Asset from the Registry Module. The Data Values are no longer mapped to the registry module and the
   asset cannot be considered for process execution steps within the SWAP-IT Architecture anymore.

3. **Filter Asset:** Provision of assets to the swap-it-execution-engine from a set of registered assets. The Filter Asset Method can
   evaluate product capabilities and match them against asset capabilities.


<img src="documentation/source/images/registry_overview.png" alt="">


An extensive documentation or the SWAP-IT Registry Module can be found here: https://fraunhoferiosb.github.io/swap-it-registry-module
or build from the repository. Here, sphinx and the sphinx rtd theme are required. Both can be installed with:

    pip install sphinx 
    pip install sphinx-rtd-theme


Build the documentation:

    cd swap-it-registry-module
    #html
    sphinx-build -M html documentation/source/ documentation/build/html
    #pdf
    sphinx-build -b pdf documentation/source/ documentation/build/pdf/


## Dependencies
The SWAP-IT Registry Module requires a locally installed version of the C-based OPC UA SDK open62541 (https://github.com/open62541/open62541) version 1.4.6,
as well as a locally installed version of the  swap-it-open62541-server-template (https://github.com/FraunhoferIOSB/swap-it-open62541-server-template).


## Build the Registry-Module:

The default URL of the registry module is: **opc.tcp://localhost:8000**

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

    /*build and start the registry module*/
    git clone https://github.com/FraunhoferIOSB/swap-it-registry-module
    cd swap-it-registry-module
    mkdir build && cd build
    cmake ..
    make 
    ./swap-it-registry-module

## Related Projects
Since the SWAP-IT Registry Module is part of the SWAP-IT Architecture, its application is linked to other SWAP-IT projects. Here are some other relevant repositories:

- SWAP-IT Demo Scenario: https://github.com/swap-it/demo-scenario
- SWAP-IT open62541 server-template: https://github.com/FraunhoferIOSB/swap-it-open62541-server-template
- SWAP-IT Execution Engine: https://github.com/FraunhoferIOSB/swap-it-execution-engine
- PFDL Scheduler: https://github.com/iml130/pfdl
- SWAP-IT Dashboard: https://github.com/iml130/swap-it-dashboard
- Common Information Model: https://github.com/FraunhoferIOSB/swap-it-common-information-model

