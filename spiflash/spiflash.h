/*


*/
#pragma once

#include <esp_log.h>
#include <esp_err.h>
#include <esp_spiffs.h>
// #include <vfs/esp_vfs_fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"


esp_err_t spiflash_init();