/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "blink.h"

static const char *TAG = "blink_task";

//#define BLINK_GPIO CONFIG_BLINK_LED_GPIO
#define BLINK_GPIO 21

static uint8_t s_led_state = 0;

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

static void blink_task(void *pvParameters)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    while (1) {
        ESP_LOGI(TAG, "Turning the LED[%.2d] %s!", BLINK_GPIO, s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

esp_err_t start_blink(void)
{
    if (xTaskCreate(blink_task, TAG, CONFIG_BLINK_TASK_STACK_SIZE, NULL,
                    CONFIG_BLINK_TASK_PRIORITY, NULL) != pdPASS)
        return ESP_FAIL;
    return ESP_OK;
}
