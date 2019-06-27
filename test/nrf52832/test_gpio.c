#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/io.h"
#include "hal/hal_gpio.h"
#include "pins.h"

static void power_n_clock_init(void)
{
	/*Using LDO*/
	NRF_POWER->DCDCEN = POWER_DCDCEN_DCDCEN_Disabled;
	NRF_POWER->TASKS_LOWPWR = 0x1;

	/*Enable RTC clock source*/

	/*Low power init - basically all off*/
	NRF_RADIO->POWER = 0;
#if 1
	NRF_POWER->RAM[0].POWERSET = 0x03;
#endif
	NRF_POWER->RAM[0].POWERSET = 0x0300;
	NRF_POWER->RAM[1].POWERCLR = 0x0303;
	NRF_POWER->RAM[2].POWERCLR = 0x0303;
}

static void button1_interrupt(void)
{
	hal_gpio_pin_toggle(NRF_P0, BOARD_LED1_bp);
}

static void button2_interrupt(void)
{
	hal_gpio_pin_toggle(NRF_P0, BOARD_LED2_bp);
}

static void button3_interrupt(void)
{
	hal_gpio_pin_toggle(NRF_P0, BOARD_LED3_bp);
}

static void button4_interrupt(void)
{
	hal_gpio_pin_toggle(NRF_P0, BOARD_LED4_bp);
}

void main(void)
{
	/*Initialize board*/
	power_n_clock_init();

	hal_gpio_config(NRF_P0, BOARD_LED1_bp, GPIO_OUTPUT, GPIO_NOPULL);
    hal_gpio_config(NRF_P0, BOARD_LED2_bp, GPIO_OUTPUT, GPIO_NOPULL);
    hal_gpio_config(NRF_P0, BOARD_LED3_bp, GPIO_OUTPUT, GPIO_NOPULL);
    hal_gpio_config(NRF_P0, BOARD_LED4_bp, GPIO_OUTPUT, GPIO_NOPULL);
	
	if(!hal_gpio_install_interrupt(NRF_P0, BOARD_BUTTON1_bp, GPIO_PULL_UP, GPIO_INT_FALLING, &button1_interrupt)){
		while(1){
		}
	}

    if(!hal_gpio_install_interrupt(NRF_P0, BOARD_BUTTON2_bp, GPIO_PULL_UP, GPIO_INT_FALLING, &button2_interrupt)){
		while(1){
		}
	}

    if(!hal_gpio_install_interrupt(NRF_P0, BOARD_BUTTON3_bp, GPIO_PULL_UP, GPIO_INT_FALLING, &button3_interrupt)){
		while(1){
		}
	}

    if(!hal_gpio_install_interrupt(NRF_P0, BOARD_BUTTON4_bp, GPIO_PULL_UP, GPIO_INT_FALLING, &button4_interrupt)){
		while(1){
		}
	}

	while (1) {
		__WFE();
		// Clear the internal event register.
		__SEV();
		__WFE();
	}
}