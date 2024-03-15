/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define ON 00000001
#define OFF 00000010
#define TOGGLE 00000011
/*==================[internal data definition]===============================*/

struct leds             //Mi estruct es de tipo leds
{
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;    //indica el tiempo de cada ciclo
	uint8_t mode;       //ON, OFF, TOGGLE
} my_leds;              //Creo un struct que se llame my_leds


/*==================[internal functions declaration]=========================*/

void diagrama_de_flujo( struct leds *variable ){
	LedsInit();

	if (variable->mode==ON)
	{
		if (variable->n_led==1){
			LedOn(LED_1);
		}else if (variable->n_led==2){
			LedOn(LED_2);
		}else if (variable->n_led==3){
			LedOn(LED_3);
		}		
	}else if (variable->mode==OFF)
	{
		if (variable->n_led==1){
			LedOff(LED_1);
		}else if (variable->n_led==2){
			LedOff(LED_2);
		}else if (variable->n_led==3){
			LedOff(LED_3);
		}
	}else if (variable->mode==TOGGLE)
	{
		while (/* condition */)
		{
			/* code */
		}
		


	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit(); //inicializa
	SwitchesInit(); //
    while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;
			case SWITCH_1|SWITCH_2:
    			LedToggle(LED_3);
    		break;
    	}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}



