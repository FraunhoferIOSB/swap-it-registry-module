/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef EVALUATE_TEST_RESULTS_H
#define EVALUATE_TEST_RESULTS_H

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel_async.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/server_config_default.h>
#include "open62541/client_highlevel.h"
#include "types_common_generated.h"
#include "types_common_generated_handling.h"


int check_name_list(UA_QualifiedName *sources, UA_QualifiedName target, size_t num_sources);
UA_Boolean check_string(UA_Client *aggregate_client, UA_Client *aggregation_client,
                        UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server);
UA_Boolean check_state(UA_Client *aggregate_client, UA_Client *aggregation_client,
                       UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server);
UA_Boolean check_num_capa(UA_Client *aggregate_client, UA_Client *aggregation_client,
    UA_Variant value, UA_NodeId aggregate_nodeId, UA_NodeId aggregation_nodeId, UA_Boolean write_aggregation_server);
void check_queue(UA_Queue_Data_Type val_1, UA_Queue_Data_Type val_2, UA_Int16 entry_number, UA_Boolean initial_value);
UA_Boolean check_registered_agents(UA_QualifiedName *nameList, size_t nameListSize, UA_QualifiedName targetName, size_t target_nbr);

#endif //EVALUATE_TEST_RESULTS_H
