/*! @mainpage Proyecto Integrador
 *
 * \section genDesc General Description
 *
 * La funcion de la aplicacion es la de permitir controlar mediante una aplicacion del celular por Bluetooth el encendido y apagado segun el tiempo y el consumo, y ademas poder visualizar la corriente que consume
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Relee	 	| 	5v 	    	|
 * | 	Relee	 	| 	GND 		|
 * | 	Relee	 	| 	GPIO_9 		|
 * | 	ASC712  	| 	5v 		    |
 * | 	ASC712   	| 	GND 		|
 * | 	ASC712  	| 	GPIO_1 		|
 *
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 21/06/2024 | Document creation		                         |
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
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "ble_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500

int16_t tension = 220;           /**< Variable donde se guarda la tension de linea */
uint16_t voltajeSensor = 0;      /**< Variable donde se guarda la medicion del ASC712 */
uint16_t corriente = 0;          /**< Variable donde se guarda la corriente luego de ser calculada */
uint16_t corrienteBT = 0;       /**< Variable donde se guarda la corriente luego de ser calculada para ser compartida a la aplicacion*/
uint32_t consumo = 0;           /**< Variable donde se guarda el consumo del aparato luego de ser calculado */
uint32_t consumo_maximo = 0;     /**< Variable donde se guarda el consumo maximo determinado por la aplicacion */
uint16_t temporizador = 0;      /**< Variable donde se guarda el tiempo que el dispositivo lleva encendido en segundos */
uint16_t tiempoApagado = 0;     /**< Variable donde se guarda el tiempo maximo que el dispositivo debe permanecer encendido determinado por la apliacion */

float corriente_conversor = 0.0;    /**< Variable que se utiliza para convertir los valores de corriente de entero a flotante*/
float voltajeSensor_conversor = 0.0; /**< Variable que se utiliza para convertir los valores medidos por el sensor de entero a flotante*/

float Imax = 0;  /**< Variable que se utiliza para guardar el valor maximo del sensor de corriente*/
float Imin = 0;  /**< Variable que se utiliza para guardar el valor minimo del sensor de corriente*/

uint8_t contador_corriente = 0; /**< Variable que se utiliza como contador del numero de veces que se realiza la medicion*/

bool ON_OFF = false; /**< Variable que almacena si el relee esta activado o desactivado*/

char msgI[30];  /**< Variable que almacena los datos que se envian a la aplicacion*/

TaskHandle_t medir_corriente_task_handle = NULL; /**< Task Handler del medidor de corriente*/
TaskHandle_t sistema_task_handle = NULL; /**< Task Handler del sistema*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*!
 *
 * @fn void FuncTimerMedirCorriente(void *param)
 * @brief  Funcion del timer A que controla medirCorriente()
 * @return
 */
void FuncTimerMedirCorriente(void *param)
{
    vTaskNotifyGiveFromISR(medir_corriente_task_handle, pdFALSE);
}

/*!
 * @fn void FuncTimerSistema(void *param)
 * @brief  Funcion del timer B que controla Sistema()
 * @return
 */
void FuncTimerSistema(void *param)
{
    vTaskNotifyGiveFromISR(sistema_task_handle, pdFALSE);
}


/*!
 * @fn void ActivaRelee()
 * @brief  Funcion que activa el relee
 * @return
 */
void ActivaRelee()
{
    GPIOOn(GPIO_9); // desactiva GPIO 9
}

/*!
 * @fn void DesactivaRelee()
 * @brief  Funcion que desactiva el relee
 * @return
 */
void DesactivaRelee()
{
    GPIOOff(GPIO_9); // Activa GPIO 9
}

/*!
 * @fn void ApagadoProgramado()
 * @brief  Funcion que desactiva el relee en funcion del tiempo que transcurre y el tiempo programado para el apagado
 * @return
 */
void ApagadoProgramado() 
{
    if (tiempoApagado != 0)
    {
        // UartSendString(UART_PC, "Tiempo: ");
        // UartSendString(UART_PC, (char *)UartItoa(temporizador, 10));
        // UartSendString(UART_PC, " seg");
        // UartSendString(UART_PC, "\n\r");
        temporizador++;

        if (temporizador >= tiempoApagado)
        {
            DesactivaRelee();
            temporizador = 0;
            ON_OFF = false;
        }
    }
}

/*!
 * @fn void ApagarSegunConsumo() 
 * @brief  Funcion que desactiva el relee en funcion de la potencia consumida y el consumo maximo programado
 * @return
 */
void ApagarSegunConsumo() 
{
    if (consumo_maximo != 0)
    {
        // UartSendString(UART_PC, "corriente: ");
        // UartSendString(UART_PC, (char *)UartItoa(corrienteBT, 10));
        // UartSendString(UART_PC, " mA");
        // UartSendString(UART_PC, "\n\r");

        consumo = consumo + (int)((220 * corrienteBT) / 1000);
        // UartSendString(UART_PC, "consumo: ");
        // UartSendString(UART_PC, (char *)UartItoa(consumo, 10));
        // UartSendString(UART_PC, " W.s");
        // UartSendString(UART_PC, "\n\r");

        // UartSendString(UART_PC, "consumo maximo: ");
        // UartSendString(UART_PC, (char *)UartItoa(consumo_maximo, 10));
        // UartSendString(UART_PC, " W.s");
        // UartSendString(UART_PC, "\n\r");
        if (consumo >= consumo_maximo)
        {
            DesactivaRelee();
            ON_OFF = false;
            consumo = 0;
        }
    }
}

/*!
 * @fn static void MedidorCorriente(void *param)
 * @brief  Funcion que mide la corriente que consume el dispositivo, convirtiendo el valor del sensor en corriente 
 * @return
 */
static void MedidorCorriente(void *param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogInputReadSingle(CH1, &voltajeSensor); // Mide la tension proporcional a la corriente

        if (Imax == 0)
        {
            Imax = voltajeSensor;
        }

        if (Imin == 0)
        {
            Imin = voltajeSensor;
        }

        if (voltajeSensor > Imax)
        {
            Imax = voltajeSensor;
        }

        if (voltajeSensor < Imin)
        {
            Imin = voltajeSensor;
        }

        contador_corriente++;

        if (contador_corriente == 250)
        {
            corriente_conversor = ((Imax - Imin) - 17.0) / 0.104;
            corriente = (uint16_t)(corriente_conversor);

            // UartSendString(UART_PC, "Corriente: ");
            // UartSendString(UART_PC, (char *)UartItoa(corriente, 10));
            // UartSendString(UART_PC, "\n\r");

                corrienteBT = corriente;
        
            contador_corriente = 0;
            corriente = 0;
            voltajeSensor = 0;
            voltajeSensor_conversor = 0.0;
            corriente_conversor = 0.0;
            Imax = 0;
            Imin = 0;
        }
    }
}

/**
 * @brief Función a ejecutarse ante un interrupción de recepción 
 * a través de la conexión BLE.
 * 
 * @param data      Puntero a array de datos recibidos
 * @param length    Longitud del array de datos recibidos
 */
void read_data(uint8_t *data, uint8_t length)
{
    uint8_t i = 1;
    static uint8_t time = 0;
    static uint32_t pot = 0;

    if (data[0] == 'O')
    {
        ActivaRelee();
        ON_OFF = true;
    }

    if (data[0] == 'o')
    {
        DesactivaRelee();
        ON_OFF = false;
        consumo = 0;
        temporizador = 0;
    }

    if (data[0] == 'T')
    {
        /* El slidebar de tiempo envía los datos con el formato "T" + value + "t" */
        time = 0;
        while (data[i] != 't')
        {
            /* Convertir el valor ASCII a un valor entero */
            time = time * 10;
            time = time + (data[i] - '0');
            i++;
        }
        tiempoApagado = time;
    }

    else if (data[0] == 'P')
    {
        /* El slidebar de potencia envia en formato "P" + value + "p" */
        pot = 0;
        while (data[i] != 'p')
        {
            /* Convertir el valor ASCII a un valor entero */
            pot = pot * 10;
            pot = pot + (data[i] - '0');
            i++;
        }
        consumo_maximo = pot;
    }
}

/*!
 * @fn void sistema()
 * @brief  Funcion que maneja el tiempo y consumo del dispositivo cuando el relee esta activado 
 * @return
 */
void sistema()
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ON_OFF == true)
        {
            ApagadoProgramado();
            ApagarSegunConsumo();
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    ble_config_t ble_configuration = {
        "Domotica",
        read_data};
    BleInit(&ble_configuration);

    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = 2000,
        .func_p = FuncTimerMedirCorriente,
        .param_p = NULL};

    TimerInit(&timer_medir);

    timer_config_t timer_sistema = {
        .timer = TIMER_B,
        .period = 1000000,
        .func_p = FuncTimerSistema,
        .param_p = NULL};

    TimerInit(&timer_sistema);

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

    GPIOInit(GPIO_9, GPIO_OUTPUT);

    xTaskCreate(&MedidorCorriente, "Mide la corriente consumida", 512, NULL, 5, &medir_corriente_task_handle);
    xTaskCreate(&sistema, "sistema", 512, NULL, 5, &sistema_task_handle);

    TimerStart(timer_medir.timer);
    TimerStart(timer_sistema.timer);

    while (1)
    {
        switch (BleStatus())
        {
        case BLE_OFF:
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            break;
        case BLE_DISCONNECTED:
            LedOff(LED_1);
            LedOn(LED_2);
            LedOff(LED_3);
            break;
        case BLE_CONNECTED:
            LedOff(LED_1);
            LedOff(LED_2);
            LedOn(LED_3);
            break;
        }
        sprintf(msgI, "*G%d*", corrienteBT);
        BleSendString(msgI);

        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}

/*==================[end of file]============================================*/