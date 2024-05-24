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
#include "rtc_mcu.h"

/*==================[macros and definitions]=================================*/

int16_t tension = 220;
int16_t corriente;
int16_t consumo;
int16_t consumo_maximo;

rtc_t tiempo_interno;

TaskHandle_t medir_corriente_task_handle = NULL;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

void FuncTimerMedirCorriente(void *param)
{
    vTaskNotifyGiveFromISR(medir_corriente_task_handle, pdFALSE);
}

void InicioApagadoProgramado(int8_t t_inicio, int8_t t_apagado)
{

    while (true)
    {
        RtcRead(&tiempo_interno);
        if (tiempo_interno.hour>=t_inicio)
        {
            ActivaRelee();
        }

         if (tiempo_interno.hour>=t_inicio)
        {
            ActivaRelee();
        }
        
    }
    

}

void ApagarSegunConsumo(int16_t v_consumo)
{
    while(true){

        if (consumo >= consumo_maximo)
        {
            DesactivaRelee();
        }
        
    }
}

void ActivaRelee()
{
    // GPIO_Activate
}

void DesactivaRelee()
{
    // GPIO_Deactivate
}

void MedidorCorriente()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogInputReadSingle(CH1, &corriente); //Mide la tension proporcional a la corriente

        consumo = consumo + corriente*tension; //Calcula la potencia y la suma al consumo


    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    AnalogOutputInit();

    // Parametros Timer
    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = 2000,
        .func_p = FuncTimerMedirCorriente,
        .param_p = NULL};

    TimerInit(&timer_medir);

    // Puerto serie
    serial_config_t puertoserie = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL};

    UartInit(&puertoserie);

    // Entrada analogica
    analog_input_config_t config = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };

    AnalogInputInit(&config);

    TimerStart(timer_medir.timer);

    rtc_t tiempo = {
        tiempo.year = 2024,
        tiempo.month = 5,
        tiempo.mday = 22,
        tiempo.wday = 3,
        tiempo.hour = 11,
        tiempo.min = 6,
        tiempo.sec = 0};

    RtcConfig(&tiempo);

}

/*==================[end of file]============================================*/