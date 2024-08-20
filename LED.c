#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/pwm.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define TEMP_SENSOR 4  // ADC channel 4 is connected to the internal temperature sensor
#define BASE_TEMP 23.0f  // Base temperature
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)


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
        pwm_set_gpio_level(LED_PIN, bit ? 65535 : 0);  // Full on for 1, off for 0
        sleep_ms(1000);  // Each bit lasts for 1 second
    }
    printf("\n");

    // Turn off LED for 4 seconds after the pattern
    pwm_set_gpio_level(LED_PIN, 0);
    sleep_ms(4000);
}

void save_to_flash(int value) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0, FLASH_PAGE_SIZE);
    buffer[0] = (uint8_t)value;

    // Erase the sector before writing
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);

    // Program the flash
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);

    printf("Saved value %d to flash\n", value);
}

int read_from_flash() {
    return *(uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(LED_PIN);

    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_enabled(slice_num, true);

    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    int last_saved_diff = read_from_flash();
    printf("Last saved temperature difference: %d\n", last_saved_diff);

    while (1) {
        float temp = read_temperature();
        int temp_diff = (int)roundf(temp - BASE_TEMP);
        
        printf("Temperature: %.2f C\n", temp);
        printf("Rounded difference from %.1f C: %d\n", BASE_TEMP, temp_diff);

        if (temp_diff != last_saved_diff) {
            save_to_flash(temp_diff);
            last_saved_diff = temp_diff;
        }

        blink_binary(temp_diff);
    }

    return 0;
}
