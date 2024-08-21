#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "light_task.h"


void light_function(void)
{
    // LED_PIN을 출력 모드로 설정

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

        // LED 켜기
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000UL));  // 1초 대기

        // LED 끄기
    gpio_set_level(LED_PIN, 0);
}
