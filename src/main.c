/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
#include <asf.h>

int main (void)
{
	board_init();

	LED_On(LED0);
	LED_On(LED1);
	LED_On(LED2);
	LED_On(LED3);
}
