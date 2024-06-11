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
uint16_t voltajeSensor = 0;
uint16_t corriente = 0;
int16_t consumo;
int16_t consumo_maximo;

bool switch01 = false;
bool switch02 = false;

float sensibilidad = 66.0;
float corriente_conversor = 0;
float voltajeSensor_conversor = 0;
float voltajeCte = 2480.0;
float factor_conversor = 100;
float Imax = 0;
float Imin = 0;

uint8_t contador_corriente = 0;

uint8_t teclas;

bool ON_OFF;

rtc_t tiempo_interno;

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
    while (true)
    {
        RtcRead(&tiempo_interno);
        if (tiempo_interno.hour >= t_inicio)
        {
            ActivaRelee();
        }

        if (tiempo_interno.hour >= t_apagado)
        {
            DesactivaRelee();
        }
    }
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

        //

        // teclas = SwitchesRead();

        // switch (teclas)
        // {
        // case SWITCH_1:
        //     ActivaRelee();
        //     break;
        // default:
        //     DesactivaRelee();
        //     break;
        // }

        // if (ON_OFF = true)
        // {
        //     DesactivaRelee();
        // }
        // else if (ON_OFF = false)
        // {
        //     ActivaRelee();
        // }
    }
}

static void MedidorCorriente(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (switch02 == true)
        {

            corriente = 0;
            voltajeSensor = 0;
            voltajeSensor_conversor = 0;
            corriente_conversor = 0;

            AnalogInputReadSingle(CH1, &voltajeSensor); // Mide la tension proporcional a la corriente

            voltajeSensor_conversor = ((float)voltajeSensor);
            corriente_conversor = corriente_conversor * 0.9 + 0.1 * ((voltajeSensor_conversor - voltajeCte) / sensibilidad);

            if (corriente > Imax)
            {
                Imax = corriente;
            }

            if (corriente < Imin)
            {
                Imin = corriente;
            }

            //  corriente_conversor = corriente_conversor; + ((voltajeSensor_conversor - voltajeCte) / sensibilidad) * factor_conversor;

            contador_corriente++;

            if (contador_corriente == 100)
            {
                // corriente_conversor = corriente_conversor / 100;

                // corriente_conversor = voltajeSensor_conversor / 100;

                // uint16_t corriente = (unsigned int)corriente_conversor;
                UartSendString(UART_PC, (char *)UartItoa(corriente, 10)); // muestra la corriente por terminal
                UartSendString(UART_PC, "\n\r");
                // corriente_conversor = 0;
                contador_corriente = 0;
                // voltajeSensor_conversor = 0;

                (((Imax - Imin) / 2) );
            }

            // float get_corriente()
            // {
            //   float voltajeSensor;
            //   float corriente=0;
            //   long tiempo=millis();
            //   float Imax=0;
            //   float Imin=0;
            //   while(millis()-tiempo<500)//realizamos mediciones durante 0.5 segundos
            //   {
            //     voltajeSensor = analogRead(A0) * (5.0 / 1023.0);//lectura del sensor
            //     corriente=0.9*corriente+0.1*((voltajeSensor-2.527)/Sensibilidad); //Ecuación  para obtener la corriente
            //     if(corriente>Imax)Imax=corriente;
            //     if(corriente<Imin)Imin=corriente;
            //   }
            //   return(((Imax-Imin)/2)-offset);
            // }

            // consumo = consumo + corriente*tension; // Calcula la potencia y la suma al consumo
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    // rtc_t tiempo = {
    //     tiempo.year = 2024,
    //     tiempo.month = 5,
    //     tiempo.mday = 22,
    //     tiempo.wday = 3,
    //     tiempo.hour = 11,
    //     tiempo.min = 6,
    //     tiempo.sec = 0};

    // RtcConfig(&tiempo);

    AnalogOutputInit();

    // Parametros Timer
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

    xTaskCreate(&MedidorCorriente, "Mide la corriente consumida", 2048, NULL, 5, &medir_corriente_task_handle);
    xTaskCreate(&PrendeApagaRele, "Enciende o apaga el rele", 2048, NULL, 5, &ON_OFF_Switch_task_handle);

    TimerStart(timer_medir.timer);
    TimerStart(timer_rele.timer);
}



/*==================[end of file]============================================*/