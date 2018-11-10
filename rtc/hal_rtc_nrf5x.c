#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cmsis_gcc.h"
#include "cpu/io.h"
#include "hal/rtc.h"


#define __HAL_DISABLE_INTERRUPTS(x)                     \
    do {                                                \
        x = __get_PRIMASK();                            \
        __disable_irq();                                \
    } while(0);

#define __HAL_ENABLE_INTERRUPTS(x)                      \
    do {                                                \
        if (!x) {                                       \
            __enable_irq();                             \
        }                                               \
    } while(0);


static uint32_t counter_high;
static TAILQ_HEAD(hal_timer_qhead, hal_rtc_timer) hal_timer_q;


static void set_ocmp(uint32_t expiry)
{
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
        } else  {
            if (delta_t < (1UL << 24)) {
                NRF_RTC0->CC[0] = expiry & 0x00ffffff;
            } else {
                /* CC too far ahead. Just make sure we set compare far ahead */
                NRF_RTC0->CC[0] = cntr + (1UL << 23);
            }

            NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;
        }
}


static void disable_ocmp(void)
{
	NRF_RTC0->INTENCLR = RTC_INTENCLR_COMPARE0_Msk;
}


static void chk_queue(void)
{
	int32_t delta;
	uint32_t tcntr;
	uint32_t ctx;
	struct hal_rtc_timer *timer;

	/* disable interrupts */
	__HAL_DISABLE_INTERRUPTS(ctx);
	while ((timer = TAILQ_FIRST(&hal_timer_q)) != NULL) {
		tcntr = hal_rtc_time();
		/*
		 * If we are within 3 ticks of RTC, we wont be able to set compare.
		 * Thus, we have to service this timer early.
		 */
		delta = -3;

		if ((int32_t)(tcntr - timer->expiry) >= delta) {
			TAILQ_REMOVE(&hal_timer_q, timer, link);
			timer->link.tqe_prev = NULL;
			timer->cb_fun(timer->arg);
		} else {
			break;
		}
	}

	
	/* Any timers left on queue? If so, we need to set OCMP */
	timer = TAILQ_FIRST(&hal_timer_q);
	if (timer) {
		set_ocmp(timer->expiry);
	} else {
		disable_ocmp();
        }

	__HAL_ENABLE_INTERRUPTS(ctx);
}


void hal_rtc_init(void)
{	
	counter_high = 0;

	/* We only allow to run at 32768Hz clock speed
	 * High counter is used to provide 32-bit timer
	 */
        NRF_RTC0->TASKS_STOP = 1;
	NRF_RTC0->PRESCALER = 0;
        NRF_RTC0->EVENTS_OVRFLW = 0;
	NRF_RTC0->INTENSET = RTC_INTENSET_OVRFLW_Msk;
	NRF_RTC0->TASKS_CLEAR = 1;
	NRF_RTC0->TASKS_START = 1;

	//NVIC_SetPriority(RTC0_IRQn, configKERNEL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(RTC0_IRQn);
}


void hal_rtc_deinit(void)
{	
	NRF_RTC0->TASKS_STOP = 1;
	NVIC_DisableIRQ(RTC0_IRQn);
	NVIC_ClearPendingIRQ(RTC0_IRQn);
}


uint32_t hal_rtc_time(void)
{
	uint32_t low32;
	uint32_t ctx;
	uint32_t tcntr;
	__HAL_DISABLE_INTERRUPTS(ctx);
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
	__HAL_ENABLE_INTERRUPTS(ctx);

	return tcntr;
}


bool hal_rtc_start(struct hal_rtc_timer *timer, uint32_t ticks)
{
	uint32_t tick;
        tick = hal_rtc_time() + ticks;
	return hal_rtc_start_at(timer, tick);
}


bool hal_rtc_start_at(struct hal_rtc_timer *timer, uint32_t tick)
{
	uint32_t ctx;
	struct hal_rtc_timer *entry;
    
	if ((timer == NULL) || (timer->link.tqe_prev != NULL) ||
			(timer->cb_fun == NULL)) {
		return false;
	}
    
	timer->expiry = tick;

	__HAL_DISABLE_INTERRUPTS(ctx);

	if (TAILQ_EMPTY(&hal_timer_q)) {
		TAILQ_INSERT_HEAD(&hal_timer_q, timer, link);
	} else {
		TAILQ_FOREACH(entry, &hal_timer_q, link) {
			if ((int32_t)(timer->expiry - entry->expiry) < 0) {
				TAILQ_INSERT_BEFORE(entry, timer, link);
				break;
			}
		}
		
		if (!entry) {
			TAILQ_INSERT_TAIL(&hal_timer_q, timer, link);
		}
	}

	/* If this is the head, we need to set new OCMP */
	if (timer == TAILQ_FIRST(&hal_timer_q)) {
		set_ocmp(timer->expiry);
	}

	__HAL_ENABLE_INTERRUPTS(ctx);
}


void hal_rtc_stop(struct hal_rtc_timer *timer)
{
	uint32_t ctx;
	int reset_ocmp;
	struct hal_rtc_timer *entry;

	if (timer == NULL) {
		return;
	}

	__HAL_DISABLE_INTERRUPTS(ctx);

	if (timer->link.tqe_prev != NULL) {
		reset_ocmp = 0;
		if (timer == TAILQ_FIRST(&hal_timer_q)) {
			/* If first on queue, we will need to reset OCMP */
			entry = TAILQ_NEXT(timer, link);
			reset_ocmp = 1;
		}

		TAILQ_REMOVE(&hal_timer_q, timer, link);
		timer->link.tqe_prev = NULL;
		if (reset_ocmp) {
			if (entry) {
				set_ocmp(entry->expiry);
			} else {
				disable_ocmp();
			}
		}
	}

	__HAL_ENABLE_INTERRUPTS(ctx);
}


void rtc0_handler(void)
{
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