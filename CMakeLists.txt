#[[/*
* Licensed under the MIT License.
* For details on the licensing terms, see the LICENSE file.
* SPDX-License-Identifier: MIT
*
* Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
*
*/]]
cmake_minimum_required(VERSION 3.20.0)
project(swap-it-registry-module)

find_package(open62541 1.4 REQUIRED)
set(CMAKE_C_STANDARD 99)

#set( CMAKE_C_FLAGS "-Wall -Werror -Wunused -O4 -fvisibility=hidden ${CMAKE_C_FLAGS}" )
set(GCC_COVERAGE_COMPILE_FLAGS  "--coverage -g -O0 -fprofile-arcs -ftest-coverage")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")


file(MAKE_DIRECTORY "${GENERATE_OUTPUT_DIR}")
set(GENERATE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/src_generated/")
include_directories("${GENERATE_OUTPUT_DIR}")

set(PATH_TO_INFORMATION_MODELS ${PROJECT_SOURCE_DIR}/model)
set(INFORMATION_MODEL_DIR ${PROJECT_SOURCE_DIR}/tests/InformationModels)

set(INCLUDE_SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/registry_module_config_parse.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_init.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_clear.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_client_interface.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_internal.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_namespace_solver.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_node_iterator.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_synchronization.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_capability_matching_functions.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_register_agent.c
        ${PROJECT_SOURCE_DIR}/src/registry_module_callback_functions.c
)

set(INCLUDE_HDR_FILES
        ${PROJECT_SOURCE_DIR}/include/registry_module_config_parse.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_init.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_clear.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_client_interface.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_internal.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_namespace_solver.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_node_iterator.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_synchronization.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_capability_matching_functions.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_register_agent.h
        ${PROJECT_SOURCE_DIR}/include/registry_module_callback_functions.h
)

#generate c code from the common model
ua_generate_nodeset_and_datatypes(
        NAME "common"
        FILE_BSD "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        FILE_CSV "${PATH_TO_INFORMATION_MODELS}/CommonModelDesign.csv"
        FILE_NS "${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.NodeSet2.xml"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)

#generate c code from the pfdl types model
ua_generate_nodeset_and_datatypes(
        NAME "pfdl_parameter"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/DemoScenarioTypes.ModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.NodeSet2.xml"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)
#generate c code from the warehouse model and set the dependencies to the common and pfdl types models
ua_generate_nodeset_and_datatypes(
        NAME "warehouse"
        DEPENDS "common"
        DEPENDS "pfdl_parameter"
        FILE_BSD "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.Types.bsd"
        FILE_CSV "${INFORMATION_MODEL_DIR}/DemoScenarioWarehouse.ModelDesign.csv"
        FILE_NS "${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Warehouse.Model.NodeSet2.xml"
        IMPORT_BSD "TYPES_COMMON#${PATH_TO_INFORMATION_MODELS}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        IMPORT_BSD "TYPES_PFDL_PARAMETER#${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)

add_executable(swap-it-registry-module main.c
        ${UA_NODESET_COMMON_SOURCES}
        ${UA_TYPES_COMMON_SOURCES}
        deps/cj5.c
        deps/parse_num.c
        ${INCLUDE_SRC_FILES}
        ${INCLUDE_HDR_FILES}
)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/deps/cmake")
add_dependencies(swap-it-registry-module open62541-generator-ns-common)
target_link_libraries(swap-it-registry-module open62541::open62541)
target_link_libraries(swap-it-registry-module pthread)


set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/deps/cmake")
enable_testing()
add_subdirectory(tests/unit_tests)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
