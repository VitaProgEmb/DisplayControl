/*

*/
#include "../configs.h"
#include "spiflash.h"

#include <esp_timer.h>

//=====================================
static const char *TAG ="spiffs";
//=====================================
esp_err_t spiflash_init()
{
  esp_err_t ret;
// #ifdef DEBUG_MODE ==  1
//   ESP_LOGW(TAG,"init spiffs");
// #endif
esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
  };

  ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) 
  {
    if (ret == ESP_FAIL)
    {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } 
    else if (ret == ESP_ERR_NOT_FOUND) 
    {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } 
    else 
    {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
  }
//=====================================
size_t total = 0, used = 0;
ret = esp_spiffs_info(conf.partition_label, &total, &used);
if (ret != ESP_OK) 
{
  ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
} 
else 
{
  ESP_LOGI(TAG, "Partition size: total: %d, used: %d free :%d", total, used,total - used);
}
/*
  FILE *fil = fopen("/spiffs/index.html","r");
  if(fil)
  {
    ESP_LOGW(TAG,"file open");  

  }else
    ESP_LOGW(TAG,"file not open"); 
  
 
  
  struct stat stt;
  stat("/spiffs/index.html",&stt);
    
  char *data = malloc(500);
  if(data)
  {

    size_t n =  fread(data,1,490,fil);
  
    data[n]=0;    
    ESP_LOGW(TAG,"data :%s  n:%d size :%d eof :%d",  data, n, 
              (uint32_t)stt.st_size,feof(fil));
  }
  */
  return ESP_OK;
}