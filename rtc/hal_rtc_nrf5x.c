#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "cmsis_gcc.h"
#include "cpu/io.h"
#include "hal/rtc.h"



static List_t timer_list;

static uint32_t counter_high;



static void set_ocmp(uint32_t expiry) {
	int32_t delta_t;
	uint32_t temp;
	uint32_t cntr;

	NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
	temp = counter_high;

	cntr = NRF_RTC0->COUNTER;
	if (NRF_RTC0->EVENTS_OVRFLW) {
		temp += (1UL << 24);
		cntr = NRF_RTC0->COUNTER;
	}

	temp |= cntr;
	delta_t = (int32_t)(expiry - temp);

	/*
	 * The nrf documentation states that you must set the output
	 * compare to 2 greater than the counter to guarantee an interrupt.
	 * Since the counter can tick once while we check, we make sure
	 * it is greater than 2.
	 */
	if (delta_t < 3) {
		NVIC_SetPendingIRQ(RTC0_IRQn);
	} else {
		if (delta_t < (1UL << 24)) {
			NRF_RTC0->CC[0] = expiry & 0x00ffffff;
		} else {
			/* CC too far ahead. Just make sure we set compare far ahead */
			NRF_RTC0->CC[0] = cntr + (1UL << 23);
		}

		NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;
	}
}


static void disable_ocmp(void) {
	NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
}


static void chk_queue(void) {
	int32_t delta;
	uint32_t tcntr;
	uint32_t ctx;
	struct hal_rtc_timer *timer;

	/* disable interrupts */
	portDISABLE_INTERRUPTS();

	timer = listGET_OWNER_OF_HEAD_ENTRY(&timer_list);
	for(size_t i = 0; i < listCURRENT_LIST_LENGTH( &timer_list ); i++){
		
        tcntr = hal_rtc_time();
        /*
		 * If we are within 3 ticks of RTC, we wont be able to set compare.
		 * Thus, we have to service this timer early.
		 */
		delta = -3;

		if ((int32_t)(tcntr - listGET_LIST_ITEM_VALUE(&(timer->lnode))) >= delta) {
			uxListRemove(&(timer->lnode));
			timer->cb_fun(timer->arg);			
		} else {
			break;
		}
        timer = listGET_LIST_ITEM_OWNER(listGET_NEXT(&(timer->lnode)));
	}

	/* Any timers left on queue? If so, we need to set OCMP */
	if(!listLIST_IS_EMPTY(&timer_list)){
		timer = listGET_OWNER_OF_HEAD_ENTRY(&timer_list);
        set_ocmp(listGET_LIST_ITEM_VALUE(&(timer->lnode)));
	} else {
		disable_ocmp();
	}

	portENABLE_INTERRUPTS();;
}

void hal_rtc_init(void) {
	vListInitialise(&timer_list);

	counter_high = 0;
	/* Start the clock source if not already started*/
	if ((NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) == CLOCK_LFCLKSTAT_STATE_NotRunning) {
		NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos;
		NRF_CLOCK->TASKS_LFCLKSTART = 1;
		while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
		NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	}

	/* We only allow to run at 32768Hz clock speed
	 * High counter is used to provide 32-bit timer
	 */
	NRF_RTC0->TASKS_STOP = 1;
	NRF_RTC0->PRESCALER = 0;
	NRF_RTC0->EVENTS_OVRFLW = 0;
	NRF_RTC0->INTENSET = RTC_INTENSET_OVRFLW_Msk;
	NRF_RTC0->TASKS_CLEAR = 1;
	NRF_RTC0->TASKS_START = 1;

	NVIC_SetPriority(RTC0_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(RTC0_IRQn);
}

void hal_rtc_deinit(void) {
	NRF_RTC0->TASKS_STOP = 1;
	NVIC_DisableIRQ(RTC0_IRQn);
	NVIC_ClearPendingIRQ(RTC0_IRQn);

	NRF_CLOCK->TASKS_LFCLKSTOP = 1;
}

uint32_t hal_rtc_time(void) {
	uint32_t low32;
	uint32_t ctx;
	uint32_t tcntr;
	portDISABLE_INTERRUPTS();
	tcntr = counter_high;
	low32 = NRF_RTC0->COUNTER;

	if (NRF_RTC0->EVENTS_OVRFLW) {
		tcntr += (1UL << 24);
		counter_high = tcntr;
		low32 = NRF_RTC0->COUNTER;
		NRF_RTC0->EVENTS_OVRFLW = 0;
		NVIC_SetPendingIRQ(RTC0_IRQn);
	}

	tcntr |= low32;
	portENABLE_INTERRUPTS();

	return tcntr;
}

bool hal_rtc_start(struct hal_rtc_timer *timer, uint32_t ticks) {
	uint32_t tick;
	tick = hal_rtc_time() + ticks;
	return hal_rtc_start_at(timer, tick);
}

bool hal_rtc_start_at(struct hal_rtc_timer *timer, uint32_t tick) {
	uint32_t ctx;
	struct hal_rtc_timer *entry;

	if ((timer == NULL) ||
		(timer->cb_fun == NULL)) {
		return false;
	}

	portDISABLE_INTERRUPTS();
	
    listSET_LIST_ITEM_VALUE(&(timer->lnode), tick);
    listSET_LIST_ITEM_OWNER(&(timer->lnode), timer);

    vListInitialiseItem( &( timer->lnode ) );
	vListInsert(&timer_list, &(timer->lnode));

	if(timer == listGET_OWNER_OF_HEAD_ENTRY(&timer_list)){
		set_ocmp(listGET_LIST_ITEM_VALUE(&(timer->lnode)));
	}

	portENABLE_INTERRUPTS();
}

void hal_rtc_stop(struct hal_rtc_timer *timer) {
	uint32_t ctx;
	int reset_ocmp;
	struct hal_rtc_timer *entry;

	if (timer == NULL) {
		return;
	}

	portDISABLE_INTERRUPTS();
	reset_ocmp = 0;

	if (timer == listGET_OWNER_OF_HEAD_ENTRY(&timer_list)) {
		/* If first on queue, we will need to reset OCMP */
		entry = listGET_LIST_ITEM_OWNER(listGET_NEXT(&(timer->lnode)));
		reset_ocmp = 1;
	}

	uxListRemove(&(timer->lnode));
	if (reset_ocmp) {
		if (!listLIST_IS_EMPTY(&timer_list)) {
			set_ocmp(listGET_LIST_ITEM_VALUE(&(entry->lnode)));
		} else {
			disable_ocmp();
		}
	}

	portENABLE_INTERRUPTS();
}

void rtc0_handler(void) {
	uint32_t compare, overflow;
	compare = NRF_RTC0->EVENTS_COMPARE[0];

	if (compare) {
		NRF_RTC0->EVENTS_COMPARE[0] = 0;
	}

	overflow = NRF_RTC0->EVENTS_OVRFLW;
	if (overflow) {
		NRF_RTC0->EVENTS_OVRFLW = 0;
		counter_high += (1UL << 24);
	}

	chk_queue();

	/* Recommended by nordic to make sure interrupts are cleared */
	compare = NRF_RTC0->EVENTS_COMPARE[0];
}