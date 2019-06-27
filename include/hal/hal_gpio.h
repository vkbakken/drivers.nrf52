#ifndef HAL_GPIO_H_INCLUDED
#define HAL_GPIO_H_INCLUDED


#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GPIO_INPUT = 0UL,
	GPIO_OUTPUT
} gpio_dir_t;

typedef enum {
	GPIO_NOPULL = 0UL,
	GPIO_PULL_DOWN,
	GPIO_PULL_UP,
} gpio_pull_t;

typedef enum {
	GPIO_INT_RISING = 0UL,
	GPIO_INT_FALLING,
} gpio_interrupt_signal_t;

typedef void (interrupt_callback_t)(void);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @param dir 
 * @param pull 
 */
void hal_gpio_config(void * port, uint8_t pin, gpio_dir_t dir, gpio_pull_t pull);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @return true 
 * @return false 
 */
bool hal_gpio_pin_read(void * port, uint8_t pin);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @param value 
 */
void hal_gpio_pin_set(void * port, uint8_t pin);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @param value 
 */
void hal_gpio_pin_clear(void * port, uint8_t pin);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 */
void hal_gpio_pin_toggle(void * port, uint8_t pin);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @param interrupt_signal 
 * @param callback 
 * @return true 
 * @return false 
 */
bool hal_gpio_install_interrupt(void * port, uint8_t pin, gpio_pull_t pull, gpio_interrupt_signal_t interrupt_signal, interrupt_callback_t * callback);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @return true 
 * @return false 
 */
bool hal_gpio_uninstall_interrupt(void * port, uint8_t pin);

/**
 * @brief 
 * 
 * @param port 
 * @param pin 
 * @return uint32_t 
 */
uint32_t hal_gpio_port_read(void * port);

/**
 * @brief 
 * 
 * @param port 
 * @param value 
 */
void hal_gpio_port_set(void * port, bool value);

/**
 * @brief 
 * 
 * @param port 
 * @param value 
 */
void hal_gpio_port_clear(void * port, bool value);

/**
 * @brief 
 * 
 * @param port 
 */
void hal_gpio_port_toggle(void * port);

#ifdef __cplusplus
}
#endif
#endif /*HAL_GPIO_H_INCLUDED*/
