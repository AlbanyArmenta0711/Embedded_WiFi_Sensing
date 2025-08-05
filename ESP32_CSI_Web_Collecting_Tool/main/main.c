#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "./tasks/wifi_app.h"
#include "./components/reset_button.h"

void app_main(void)
{
    //Initialie NVS
    esp_err_t ret = nvs_flash_init(); 
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    //Start Wi-Fi task
    wifi_app_start();
    
    //Start the button task
    reset_button_config(); 
}