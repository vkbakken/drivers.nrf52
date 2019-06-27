#ifndef HAL_BLE_H_INCLUDED
#define HAL_BLE_H_INCLUDED


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


void hal_rf_ble_pwr_on(void);
void hal_rf_ble_pwr_off(void);
void hal_rf_ble_reset(void);
void hal_rf_ble_address(uint8_t *addr);
void hal_rf_ble_set_tx_pwr(uint8_t pwr);
void hal_rf_ble_send_adv(uint8_t ch, uint8_t *data, void (*cb_done)(void));
void hal_rf_ble_recv(uint8_t ch, uint8_t length, uint8_t *data, void (*cb_done)(void));


#ifdef __cplusplus
}
#endif
#endif /*HAL_RTC_H_INCLUDED*/
