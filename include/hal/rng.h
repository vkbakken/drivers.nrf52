#ifndef HAL_RNG_H_INCLUDED
#define HAL_RNG_H_INCLUDED


#include <stdint.h>


uint8_t hal_rng_do8(void);
uint16_t hal_rng_do16(void);
uint32_t hal_rng_do32(void);
#endif /*HAL_RNG_H_INCLUDED*/
