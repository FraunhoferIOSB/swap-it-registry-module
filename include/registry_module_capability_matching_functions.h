/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef OPEN62541_CAPABILITY_MATCHING_FUNCTIONS_H_
#define OPEN62541_CAPABILITY_MATCHING_FUNCTIONS_H_

#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

extern void(* matching_functions[])(UA_Variant resource_value, UA_Variant product_value, UA_Boolean *agent_is_suitable);

#endif //OPEN62541_CAPABILITY_MATCHING_FUNCTIONS_H_
