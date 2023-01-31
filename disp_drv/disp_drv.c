
/*
*/

#include "soc/gpio_sig_map.h"
#include "soc/i2s_struct.h"
#include "pins.h"
#include "soc/io_mux_reg.h"
#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "rom/gpio.h"
#include "esp_log.h"
#include "disp_drv.h"

volatile bool previousBufferFree = true;
lldesc_t *dma, *dma2;
uint16_t *test_patern;


// Todo: handle IS20? (this is hard coded for I2S1 only)
static void IRAM_ATTR i2s_isr(void* arg) {
   
    REG_WRITE(I2S_INT_CLR_REG(1), (REG_READ(I2S_INT_RAW_REG(1)) & 0xffffffc0) | 0x3f);
    // at this point, the previously active buffer is free, go ahead and write to it
    // previousBufferFree = true;
    
    // if(shiftCompleteCallback)
    //     shiftCompleteCallback();
}

static void gpio_setup_out(int gpio, int sig) {
    if (gpio == -1) return;
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction(gpio, GPIO_MODE_DEF_OUTPUT);
    gpio_matrix_out(gpio, sig, false, false);
    
}

static void dma_reset(i2s_dev_t *dev) {
    dev->lc_conf.in_rst=1; dev->lc_conf.in_rst=0;
    dev->lc_conf.out_rst=1; dev->lc_conf.out_rst=0;
}

static void fifo_reset(i2s_dev_t *dev) {
    dev->conf.rx_fifo_reset=1; dev->conf.rx_fifo_reset=0;
    dev->conf.tx_fifo_reset=1; dev->conf.tx_fifo_reset=0;
}


void set_bit(uint16_t *data,uint8_t bit)
{
    for(int i=0;i<8;i++)
    {
        data[i]=1<<bit;
    }
}

void disp_drv_init(const drv_conf_t *conf) {
   

    volatile i2s_dev_t *dev = &I2S1; 
    //Figure out which signal numbers to use for routing
    //printf("Setting up parallel I2S bus at I2S%d\n", i2snum(dev));
    //Route the signals
   
    volatile int signal_out = I2S1O_DATA_OUT8_IDX; 
    for (int x=0; x < 16; x++) {
        gpio_setup_out(conf->gpio_pins[x], signal_out + x);
    }
    //ToDo: Clk/WS may need inversion?
    if(CLK != -1)
    {
        gpio_set_direction(CLK, GPIO_MODE_DEF_OUTPUT);
        gpio_matrix_out(CLK, I2S1O_WS_OUT_IDX, true, false);
    }
    // gpio_setup_out(CLK, I2S1O_WS_OUT_IDX);
    
    // Power on dev
    periph_module_enable(PERIPH_I2S1_MODULE);
    //Initialize I2S dev
    dev->conf.rx_reset=1; dev->conf.rx_reset=0;
    dev->conf.tx_reset=1; dev->conf.tx_reset=0;
    dma_reset(dev);
    fifo_reset(dev);
    
    //Enable LCD mode
    dev->conf2.val=0;
    dev->conf2.lcd_en=1;
        
    // Enable "One datum will be written twice in LCD mode" - for some reason, if we don't do this in 8-bit mode, data is updated on half-clocks not clocks
    // if(cfg->bits == I2S_PARALLEL_BITS_8)
    dev->conf2.lcd_tx_wrx2_en=1;   //FIXME !!!
    // dev->conf2.lcd_tx_sdx2_en=0;   //FIXME !!! not use

    dev->sample_rate_conf.val=0;
    dev->sample_rate_conf.rx_bits_mod=16;
    dev->sample_rate_conf.tx_bits_mod=16;
    dev->sample_rate_conf.rx_bck_div_num=1; //FIXME !!!!! 4
    dev->sample_rate_conf.tx_bck_div_num=1; //FIXME !!!!! 4
    //ToDo: Unsure about what this does...

    // because conf2.lcd_tx_wrx2_en is set for 8-bit mode, the clock speed is doubled, drop it in half here
    // if(cfg->bits == I2S_PARALLEL_BITS_8)
    //     dev->sample_rate_conf.tx_bck_div_num=2;
    // else
    //FIXME !!!
    
    dev->clkm_conf.val=0;
    dev->clkm_conf.clka_en=1;
    dev->clkm_conf.clkm_div_a=63;
    dev->clkm_conf.clkm_div_b=63;
    //We ignore the possibility for fractional division here, clkspeed_hz must round up for a fractional clock speed, must result in >= 2
    dev->clkm_conf.clkm_div_num=80000000L/(conf->clk_freq + 1);

	
    dev->fifo_conf.val=0;
    dev->fifo_conf.rx_fifo_mod_force_en=1;
    dev->fifo_conf.tx_fifo_mod_force_en=1;


    dev->fifo_conf.tx_fifo_mod=1;
    // dev->fifo_conf.rx_fifo_mod=1; 
    dev->fifo_conf.rx_data_num=32; //Thresholds. 
    dev->fifo_conf.tx_data_num=32;//FIXME //32
    dev->fifo_conf.dscr_en=1;
    
    dev->conf1.val=0;
    dev->conf1.tx_stop_en=0;
    dev->conf1.tx_pcm_bypass=1;
    
    dev->conf_chan.val=0;
    dev->conf_chan.tx_chan_mod=1;
    dev->conf_chan.rx_chan_mod=1;
    
    //Invert ws to be active-low... ToDo: make this configurable
    //dev->conf.tx_right_first=1;
    dev->conf.tx_right_first=0;
    //dev->conf.rx_right_first=1;
    dev->conf.rx_right_first=0;
    
    dev->timing.val=0;
    
    //++++++++++++TEST+++++++++++++++++++++++++++++++++++++++++
    

    dma = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t),MALLOC_CAP_DMA);
    dma2 = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t),MALLOC_CAP_DMA);
    // uint16_t *test_patern = (uint16_t*)heap_caps_malloc(sizeof(uint16_t)*16,MALLOC_CAP_DMA); 
    void *buffr =heap_caps_malloc(32, MALLOC_CAP_DMA);
    memset(buffr, 0, 32);

    // test_patern = (uint16_t*)malloc(sizeof(uint16_t)*16);
    
   
    //:0x3ffb670c
   
    

    //ch0 0
    //ch1 1
    //ch2 2
    //ch3 3
    //ch4 4
    //ch5 5
    //ch6 6
    //ch6 7

    
    dma->size = 16*2;
    dma->length = 16*2;
    dma->buf =(uint8_t*)buffr;
    dma->eof = 1;
    dma->sosf = 0;
    dma->owner = 1;
    dma->offset = 0;
    dma->qe.stqe_next = dma;

    // dma2->size = 32;
    // dma2->length = 32;
    // dma2->buf =(uint8_t*)test_patern;
    // dma2->eof = 0;
    // dma2->sosf = 0;
    // dma2->owner = 1;
    // dma2->offset = 0;

    // dma2->qe.stqe_next = dma;

    
    
    //++++++++++++TEST+++++++++++++++++++++++++++++++++++++++++



    //Allocate DMA descriptors
    // i2s_state[i2snum(dev)]=malloc(sizeof(i2s_parallel_state_t));
    // assert(i2s_state[i2snum(dev)] != NULL);
    // i2s_parallel_state_t *st=i2s_state[i2snum(dev)];

    // st->desccount_a = cfg->desccount_a;
    // st->desccount_b = cfg->desccount_b;
    // st->dmadesc_a = cfg->lldesc_a;
    // st->dmadesc_b = cfg->lldesc_b;

    //Reset FIFO/DMA -> needed? Doesn't dma_reset/fifo_reset do this?
    dev->lc_conf.in_rst=1; dev->lc_conf.out_rst=1; dev->lc_conf.ahbm_rst=1; dev->lc_conf.ahbm_fifo_rst=1;
    dev->lc_conf.in_rst=0; dev->lc_conf.out_rst=0; dev->lc_conf.ahbm_rst=0; dev->lc_conf.ahbm_fifo_rst=0;
    dev->conf.tx_reset=1; dev->conf.tx_fifo_reset=1; dev->conf.rx_fifo_reset=1;
    dev->conf.tx_reset=0; dev->conf.tx_fifo_reset=0; dev->conf.rx_fifo_reset=0;
    
    // setup I2S Interrupt
    SET_PERI_REG_BITS(I2S_INT_ENA_REG(1), I2S_OUT_EOF_INT_ENA_V, 1, I2S_OUT_EOF_INT_ENA_S);
    // allocate a level 1 intterupt: lowest priority, as ISR isn't urgent and may take a long time to complete
    esp_intr_alloc(ETS_I2S1_INTR_SOURCE, (int)(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1), i2s_isr, NULL, NULL);


    dev->out_link.stop = 1;
    dev->out_link.start = 0;
    dev->conf.tx_start = 0;
    //Start dma on front buffer (buffer a)
    //FIXME
    dev->lc_conf.val=I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN | I2S_OUT_DATA_BURST_EN;
    // dev->lc_conf.val = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;
    /* FIXME dma init pointer and start!
    dev->out_link.addr=((uint32_t)(&st->dmadesc_a[0]));
    */
    dev->out_link.addr=(uint32_t)dma;
    dev->out_link.start=1;
    dev->conf.tx_start=1;
   
}