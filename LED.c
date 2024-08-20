#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define TEMP_SENSOR 4  // ADC channel 4 is connected to the internal temperature sensor
#define BASE_TEMP 23.0f  // Base temperature

float read_temperature() {
    adc_select_input(TEMP_SENSOR);
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / (1 << 12);  // Convert to voltage
    float temperature = 27 - (voltage - 0.706) / 0.001721;  // Convert to temperature in Celsius
    return temperature;
}

void set_led_brightness(float brightness) {
    // The PWM counter runs at 125MHz. Dividing by 255 gives us a range of 0-490,196 for the PWM level
    uint16_t pwm_level = (uint16_t)(brightness * 490196.0f);
    pwm_set_gpio_level(LED_PIN, pwm_level);
}

void blink_binary(int value) {
    printf("Binary representation: ");
    for (int i = 7; i >= 0; i--) {  // 8-bit representation
        int bit = (value >> i) & 1;
        printf("%d", bit);
        gpio_put(LED_PIN, bit);  // LED on for 1, off for 0
        sleep_ms(1000);  // Each bit lasts for 1 second
    }
    printf("\n");

    // Turn off LED for 4 seconds after the pattern
    gpio_put(LED_PIN, 0);
    sleep_ms(4000);
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    while (1) {
        float temp = read_temperature();
        int temp_diff = (int)roundf(temp - BASE_TEMP);
        
        printf("Temperature: %.2f C\n", temp);
        printf("Rounded difference from %.1f C: %d\n", BASE_TEMP, temp_diff);

        blink_binary(temp_diff);
    }

    return 0;
}
