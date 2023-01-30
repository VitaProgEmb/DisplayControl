#pragma once
//==============================//
//==============================//
#ifdef _cplusplus
extern "C"{
#endif
//==========================//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "../build/config/sdkconfig.h"
#include "../sys_config_storage/sys_storage.h"

//==========================//



//==========FOR STA=========//
/* Примеры используют конфигурацию Wi -Fi, которую вы можете установить с помощью проекта
  Меню конфигурации

   Если вы не хотите, просто измените приведенные ниже записи на строки с
   конфигурация, которую вы хотите - т.е. #define example_wifi_ssid "mywifissid"
*/
#if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
  #define ESP_STA_WIFI_SSID      CONFIG_STA_WIFI_SSID
  #define ESP_STA_WIFI_PASS      CONFIG_STA_WIFI_PASSWORD
  #define ESP_STA_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
#endif

#if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
  #define ESP_AP_WIFI_SSID      CONFIG_AP_WIFI_SSID
  #define ESP_AP_WIFI_PASS      CONFIG_AP_WIFI_PASSWORD
  #define ESP_AP_WIFI_CHANNEL   CONFIG_AP_WIFI_CHANNEL
  #define MAX_AP_STA_CONN       CONFIG_AP_MAX_STA_CONN
#endif


#if CONFIG_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif
//==========================//
//==========FOR AP=========//
/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

//==========================//

/* FreeRTOS event group to signal when we are connected*/


struct
{
  char sta_ssid[30];
  char sta_pasw[30];
}wifi_par;


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//==========================//
esp_netif_ip_info_t info;



// typedef enum WiFiMode{STA=0,AP,STA_AP};

void WiFIInit();
void WiFIStart();
void WiFIStop();
esp_err_t WiFIDeinit();
void WiFReconnect();
void APdesconnect();
void APconnect();
void STAdesconnect();
void STAconnect();
#if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
void wifi_sta_aut_set(const char *_ssid,const char *_pasword);
#endif
#if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
void wifi_ap_aut_set(const char *_ssid, const char *_pasword);
#endif
void wifi_set_aut_par_reconnect();

#ifdef _cplusplus
}
#endif