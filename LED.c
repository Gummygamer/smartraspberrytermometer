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
#define TEMP_SENSOR 4
#define BASE_TEMP 23.0f
#define MAX_TEMP 33.0f
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define HISTORY_SIZE 100

float read_temperature() {
    adc_select_input(TEMP_SENSOR);
    uint16_t raw = adc_read();
    float voltage = raw * 3.3f / (1 << 12);
    float temperature = 27 - (voltage - 0.706) / 0.001721;
    return temperature;
}

void set_led_brightness(float temperature) {
    float brightness = (temperature <= BASE_TEMP) ? 0.0f :
                       (temperature >= MAX_TEMP) ? 1.0f :
                       (temperature - BASE_TEMP) / (MAX_TEMP - BASE_TEMP);
    uint16_t pwm_level = (uint16_t)(brightness * 65535.0f);
    pwm_set_gpio_level(LED_PIN, pwm_level);
}

void save_to_flash(int8_t *history, int count) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0, FLASH_PAGE_SIZE);
    memcpy(buffer, history, count);
    //flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);

    printf("Saved %d temperature differences to flash\n", count);
}

void read_from_flash(int8_t *history, int *count) {
    const int8_t *flash_data = (const int8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    *count = 0;
    
    for (int i = 0; i < HISTORY_SIZE; i++) {
        //if (flash_data[i] == -128) break;  // Use -128 as end marker
        history[i] = flash_data[i];
        (*count)++;
    }

    printf("Read %d temperature differences from flash\n", *count);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    gpio_init(LED_PIN);
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_enabled(slice_num, true);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    int8_t temp_history[HISTORY_SIZE];
    int history_count = 0;

    read_from_flash(temp_history, &history_count);
    while (1) {
        float temp = read_temperature();
        int temp_diff = (int)roundf(temp - BASE_TEMP);
        
        printf("Temperature: %.2f C\n", temp);
        printf("Rounded difference from %.1f C: %d\n", BASE_TEMP, temp_diff);

        set_led_brightness(temp);

        if (history_count == 0 || temp_diff != temp_history[history_count - 1]) {
            if (history_count == HISTORY_SIZE) {
                memmove(temp_history, temp_history + 1, HISTORY_SIZE - 1);
                history_count--;
        }
            temp_history[history_count++] = (int8_t)temp_diff;
            save_to_flash(temp_history, history_count);
    }

        printf("Temperature history: ");
        for (int i = 0; i < history_count; i++) {
            printf("%d ", temp_history[i]);
}
        printf("\n");

        sleep_ms(1000);
    }

    return 0;
}