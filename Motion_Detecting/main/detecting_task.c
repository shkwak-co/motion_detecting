#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "detecting_task.h"
#include "buzzer_task.h"
#include "light_task.h"

const static char *TAG = "det_task";

void Sensor_Init(void);
esp_err_t send_http_request(const char *url);
void Sensor_Task(void* param);
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

// Sensor 초기화
void Sensor_Init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;       // Disable interrupt
    io_conf.mode = GPIO_MODE_INPUT;              // Set as input mode
    io_conf.pin_bit_mask = (1ULL << PIR_SENSOR_PIN);  // Bitmask for the selected pin
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     // Disable pull-up mode
    gpio_config(&io_conf);
}

esp_err_t send_http_request(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET request succeeded");
        buzzer_function();
        // light_function();
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

// Sensor 감지 작업
void Sensor_Task(void* param)
{
    ESP_LOGI(TAG, "Start detecting");

    while(1)
    {
        int motion_detected = gpio_get_level(PIR_SENSOR_PIN);
        if(motion_detected)
        {
            ESP_LOGE(TAG, "Motion Detected");
            send_http_request(SERVER_URL);  // 동작 감지 시 GET 요청 전송
        }
        else
        {
            // ESP_LOGI(TAG, "No Motion Detected");
        }
        vTaskDelay(pdMS_TO_TICKS(1000UL));  // 1초마다 감지
    }
}


// Wi-Fi 이벤트 핸들러
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        switch(event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "connection to AP");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "fail to connect");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected");
                break;
        }
    }
}
