#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu/io.h"
#include "hal/rtc.h"

void (*internal_timeout_func)(void);
void (*internal_overflow_func)(void);

void hal_rtc_init(void)
{
	/* Start the clock source if not already started*/
	if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) == CLOCK_LFCLKSTAT_STATE_NotRunning) {
		NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos;
		NRF_CLOCK->TASKS_LFCLKSTART = 1;
		while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
			;
		NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	}

	/* We only allow to run at 32768Hz clock speed
	 * High counter is used to provide 32-bit timer
	 */
	NRF_RTC0->TASKS_STOP = 1;
	NRF_RTC0->PRESCALER = 0;
	NRF_RTC0->TASKS_CLEAR = 1;
	NRF_RTC0->TASKS_START = 1;

	NVIC_SetPriority(RTC0_IRQn, 0);
	NVIC_EnableIRQ(RTC0_IRQn);
}

void hal_rtc_deinit(void)
{
	NRF_RTC0->TASKS_STOP = 1;
	NVIC_DisableIRQ(RTC0_IRQn);
	NVIC_ClearPendingIRQ(RTC0_IRQn);

	NRF_CLOCK->TASKS_LFCLKSTOP = 1;
}

uint32_t hal_rtc_get_tick(void)
{
	return NRF_RTC0->COUNTER;
}

void hal_rtc_clear_tick(void)
{
	NRF_RTC0->TASKS_CLEAR;
}

bool hal_rtc_set_periodic(void (*timeout_func)(void), uint32_t ticks)
{
	if (ticks & 0x00FFFFFF != 0) {
		return false;
	}
	uint32_t tick;
	tick = hal_rtc_get_tick() + ticks;
	return hal_rtc_set_compare(timeout_func, tick);
}

bool hal_rtc_set_compare(void (*timeout_func)(void), uint32_t tick)
{
	if (NULL == timeout_func) {
		return false;
	}

	internal_timeout_func = timeout_func;

	NRF_RTC0->CC[0] = 0x00FFFFFF & tick;
	NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;
	return true;
}

void hal_rtc_enable_overflow(void (*overflow_func)(void))
{
	internal_overflow_func = overflow_func;

	NRF_RTC0->EVENTS_OVRFLW = 0;
	NRF_RTC0->INTENSET = RTC_INTENSET_OVRFLW_Msk;
}

void hal_rtc_disable_overflow(void (*overflow_func)(void))
{
	internal_overflow_func = NULL;

	NRF_RTC0->EVENTS_OVRFLW = 0;
	NRF_RTC0->INTENCLR = RTC_INTENSET_OVRFLW_Msk;
}

void rtc0_handler(void)
{
	if (NRF_RTC0->EVENTS_COMPARE[0]) {
		NRF_RTC0->EVENTS_COMPARE[0] = 0;
		NRF_RTC0->INTENCLR = RTC_INTENSET_COMPARE0_Msk;
		if (internal_timeout_func)
			internal_timeout_func();
	}

	if (NRF_RTC0->EVENTS_OVRFLW) {
		NRF_RTC0->EVENTS_OVRFLW = 0;
		NRF_RTC0->INTENCLR = RTC_INTENSET_OVRFLW_Msk;
		if (internal_overflow_func)
			internal_overflow_func();
	}
}