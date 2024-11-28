# Registry Module


## Build Documentation

To build the documentation, sphinx and the sphinx rtd theme are required. Both can be installed with:

    pip install sphinx 
    pip install sphinx-rtd-theme


Build the documentation:

    cd swap-it-registry-module
    sphinx-build -M html documentation/source/ documentation/build/

## Build the Registry-Module:

    /*install dependencies check and open62541)*/
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

Build the documentation:

    cd this repository
    #html
    sphinx-build -M html documentation/source/ documentation/build/html
    #pdf
    sphinx-build -b pdf documentation/source/ documentation/build/pdf/