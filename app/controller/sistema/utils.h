#ifndef SISTEMA_H
#define SISTEMA_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "utils.c"
#include <stdio.h>


// Definições de pinos
#define BTN_A 5
#define LED_VM 13
#define LED_AZ 12
#define LED_VD 11

// Definição de tempo de debounce para o botão
#define DEBOUNCE_TIME 200

// Funções de inicialização e manipulação de hardware


// Função de interrupção para o botão
void gpio_callback(uint gpio, uint32_t events);

// Funções para display e controle de LEDs
void ConfiguraDisplay(void);
void npInit(uint pin);
void npClear(void);

// Funções para controle de ADC (se houver)


extern volatile bool button_a_pressed; // Variável externa para controle do estado do botão
extern volatile uint32_t last_interrupt_time; // Variável externa para controle do tempo da última interrupção

#endif // SISTEMA_H
