#ifndef HAL_SPI_H_INCLUDED
#define HAL_SPI_H_INCLUDED


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void hal_spi_init(void);
void hal_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t cnt);
void hal_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t cnt);

#ifdef __cplusplus
}
#endif
#endif /*HAL_SPI_H_INCLUDED*/


