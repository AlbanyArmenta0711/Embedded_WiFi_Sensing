#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <esp_wifi_types.h>

#define INFORMER_CONSOLE_MODE 1
#define INFORMER_SEND_SD 2
#define INFORMER_SEND_SERIAL_COMM 3
#define INFORMER_SEND_SD_SC 4

typedef enum usr_data_representation{
    DATA_ASCII = 1, 
    DATA_BINARY
} usr_data_representation_enum; 

//Serial Communication Configuration
typedef struct serial_config {
    int sc_baudrate; 
    uint8_t sc_rx_pin; 
    uint8_t sc_tx_pin; 
} serial_config_t; 

//SD Configuration
typedef struct sd_config {
    uint8_t sd_clk_pin; 
    uint8_t sd_mosi_pin; 
    uint8_t sd_miso_pin; 
    uint8_t sd_cs_pin; 
} sd_config_t;

//CSI configuration struct that is built from user inputs in the HTTP server
typedef struct user_csi_configuration {
    usr_data_representation_enum data_mode; 
    int op_mode; 
    uint8_t mac_subs[6];
    wifi_bandwidth_t bandwidth; //See ESP Wi-Fi types for enum values
    int channel; 
    int packet_frequency; 
    //Data to report in the informer app 
    bool inform_mac_subscribed; 
    bool inform_csi_data; 
    bool inform_rssi; 
    bool inform_channel_bw; 
    bool inform_noise_floor; 
    bool inform_timestamp; 
    bool inform_ant_num; 
    bool inform_csi_len; 
    uint8_t informer_mode; 
    serial_config_t sc_config; 
    //SD configuration
    sd_config_t sd_config; 
} user_csi_configuration_t; 

/*
 * Function to set default values to user csi configuration
 */
void set_default_csi_configuration(user_csi_configuration_t * usr_csi_config);

#endif 