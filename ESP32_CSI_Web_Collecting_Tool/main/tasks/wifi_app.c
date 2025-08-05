/*
 * Wi-Fi app main source file
 * Created on: 02/01/2024
 * by: Jesus A. Armenta-Garcia
 */
#include <string.h>
#include <netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_now.h"
#include "esp_mac.h"

#include "wifi_app.h"
#include "http_server.h"
#include "csi_app.h"
#include "../components/nvs_storage.h"
#include "../components/sd_component.h"
#include "../common/utils.h"

//Tag used for identifying Wi-Fi app console messages
static const char * TAG = "Wi-Fi app";

static QueueHandle_t wifi_app_queue; 

static int current_mode = 0; //0 AP, 1 STA

static bool is_wifi_initialized_as_ap = false;

//esp-netif objects
esp_netif_t * g_esp_netif_ap = NULL; 
esp_netif_t * g_esp_netif_sta = NULL; 

//Configuration for CSI task
wifi_csi_config_t csi_config;

//Configuration received from the webserver to configure entire CSI collecting app
user_csi_configuration_t * app_configuration; 

/*
 * Wi-Fi app event handler
 * @param args, aside from event data, that is passed to the handler when it is called
 * @param event_base the base id of the event to register the handler for
 * @param event_id the id of the event to handle
 * @param event_data event data
 */
static void wifi_event_handler(void * args, esp_event_base_t event_base, int32_t event_id, void * event_data){
    wifi_event_ap_staconnected_t * sta_conn_event; 
    wifi_event_sta_disconnected_t * sta_disconn_event;
    wifi_app_message_t n_msg;
    char ssid[32] = "";
    if(event_base == WIFI_EVENT){
        switch(event_id){
            //Transmitter has been connected to receiver access point
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                n_msg.msgID = MSG_START_CSI_APP_AS_SENDER;
                n_msg.data = NULL;
                wifi_app_send_message(n_msg);
                break;

            //Station connected to access point event
            case WIFI_EVENT_AP_STACONNECTED:
                sta_conn_event = (wifi_event_ap_staconnected_t * ) event_data; 
                ESP_LOGI(TAG, "station connected, AID = %d",  sta_conn_event->aid);
                break; 
            
            //Transmitter as station has disconnect from receiver as access point
            case WIFI_EVENT_STA_DISCONNECTED: 
                sta_disconn_event = (wifi_event_sta_disconnected_t * ) event_data; 
                //Device was either sending or receiving CSI already
                if (current_mode == 1) {
                    n_msg.msgID = MSG_ASK_CSI_TO_STOP;
                    n_msg.data = NULL;
                    wifi_app_send_message(n_msg);
                }
                for(int i = 0; i < sta_disconn_event->ssid_len; i++){
                    ssid[i] = (char) sta_disconn_event->ssid[i];
                }
                ssid[sta_disconn_event->ssid_len] = '\0';
                ESP_LOGI(TAG, "station disconnected from %s, reason = %d. Retrying connection", ssid, sta_disconn_event->reason);
                //Retry connection
                ESP_ERROR_CHECK(esp_wifi_connect());
                break; 

            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START event received");
                break;
        }
    } 
}

BaseType_t wifi_app_send_message(wifi_app_message_t msg){
    return xQueueSend(wifi_app_queue, &msg, portMAX_DELAY);
}

/*
 * Function to configure Wi-Fi as station after initialization for CSI handling
 */
static esp_netif_t * wifi_config_sta(){
    uint8_t sta_mac [8];
    esp_netif_t * esp_netif_sta = esp_netif_create_default_wifi_sta(); 
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, app_configuration->bandwidth)); 
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G)); //Force to 802.11n 
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_APP_CSI_COLLECTION_SSID,
            .password = WIFI_APP_CSI_COLLECTION_PASSWORD
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    //Set channel according to bandwidth selected
    if (app_configuration->bandwidth == WIFI_BW_HT20) {
        ESP_ERROR_CHECK(esp_wifi_set_channel(app_configuration->channel, WIFI_SECOND_CHAN_NONE));
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_channel(app_configuration->channel, WIFI_SECOND_CHAN_ABOVE));
    }
    //ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(ESP_IF_WIFI_STA, WIFI_PHY_RATE_MCS0_SGI));
    esp_efuse_mac_get_default(sta_mac);
    ESP_LOGI(TAG, "station initialized! MAC: "MACSTR"", MAC2STR(sta_mac));
    return esp_netif_sta; 
}

static esp_netif_t * wifi_config_ap_for_collection(){
    esp_netif_t * esp_netif_ap = esp_netif_create_default_wifi_ap(); 
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_APP_CSI_COLLECTION_SSID,
            .password = WIFI_APP_CSI_COLLECTION_PASSWORD, 
            .ssid_len = strlen(WIFI_APP_CSI_COLLECTION_SSID),
            .channel = app_configuration->channel,
            .ssid_hidden = WIFI_APP_CSI_COLLECTION_HIDDEN,
            .max_connection = WIFI_APP_CSI_COLLECTION_MAX_CONNECTIONS,
            .authmode = WIFI_APP_CSI_COLLECTION_AUTH_MODE
        }
    };
    //Set static IP 
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(esp_netif_ap)); //Stop DHCP service to configure it
    esp_netif_ip_info_t ap_ip_info; 
    memset(&ap_ip_info, 0, sizeof(esp_netif_ip_info_t));
    ap_ip_info.ip.addr = ipaddr_addr(WIFI_APP_CSI_COLLECTION_IP_ADDRESS);
    ap_ip_info.netmask.addr = ipaddr_addr(WIFI_APP_AP_NETMASK);
    ap_ip_info.gw.addr = ipaddr_addr(WIFI_APP_CSI_COLLECTION_IP_ADDRESS);
    if(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info) != ESP_OK){
        ESP_LOGE(TAG, "Failed to set static IP info");
    }
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap)); //Start DHCP service again
    //Show access point information
    ESP_LOGI(TAG, "SSID: %s", WIFI_APP_CSI_COLLECTION_SSID);
    ESP_LOGI(TAG, "IP: %s", WIFI_APP_CSI_COLLECTION_IP_ADDRESS);
    ESP_LOGI(TAG, "Gateway: %s", WIFI_APP_CSI_COLLECTION_IP_ADDRESS);
    ESP_LOGI(TAG, "Netmask: %s", WIFI_APP_AP_NETMASK);
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP,mac));
    ESP_LOGI(TAG, "MAC address as AP: "MACSTR"", MAC2STR(mac));

    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    ESP_LOGI(TAG, "MAC address as STA: "MACSTR"", MAC2STR(mac));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, app_configuration->bandwidth));
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G)); //Force to 802.11b/g
    ESP_LOGI(TAG, "Wi-Fi app configuration finished.");

    //Start Wi-Fi service
    ESP_ERROR_CHECK(esp_wifi_start());

    return esp_netif_ap; 
}

/*
 * Function to configure Wi-Fi as access point after initialization
 */
static esp_netif_t * wifi_config_ap(){
    //Create default network access point
    esp_netif_t * esp_netif_ap = esp_netif_create_default_wifi_ap(); 
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_APP_SSID,
            .password = WIFI_APP_PASSWORD, 
            .ssid_len = strlen(WIFI_APP_SSID),
            .channel = WIFI_APP_CHANNEL,
            .ssid_hidden = WIFI_APP_AP_HIDDEN,
            .max_connection = WIFI_APP_MAX_CONNECTIONS,
        }
    };
    
    //Set static IP 
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(esp_netif_ap)); //Stop DHCP service to configure it
    esp_netif_ip_info_t ap_ip_info; 
    memset(&ap_ip_info, 0, sizeof(esp_netif_ip_info_t));
    ap_ip_info.ip.addr = ipaddr_addr(WIFI_APP_AP_IP_ADDRESS);
    ap_ip_info.netmask.addr = ipaddr_addr(WIFI_APP_AP_NETMASK);
    ap_ip_info.gw.addr = ipaddr_addr(WIFI_APP_AP_IP_ADDRESS);
    if(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info) != ESP_OK){
        ESP_LOGE(TAG, "Failed to set static IP info");
    }
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap)); //Start DHCP service again
    //Show access point information
    ESP_LOGI(TAG, "SSID: %s", WIFI_APP_SSID);
    ESP_LOGI(TAG, "IP: %s", WIFI_APP_AP_IP_ADDRESS);
    ESP_LOGI(TAG, "Gateway: %s", WIFI_APP_AP_IP_ADDRESS);
    ESP_LOGI(TAG, "Netmask: %s", WIFI_APP_AP_NETMASK);
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP,mac));
    ESP_LOGI(TAG, "MAC address as AP: "MACSTR"", MAC2STR(mac));
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    ESP_LOGI(TAG, "MAC address as STA: "MACSTR"", MAC2STR(mac));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_APP_BANDWIDTH));
    ESP_LOGI(TAG, "Wi-Fi app configuration finished.");

    //Start Wi-Fi service
    ESP_ERROR_CHECK(esp_wifi_start());

    return esp_netif_ap; 
}

/*
 * Function to deinit Wi-Fi to make a change in operation mode
 */
static void wifi_deinit(){
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_netif_destroy_default_wifi(g_esp_netif_ap);
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler
    ));
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    g_esp_netif_ap = NULL;
}

/*
 * Function to initialize Wi-Fi access point
 */
static void wifi_init(){
    //Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    //Create system Event task and initialize an application event's callback function
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, 
        ESP_EVENT_ANY_ID,
        &wifi_event_handler, //Function callback for when a Wi-Fi event occurs
        NULL,
        NULL
    ));

    //Set default configuration
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT(); 
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_LOGI(TAG, "Wi-Fi init phase finished");
}

/*
 * Wi-Fi app task that will be running in a core scheduled by FreeRTOS
 */
static void wifi_task(void * parameters){
    wifi_app_message_t msg; 
    wifi_app_message_t n_msg;
    app_configuration = nvs_load_csi_configuration(); 
    //Check if CSI has been configured previously
    if (app_configuration != NULL) { 
        if (app_configuration->op_mode == CSI_RECV || app_configuration->op_mode == CSI_SNDR) {
            //Send message to start CSI collection with stored configuration
            n_msg.msgID = MSG_CHANGE_TO_CSI_COLLECTION; 
            n_msg.data = app_configuration;
        } 
    } else {
        //No configuration was saved, start as AP mode
        n_msg.msgID = MSG_START_AP_MODE; 
        n_msg.data = NULL;
    }

    wifi_app_send_message(n_msg);

    //Keep running the task for message handling
    while(1){
        if(xQueueReceive(wifi_app_queue, &msg, portMAX_DELAY)){
            switch(msg.msgID){

                case MSG_START_AP_MODE:
                    ESP_LOGI(TAG, "received MSG_START_AP_MODE");
                    //Call function corresponding to Wi-Fi init phase according to Espressif
                    wifi_init(); 
                    g_esp_netif_ap = wifi_config_ap(); 
                    ESP_LOGI(TAG, "Wi-Fi app started");
                    is_wifi_initialized_as_ap = true; 
                    //Start webserver
                    http_start_server(); 
                    current_mode = 0; 
                    break; 

                case MSG_START_CSI_APP_AS_RECEIVER:
                    ESP_LOGI(TAG, "received MSG_START_CSI_APP_AS_RECEIVER");
                    //First set default config must be called 
                    csi_config = csi_app_default_config();
                    //Start CSI task
                    start_csi_app(csi_config, app_configuration);
                    current_mode = 1; 
                    break;

                case MSG_START_CSI_APP_AS_SENDER:
                    ESP_LOGI(TAG, "received MSG_START_CSI_APP_AS_SENDER");
                    //First set default config must be called 
                    csi_config = csi_app_default_config();
                    //Start CSI task
                    start_csi_app(csi_config, app_configuration);
                    current_mode = 1; 
                    break; 

                case MSG_CHANGE_TO_CSI_COLLECTION: 
                    ESP_LOGI(TAG, "received MSG_CHANGE_TO_CSI_COLLECTION from HTTP server");
                    app_configuration = (user_csi_configuration_t *) msg.data; 
                    ESP_LOGI(TAG, "CSI operation mode: %d", app_configuration->op_mode);
                    ESP_LOGI(TAG, "CSI Informer mode: %d", app_configuration->informer_mode);
                    //Stop Wi-Fi AP
                    //Call function to start de-init if Wi-Fi has been initialized previously
                    if (is_wifi_initialized_as_ap){
                        wifi_deinit();
                        is_wifi_initialized_as_ap = false; 
                        ESP_LOGI(TAG, "AP mode has been stopped");
                    }
                    wifi_init(); 
                    //Disable power saving for minimizing delay in Wi-Fi data receiving
                    esp_wifi_set_ps(WIFI_PS_NONE);  //MOVE TO STA CONFIGURATION IF ERROR
                    if (app_configuration->op_mode == CSI_RECV) {
                        g_esp_netif_ap = wifi_config_ap_for_collection(); 
                        ESP_LOGI(TAG, "Wi-Fi app started for CSI Collection");
                        wifi_app_message_t n_msg = {
                            .msgID = MSG_START_CSI_APP_AS_RECEIVER,
                            .data = NULL
                        };
                        wifi_app_send_message(n_msg);
                    } else {
                        g_esp_netif_sta = wifi_config_sta();
                        ESP_ERROR_CHECK(esp_wifi_connect());
                        
                    }
                    break;

                case MSG_ASK_CSI_TO_STOP:
                    stop_csi_app();
                    break;

                case MSG_RESET_WIFI:
                     //Clear also CSI configuration from NVS
                    nvs_clear_configuration(); 
                    esp_restart();
                    break; 

                default:
                    ESP_LOGE(TAG, "Message received not defined for handling");
                    break;
            }
        }
    }
}

void wifi_app_start(void){
    ESP_LOGI(TAG, "Starting Wi-Fi app");
    app_configuration = (user_csi_configuration_t * ) malloc(sizeof(user_csi_configuration_t));
    //Create message queue for handling communication between tasks
    wifi_app_queue = xQueueCreate(10, sizeof(wifi_app_message_t));
    //Create Wi-Fi app task
    xTaskCreatePinnedToCore(&wifi_task, "wifi_app_task", WIFI_APP_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL, WIFI_APP_CORE_ID);
}