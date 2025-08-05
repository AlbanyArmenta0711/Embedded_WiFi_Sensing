
#ifndef SD_COMPONENT_H
#define SD_COMPONENT_H

#include "../common/utils.h"

#define SD_PIN_CLK 19 
#define SD_PIN_MOSI 22
#define SD_PIN_MISO 23
#define SD_PIN_CS 18

#define MOUNT_POINT "/sdcard"
#define BASE_NAME MOUNT_POINT"/csi_data_"

/* 
 * Function to get the name a new CSI file where info will be written
 * @return string path of the file where CSI data will be saved
 */
const char * get_csi_filename();

/*
 * Function to open (or create) a CSI file given a specified path
 * init_sd_storage must be called first
 * @param path: file path in SD storage
 * @return a file descriptor to the open file
 */
FILE * open_file(char * path); 

/*
 * Function to open (or create) a CSI binary file given a specified path
 * init_sd_storage must be called first
 * @param path: file path in SD storage
 * @return a file descriptor to the open file
 */
FILE * open_binary_file (char * path);

/*
 * Function to close a file given a file pointer
 * @param fd: file pointer to open file
 */
void close_file(FILE * fd);

/*
 * Function that writes to SD a CSI measurement
 * @param fd: file descriptor
 * @param data: data to write into the file
 */
void write_csi_into_sd(FILE * fd, char * data);

/*
 * Function to write bin data in SD file
 * @param fd: file descriptor
 * @param buff: data buff to store in file
 * @param buff_len: size of buffer to store in file
 */
void write_bin_data(FILE * fd, void * buff, ssize_t buff_len);

/*
 * Function to unmount the SD card and free the SPI bus
 */
void unmount_sd_storage(); 

/*
 * Function to initalize SD card and mount the filesystem
 * @param sd_config: SD configuration from configuration received from HTTP server or NVS
 * @return ESP_OK if UART was initialized succesfully, ESP_FAIL otherwise
 */
esp_err_t init_sd_storage(sd_config_t sd_config);

#endif 