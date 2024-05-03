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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

bool medir;
bool hold;
uint8_t teclaPC;

TaskHandle_t medir_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

void FuncTimerMedir(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}


void ControlarMedir()
{
	medir = !medir;
}

void ControlarHold()
{
	hold = !hold;
}


void FuncControlarEDU(void *param)
{
		UartReadByte(UART_PC, &teclaPC);

		if (teclaPC == 'O')
		{
			ControlarMedir();
		}
		else if (teclaPC == 'H')
		{
			ControlarHold();
		}

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
				UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
				UartSendString(UART_PC, " cm\r\n");
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


/*==================[external functions definition]==========================*/
void app_main(void)
{
	
	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = 1000000,
		.func_p = FuncTimerMedir,
		.param_p = NULL};

	TimerInit(&timer_medir);

	serial_config_t puertoserie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = FuncControlarEDU,
		.param_p = NULL};

	UartInit(&puertoserie);

	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	SwitchActivInt(SWITCH_1, ControlarMedir, NULL);
	SwitchActivInt(SWITCH_2, ControlarHold, NULL);

	xTaskCreate(&ShowDistanceTask, "Mostrar_Distancia_con_Leds", 2048, NULL, 5, &medir_task_handle);

	
	TimerStart(timer_medir.timer);
	
	}

	/*==================[end of file]============================================*/