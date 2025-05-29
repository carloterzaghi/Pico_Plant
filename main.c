// ===== INCLUSÃO DE BIBLIOTECAS =====
#include "pico/stdlib.h"                    // Funções básicas do Raspberry Pi Pico
#include "hardware/adc.h"                   // Interface para uso do ADC
#include "auxiliary_codes/pwm_code.h"       // Controle de alimentação via PWM
#include <stdio.h>                          // Entrada e saída padrão
#include "hardware/i2c.h"                   // Comunicação I2C
#include "hardware/gpio.h"                  // Controle de GPIOs
#include "auxiliary_codes/oled_ssd1306.h"   // Funções gráficas para display OLED

// ===== Definições de Hardware =====
#define SENSOR_ADC_GPIO 26     // GPIO 26 - Entrada analógica (ADC0) para sensor de umidade
#define SENSOR_PWR_GPIO 2      // GPIO 2  - Saída PWM para alimentação do sensor
#define PUMP_GPIO 15           // GPIO 15 - Saída digital para controle da bomba d'água
#define I2C_SDA 14             // GPIO 14 - Linha de dados I2C
#define I2C_SCL 11             // GPIO 11 - Linha de clock I2C

// ===== Parâmetros de Sistema =====
#define SOIL_VOLTAGE 2.5f      // Limiar de tensão: ≥2.5V = solo seco, <2.5V = solo úmido

int main() {
    // === Inicialização do Sistema ===
    stdio_init_all();                      // Comunicação serial via USB
    printf("Inicializando faces animadas...\n");

    // --- Inicialização do barramento I2C ---
    i2c_init(I2C_PORT, 100 * 1000);        // Frequência I2C = 100kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);                 // Pull-ups necessários para I2C
    gpio_pull_up(I2C_SCL);

    // --- Inicialização do Display OLED ---
    printf("Inicializando OLED...\n");
    sleep_ms(100);
    oled_init();
    printf("OLED inicializado! Iniciando animação...\n");

    // === Configuração de Hardware ===

    // --- Alimentação do sensor por PWM ---
    setup_pwm_power(SENSOR_PWR_GPIO);      // Estabiliza 3.3V via PWM para sensor analógico

    // --- Controle da bomba d'água ---
    gpio_init(PUMP_GPIO);                  
    gpio_set_dir(PUMP_GPIO, GPIO_OUT);     // Configura GPIO como saída

    // --- Inicialização do ADC ---
    adc_init();                            
    adc_gpio_init(SENSOR_ADC_GPIO);        // Ativa função analógica no GPIO 26
    adc_select_input(0);                   // Seleciona canal 0 do ADC

    // === Controle Inteligente ===
    int motor_desligado = 0;               // Flag de estado da bomba
                                           // 0 = bomba pode ser ligada
                                           // 1 = bomba já foi desligada após solo umedecido

    // === Loop Principal ===
    while (true) {
        // --- Leitura do sensor ---
        uint16_t leitura = adc_read();
        float tensao = leitura * 3.3f / 4095.0f; 
        printf("Leitura ADC: %d\tTensão: %.2f V\n", leitura, tensao);

        // --- Lógica de Irrigação ---
        if (tensao >= SOIL_VOLTAGE) {
            // Condição: solo seco
            printf("Mostrando rosto triste :(\n");
            draw_sad_face();                      // Exibe rosto triste no OLED
            motor_desligado = 0;
            gpio_put(PUMP_GPIO, 1);               // Liga a bomba
            sleep_ms(9000);                       // Irriga por 9 segundos
        } 
        else if (motor_desligado == 0) {
            // Condição: solo umedecido e bomba ainda ligada
            printf("Mostrando rosto feliz :)\n");
            draw_happy_face();                    // Exibe rosto feliz no OLED
            gpio_put(PUMP_GPIO, 0);               // Desliga a bomba
            motor_desligado = 1;                  // Evita desligamento repetido
        }

        // Aguarda 1 segundo antes da próxima leitura
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