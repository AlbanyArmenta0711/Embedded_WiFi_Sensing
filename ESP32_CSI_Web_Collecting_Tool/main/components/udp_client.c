#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "udp_client.h"

static const char * TAG = "UDP client";

static udp_dest_config_t dest_config;
static int socket_fd; 

int send_to_socket(const void * msg, size_t length){
    int err = 0;
    struct sockaddr_in dest_addr; 
    dest_addr.sin_addr.s_addr = inet_addr(UDP_CLIENT_DEFAULT_HOST_IP);
    dest_addr.sin_family = UDP_CLIENT_DEFAULT_SIN_FAMILY; 
    dest_addr.sin_port = htons(UDP_CLIENT_DEFAULT_SIN_PORT);
    err = sendto(socket_fd, UDP_CLIENT_DEFAULT_PAYLOAD, strlen(UDP_CLIENT_DEFAULT_PAYLOAD), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err > 0 || errno != ENOMEM) {
        ESP_LOGW(TAG, "enomem found");
        return err; 
    } else {
        vTaskDelay(1);
    }
    return err; 
}

void set_dest_socket_config(udp_dest_config_t config){
    dest_config = config; 
}

//Function to set socket options
static int set_socket_options() {
    int err; 
    //Timeout is configured to 10 seconds
    struct timeval timeout = {
        .tv_sec = 10,
        .tv_usec = 0
    };
    int buff_size = 8192;
    err = setsockopt(socket_fd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(int));
    err = setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    return err;
}

int destroy_socket() {
    int err = 0;
    err = close(socket_fd);
    if (err == 0) {
        ESP_LOGI(TAG, "socket closed");
    } else {
        ESP_LOGW(TAG, "error closing the socket");
        err = -1; 
    }
    return err; 
}

int create_socket(int8_t domain, int8_t type, int8_t protocol){
    int opt_err; 
    socket_fd = socket(domain, type, protocol);
    if (socket_fd < 0) {
        ESP_LOGE(TAG, "Error creating socket"); 
    } else {
        opt_err = set_socket_options();
        if (opt_err == 0){
            ESP_LOGI(TAG, "Socket created");
        } else {
            return -1; 
        }
    }
    return socket_fd;
}