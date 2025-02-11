#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 22
#define LED_RGB_PIN 12
#define PWM_FREQUENCY 50
#define PERIOD_US 20000 // 20ms (50Hz)
#define MIN_PULSE_WIDTH 500   // 0° position
#define MID_PULSE_WIDTH 1470  // 90° position
#define MAX_PULSE_WIDTH 2400  // 180° position
#define SMOOTH_STEP 5         // Incremento para suavização
#define DELAY_MS 10           // Delay entre incrementos

void set_servo_pulse(uint gpio, uint pulse_width_us) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint clock_divider = 64; // Configuração para obter a frequência desejada
    uint wrap_value = (125000000 / (clock_divider * PWM_FREQUENCY)) - 1;
    pwm_set_clkdiv(slice_num, clock_divider);
    pwm_set_wrap(slice_num, wrap_value);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(gpio), (pulse_width_us * wrap_value) / PERIOD_US);
    pwm_set_enabled(slice_num, true);
}

void set_led_brightness(uint pulse_width_us) {
    int brightness = ((pulse_width_us - MIN_PULSE_WIDTH) * 255) / (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH);
    uint slice_num = pwm_gpio_to_slice_num(LED_RGB_PIN);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(LED_RGB_PIN), brightness);
}

void move_servo_smoothly(uint start, uint end) {
    int step = (start < end) ? SMOOTH_STEP : -SMOOTH_STEP;
    for (uint pulse = start; (step > 0) ? (pulse <= end) : (pulse >= end); pulse += step) {
        set_servo_pulse(SERVO_PIN, pulse);
        set_led_brightness(pulse);
        sleep_ms(DELAY_MS);
    }
}

int main() {
    stdio_init_all();
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_RGB_PIN, GPIO_FUNC_PWM);
    uint slice_num_led = pwm_gpio_to_slice_num(LED_RGB_PIN);
    pwm_set_wrap(slice_num_led, 255);
    pwm_set_clkdiv(slice_num_led, 4.0f);
    pwm_set_enabled(slice_num_led, true);

    while (1) {
        set_servo_pulse(SERVO_PIN, MAX_PULSE_WIDTH); // 180°
        set_led_brightness(MAX_PULSE_WIDTH);
        sleep_ms(5000);

        set_servo_pulse(SERVO_PIN, MID_PULSE_WIDTH); // 90°
        set_led_brightness(MID_PULSE_WIDTH);
        sleep_ms(5000);

        set_servo_pulse(SERVO_PIN, MIN_PULSE_WIDTH); // 0°
        set_led_brightness(MIN_PULSE_WIDTH);
        sleep_ms(5000);

        move_servo_smoothly(MIN_PULSE_WIDTH, MAX_PULSE_WIDTH); // Movimento suave 0° -> 180°
        move_servo_smoothly(MAX_PULSE_WIDTH, MIN_PULSE_WIDTH); // Movimento suave 180° -> 0°
    }
}