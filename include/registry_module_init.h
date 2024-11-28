/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_INIT_H_
#define REGISTRY_MODULE_INIT_H_

#include <open62541/server.h>
#include "registry_module_client_interface.h"
#include "registry_module_internal.h"
#include "registry_module_callback_functions.h"

UA_StatusCode initRegistryModuleServerStructure(UA_Server *server);
UA_StatusCode initAggregationServerNS(UA_Server * server, UA_AggregationServerConfig *asc);
UA_Server *start_registry_module(UA_agent_list *asc);

#endif //REGISTRY_MODULE_INIT_H_
