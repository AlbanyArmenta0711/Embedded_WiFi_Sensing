/*
 * Informer app main source file
 * Created on: 02/07/2024
 * by: Jesus A. Armenta-Garcia
 */
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

#include "../common/utils.h"
#include "informer.h"
#include "../components/sd_component.h"
#include "../components/uart_component.h"

static const char * TAG = "Informer app"; 

char filename[100];
char * start_csi_string = "CSI_DATA,"; 

user_csi_configuration_t * informer_config; 

//Queue to handle event messages between tasks
static QueueHandle_t informer_app_queue; 

//File descriptor to SD file in case SD CSI saving is selected
static FILE * csi_fd = NULL; 

static esp_err_t sd_initialized = ESP_FAIL;
static esp_err_t uart_initialized = ESP_OK; 

/*
 * Function to obtain the CSI string message to inform according to user configuration in ASCII
 * @param msg: string where the message will be stored 
 */
static void get_csi_string_message(char * msg, wifi_csi_info_t info) {
    char aux[20] = "";
    strcat(msg, start_csi_string);
    if (informer_config->inform_mac_subscribed) {
        sprintf(aux, ""MACSTR",", MAC2STR(info.mac) );
        strcat(msg, aux);
    }
    if (informer_config->inform_rssi) {
        sprintf(aux, "%d,",info.rx_ctrl.rssi);
        strcat(msg, aux);
    }
    if (informer_config->inform_channel_bw) {
        sprintf(aux, "%d,", info.rx_ctrl.cwb);
        strcat(msg, aux);
    }
    if (informer_config->inform_noise_floor) {
        sprintf(aux, "%d,", info.rx_ctrl.noise_floor);
        strcat(msg, aux);
    }
    if (informer_config->inform_timestamp) {
        sprintf(aux, "%d,", info.rx_ctrl.timestamp);
        strcat(msg, aux);
    }
    if (informer_config->inform_ant_num) {
        sprintf(aux, "%d,", info.rx_ctrl.ant);
        strcat(msg, aux);
    }
    if (informer_config->inform_csi_len) {
        sprintf(aux, "%d,", info.len);
        strcat(msg, aux); 
    }
    if (informer_config->inform_csi_data) {
        sprintf(aux, "[%d", info.buf[0]);
        strcat(msg, aux); 
        for (int i = 1; i < info.len; i++) {
            sprintf(aux, ",%d", info.buf[i]);
            strcat(msg, aux); 
        }
        strcat(msg, "]\n");
    }
}


/*
 * Function to handle CSI info by sending it to console, COM port or SD
 * @param info: Wi-Fi CSI info received by the callback
 */
static void send_information(wifi_csi_info_t info){
    
    char csi_string[1024] = "";
    switch (informer_config->informer_mode) {

        case INFORMER_CONSOLE_MODE:
            //ASCII as default
            //Will send data to STDOUT (USB interface) as ASCII
            if (informer_config->data_mode == DATA_ASCII) {
                get_csi_string_message(csi_string, info);
                ets_printf("%s", csi_string); 
            } else {
                //Will send data to STDOUT (USB interface) as ASCII
                write(fileno(stdout), start_csi_string, strlen(start_csi_string));
                fflush(stdout);
                if (informer_config->inform_mac_subscribed) {
                    write(fileno(stdout), info.mac, sizeof(uint8_t) * 6);
                    fflush(stdout);
                }
                if (informer_config->inform_rssi) {
                    int8_t rssi = info.rx_ctrl.rssi;
                    write(fileno(stdout), &rssi, sizeof(rssi));
                    fflush(stdout);
                }
                if (informer_config->inform_channel_bw) {
                    uint8_t cwb = info.rx_ctrl.cwb;
                    write(fileno(stdout), &cwb, sizeof(cwb));
                    fflush(stdout);
                }
                if (informer_config->inform_noise_floor) {
                    int8_t noise_floor = info.rx_ctrl.noise_floor;
                    write(fileno(stdout), &noise_floor, sizeof(noise_floor));
                    fflush(stdout);
                }
                if (informer_config->inform_timestamp) {
                    uint32_t timestamp = info.rx_ctrl.timestamp;
                    write(fileno(stdout), &timestamp, sizeof(timestamp));
                    fflush(stdout);
                }
                if (informer_config->inform_ant_num) {
                    uint8_t ant_num = info.rx_ctrl.ant;
                    write(fileno(stdout), &ant_num, sizeof(ant_num));
                    fflush(stdout);
                }
                if (informer_config->inform_csi_len) {
                    write(fileno(stdout), &info.len, sizeof(info.len));
                    fflush(stdout);
                }
                if (informer_config->inform_csi_data) {
                    write(fileno(stdout), info.buf, info.len);
                    fflush(stdout);
                }
            }
            break; 

        case INFORMER_SEND_SD:
            if (sd_initialized == ESP_OK) {
                //Will save data as ASCII text
                if (informer_config->data_mode == DATA_ASCII) {
                    csi_fd = open_file(filename);
                    if (csi_fd != NULL) {
                        get_csi_string_message(csi_string, info); 
                        write_csi_into_sd(csi_fd, csi_string);
                        close_file(csi_fd);
                    }
                } else {
                    //Will save data as binary
                    csi_fd = open_binary_file(filename); 
                    if (csi_fd != NULL){
                        //Every new CSI measurement will start with "CSI_DATA,"
                        write_bin_data(csi_fd, start_csi_string, strlen(start_csi_string)); 
                        if (informer_config->inform_mac_subscribed) {
                            write_bin_data(csi_fd, info.mac, sizeof(uint8_t) * 6);
                        }
                        if (informer_config->inform_rssi) {
                            int8_t rssi = info.rx_ctrl.rssi;
                            write_bin_data(csi_fd, &rssi, sizeof(rssi));
                        }
                        if (informer_config->inform_channel_bw) {
                            uint8_t cwb = info.rx_ctrl.cwb;
                            write_bin_data(csi_fd, &cwb, sizeof(cwb));
                        }
                        if (informer_config->inform_noise_floor) {
                            int8_t noise_floor = info.rx_ctrl.noise_floor;
                            write_bin_data(csi_fd, &noise_floor, sizeof(noise_floor));
                        }
                        if (informer_config->inform_timestamp) {
                            uint32_t timestamp = info.rx_ctrl.timestamp;
                            write_bin_data(csi_fd, &timestamp, sizeof(timestamp));
                        }
                        if (informer_config->inform_ant_num) {
                            uint8_t ant_num = info.rx_ctrl.ant;
                            write_bin_data(csi_fd, &ant_num, sizeof(ant_num));
                        }
                        if (informer_config->inform_csi_len) {
                            write_bin_data(csi_fd, &info.len, sizeof(info.len));
                        }
                        if (informer_config->inform_csi_data) {
                            write_bin_data(csi_fd, info.buf, info.len);
                        }
                        
                        close_file(csi_fd); 
                    }
                }
            }
            break; 

        case INFORMER_SEND_SERIAL_COMM:
            if (uart_initialized == ESP_OK) {
                if (informer_config->data_mode == DATA_ASCII) {
                    get_csi_string_message(csi_string, info); 
                    if(uart_write_bytes(DEFAULT_UART_PORT, (const char *) csi_string, strlen(csi_string)) == -1)
                        ESP_LOGW(TAG, "error sending  msg");
                } else {
                    //Every new CSI measurement will start with "CSI_DATA,"
                    if(uart_write_bytes(DEFAULT_UART_PORT, (const char *) start_csi_string, strlen(start_csi_string)) == -1)
                        ESP_LOGW(TAG, "error writing start msg");
                    if (informer_config->inform_mac_subscribed) {
                        uart_write_bytes(DEFAULT_UART_PORT, info.mac, sizeof(uint8_t) * 6); 
                    }
                    if (informer_config->inform_rssi) {
                        int8_t rssi = info.rx_ctrl.rssi;
                        uart_write_bytes(DEFAULT_UART_PORT, &rssi, sizeof(rssi));
                    }
                    if (informer_config->inform_channel_bw) {
                        uint8_t cwb = info.rx_ctrl.cwb;
                        uart_write_bytes(DEFAULT_UART_PORT, &cwb, sizeof(cwb));
                    }
                    if (informer_config->inform_noise_floor) {
                        int8_t noise_floor = info.rx_ctrl.noise_floor;
                        uart_write_bytes(DEFAULT_UART_PORT, &noise_floor, sizeof(noise_floor));
                    }
                    if (informer_config->inform_timestamp) {
                        uint32_t timestamp = info.rx_ctrl.timestamp;
                        uart_write_bytes(DEFAULT_UART_PORT, &timestamp, sizeof(timestamp));
                    }
                    if (informer_config->inform_ant_num) {
                        uint8_t ant_num = info.rx_ctrl.ant;
                        uart_write_bytes(DEFAULT_UART_PORT, &ant_num, sizeof(ant_num));
                    }
                    if (informer_config->inform_csi_len) {
                        uart_write_bytes(DEFAULT_UART_PORT, &info.len, sizeof(info.len));
                    }
                    if (informer_config->inform_csi_data) {
                        if(uart_write_bytes(DEFAULT_UART_PORT, info.buf, info.len) == -1)
                            ESP_LOGW(TAG, "error writing csi buff");
                    }
                }
            }
            break;  
        
        case INFORMER_SEND_SD_SC:
            get_csi_string_message(csi_string, info); 
            if (uart_initialized == ESP_OK) {
                uart_write_bytes(DEFAULT_UART_PORT, (const char *) csi_string, strlen(csi_string));
            } else {
                ESP_LOGE(TAG, "Error writing in Serial");
            }
            if (sd_initialized == ESP_OK) {
                csi_fd = open_file(filename);
                if (csi_fd != NULL) {
                    get_csi_string_message(csi_string, info); 
                    write_csi_into_sd(csi_fd, csi_string);
                    close_file(csi_fd); 
                }
            } else {
                ESP_LOGE(TAG, "Error writing to SD");
            }
            break; 

        default: 
            ESP_LOGE(TAG, "No informer mode defined for %d", informer_config->op_mode);
            break; 
    }
    
    //free(info.buf); //Free memory that was dynamically allocated 
}

BaseType_t informer_app_send_message(informer_app_message_t msg){
    return xQueueSend(informer_app_queue, &msg, portMAX_DELAY); 
}

static void informer_task(void * parameters){
    informer_app_message_t msg; 
    while(1){
        if(xQueueReceive(informer_app_queue, &msg, portMAX_DELAY)){
            switch(msg.msgID){
                case MSG_INFORMER_APP_INITIALIZED:
                    ESP_LOGI(TAG, "received MSG_INFORMER_APP_INITIALIZED");
                    break; 

                case MSG_CSI_RECEIVED: 
                    //ESP_LOGI(TAG, "received MSG_CSI_RECEIVED");
                    send_information(msg.info); 
                    vTaskDelay(1);
                    break; 

                case MSG_STOP_INFORMER_APP: 
                    if (informer_config->informer_mode == INFORMER_SEND_SD || informer_config->informer_mode == INFORMER_SEND_SD_SC) {
                        if (csi_fd != NULL) {
                            close_file(csi_fd); 
                            unmount_sd_storage(); 
                        }
                    }
                    break; 
            }
        }
    }
}


void start_informer_app(user_csi_configuration_t * app_configuration){
    ESP_LOGI(TAG, "starting Informer app...");
    informer_config = app_configuration; 

    //Create message queue for Informer app
    informer_app_queue = xQueueCreate(20, sizeof(informer_app_message_t)); 

    ESP_LOGI(TAG, "informer mode identified: %d", informer_config->informer_mode);
    if (informer_config->informer_mode == INFORMER_SEND_SD) {
        //Init sd storage and create CSI file
        sd_initialized = init_sd_storage(app_configuration->sd_config); 
        if (sd_initialized == ESP_OK) {
            strcpy(filename, get_csi_filename());
            ESP_LOGI(TAG, "Storing CSI in %s", filename);
        }
        

    } else if (informer_config->informer_mode == INFORMER_SEND_SERIAL_COMM) {
        uart_initialized = init_uart_component(informer_config->sc_config, get_default_uart_configuration()); 
    }

    //Create task for Informer app
    xTaskCreatePinnedToCore(&informer_task, "informer-task", INFORMER_APP_STACK_SIZE, NULL, INFORMER_APP_TASK_PRIORITY, NULL, INFORMER_APP_CORE_ID); 
}