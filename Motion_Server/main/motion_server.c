#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "motion_server.h"


static const char *TAG = "Server";
static int count = 0;

/* An HTTP GET handler */
static esp_err_t detected_get_handler(httpd_req_t *req)
{
    count++;
    ESP_LOGI(TAG, "detected %d times", count);

    const char* resp_str = "response";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t detect = {
    .uri       = "/detected",
    .method    = HTTP_GET,
    .handler   = detected_get_handler,
};
static esp_err_t count_get_handler(httpd_req_t *req)
{
    char resp[32];
    sprintf(resp, "detected %d times", count);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t counting = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = count_get_handler,
};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &detect);
        httpd_register_uri_handler(server, &counting);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

// Wi-Fi 이벤트 핸들러
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "STA connected");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "STA disconnected");
    }
}

void wifi_init()
{
    esp_err_t err = ESP_FAIL;

    // Create the default AP network interface
    esp_netif_create_default_wifi_ap(); 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_ap_config ={
        .ap= {
            .ssid = "ESP32_Detect",
            .channel = 1,
            .password = "12345678",
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg ={
                .required = true,
            }
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    err = esp_wifi_start();
    if(err == ESP_OK)
        ESP_LOGI(TAG, "WIFI_MODE_AP started. SSID:%s password:%s", "ESP32_Detect", "12345678");
    else
        ESP_LOGW(TAG, "Fail to open AP");

    // Register event handler for Wi-Fi events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
}

void Server_task(void *param)
{
    wifi_init();

    start_webserver();

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip_info));
    ESP_LOGI(TAG, "AP IP Address: " IPSTR, IP2STR(&ip_info.ip));

    while(1)
    {
        /**
         * 감지했을 때 task가 할 동작
         */
        vTaskDelay(pdMS_TO_TICKS(1000UL));
    }
}
