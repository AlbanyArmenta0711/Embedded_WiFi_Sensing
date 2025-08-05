/*
 * Header file that includes constants and function prototypes for the udp client
 * Created on: 04/17/2024
 * by: Jesus A. Armenta-Garcia
 */
#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include "lwip/sockets.h"
#include "../tasks/wifi_app.h"

#define UDP_CLIENT_DEFAULT_PAYLOAD "1"
#define UDP_CLIENT_DEFAULT_HOST_IP WIFI_APP_CSI_COLLECTION_IP_ADDRESS
#define UDP_CLIENT_DEFAULT_SIN_FAMILY AF_INET 
#define UDP_CLIENT_DEFAULT_SIN_PORT 2222
#define UDP_CLIENT_DEFAULT_ADDR_FAMILY AF_INET
#define UDP_CLIENT_DEFAULT_IP_PROTOCOL IPPROTO_IP

typedef struct udp_dest_config {
    char * dest_ip; 
    uint8_t sin_family; 
    uint16_t sin_port; 
    uint8_t addr_family; 
    uint8_t ip_protocol;
} udp_dest_config_t;

/*
 * Function to set default destination configuration
 * @param config: destination configuration
 */
void set_dest_socket_config(udp_dest_config_t config);

/*
 * Function to send a message on a socket. Destination socket configuration must be set first and socket must be created
 * @param msg: pointer to the buffer containing the message to be sent
 * @param length: size of the message in bytes
 * @return the number of bytes sent. Otherwise, -1 if error
 */
int send_to_socket(const void * msg, size_t length); 

/*
 * Function to destroy the socket and close the file descriptor associated to the socket
 * @return 0 on success, -1 otherwise.
 */
int destroy_socket(); 

/*
 * Function to create an endpoint for communication (socket) given a following configuration
 * @param addr_family: domain in which a socket is to be created (address family)
 * @param type: type of socket to be created
 * @param protocol: particular protocol to be used with the socket
 * @return if successful, a nonnegative integer which is the socket file descriptor
 */
int create_socket(int8_t domain, int8_t type, int8_t protocol);

#endif