/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{

	uint8_t aux_digito;
	uint8_t aux_resto;

	for (int i = 0; i < (digits); i++)
	{
		aux_resto = data % 10;
		aux_digito = data / 10;
		bcd_number[(digits - 1) - i] = aux_resto;
		data = aux_digito;
	}
}

void config_BCD_GPIO(uint8_t dig, gpioConf_t *vec)
{
	for (int i = 0; i < 4; i++)
	{
		if (dig & 1 << i)
		{
			GPIOOn(vec[i].pin);
		}
		else
		{
			GPIOOff(vec[i].pin);
		}
	}
}

void mostrar_por_display(uint32_t dato, uint8_t dig, gpioConf_t *vec, gpioConf_t *vecmap)
{

	uint8_t vector_num[dig];
	convertToBcdArray(dato, dig, vector_num);

	for (int i = 0; i < dig; i++)
	{
		config_BCD_GPIO(vector_num[i], vec);
		GPIOOn(vecmap[i].pin);
		GPIOOff(vecmap[i].pin);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	gpioConf_t vector[4] = {
		{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};

	gpioConf_t vectormap[3] = {
		{GPIO_19, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};

	for (int i = 0; i < 4; i++)
	{
		GPIOInit(vector[i].pin, vector[i].dir);
	}

	for (int i = 0; i < 3; i++)
	{
		GPIOInit(vectormap[i].pin, vectormap[i].dir);
	}

	uint32_t dato = 123;
	uint8_t digits = 3;

	mostrar_por_display(dato, digits, vector, vectormap);
}
/*==================[end of file]============================================*/