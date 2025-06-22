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
#define SOIL_MIN_VOLTAGE 1.44f      // Tensão mínima: >1.44V = mínimo para o solo estar úmido, depois do seco
#define SOIL_MAX_VOLTAGE 0.58f      // Tensão máxima: <0.58V = máximo para o solo estar úmido, antes de ficar muito úmido e não matar a planta

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
        if (tensao < SOIL_MAX_VOLTAGE && motor_desligado == 0) {
            // Condição: Menor que 0.58V e bomba não foi desligada
            printf("Mostrando rosto triste :(\n");
            draw_sad_face();                      // Exibe rosto triste no OLED
            gpio_put(PUMP_GPIO, 1);               // Liga a bomba
            sleep_ms(9000);                       // Irriga por 9 segundos
        } 
        else if (tensao >= SOIL_MAX_VOLTAGE) {
            // Condição: Maior ou igual a 0.58V
            printf("Mostrando rosto feliz :)\n");
            draw_happy_face();                    // Exibe rosto feliz no OLED
            gpio_put(PUMP_GPIO, 0);               // Desliga a bomba
            motor_desligado = 1;                  // Evita desligamento repetido
        }
        else if (tensao < SOIL_MIN_VOLTAGE) {
            // Condição: Menor que 1.44V
            printf("Mostrando rosto triste :(\n");
            draw_sad_face();                      // Exibe rosto triste no OLED
            motor_desligado = 0;
            gpio_put(PUMP_GPIO, 1);               // Liga a bomba
            sleep_ms(9000);                       // Irriga por 9 segundos
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
 *    - Compara com os limiares definidos
 *    - Controla bomba e exibe rosto no OLED conforme a umidade
 * 
 * 3. ESTADOS DO SISTEMA (baseado na lógica atual):
 *    - SOLO MUITO ÚMIDO (tensão < 0.58V e motor_desligado == 0): Liga bomba por 9 segundos, mostra rosto triste
 *    - SOLO ÚMIDO (tensão >= 0.58V): Desliga bomba, mostra rosto feliz, evita desligamento repetido
 *    - SOLO SECO (tensão > 1.44V): Liga bomba por 9 segundos, mostra rosto triste, permite novo ciclo
 * 
 * 4. PREVENÇÃO DE CICLOS:
 *    - Flag 'motor_desligado' evita comandos repetidos de desligamento da bomba
 *    - Melhora eficiência e durabilidade do sistema
 * 
 * ===== TABELA DE REFERÊNCIA (Baseada em dados experimentais) =====
 * 
 * | Tensão (V) | Água (mL) | Estado           |
 * |------------|-----------|------------------|
 * | 3.11-1.66  | 0-11      | Seco             |
 * | 1.44-0.58  | 12-20     | Úmido Adequado   |
 * | V <= 0.54  | mL >= 21  | Muito Úmido      |
 * 
 * O sistema considera:
 * - SOLO MUITO ÚMIDO:     tensão < 0.58V e motor_desligado == 0 (bomba ON)
 * - SOLO ÚMIDO:           tensão >= 0.58V (bomba OFF)
 * - SOLO SECO:            tensão < 1.44V (bomba ON, libera novo ciclo)
 */