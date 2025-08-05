/*
 * Header file that includes constants and function prototypes for the Wi-Fi app
 * Created on: 01/30/2024
 * by: Jesus A. Armenta-Garcia
 */


#ifndef WIFI_APP_H
#define WIFI_APP_H

#include <esp_wifi_types.h>
#include "freertos/FreeRTOS.h"
#include "Freertos/task.h"

/*
 * Constants for Wi-Fi FreeRTOS task 
 */
#define WIFI_APP_STACK_SIZE 8192
#define WIFI_APP_TASK_PRIORITY 5 //6 is top priority, for tasks related to interruptions
#define WIFI_APP_CORE_ID 0 

/*
 * Constants for Wi-Fi Access Point configuration
 */
#define WIFI_APP_SSID "CSI_Collecting_ESP32"
#define WIFI_APP_PASSWORD "root1234"
#define WIFI_APP_AP_IP_ADDRESS "192.168.1.1"
#define WIFI_APP_AP_NETMASK "255.255.255.0"
#define WIFI_APP_AP_HIDDEN 0 //1 - Hide SSID, 0 - Show SSID 
#define WIFI_APP_CHANNEL 1
#define WIFI_APP_MAX_CONNECTIONS 5
#define WIFI_APP_BEACON_INTERVAL 100
#define WIFI_APP_BANDWIDTH WIFI_BW_HT20

/*
 * Constants for Wi-Fi access point in CSI Collection (Rx as AP)
 */
#define WIFI_APP_CSI_COLLECTION_SSID "CSI_Collection"
#define WIFI_APP_CSI_COLLECTION_PASSWORD "root1234"
#define WIFI_APP_CSI_COLLECTION_IP_ADDRESS "192.168.1.1"
#define WIFI_APP_CSI_COLLECTION_HIDDEN 0 //1 - Hide SSID, 0 - Show SSID 
#define WIFI_APP_CSI_COLLECTION_MAX_CONNECTIONS 5
#define WIFI_APP_CSI_COLLECTION_AUTH_MODE WIFI_AUTH_WPA2_PSK

/*
 * Constants for Wi-Fi station configuration
 */
#define WIFI_APP_MAX_RETRIES 3

/*
 * Utils for handling messages between tasks
 */

//Enum for Wi-Fi message IDs
typedef enum wifi_app_message_id{
    MSG_WIFI_APP_STA_CONNECTED = 0, 
    MSG_WIFI_APP_STA_DISCONNECTED,
    MSG_START_AP_MODE,
    MSG_RESET_WIFI, 
    MSG_CHANGE_TO_CSI_COLLECTION,
    MSG_START_CSI_APP_AS_RECEIVER,
    MSG_START_CSI_APP_AS_SENDER,
    MSG_ASK_CSI_TO_STOP
} wifi_app_message_id_enum;

//Message structure
typedef struct wifi_app_message{
    wifi_app_message_id_enum msgID; 
    void * data; 
} wifi_app_message_t; 

/*
 * Function for sending a message to the Wi-Fi app task queue
 * @param msg, the message to send to the task queue
 * @return pdTrue if the message was successfully posted to the task queue
 */
//Function for sending a message to the Wi-Fi app task queue
BaseType_t wifi_app_send_message(wifi_app_message_t msg); 

/*
 * Function for getting the current Wi-Fi configuration
 * @return a pointer to the current Wi-Fi configuration
 */
wifi_config_t * wifi_app_get_wifi_config(void); 

/*
 * Function to be called by main task to start the Wi-Fi app
 */
void wifi_app_start(void);

#endif