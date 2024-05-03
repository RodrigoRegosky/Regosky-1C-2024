/*! @mainpage Proyecto 1 - Ejercicio 6
 *
 * \section genDesc General Description
 *
 * La resolucion del Ejercicio 6 es una función que recibe un dato de 32 bits, la cantidad de dígitos de salida y dos vectores de estructuras del tipo gpioConf_t. 
 * La función permite mostrar por display el valor que recibe.
 * 
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 *
 * @author Rodrigo Ivan Regosky 
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

/*!
 * 
 * @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 * @brief Funcion que convierte un numero entero a formato BCD y guarda cada digito convertido en un vector
 * @param data El numero que se quiere convertir a BCD
 * @param digits La cantidad de digitos del numero a convertir
 * @param bcd_number Puntero a vector donde se almacena cada digito del numero pasado a BCD
 * @return 
 */

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

/*!
 * 
 * @fn void config_BCD_GPIO(uint8_t dig, gpioConf_t *vec)
 * @brief Funcion que cambia el estado de un vector de GPIO según el estado del bit correspondiente en un digito BCD que recibe
 * @param dig Digito BCD
 * @param vec Vector que mapea los GPIO 20, 21, 22 y 23
 * @return 
 */


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

/*!
 * 
 * @fn void mostrar_por_display(uint32_t dato, uint8_t dig, gpioConf_t *vec, gpioConf_t *vecmap)
 * @brief Funcion que muestra por Display un numero de 32 bits 
 * @param dato Numero que se desea mostrar por Display 
 * @param dig Cantidad de digitos del numero ingresado
 * @param vec Vector que mapea los GPIO 20, 21, 22 y 23
 * @param vecmap Vector que mapea los GPIO 9, 18 y 19
 * @return 
 */

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