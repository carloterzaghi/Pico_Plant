# Sistema de Irrigação Inteligente com Raspberry Pi Pico

Um sistema embarcado para monitoramento de umidade do solo e controle automatizado de irrigação, utilizando o microcontrolador Raspberry Pi Pico.

## Visão Geral

Este projeto visa criar um sistema de irrigação eficiente e autônomo para plantas, especialmente projetado para uso doméstico ou em pequenos cultivos. Ele utiliza um sensor de umidade do solo capacitivo, criado previamente para somente esse projeto, para detectar quando o solo está seco e, em seguida, aciona uma bomba d'água para irrigar a planta por um período determinado. O sistema é construído sobre o Raspberry Pi Pico e programado em C/C++ utilizando o Pico SDK.

## Funcionalidades

* **Monitoramento Contínuo da Umidade do Solo**: Lê a tensão do sensor de umidade do solo a cada segundo para determinar o nível de umidade[cite: 2].
* **Controle Inteligente da Bomba D'água**: Ativa a bomba d'água automaticamente quando a umidade do solo atinge um nível crítico (tensão $\ge$ 2.5V)[cite: 2].
* **Irrigação Otimizada**: Mantém a bomba ligada por 9 segundos para garantir a hidratação adequada do solo[cite: 2].
* **Prevenção de Comandos Repetidos**: Uma *flag* (`motor_desligado`) evita o envio de comandos desnecessários à bomba, aumentando a eficiência e durabilidade[cite: 2].
* **Alimentação Estável do Sensor**: Utiliza PWM (Pulse Width Modulation) configurado com 100% de *duty cycle* no GPIO 2 para fornecer uma alimentação de 3.3V estáveis e contínua ao sensor de umidade, garantindo leituras precisas[cite: 2, 3].
* **Comunicação Serial USB**: Fornece *feedback* em tempo real das leituras do sensor via comunicação serial para fins de depuração e monitoramento[cite: 2].

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

* `CMakeLists.txt`: Arquivo de configuração do CMake para construir o projeto[cite: 1].
* `main.c`: Contém a lógica principal do sistema, incluindo a inicialização, leitura do ADC, e controle da bomba[cite: 2].
* `auxiliary_codes/pwm_code.c`: Implementa a função de configuração do PWM para alimentar o sensor[cite: 3].
* `auxiliary_codes/pwm_code.h`: Declaração da função de configuração do PWM.

## Lógica de Operação Detalhada

O sistema opera com base na leitura da tensão do sensor de umidade do solo, que é convertido por um ADC (Conversor Analógico-Digital) no Raspberry Pi Pico[cite: 2].

* **Valores de Referência**:
    * **Tensão $\ge$ 2.5V**: Indica que o solo está seco[cite: 2].
        * **Ação**: A bomba d'água é ligada por 9 segundos[cite: 2]. A *flag* `motor_desligado` é resetada para permitir futuras operações de desligamento[cite: 2].
    * **Tensão < 2.5V**: Indica que o solo está úmido[cite: 2].
        * **Ação**: Se a *flag* `motor_desligado` for `0` (indicando que a bomba pode estar ligada ou foi ligada recentemente), a bomba d'água é desligada[cite: 2]. A *flag* `motor_desligado` é então definida como `1` para evitar comandos repetidos de desligamento[cite: 2].

* **Tabela de Referência da Umidade do Solo**:

| Leitura ADC (0-4095) | Tensão (V)   | Condição do Solo | Ação      |
| :------------------- | :----------- | :--------------- | :-------- |
| 0 - 2478             | 0.0 - 1.99   | Muito Úmido      | Bomba OFF [cite: 2] |
| 2479 - 3100          | 2.0 - 2.49   | Úmido Adequado   | Bomba OFF [cite: 2] |
| 3101 - 4095          | 2.5 - 3.3    | Seco             | Bomba ON  [cite: 2] |

