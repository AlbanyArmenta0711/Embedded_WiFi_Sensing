# ESP32 CSI Web Collecting Tool

Tool that leverages two ESP32 for collecting Wi-Fi CSI. Its parameters can be configured easily by an integrated webform that can be accessed through any browser.

It allows sending CSI data through its USB serial port, through GPIO pins by UART communication and SD storaging. 

## Content

The project inside main directory is structured as follows: 

```
├── main
│   ├── common          //Contains header files that define C structures and constants that are used all along the project.
│   │     └── utils.h
│   ├── components      //Header and source files for defining and configuring device components for communication and storing
│   │     ├── nvs_storage.c
│   │     ├── nvs_storage.h
│   │     ├── reset_button.c
│   │     ├── reset_button.h
│   │     ├── sd_component.c
│   │     ├── sd_component.h
│   │     ├── uart_component.c
│   │     ├── uart_component.h
│   │     ├── udp_client.c
│   │     └── udp_client.h
│   ├── tasks       //Header and source files that define the tasks running with FreeRTOS for tool functionality
│   │     ├── csi_app.c
│   │     ├── csi_app.h
│   │     ├── http_server.c
│   │     ├── http_server.h
│   │     ├── informer.c
│   │     ├── informer.h
│   │     ├── wifi_app.c
│   │     └── wifi_app.h
│   ├── webpage     //Files for the web page for tool configuration
│   │     ├── app.js
│   │     ├── bootstrap.min.css
│   │     ├── esp32.png
│   │     ├── favicon.ico
│   │     ├── index.html
│   │     └── jquery-3.3.1.min.js
│   ├── CMakeLists.txt
│   └── main.c
├── CMakeLists.txt
├── partitions.csv
└── README.md
```

## Components 

- **nvs_storage:** 
- **reset_button:**
- **sd_component:**
- **uart_component:** 
- **udp_client:** 

## Tasks
- **csi_app:** 
- **http_server:**
    The HTTP Server registers Uniform Resource Identifiers (URIs) for fetching resources, such as the web page containing the web form or executing JavaScript functions to handle HTTP requests. One of the main URIs of the server is the one that handles a POST request, i.e., sends data to the server, which triggers when the Submit button of the web form is pushed. This request is sent to the server, i.e., the ESP32 working as an AP, a JSON file, which is a text value file that stores data in pairs of key and value, containing the tool configuration for collecting CSI. The server then notifies the Wi-Fi task to start CSI collection, setting the device operation mode to Rx or Tx. 

    To provide an in-depth understanding of the interactions between the Wi-Fi task and the HTTP Server, the following figure illustrates a sequence diagram that describes how these tasks interact with each other through function calls, return values, and message passing using task communication queues. 

    <img src="./images/sequence_diagram_setup.png" alt="HTTP server sequence diagram" width="940" height="760">

- **informer:** 
- **wifi_app:** 
    The Wi-Fi Task is responsible for starting every other tool task when needed. At device startup, this task checks if tool       configuration data has been created when the web form is submitted in the NVS. If configuration data can not be found, the device will start as an  AP with network parameters shown in the following table: 

    | Network Parameter | Value                |
    |-------------------|----------------------|
    | SSID              | CSI_Collecting_ESP32 |
    | Password          | root1234             |
    | Wi-Fi Channel     | 1                    |
    | Bandwidth         | 20MHz                |
    | IP address        | 192.168.1.1          |

    Furthermore, when configuring the tool, the Wi-Fi Task starts the HTTP Server to handle HTTP methods related to the web form. 
    However, if configuration data is found, the device will load the configuration defined previously through the web form. It will change to the respective operation mode for CSI collection, initiating the CSI Task. 
