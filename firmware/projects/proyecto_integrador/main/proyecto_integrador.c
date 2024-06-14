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
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

int16_t tension = 220;      // Tension de linea
uint16_t voltajeSensor = 0; // Aca se guarda la medicion
uint16_t corriente = 0;
int16_t consumo;
int16_t consumo_maximo;

bool switch01 = false;
bool switch02 = false;

float sensibilidad = 0.066; // Constante propia del medidor

float corriente_conversor = 0.0;
float voltajeSensor_conversor = 0.0;
float voltajeCte = 2.5;
float filtroPasaBajos = 0;
int16_t aux_int = 0;
float factorConv = (5.0 / 4095.0);

// float factor_conversor = 100;

float Imax = 0;
float Imin = 0;

uint8_t contador_corriente = 0;
uint16_t Cont2 = 0;

uint8_t teclas;

bool ON_OFF;

TaskHandle_t medir_corriente_task_handle = NULL;
TaskHandle_t ON_OFF_Switch_task_handle = NULL;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

void FuncTimerMedirCorriente(void *param)
{
    vTaskNotifyGiveFromISR(medir_corriente_task_handle, pdFALSE);
}

void FuncTimerPrenderApagarRele(void *param)
{
    vTaskNotifyGiveFromISR(ON_OFF_Switch_task_handle, pdFALSE);
}

void ActivaRelee()
{
    GPIOOn(GPIO_9); // desactiva GPIO 9
}

void DesactivaRelee()
{
    GPIOOff(GPIO_9); // Activa GPIO 9
}

void tecla01()
{
    switch01 = !switch01;
    UartSendString(UART_PC, "tecla 1 \n\r");
}

void tecla02()
{
    switch02 = !switch02;
    UartSendString(UART_PC, "tecla 2 \n\r");
}

void InicioApagadoProgramado(int8_t t_inicio, int8_t t_apagado) // activa desactiva relee segun hora
{
    // while (true)
    // {
    //     RtcRead(&tiempo_interno);
    //     if (tiempo_interno.hour >= t_inicio)
    //     {
    //         ActivaRelee();
    //     }

    //     if (tiempo_interno.hour >= t_apagado)
    //     {
    //         DesactivaRelee();
    //     }
    // }
}

void ApagarSegunConsumo(int16_t v_consumo) // activa desactiva relee segun consumo
{
    while (true)
    {
        if (consumo >= consumo_maximo)
        {
            DesactivaRelee();
        }
    }
}

static void PrendeApagaRele(void *param) // activa desactiva relee con el boton SWITCH 1
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (switch01 == true)
        {
            ActivaRelee();
        }
        else if (switch01 == false)
        {
            DesactivaRelee();
        }
    }
}

static void MedidorCorriente(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (switch02 == true)
        {
            AnalogInputReadSingle(CH1, &voltajeSensor); // Mide la tension proporcional a la corriente

            voltajeSensor_conversor = (float)((voltajeSensor) * (factorConv));
            corriente_conversor = ((voltajeSensor_conversor - 2.5) / sensibilidad);

            filtroPasaBajos = (filtroPasaBajos * 0.9) + (0.1 * ((voltajeSensor_conversor - voltajeCte) / sensibilidad));

            if (Imax == 0)
            {
                Imax = filtroPasaBajos;
            }

            if (Imin == 0)
            {
                Imin = filtroPasaBajos;
            }

            if (filtroPasaBajos > Imax)
            {
                Imax = filtroPasaBajos;
            }

            if (filtroPasaBajos < Imin)
            {
                Imin = filtroPasaBajos;
            }

            contador_corriente++;
            // Cont2++;

            if (contador_corriente == 250)
            {
                corriente_conversor = ((Imax - Imin) / 2) * 100;
                corriente = (uint16_t)(corriente_conversor * 10);

                UartSendString(UART_PC, "voltajeSensor: ");
                UartSendString(UART_PC, (char *)UartItoa(voltajeSensor, 10));
                UartSendString(UART_PC, "\n\r");

                voltajeSensor_conversor = voltajeSensor_conversor * 100;
                UartSendString(UART_PC, "voltajeSensor_conversor: ");
                UartSendString(UART_PC, (char *)UartItoa(voltajeSensor_conversor, 10));
                UartSendString(UART_PC, "\n\r");

                // UartSendString(UART_PC, "corriente_conversor: ");
                // UartSendString(UART_PC, (char *)UartItoa(corriente_conversor, 10));
                // UartSendString(UART_PC, "\n\r");

                // UartSendString(UART_PC, "Imax: ");
                // aux_int = (int)(Imax);
                // UartSendString(UART_PC, (char *)UartItoa(aux_int, 10));
                // UartSendString(UART_PC, "\n\r");

                // // //snprintf(fstr, sizeof(fstr), "%.2f", Imin);
                // // //UartSendString(UART_PC, fstr);
                // UartSendString(UART_PC, "Imin: ");
                // aux_int = (int)(Imin);
                // UartSendString(UART_PC, (char *)UartItoa(aux_int, 10));
                // UartSendString(UART_PC, "\n\r");

                // UartSendString(UART_PC, "Corriente: ");
                // UartSendString(UART_PC, (char *)UartItoa(corriente, 10));
                // UartSendString(UART_PC, "\n\r");

                // if (Cont2 == 10000)
                // {
                //     Imax = voltajeSensor;
                //     Imin = voltajeSensor;
                //     Cont2 = 0;
                // }

                contador_corriente = 0;
                corriente = 0;
                voltajeSensor = 0;
                voltajeSensor_conversor = 0.0;
                corriente_conversor = 0.0;
                filtroPasaBajos = 0.0;
                aux_int = 0;
                Imax = 0;
                Imin = 0;
            }
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = 2000,
        .func_p = FuncTimerMedirCorriente,
        .param_p = NULL};

    TimerInit(&timer_medir);

    timer_config_t timer_rele = {
        .timer = TIMER_B,
        .period = 20000,
        .func_p = FuncTimerPrenderApagarRele,
        .param_p = NULL};

    TimerInit(&timer_rele);

    AnalogOutputInit();

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
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&config);

    LedsInit();
    SwitchesInit();

    SwitchActivInt(SWITCH_1, tecla01, NULL);
    SwitchActivInt(SWITCH_2, tecla02, NULL);

    GPIOInit(GPIO_9, GPIO_OUTPUT);

    // cambio el 2048 por 512
    xTaskCreate(&MedidorCorriente, "Mide la corriente consumida", 512, NULL, 5, &medir_corriente_task_handle);
    xTaskCreate(&PrendeApagaRele, "Enciende o apaga el rele", 512, NULL, 5, &ON_OFF_Switch_task_handle);

    TimerStart(timer_medir.timer);
    TimerStart(timer_rele.timer);
}

/*==================[end of file]============================================*/