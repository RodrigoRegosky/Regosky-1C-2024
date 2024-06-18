/*! @mainpage recuperatorio
 *
 * \section genDesc General Description
 *
 * La resolucion del recuperatorio es un programa que permite medir la temperatura mientras este dentro de un rango de distancia especifica
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Alarma	 	| 	GPIO_9		|
 * | 	HC-SR04	 	| 	GPIO_2 		|
 * | 	HC-SR04	 	| 	GPIO_3 		|
 * | 	termopila	| 	GPIO_1 		|
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 18/06/2024 | Document creation		                         |
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
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/

int16_t distancia = 0;          /**< Variable que guarda la distancia medida */
float temperatura = 0;          /**< Variable que guarda la temperatura medida */
int16_t tempInt = 0;            /**< Variable para convertir el valor de la temperatura de flotante a entero */
int16_t voltaje = 0;            /**< Variable para guardar el valor de tension obtenido en la termopila */
int8_t contadorTemperatura = 0; /**< Variable que cuenta las 10 mediciones de la temperatura */
float sumatoriaTemperatura = 0; /**< Variable que suma las 10 mediciones de la temperatura para luego sacar el promedio */

bool distanciaMedicion = false;  /**< Booleana para comprobar si la distnacia esa dentro del rango de medicion */
bool nuevoCicloDeMedidas = true; /**< Booleana para comprobar si es un nuevo ciclo de mediciones */

TaskHandle_t medir_distancia_task_handle = NULL;   /**< task handler de la funcion medir distancia */
TaskHandle_t medir_temperatura_task_handle = NULL; /**< task handler de la funcion medir temperatura */

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*!
 *
 * @fn void FuncTimerMedirDistancia(void *param)
 * @brief  Funcion del timer A que controla MedirDistancia()
 * @return
 */
void FuncTimerMedirDistancia(void *param)
{
    vTaskNotifyGiveFromISR(medir_distancia_task_handle, pdFALSE);
}

/*!
 *
 * @fn void FuncTimerMedirTemperatura(void *param)
 * @brief  Funcion del timer B que controla MedirTemperatura()
 * @return
 */

void FuncTimerMedirTemperatura(void *param)
{
    vTaskNotifyGiveFromISR(medir_temperatura_task_handle, pdFALSE);
}

/*!
 *
 * @fn void medirDistancia()
 * @brief  Funcion que mide la distancia a la persona y enciende los LEDs de acuedo a la misma
 * @return
 */

void medirDistancia()
{
    while (true)
    {

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        distancia = HcSr04ReadDistanceInCentimeters();
        if (distancia < 8)
        {
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);

            distanciaMedicion = false;
        }
        else if (distancia > 12)
        {
            LedOff(LED_1);
            LedOff(LED_2);
            LedOn(LED_3);

            distanciaMedicion = false;

            if (distancia > 140)
            {
                nuevoCicloDeMedidas = true; // Si detecta que se alejo mas de 140 cm se considera un nuevo ciclo
                GPIOOff(GPIO_9);            // se apaga la alarma en caso de que estuviese prendida
            }
        }
        else if ((distancia <= 12) && (distancia >= 8))
        {
            LedOff(LED_1);
            LedOn(LED_2);
            LedOff(LED_3);

            distanciaMedicion = true;
        }
    }
}

/*!
 *
 * @fn void medirTemperatura()
 * @brief  Funcion que mide la tension de la termopila y la convierte a temperatura. Toma 10 mediciones, las promedia y envia por  por puerto serie a la PC junto a la distancia. Tambien enciende una alarma segun la temperatura.
 * @return
 */

void medirTemperatura()
{
    while (true)
    {

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (distanciaMedicion == true) // Comprueba si la distancia esta dentro del rango
        {

            if (nuevoCicloDeMedidas == true) // comprueba que sea la primer medicion que se realiza del ciclo
            {
                AnalogInputReadSingle(CH1, &voltaje);            // Lee el voltaje de la termopila
                sumatoriaTemperatura = ((voltaje + 2200) / 110); // Convierte voltaje a temperatura
                contadorTemperatura++;

                if (contadorTemperatura == 10) // Cada 10 mediciones entra en el ciclo
                {
                    temperatura = (sumatoriaTemperatura / 10); // calcula el promedio de las 10 mediciones

                    tempInt = (int)temperatura;

                    // Saca por el puerto serie de la forma "temperatura] Cº persona a [distancia] cm"
                    UartSendString(UART_PC, (char *)UartItoa(tempInt, 10));
                    UartSendString(UART_PC, " Cº persona a ");
                    UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
                    UartSendString(UART_PC, " cm ");
                    UartSendString(UART_PC, "\n\r");

                    // Comprueba si la temperatura amerita prender la alarma
                    if (temperatura <= 37.5)
                    {
                        GPIOOff(GPIO_9);
                    }
                    else if (temperatura > 37.5)
                    {
                        GPIOOn(GPIO_9);
                    }

                    temperatura = 0;
                    contadorTemperatura = 0;
                    sumatoriaTemperatura = 0;
                }

                nuevoCicloDeMedidas = false; // Una vez realizada la medicion y mostrar por el puerto serie, se setea a falso para no realizar mas mediciones hasta el siguiente ciclo
            }
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    // Timers
    timer_config_t timer_distancia = {
        .timer = TIMER_A,
        .period = 1000000,
        .func_p = FuncTimerMedirDistancia,
        .param_p = NULL};

    TimerInit(&timer_distancia);

    timer_config_t timer_temperatura = {
        .timer = TIMER_B,
        .period = 100000,
        .func_p = FuncTimerMedirTemperatura,
        .param_p = NULL};

    TimerInit(&timer_temperatura);

    // Puerto serie
    serial_config_t puertoserie = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL};

    UartInit(&puertoserie);

    analog_input_config_t configTermopila = {
        .input = CH1,
        .mode = ADC_SINGLE,
    };
    AnalogInputInit(&configTermopila);

    GPIOInit(GPIO_9, GPIO_OUTPUT);

    HcSr04Init(GPIO_3, GPIO_2);

    LedsInit();

    xTaskCreate(&medirDistancia, "Mide la distancia a la persona", 2048, NULL, 5, &medir_distancia_task_handle);
    xTaskCreate(&medirTemperatura, "Mide la temperatura", 2048, NULL, 5, &medir_temperatura_task_handle);

    TimerStart(timer_distancia.timer);
    TimerStart(timer_temperatura.timer);
}
/*==================[end of file]============================================*/