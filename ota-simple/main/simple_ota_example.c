/* OTA example with button trigger, blink LED, and serial message

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"

#include "protocol_examples_common.h"
#include "driver/gpio.h"

#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
#include "esp_crt_bundle.h"
#endif

#if CONFIG_EXAMPLE_CONNECT_WIFI
#include "esp_wifi.h"
#endif

// Define the GPIO pin for the button and LED
#define GPIO_BUTTON_PIN 0  // Button pin, GPIO0
#define GPIO_LED_PIN 33     // LED pin, GPIO33

static const char *TAG = "simple_ota_example";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");


/* Event handler for catching system events */
static void _https_ota_event_handler(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data)
{
    if (event_base == ESP_HTTPS_OTA_EVENT) {
        switch (event_id) {
            case ESP_HTTPS_OTA_START:
                ESP_LOGI(TAG, "OTA started");
                break;
            case ESP_HTTPS_OTA_CONNECTED:
                ESP_LOGI(TAG, "Connected to server");
                break;
            case ESP_HTTPS_OTA_GET_IMG_DESC:
                ESP_LOGI(TAG, "Reading Image Description");
                break;
            case ESP_HTTPS_OTA_VERIFY_CHIP_ID:
                ESP_LOGI(TAG, "Verifying chip id of new image: %d", *(esp_chip_id_t *)event_data);
                break;
            case ESP_HTTPS_OTA_DECRYPT_CB:
                ESP_LOGI(TAG, "Callback to decrypt function");
                break;
            case ESP_HTTPS_OTA_WRITE_FLASH:
                ESP_LOGD(TAG, "Writing to flash: %d written", *(int *)event_data);
                break;
            case ESP_HTTPS_OTA_UPDATE_BOOT_PARTITION:
                ESP_LOGI(TAG, "Boot partition updated. Next Partition: %d", *(esp_partition_subtype_t *)event_data);
                break;
            case ESP_HTTPS_OTA_FINISH:
                ESP_LOGI(TAG, "OTA finish");
                break;
            case ESP_HTTPS_OTA_ABORT:
                ESP_LOGI(TAG, "OTA abort");
                break;
        }
    }
}

// Initialize the button GPIO pin
void init_button(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,  // No interrupt on the pin
        .mode = GPIO_MODE_INPUT,         // Set the pin as input
        .pin_bit_mask = (1ULL << GPIO_BUTTON_PIN), // Set the pin mask for the button
        .pull_up_en = 1                  // Enable pull-up resistor
    };
    gpio_config(&io_conf);
}

// Initialize the LED GPIO pin
void init_led(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,  // No interrupt on the pin
        .mode = GPIO_MODE_OUTPUT,        // Set the pin as output for the LED
        .pin_bit_mask = (1ULL << GPIO_LED_PIN), // Set the pin mask for the LED
        .pull_down_en = 0,               // Disable pull-down resistor
        .pull_up_en = 0                  // Disable pull-up resistor
    };
    gpio_config(&io_conf);
}

// OTA task: This task handles the OTA update process when the button is pressed
void simple_ota_example_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting OTA task");

    // Wait until the button is pressed
    ESP_LOGI(TAG, "Waiting for the button to be pressed...");
    while (gpio_get_level(GPIO_BUTTON_PIN) != 0) {
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay for 100 ms between checks
    }

    ESP_LOGI(TAG, "Button pressed, starting OTA...");

    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL,
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach,
#else
        .cert_pem = (char *)server_cert_pem_start,
#endif
        .event_handler = _https_ota_event_handler,
        .keep_alive_enable = true,

};

    esp_https_ota_config_t ota_config = {
        .bulk_flash_erase = true,
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Attempting to download firmware from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA succeeded, rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA firmware upgrade failed");
    }

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Task to blink the LED and print a message every 1 second
void blink_led_task(void *pvParameter)
{
    while (1) {
        // Toggle the LED state
        gpio_set_level(GPIO_LED_PIN, 1);  // Turn LED ON
        ESP_LOGI(TAG, "LED [%d] ON and message printed...",GPIO_LED_PIN);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second

        gpio_set_level(GPIO_LED_PIN, 0);  // Turn LED OFF
        ESP_LOGI(TAG, "LED OFF [%d] and message printed...",GPIO_LED_PIN);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
    }
}

// Main function (app_main): Initializes components and creates tasks
void app_main(void)
{
    ESP_LOGI(TAG, "Initializing OTA example");

    // Initialize Non-Volatile Storage (NVS)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize button and LED
    init_button();
    init_led();

    ESP_ERROR_CHECK(esp_netif_init());  // Initialize network interfaces
    ESP_ERROR_CHECK(esp_event_loop_create_default());  // Create default event loop
    ESP_ERROR_CHECK(example_connect());  // Establish network connection

#if CONFIG_EXAMPLE_CONNECT_WIFI
    esp_wifi_set_ps(WIFI_PS_NONE);  // Disable Wi-Fi power save mode for better OTA performance
#endif

    // Create OTA task
    xTaskCreate(&simple_ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
    
    // Create LED blink task
    xTaskCreate(&blink_led_task, "blink_led_task", 2048, NULL, 5, NULL);
}
