#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
// #include "driver/periph_ctrl.h"
#include "driver/periph_ctrl.h"
#include "soc/io_mux_reg.h"
#include "rom/lldesc.h"






typedef struct {
    void *memory;
    size_t size;
} i2s_parallel_buffer_desc_t;


typedef struct
{
  int gpio_pins[16];
  uint8_t gpio_clk;
  uint32_t clk_freq;   
}drv_conf_t;


typedef struct {
    int gpio_bus[24];
    int gpio_clk;
    int clkspeed_hz;

    i2s_parallel_buffer_desc_t *bufa;
    i2s_parallel_buffer_desc_t *bufb; // only used with double buffering
    int desccount_a;
    int desccount_b;      // only used with double buffering
    lldesc_t * lldesc_a;
    lldesc_t * lldesc_b;  // only used with double buffering
} i2s_parallel_config_t;



typedef void (*callback)(void);
void setShiftCompleteCallback(callback f);
void disp_drv_init(const drv_conf_t *conf);