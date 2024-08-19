#include <stdio.h>
#include <string.h>
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

const static char *TAG = "Sensor Test";

void net_init()
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
}

void app_main(void)
{
    net_init();

    // PIR 센서 초기화
    Sensor_Init();
    ESP_LOGI(TAG, "Sensor ready");

    // Sensor_Task를 생성하여 PIR 센서를 감지합니다.
    xTaskCreate(Sensor_Task, "motion_sensor_task", 4096, NULL, 5, NULL);

    // 메인 루프
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000UL));
    }
}
