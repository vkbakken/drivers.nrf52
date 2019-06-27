#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu/io.h"
#include "hal/rf_ble.h"

static uint8_t m_adv_pdu[40] = {
	0x22,
	25,
	0,

	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,

	0x02,
	0x01,
	0x06,

	0x03,
	0x03,
	0xaa,
    0xfe,

    11,
    0x16,
	0xaa,
    0xfe,
    0x10,
    0xd8,

	0x02,
	'o','n','i','o',
	0
};

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

uint8_t ch = 37;
void send_done(void)
{
	++ch;
	if (ch == 40) {
		ch = 37;
		hal_rf_ble_pwr_off();
	} else {
		hal_rf_ble_send_adv(ch, m_adv_pdu, send_done);
	}
}

void main(void)
{
	/*Initialize board*/
	power_n_clock_init();

	
	hal_rf_ble_address(m_adv_pdu + 3);
    hal_rf_ble_set_tx_pwr(RADIO_TXPOWER_TXPOWER_Neg40dBm);

	while (1) {
		hal_rf_ble_pwr_on();
		hal_rf_ble_send_adv(ch, m_adv_pdu, &send_done);
		uint32_t tmp = 500000;
		while (tmp--) {
		}
	}
}