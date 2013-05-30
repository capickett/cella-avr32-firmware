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

static const gpio_map_t ADC_GPIO_MAP = 
{
	{ADC_LIGHT_PIN, ADC_LIGHT_FUNCTION}
};

void get_entropy(uint8_t *entropy_buf, uint8_t bytes)
{
	int i;
	for (i = 0; i < bytes; ++i) {
		adc_start(ADC);
		entropy_buf[i] = adc_get_value(ADC, ADC_LIGHT_CHANNEL);
	}
}

void entropy_init()
{
	gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) / sizeof(ADC_GPIO_MAP[0]));
	AVR32_ADC.mr |= 0xA << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(ADC);
	adc_enable(ADC, ADC_LIGHT_CHANNEL);
}