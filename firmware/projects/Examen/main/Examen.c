/*! @mainpage Examen
 *
 * \section genDesc General Description
 *
 * La resolucion del parcial es una aplicacion que controla un dispositivo que permite controlar el riego y el pH de una plantera.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
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
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"


/*==================[macros and definitions]=================================*/
/**
 * @brief Variables donde se guardan las mediciones
 */
int16_t medicionPH;		 /**< Medicion de PH */
int16_t medicionHumedad; /**< Medicion de la humedad */
float medicionTension;	 /**< Medicion de la tension */
int8_t teclaSistema;	 /**< Se almacena la tecla que se oprimio*/

/**
 * @brief constante de converion de tension a PH
 */
float cteConversionVoltaPH = 3 / 14; /**< constante para convertir de tension a PH */

/**
 * @brief Variables booleanas donde se almacena si una bomba esta encendida o apagada
 */
bool estadoBombaRiego = false;
bool estadoBombaPhA = false;
bool estadobombaPhB = false;
bool estadoHumedad = true;
bool estadoSistema = true;

/**
 * @brief Task Handlers
 */
TaskHandle_t Sistema_de_mediciones_task_handle = NULL;
TaskHandle_t Informar_estado_sistema_task_handle = NULL;
TaskHandle_t Controlar_Sistema_task_handle = NULL;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*!
 *
 * @fn void FuncTimerSistemaDeMediciones(void *param)
 * @brief Funcion del timer A que controla sistema de mediciones
 * @return
 */

void FuncTimerSistemaDeMediciones(void *param)
{
	vTaskNotifyGiveFromISR(Sistema_de_mediciones_task_handle, pdFALSE);
}

/*!
 *
 * @fn void FuncTimerInformarEstadoSistema(void *param)
 * @brief Funcion del timer B que controla mostrar la informacion del sistema
 * @return
 */
void FuncTimerInformarEstadoSistema(void *param)
{
	vTaskNotifyGiveFromISR(Informar_estado_sistema_task_handle, pdFALSE);
}

/*!
 *
 * @fn void FuncTimerControlarSistema(void *param)
 * @brief Funcion del timer C que controla el apagado o encendido del sistema
 * @return
 */
void FuncTimerControlarSistema(void *param)
{
	vTaskNotifyGiveFromISR(Controlar_Sistema_task_handle, pdFALSE);
}


/*!
 *
 * @fn void SensorDeRiego()
 * @brief mide la humedad y la guarda en la variable medicionHumedad
 * @return
 */
void SensorDeRiego()
{
	AnalogInputReadSingle(CH1, &medicionHumedad);
}

/*!
 *
 * @fn void SensorTension()
 * @brief Mide el ph que se sensa en voltios y lo guarda en medicionTension
 * @return
 */
void SensorTension()
{
	AnalogInputReadSingle(CH2, &medicionTension);
}

/*!
 *
 * @fn void controlBombaRiego()
 * @brief controla el estado de la bomba de riego, permite activar y desactivar y avisa cuando esta activa
 * @return
 */
void controlBombaRiego()
{
	if (estadoBombaRiego = true)
	{
		GPIOOff(GPIO_9);
	}
	else if (estadoBombaRiego = false)
	{
		GPIOOn(GPIO_9);
		UartSendString(UART_PC, "Bomba de riego encendida");
		UartSendString(UART_PC, "\n\r");
	}
}

/*!
 *
 * @fn void controlPhA()
 * @brief controla el estado de la bomba de PH acido, permite activar/desactivar y avisa cuando esta activa
 * @return
 */
void controlPhA()
{
	if (estadoBombaPhA = true)
	{
		GPIOOff(GPIO_10);
	}
	else if (estadoBombaPhA = false)
	{
		GPIOOn(GPIO_10);
		UartSendString(UART_PC, "Bomba de PhA encendida");
		UartSendString(UART_PC, "\n\r");
	}
}

/*!
 *
 * @fn void controlPhB()
 * @brief controla el estado de la bomba de PH basico, permite activar/desactivar y avisa cuando esta activa
 * @return
 */
void controlPhB()
{
	if (estadobombaPhB = true)
	{
		GPIOOff(GPIO_11);
	}
	else if (estadobombaPhB = false)
	{
		GPIOOn(GPIO_11);
		UartSendString(UART_PC, "Bomba de PhB encendida");
		UartSendString(UART_PC, "\n\r");
	}
}

/*!
 *
 * @fn void MedidorHumedad()
 * @brief controla el estado de la humedad, si es baja activa bomba de riego, si es alta lo desactiva
 * @return
 */
void MedidorHumedad()
{
	SensorDeRiego();

	if (medicionHumedad < 1)
	{
		estadoHumedad = false;
		if (estadoBombaRiego = false)
		{
			controlBombaRiego();
		}
	}
	else if (medicionHumedad >= 1)
	{
		estadoHumedad = true;
		if (estadoBombaRiego = true)
		{
			controlBombaRiego();
		}
	}
}

/*!
 *
 * @fn void medidorPH()
 * @brief controla el PH, si es menor a 6 activa bomba B y desactiva A , si es mayor a 6.7 activa bomba A y desactiva B, si el ph es correcto desactiva bomba A y B
 * @return
 */
void medidorPH()
{
	SensorTension();

	medicionPH = (uint16_t)(medicionTension * cteConversionVoltaPH);

	if (medicionPH < 6) //
	{
		if (estadobombaPhB = false)
		{
			controlPhB();
		}
		if (estadoBombaPhA = true)
		{
			controlPhA();
		}
	}
	if (medicionPH > 6.7)
	{
		if (estadobombaPhB = true)
		{
			controlPhB();
		}
		if (estadoBombaPhA = false)
		{
			controlPhA();
		}
	}
	if (medicionPH >= 6 && medicionPH <= 6.7)
	{
		if (estadobombaPhB = true)
		{
			controlPhB();
		}
		if (estadoBombaPhA = true)
		{
			controlPhA();
		}
	}
}

/*!
 *
 * @fn void informarEstadoSistema()
 * @brief  informa por mensaje de la UART el estado de la humedad y el ph
 * @return
 */
void informarEstadoSistema()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (estadoHumedad = true)
		{
			UartSendString(UART_PC, "PH: ");
			UartSendString(UART_PC, (char *)UartItoa(medicionPH, 10));
			UartSendString(UART_PC, ", humedad correcta");
			UartSendString(UART_PC, "\n\r");
		}
		if (estadoHumedad = false)
		{
			UartSendString(UART_PC, "PH: ");
			UartSendString(UART_PC, (char *)UartItoa(medicionPH, 10));
			UartSendString(UART_PC, ", humedad incorrecta");
			UartSendString(UART_PC, "\n\r");
		}
	}
}

/*!
 *
 * @fn void SistemaDeMediciones()
 * @brief  controla las mediciones de la humedad y el ph
 * @return
 */
void SistemaDeMediciones()
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		medidorPH();
		MedidorHumedad();
	}
}

/*!
 *
 * @fn void activarSistema()
 * @brief  modifica la variable tecla a valor de encendido
 * @return
 */
void activarSistema()
{
	teclaSistema = 1;
}

/*!
 *
 * @fn void desactivarSistema()
 * @brief  modifica la variable tecla a valor de apagado
 * @return
 */
void desactivarSistema()
{
	teclaSistema = 2;
}

/*!
 *
 * @fn void Sistema()
 * @brief  Controla el sistema completo, permite prender o apagar el sistema. Al encender resetea los timers. Al apagar detiene las bombas y detiene los timers
 * @return
 */
void Sistema()
{
	while (true)
	{

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		UartReadByte(UART_PC, &teclaSistema);

		if (teclaSistema == 1)
		{
			if (estadoSistema = false)
			{
				TimerReset(TIMER_A);
				TimerReset(TIMER_B);
				estadoSistema = true;
			}
		}
		else if (teclaSistema == 2)
		{
			if (estadoSistema = true)
			{
				if (estadoBombaPhA = true)
				{
					controlPhA();
				}
				if (estadobombaPhB = true)
				{
					controlPhB();
				}
				if (estadoBombaRiego = true)
				{
					controlBombaRiego();
				}

				TimerStop(TIMER_A);
				TimerStop(TIMER_B);
				estadoSistema = false;
			}
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	AnalogOutputInit();

	// Parametros Timer
	timer_config_t timer_mediciones = {
		.timer = TIMER_A,
		.period = 3000000,
		.func_p = FuncTimerSistemaDeMediciones,
		.param_p = NULL};

	TimerInit(&timer_mediciones);

	timer_config_t timer_informar_estado = {
		.timer = TIMER_B,
		.period = 5000000,
		.func_p = FuncTimerInformarEstadoSistema,
		.param_p = NULL};

	TimerInit(&timer_informar_estado);

	timer_config_t timer_sistema = {
		.timer = TIMER_C,
		.period = 1000000,
		.func_p = FuncTimerControlarSistema,
		.param_p = NULL};

	TimerInit(&timer_sistema);

	serial_config_t puertoserie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&puertoserie);

	analog_input_config_t configHumedad = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&configHumedad);

	analog_input_config_t configPh = {
		.input = CH1,
		.mode = ADC_SINGLE,
	};
	AnalogInputInit(&configPh);

	SwitchesInit();
	
	SwitchActivInt(SWITCH_1, activarSistema, NULL);
	SwitchActivInt(SWITCH_2, desactivarSistema, NULL);

	GPIOInit(GPIO_9, GPIO_OUTPUT);
	GPIOInit(GPIO_10, GPIO_OUTPUT);
	GPIOInit(GPIO_11, GPIO_OUTPUT);

	xTaskCreate(&SistemaDeMediciones, "Mide PH y Humedad", 2048, NULL, 5, &Sistema_de_mediciones_task_handle);
	xTaskCreate(&informarEstadoSistema, "Informa el estado del sistema", 2048, NULL, 5, &Informar_estado_sistema_task_handle);
	xTaskCreate(&Sistema, "controla el estado del sistema", 2048, NULL, 5, &Controlar_Sistema_task_handle);

	TimerStart(timer_mediciones.timer);
	TimerStart(timer_informar_estado.timer);
	TimerStart(timer_sistema.timer);
}
/*==================[end of file]============================================*/