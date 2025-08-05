/*
 * CSI app main source file
 * Created on: 02/02/2024
 * by: Jesus A. Armenta-Garcia
 */
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "rom/ets_sys.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "csi_app.h"
#include "wifi_app.h"
#include "informer.h"
#include "../components/udp_client.h"

static const char * TAG = "CSI app";

//Queue to handle event messages between tasks
static QueueHandle_t csi_app_queue; 

static bool has_stopped = false; 

static bool socket_created;

user_csi_configuration_t * csi_app_configuration; 

//Timer to control message sending according to sampling frequency
esp_timer_handle_t message_timer;
int period_ms; 

//Variable to register last timestamp of csi callback
static uint32_t last_timestamp = 0; 
static bool first_csi_callback = true; 

//Callback for timer, sends a message when timer runs out
static void timer_callback(void * arg) {
    csi_app_message_t n_msg = {
        .msgID = MSG_SEND_MESSAGE
    };
    csi_app_send_message(n_msg);
    vTaskDelay(1); //Avoid triggering IDLE task watchdog
}

/* Callback that will be called when a new CSI data packet is available
 * besides defined here, Wi-Fi task is the one listeting to this callback
 * @param ctx, context argument 
 * @param info, CSI data received
 */
static void csi_received_callback(void * ctx, wifi_csi_info_t * info){
    int sampling_delay = 1000.0 / csi_app_configuration->packet_frequency; 
    float tolerance = sampling_delay * 0.10;
    //Check for valid argument 
    if(!info || !info->buf){
        return; 
    }
    if(memcmp(info->mac, csi_app_configuration->mac_subs, 6)){
        return; 
    }
    //Use timestamp of csi data to avoid UDP repeated messages in low sampling frequency
    if (csi_app_configuration->packet_frequency < 30) {
        if (first_csi_callback) {
            first_csi_callback = false; 
            last_timestamp = info->rx_ctrl.timestamp; 
            //Send message to informer task to show/save the CSI data 
            informer_app_message_t n_msg = {
                .msgID = MSG_CSI_RECEIVED,
                .info = *info
            };
            informer_app_send_message(n_msg); 
        } else {
            uint32_t current_timestamp = info->rx_ctrl.timestamp; 
            if ((current_timestamp - last_timestamp)/1000.0 >= sampling_delay - tolerance ){
                last_timestamp = current_timestamp; 
                //Send message to informer task to show/save the CSI data 
                informer_app_message_t n_msg = {
                    .msgID = MSG_CSI_RECEIVED,
                    .info = *info
                };
                informer_app_send_message(n_msg); 
            }
        }
    } else {
        //Send message to informer task to show/save the CSI data 
        informer_app_message_t n_msg = {
            .msgID = MSG_CSI_RECEIVED,
            .info = *info
        };
        informer_app_send_message(n_msg); 
    }
}

wifi_csi_config_t csi_app_default_config(void){
    wifi_csi_config_t csi_config = {
        .lltf_en = CSI_APP_LLTF,
        .htltf_en = CSI_APP_HT_LTF,
        .stbc_htltf2_en = CSI_APP_STBC_HT_LTF,
        .ltf_merge_en = CSI_APP_LTF_MERGE, 
        .channel_filter_en = CSI_APP_CHANNEL_FILTER, 
        .manu_scale = CSI_APP_MANU_SCALE, 
        .shift = CSI_APP_SHIFT
    };
    return csi_config; 
}

BaseType_t csi_app_send_message(csi_app_message_t msg){
    return xQueueSend(csi_app_queue, &msg, portMAX_DELAY);
}

//Function to stop Wi-Fi CSI
static void csi_deinit() {
    if (csi_app_configuration->op_mode == CSI_RECV) {
        ESP_ERROR_CHECK(esp_wifi_set_csi(false));
    } else if (csi_app_configuration->op_mode == CSI_SNDR) {
        if (socket_created) {
            int err = destroy_socket();
            if (err == 0) {
                ESP_LOGI(TAG, "Socket closed and Wi-Fi CSI deactivated!");
            }
        }
    }
}

//Function to init Wi-Fi CSI
static void csi_init(wifi_csi_config_t * config){
    period_ms = 1000000 / csi_app_configuration->packet_frequency; 
    if(csi_app_configuration->op_mode == CSI_RECV){
        //ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true)); //CSI callback will filter by MAC address
        //Set CSI config received
        ESP_ERROR_CHECK(esp_wifi_set_csi_config(config));
        //Set CSI receiving callback function 
        ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(csi_received_callback, NULL));
        ESP_ERROR_CHECK(esp_wifi_set_csi(true));

    } else if(csi_app_configuration->op_mode == CSI_SNDR){
        socket_created = false; 
        //init socket for UDP communication with Rx
        while (!socket_created) {
            ESP_LOGI(TAG, "Creating socket for communication..."); 
            socket_created = create_socket(UDP_CLIENT_DEFAULT_ADDR_FAMILY, SOCK_DGRAM, UDP_CLIENT_DEFAULT_IP_PROTOCOL); 
        }
        //Configure and start periodic timer
        esp_timer_create_args_t timer_config = {
            .arg = NULL, 
            .callback = timer_callback,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "send_message_timer",
            .skip_unhandled_events = true
        };
        ESP_ERROR_CHECK(esp_timer_create(&timer_config, &message_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(message_timer,period_ms));
    }
}

static void csi_task(void * config){
    csi_app_message_t msg; 
    int send_err; 
    wifi_csi_config_t * csi_config = (wifi_csi_config_t * ) config;
    //Autosend message to start CSI collection
    csi_app_message_t n_msg = {
        .msgID = MSG_START_CSI_COLLECTION
    };
    csi_app_send_message(n_msg);
    while(1){
        if(xQueueReceive(csi_app_queue, &msg, portMAX_DELAY)){
            switch(msg.msgID){

                case MSG_START_CSI_COLLECTION:
                    ESP_LOGI(TAG, "received MSG_START_CSI_COLLECTION");
                    //Init CSI for data receiving
                    csi_init(csi_config);
                    ESP_LOGI(TAG, "CSI initialized!");
                    has_stopped = false; 
                    break;

                case MSG_STOP_CSI_COLLECTION:
                    if (!has_stopped) {
                        ESP_LOGI(TAG, "received MSG_STOP_CSI_COLLECTION");
                        has_stopped = true; 
                        ESP_ERROR_CHECK(esp_timer_stop(message_timer));
                        ESP_ERROR_CHECK(esp_timer_delete(message_timer));
                        csi_deinit(); 
                    }
                    break; 
                
                case MSG_SEND_MESSAGE:
                    if(!has_stopped){
                        ESP_LOGI(TAG, "received MSG_SEND_MESSAGE");
                        send_err = send_to_socket(UDP_CLIENT_DEFAULT_PAYLOAD, sizeof(UDP_CLIENT_DEFAULT_PAYLOAD));
                        if (send_err == -1) {
                            ESP_LOGW(TAG, "error sending data to socket"); 
                        }
                    }
                    break;

                default:
                    ESP_LOGE(TAG, "Message received not defined for handling");
                    break;
            }
        }
    }
}

void stop_csi_app(){
    csi_app_message_t n_msg = {
        .msgID = MSG_STOP_CSI_COLLECTION
    };
    csi_app_send_message(n_msg);
}

void start_csi_app(wifi_csi_config_t config, user_csi_configuration_t * http_csi_app_configuration){
    ESP_LOGI(TAG, "starting CSI app...");
    csi_app_configuration = http_csi_app_configuration; 
    //Create message queue for CSI app
    csi_app_queue = xQueueCreate(10, sizeof(csi_app_message_t));
    //Start informer task if device is configured as receiver
    if (http_csi_app_configuration->op_mode == CSI_RECV)
        start_informer_app(http_csi_app_configuration); 
    //Create task for CSI app
    xTaskCreatePinnedToCore(&csi_task, "csi-task", CSI_APP_STACK_SIZE, &config, CSI_APP_TASK_PRIORITY, NULL, CSI_APP_CORE_ID);
}