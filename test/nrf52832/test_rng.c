#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/io.h"
#include "hal/rng.h"

static uint8_t random8;
static uint16_t random16;
static uint32_t random32;



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

void main(void)
{
	/*Initialize board*/
	power_n_clock_init();

	while (1) {
		random8 = hal_rng_do8();
        random16 = hal_rng_do16();
        random32 = hal_rng_do32();

		uint32_t tmp = 500000;
		while (tmp--) {
		}
	}
}