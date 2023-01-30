/*

*/
#include "uart.h"
#include "esp_intr_alloc.h"
#include <string.h>

// #include <hal/uart_ll.h>>

#define RD_BUF_SIZE 2048
static const char *TAG = "uart";

#define TETS_SEND_SIZE 20000

uint8_t *in ,*out;

static QueueHandle_t uart_qu;
TimerHandle_t tmr;

char *cmd[] ={"#EOF:12| ","#NEW:dert| ","#DEL:44","#SET:0"};

static void time_out( TimerHandle_t xTimer )
{
   static int cnt=0;
   uart_write_bytes(UART2_T,(char*)cmd[cnt],strlen(cmd[cnt]));
   cnt++;
   if(cnt>3)cnt=0; 
    // uart_write_bytes(UART2_T,(uint8_t*)out,TETS_SEND_SIZE);
}




TaskHandle_t uart_th;
static void uart_task(void *arg)
{
  uint32_t chank=0;
  uart_event_t event;
  size_t buffered_size;
  uint8_t* dtmp = (uint8_t*) malloc(2048);
  while (1)
  {
     //Waiting for UART event.
    if(xQueueReceive(uart_qu, (void * )&event, (portTickType)portMAX_DELAY)) {
        // bzero(dtmp, RD_BUF_SIZE);
        // ESP_LOGI(TAG, "uart[%d] event:", UART2);
        switch(event.type) {
            //Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                uart_read_bytes(UART2_T,(uint8_t*)&in[chank], event.size, portMAX_DELAY);
                chank+=event.size;
                ESP_LOGI(TAG, "[UART DATA]: %d chank :%d", event.size,chank);
                if(in[0]=='#')
                {
                  char cmd[30];
                  in[event.size] = '\0';
                  strcpy(cmd,(char*)&in[1]);
                  printf("====>%s\n",cmd);
                  // uart_flush(UART2_T);
                  
                  char *token = strtok(cmd,":|");
                  while (token)
                  {
                    int32_t ret = strtol(token, NULL, 10);
                    printf("ret :%d\n",ret);
                    ESP_LOGW(TAG,"CMD PAR:%s",token);
                    token = strtok(NULL,":|");
                  }
                  
                }
                chank=0;
                uart_flush_input(UART2_T);
                if(chank >= TETS_SEND_SIZE)
                {
                  printf("test eq in out buff!\n");
                  for(uint32_t i=0;i<TETS_SEND_SIZE;i++)
                  {
                    //j;j;j;j
                    if(in[i] != out[i])
                      printf("error index :%d\n",i);
                  }
                  printf("finish test!\n");
                }
                // printf("==>%s\n",dtmp);
                // ESP_LOGI(TAG, "[DATA EVT]:");
                // uart_write_bytes(UART2, (const char*) dtmp, event.size);
                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART2_T);
                xQueueReset(uart_qu);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART2_T);
                xQueueReset(uart_qu);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            //UART_PATTERN_DET
            //Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
        }
      }
    
  }
  vTaskDelete(NULL);
}

void test_send()
{
  in=(uint8_t*)malloc(TETS_SEND_SIZE);
  out=(uint8_t*)malloc(TETS_SEND_SIZE);
  assert(in);
  assert(out);
  memset(in,0,TETS_SEND_SIZE);
  for(uint32_t i=0;i < TETS_SEND_SIZE; i++)
  {
    out[i]= rand() % 255;
  }
}


void uart_init()
{
  test_send();
  uart_config_t uart_config = {
        // .baud_rate = 115200,
        .baud_rate = 10000000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
  esp_err_t err;
  err =  uart_driver_install(UART2_T, 2048, 2048, 2, &uart_qu, 0);

  ESP_LOGW(TAG,"uart_driver_install :%s",esp_err_to_name(err));
  err =  uart_param_config(UART2_T,&uart_config);
  ESP_LOGW(TAG,"uart_param_config :%s",esp_err_to_name(err));
  err =  uart_set_pin(UART2_T , 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  ESP_LOGW(TAG,"uart_set_pin :%s",esp_err_to_name(err));
  // uart_enable_rx_intr(UART2);
  uart_set_rx_full_threshold(UART2_T,40);
  //===========================================
  xTaskCreatePinnedToCore(uart_task,"uart",2048,
                          NULL,20,&uart_th,1);
  vTaskDelay(pdMS_TO_TICKS(1000));
  // uart_write_bytes(UART2_T,(uint8_t*)out,TETS_SEND_SIZE);
  // uart_write_bytes(UART2_T,(char*)"#123",4);
   tmr = xTimerCreate("tmr",pdMS_TO_TICKS(2000),pdTRUE,(void*)1,time_out); 
   xTimerStart(tmr,0);
  //===========================================
}