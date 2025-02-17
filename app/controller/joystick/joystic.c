#include "hardware/adc.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define SENSITIVITY 15        // Mínima mudança para considerar alteração
#define SHAKE_THRESHOLD 150   // Variação para detectar tremor


//variaveis limites para verificar tremores e ausencia de posicao
#define CENTER_THRESHOLD 400  
#define MAX_ADC_VALUE 4095    

uint tremor_count = 0; // contabiliza os tremores

#define LED_VM 13
#define LED_AZ 12
#define LED_VD 11

int tempoExame = 10; //variavel que indica o tempo total de um exame de tremor

typedef enum {
    CENTRO,
    CIMA,
} Direcao;


//retorna os tremores
uint getTremores(){
    return tremor_count;
}

//inicializa o adc
void init_adc() {
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
}


// ler o valor adc
uint read_adc_value(uint channel) {
    adc_select_input(channel);
    return adc_read();
}

//verifica se teve mudança de posicao
bool has_changed(uint new_value, uint last_value) {
    return (new_value > last_value + SENSITIVITY || new_value < last_value - SENSITIVITY);
}

//verifica se teve tremor
bool is_shaking(uint new_value, uint last_value) {
    return (new_value > last_value + SHAKE_THRESHOLD || new_value < last_value - SHAKE_THRESHOLD);
}

// detecta a direcao atual do joystic, nesse codigo tudo q n for cima é centro
Direcao detectar_direcao(uint x, uint y) {
    if (y > (MAX_ADC_VALUE - CENTER_THRESHOLD)) return CIMA;
    return CENTRO;
}

// retorna o tempoExame
int getTempoExame(){
    return tempoExame;
}

//funcao que espera ate mover o joystic para cima
void esperar_cima() {
    uint x, y;
    Direcao direcao;
    RenderizaTextoDisplay("MOVA O JOYSTIC", "Para CIMA para ", "iniciar");
    do {
        setaPraCima(255,255,0);
        x = read_adc_value(1);
        y = read_adc_value(0);
        direcao = detectar_direcao(x, y);
    } while (direcao != CIMA);
}

//verifica com base nas mudanças se houve tremores
void verificar_tremor(uint adc_x_raw, uint adc_y_raw, uint *last_x, uint *last_y) {
    if (has_changed(adc_x_raw, *last_x) || has_changed(adc_y_raw, *last_y)) {
        if (is_shaking(adc_x_raw, *last_x) || is_shaking(adc_y_raw, *last_y)) {
            tremor_count++;
        }
        *last_x = adc_x_raw;
        *last_y = adc_y_raw;
    }
}

//funcao que define a posicao atual, 
void verificar_posicao(Direcao direcao_atual) {
    if (direcao_atual != CIMA ) {
        setaPraCima(255, 0, 0);
    } else if (direcao_atual == CIMA) {
        setaPraCima(0,255,0);
    }
}

// Função para calcular a taxa de tremores por segundo
float calculaTaxaTremores(int tremor_count, float tempoSegundos) {
    return tremor_count / tempoSegundos;
}

// Função para determinar o nível de tremor com base na taxa de tremores por segundo
int determinaNivelTremor(float tremoresPorSegundo) {
    // o calculo é bem simples, ja que se trata de um prototipo inicial, se ele tiver 1 tremor por segundo é baixo, 2 é medio, maior que 5 é alto
    if (tremoresPorSegundo > 5.0) {
        return 3; // Alto
    } else if (tremoresPorSegundo > 2.0) {
        return 2; // Médio
    } else if (tremoresPorSegundo > 1.0) {
        return 1; 
    }else
        return 0;
}


//função que calcula e mostra no display o nivel de tremores 
void checarTremores() {
    float tempoExameSegundos = getTempoExame(); 
    float tremoresPorSegundo = calculaTaxaTremores(tremor_count, tempoExameSegundos);
    int nivel_tremor = determinaNivelTremor(tremoresPorSegundo);

    switch (nivel_tremor) {
        case 3:
            //exibe mensagem no display oled e liga o led rgb na cor vermelha
            RenderizaTextoDisplay("Nivel de tremor:  ", " ALTO ", "");
            gpio_put(LED_VM, 1);
            gpio_put(LED_AZ, 0);
            gpio_put(LED_VD, 0);
            break;
        case 2:
        //exibe mensagem no display oled e liga o led rgb na cor amarela
            RenderizaTextoDisplay("Nivel de tremor:  ", " MEDIO ", "");
            gpio_put(LED_VM, 1);
            gpio_put(LED_AZ, 0);
            gpio_put(LED_VD, 1);
            break;
        case 1:
        //exibe mensagem no display oled e liga o led rgb na cor verde
            RenderizaTextoDisplay("Nivel de tremor:  ", " BAIXO ", "");
            gpio_put(LED_VM, 0);
            gpio_put(LED_AZ, 0);
            gpio_put(LED_VD, 1);
            break;
        case 0:
            //exibe mensagem no display oled e não liga o led
            RenderizaTextoDisplay("Nivel de tremor:  ", " ZERO ", "");
            break;
    }
}