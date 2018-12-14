#ifndef HAL_SPI_H_INCLUDED
#define HAL_SPI_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hal_spi_instance{
    uint8_t SS_pin;
    uint8_t device_address;
    uint32_t timeout;
};
typedef struct hal_spi_instance hal_spi_instance_t;


void hal_spi_init(void);
void hal_spi_deinit(void);

bool hal_spi_write(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size);
bool hal_spi_read(hal_spi_instance_t *spi_instance, uint8_t *data, uint8_t size);

#ifdef __cplusplus
}
#endif
#endif /*HAL_SPI_H_INCLUDED*/


