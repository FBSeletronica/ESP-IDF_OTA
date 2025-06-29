/******************************************************************************
*
* This code is part of the course "Programe o ESP32 com ESP-IDF 5" by Fábio Souza
* The course is available on https://cursos.embarcados.com.br
*
* This example code Creative Commons Attribution 4.0 International License.
* When using the code, you must keep the above copyright notice,
* this list of conditions and the following disclaimer in the source code.
* (http://creativecommons.org/licenses/by/4.0/)

* Author: Vinicius Silva <silva.viniciusr@gmail.com>
* This code is for teaching purposes only.
* No warranty of any kind is provided.
*******************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"

#include "blink.h"
#include "delta_ota.h"

static const char *TAG = "main_app";

void app_main(void)
{

    ESP_LOGI(TAG, "Starting blink task...");
    ESP_ERROR_CHECK(start_blink());

    ESP_LOGI(TAG, "Initialising WiFi Connection...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    ESP_LOGI(TAG, "Starting Delta OTA task...");
    ESP_ERROR_CHECK(dota_init());

}
