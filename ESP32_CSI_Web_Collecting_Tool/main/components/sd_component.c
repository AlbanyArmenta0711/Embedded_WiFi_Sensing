/*
 * SD source file that handles SD operations
 * Created on: 02/27/2024
 * by: Jesus A. Armenta-Garcia
 */
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <dirent.h>
#include "esp_log.h"

#include "sd_component.h"

static const char * TAG = "SD Component"; 

sdmmc_host_t host; 
sdmmc_card_t * card; 

//Function that iterate over the base mount directory to determine the next CSI filename based on the number of files
static int get_csi_file_count() { 
    DIR * dir = opendir(MOUNT_POINT);
    struct dirent * file_entry; 
    int file_count = 0; 

    while ((file_entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "Found file %s", file_entry->d_name);
        file_count++;
    }
    closedir(dir);
    return file_count; 
}


const char * get_csi_filename() {  
    static char filename[100]; 
    char file_number[10];
    strcpy(filename, BASE_NAME);
    sprintf(file_number, "%d", get_csi_file_count());
    strcat(filename,file_number);
    return filename;
}

void  write_csi_into_sd(FILE * fd, char * data) {
    fprintf(fd, data);
}

void write_bin_data(FILE * fd, void * buff, ssize_t buff_len) {
    fwrite(buff, buff_len, 1, fd); 
}

FILE * open_binary_file (char * path) {
    static FILE * fd = NULL; 
    fd = fopen(path, "ab");
    if (fd == NULL) {
        ESP_LOGE(TAG, "Error opening file: %s", path); 
        return NULL; 
    }
    return fd; 
}

FILE * open_file(char * path) {
    static FILE * fd = NULL; 
    fd = fopen(path, "a");
    if (fd == NULL) {
        ESP_LOGE(TAG, "Error opening file: %s", path); 
        return NULL; 
    }
    return fd; 
}

void close_file(FILE * fd){
    if (fclose(fd) != 0) {
        ESP_LOGE(TAG, "Error closing file");
    }
}

void unmount_sd_storage() {
    esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
    ESP_LOGI(TAG, "SD card unmounted"); 

    //deinitialize the bus
    spi_bus_free(host.slot);
}

esp_err_t init_sd_storage(sd_config_t sd_config) {
    esp_err_t err; 
    //ERROR WHEN CALLING FUNCTION FOR SETTING DEFAULT VALUES
    /*sdmmc_host_t n_host = {
        .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG, 
        .slot = SDSPI_DEFAULT_HOST, 
        .max_freq_khz = SDMMC_FREQ_DEFAULT, 
        .io_voltage = 3.3f, 
        .init = &sdspi_host_init, 
        .set_bus_width = NULL, 
        .get_bus_width = NULL, 
        .set_bus_ddr_mode = NULL, 
        .set_card_clk = &sdspi_host_set_card_clk, 
        .set_cclk_always_on = NULL, 
        .do_transaction = &sdspi_host_do_transaction, 
        .deinit_p = &sdspi_host_remove_device, 
        .io_int_enable = &sdspi_host_io_int_enable, 
        .io_int_wait = &sdspi_host_io_int_wait, 
        .command_timeout_ms = 0, 
        .get_real_freq = &sdspi_host_get_real_freq,
        .command_timeout_ms = 3000
    };/*/
    sdmmc_host_t n_host = SDSPI_HOST_DEFAULT(); 
    host = n_host;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true, 
        .max_files = 5,
        .allocation_unit_size = 512 * 32 //Sector size = 512 bytes, 16kB allocation size
    };

    //SPI bus configuration
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = sd_config.sd_mosi_pin,
        .miso_io_num = sd_config.sd_miso_pin, 
        .sclk_io_num = sd_config.sd_clk_pin,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1, 
        .max_transfer_sz = 16384 //before 4000
    };

    err = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA); 
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return ESP_FAIL;
    }

    //Initialize the slot and mount filesystem
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT(); 
    slot_config.gpio_cs = sd_config.sd_cs_pin; 
    slot_config.host_id = host.slot; 
    ESP_LOGI(TAG, "Mounting filesystem...");
    err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK) {
        if (err == ESP_FAIL) {
            ESP_LOGE(TAG, "Fail to mount filesystem");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card (%s)", esp_err_to_name(err));
        }
        return ESP_FAIL; 
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    //print SD properties
    sdmmc_card_print_info(stdout, card);
    return ESP_OK; 
}