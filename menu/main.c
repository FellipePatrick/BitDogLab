#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <string.h>

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
const uint JOYSTICK_X = 26; 
const uint JOYSTICK_Y = 27;  
const uint BUTTON_PIN = 22;
const uint BUZZER_PIN = 21;
const uint LED = 12; // LED conectado

const int ADC_CHANNEL_0 = 0; // Canal ADC para o eixo X do joystick
const int ADC_CHANNEL_1 = 1; // Canal ADC para o eixo Y do joystick


const int LED_B = 13;                    // Pino para controle do LED azul via PWM
const int LED_R = 11;                    // Pino para controle do LED vermelho via PWM
const float DIVIDER_PWM = 16.0;          // Divisor fracional do clock para o PWM
const uint16_t PERIODJ = 4096;            // Período do PWM (valor máximo do contador)
uint16_t led_b_level, led_r_level = 100; // Inicialização dos níveis de PWM para os LEDs
uint slice_led_b, slice_led_r;           // Variáveis para armazenar os slices de PWM correspondentes aos LEDs

void setup_joystick()
{
  // Inicializa o ADC e os pinos de entrada analógica
  adc_init();         // Inicializa o módulo ADC
  adc_gpio_init(JOYSTICK_X); // Configura o pino VRX (eixo X) para entrada ADC
  adc_gpio_init(JOYSTICK_Y); // Configura o pino VRY (eixo Y) para entrada ADC

  // Inicializa o pino do botão do joystick
  gpio_init(BUTTON_PIN);             // Inicializa o pino do botão
  gpio_set_dir(BUTTON_PIN, GPIO_IN); // Configura o pino do botão como entrada
  gpio_pull_up(BUTTON_PIN);          // Ativa o pull-up no pino do botão para evitar flutuações
}

// Função para configurar o PWM de um LED (genérica para azul e vermelho)
void setup_pwm_led_joy(uint led, uint *slice, uint16_t level)
{
  gpio_set_function(led, GPIO_FUNC_PWM); // Configura o pino do LED como saída PWM
  *slice = pwm_gpio_to_slice_num(led);   // Obtém o slice do PWM associado ao pino do LED
  pwm_set_clkdiv(*slice, DIVIDER_PWM);   // Define o divisor de clock do PWM
  pwm_set_wrap(*slice, PERIODJ);          // Configura o valor máximo do contador (período do PWM)
  pwm_set_gpio_level(led, level);        // Define o nível inicial do PWM para o LED
  pwm_set_enabled(*slice, true);         // Habilita o PWM no slice correspondente ao LED
}


int opcao = 0;
volatile bool botao_pressionado = false;

const uint star_wars_notes[] = {
    330, 330, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 523, 494, 440, 392, 330,
    659, 784, 659, 523, 494, 440, 392, 330,
    659, 659, 330, 784, 880, 698, 784, 659,
    523, 494, 440, 392, 659, 784, 659, 523,
    494, 440, 392, 330, 659, 523, 659, 262,
    330, 294, 247, 262, 220, 262, 330, 262,
    330, 294, 247, 262, 330, 392, 523, 440,
    349, 330, 659, 784, 659, 523, 494, 440,
    392, 659, 784, 659, 523, 494, 440, 392
};

const uint note_duration[] = {
    500, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 650, 500, 150, 300, 500, 350,
    150, 300, 500, 150, 300, 500, 350, 150,
    300, 650, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 350, 150, 300, 500, 500,
    350, 150, 300, 500, 500, 350, 150, 300,
};

const uint16_t PERIOD = 2000;   // Período do PWM (máximo do contador
const uint16_t LED_STEP = 100;  // Passo de variação do LED
uint16_t led_level = 100;       // Nível inicial do LED PWM


void botao_callback(uint gpio, uint32_t events) {
    if (botao_pressionado)
    {
       botao_pressionado = false;
    }else{
        botao_pressionado = true;
    }
    
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

void play_tone(uint pin, uint frequency, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2);
    sleep_ms(duration_ms);
    pwm_set_gpio_level(pin, 0);
    sleep_ms(50);
}

void play_star_wars(uint pin) {
    for (int i = 0; i < sizeof(star_wars_notes) / sizeof(star_wars_notes[0]); i++) {
        if (star_wars_notes[i] == 0) {
            sleep_ms(note_duration[i]);
        } else {
            play_tone(pin, star_wars_notes[i], note_duration[i]);
        }
    }
}

void ConfiguraDisplay() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
}

void RenderizaTextoDisplay(char *text1, char *text2, char *text3) {
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 0, 0, text1);
    ssd1306_draw_string(ssd, 0, 8, text2);
    ssd1306_draw_string(ssd, 0, 16, text3);

    render_on_display(ssd, &frame_area);
}

void RenderizaOpcaoSelecionada() {
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    char opcao_texto[20];
    sprintf(opcao_texto, (opcao == 3) ? "Modo Joystick" : (opcao == 2) ? "Modo Buzzer" : "Modo RGB");

    ssd1306_draw_string(ssd, 0, 0, opcao_texto);
    render_on_display(ssd, &frame_area);
}


void setup_pwm_led() {
    uint slice = pwm_gpio_to_slice_num(LED);
    gpio_set_function(LED, GPIO_FUNC_PWM);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(LED, led_level);
    pwm_set_enabled(slice, true);
}

void efeito_led() {
    static uint up_down = 1;

    pwm_set_gpio_level(LED, led_level);
    sleep_ms(500);

    if (up_down) {
        led_level += LED_STEP;
        if (led_level >= PERIOD) up_down = 0;
    } else {
        led_level -= LED_STEP;
        if (led_level <= LED_STEP) up_down = 1;
    }
}

void setup()
{
  stdio_init_all();                                // Inicializa a porta serial para saída de dados
  setup_joystick();                                // Chama a função de configuração do joystick
  setup_pwm_led_joy(LED_B, &slice_led_b, led_b_level); // Configura o PWM para o LED azul
  setup_pwm_led_joy(LED_R, &slice_led_r, led_r_level); // Configura o PWM para o LED vermelho
}
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value)
{
  // Leitura do valor do eixo X do joystick
  adc_select_input(ADC_CHANNEL_0); // Seleciona o canal ADC para o eixo X
  sleep_us(2);                     // Pequeno delay para estabilidade
  *vrx_value = adc_read();         // Lê o valor do eixo X (0-4095)

  // Leitura do valor do eixo Y do joystick
  adc_select_input(ADC_CHANNEL_1); // Seleciona o canal ADC para o eixo Y
  sleep_us(2);                     // Pequeno delay para estabilidade
  *vry_value = adc_read();         // Lê o valor do eixo Y (0-4095)
}


int main() {
     uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
    setup();        
    stdio_init_all();
    ConfiguraDisplay();
    pwm_init_buzzer(BUZZER_PIN);
    setup_pwm_led();

    adc_init();
    adc_gpio_init(JOYSTICK_Y);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &botao_callback);

    bool led_ativo = false;
    while (true) {
        adc_select_input(0);
        uint16_t adc_y = adc_read();

        if (botao_pressionado) {
            RenderizaOpcaoSelecionada();
            switch (opcao) {
                case 1:  // Modo LED RGB
                    if (!led_ativo) {
                        led_ativo = true;
                    }
                    efeito_led();
                    break;
                case 2:  // Modo Buzzer
                    if (led_ativo) {
                        pwm_set_gpio_level(LED, 0); // Desliga o LED ao sair do modo LED
                        led_ativo = false;
                    }
                    play_star_wars(BUZZER_PIN);
                    break;
                case 3:  // Modo Joystick
                    if (led_ativo) {
                        pwm_set_gpio_level(LED, 0);
                        led_ativo = false;
                    }
                      joystick_read_axis(&vrx_value, &vry_value); // Lê os valores dos eixos do joystick
                        // Ajusta os níveis PWM dos LEDs de acordo com os valores do joystick
                        pwm_set_gpio_level(LED_B, vrx_value); // Ajusta o brilho do LED azul com o valor do eixo X
                        pwm_set_gpio_level(LED_R, vry_value); 
                    break;
                default:
                    break;
            }
        } else {
            led_level = 100;
            led_b_level, led_r_level = 100;
            pwm_set_gpio_level(LED_B, 0); 
            pwm_set_gpio_level(LED_R, 0); 
            led_ativo = false;
            pwm_set_gpio_level(LED, 0); 
            if (adc_y > 2000 && opcao < 3) {
                opcao++;
                sleep_ms(200);
            }
            if (adc_y < 500 && opcao > 1) {
                opcao--;
                sleep_ms(200);
            }

            char text1[20], text2[20], text3[20];
            sprintf(text1, (opcao == 3) ? "o JoyStick" : "JoyStick");
            sprintf(text2, (opcao == 2) ? "o Buzzer" : "Buzzer");
            sprintf(text3, (opcao == 1) ? "o LED RGB" : "LED RGB");

            RenderizaTextoDisplay(text1, text2, text3);
        }
        sleep_ms(100);
    }

    return 0;
}
