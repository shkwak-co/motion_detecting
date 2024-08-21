#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "buzzer_task.h"


void buzzer_function(void)
{
    // BUZZER_PIN을 출력 모드로 설정
    // gpio_pad_select_gpio(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000UL));  // 1초 대기

        // 부저 끄기
    gpio_set_level(BUZZER_PIN, 0);
}
