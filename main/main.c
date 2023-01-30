/*

*/
#include <stdio.h>
#include <esp_log.h>
#include "../uart/uart.h"
#include "../wifi/wifi_ap_sta.h"
#include "../http/http.h"

//=================================
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "spiffs_config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include "../sys_config_storage/sys_storage.h"
#include "driver/gpio.h"
//=================================

static const char *TAG = "main";


void app_main(void)
{
   sys_main_init();
  
  //ESP_ERR_NVS_NOT_FOUND
  //  tmr =  xTimerCreate("timer",pdMS_TO_TICKS(60000),pdFALSE,NULL,cb);
  //  xTimerStart(tmr,0);
}
