#ifndef HAL_SPI_H_INCLUDED
#define HAL_SPI_H_INCLUDED


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

enum hal_spi_error_code{
    success =0,
    timeout,
}
typedef enum hal_spi_error_code hal_spi_error_code_t;


void hal_spi_init(void);
void hal_spi_deinit(void);
hal_spi_error_code_t hal_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t cnt, uint32_t timeout);
hal_spi_error_code_t hal_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t cnt, uint32_t timeout);

#ifdef __cplusplus
}
#endif
#endif /*HAL_SPI_H_INCLUDED*/


