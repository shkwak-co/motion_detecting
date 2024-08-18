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

#define PIR_SENSOR_PIN GPIO_NUM_27
#define SERVER_URL "http://192.168.4.1/detected"  // 서버의 IP 주소와 경로를 설정하세요.

const static char *TAG = "Sensor Test";

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

// HTTP 클라이언트 요청을 보내는 함수
esp_err_t send_http_request(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET request succeeded");
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
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
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

void app_main(void)
{
  //nvs 설정
    esp_err_t ret = nvs_flash_init ();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK (nvs_flash_erase ());
        ret = nvs_flash_init ();
    }
    ESP_ERROR_CHECK (ret);

  // event loop 설정
    ESP_ERROR_CHECK (esp_event_loop_create_default ());

  // 네트워크 인터페이스 설정
    ESP_ERROR_CHECK (esp_netif_init ());

    // Wi-Fi STA 설정
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    wifi_config_t wifi_sta_config = { };

    strcpy ((char *) wifi_sta_config.sta.ssid, "ESP32_Detect");
    strcpy ((char *) wifi_sta_config.sta.password, "12345678");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Wi-Fi 이벤트 핸들러 등록


    // PIR 센서 초기화
    Sensor_Init();
    ESP_LOGI(TAG, "Sensor ready");

    // Sensor_Task를 생성하여 PIR 센서를 감지합니다.
    xTaskCreate(Sensor_Task, "motion_sensor_task", 4096, NULL, 5, NULL);

    // 메인 루프
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000UL)); // 1초 대기
    }
}
