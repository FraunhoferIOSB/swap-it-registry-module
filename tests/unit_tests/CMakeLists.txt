#[[/*
* Licensed under the MIT License.
* For details on the licensing terms, see the LICENSE file.
* SPDX-License-Identifier: MIT
*
* Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
*
*/]]

find_package(Check REQUIRED)
set(registry_module_mechanism_sources
        ../../src/registry_module_internal.c
        ../../src/registry_module_config_parse.c
        ../../src/registry_module_client_interface.c
        ../../src/registry_module_node_iterator.c
        ../../src/registry_module_clear.c
        ../../src/registry_module_namespace_solver.c
        ../../src/registry_module_synchronization.c
        ../../src/registry_module_init.c
        ../../src/registry_module_callback_functions.c
        ../../src/registry_module_capability_matching_functions.c
        ../../src/registry_module_register_agent.c
)

set(registry_module_mechanism_deps
        ../../deps/cj5.c
        ../../deps/parse_num.c
)

set(test_sources
        ../src/start_registry_module.c
        ../src/start_client.c
        ../src/start_swap_server.c
        ../src/evaluate_test_results.c
)

set(COMMON_INFORMATION_MODEL_DIR ${PROJECT_SOURCE_DIR}/model)
set(INFORMATION_MODEL_DIR ${PROJECT_SOURCE_DIR}/tests/InformationModels)
#generate c code from the common model
ua_generate_nodeset_and_datatypes(
        NAME "common"
        FILE_BSD "${COMMON_INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        FILE_CSV "${COMMON_INFORMATION_MODEL_DIR}/CommonModelDesign.csv"
        FILE_NS "${COMMON_INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.NodeSet2.xml"
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
        IMPORT_BSD "TYPES_COMMON#${COMMON_INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Common.Model.Types.bsd"
        IMPORT_BSD "TYPES_PFDL_PARAMETER#${INFORMATION_MODEL_DIR}/SWAP.Fraunhofer.Demo.Scenario.Model.Types.bsd"
        OUTPUT_DIR ${GENERATE_OUTPUT_DIR}
        INTERNAL
)

set(custom_information_models
        ${UA_NODESET_COMMON_SOURCES}
        ${UA_TYPES_COMMON_SOURCES}
        ${UA_NODESET_PFDL_PARAMETER_SOURCES}
        ${UA_TYPES_PFDL_PARAMETER_SOURCES}
        ${UA_NODESET_WAREHOUSE_SOURCES}
        ${UA_TYPES_WAREHOUSE_SOURCES}
)

add_executable(check_registry_module
        ../check_registry_module_configs.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)

add_dependencies(check_registry_module open62541-generator-ns-common)
target_link_libraries(check_registry_module swap_server_template)
target_link_libraries(check_registry_module open62541::open62541)
target_link_libraries(check_registry_module pthread)
target_link_libraries(check_registry_module ${CHECK_LIBRARIES})
add_test(NAME check_registry_module
        COMMAND check_registry_module)

add_executable(check_registry_module_structure
        ../check_registry_module_structure.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
        ../include/start_swap_server.h
)
add_dependencies(check_registry_module_structure open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_structure swap_server_template)
target_link_libraries(check_registry_module_structure open62541::open62541)
target_link_libraries(check_registry_module_structure pthread)
target_link_libraries(check_registry_module_structure ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_structure
        COMMAND check_registry_module_structure)

add_executable(check_registry_module_method_call
        ../check_registry_module_method_call.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)

add_dependencies(check_registry_module_method_call open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_method_call swap_server_template)
target_link_libraries(check_registry_module_method_call open62541::open62541)
target_link_libraries(check_registry_module_method_call pthread)
target_link_libraries(check_registry_module_method_call ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_method_call
        COMMAND check_registry_module_method_call)

add_executable(check_registry_module_synchronization
        ../check_registry_module_synchronization.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)
add_dependencies(check_registry_module_synchronization open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_synchronization swap_server_template)
target_link_libraries(check_registry_module_synchronization open62541::open62541)
target_link_libraries(check_registry_module_synchronization pthread)
target_link_libraries(check_registry_module_synchronization ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_synchronization
        COMMAND check_registry_module_synchronization)

add_executable(check_registry_module_register_mechanism
        ../check_registry_module_register_agent.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)

add_dependencies(check_registry_module_register_mechanism open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_register_mechanism swap_server_template)
target_link_libraries(check_registry_module_register_mechanism open62541::open62541)
target_link_libraries(check_registry_module_register_mechanism pthread)
target_link_libraries(check_registry_module_register_mechanism ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_register_mechanism
        COMMAND check_registry_module_register_mechanism)

add_executable(check_registry_module_unregister_mechanism
        ../check_register_module_unregister_agent.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)

add_dependencies(check_registry_module_unregister_mechanism open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_unregister_mechanism swap_server_template)
target_link_libraries(check_registry_module_unregister_mechanism open62541::open62541)
target_link_libraries(check_registry_module_unregister_mechanism pthread)
target_link_libraries(check_registry_module_unregister_mechanism ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_unregister_mechanism
        COMMAND check_registry_module_unregister_mechanism)


add_executable(check_registry_module_filter_mechanism
        ../check_registry_module_filter_agents.c
        ${test_sources}
        ${custom_information_models}
        ${registry_module_mechanism_sources}
        ${registry_module_mechanism_deps}
)

add_dependencies(check_registry_module_filter_mechanism open62541-generator-ns-common open62541-generator-ns-warehouse)
target_link_libraries(check_registry_module_filter_mechanism swap_server_template)
target_link_libraries(check_registry_module_filter_mechanism open62541::open62541)
target_link_libraries(check_registry_module_filter_mechanism pthread)
target_link_libraries(check_registry_module_filter_mechanism ${CHECK_LIBRARIES})
add_test(NAME check_registry_module_filter_mechanism
        COMMAND check_registry_module_filter_mechanism)

configure_file(../configs/config_empty.json5 ${CMAKE_CURRENT_BINARY_DIR}/config_empty.json5 COPYONLY)
configure_file(../configs/config_incomplete.json5 ${CMAKE_CURRENT_BINARY_DIR}/config_incomplete.json5 COPYONLY)
configure_file(../configs/config_valid.json5 ${CMAKE_CURRENT_BINARY_DIR}/config_valid.json5 COPYONLY)
