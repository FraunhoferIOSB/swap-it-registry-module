/*
* Licensed under the MIT License.
 * For details on the licensing terms, see the LICENSE file.
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 (c) Fraunhofer IOSB (Author: Florian Düwel)
 *
 */

#include "../../tests/include/start_swap_server.h"

UA_Boolean running = true;
static void stopHandler(int sign) {
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
  running = false;
}

int main(void) {

  /*run a swap server that registers itself in the registry module*/
  test_swap_server_config aggregate_conf_1;
  aggregate_conf_1.register_server = true;
  aggregate_conf_1.running = &running;
  aggregate_conf_1.conf = "../../tests/configs/warehouse.json5";
  start_swap_server_thread(&aggregate_conf_1);

  test_swap_server_config aggregate_conf_2;
  aggregate_conf_2.register_server = true;
  aggregate_conf_2.running = &running;
  aggregate_conf_2.conf = "../../tests/configs/warehouse_1.json5";
  start_swap_server_thread(&aggregate_conf_2);

  test_swap_server_config aggregate_conf_3;
  aggregate_conf_3.register_server = true;
  aggregate_conf_3.running = &running;
  aggregate_conf_3.conf = "../../tests/configs/warehouse_2.json5";
  start_swap_server_thread(&aggregate_conf_3);

  test_swap_server_config aggregate_conf_4;
  aggregate_conf_4.register_server = true;
  aggregate_conf_4.running = &running;
  aggregate_conf_4.conf = "../../tests/configs/warehouse_3.json5";
  start_swap_server_thread(&aggregate_conf_4);

  test_swap_server_config aggregate_conf_5;
  aggregate_conf_5.register_server = true;
  aggregate_conf_5.running = &running;
  aggregate_conf_5.conf = "../../tests/configs/warehouse_4.json5";
  start_swap_server_thread(&aggregate_conf_5);

  while(running)
    sleep(1);

  return EXIT_SUCCESS;
}

