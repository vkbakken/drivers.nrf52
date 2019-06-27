#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/io.h"


static void pin_init(void)
{
	NRF_P0->PIN_CNF[28] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
						  (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
						  (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos) |
						  (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
						  (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

	NRF_P0->PIN_CNF[5] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
						 (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
						 (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
						 (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
						 (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

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

		uint32_t tmp = 500000;
		while (tmp--) {
		}
	}
}