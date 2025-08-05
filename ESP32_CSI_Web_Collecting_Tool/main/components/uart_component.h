/*
 * Header file for handling UART communication (Serial Communication)
 * Functions in this file follows the programming workflow proposed by
 * idf.
 * Created on: 02/28/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef UART_COMPONENT_H
#define UART_COMPONENT_H

#include "driver/uart.h"

#define DEFAULT_BAUD_RATE 230400
#define DEFAULT_UART_PORT UART_NUM_1
#define DEFAULT_BUFF_SIZE 16384
#define DEFAULT_RX_PIN 5
#define DEFAULT_TX_PIN 4 

/*
 * Function to change baud rate 
 * @param baudrate: UART baudrate to set
 * @return ESP_OK on Success - ESP_FAIL on error
 */
esp_err_t change_baud_rate(uint32_t baudrate);

/*
 * Function to get UART default communication parameters
 * @return a uart_config_t structure with default configuration
 */
uart_config_t get_default_uart_configuration();

/*
 * Function to install UART driver and set configuration
 * @param serial_config: Serial Communication configuration
 * @param uart_config: UART configuration to be set
 * @return ESP_OK if UART was initialized succesfully, ESP_FAIL otherwise
 */
esp_err_t init_uart_component(serial_config_t serial_config, uart_config_t uart_config); 

#endif