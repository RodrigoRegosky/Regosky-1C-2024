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
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

// bool medir;
// bool hold;
// uint8_t teclaPC;
uint16_t medicion_Osc;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t ECG_task_handle = NULL;

TaskHandle_t main_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/

const char ecg[BUFFER_SIZE] = {
	76,
	77,
	78,
	77,
	79,
	86,
	81,
	76,
	84,
	93,
	85,
	80,
	89,
	95,
	89,
	85,
	93,
	98,
	94,
	88,
	98,
	105,
	96,
	91,
	99,
	105,
	101,
	96,
	102,
	106,
	101,
	96,
	100,
	107,
	101,
	94,
	100,
	104,
	100,
	91,
	99,
	103,
	98,
	91,
	96,
	105,
	95,
	88,
	95,
	100,
	94,
	85,
	93,
	99,
	92,
	84,
	91,
	96,
	87,
	80,
	83,
	92,
	86,
	78,
	84,
	89,
	79,
	73,
	81,
	83,
	78,
	70,
	80,
	82,
	79,
	69,
	80,
	82,
	81,
	70,
	75,
	81,
	77,
	74,
	79,
	83,
	82,
	72,
	80,
	87,
	79,
	76,
	85,
	95,
	87,
	81,
	88,
	93,
	88,
	84,
	87,
	94,
	86,
	82,
	85,
	94,
	85,
	82,
	85,
	95,
	86,
	83,
	92,
	99,
	91,
	88,
	94,
	98,
	95,
	90,
	97,
	105,
	104,
	94,
	98,
	114,
	117,
	124,
	144,
	180,
	210,
	236,
	253,
	227,
	171,
	99,
	49,
	34,
	29,
	43,
	69,
	89,
	89,
	90,
	98,
	107,
	104,
	98,
	104,
	110,
	102,
	98,
	103,
	111,
	101,
	94,
	103,
	108,
	102,
	95,
	97,
	106,
	100,
	92,
	101,
	103,
	100,
	94,
	98,
	103,
	96,
	90,
	98,
	103,
	97,
	90,
	99,
	104,
	95,
	90,
	99,
	104,
	100,
	93,
	100,
	106,
	101,
	93,
	101,
	105,
	103,
	96,
	105,
	112,
	105,
	99,
	103,
	108,
	99,
	96,
	102,
	106,
	99,
	90,
	92,
	100,
	87,
	80,
	82,
	88,
	77,
	69,
	75,
	79,
	74,
	67,
	71,
	78,
	72,
	67,
	73,
	81,
	77,
	71,
	75,
	84,
	79,
	77,
	77,
	76,
	76,
};

void FuncTimerMedir(void *param)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

void FuncTimerECG(void *param)
{
	vTaskNotifyGiveFromISR(ECG_task_handle, pdFALSE);
}

// void ControlarMedir()
// {
// 	medir = !medir;
// }

// void ControlarHold()
// {
// 	hold = !hold;
// }

// void FuncControlarEDU(void *param)
// {
// 	UartReadByte(UART_PC, &teclaPC);

// 	if (teclaPC == 'O')
// 	{
// 		ControlarMedir();
// 	}
// 	else if (teclaPC == 'H')
// 	{
// 		ControlarHold();
// 	}
// }

void mostrarECG(void *pvParameter)
{

	int i = 0;

	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogOutputWrite(ecg[i]);
		i++;
		if (i == BUFFER_SIZE)
		{
			i = 0;
		}
	}
}

void MedicionOsciloscopio(void *pvParameter)
{
	while (true)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &medicion_Osc);

		UartSendString(UART_PC, (char *)UartItoa(medicion_Osc, 10));
		UartSendString(UART_PC, "\n\r");
	}
}

// static void ShowDistanceTask(void *pvParameter)
// {
// 	uint16_t distancia;

// 	while (true)
// 	{
// 		// ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

// 		if (medir == true)
// 		{
// 			distancia = HcSr04ReadDistanceInCentimeters();
// 			if (distancia < 10)
// 			{
// 				LedOff(LED_1);
// 				LedOff(LED_2);
// 				LedOff(LED_3);
// 			}
// 			else if ((distancia >= 10) & (distancia < 20))
// 			{
// 				LedOn(LED_1);
// 				LedOff(LED_2);
// 				LedOff(LED_3);
// 			}
// 			else if ((distancia >= 20) & (distancia < 30))
// 			{
// 				LedOn(LED_1);
// 				LedOn(LED_2);
// 				LedOff(LED_3);
// 			}
// 			else if (distancia >= 30)
// 			{
// 				LedOn(LED_1);
// 				LedOn(LED_2);
// 				LedOn(LED_3);
// 			}

// 			if (hold == false)
// 			{
// 				LcdItsE0803Write(distancia);
// 				UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
// 				UartSendString(UART_PC, " cm\r\n");
// 			}
// 		}
// 		else
// 		{
// 			LedsOffAll();
// 			LcdItsE0803Off();
// 		}

// 		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
// 		// vTaskDelay(1000 / portTICK_PERIOD_MS);
// 	}
// }

/*==================[external functions definition]==========================*/
void app_main(void)
{

AnalogOutputInit();
	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = 2000,
		.func_p = FuncTimerMedir,
		.param_p = NULL};

	TimerInit(&timer_medir);

	timer_config_t timer_ECG = {
		.timer = TIMER_B,
		.period = 4000,
		.func_p = FuncTimerECG,
		.param_p = NULL};

	TimerInit(&timer_ECG);

	serial_config_t puertoserie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&puertoserie);

	// LedsInit();
	// SwitchesInit();
	// HcSr04Init(GPIO_3, GPIO_2);
	// LcdItsE0803Init();
	// SwitchActivInt(SWITCH_1, ControlarMedir, NULL);
	// SwitchActivInt(SWITCH_2, ControlarHold, NULL);

	// xTaskCreate(&ShowDistanceTask, "Mostrar_Distancia_con_Leds", 2048, NULL, 5, &medir_task_handle);

	analog_input_config_t config = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};

	AnalogInputInit(&config);
	

	xTaskCreate(&MedicionOsciloscopio, "Medir tension con el Osciloscopio", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&mostrarECG, "Muestra señal ECG", 2048, NULL, 5, &ECG_task_handle);

	TimerStart(timer_medir.timer);
	TimerStart(timer_ECG.timer);
}

/*==================[end of file]============================================*/