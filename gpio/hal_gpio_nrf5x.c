#include <cpu/io.h>
#include <hal/hal_gpio.h>
#include <stddef.h>

#define NRF_GPIO_PORT(port_) ((NRF_GPIO_Type *)port_)

#define NRF_GPIOTE_CH_NUM 8
#define NRF_GPIOTE_INT_IN_MASK (GPIOTE_INTENSET_IN0_Msk | GPIOTE_INTENSET_IN1_Msk | \
								GPIOTE_INTENSET_IN2_Msk | GPIOTE_INTENSET_IN3_Msk | \
								GPIOTE_INTENSET_IN4_Msk | GPIOTE_INTENSET_IN5_Msk | \
								GPIOTE_INTENSET_IN6_Msk | GPIOTE_INTENSET_IN7_Msk)

typedef struct {
	bool installed;
	uint8_t pin_assigned;
	interrupt_callback_t *handler;
} gpiote_controller_t;

static gpiote_controller_t gpiote_controller[NRF_GPIOTE_CH_NUM];

static bool interrupt_enabled = false;

__STATIC_INLINE gpiote_controller_t *hal_gpiote_nrfx_get_free_channel(void) {
	for (uint8_t i = 0; i < NRF_GPIOTE_CH_NUM; ++i) {
		if (!gpiote_controller[i].installed)
			return &gpiote_controller[i];
	}
	return NULL;
}

__STATIC_INLINE gpiote_controller_t * hal_gpiote_nrfx_check_pin_registered(uint8_t pin_number){
	for (uint8_t i = 0; i < NRF_GPIOTE_CH_NUM; ++i) {
		if (gpiote_controller[i].installed && gpiote_controller[i].pin_assigned == pin_number)
			return &gpiote_controller[i];
	}
	return NULL;
}

__STATIC_INLINE void hal_gpiote_nrfx_configure(uint32_t idx, uint32_t pin_number, gpio_interrupt_signal_t interrupt_signal) {
	uint32_t polarity = 1UL + interrupt_signal;
	NRF_GPIOTE->CONFIG[idx] &= ~(GPIOTE_CONFIG_PSEL_Msk | GPIOTE_CONFIG_POLARITY_Msk);
	NRF_GPIOTE->CONFIG[idx] |= ((pin_number << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PSEL_Msk) |
							   ((polarity << GPIOTE_CONFIG_POLARITY_Pos) & GPIOTE_CONFIG_POLARITY_Msk);
	NRF_GPIOTE->CONFIG[idx] |= GPIOTE_CONFIG_MODE_Event;
}

__STATIC_INLINE uint32_t hal_gpiote_nrfx_int_is_enabled(uint32_t mask) {
	return (NRF_GPIOTE->INTENSET & mask);
}

__STATIC_INLINE void hal_gpio_interrupt_enable(void) {
	NVIC_SetPriority(GPIOTE_IRQn, 0);
	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_EnableIRQ(GPIOTE_IRQn);
	NRF_GPIOTE->EVENTS_PORT = 0UL;
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
}

__STATIC_INLINE void hal_gpio_interrupt_disable(void) {
	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_DisableIRQ(GPIOTE_IRQn);
}

void hal_gpio_config(void * port, uint8_t pin_number, gpio_dir_t dir, gpio_pull_t pull) {
	NRF_GPIO_PORT(port)->PIN_CNF[pin_number] = (dir << GPIO_PIN_CNF_DIR_Pos) |
											   (pull << GPIO_PIN_CNF_PULL_Pos) |
											   (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
											   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
											   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

bool hal_gpio_pin_read(void * port, uint8_t pin_number) {
	return (NRF_GPIO_PORT(port)->IN >> pin_number) & 0x1UL;
}

void hal_gpio_pin_set(void * port, uint8_t pin_number) {
	NRF_GPIO_PORT(port)->OUTSET = 0x1UL << pin_number;
}

void hal_gpio_pin_clear(void * port, uint8_t pin_number) {
	NRF_GPIO_PORT(port)->OUTCLR = 0x1UL << pin_number;
}

void hal_gpio_pin_toggle(void * port, uint8_t pin_number) {
	uint32_t port_state = NRF_GPIO_PORT(port)->OUT;

	NRF_GPIO_PORT(port)->OUTSET = (~port_state & (0x1UL << pin_number));
	NRF_GPIO_PORT(port)->OUTCLR = (port_state & (0x1UL << pin_number));
}

bool hal_gpio_install_interrupt(void * port, uint8_t pin_number, gpio_pull_t pull, gpio_interrupt_signal_t interrupt_signal, interrupt_callback_t *callback) {
	gpiote_controller_t *tmp_controller = NULL;

	if (NULL == hal_gpiote_nrfx_check_pin_registered(pin_number)) {
		NRF_GPIO_PORT(port)->PIN_CNF[pin_number] = (GPIO_INPUT << GPIO_PIN_CNF_DIR_Pos) |
												   ((pull == GPIO_PULL_UP ? pull + 1 : pull) << GPIO_PIN_CNF_PULL_Pos) |
												   (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
												   (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
												   (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

		tmp_controller = hal_gpiote_nrfx_get_free_channel();

		if (tmp_controller != NULL && callback != NULL) {
			tmp_controller->pin_assigned = pin_number;
			tmp_controller->handler = callback;
			tmp_controller->installed = true;

			uint32_t channel_index = (tmp_controller - gpiote_controller);

			hal_gpiote_nrfx_configure(channel_index, pin_number, interrupt_signal);

			NRF_GPIOTE->EVENTS_IN[channel_index] = 0UL;
			NRF_GPIOTE->INTENSET = 1UL << channel_index;

			if (!interrupt_enabled) {
				hal_gpio_interrupt_enable();
				interrupt_enabled = true;
			}

			return true;
		}
	}
	return false;
}

bool hal_gpio_uninstall_interrupt(void * port, uint8_t pin_number) {
	gpiote_controller_t *tmp_controller = hal_gpiote_nrfx_check_pin_registered(pin_number);
	if (NULL != tmp_controller) {
		tmp_controller->pin_assigned = 0;
		tmp_controller->handler = NULL;
		tmp_controller->installed = false;

        uint32_t channel_index = (tmp_controller - gpiote_controller);
		NRF_GPIOTE->EVENTS_IN[channel_index] = 0UL;
		NRF_GPIOTE->INTENCLR = 1UL << channel_index;
        /* Disable IRQ if there are not any channels enable */
		if (!(NRF_GPIOTE->INTENSET & NRF_GPIOTE_INT_IN_MASK) && interrupt_enabled) {
			hal_gpio_interrupt_disable();
			interrupt_enabled = false;
		}
		return true;
	}
	return false;
}

uint32_t hal_gpio_port_read(void * port) {
	return NRF_GPIO_PORT(port)->IN;
}

void hal_gpio_port_set(void * port, bool value) {
	NRF_GPIO_PORT(port)->OUTSET = value;
}

void hal_gpio_port_clear(void * port, bool value) {
	NRF_GPIO_PORT(port)->OUTCLR = value;
}

void hal_gpio_port_toggle(void * port) {
	uint32_t port_state = NRF_GPIO_PORT(port)->OUT;

	NRF_GPIO_PORT(port)->OUTSET = (~port_state & 0xFFFFFFFF);
	NRF_GPIO_PORT(port)->OUTCLR = (port_state & 0xFFFFFFFF);
}

void gpiote_handler(void) {
	uint32_t status = 0;
	uint32_t mask = GPIOTE_INTENSET_IN0_Msk;

	for (uint8_t i = 0; i < NRF_GPIOTE_CH_NUM; i++) {
		if (NRF_GPIOTE->EVENTS_IN[i] && hal_gpiote_nrfx_int_is_enabled(mask)) {
			NRF_GPIOTE->EVENTS_IN[i] = 0UL;
			status |= mask;
		}
		mask <<= 1;
	}

	if (status & NRF_GPIOTE_INT_IN_MASK) {
		mask = GPIOTE_INTENSET_IN0_Msk;
		for (uint8_t i = 0; i < NRF_GPIOTE_CH_NUM; i++) {
			if (mask & status) {
				if (gpiote_controller[i].installed && gpiote_controller[i].handler) {
					gpiote_controller[i].handler();
				}
			}
			mask <<= 1;
		}
	}
}
