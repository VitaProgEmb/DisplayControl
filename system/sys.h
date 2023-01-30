/*

*/
#pragma once
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/event_groups.h>
#include "../spiflash/spiflash.h"

/******************************************************************************/
EventGroupHandle_t EventGroup;
#define RECONECT (1<<0)
/******************************************************************************
 * 
 *                                команды CMD
 * ___________________________________________________________________________
 * добавить видео #NV:<"name" string >:<"fps" int>:<"duration"hh.mm.ss" int>:->
 * добавить картинку #NI:<"name" string >:<"duration"hh.mm.ss" int>:->
 * удалить файл #DEL:<"name" string >:->
 * 
******************************************************************************/


void sys_main_init();
