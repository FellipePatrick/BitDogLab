#ifndef NPMATRIX_H
#define NPMATRIX_H

#include "hardware/pio.h"
#include "pico/stdlib.h"

#define LED_COUNT 25
#define LED_PIN 7
#define ROWS 5
#define COLS 5


// Definição de pixel GRB
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

extern npLED_t leds[25]; // Buffer de pixels
extern PIO np_pio;       // Variáveis para PIO
extern uint sm;

// Funções
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
void adjustBrightness(npLED_t *led, uint8_t brightness);
void setaPraCima(uint8_t r, uint8_t g, uint8_t b);
void desligarMatriz();

#endif // NPMATRIX_H
