/*
 * Header file that includes constants and function prototypes for saving and loading CSI configuration
 * defined in the HTTP server in the NVS
 * Created on: 02/13/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include "../common/utils.h"

/*
 * Save CSI configuration in the NVS
 * @param csi_configuration, CSI configuration received from the HTTP server
 * @return ESP_OK if successful
 */
esp_err_t nvs_save_csi_configuration(user_csi_configuration_t * csi_configuration);

/*
 * Loads the CSI configuration stored in the NVS
 * @return true if conf file is found
 */
user_csi_configuration_t * nvs_load_csi_configuration(void); 

/*
 * Clears the CSI configuration file from the NVS
 * @return ESP_OK if successful 
 */
esp_err_t nvs_clear_configuration(void);

#endif