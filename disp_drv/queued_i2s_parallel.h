


#pragma once

#include <stdint.h>
#include "soc/i2s_struct.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "soc/gpio_periph.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "esp_private/periph_ctrl.h"
#include "soc/gpio_sig_map.h"
#include "soc/io_mux_reg.h"
#include "driver/gpio.h"
#include "rom/lldesc.h"
#include "esp_heap_caps.h"
#include "queued_i2s_parallel.h"
#include "esp_attr.h"
#include "pins.h"
//#define showDebugPulse

/*
The code here is a simple driver for the I2S parallel mode. It effectively has two buffers of configurable size. It configures the
I2S peripheral to stream out the contents of one buffer, while using a callback to let an external bit of code refill the other one.
Once the first buffer has finished, the I2S peripheral continues to the 2nd one and calls the callback to refill the 1st one.
*/

/*
Callback to refill the buffer that just has been streamed out. Buff is a pointer to the buffer to be filled,
arg is an user-configurable argument that was passed to the i2s_parallel_setup command.
NOTE: This callback is called in IRQ context!
*/
typedef void (*i2s_parallel_refill_buffer_cb_t)(void *buff, int len, void *arg);


extern uint16_t *bufferToFill;


//Amount of bits for I2S port. Note: anything but 16-bit is untested.
typedef enum {
	I2S_PARALLEL_BITS_8=8,
	I2S_PARALLEL_BITS_16=16,
	I2S_PARALLEL_BITS_32=32,
} i2s_parallel_cfg_bits_t;

//Configuration struct for i2s_parallel_setup.
typedef struct {
	int gpio_bus[24];							//GPIO-pins to connect the parallel bus to. Note: use a value of -1 for not-used pins
	int clkspeed_hz;							//Clockspeed, in hertz
	i2s_parallel_cfg_bits_t bits;				//Width of the parallel output bus
	int bufsz;									//Buffer size of each of the two buffers, in bytes
	i2s_parallel_refill_buffer_cb_t refill_cb;	//Callback to refill a buffer that's just been sent
	void *refill_cb_arg;						//Argument for the callback
} i2s_parallel_config_t;

//Setup an i2s parallel bus
void i2s_parallel_setup(i2s_dev_t *dev, const i2s_parallel_config_t *cfg);

//Start sending out data
void i2s_parallel_start(i2s_dev_t *dev);
