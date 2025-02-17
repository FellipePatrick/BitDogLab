#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "matrizLed.h"



npLED_t leds[LED_COUNT]; // Buffer de pixels
PIO np_pio;              // Variáveis para PIO
uint sm;

void adjustBrightness(npLED_t *led, uint8_t brightness) {
    led->R = (led->R * brightness) / 255;
    led->G = (led->G * brightness) / 255;
    led->B = (led->B * brightness) / 255;
}

void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

void setaPraCima(uint8_t r, uint8_t g, uint8_t b) {
    npSetLED(2, r, g, b);
    npWrite();
    sleep_ms(100);
    npSetLED(7, r, g, b);
    npWrite();
    sleep_ms(100);
    npSetLED(12, r, g, b);
    npSetLED(14, r, g, b);
    npSetLED(10, r, g, b);
    npWrite();
    sleep_ms(100);
    npSetLED(13, r, g, b);
    npSetLED(11, r, g, b);
    npSetLED(16, r, g, b);
    npSetLED(17, r, g, b);
    npSetLED(18, r, g, b);
    npWrite();
    sleep_ms(100);
    npSetLED(22, r, g, b);
    npWrite();
    sleep_ms(100);
    
    npSetLED(2, 0, 0, 0);
    npSetLED(7, 0, 0, 0);
    npSetLED(12, 0, 0, 0);
    npSetLED(14, 0, 0, 0);
    npSetLED(10, 0, 0, 0);
    npSetLED(13, 0, 0, 0);
    npSetLED(11, 0, 0, 0);
    npSetLED(16, 0, 0, 0);
    npSetLED(17, 0, 0, 0);
    npSetLED(18, 0, 0, 0);
    npSetLED(22, 0, 0, 0);
    npWrite();
    sleep_ms(100);
}

void desligarMatriz() {
    npSetLED(2, 0, 0, 0);
    npSetLED(7, 0, 0, 0);
    npSetLED(12, 0, 0, 0);
    npSetLED(14, 0, 0, 0);
    npSetLED(10, 0, 0, 0);
    npSetLED(13, 0, 0, 0);
    npSetLED(11, 0, 0, 0);
    npSetLED(16, 0, 0, 0);
    npSetLED(17, 0, 0, 0);
    npSetLED(18, 0, 0, 0);
    npSetLED(22, 0, 0, 0);
    npWrite();
    sleep_ms(100);
}
