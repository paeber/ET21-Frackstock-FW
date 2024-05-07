//
//
//
//

#ifndef GPIO_H
#define GPIO_H

#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define BUILTIN_LED_PIN     PICO_DEFAULT_LED_PIN
#define BUTTON_PIN          4    

void GPIO_init();
void GPIO_Button_Tick();
void GPIO_LED_toggle();
void GPIO_LED_set(uint8_t state);
uint16_t GPIO_Button_getStates();

#endif // GPIO_H