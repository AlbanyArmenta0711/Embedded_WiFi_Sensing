/*
 * Header file that includes constants and function prototypes for the Informer task
 * Created on: 02/07/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef INFORMER_H
#define INFORMER_H

#include <stdbool.h>
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"

/*
 * Constants for Wi-Fi FreeRTOS task 
 */
#define INFORMER_APP_STACK_SIZE 8192
#define INFORMER_APP_TASK_PRIORITY 5 //Equal to idle task to avoid starvation BEFORE WAS EQUAL TO tskIDLE_PRIORITY
#define INFORMER_APP_CORE_ID 1 

//Enum for Wi-Fi message IDs
typedef enum informer_app_message_id{
    MSG_CSI_RECEIVED = 0,
    MSG_INFORMER_APP_INITIALIZED,
    MSG_STOP_INFORMER_APP
} informer_app_message_id_enum; 

typedef struct csi_data{
    wifi_pkt_rx_ctrl_t pkt_ctrl;
    uint8_t mac_addr[6]; 
    uint16_t len;   
    int8_t * buf; 
} csi_data_t;

typedef struct informer_app_message {
    informer_app_message_id_enum msgID;
    wifi_csi_info_t info; //CSI info to report
} informer_app_message_t; 

//Function for sending a message to the task queue 
BaseType_t informer_app_send_message(informer_app_message_t msg); 

/*
 * Function to start the Informer app, which can send CSI info to console, serial COM or SD
 * @param app_configuration: parameter that includes the informer configuration
 */
void start_informer_app(user_csi_configuration_t * app_configuration); 

#endif