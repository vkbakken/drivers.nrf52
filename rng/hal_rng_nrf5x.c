#include <stdint.h>

#include "cpu/io.h"
#include "hal/rng.h"

uint8_t hal_rng_do8(void) {
	uint8_t seed;
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed = (uint8_t)NRF_RNG->VALUE;
	NRF_RNG->TASKS_STOP = 0x1;
	return seed;
}

uint16_t hal_rng_do16(void) {
	uint16_t seed;
	/*Round 1*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed = (uint8_t)NRF_RNG->VALUE;
	seed <<= 8;
	/*Round 2*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed |= (uint8_t)NRF_RNG->VALUE;

	NRF_RNG->TASKS_STOP = 0x1;
	return seed;
}

uint32_t hal_rng_do32(void) {
	uint32_t seed;
	/*Round 1*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed = (uint8_t)NRF_RNG->VALUE;
    seed <<= 8;
	/*Round 2*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed |= (uint8_t)NRF_RNG->VALUE;
	seed <<= 8;
	/*Round 3*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed |= (uint8_t)NRF_RNG->VALUE;
	seed <<= 8;
	/*Round 4*/
	NRF_RNG->EVENTS_VALRDY = 0;
	NRF_RNG->TASKS_START = 0x1;
	while (NRF_RNG->EVENTS_VALRDY == 0) {
		;
	}
	seed |= (uint8_t)NRF_RNG->VALUE;

	NRF_RNG->TASKS_STOP = 0x1;
	return seed;
}