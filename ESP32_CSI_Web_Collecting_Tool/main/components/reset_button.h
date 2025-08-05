/*
 * Header file that includes constants and function prototypes for the Reset Button task
 * Created on: 02/14/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef RESET_BUTTON_H
#define RESET_BUTTON_H

/*
 * Constants for the reset button task
 */
#define RESET_BUTTON_TASK_PRIORITY 6 //Top priority
#define RESET_BUTTON_TASK_STACK_SIZE 2048

//Interrupt flag
#define ESP_INTR_FLAG_DEFAULT 0

//Wi-Fi reset button GPIO on the DevKit
#define WIFI_RESET_BUTTON 0 

/*
 * Function that configures the reset button and the interruption
 */
void reset_button_config(void);

#endif