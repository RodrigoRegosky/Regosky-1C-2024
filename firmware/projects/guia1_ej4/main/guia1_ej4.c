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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

void  convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{

uint8_t aux_digito;
uint8_t aux_resto;


	for (int i = 0; i < (digits); i++)
	{
		aux_resto = data%10;
		aux_digito = data/10;
		bcd_number[(digits-1)-i] = aux_resto;
		data=aux_digito;
	}
	
}



/*==================[external functions definition]==========================*/
void app_main(void){

uint8_t bcd_number[3];
uint32_t data = 456;
uint8_t digits = 3;

convertToBcdArray(data, digits, bcd_number);

for (int i = 0; i < digits; i++)
{
	printf("%d", bcd_number[i]);
}

}
/*==================[end of file]============================================*/