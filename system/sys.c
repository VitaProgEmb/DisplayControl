/*

*/
#include "sys.h"
#include <freertos/timers.h>
#include <stdio.h>
#include "../wifi/wifi_ap_sta.h"
#include "../http/http.h"

#include "../sys_config_storage/sys_storage.h"
#include "../spiflash/spiflash.h"
#include "../wifi/wifi_ap_sta.h"
#include "../http/http.h"

static const char *TAG = "system";
xTimerHandle tmr;


void reconect_wifi()
{
  ESP_LOGE(TAG,"reconnect wifi");
  if(sys_storage_set_wifi_aut_conf() == ESP_OK)
  {
    ESP_LOGE(TAG,"wifi----->");
    ESP_ERROR_CHECK(esp_wifi_stop());
    wifi_ap_aut_set(wifi_conf.ap_ssid,wifi_conf.ap_pasw);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGE(TAG,"save chenge and reconect");
  }
}



TaskHandle_t sys_tsk;
static void sys_task(void *arg)
{

  sys_storage_init();
  spiflash_init();
  WiFIInit();
  ESP_LOGE(TAG,"-----> system task :%d",cpu_hal_get_core_id());
  httpd_handle_t server =  start_webserver();
  while (1)
  {
    EventBits_t bit = xEventGroupWaitBits(EventGroup,RECONECT,pdTRUE,pdFALSE,0);
                                              
    if((bit & RECONECT) != 0)
      reconect_wifi();
  }
  vTaskDelete(NULL);
}

void sys_main_init()
{
  EventGroup = xEventGroupCreate();
  if( EventGroup == NULL )
  {
    ESP_LOGW(TAG,"event group no create");
  }
  else
  {
    ESP_LOGW(TAG,"event group create");
  }
  xTaskCreatePinnedToCore(sys_task,"sys task",6000,
                          NULL,1,&sys_tsk,1);
  // tmr = xTimerCreate("timer",pdMS_TO_TICKS(3000),pdTRUE,NULL,time_out);
  // xTimerStart(tmr,0);
}