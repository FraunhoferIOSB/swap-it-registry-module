/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_CONFIG_PARSE_H_
#define REGISTRY_MODULE_CONFIG_PARSE_H_

#include "../deps/cj5.h"
#include "registry_module_synchronization.h"
#include "registry_module_register_agent.h"


UA_StatusCode
loadAggregationConfig(UA_ByteString json, UA_AggregationServerConfig *asc);
UA_StatusCode loadRegisterAgentConfig(UA_Server *server, UA_ByteString json, UA_AggregationServerConfig *asc);

#endif //REGISTRY_MODULE_CONFIG_PARSE_H_
