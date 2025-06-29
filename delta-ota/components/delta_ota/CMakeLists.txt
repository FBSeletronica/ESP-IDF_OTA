idf_component_register(SRCS "delta_ota.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES mbedtls esp_driver_gpio esp_http_client esp_partition app_update esp_timer)
