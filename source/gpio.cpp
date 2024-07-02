//
//
//
//

#include "gpio.h"
#include "main.h"
#include "hardware/gpio.h"

#include "led_ring.h"
#include "frackstock.h"
#include "interrupts.h"
#include "serial.h"

// Global variables
bool GPIO_INT_FLAG = false;
uint16_t button_state = 0x00;
uint8_t increment_allowed = 0;


/**
 * @brief Initializes the GPIO pins for the application.
 * 
 * This function initializes the GPIO pin for the built-in LED and the button.
 * It sets the direction of the pins as output for the LED and input for the button.
 */
void GPIO_init() {
    // Initialize the GPIO pin
    gpio_init(BUILTIN_LED_PIN);
    gpio_set_dir(BUILTIN_LED_PIN, GPIO_OUT);
    
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    // Set the interrupt for the button
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &handle_Interrupts);
}


/**
 * Toggles the state of the built-in LED.
 */
void GPIO_LED_toggle(){
    gpio_get(BUILTIN_LED_PIN) ? gpio_put(BUILTIN_LED_PIN, 0) : gpio_put(BUILTIN_LED_PIN, 1);
}


/**
 * Sets the state of the LED connected to the GPIO pin.
 *
 * @param state The state to set the LED to. Use 1 for ON and 0 for OFF.
 */
void GPIO_LED_set(uint8_t state){
    gpio_put(BUILTIN_LED_PIN, state);
}


/**
 * @brief Get the stored states of the button.
 *
 * This function returns the oldest 16 states of the button as a uint16_t value. 
 * The least significant bit represents the newest state, and the most significant bit represents the oldest state.
 *
 * @return The states of the button as a uint16_t value.
 */
uint16_t GPIO_Button_getStates(){
    return button_state;
}

/**
 * @brief Function to handle button tick event.
 * 
 * This function is responsible for handling the button tick event. It updates the button state, checks if the button is pressed, and performs specific actions based on the button state.
 */
void GPIO_Button_Tick(){
    static uint8_t bit_changes_num = 0;
    static uint8_t bit_changes_en = 1;

    button_state = button_state << 1;
    if(gpio_get(BUTTON_PIN))
    {
        button_state |= 0x01;

        if(increment_allowed && !(button_state >> 1 & 0x01))
        {
            increment_allowed = 0;
            FRACK_inc_beer();
            SEG_set_mode(SEG_MODE_CUSTOM);
            SEG_write_number(frackstock.beer);
        }

        if(button_state == 0x7FFF)
        {
            increment_allowed = 10;
            LED_Ring_set_color(0,255,0);
            LED_Ring_set_mode(LED_MODE_ON);
            
        } else if (button_state == 0x00FF) {
            LED_Ring_set_color(255,0,0);
            LED_Ring_set_mode(LED_MODE_BLINK);
            SEG_set_mode(SEG_MODE_CUSTOM);
            SEG_write_number(frackstock.beer);
        }  
        
    }

    if(increment_allowed){
        increment_allowed--;
    }

    if(button_state == 0){
        GPIO_INT_FLAG = false;
    }

    if((button_state & 0x8000) && bit_changes_en){
        // check number of bit changes in the last 16 states
        uint16_t temp = button_state;
        bit_changes_en = 0; // reset bit changes enable

        for(int i = 0; i < 15; i++){
            if((temp & 0x01) != ((temp >> 1) & 0x01)){
                bit_changes_num++;
            }
            temp = temp >> 1;
        }
        SERIAL_printf("Bit changes: %d\n", bit_changes_num);
        if(bit_changes_num >= 3){
            if(LED_Ring_get_mode_restore() == (uint8_t)LED_MODE_OFF){
                LED_Ring_set_mode_restore((eLED_MODE)FRACK_get_led_mode());
            } else {
                LED_Ring_set_mode_restore(LED_MODE_OFF);
            }
        }

    } else if (button_state == 0) {
        bit_changes_num = 0;
        bit_changes_en = 1;
    }
}