/*

*/
#pragma once
#include <esp_log.h>
#include <string.h>
#include "nvs_flash.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../configs.h"




struct
{
  char sta_ssid[30];
  char sta_pasw[30];
  char ap_ssid[30];
  char ap_pasw[30];
} wifi_conf;

void test_sys_storege();
esp_err_t sys_storage_init();
esp_err_t sys_storage_get_wifi_aut_conf();
esp_err_t sys_storage_set_wifi_aut_conf();
void test3();