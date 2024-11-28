/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_CLEAR_H_
#define REGISTRY_MODULE_CLEAR_H_

#include "registry_module_internal.h"

UA_StatusCode clear_node_context(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle);
UA_StatusCode clear_registered_modules_config(UA_AggregateConfig *aggregateConfig);
UA_StatusCode clear_registry_module(UA_Server *server, UA_agent_list *asc);

#endif //REGISTRY_MODULE_CLEAR_H_
