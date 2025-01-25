#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include <string.h>  

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12
#define BTN_A_PIN 5

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

int A_state = 0; 

//funcao para configurar o display
void ConfiguraDisplay() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
}

void RenderizaTextoDisplayQuebrado(char *text) {
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // inicializa o buffer do display com zeros
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    int line_length = 13; 
    int line = 0;         
    char line_buffer[line_length + 1];

    // quebra o texto em linhas
    for (int i = 0; i < strlen(text); i += line_length) {
        strncpy(line_buffer, text + i, line_length);
        line_buffer[line_length] = '\0';
        ssd1306_draw_string(ssd, 0, line * 8, line_buffer); 
        line++;
    }

    render_on_display(ssd, &frame_area);
}

//funcao para fechar o sinal
void SinalFechado() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    RenderizaTextoDisplayQuebrado("SINAL FECHADO AGUARDE");
}

//funcao para abrir o sinal
void SinalAberto() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
    RenderizaTextoDisplayQuebrado("SINAL ABERTO ATRAVESSAR COM CUIDADO");
}

int WaitWithRead(int timeMS) {
    for (int i = 0; i < timeMS; i += 100) {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1) {
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

int main() {
    stdio_init_all();

    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    ConfiguraDisplay();

    while (true) {
        SinalFechado();
        A_state = WaitWithRead(8000);

        if (A_state) {
            SinalAberto();
            sleep_ms(10000);
        }
    }

    return 0;
}
