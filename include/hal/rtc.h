#ifndef HAL_RTC_H_INCLUDED
#define HAL_RTC_H_INCLUDED


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

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



/*Segger embedded studio originally has offsetof macro which cannot be used in macros (like STATIC_ASSERT).
  This redefinition is to allow using that. */
#if defined(__SES_ARM) && defined(__GNUC__)
#undef offsetof
#define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#endif
typedef enum
{
    /*lint -save -e30*/
    NRF_RTC_EVENT_TICK        = offsetof(NRF_RTC_Type,EVENTS_TICK),       /**< Tick event. */
    NRF_RTC_EVENT_OVERFLOW    = offsetof(NRF_RTC_Type,EVENTS_OVRFLW),     /**< Overflow event. */
    NRF_RTC_EVENT_COMPARE_0   = offsetof(NRF_RTC_Type,EVENTS_COMPARE[0]), /**< Compare 0 event. */
    NRF_RTC_EVENT_COMPARE_1   = offsetof(NRF_RTC_Type,EVENTS_COMPARE[1]), /**< Compare 1 event. */
    NRF_RTC_EVENT_COMPARE_2   = offsetof(NRF_RTC_Type,EVENTS_COMPARE[2]), /**< Compare 2 event. */
    NRF_RTC_EVENT_COMPARE_3   = offsetof(NRF_RTC_Type,EVENTS_COMPARE[3])  /**< Compare 3 event. */
    /*lint -restore*/
} rtc_event_t;

__STATIC_INLINE  void hal_rtc0_cc_set(uint32_t ch, uint32_t cc_val)
{
    NRF_RTC0->CC[ch] = cc_val;
}

__STATIC_INLINE void hal_rtc0_int_disable(uint32_t mask)
{
    NRF_RTC0->INTENCLR = mask;
}

__STATIC_INLINE void hal_rtc0_event_clear(rtc_event_t event)
{
    *((volatile uint32_t *)((uint8_t *)NRF_RTC0 + (uint32_t)event)) = 0;
    volatile uint32_t dummy = *((volatile uint32_t *)((uint8_t *)NRF_RTC0 + (uint32_t)event));
    (void)dummy;
}

__STATIC_INLINE void hal_rtc0_int_enable(uint32_t mask)
{
    NRF_RTC0->INTENSET = mask;
}

__STATIC_INLINE uint32_t hal_rtc0_counter_get(void)
{
     return NRF_RTC0->COUNTER;
}
#ifdef __cplusplus
}
#endif
#endif /*HAL_RTC_H_INCLUDED*/
