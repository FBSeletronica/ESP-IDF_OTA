idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES blink delta_ota nvs_flash esp_wifi console)
