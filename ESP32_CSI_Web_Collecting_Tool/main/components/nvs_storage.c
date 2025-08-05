#include "nvs_flash.h"
#include "esp_log.h"

#include "nvs_storage.h"

static const char * TAG = "NVS Storage"; 
static const char * NAMESPACE_KEY = "csi_conf";

//NVS namespace
const char * nvs_storage_namespace = "csi_conf";

esp_err_t nvs_save_csi_configuration (user_csi_configuration_t * csi_configuration) {
    nvs_handle handler;
    esp_err_t esp_err; 
    ESP_LOGI(TAG, "Saving CSI configuration in NVS, op_mode: %d", csi_configuration->op_mode);
    
    //Open NVS with given namespace
    esp_err = nvs_open(nvs_storage_namespace, NVS_READWRITE, &handler);
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS with namespace %s", esp_err_to_name(esp_err), nvs_storage_namespace);
        return esp_err; 
    } else {
        //Store csi_configuration in NVS
        esp_err = nvs_set_blob(handler, NAMESPACE_KEY, csi_configuration, sizeof(user_csi_configuration_t)); 
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) storing csi_configuration struct", esp_err_to_name(esp_err));
            return esp_err; 
        } else {
            //Commit 
            esp_err = nvs_commit(handler);
            if (esp_err != ESP_OK) {
                ESP_LOGE(TAG, "Error (%s) comitting changes", esp_err_to_name(esp_err)); 
                return esp_err; 
            } 
        }
        nvs_close(handler); 
        ESP_LOGI(TAG, "Wrote CSI configuration into NVS");
    }
    return ESP_OK; 
}

user_csi_configuration_t * nvs_load_csi_configuration(void) {
    size_t usr_csi_config_size = sizeof(user_csi_configuration_t);
    user_csi_configuration_t * usr_csi_conf = NULL; 
    nvs_handle handler; 
    esp_err_t esp_err; 
    ESP_LOGI(TAG, "Loading CSI configuration from NVS");

    //Open NVS
    esp_err = nvs_open(nvs_storage_namespace, NVS_READONLY, &handler); 
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS with namespace %s", esp_err_to_name(esp_err), nvs_storage_namespace);
    } else {
        usr_csi_conf = (user_csi_configuration_t *) malloc(sizeof(user_csi_configuration_t)); 

        esp_err = nvs_get_blob(handler, NAMESPACE_KEY, usr_csi_conf, &usr_csi_config_size); 
        if (esp_err != ESP_OK) {
            ESP_LOGI(TAG, "Error (%s) recovering the CSI configuration from the blob", esp_err_to_name(esp_err)); 
            return NULL; 
        }
        ESP_LOGI(TAG, "CSI configuration recovered from NVS");
        nvs_close(handler);
    }

    return usr_csi_conf; 
}

esp_err_t nvs_clear_configuration(void) {
    nvs_handle handler; 
    esp_err_t esp_err; 
    ESP_LOGI(TAG, "Clearing CSI configuration from NVS");

    //Open NVS
    esp_err = nvs_open(nvs_storage_namespace, NVS_READWRITE, &handler); 
    if (esp_err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS with namespace %s", esp_err_to_name(esp_err), nvs_storage_namespace);
        return esp_err; 
    } else {
        //Erase data in NVS
        esp_err = nvs_erase_all(handler);
        
        if (esp_err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) erasing CSI configuration from NVS",esp_err_to_name(esp_err) );
            return esp_err; 
        } else {
            //Commit erasure
            esp_err = nvs_commit(handler);

            if (esp_err != ESP_OK) {
                ESP_LOGE(TAG, "Error (%s) in NVS erasure commit!", esp_err_to_name(esp_err));
                return esp_err; 
            }
            nvs_close(handler);
            ESP_LOGI(TAG, "NVS erased!");
        }
    }
    return ESP_OK; 
}