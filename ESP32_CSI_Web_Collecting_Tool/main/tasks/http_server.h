/*
 * Header file that includes constants and function prototypes for the HTTP server
 * Created on: 02/08/2024
 * by: Jesus A. Armenta-Garcia
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdbool.h> 

#define HEADER_OP_MODE "OpModes"
#define HEADER_MAC_ADDRESS "MacAddress"
#define HEADER_COMMUNICATION_CHANNEL "Communication"
#define HEADER_FREQUENCY "Frequency"
#define HEADER_BANDWIDTH "Bandwidth"
#define HEADER_DATA_REPRESENTATION "DataRepresentation"
#define HEADER_IS_MAC_ADDRESS "IsMacAddress"
#define HEADER_IS_CSI_BUFF "IsCSIBuff"
#define HEADER_IS_RSSI "IsRSSI"
#define HEADER_IS_CHANNEL_BANDWIDTH "IsChannelBandwidth"
#define HEADER_IS_NOISE_FLOOR "IsNoiseFloor"
#define HEADER_IS_TIMESTAMP "IsTimestamp"
#define HEADER_IS_ANTENNA_NUMBER "IsAntennaNumber"
#define HEADER_IS_CSI_DATA_LENGTH "IsCSIDataLength"
#define HEADER_INFORMER_MODE "InformerMode"
#define HEADER_MISO "MISO"
#define HEADER_MOSI "MOSI"
#define HEADER_CLK "CLK"
#define HEADER_CS "CS"
#define HEADER_BAUDRATE "Baudrate"
#define HEADER_TX "TX"
#define HEADER_RX "RX"

void http_start_server(void); 

void http_stop_server(void); 

#endif