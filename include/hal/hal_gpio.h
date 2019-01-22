#ifndef HAL_GPIO_H_INCLUDED
#define HAL_GPIO_H_INCLUDED


#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GPIO_INPUT = 0,
	GPIO_OUTPUT
} gpio_dir_t;


typedef enum {
	GPIO_NOPULL = 0,
	GPIO_PULL_DOWN,
	GPIO_PULL_UP,
} gpio_pull_t;


typedef enum {
	GPIO_INT_RISING = 0,
	GPIO_INT_FALLING,
} gpio_interrupt_signal_t;

/**
 * @brief 
 * 
 * @param pin 
 * @param dir 
 * @param pull 
 */
void hal_gpio_config(uint8_t pin, gpio_dir_t dir, gpio_pull_t pull);

/**
 * @brief 
 * 
 * @param pin 
 * @return true 
 * @return false 
 */
bool hal_gpio_input_read(uint8_t pin);

/**
 * @brief 
 * 
 * @param pin 
 * @param value 
 */
void hal_gpio_output_write(uint8_t pin, bool value);

/**
 * @brief 
 * 
 * @param pin 
 */
void hal_gpio_output_toggle(uint8_t pin);

/**
 * @brief 
 * 
 * @param pin 
 * @param interrupt_signal 
 * @param callback 
 */
bool hal_gpio_attach_interrupt(uint8_t pin, gpio_interrupt_signal_t interrupt_signal, void (*callback)(void));


#ifdef __cplusplus
}
#endif
#endif /*HAL_GPIO_H_INCLUDED*/