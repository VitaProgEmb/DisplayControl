/*

*/
#include "sys_storage.h"

static const char *TAG = "sys storage";

static const char *wifi_aut_key[] = {"sta_ssid", "sta_pasw", "ap_ssid", "ap_pasw"};
const char *wifi_conf_aut[] = {DEFAULT_STA_SSID, DEFAULT_STA_PASW, DEFAULT_AP_SSID, DEFAULT_AP_PASW};

esp_err_t sys_storage_init()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ret = nvs_flash_erase();
    ret = nvs_flash_init();
  }
  ESP_LOGI(TAG, "Initializing SPIFFS");
  return ret;
}

void test_sys_storege()
{
  esp_err_t err;
  nvs_handle_t nvs;
  char test[30] = "";
  size_t len = 30;
  nvs_open("wifi_conf", NVS_READONLY, &nvs);
  err = nvs_get_str(nvs, "sta_ssid", test, &len);
  ESP_LOGW(TAG, "test read sta ssid :%s :%s", test, esp_err_to_name(err));
  len = 30;
  err = nvs_get_str(nvs, "sta_pasw", test, &len);
  ESP_LOGW(TAG, "test read sta pasw :%s :%s", test, esp_err_to_name(err));
  len = 30;
  err = nvs_get_str(nvs, "ap_ssid", test, &len);
  ESP_LOGW(TAG, "test read ap ssid :%s :%s", test, esp_err_to_name(err));
  len = 30;
  err = nvs_get_str(nvs, "ap_pasw", test, &len);
  ESP_LOGW(TAG, "test read ap pasw :%s :%s", test, esp_err_to_name(err));
  nvs_close(nvs);
  len = 0;
  ESP_LOGW(TAG, "test struct sta_ssid :%s", wifi_conf.sta_ssid);
  ESP_LOGW(TAG, "test struct sta_pasw :%s", wifi_conf.sta_pasw);
  ESP_LOGW(TAG, "test struct ap_ssid :%s", wifi_conf.ap_ssid);
  ESP_LOGW(TAG, "test struct ap_pasw :%s", wifi_conf.ap_pasw);
  nvs_close(nvs);
}

void test3()
{
  char *test = wifi_conf.sta_ssid;
  for (int i = 0; i < 4; i++)
  {
    printf("data test :%s ,key:%s\n", &test[i * 30], wifi_aut_key[i]);
  }
}


esp_err_t sys_storage_set_wifi_aut_conf()
{
  nvs_handle_t nvs;
  char *w_conf = wifi_conf.sta_ssid;
  if (nvs_open("wifi_conf", NVS_READWRITE, &nvs) == ESP_OK)
  {
    for(int i=0;i<4;i++)
    {
      if(nvs_set_str(nvs,wifi_aut_key[i],&w_conf[i * 30]) != ESP_OK)
        return ESP_FAIL;
    }
  }
 return ESP_OK;
}


esp_err_t sys_storage_get_wifi_aut_conf()
{
  nvs_handle_t nvs;
  size_t len;
  char *w_conf = wifi_conf.sta_ssid;
  if (nvs_open("wifi_conf", NVS_READWRITE, &nvs) == ESP_OK)
  {
    for (int i = 0; i < 4; i++)
    {
      len = 30;
      if (nvs_get_str(nvs, wifi_aut_key[i], &w_conf[i * 30], &len) == ESP_ERR_NVS_NOT_FOUND)
      {
        if (nvs_set_str(nvs, wifi_aut_key[i], wifi_conf_aut[i]) != ESP_OK)
          return ESP_FAIL;
        strcpy(&w_conf[i * 30], wifi_conf_aut[i]);
      }
    }
  }
  else
    return ESP_FAIL;
  if (nvs_commit(nvs) != ESP_OK)
    return ESP_FAIL;
  return ESP_OK;
}

/*
esp_err_t sys_storage_get_wifi_aut_conf()
{
  nvs_handle_t nvs;
  size_t len=0;
  esp_err_t err;
  // nvs_flash_erase();
  err = nvs_open("wifi_conf",NVS_READWRITE,&nvs);
  if(err != ESP_OK)
    return err;
  err = nvs_get_str(nvs,"sta_ssid",wifi_conf.sta_ssid,&len);
  if(err == ESP_ERR_NVS_NOT_FOUND){
    err = nvs_set_str(nvs, "sta_ssid", DEFAULT_STA_SSID);
    ESP_LOGE(TAG,"write default sta ssid :%s >>>>>%s",esp_err_to_name(err),DEFAULT_STA_SSID);
    strcpy(wifi_conf.sta_ssid, DEFAULT_STA_SSID);
  }

  //======================================================//
  err = nvs_get_str(nvs,"sta_pasw",wifi_conf.sta_pasw,&len);
  if(err == ESP_ERR_NVS_NOT_FOUND){
    err = nvs_set_str(nvs, "sta_pasw", DEFAULT_STA_PSW);
    ESP_LOGE(TAG,"write default sta pasw :%s >>>>>%s",esp_err_to_name(err),DEFAULT_STA_PSW);
    strcpy(wifi_conf.sta_pasw, DEFAULT_STA_PSW);
  }
  //======================================================//
  err = nvs_get_str(nvs,"ap_ssid",wifi_conf.ap_ssid,&len);
  if(err == ESP_ERR_NVS_NOT_FOUND){
    err = nvs_set_str(nvs, "ap_ssid", DEFAULT_AP_SSID);
    ESP_LOGE(TAG,"write default ap ssid :%s >>>>>%s",esp_err_to_name(err),DEFAULT_AP_SSID);
    strcpy(wifi_conf.ap_ssid, DEFAULT_AP_SSID);
  }
  //======================================================//
  err = nvs_get_str(nvs,"ap_pasw",wifi_conf.ap_pasw,&len);
  if(err == ESP_ERR_NVS_NOT_FOUND){
    err = nvs_set_str(nvs, "ap_pasw", DEFAULT_AP_PASW);
    ESP_LOGE(TAG,"write default ap pasw:%s >>>>>%s",esp_err_to_name(err), DEFAULT_AP_PASW);
    strcpy(wifi_conf.ap_pasw, DEFAULT_AP_PASW);
  }
  nvs_commit(nvs);
  printf("====================================\n");
  return ESP_OK;
}
*/

/*
  printf("end app_main\n");
  esp_err_t nvs_err =  nvs_open("wifi_seting",NVS_READWRITE,&nvs);
  ESP_LOGW(TAG,"nvs_open :%s",esp_err_to_name(nvs_err));
  char val[30]="";
  size_t len;
  nvs_err =  nvs_get_str(nvs,"sta_ssid",val,&len);
  if(nvs_err == ESP_ERR_NVS_NOT_FOUND)
  {
    nvs_err =  nvs_set_str(nvs,"sta_ssid","TEST STA");
    ESP_LOGW(TAG,"nvs_set_str :%s",esp_err_to_name(nvs_err));
  }
  ESP_LOGW(TAG,"nvs_get_str :%s :%s :%d",esp_err_to_name(nvs_err),val,len);

  nvs_close(nvs);
*/