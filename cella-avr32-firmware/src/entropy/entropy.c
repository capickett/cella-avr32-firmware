/*
 * entropy.c
 *
 * Created: 5/28/2013 4:49:11 PM
 *  Author: administrator
 */ 

#include <asf.h>
#include "entropy.h"
#include "delay.h"

#define ADC						(&AVR32_ADC)
#define MS_DELAY				10

static const gpio_map_t ADC_GPIO_MAP = 
{
	{ADC_LIGHT_PIN, ADC_LIGHT_FUNCTION}
};

bool get_entropy(uint8_t *entropy_buf, uint8_t bytes)
{
	adc_start(ADC);
	int i;
	for (i = 0; i < bytes/2; ++i) {
		delay_ms(MS_DELAY);
		entropy_buf[2*i] = adc_get_value(ADC, ADC_LIGHT_CHANNEL);
	}
}

void entropy_init()
{
	sysclk_enable_pbb_module(SYSCLK_ADC);
	gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) / sizeof(ADC_GPIO_MAP[0]));
	adc_configure(ADC);
	adc_enable(&AVR32_ADC, ADC_LIGHT_CHANNEL);
}