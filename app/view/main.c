#include <stdio.h>
#include "pico/stdlib.h"
#include "controller/sistema/utils.h"
#include "controller/wifi/client.h"

//variaveis para armazenar os valores da requisicao

char request[200];
char tremores_send[20];

struct tcp_pcb *pcb;

//Coloque abaixo o hostname e sua senha do wifi

char hostName[] = "internet";

char senhaWifi[] = "12345678";  

int controlarWifi = 0;

int main() {
    

    //trecho que inicializa o sistema
    inicializarSistema();
    adc_init();
    
    //trecho que conecta o wifi

    //define uma variavel auxiliar e pega o tempo absoluto em outra
    uint32_t last_print_time = 0;

    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    

    //renderiza na tela a frase abaixo
    RenderizaTextoDisplay("CONECTANDO O  ", " WiFi ", "");

    //cria uma varaivel que armazena o valor bool do metodo connect wifi
    bool wifi_status = connect_wifi(hostName, senhaWifi);
    
    //Enquanto não conectar ao wifi o sistema nao começa
    while (!wifi_status) {
        
        current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_print_time >= 1000) {
            RenderizaTextoDisplay("CONECTANDO O  ", " WiFi ", "");
            last_print_time = current_time;
            controlarWifi++;
        }
        if(controlarWifi == 2){
            RenderizaTextoDisplay("VERIFIQUE AS  ", " CREDENCIAIS ", " WIFI ");
            controlarWifi= 0;
        }

        wifi_status = connect_wifi(hostName, senhaWifi);
    }

    //Exibi a mensagem no display oled
    RenderizaTextoDisplay("CONECTADO AO  ", " WiFi ", "");

    sleep_ms(500);

    //Configuracao do ip server para o envio da requisicao

    ip_addr_t server_ip;
    resolve_name("api.thingspeak.com", &server_ip);


    //captura das coordenadas iniciais

    uint last_x = read_adc_value(1);
    uint last_y = read_adc_value(0);
    Direcao last_direcao = detectar_direcao(last_x, last_y);
    absolute_time_t start_time = get_absolute_time();


    //Variaveis auxiliares para ajudar na interface
    int estado = 0;
    int time = getTempoExame();
    int cont = 1;

    while (true) {

        //Se o botao tiver pressionado ativar modo Exame e comeca toda a rotina
        if (button_a_pressed) {
            switch (estado) {
                case 0:
                    RenderizaTextoDisplay("Aperte o BTN A", "Para encerrar", "");
                    sleep_ms(2000);
                    //a funcao esperar cima, prende o usuario ate ele mover o joystic para cima
                    esperar_cima();
                    //atualizacao do estado 1, para quando o loop reiniciar o proximo case ser assionado 
                    estado = 1;
                    break;
                case 1:
                    //O case 1 é o case que cuida do exame, emite mensagens e cuida da verificação dos tremores do paciente
                    uint adc_x_raw = read_adc_value(1);
                    uint adc_y_raw = read_adc_value(0);
                    Direcao direcao_atual = detectar_direcao(adc_x_raw, adc_y_raw);

                    //verificar tremor verifica se o usuario saiu dos limites estabelecidos para aquele exame, limites no caso coordenadas no plano cartesiano
                    verificar_tremor(adc_x_raw, adc_y_raw, &last_x, &last_y);
                    char buffer[10];  
                    sprintf(buffer, "%d", time);

                    //Parte que renderiza uma mensagem na tela para o usuario com base no tempo
                    if(time > getTempoExame()/3 && time <= getTempoExame())
                        RenderizaTextoDisplay("Respira fundo ", " Tempo:  ", buffer);
                    if(time > getTempoExame()/10 && time <= getTempoExame()/3)
                        RenderizaTextoDisplay("Calma ", " Tempo:  ", buffer);
                    if(time > 0 && time <= getTempoExame()/10)
                        RenderizaTextoDisplay("Esta quase la ", " Tempo:  ", buffer);
                    
                    //verifica a posicao a cada segundo
                    if(cont == 5){
                        verificar_posicao(direcao_atual);
                        time--;
                        cont = 1;
                    }else
                        cont++;
                    // ao zera o time chama o proximo case
                    if(time == 0)
                        estado = 2;
                    break;
                case 2:

                    // checa tremores é uma função que analisa o numero de tremores com base na variavel tremor_count
                    // tremor_count é um contador que toda vez que ele treme soma 1
                    checarTremores(&start_time);

                    // a parte abaixo é a finalização da mensagem http e o envio para o servidor
                    uint16_t raw = adc_read();
                    sprintf(tremores_send, "%d", tremor_count);
                 
                    sprintf(request, 
                            "GET /update?api_key=74J7A0CI9Q1LDVWK&field1=%s HTTP/1.1\r\n"
                            "Host: api.thingspeak.com\r\n"
                            "Connection: close\r\n\r\n",
                            tremores_send);
            
                    pcb = tcp_new();
                    client_create(pcb, &server_ip, 80);
                    client_write(pcb, request);
                    sleep_ms(1000);
                    client_close(pcb);

                    //apos essa finalizacao estado vai para 3 e cai no default
                    estado = 3;
                    break;
                default:
                    break;
            }
            sleep_ms(200);
        } else {
            //desligarMatriz serve para garantir que os leds da matriz de leds sempre estejam desligados ao iniciar
            desligarMatriz();

            //aqui é o resete das variaveis auxiliares
            estado = 0;
            time = getTempoExame();
            cont = 1;
            tremor_count = 0;

            //desligarLedRGB garante que sempre o led rgb esteja desligado
            desligarLedRGB();

            //por fim a renderização da tela inicial
            RenderizaTextoDisplay("Aperte o BTN A", "Para iniciar", "");
            sleep_ms(100);
        }
    }
}
