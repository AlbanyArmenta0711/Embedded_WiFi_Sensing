idf_component_register( 
    SRCS "components/udp_client.c" "components/nvs_storage.c" "tasks/http_server.c" "components/sd_component.c" "tasks/informer.c" "tasks/csi_app.c" "tasks/wifi_app.c" "main.c" "components/reset_button.c" "components/sd_component.c" "components/uart_component.c"
    INCLUDE_DIRS "."
    EMBED_FILES webpage/app.js webpage/favicon.ico webpage/index.html webpage/jquery-3.3.1.min.js webpage/bootstrap.min.css webpage/esp32.png
)