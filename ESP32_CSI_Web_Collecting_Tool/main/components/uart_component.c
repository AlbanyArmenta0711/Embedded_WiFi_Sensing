#include "driver/gpio.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "../common/utils.h"
#include "uart_component.h"

static const char * TAG = "UART Component";

uart_config_t current_uart_config; 
serial_config_t sc_config; 

esp_err_t change_baud_rate(uint32_t baudrate) {
    return uart_set_baudrate(DEFAULT_UART_PORT, baudrate);
}

uart_config_t get_default_uart_configuration() {
    uart_config_t config = {
        .baud_rate = DEFAULT_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS, 
        .parity = UART_PARITY_DISABLE, 
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, 
        .rx_flow_ctrl_thresh = 122,
    };
    return config; 
}

esp_err_t init_uart_component(serial_config_t serial_config, uart_config_t config) {
    esp_err_t err; 
    config.baud_rate = serial_config.sc_baudrate;
    //Configure UART parameters
    err = uart_param_config(DEFAULT_UART_PORT, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting parameters");
        return ESP_FAIL; 
    } else {
        //Install UART drivers
        err = uart_driver_install(DEFAULT_UART_PORT, DEFAULT_BUFF_SIZE, 0, 0, NULL, 0); 
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error installing UART driver");
            return ESP_FAIL; 
        } else {
            err = uart_set_pin(DEFAULT_UART_PORT, serial_config.sc_tx_pin, serial_config.sc_rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error setting pins for UART communication");
                return ESP_FAIL; 
            }
        }
    }
    ESP_LOGI(TAG, "UART configured");
    return ESP_OK; 
}