/*


*/
#pragma once
//=============================
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "../configs.h"
#include <esp_log.h>
//=============================
#define MOUNT_POINT "/sdcard"
#define SD_CARD_FORMAT false



void sdcard_init();