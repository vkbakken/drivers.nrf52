#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/io.h"
#include "hal/rtc.h"
#include "pins.h"
#include "hal/hal_gpio.h"

static uint32_t timer;

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

void timeout(void)
{
	timer++;
    hal_gpio_pin_toggle(NRF_P0, BOARD_LED1_bp);
	hal_rtc_set_periodic(&timeout, 32768);
}

void main(void)
{
	/*Initialize board*/
	power_n_clock_init();

	hal_rtc_init();
	hal_rtc_set_periodic(&timeout, 32768);
    hal_gpio_config(NRF_P0, BOARD_LED1_bp, GPIO_OUTPUT, GPIO_NOPULL);

	while (1) {
		__WFE();
		// Clear the internal event register.
		__SEV();
		__WFE();
	}
}