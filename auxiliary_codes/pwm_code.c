// ===== INCLUSÃO DE BIBLIOTECAS =====
#include "pwm_code.h"                       // Header com declarações das funções PWM
#include "hardware/pwm.h"                   // Biblioteca do Pico SDK para controle PWM
#include "hardware/gpio.h"                  // Biblioteca do Pico SDK para controle GPIO

void setup_pwm_power(uint gpio) {
    
    // ===== CONFIGURAÇÃO DA FUNÇÃO DO GPIO =====
    gpio_set_function(gpio, GPIO_FUNC_PWM);  // Define GPIO para função PWM
                                             // Remove qualquer configuração anterior
                                             // (digital input/output, SPI, I2C, etc.)

    // ===== IDENTIFICAÇÃO DO SLICE PWM =====
    uint slice_num = pwm_gpio_to_slice_num(gpio); // Identifica qual slice PWM controla este GPIO
                                                   // Cada slice controla 2 canais (A e B)
                                                   // Ex: GPIO 2 → Slice 1, GPIO 3 → Slice 1

    // ===== CONFIGURAÇÃO DOS PARÂMETROS PWM =====
    pwm_config config = pwm_get_default_config(); // Obtém configuração padrão do PWM
                                                   // Valores padrão: clkdiv=1, wrap=65535
    
    // ----- CONFIGURAÇÃO DO DIVISOR DE CLOCK -----
    pwm_config_set_clkdiv(&config, 4.0f);         // Define divisor de clock = 4
                                                   // Clock do sistema: 125 MHz
                                                   // Clock do PWM: 125MHz ÷ 4 = 31.25 MHz
                                                   // Reduz frequência para operação mais estável

    // ----- CONFIGURAÇÃO DA RESOLUÇÃO PWM -----
    pwm_config_set_wrap(&config, 65535);          // Define valor máximo do contador = 65535
                                                   // Resolução: 16 bits (2^16 - 1 = 65535)
                                                   // Frequência PWM: 31.25MHz ÷ 65536 ≈ 477 Hz
                                                   // CORREÇÃO: 31.25MHz ÷ 65536 ≈ 477 Hz

    // ===== INICIALIZAÇÃO DO SLICE PWM =====
    pwm_init(slice_num, &config, true);           // Inicializa o slice com configurações definidas
                                                   // Parâmetros:
                                                   // - slice_num: qual slice configurar
                                                   // - &config: ponteiro para configuração
                                                   // - true: habilita PWM imediatamente

    // ===== CONFIGURAÇÃO DO DUTY CYCLE =====
    pwm_set_gpio_level(gpio, 65535);              // Define duty cycle = 100%
                                                   // 65535 = valor máximo (wrap value)
                                                   // Resultado: saída sempre em 3.3V
                                                   // Funciona como fonte de alimentação DC
}

/*
 * ===== DETALHES TÉCNICOS DO PWM =====
 * 
 * 1. CÁLCULO DA FREQUÊNCIA:
 *    Frequência = Clock_Sistema ÷ Divisor ÷ (Wrap + 1)
 *    Frequência = 125MHz ÷ 4 ÷ 65536 ≈ 477 Hz
 * 
 * 2. DUTY CYCLE:
 *    Duty Cycle = (Level ÷ Wrap) × 100%
 *    Duty Cycle = (65535 ÷ 65535) × 100% = 100%
 * 
 * 3. TENSÃO DE SAÍDA:
 *    Tensão Média = Vcc × Duty Cycle
 *    Tensão Média = 3.3V × 100% = 3.3V
 * 
 * 4. VANTAGENS DO PWM vs GPIO DIGITAL:
 *    - Maior estabilidade de tensão
 *    - Menor ruído elétrico
 *    - Melhor para alimentação de sensores analógicos
 *    - Capacidade de ajuste fino (se necessário)
 * 
 * ===== APLICAÇÃO NO SISTEMA DE IRRIGAÇÃO =====
 * 
 * No contexto do sistema de irrigação, esta função é usada para:
 * - Alimentar o sensor de umidade do solo com 3.3V estáveis
 * - Garantir leituras ADC consistentes e precisas
 * - Evitar flutuações de tensão que poderiam causar leituras falsas
 * - Permitir operação contínua do sensor sem degradação
 */