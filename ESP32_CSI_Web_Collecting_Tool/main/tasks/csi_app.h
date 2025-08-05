/*
 * Header file that includes constants and function prototypes for the CSI app
 * Created on: 02/01/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef CSI_APP_H
#define CSI_APP_H

#include <stdbool.h>
#include "esp_wifi.h"
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "../common/utils.h"


/*
 * Constants for Wi-Fi FreeRTOS task 
 */
#define CSI_APP_STACK_SIZE 4096
#define CSI_APP_TASK_PRIORITY 3 //Lower priority compared to Wi-Fi task
#define CSI_APP_CORE_ID 1 //Run in a separate core 

/*
 * CSI operation modes
 */
#define CSI_RECV 1
#define CSI_SNDR 2

#define CSI_APP_FREQ 20

/*
 * Default configuration values for CSI 
 */
#define CSI_APP_LLTF false //128 bytes of data per packet
#define CSI_APP_HT_LTF true //256 bytes of data per packet
#define CSI_APP_STBC_HT_LTF false //384 bytes of data per packet
#define CSI_APP_LTF_MERGE false 
#define CSI_APP_CHANNEL_FILTER false 
#define CSI_APP_MANU_SCALE false 
#define CSI_APP_SHIFT false 

/*
 * Utils for handling messages between tasks
 */

//Enum for Wi-Fi message IDs
typedef enum csi_app_message_id{
    MSG_START_CSI_COLLECTION = 0, 
    MSG_STOP_CSI_COLLECTION,
    MSG_SEND_MESSAGE
} csi_app_message_id_enum;

typedef struct csi_app_message {
    csi_app_message_id_enum msgID; 
} csi_app_message_t;

//Function for sending a message to the task queue
BaseType_t csi_app_send_message(csi_app_message_t msg);

//Gets the current CSI configuration
wifi_csi_config_t * csi_app_get_config(void);

//Set default CSI configuration
wifi_csi_config_t csi_app_default_config(void);


/*
 * Function to stop the CSI app when a change in operation mode occurs or a disconnection event
 */
void stop_csi_app(); 

/*
 * Function start the CSI app either for CSI receiver or sender
 * @param config: current CSI Wi-Fi configuration 
 * @param http_app_configuration: CSI app configuration received from HTTP webserver
 */
void start_csi_app(wifi_csi_config_t config, user_csi_configuration_t * http_app_configuration);

#endif