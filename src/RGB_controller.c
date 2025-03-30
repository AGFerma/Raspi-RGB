#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "ws2811.h"

// Configuration des LED WS2812B
#define LED_COUNT 30 // Nombre de LED dans le bandeau
#define TARGET_FREQ WS2811_TARGET_FREQ
#define GPIO_PIN 21 // Utilisation de GPIO 21 (PCM_DOUT)
#define DMA 10

ws2811_t ledstring = {
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel = {
        [0] = {
            .gpionum = GPIO_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = WS2811_STRIP_GRB,
            .brightness = 255,
        },
        [1] = {0},
    },
};

void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b)
{
    ledstring.channel[0].leds[index] = (r << 16) | (g << 8) | b;
}

void show_leds()
{
    ws2811_render(&ledstring);
}

int main()
{
    if (ws2811_init(&ledstring) != WS2811_SUCCESS)
    {
        printf("Erreur d'initialisation de WS2812B\n");
        return 1;
    }

    // Allumer les LED en rouge, vert et bleu en boucle
    while (1)
    {
        set_led_color(0, 255, 0, 0); // Rouge
        set_led_color(1, 0, 255, 0); // Vert
        set_led_color(2, 0, 0, 255); // Bleu
        show_leds();
        sleep(1);
    }

    // Libération des ressources
    ws2811_fini(&ledstring);
    return 0;
}