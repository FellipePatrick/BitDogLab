#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

const uint LED_PIN = 12;
const uint JOYSTICK_X = 26;  // ADC0
const uint JOYSTICK_Y = 27;  // ADC1

// === Mutex global para ADC ===
SemaphoreHandle_t xAdcMutex;

// === Tarefa 1: Piscar LED ===
void vBlinkTask(void *pvParameters) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(50));  // Aceso 50ms

        gpio_put(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(950));  // Apagado 950ms
    }
}

// === Tarefa 2: Leitura da Temperatura ===
void vTempTask(void *pvParameters) {
    adc_init();
    adc_set_temp_sensor_enabled(true);  // Habilita sensor de temperatura

    while (1) {
        if (xSemaphoreTake(xAdcMutex, portMAX_DELAY) == pdTRUE) {
            adc_select_input(4);  // Canal 4 = temperatura interna
            uint16_t adc_raw = adc_read();
            xSemaphoreGive(xAdcMutex);

            float voltage = adc_raw * 3.3f / (1 << 12);  // 12-bit ADC => 4096
            float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;

            printf("Temperatura interna: %.2f C\n", temperature);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// === Tarefa 3: Leitura do joystick analógico ===
void vJoystickTask(void *pvParameters) {
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    const int16_t CENTER = 2048;   // Meio da faixa ADC 12-bit (0-4095)
    const int16_t THRESHOLD = 800; // Limiar para detectar movimento

    char *last_direction = "Centro";

    while (1) {
        uint16_t raw_x = 0;
        uint16_t raw_y = 0;

        if (xSemaphoreTake(xAdcMutex, portMAX_DELAY) == pdTRUE) {
            adc_select_input(0);  // ADC0 = GPIO26
            raw_x = adc_read();

            adc_select_input(1);  // ADC1 = GPIO27
            raw_y = adc_read();

            xSemaphoreGive(xAdcMutex);
        }

        if (raw_y < CENTER - THRESHOLD) {
            last_direction = "Esquerda";
        } else if (raw_y > CENTER + THRESHOLD) {
            last_direction = "Baixo";
        } else if (raw_x < CENTER - THRESHOLD) {
            last_direction = "Direita";
        } else if (raw_x > CENTER + THRESHOLD) {
            last_direction = "Cima";
        } else {
            last_direction = "Centro";
        }

        printf("Joystick: %s (X=%d, Y=%d)\n", last_direction, raw_x, raw_y);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main() {
    stdio_init_all();

    // Cria mutex do ADC
    xAdcMutex = xSemaphoreCreateMutex();

    xTaskCreate(vBlinkTask, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTempTask, "Temperature Task", 256, NULL, 1, NULL);
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
    return 0;
}
