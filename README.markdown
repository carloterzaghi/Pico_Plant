# Sistema de Irrigação Inteligente com Raspberry Pi Pico

Um sistema embarcado para monitoramento de umidade do solo e controle automatizado de irrigação, utilizando o microcontrolador Raspberry Pi Pico.

## Visão Geral

Este projeto visa criar um sistema de irrigação eficiente e autônomo para plantas, especialmente projetado para uso doméstico ou em pequenos cultivos. Ele utiliza um sensor de umidade do solo capacitivo, criado previamente para somente esse projeto, para detectar o nível de umidade do solo e acionar uma bomba d'água conforme a necessidade. O sistema é construído sobre o Raspberry Pi Pico e programado em C/C++ utilizando o Pico SDK.

## Funcionalidades

* **Monitoramento Contínuo da Umidade do Solo**: Lê a tensão do sensor de umidade do solo a cada segundo para determinar o nível de umidade.
* **Controle Inteligente da Bomba D'água**: Ativa ou desativa a bomba automaticamente conforme o estado do solo, com lógica baseada em limiares de tensão.
* **Irrigação Otimizada**: Mantém a bomba ligada por 9 segundos quando necessário para garantir a hidratação adequada do solo.
* **Prevenção de Comandos Repetidos**: Uma *flag* (`motor_desligado`) evita o envio de comandos desnecessários à bomba, aumentando a eficiência e durabilidade.
* **Alimentação Estável do Sensor**: Utiliza PWM (Pulse Width Modulation) configurado com 100% de *duty cycle* no GPIO 2 para fornecer uma alimentação de 3.3V estáveis ao sensor de umidade, garantindo leituras precisas.
* **Comunicação Serial USB**: Fornece *feedback* em tempo real das leituras do sensor via comunicação serial para fins de depuração e monitoramento.
* **Feedback Visual**: Mostra rostos felizes ou tristes em um display OLED conforme o estado do solo.

## Hardware

### Esquema Elétrico do Controlador de Irrigação Inteligente
![Esquema elétrico do controlador](images/Schematic_Pico-Plant.png)

### Exemplo de PCB do Controlador
#### Top Layer
![PCB Top Layer do controlador](images/PCB_Pico-Plant_top_layer.png)

#### Bottom Layer
![PCB Bottom Layer do controlador](images/PCB_Pico-Plant_bottom_layer.png)

### Esquema Elétrico do Sensor Capacitivo de Umidade do Solo
![Esquema elétrico do sensor](images/Schematic_Capacitive-Soil-Moisture-Sensor.png)

### Exemplo de PCB do Sensor Capacitivo
#### Bottom Layer
![PCB Bottom Layer do sensor](images/PCB_Capacitive-Soil-Moisture-Sensor.png)

## Estrutura do Projeto

* `CMakeLists.txt`: Arquivo de configuração do CMake para construir o projeto.
* `main.c`: Contém a lógica principal do sistema, incluindo a inicialização, leitura do ADC, controle da bomba e exibição no OLED.
* `auxiliary_codes/pwm_code.c`: Implementa a função de configuração do PWM para alimentar o sensor.
* `auxiliary_codes/pwm_code.h`: Declaração da função de configuração do PWM.

## Lógica de Operação Detalhada

O sistema opera com base na leitura da tensão do sensor de umidade do solo, convertida pelo ADC do Raspberry Pi Pico. A lógica de irrigação é baseada em três faixas principais de tensão, conforme dados experimentais e a lógica implementada no código:

* **Valores de Referência**:
    * **Solo Muito Úmido**: Tensão < 0.58V **e** `motor_desligado == 0`
        * **Ação**: Liga a bomba por 9 segundos, mostra rosto triste no OLED.
    * **Solo Úmido**: Tensão >= 0.58V
        * **Ação**: Desliga a bomba, mostra rosto feliz no OLED, evita desligamento repetido (seta `motor_desligado = 1`).
    * **Solo Seco**: Tensão < 1.44V
        * **Ação**: Liga a bomba por 9 segundos, mostra rosto triste no OLED, libera novo ciclo (seta `motor_desligado = 0`).

* **Tabela de Referência da Umidade do Solo**:

| Tensão (V) | Água (mL) | Estado           |
|------------|-----------|------------------|
| 3.11-1.66  | 0-11      | Seco             |
| 1.44-0.58  | 12-20     | Úmido Adequado   |
| V <= 0.54  | mL >= 21  | Muito Úmido      |

*Dados coletados:*

Materiais utilizados: `copo com 84757,03 mm^3` e `seringa com medidas de 1 em 1 mL`.

![Gráfico Tensão (V) x Água (mL)](images/tensaoxagua.png)
![Gráfico Tensão (V) x Consentração de Terra por Água (mm^3/mL)](images/tensaoxconcentracao.png)

* **Prevenção de Ciclos**:  
A flag `motor_desligado` evita comandos repetidos de desligamento da bomba, melhorando a eficiência e durabilidade do sistema.

---

