#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmsis_gcc.h"
#include "cpu/io.h"
#include "hal/rf_ble.h"

#include "FreeRTOS.h"

#define __HAL_DISABLE_INTERRUPTS(x) \
	do {                            \
		x = __get_PRIMASK();        \
		__disable_irq();            \
	} while (0);

#define __HAL_ENABLE_INTERRUPTS(x) \
	do {                           \
		if (!x) {                  \
			__enable_irq();        \
		}                          \
	} while (0);

#define HAL_CH_IDX_TO_FREQ_OFFS(index)                                                                                  \
	(((index) == 37) ? (2)                                                                                              \
					 : (((index) == 38) ? (26)                                                                          \
										: (((index) == 39) ? (80)                                                       \
														   : ((/*((index) >= 0) &&*/ ((index) <= 10)) ? ((index)*2 + 4) \
																									  : ((index)*2 + 6)))))

/**@brief The maximum possible length in device discovery mode. */
#define DD_MAX_PAYLOAD_LENGTH (31 + 6)

/**@brief The default SHORTS configuration. */
#define DEFAULT_RADIO_SHORTS                                                 \
	(                                                                        \
		(RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | \
		(RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos))

/**@brief The default CRC init polynominal. 
   @note Written in little endian but stored in big endian, because the BLE spec. prints
         is in little endian but the HW stores it in big endian. */
#define CRC_POLYNOMIAL_INIT_SETTINGS ((0x5B << 0) | (0x06 << 8) | (0x00 << 16))

static uint8_t priv_pwr;
static uint32_t rng_seed;
static void (*fun_cb)(void);

void hal_rf_ble_pwr_on(void) {
	/* Enable HF clock and set RF power on*/
	if ((NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKRUN_STATUS_Msk) == CLOCK_HFCLKSTAT_STATE_NotRunning) {
		NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
		NRF_CLOCK->TASKS_HFCLKSTART = 0x1;
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
	}

	NRF_RADIO->POWER = RADIO_POWER_POWER_Enabled;
}

void hal_rf_ble_reset(void) {
	NRF_RADIO->POWER = RADIO_POWER_POWER_Disabled;
	asm volatile("DMB" ::: "memory");
	NRF_RADIO->POWER = RADIO_POWER_POWER_Enabled;
	asm volatile("DMB" ::: "memory");
}

void hal_rf_ble_pwr_off(void) {
	/* Disable HF clock and RF power off*/
	NRF_RADIO->POWER = RADIO_POWER_POWER_Disabled;
	NVIC_DisableIRQ(RADIO_IRQn);
	NRF_CLOCK->TASKS_HFCLKSTOP = 0x1;
}

void hal_rf_ble_address(uint8_t *addr) {
	uint32_t address0 = NRF_FICR->DEVICEADDR[0];
	uint32_t address1 = NRF_FICR->DEVICEADDR[1];

	addr[0] = (uint8_t)address0;
	address0 >>= 8;
	addr[1] = (uint8_t)address0;
	address0 >>= 8;
	addr[2] = (uint8_t)address0;
	address0 >>= 8;
	addr[3] = (uint8_t)address0;
	addr[4] = (uint8_t)address1;
	address1 >>= 8;
	addr[5] = (uint8_t)address1;
}

void hal_rf_ble_set_tx_pwr(uint8_t pwr) {
	priv_pwr = pwr;
}

void hal_rf_ble_send_adv(uint8_t ch, uint8_t *data, void (*cb_done)(void)) {
	uint32_t ctx;
	__HAL_DISABLE_INTERRUPTS(ctx);
	fun_cb = cb_done;
	NRF_RADIO->PCNF0 = (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | (((2UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) | (((6UL) << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk);

	NRF_RADIO->PCNF1 = (((RADIO_PCNF1_ENDIAN_Little) << RADIO_PCNF1_ENDIAN_Pos) & RADIO_PCNF1_ENDIAN_Msk) | (((3UL) << RADIO_PCNF1_BALEN_Pos) & RADIO_PCNF1_BALEN_Msk) | (((0UL) << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) | ((((uint32_t)DD_MAX_PAYLOAD_LENGTH) << RADIO_PCNF1_MAXLEN_Pos) & RADIO_PCNF1_MAXLEN_Msk) | ((RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos) & RADIO_PCNF1_WHITEEN_Msk);

	NRF_RADIO->CRCPOLY = (uint32_t)CRC_POLYNOMIAL_INIT_SETTINGS;
	NRF_RADIO->CRCCNF = (((RADIO_CRCCNF_SKIPADDR_Skip) << RADIO_CRCCNF_SKIPADDR_Pos) & RADIO_CRCCNF_SKIPADDR_Msk) | (((RADIO_CRCCNF_LEN_Three) << RADIO_CRCCNF_LEN_Pos) & RADIO_CRCCNF_LEN_Msk);
	NRF_RADIO->CRCINIT = 0x555555;

	NRF_RADIO->MODE = ((RADIO_MODE_MODE_Ble_1Mbit) << RADIO_MODE_MODE_Pos) & RADIO_MODE_MODE_Msk;
	NRF_RADIO->MODECNF0 = RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos;

	NRF_RADIO->TIFS = 100;

	NRF_RADIO->TXPOWER = priv_pwr;
	NRF_RADIO->FREQUENCY = HAL_CH_IDX_TO_FREQ_OFFS(ch);
	NRF_RADIO->DATAWHITEIV = ch;

	NRF_RADIO->PREFIX0 = 0x8E;
	NRF_RADIO->BASE0 = 0x89BED600;

	NRF_RADIO->PACKETPTR = (uint32_t)data;

	/*Disable all interrupts*/
	NRF_RADIO->INTENCLR = 0xFFFFffff;
	NRF_RADIO->SHORTS = DEFAULT_RADIO_SHORTS;

	NVIC_SetPriority(RADIO_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);
	NRF_RADIO->INTENSET = (RADIO_INTENSET_DISABLED_Enabled << RADIO_INTENSET_DISABLED_Pos);
	NRF_RADIO->EVENTS_DISABLED = 0;
	NRF_RADIO->TASKS_TXEN = 0x1;
	__HAL_ENABLE_INTERRUPTS(ctx);
}

void hal_rf_ble_recv(uint8_t ch, uint8_t length, uint8_t *data, void (*cb_done)(void)) {
}

void radio_handler(void) {
	NRF_RADIO->EVENTS_DISABLED = 0;
	if (fun_cb) {
		fun_cb();
	}

	asm volatile("DMB" ::: "memory");
}