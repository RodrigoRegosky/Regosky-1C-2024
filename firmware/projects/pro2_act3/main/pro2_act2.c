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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "switch.h"
#include "lcditse0803.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

bool medir;
bool hold;

TaskHandle_t medir_task_handle = NULL;
TaskHandle_t tecla_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

void FuncTimerMedir(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

void FuncTimerTecla(void *param)
{
	vTaskNotifyGiveFromISR(tecla_task_handle, pdFALSE);
}

static void ShowDistanceTask(void *pvParameter)
{
	uint16_t distancia;

	while (true)
	{

		// ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
		}
		else
		{
			LedsOffAll();
			LcdItsE0803Off();
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void TECTask(void *pvParameter)
{
	while (true)
	{
		// ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		int8_t tecla = SwitchesRead();

		if (tecla == SWITCH_1)
		{
			medir = !medir;
		}
		else if (tecla == SWITCH_2)
		{
			hold = !hold;
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();

	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = 1000000,
		.func_p = FuncTimerMedir,
		.param_p = NULL};

	TimerInit(&timer_medir);

	timer_config_t timer_tecla = {
		.timer = TIMER_B,
		.period = 50000,
		.func_p = FuncTimerTecla,
		.param_p = NULL};

	TimerInit(&timer_tecla);

	xTaskCreate(&ShowDistanceTask, "Mostrar_Distancia_con_Leds", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&TECTask, "TEC_1", 512, NULL, 5, &tecla_task_handle);

	TimerStart(timer_medir.timer);
	TimerStart(timer_tecla.timer);
}

/*==================[end of file]============================================*/