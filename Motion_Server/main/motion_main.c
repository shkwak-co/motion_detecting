#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motion_server.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <nvs_flash.h>
#include "esp_netif.h"

void net_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void app_main(void)
{
    net_init();

    /**
     * #TODO
     * MODBUS - Master
     * 
     */
    xTaskCreate(Server_task, "server", 1024*10, NULL, 5, NULL);
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000UL));
    }
}