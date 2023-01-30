#include "wifi_ap_sta.h"
#include "../sys_config_storage/sys_storage.h"
#include <hal/cpu_hal.h>

static char *TAG = "WiFi";
static EventGroupHandle_t s_wifi_event_group;

#if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
  static int s_retry_num = 0;
#endif
//=====================STA EVENT HANDLERS=======================//
#if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG,"STA DESCONNECT");
        if (s_retry_num < ESP_STA_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP try:%d",s_retry_num);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            s_retry_num=0;
            ESP_LOGW(TAG,"new retry connect to AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGW(TAG,"STA CONNECT");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGE(TAG,"-----> core wifi :%d",cpu_hal_get_core_id());
        s_retry_num = 0;
        // if(server == NULL)server = start_webserver();
        
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
#endif
//==============================================================//
//======================AP EVENT HANDLERS=======================//
#if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGW(TAG,"AP CONNECT");
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
         
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGW(TAG,"AP DESCONNECT");
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        
    } else if(event_id == IP_EVENT_AP_STAIPASSIGNED){
      ESP_LOGW(TAG,"AP IP ASSIGNED");
    }
}
#endif
//==============================================================//
void wifi_set_aut_par_reconnect()
{
  esp_wifi_disconnect();
  wifi_ap_aut_set(wifi_conf.ap_ssid, wifi_conf.ap_pasw);
  esp_wifi_connect();
}
//==============================================================//
//==============================================================//
esp_err_t WiFIDeinit()
{
  wifi_config_t sta_config = {
        .sta = {
            // .ssid = ESP_STA_WIFI_SSID ,
            // .password = ESP_STA_WIFI_PASS,
            .ssid = "" ,
            .password = "",
            /* Настройка пароля подразумевает, что станция будет подключаться ко всем режимам безопасности, включая WEP/WPA.
             * Однако эти режимы устарели и не рекомендуются использовать.В случае вашей точки доступа
             * не поддерживает WPA2, этот режим можно включить, комментируя ниже строки */
	          .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

      wifi_config_t ap_config = {
        .ap = {
            .ssid = "PIZDA",
            .ssid_len = strlen("PIZDA"),
            .channel = ESP_AP_WIFI_CHANNEL,
            .password = "12345678",
            .max_connection = MAX_AP_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
            
        },
    };
    if (strlen(ESP_AP_WIFI_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
  strcpy((char*)sta_config.sta.ssid,wifi_par.sta_ssid);
  strcpy((char*)sta_config.sta.password,wifi_par.sta_pasw);

  esp_err_t err =  esp_wifi_disconnect();
  ESP_LOGW(TAG,"wifi_disconnect :%s",esp_err_to_name(err));
  // vTaskDelay(100);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
  // vTaskDelay(100);
  ESP_ERROR_CHECK(esp_wifi_connect());
 
   return 0;
}

#if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
void wifi_sta_aut_set(const char *_ssid,const char *_pasword)
{
  wifi_config_t sta_config = {
        .sta = {
	        .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
    strcpy((char*)sta_config.sta.ssid, _ssid);
    strcpy((char*)sta_config.sta.password, _pasword);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
}
#endif

#if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
void wifi_ap_aut_set(const char *_ssid, const char *_pasword)
{
  wifi_config_t ap_config = {
        .ap = {
            .ssid_len = strlen(_ssid),
            .channel = ESP_AP_WIFI_CHANNEL,
            .max_connection = MAX_AP_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(ESP_AP_WIFI_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
  strcpy((char*)ap_config.ap.ssid, _ssid);
  strcpy((char*)ap_config.ap.password, _pasword);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
}
#endif



void WiFIInit()
{
   printf("test wifi==>\n");
   sys_storage_get_wifi_aut_conf();
   test_sys_storege();
   ESP_LOGE(TAG,"-----> core wifi init :%d",cpu_hal_get_core_id());
   #if CONFIG_WIFI_CONNECT_AP
    #define mode "AP"
   #elif CONFIG_WIFI_CONNECT_STA
    #define mode "STA"
   #elif CONFIG_WIFI_CONNECT_APSTA
    #define mode "AP+STA"
   #endif
   ESP_LOGI(TAG,"wifi init mode:%s",mode);

   //======================================//
   //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    //======================================//
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    
    #if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
    esp_netif_t *sta =  esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(sta,"perduck");
    #endif

    #if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
    esp_netif_create_default_wifi_ap();
   
    #endif
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    #if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
  #endif

  #if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
  #endif                                                      
  
  //======================================//
    //======================================//
  #if CONFIG_WIFI_CONNECT_AP
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
  #elif CONFIG_WIFI_CONNECT_STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
  #elif CONFIG_WIFI_CONNECT_APSTA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  #endif
  //======================================//
  #if CONFIG_WIFI_CONNECT_STA || CONFIG_WIFI_CONNECT_APSTA
  wifi_sta_aut_set(wifi_conf.sta_ssid, wifi_conf.sta_pasw);
  #endif
  #if CONFIG_WIFI_CONNECT_AP || CONFIG_WIFI_CONNECT_APSTA
  wifi_ap_aut_set(wifi_conf.ap_ssid, wifi_conf.ap_pasw);
  #endif
  //======================================//
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init %s finished.",mode);
    /* Ожидание, пока не будет установлено соединение (wifi_connected_bit) или подключение к максимуму
     * Количество повторных переводов (WiFi_fail_bit).Биты установлены event_handler () (см. Выше) */   
}