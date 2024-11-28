/*
 * Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */
#ifndef START_REGISTRY_MODULE_H
#define START_REGISTRY_MODULE_H

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>

#include "../../include/registry_module_init.h"
#include "../../include/registry_module_clear.h"

#include <pthread.h>


typedef struct{
  UA_Boolean *running;
  UA_Server *server;
  pthread_t threadId;
}test_registry_module_config;


void start_registry_module_server_thread(test_registry_module_config *aggr_config);

#endif //START_REGISTRY_MODULE_H
