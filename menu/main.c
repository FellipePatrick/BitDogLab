#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t {
 uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
* Inicializa a máquina PIO para controle da matriz de LEDs.
*/
void npInit(uint pin) {

 // Cria programa PIO.
 uint offset = pio_add_program(pio0, &ws2818b_program);
 np_pio = pio0;

 // Toma posse de uma máquina PIO.
 sm = pio_claim_unused_sm(np_pio, false);
 if (sm < 0) {
   np_pio = pio1;
   sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
 }

 // Inicia programa na máquina PIO obtida.
 ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

 // Limpa buffer de pixels.
 for (uint i = 0; i < LED_COUNT; ++i) {
   leds[i].R = 0;
   leds[i].G = 0;
   leds[i].B = 0;
 }
}

/**
* Atribui uma cor RGB a um LED.
*/
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
 leds[index].R = r;
 leds[index].G = g;
 leds[index].B = b;
}

/**
* Limpa o buffer de pixels.
*/
void npClear() {
 for (uint i = 0; i < LED_COUNT; ++i)
   npSetLED(i, 0, 0, 0);
}

/**
* Escreve os dados do buffer nos LEDs.
*/
void npWrite() {
 // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
 for (uint i = 0; i < LED_COUNT; ++i) {
   pio_sm_put_blocking(np_pio, sm, leds[i].G);
   pio_sm_put_blocking(np_pio, sm, leds[i].R);
   pio_sm_put_blocking(np_pio, sm, leds[i].B);
 }
}

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint JOYSTICK_Y = 27;
const uint JOYSTICK_X = 26;  // Novo: eixo horizontal do joystick

const int LED_R = 13;
const int LED_G = 11;

void setup_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_Y);  // Eixo vertical
    adc_gpio_init(JOYSTICK_X);  // Novo: eixo horizontal
}

void ConfiguraDisplay() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
}

void RenderizaTextoDisplay(int opcao, bool PT, bool GR, bool HO, bool DI) {
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    char text[20], setaUp[5], setaDown[5];
    int valor = 0;

    switch (opcao) {
        case 1: sprintf(text, "Porta 0 GR"); valor = GR; break;
        case 2: sprintf(text, "Porta 1 HO"); valor = HO; break;
        case 3: sprintf(text, "Porta 2 DI"); valor = DI; break;
        case 4: sprintf(text, "Porta 3 PT"); valor = PT; break;
        default: sprintf(text, ""); break;
    }

    ssd1306_draw_string(ssd, 10, 20, text);

    char valor_str[10];
    sprintf(valor_str, "Valor: %d", valor);
    ssd1306_draw_string(ssd, 10, 40, valor_str);

    render_on_display(ssd, &frame_area);
}

// Função que simula a porta lógica
int verificaCatraca(bool PT, bool GR, bool HO, bool DI) {
    return !PT + (GR && HO && DI) ? 1 : 0;
}

// Função para atualizar LED
void atualiza_led(int resultado) {
    if (resultado == 1) {
        gpio_put(LED_R, 0); // Desliga vermelho
        gpio_put(LED_G, 1); // Liga verde
    } else {
        gpio_put(LED_R, 1); // Liga vermelho
        gpio_put(LED_G, 0); // Desliga verde
    }
}

int main() {
    stdio_init_all();
    ConfiguraDisplay();
    setup_joystick();

    npInit(LED_PIN);
    npClear();

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    int opcao = 1;
    int resultado;

    bool GR = true;
    bool HO = true;
    bool DI = true;
    bool PT = false;
    
    while (true) {
        // Leitura vertical
        adc_select_input(0);  // JOYSTICK_Y
        uint16_t adc_y = adc_read();

        if (adc_y > 2000 && opcao < 4) {
            opcao++;
            sleep_ms(200);
        }
        if (adc_y < 500 && opcao > 1) {
            opcao--;
            sleep_ms(200);
        }

        // Leitura horizontal
        adc_select_input(1);  // JOYSTICK_X
        uint16_t adc_x = adc_read();

        if (adc_x > 2500) {
            switch (opcao) {
                case 1: GR = true; break;
                case 2: HO = true; break;
                case 3: DI = true; break;
                case 4: PT = true; break;
            }
            sleep_ms(200);
        }
        if (adc_x < 500) {
            switch (opcao) {
                case 1: GR = false; break;
                case 2: HO = false; break;
                case 3: DI = false; break;
                case 4: PT = false; break;
            }
            sleep_ms(200);
        }

        npSetLED(0, GR ? 0 : 50, GR ? 50 : 0, 0);
        npSetLED(1, HO ? 0 : 50, HO ? 50 : 0, 0);
        npSetLED(2, DI ? 0 : 50, DI ? 50 : 0, 0);
        npSetLED(3, PT ? 0 : 50, PT ? 50 : 0, 0);
        npWrite();
        sleep_ms(50);
        resultado = verificaCatraca(PT, GR, HO, DI);
        atualiza_led(resultado);
        RenderizaTextoDisplay(opcao, PT, GR, HO, DI);

        sleep_ms(50);
    }

    return 0;
}