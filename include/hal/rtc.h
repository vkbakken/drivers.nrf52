#ifndef HAL_RTC_H_INCLUDED
#define HAL_RTC_H_INCLUDED


#include <stdint.h>
#include "cmsis_gcc.h"

#ifdef __cplusplus
extern "C" {
#endif


void hal_rtc_init(void);
void hal_rtc_deinit(void);

uint32_t hal_rtc_get_tick(void);
void hal_rtc_clear_tick(void);

bool hal_rtc_set_periodic(void (*timeout_func)(void), uint32_t ticks);
bool hal_rtc_set_compare(void (*timeout_func)(void), uint32_t tick);


#ifdef __cplusplus
}
#endif
#endif /*HAL_RTC_H_INCLUDED*/
