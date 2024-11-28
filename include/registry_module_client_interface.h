/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef REGISTRY_MODULE_CLIENT_INTERFACE_H_
#define REGISTRY_MODULE_CLIENT_INTERFACE_H_
#include <pthread.h>
#include <unistd.h>

#include <open62541/plugin/log_stdout.h>
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include "open62541/client_highlevel.h"
//#include "../deps/common.h"

#include "registry_module_node_iterator.h"

void *aggregate_client_connection(void *ptr);

#endif //REGISTRY_MODULE_CLIENT_INTERFACE_H_
