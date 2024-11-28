/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef OPEN62541_CALLBACK_FUNCTIONS_H_
#define OPEN62541_CALLBACK_FUNCTIONS_H_


#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include "namespace_common_generated.h"
#include "types_common_generated.h"
#include "common_nodeids.h"
#include "registry_module_register_agent.h"
#include "registry_module_capability_matching_functions.h"


UA_StatusCode add_agent_to_registry(UA_Server *server,
                                           const UA_NodeId *sessionId, void *sessionHandle,
                                           const UA_NodeId *methodId, void *methodContext,
                                           const UA_NodeId *objectId, void *objectContext,
                                           size_t inputSize, const UA_Variant *input,
                                           size_t outputSize, UA_Variant *output);

UA_StatusCode remove_agent_from_registry(UA_Server *server,
                                                const UA_NodeId *sessionId, void *sessionHandle,
                                                const UA_NodeId *methodId, void *methodContext,
                                                const UA_NodeId *objectId, void *objectContext,
                                                size_t inputSize, const UA_Variant *input,
                                                size_t outputSize, UA_Variant *output);


UA_StatusCode get_capability_values(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);

UA_StatusCode filter_agents(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionHandle,
                                   const UA_NodeId *methodId, void *methodContext,
                                   const UA_NodeId *objectId, void *objectContext,
                                   size_t inputSize, const UA_Variant *input,
                                   size_t outputSize, UA_Variant *output);


#endif //OPEN62541_CALLBACK_FUNCTIONS_H_
