#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "controller/display/display.h"
#include "controller/matriz/matrizLed.h"
#include "controller/joystick/joystic.c"

#define BTN_A 5
#define LED_VM 13
#define LED_AZ 12
#define LED_VD 11


const uint32_t debounce_time = 200;

volatile uint32_t last_interrupt_time = 0;
volatile bool button_a_pressed = false;

//funcao que é chamada na interrupção do sistema pelo botao a
void gpio_callback(uint gpio, uint32_t events) { 
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if ((current_time - last_interrupt_time) > debounce_time) {  
        button_a_pressed = !button_a_pressed;
        last_interrupt_time = current_time; 
    }
}

//inicializa o ledRGB
void inicializarLedRGB(){
    gpio_init(LED_VM);
    gpio_set_dir(LED_VM, GPIO_OUT);

    gpio_init(LED_AZ); 
    gpio_set_dir(LED_AZ, GPIO_OUT); 
    
    gpio_init(LED_VD); 
    gpio_set_dir(LED_VD, GPIO_OUT); 
}

//desliga o ledRGB
void desligarLedRGB(){
    gpio_put(LED_VM, 0);
    gpio_put(LED_AZ, 0);
    gpio_put(LED_VD, 0);
}

//inicializa o botao
void inicializarBotao(){
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

//inicializa o sistema
void inicializarSistema(){
    stdio_init_all();

    ConfiguraDisplay();

    npInit(LED_PIN);
    npClear();

    init_adc();

    inicializarLedRGB();
    
    inicializarBotao();
}
