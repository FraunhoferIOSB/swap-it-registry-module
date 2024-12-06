#[[/*
* Licensed under the MIT License.
* For details on the licensing terms, see the LICENSE file.
* SPDX-License-Identifier: MIT
*
* Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
*
*/]]

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

add_executable(register_manually
        register/register_manually.c
        ${custom_information_models}
)
add_dependencies(register_manually open62541-generator-ns-common open62541-generator-ns-pfdl_parameter open62541-generator-ns-warehouse)
target_link_libraries(register_manually swap_server_template)
target_link_libraries(register_manually open62541::open62541)

add_executable(register
        register/register.c
        ${custom_information_models}
)
add_dependencies(register open62541-generator-ns-common open62541-generator-ns-pfdl_parameter open62541-generator-ns-warehouse)
target_link_libraries(register swap_server_template)
target_link_libraries(register open62541::open62541)

add_executable(unregister
        unregister/unregister.c
        ${custom_information_models}
)
add_dependencies(unregister open62541-generator-ns-common open62541-generator-ns-pfdl_parameter open62541-generator-ns-warehouse)
target_link_libraries(unregister swap_server_template)
target_link_libraries(unregister open62541::open62541)

add_executable(filter
        filter/filter.c
        ../tests/src/start_swap_server.c
        ${custom_information_models}
)
add_dependencies(filter open62541-generator-ns-common open62541-generator-ns-pfdl_parameter open62541-generator-ns-warehouse)
target_link_libraries(filter swap_server_template)
target_link_libraries(filter open62541::open62541)


add_executable(service_filtering
        filter/service_filtering.c
)
target_link_libraries(service_filtering open62541::open62541)

add_executable(capability_filtering
        filter/capability_filtering.c
)
target_link_libraries(capability_filtering open62541::open62541)