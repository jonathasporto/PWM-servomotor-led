#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Definição dos pinos e parâmetros do PWM
#define SERVO_PIN 22          // Pino GPIO conectado ao servo
#define LED_RGB_PIN 12        // Pino GPIO conectado ao LED RGB
#define PWM_FREQUENCY 50      // Frequência do PWM em Hz (50Hz para servos)
#define PERIOD_US 20000       // Período do PWM em microssegundos (20ms para 50Hz)
#define MIN_PULSE_WIDTH 500   // Largura de pulso para 0° (500us)
#define MID_PULSE_WIDTH 1470  // Largura de pulso para 90° (1470us)
#define MAX_PULSE_WIDTH 2400  // Largura de pulso para 180° (2400us)
#define SMOOTH_STEP 5         // Incremento para suavização do movimento
#define DELAY_MS 10           // Delay entre incrementos (em milissegundos)

// Função para configurar o pulso PWM do servo
void set_servo_pulse(uint gpio, uint pulse_width_us) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);  // Obtém o slice do PWM para o pino
    uint clock_divider = 64;                       // Divisor de clock para obter a frequência desejada
    uint wrap_value = (125000000 / (clock_divider * PWM_FREQUENCY)) - 1;  // Calcula o valor de wrap
    pwm_set_clkdiv(slice_num, clock_divider);      // Configura o divisor de clock
    pwm_set_wrap(slice_num, wrap_value);           // Configura o valor de wrap
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), (pulse_width_us * wrap_value) / PERIOD_US);  // Define o nível do canal
    pwm_set_enabled(slice_num, true);              // Habilita o PWM
}

// Função para ajustar o brilho do LED RGB com base na posição do servo
void set_led_brightness(uint pulse_width_us) {
    int brightness = ((pulse_width_us - MIN_PULSE_WIDTH) * 255) / (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH);  // Mapeia a largura do pulso para brilho (0-255)
    uint slice_num = pwm_gpio_to_slice_num(LED_RGB_PIN);  // Obtém o slice do PWM para o LED
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_RGB_PIN), brightness);  // Define o brilho do LED
}

// Função para mover o servo suavemente entre duas posições
void move_servo_smoothly(uint start, uint end) {
    int step = (start < end) ? SMOOTH_STEP : -SMOOTH_STEP;  // Define o passo de incremento (positivo ou negativo)
    for (uint pulse = start; (step > 0) ? (pulse <= end) : (pulse >= end); pulse += step) {  // Loop para mover o servo
        set_servo_pulse(SERVO_PIN, pulse);  // Atualiza a posição do servo
        set_led_brightness(pulse);          // Atualiza o brilho do LED
        sleep_ms(DELAY_MS);                // Aguarda um pequeno delay para suavizar o movimento
    }
}

// Função principal
int main() {
    stdio_init_all();  // Inicializa a comunicação serial (para debug, se necessário)

    // Configuração dos pinos como saída PWM
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);  // Configura o pino do servo como PWM
    gpio_set_function(LED_RGB_PIN, GPIO_FUNC_PWM);  // Configura o pino do LED como PWM

    // Configuração do PWM para o LED RGB
    uint slice_num_led = pwm_gpio_to_slice_num(LED_RGB_PIN);  // Obtém o slice do PWM para o LED
    pwm_set_wrap(slice_num_led, 255);  // Define o valor de wrap para 255 (brilho máximo)
    pwm_set_clkdiv(slice_num_led, 4.0f);  // Configura o divisor de clock para o LED
    pwm_set_enabled(slice_num_led, true);  // Habilita o PWM para o LED

    // Loop principal
    while (1) {
        // Move o servo para 180° e ajusta o LED
        set_servo_pulse(SERVO_PIN, MAX_PULSE_WIDTH);  // Define o pulso para 180°
        set_led_brightness(MAX_PULSE_WIDTH);          // Ajusta o brilho do LED
        sleep_ms(5000);                               // Aguarda 5 segundos

        // Move o servo para 90° e ajusta o LED
        set_servo_pulse(SERVO_PIN, MID_PULSE_WIDTH);  // Define o pulso para 90°
        set_led_brightness(MID_PULSE_WIDTH);          // Ajusta o brilho do LED
        sleep_ms(5000);                               // Aguarda 5 segundos

        // Move o servo para 0° e ajusta o LED
        set_servo_pulse(SERVO_PIN, MIN_PULSE_WIDTH);  // Define o pulso para 0°
        set_led_brightness(MIN_PULSE_WIDTH);          // Ajusta o brilho do LED
        sleep_ms(5000);                               // Aguarda 5 segundos

        // Movimento suave do servo de 0° para 180° e ajusta o LED
        move_servo_smoothly(MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);  // Movimento suave para 180°
        move_servo_smoothly(MAX_PULSE_WIDTH, MIN_PULSE_WIDTH);  // Movimento suave para 0°
    }
}