// ===== INCLUSÃO DE BIBLIOTECAS =====
#include "pico/stdlib.h"                    // Biblioteca padrão do Pico SDK
#include "hardware/adc.h"                   // Biblioteca para controle do ADC
#include "auxiliary_codes/pwm_code.h"       // Biblioteca customizada para controle PWM
#include <stdio.h>                          // Biblioteca padrão para entrada/saída

// ===== DEFINIÇÃO DE CONSTANTES =====
#define SENSOR_ADC_GPIO 26     // GPIO 26 - Entrada analógica do sensor de umidade (ADC0)
#define SENSOR_PWR_GPIO 2      // GPIO 2  - Saída PWM para alimentação do sensor
#define PUMP_GPIO 15           // GPIO 15 - Saída digital para controle da bomba d'água
#define SOIL_VOLTAGE 2.5f      // Limiar de tensão: ≥2.5V = solo seco, <2.5V = solo úmido

int main() {
    // ===== INICIALIZAÇÃO DO SISTEMA =====
    stdio_init_all();                       // Inicializa comunicação serial USB para debug

    // ===== CONFIGURAÇÃO DO SENSOR DE UMIDADE =====
    setup_pwm_power(SENSOR_PWR_GPIO);       // Configura GPIO 2 como PWM para alimentar o sensor
                                            // PWM garante alimentação estável e constante

    // ===== CONFIGURAÇÃO DA BOMBA D'ÁGUA =====
    gpio_init(PUMP_GPIO);                   // Inicializa GPIO 15 para controle da bomba
    gpio_set_dir(PUMP_GPIO, GPIO_OUT);      // Define GPIO 15 como saída digital

    // ===== CONFIGURAÇÃO DO ADC (Conversor Analógico-Digital) =====
    adc_init();                             // Inicializa o módulo ADC do Pico
    adc_gpio_init(SENSOR_ADC_GPIO);         // Configura GPIO 26 como entrada analógica
    adc_select_input(0);                    // Seleciona canal 0 do ADC (GPIO 26 = ADC0)

    // ===== VARIÁVEL DE CONTROLE =====
    int motor_desligado = 0;                // Flag para controle inteligente da bomba
                                            // 0 = bomba pode estar ligada
                                            // 1 = bomba já foi desligada (evita comandos repetidos)

    // ===== LOOP PRINCIPAL DO SISTEMA =====
    while (true) {
        // ----- LEITURA DO SENSOR DE UMIDADE -----
        uint16_t leitura = adc_read();      
        float tensao = leitura * 3.3f / 4095.0f; 
        printf("Leitura ADC: %d\tTensão: %.2f V\n", leitura, tensao);
        
        // ----- LÓGICA DE CONTROLE DA IRRIGAÇÃO -----
        if (tensao >= SOIL_VOLTAGE){       // Se tensão ≥ 2.5V, solo está seco
            motor_desligado = 0;            // Reset da flag (permite desligar bomba depois)
            gpio_put(PUMP_GPIO, 1);         // Liga a bomba d'água (nível lógico 1)
            sleep_ms(9000);                 // Mantém irrigação por 9 segundos (9000ms)
                                            // Tempo suficiente para umedecer o solo
        } 
        else if (motor_desligado == 0){     // Se solo úmido E bomba ainda não foi desligada
            gpio_put(PUMP_GPIO, 0);         // Desliga a bomba d'água (nível lógico 0)
            motor_desligado = 1;            // Marca que bomba foi desligada
                                            // Evita enviar comando "desligar" repetidamente
        }
        // Nota: Se solo úmido E motor_desligado == 1, não faz nada
        // Isso evita comandos GPIO desnecessários e melhora eficiência

        sleep_ms(1000);                     
    }

    return 0;                           
}

/*
 * ===== RESUMO DO FUNCIONAMENTO =====
 * 
 * 1. INICIALIZAÇÃO:
 *    - PWM no GPIO 2 alimenta sensor de umidade
 *    - GPIO 15 configurado para controlar bomba
 *    - ADC configurado para ler GPIO 26
 * 
 * 2. CICLO DE MONITORAMENTO (a cada 1 segundo):
 *    - Lê tensão do sensor (0-3.3V)
 *    - Compara com limiar (2.5V)
 *    - Controla bomba baseado na umidade
 * 
 * 3. ESTADOS DO SISTEMA:
 *    - SOLO SECO (≥2.5V):   Liga bomba por 9 segundos
 *    - SOLO ÚMIDO (<2.5V):  Desliga bomba (se estava ligada)
 * 
 * 4. PREVENÇÃO DE CICLOS:
 *    - Flag 'motor_desligado' evita comandos repetidos
 *    - Melhora eficiência e durabilidade do sistema
 * 
 * ===== VALORES DE REFERÊNCIA =====
 * 
 * ADC (0-4095)  |  Tensão (V)  |  Condição do Solo  |  Ação
 * --------------|--------------|-------------------|--------
 * 0    - 2478   |  0.0 - 1.99  |  Muito úmido      |  Bomba OFF
 * 2479 - 3100   |  2.0 - 2.49  |  Úmido adequado   |  Bomba OFF  
 * 3101 - 4095   |  2.5 - 3.3   |  Seco             |  Bomba ON
 */