#ifndef HAL_RTC_H_INCLUDED
#define HAL_RTC_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//struct hal_rtc_timer {
//	void (*cb_fun)(void *arg);
//	void *arg;
//	uint32_t expiry;
//        TAILQ_ENTRY(hal_rtc_timer) link;    /* Queue linked list structure */
//};

void hal_rtc_init(void);
//void hal_rtc_deinit(void);
//
//uint32_t hal_rtc_time(void);
//
//bool hal_rtc_start(struct hal_rtc_timer *timer, uint32_t ticks);
//bool hal_rtc_start_at(struct hal_rtc_timer *timer, uint32_t tick);
//void hal_rtc_stop(struct hal_rtc_timer *timer);

uint32_t hal_rtc_counter_get(void);

void hal_rtc_tick_start(void);
void hal_rtc_event_tick_clear(void);

void hal_rtc_compare_set(uint8_t channel_index, uint32_t compare_value);
void hal_rtc_compare_clear(uint8_t channel_index);

#ifdef __cplusplus
}
#endif
#endif /*HAL_RTC_H_INCLUDED*/