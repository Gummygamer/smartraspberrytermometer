#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

int main()
{
    stdio_init_all();

    // Initialize LED pin
    #ifdef PICO_DEFAULT_LED_PIN
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    #else
    const uint LED_PIN = 25;
    #endif
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    printf("LED should be on constantly\n");

    // Turn on the LED
    gpio_put(LED_PIN, 1);
    while (true) {
        printf("LED is on\n");
        sleep_ms(1000);
    }
}
