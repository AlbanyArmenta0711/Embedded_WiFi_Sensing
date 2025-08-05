/*
 * http_server app main source file
 * Created on: 02/09/2024
 * by: Jesus A. Armenta-Garcia
 */
#include <string.h>
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sys/param.h"

#include "wifi_app.h"
#include "http_server.h"
#include "../common/utils.h"
#include "../components/nvs_storage.h"

//Instance of HTTP server
httpd_handle_t http_webserver = NULL; 

char * TAG = "HTTP server";


/*
 * Webpage file's content will be added to the .rodata section in flash, available via symbol names as follows:
 * The names are generated from the full name of the file, as given in EMBED_FILES. Characters /, ., etc. are replaced 
 * with underscores. The _binary prefix in the symbol name is added by objcopy and is the same for both text and binary files.
 */
extern const uint8_t index_html_start[] asm("_binary_index_html_start"); 
extern const uint8_t index_html_end[] asm("_binary_index_html_end"); 
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
extern const uint8_t jquery_3_3_1_min_js_start[] asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[] asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");
extern const uint8_t bootstrap_min_css_start[] asm("_binary_bootstrap_min_css_start");
extern const uint8_t bootstrap_min_css_end[] asm("_binary_bootstrap_min_css_end");
extern const uint8_t esp32_png_start[] asm("_binary_esp32_png_start");
extern const uint8_t esp32_png_end[] asm("_binary_esp32_png_end");

void set_default_csi_configuration(user_csi_configuration_t * usr_csi_config){
    usr_csi_config->data_mode = DATA_ASCII; 
    uint8_t sender_mac [6] = {0x08, 0xd1, 0xf9, 0x29, 0x91, 0xe8};
    memcpy(usr_csi_config->mac_subs, sender_mac, sizeof(sender_mac));
    usr_csi_config->op_mode = 0;
    usr_csi_config->bandwidth = WIFI_APP_BANDWIDTH;
    usr_csi_config->channel = WIFI_APP_CHANNEL; 
    usr_csi_config->packet_frequency = 20; 
    usr_csi_config->inform_mac_subscribed = true; 
    usr_csi_config->inform_csi_data = true; 
    usr_csi_config->inform_rssi = true;
    usr_csi_config->inform_channel_bw = true; 
    usr_csi_config->inform_noise_floor = true; 
    usr_csi_config->inform_timestamp = true; 
    usr_csi_config->inform_ant_num = true; 
    usr_csi_config->inform_csi_len = true; 
    usr_csi_config->informer_mode = INFORMER_CONSOLE_MODE; 

    usr_csi_config->sc_config.sc_baudrate = 921600; 
    usr_csi_config->sc_config.sc_rx_pin = 5; 
    usr_csi_config->sc_config.sc_tx_pin = 4; 

    usr_csi_config->sd_config.sd_clk_pin = 19; 
    usr_csi_config->sd_config.sd_mosi_pin = 22; 
    usr_csi_config->sd_config.sd_miso_pin = 23; 
    usr_csi_config->sd_config.sd_cs_pin = 18; 

}

static esp_err_t index_html_uri_handler (httpd_req_t * data_request){
    ESP_LOGI(TAG, "index.html requested");
    httpd_resp_set_type(data_request, "text/html"); 
    httpd_resp_send(data_request, (const char *) index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static esp_err_t app_js_uri_handler (httpd_req_t * data_request){
    ESP_LOGI(TAG, "app.js requested"); 
    httpd_resp_set_type(data_request, "text/javascript");
    httpd_resp_send(data_request, (const char *) app_js_start, app_js_end - app_js_start);
    return ESP_OK;
}

static esp_err_t jquery_uri_handler(httpd_req_t * data_request){
    ESP_LOGI(TAG, "jquery requested"); 
    httpd_resp_set_type(data_request, "text/javascript");
    httpd_resp_send(data_request, (const char *) jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);
    return ESP_OK; 
}

static esp_err_t bootstrap_uri_handler(httpd_req_t * data_request){
    ESP_LOGI(TAG, "bootstrap requested"); 
    httpd_resp_set_type(data_request, "text/css");
    httpd_resp_send(data_request, (const char *) bootstrap_min_css_start, bootstrap_min_css_end - bootstrap_min_css_start);
    return ESP_OK; 
}

static esp_err_t favicon_ico_uri_handler(httpd_req_t * data_request){
    ESP_LOGI(TAG, "favicon requested");
    httpd_resp_set_type(data_request, "image/x-icon");
    httpd_resp_send(data_request, (const char *) favicon_ico_start, favicon_ico_end - favicon_ico_start);
    return ESP_OK; 
}

static esp_err_t esp32_image_uri_handler(httpd_req_t * data_request){
    ESP_LOGI(TAG, "esp32 image requested");
    httpd_resp_set_type(data_request, "image/png");
    httpd_resp_send(data_request, (const char *) esp32_png_start, esp32_png_end - esp32_png_start);
    return ESP_OK; 
}

static esp_err_t csi_configuration_json_uri_handler(httpd_req_t * data_request){
    bool csi_configuration_received = false; 
    char * str_op_mode = NULL, * str_mac_address = NULL, * str_comm_channel = NULL, * str_frequency = NULL, * str_bandwidth = NULL, * str_data_representation = NULL; 
    char * str_is_mac_address = NULL, * str_is_csi_buff = NULL, * str_is_rssi = NULL, * str_is_channel_bandwidth = NULL, * str_is_noise_floor = NULL; 
    char * str_is_timestamp = NULL, * str_is_antenna_number = NULL, * str_is_csi_data_length = NULL, * str_informer_mode = NULL; 
    char * str_miso = NULL, * str_mosi = NULL, * str_clk = NULL, * str_cs = NULL, * str_baud_rate = NULL, * str_tx = NULL, * str_rx = NULL; 

    ssize_t len_op_mode = 0, len_mac_address = 0, len_comm_channel = 0, len_frequency = 0, len_bandwidth = 0, len_data_representation = 0; 
    ssize_t len_is_mac_address = 0, len_is_csi_buff = 0, len_is_rssi = 0, len_is_channel_bandwidth = 0, len_is_noise_floor = 0; 
    ssize_t len_is_timestamp = 0, len_is_antenna_number = 0, len_is_csi_data_length = 0, len_informer_mode = 0; 
    ssize_t len_miso = 0, len_mosi = 0, len_clk = 0, len_cs = 0, len_baud_rate = 0, len_tx = 0, len_rx = 0; 
    ESP_LOGI(TAG, "csiConfiguration.json posted");

    //Operation Mode from header (both Transmitter and Receiver)
    len_op_mode = httpd_req_get_hdr_value_len(data_request, HEADER_OP_MODE) + 1; 
    if(len_op_mode > 1) {
        str_op_mode = (char *) malloc(len_op_mode);
        if(httpd_req_get_hdr_value_str(data_request, HEADER_OP_MODE, str_op_mode, len_op_mode) == ESP_OK){
            ESP_LOGI(TAG, "CSI operation mode: %s", str_op_mode);
        } else {
            ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_OP_MODE);
            return ESP_FAIL; 
        }
    } else {
        ESP_LOGE(TAG, "No header value found: %s", HEADER_OP_MODE);
        return ESP_FAIL; 
    }

    //Mac Address from header (both Transmitter and Receiver)
    len_mac_address = httpd_req_get_hdr_value_len(data_request, HEADER_MAC_ADDRESS) + 1; 
    if(len_mac_address > 1) {
        str_mac_address = (char *) malloc(len_mac_address);
        if(httpd_req_get_hdr_value_str(data_request, HEADER_MAC_ADDRESS, str_mac_address, len_mac_address) == ESP_OK){
            ESP_LOGI(TAG, "Mac Address subscribed: %s", str_mac_address);
            csi_configuration_received = true; 
        } else {
            ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_MAC_ADDRESS);
            return ESP_FAIL; 
        }
    } else {
        ESP_LOGE(TAG, "No header value found: %s", HEADER_MAC_ADDRESS);
        return ESP_FAIL; 
    }

    //Communication Channel from header (both Transmitter and Receiver)
    len_comm_channel = httpd_req_get_hdr_value_len(data_request, HEADER_COMMUNICATION_CHANNEL) + 1; 
    if(len_comm_channel > 1) {
        str_comm_channel = (char *) malloc(len_comm_channel);
        if(httpd_req_get_hdr_value_str(data_request, HEADER_COMMUNICATION_CHANNEL, str_comm_channel, len_comm_channel) == ESP_OK){
            ESP_LOGI(TAG, "Communication channel: %s", str_comm_channel);
        } else {
            ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_COMMUNICATION_CHANNEL);
            return ESP_FAIL; 
        }
    } else {
        ESP_LOGE(TAG, "No header value found: %s", HEADER_COMMUNICATION_CHANNEL);
        return ESP_FAIL; 
    }

    //Frequency from header (both Transmitter and Receiver)
    len_frequency = httpd_req_get_hdr_value_len(data_request, HEADER_FREQUENCY) + 1; 
    if(len_frequency > 1) {
        str_frequency = (char *) malloc(len_frequency);
        if(httpd_req_get_hdr_value_str(data_request, HEADER_FREQUENCY, str_frequency, len_frequency) == ESP_OK){
            ESP_LOGI(TAG, "Frequency: %s", str_frequency);
        } else {
            ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_FREQUENCY);
            return ESP_FAIL; 
        }
    } else {
        ESP_LOGE(TAG, "No header value found: %s", HEADER_FREQUENCY);
        return ESP_FAIL; 
    }

    //Bandwidth from header (both Transmitter and Receiver)
    len_bandwidth = httpd_req_get_hdr_value_len(data_request, HEADER_BANDWIDTH) + 1; 
    if(len_bandwidth > 1) {
        str_bandwidth = (char *) malloc(len_bandwidth);
        if(httpd_req_get_hdr_value_str(data_request, HEADER_BANDWIDTH, str_bandwidth, len_bandwidth) == ESP_OK){
            ESP_LOGI(TAG, "Bandwidth: %s", str_bandwidth);
        } else {
            ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_BANDWIDTH);
            return ESP_FAIL; 
        }
    } else {
        ESP_LOGE(TAG, "No header value found: %s", HEADER_BANDWIDTH);
        return ESP_FAIL; 
    }

    /********************************************************************************
     * FOR RECEIVER CONFIGURATION
     ********************************************************************************/
    //Data Representation from header (RECV = 1)
    if (strcmp(str_op_mode, "1") == 0) {
        len_data_representation = httpd_req_get_hdr_value_len(data_request, HEADER_DATA_REPRESENTATION) + 1; 
        if(len_data_representation > 1) {
            str_data_representation = (char *) malloc(len_data_representation);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_DATA_REPRESENTATION, str_data_representation, len_data_representation) == ESP_OK){
                ESP_LOGI(TAG, "Data Representation: %s", str_data_representation);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_DATA_REPRESENTATION);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s, ignore if device is set as transmitter", HEADER_DATA_REPRESENTATION);
            return ESP_FAIL; 
        }
        //Is Mac Address from header
        len_is_mac_address = httpd_req_get_hdr_value_len(data_request, HEADER_IS_MAC_ADDRESS) + 1; 
        if(len_is_mac_address > 1) {
            str_is_mac_address = (char *) malloc(len_is_mac_address);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_MAC_ADDRESS, str_is_mac_address, len_is_mac_address) == ESP_OK){
                ESP_LOGI(TAG, "Is mac address: %s", str_is_mac_address);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_MAC_ADDRESS);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_MAC_ADDRESS);
            return ESP_FAIL; 
        }

        //Is CSI Buff from header
        len_is_csi_buff = httpd_req_get_hdr_value_len(data_request, HEADER_IS_CSI_BUFF) + 1; 
        if(len_is_csi_buff > 1) {
            str_is_csi_buff = (char *) malloc(len_is_csi_buff);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_CSI_BUFF, str_is_csi_buff, len_is_csi_buff) == ESP_OK){
                ESP_LOGI(TAG, "Is csi buff: %s", str_is_csi_buff);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_CSI_BUFF);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_CSI_BUFF);
            return ESP_FAIL; 
        }

        //Is RSSI from header
        len_is_rssi = httpd_req_get_hdr_value_len(data_request, HEADER_IS_RSSI) + 1; 
        if(len_is_rssi > 1) {
            str_is_rssi = (char *) malloc(len_is_rssi);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_RSSI, str_is_rssi, len_is_rssi) == ESP_OK){
                ESP_LOGI(TAG, "Is RSSI: %s", str_is_rssi);
                csi_configuration_received = true; 
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_RSSI);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_RSSI);
            return ESP_FAIL; 
        }

        //Is Channel Bandwidth from header
        len_is_channel_bandwidth = httpd_req_get_hdr_value_len(data_request, HEADER_IS_CHANNEL_BANDWIDTH) + 1; 
        if(len_is_channel_bandwidth > 1) {
            str_is_channel_bandwidth = (char *) malloc(len_is_channel_bandwidth);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_CHANNEL_BANDWIDTH, str_is_channel_bandwidth, len_is_channel_bandwidth) == ESP_OK){
                ESP_LOGI(TAG, "Is channel bandwidth: %s", str_is_channel_bandwidth);
                csi_configuration_received = true; 
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_CHANNEL_BANDWIDTH);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_CHANNEL_BANDWIDTH);
            return ESP_FAIL; 
        }

        //Is Noise Floor from header
        len_is_noise_floor = httpd_req_get_hdr_value_len(data_request, HEADER_IS_NOISE_FLOOR) + 1; 
        if(len_is_noise_floor > 1) {
            str_is_noise_floor = (char *) malloc(len_is_noise_floor);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_NOISE_FLOOR, str_is_noise_floor, len_is_noise_floor) == ESP_OK){
                ESP_LOGI(TAG, "Is Noise Floor: %s", str_is_noise_floor);
                csi_configuration_received = true; 
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_NOISE_FLOOR);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_NOISE_FLOOR);
            return ESP_FAIL; 
        }

        //Is Timestamp from header
        len_is_timestamp = httpd_req_get_hdr_value_len(data_request, HEADER_IS_TIMESTAMP) + 1; 
        if(len_is_timestamp > 1) {
            str_is_timestamp = (char *) malloc(len_is_timestamp);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_TIMESTAMP, str_is_timestamp, len_is_timestamp) == ESP_OK){
                ESP_LOGI(TAG, "Is timestamp: %s", str_is_timestamp);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_TIMESTAMP);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_TIMESTAMP);
            return ESP_FAIL; 
        }

        //Is Antenna Number from header
        len_is_antenna_number = httpd_req_get_hdr_value_len(data_request, HEADER_IS_ANTENNA_NUMBER) + 1; 
        if(len_is_antenna_number > 1) {
            str_is_antenna_number = (char *) malloc(len_is_antenna_number);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_ANTENNA_NUMBER, str_is_antenna_number, len_is_antenna_number) == ESP_OK){
                ESP_LOGI(TAG, "Is antenna number: %s", str_is_antenna_number);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_ANTENNA_NUMBER);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_ANTENNA_NUMBER);
            return ESP_FAIL; 
        }

        //Is CSI Data Length from header
        len_is_csi_data_length = httpd_req_get_hdr_value_len(data_request, HEADER_IS_CSI_DATA_LENGTH) + 1; 
        if(len_is_csi_data_length > 1) {
            str_is_csi_data_length = (char *) malloc(len_is_csi_data_length);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_IS_CSI_DATA_LENGTH, str_is_csi_data_length, len_is_csi_data_length) == ESP_OK){
                ESP_LOGI(TAG, "Is CSI data length: %s", str_is_csi_data_length);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_IS_CSI_DATA_LENGTH);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_IS_CSI_DATA_LENGTH);
            return ESP_FAIL; 
        }
        //Informer Mode from header
        len_informer_mode = httpd_req_get_hdr_value_len(data_request, HEADER_INFORMER_MODE) + 1; 
        if(len_informer_mode > 1) {
            str_informer_mode = (char *) malloc(len_informer_mode);
            if(httpd_req_get_hdr_value_str(data_request, HEADER_INFORMER_MODE, str_informer_mode, len_informer_mode) == ESP_OK){
                ESP_LOGI(TAG, "Informer mode: %s", str_informer_mode);
            } else {
                ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_INFORMER_MODE);
                return ESP_FAIL; 
            }
        } else {
            ESP_LOGE(TAG, "No header value found: %s", HEADER_INFORMER_MODE);
            return ESP_FAIL; 
        }

        //Check if informer mode is SD or SD and Serial Comm for pin configuration
        if (strcmp(str_informer_mode, "2") == 0 || strcmp(str_informer_mode, "4") == 0) {
            //MISO from header
            len_miso = httpd_req_get_hdr_value_len(data_request, HEADER_MISO) + 1; 
            if(len_miso > 1) {
                str_miso = (char *) malloc(len_miso);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_MISO, str_miso, len_miso) == ESP_OK){
                    ESP_LOGI(TAG, "MISO pin: %s", str_miso);
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_MISO);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_MISO);
                return ESP_FAIL; 
            }

            //MOSI from header
            len_mosi = httpd_req_get_hdr_value_len(data_request, HEADER_MOSI) + 1; 
            if(len_mosi > 1) {
                str_mosi = (char *) malloc(len_mosi);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_MOSI, str_mosi, len_mosi) == ESP_OK){
                    ESP_LOGI(TAG, "MOSI pin: %s", str_mosi);
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_MOSI);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_MOSI);
                return ESP_FAIL; 
            }

            //CLK from header
            len_clk = httpd_req_get_hdr_value_len(data_request, HEADER_CLK) + 1; 
            if(len_clk > 1) {
                str_clk = (char *) malloc(len_clk);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_CLK, str_clk, len_clk) == ESP_OK){
                    ESP_LOGI(TAG, "CLK pin: %s", str_clk);
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_CLK);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_CLK);
                return ESP_FAIL; 
            }

            //CS from header
            len_cs = httpd_req_get_hdr_value_len(data_request, HEADER_CS) + 1; 
            if(len_cs > 1) {
                str_cs = (char *) malloc(len_cs);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_CS, str_cs, len_cs) == ESP_OK){
                    ESP_LOGI(TAG, "CS pin: %s", str_cs);
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_CS);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_CS);
                return ESP_FAIL; 
            }
        }

        if (strcmp(str_informer_mode, "3") == 0 || strcmp(str_informer_mode, "4") == 0) {
            //Baudrate from header
            len_baud_rate = httpd_req_get_hdr_value_len(data_request, HEADER_BAUDRATE) + 1; 
            if(len_baud_rate > 1) {
                str_baud_rate = (char *) malloc(len_baud_rate);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_BAUDRATE, str_baud_rate, len_baud_rate) == ESP_OK){
                    ESP_LOGI(TAG, "Baudrate: %s", str_baud_rate);
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_BAUDRATE);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_BAUDRATE);
                return ESP_FAIL; 
            }
            
            //TX from header
            len_tx = httpd_req_get_hdr_value_len(data_request, HEADER_TX) + 1; 
            if(len_tx > 1) {
                str_tx = (char *) malloc(len_tx);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_TX, str_tx, len_tx) == ESP_OK){
                    ESP_LOGI(TAG, "TX pin: %s", str_tx);
                    csi_configuration_received = true; 
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_TX);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_TX);
                return ESP_FAIL; 
            }

            //RX from header
            len_rx = httpd_req_get_hdr_value_len(data_request, HEADER_RX) + 1; 
            if(len_rx > 1) {
                str_rx = (char *) malloc(len_rx);
                if(httpd_req_get_hdr_value_str(data_request, HEADER_RX, str_rx, len_rx) == ESP_OK){
                    ESP_LOGI(TAG, "RX pin: %s", str_rx);
                    csi_configuration_received = true; 
                } else {
                    ESP_LOGE(TAG, "Error receiving CSI configuration header value: %s", HEADER_RX);
                    return ESP_FAIL; 
                }
            } else {
                ESP_LOGE(TAG, "No header value found: %s", HEADER_RX);
                return ESP_FAIL; 
            }
        }
        
    }
    
    if(csi_configuration_received){
        //Set values to struct usr_csi_config
        user_csi_configuration_t * usr_csi_config  = (user_csi_configuration_t *) malloc(sizeof(user_csi_configuration_t));
        set_default_csi_configuration(usr_csi_config);
        usr_csi_config->op_mode = atoi(str_op_mode);
        //Tokenize mac address to get it as array
        char * token = strtok(str_mac_address, ":");
        int i = 0; 
        while(token != NULL) {
            usr_csi_config->mac_subs[i] = (int) strtol(token, NULL, 16);
            i++;
            token = strtok(NULL, ":"); 
        }
        usr_csi_config->bandwidth = atoi(str_bandwidth);
        usr_csi_config->channel = atoi(str_comm_channel);
        usr_csi_config->packet_frequency = atoi(str_frequency); 
        //This only for receiver 
        if (usr_csi_config->op_mode == 1) {
            usr_csi_config->data_mode = atoi(str_data_representation);
            usr_csi_config->inform_mac_subscribed = (!strcmp(str_is_mac_address, "true")); 
            usr_csi_config->inform_csi_data = (!strcmp(str_is_csi_buff, "true"));
            usr_csi_config->inform_rssi = (!strcmp(str_is_rssi, "true"));
            usr_csi_config->inform_channel_bw = (!strcmp(str_is_channel_bandwidth, "true"));
            usr_csi_config->inform_noise_floor = (!strcmp(str_is_noise_floor, "true"));
            usr_csi_config->inform_timestamp = (!strcmp(str_is_timestamp, "true"));
            usr_csi_config->inform_ant_num = (!strcmp(str_is_antenna_number, "true"));
            usr_csi_config->inform_csi_len = (!strcmp(str_is_csi_data_length, "true")); 
            usr_csi_config->informer_mode = atoi(str_informer_mode);
            //Check if informer mode is Serial Communication or SD and Serial Comm for pin configuration
            if (strcmp(str_informer_mode, "3") == 0 || strcmp(str_informer_mode, "4") == 0) {
                usr_csi_config->sc_config.sc_baudrate = atoi(str_baud_rate);
                usr_csi_config->sc_config.sc_rx_pin = atoi(str_rx);
                usr_csi_config->sc_config.sc_tx_pin = atoi(str_tx); 
            }
            //Check if SD will be used according to informer mode
            if (strcmp(str_informer_mode, "2") == 0 || strcmp(str_informer_mode, "4") == 0) {
                usr_csi_config->sd_config.sd_clk_pin = atoi(str_clk);
                usr_csi_config->sd_config.sd_cs_pin = atoi(str_cs);
                usr_csi_config->sd_config.sd_miso_pin = atoi(str_miso);
                usr_csi_config->sd_config.sd_mosi_pin = atoi(str_mosi); 
            }
        }
        //Send message to Wi-Fi app with new CSI and Wi-Fi configuration
        wifi_app_message_t n_msg = {
            .msgID = MSG_CHANGE_TO_CSI_COLLECTION, 
            .data = usr_csi_config
        };
        ESP_LOGI(TAG, "Debug informer mode %d", usr_csi_config->informer_mode);
        nvs_save_csi_configuration(usr_csi_config);
        wifi_app_send_message(n_msg); 
        free(str_op_mode);
        http_stop_server(); 
    } else{
        ESP_LOGE(TAG, "Error receiving CSI configuration from HTTP server");
        return ESP_FAIL; 
    }
    return ESP_OK;
}

/*
 * Function to start the http server, registering the handlers for requests
 */
static void start_webserver(){
    //Default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); 
    //Configure default values
    config.max_uri_handlers = 10; //CHANGE IF REQUIRED
    config.recv_wait_timeout = 10; 
    config.send_wait_timeout = 10; 

    //Start the httpd server and register the uri handlers
    if(httpd_start(&http_webserver, &config) == ESP_OK){

        //index.html handler
        httpd_uri_t index_html_uri = {
            .uri = "/",
            .method = HTTP_GET, 
            .handler = index_html_uri_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_webserver, &index_html_uri); 

        //app.js handler
        httpd_uri_t app_js_uri = {
            .uri = "/app.js",
            .method = HTTP_GET, 
            .handler = app_js_uri_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_webserver, &app_js_uri); 

        //jquery handler
        httpd_uri_t jquery_uri = { 
            .uri = "/jquery-3.3.1.min.js",
            .method = HTTP_GET, 
            .handler = jquery_uri_handler,
            .user_ctx = NULL 
        };
        httpd_register_uri_handler(http_webserver, &jquery_uri); 

        //favicon handler
        httpd_uri_t favicon_ico_uri = {
            .uri = "/favicon.ico",
            .method = HTTP_GET, 
            .handler = favicon_ico_uri_handler, 
            .user_ctx = NULL 
        };
        httpd_register_uri_handler(http_webserver, &favicon_ico_uri);

        //esp32 image handler
        httpd_uri_t image_uri = {
            .uri="/esp32.png",
            .method = HTTP_GET, 
            .handler = esp32_image_uri_handler, 
            .user_ctx = NULL 
        };
        httpd_register_uri_handler(http_webserver, &image_uri);

        //bootstrap handler
        httpd_uri_t bootstrap_uri = {
            .uri = "/bootstrap.min.css",
            .method = HTTP_GET, 
            .handler = bootstrap_uri_handler, 
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_webserver, &bootstrap_uri);

        //CSI configuration handler
        httpd_uri_t csi_configuration_json_uri = {
            .uri = "/csiConfiguration.json",
            .method = HTTP_POST, 
            .handler = csi_configuration_json_uri_handler, 
            .user_ctx = NULL 
        };
        httpd_register_uri_handler(http_webserver, &csi_configuration_json_uri);
    }
}

void http_start_server(void){
    //HTTP server is used for modifying the CSI configuration
    start_webserver();
}

void http_stop_server(void){
    if(http_webserver != NULL)
        httpd_stop(http_webserver);
}