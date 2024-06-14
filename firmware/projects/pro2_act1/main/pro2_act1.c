/*! @mainpage Proyecto 2 - Actividad 1
 *
 * \section genDesc General Description
 *
 * El programa enciende determinados leds segun la distancia que mide el sensor de ultrasonido HC-SR04  y muestra el valor po una pantalla LCD
 *
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 14/06/2024 | Document creation		                         |
 *
 * @author Rodrigo Ivan Regosky 
 *
 */


/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500

/*==================[internal data definition]===============================*/

TaskHandle_t led1_task_handle = NULL;
TaskHandle_t led2_task_handle = NULL;
TaskHandle_t led3_task_handle = NULL;


bool medir; /**< variable que guarda si se esta midiendo o no */
bool hold; /**< variable que guarda si se mantiene el valor del display o se cambia */

/*==================[internal functions declaration]=========================*/

/*!
 *
 * @fn static void ShowDistanceTask(void *pvParameter)
 * @brief La funcion mide la distancia y enciende determinado numero de leds segun corresponda. Ademas saca or el display el valor de la distancia
 * @return
 */

//static void
 void ShowDistanceTask(void *pvParameter) /**< La funcion mide la distancia y enciende determinado numero de leds segun corresponda. Ademas saca or el display el valor de la distancia */
{
	uint16_t distancia;

	while (true)
	{
		if (medir == true)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
			if (distancia < 10)
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if ((distancia >= 10) & (distancia < 20))
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if ((distancia >= 20) & (distancia < 30))
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if (distancia >= 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}

			if (hold == false)
			{
				LcdItsE0803Write(distancia);
			}
		}else
		{
			LedsOffAll();
			LcdItsE0803Off();
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/*!
 *
 * @fn static void TECTask(void *pvParameter)
 * @brief la funcion lee e intepreta la lectura de las teclas para meidr o dejar de medir, y para mantener o no el valor del display
 * @return
 */
void TECTask(void *pvParameter) /**< la funcion lee e intepreta la lectura de las teclas para medir o dejar de medir, y para mantener o no el valor del display*/
{
	while (true)
	{
		int8_t tecla = SwitchesRead();

		if (tecla == SWITCH_1)
		{
			medir = !medir;
		}
		else if (tecla == SWITCH_2)
		{
			hold = !hold;
		}

		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();

	xTaskCreate(&ShowDistanceTask, "Mostrar_Distancia_con_Leds", 2048, NULL, 5, NULL);
	xTaskCreate(&TECTask, "TEC_1", 512, NULL, 5, NULL);
}

/*==================[end of file]============================================*/