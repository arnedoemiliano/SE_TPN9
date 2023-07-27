/* Copyright 2022, Laboratorio de Microprocesadores
 * Facultad de Ciencias Exactas y Tecnología
 * Universidad Nacional de Tucuman
 * http://www.microprocesadores.unt.edu.ar/
 * Copyright 2022, Esteban Volentini <evolentini@herrera.unt.edu.ar>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \brief Simple sample of use LPC HAL gpio functions
 **
 ** \addtogroup samples Sample projects
 ** \brief Sample projects to use as a starting point
 ** @{ */

/* === Headers files inclusions =============================================================== */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "bsp.h"
#include "reloj.h"
#include "chip.h"
#include <stdbool.h>
#include "digital.h"
#include "timers.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

/* === Macros definitions ====================================================================== */
//#define RES_RELOJ         6    // Cuantos digitos tiene el reloj
#define RES_DISPLAY_RELOJ    4    // Cuantos digitos del reloj se mostrarán
#define INT_PER_SECOND       1000 // interrupciones por segundo del systick
#define DELAY_SET_TIME_ALARM 3 // segundos de delay para que se active el boton set_time o set_alarm
#define MAX_IDLE_TIME        5 // cantidad de segundos antes de cancelar por inactividad
#define EVENT_F1_ON          (1 << 0)
#define EVENT_F2_ON          (1 << 1)
#define EVENT_F3_ON          (1 << 2)
#define EVENT_F4_ON          (1 << 3)
#define EVENT_ACCEPT_ON      (1 << 4)
#define EVENT_CANCEL_ON      (1 << 5)
#define EVENT_F1_OFF         (1 << 6)
#define EVENT_F2_OFF         (1 << 7)
#define EVENT_F3_OFF         (1 << 8)
#define EVENT_F4_OFF         (1 << 9)
#define EVENT_ACCEPT_OFF     (1 << 10)
#define EVENT_CANCEL_OFF     (1 << 11)
#define BIT_0                (1 << 0)
#define BIT_1                (1 << 1)

/* === Private data type declarations ========================================================== */
typedef void (*function_t)(void);
typedef enum {
    SIN_CONFIGURAR,
    MOSTRANDO_HORA,
    AJUSTANDO_MINUTOS_ACTUAL,
    AJUSTANDO_HORAS_ACTUAL,
    AJUSTANDO_MINUTOS_ALARMA,
    AJUSTANDO_HORAS_ALARMA,
} modo_t;

typedef struct key_s {
    uint16_t key_bit;
    // uint8_t mode;
    function_t funcion;
} * key_t;

/* === Private variable declarations ===========================================================*/
static board_t board;
static reloj_t reloj;
static uint8_t temp_input[4] = {0, 0, 0, 0}; // 4 porque nunca se configura la hora por minutos
static const uint8_t limite_min[] = {5, 9};
static const uint8_t limite_hs[] = {2, 3};
static bool alarma_sonando = false;
static bool flag_set_time_alarm = false; // para que el systick sepa cuando se presiono el boton
static uint8_t cnt_set_time_alarm = DELAY_SET_TIME_ALARM; // contador para el delay del boton
static bool flag_idle = false; // bandera para el "cancel" por inactividad
static uint8_t cnt_idle = MAX_IDLE_TIME;

/* === Private function declarations ===========================================================
 */

void ActivarAlarma(reloj_t reloj, bool act_desact);
void CambiarModo(modo_t modo);
void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]);
// limite indica donde se pasa despues de restar 1 a 00
void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]);

void AcceptKeyLogic(void);
void CancelKeyLogic(void);
void F1KeyLogic(void);
void F2KeyLogic(void);
void F3KeyLogic(void);
void F4KeyLogic(void);

/* === Public variable definitions ============================================================= */
modo_t modo;
EventGroupHandle_t key_group_handle;
EventGroupHandle_t clock_group_handle; // para controlar la verificacion de un nuevo segundo
SemaphoreHandle_t mode_mutex;
/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void ActivarAlarma(reloj_t reloj, bool act_desact) {
    if (act_desact) {
        DigitalOutputActivate(board->buzzer);
        alarma_sonando = true;
    } else {
        DigitalOutputDeactivate(board->buzzer);
        alarma_sonando = false;
    }
}

void CambiarModo(modo_t valor) {
    xSemaphoreTake(mode_mutex, portMAX_DELAY);
    modo = valor;
    xSemaphoreGive(mode_mutex);

    switch (modo) {
    case SIN_CONFIGURAR:
        DisplayFlashDigits(board->display, 0, 3, 250);
        break;
    case MOSTRANDO_HORA:
        DisplayFlashDigits(board->display, 0, 3, 0); // digitos sin parpadear
        // SetClockTime(reloj, (uint8_t[]){1, 2, 3, 4}, RES_DISPLAY_RELOJ);
        break;
    case AJUSTANDO_MINUTOS_ACTUAL:
        DisplayFlashDigits(board->display, 2, 3, 250);
        break;
    case AJUSTANDO_HORAS_ACTUAL:
        DisplayFlashDigits(board->display, 0, 1, 250);
        break;
    case AJUSTANDO_MINUTOS_ALARMA:
        DisplayFlashDigits(board->display, 2, 3, 250);
        break;
    case AJUSTANDO_HORAS_ALARMA:
        DisplayFlashDigits(board->display, 0, 1, 250);
        break;
    default:
        break;
    }
}

void IncrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {

    // uint8_t temp_limite_1 = limite[1];

    numero[1]++;

    // Corroboro antes si se llego al limite para que el ahora segundo if no me rompa la condicion
    // de limite
    if ((numero[0] == limite[0]) && (numero[1] > limite[1])) {
        numero[0] = 0;
        numero[1] = 0;
    }

    if (numero[1] > 9) {

        numero[1] = 0;
        numero[0]++;
    }
}

void DecrementarBCD(uint8_t numero[2], const uint8_t limite[2]) {
    numero[1]--;

    if ((numero[0] == 0) && ((int8_t)numero[1] < 0)) {
        numero[0] = limite[0];
        numero[1] = limite[1];
    }
    if ((int8_t)numero[1] < 0) {

        numero[1] = 9;
        numero[0]--;
    }
}

void AcceptKeyLogic(void) {

    // ACEPTAR

    if (modo == AJUSTANDO_MINUTOS_ACTUAL) {
        cnt_idle = MAX_IDLE_TIME;
        CambiarModo(AJUSTANDO_HORAS_ACTUAL);
    } else if (modo == AJUSTANDO_MINUTOS_ALARMA) {
        cnt_idle = MAX_IDLE_TIME;
        CambiarModo(AJUSTANDO_HORAS_ALARMA);
    } else if (modo == AJUSTANDO_HORAS_ACTUAL) {
        CambiarModo(MOSTRANDO_HORA);
        SetClockTime(reloj, temp_input, sizeof(temp_input));
    } else if (modo == AJUSTANDO_HORAS_ALARMA) {
        DisplayClearDot(board->display, DOT_0 | DOT_1 | DOT_2);
        SetAlarmTime(reloj, temp_input);
        CambiarModo(MOSTRANDO_HORA);
    } else if (modo == MOSTRANDO_HORA) {
        if (!GetAlarmTime(reloj, temp_input)) {
            ToggleHabAlarma(reloj);
            DisplaySetDot(board->display, DOT_3);
        } else if (alarma_sonando) {
            PosponerAlarma(reloj, 5);
        }
    }
}

void CancelKeyLogic(void) {
    if (modo == AJUSTANDO_MINUTOS_ACTUAL || modo == AJUSTANDO_MINUTOS_ALARMA) {
        if (GetClockTime(reloj, temp_input, sizeof(temp_input))) {
            CambiarModo(MOSTRANDO_HORA);
            DisplayClearDot(board->display, DOT_MASK);
        } else {
            CambiarModo(SIN_CONFIGURAR);
        }
    } else if (modo == AJUSTANDO_HORAS_ACTUAL) {
        cnt_idle = MAX_IDLE_TIME;
        CambiarModo(AJUSTANDO_MINUTOS_ACTUAL);
    } else if (modo == AJUSTANDO_HORAS_ALARMA) {
        cnt_idle = MAX_IDLE_TIME;
        CambiarModo(AJUSTANDO_MINUTOS_ALARMA);
    } else if (modo == MOSTRANDO_HORA) {
        if (GetAlarmTime(reloj, temp_input) && !alarma_sonando) {
            ToggleHabAlarma(reloj);
            DisplayClearDot(board->display, DOT_3);
        } else if (alarma_sonando) {
            CancelarAlarma(reloj);
        }
    }
}
// SET-TIME
void F4KeyLogic(void) {

    flag_set_time_alarm ^= 1;
    // if (cnt_set_time_alarm == 0) { // if (cnt_set_time_alarm == 0) cambio a 3 para test

    flag_idle = true;
    cnt_idle = MAX_IDLE_TIME;
    CambiarModo(AJUSTANDO_MINUTOS_ACTUAL);
    GetClockTime(reloj, temp_input, sizeof(temp_input));
    DisplayClearDot(board->display, DOT_1);
    DisplayWriteBCD(board->display, temp_input, sizeof(temp_input));
    //}
}
// SET-ALARM
void F3KeyLogic(void) {
    flag_set_time_alarm ^= 1;
    // if (cnt_set_time_alarm == 0 && modo != SIN_CONFIGURAR) {
    flag_idle = true;
    cnt_idle = MAX_IDLE_TIME;
    CambiarModo(AJUSTANDO_MINUTOS_ALARMA);
    GetAlarmTime(reloj, temp_input);
    DisplaySetDot(board->display, DOT_MASK);
    DisplayWriteBCD(board->display, temp_input, sizeof(temp_input));
    //}
}
// DECREMENT
void F2KeyLogic(void) {
    cnt_idle = MAX_IDLE_TIME;
    if (modo == AJUSTANDO_MINUTOS_ACTUAL || modo == AJUSTANDO_MINUTOS_ALARMA) {
        DecrementarBCD(&temp_input[2], limite_min);
    } else if (modo == AJUSTANDO_HORAS_ACTUAL || modo == AJUSTANDO_HORAS_ALARMA) {
        DecrementarBCD(temp_input, limite_hs);
    }
    DisplayWriteBCD(board->display, temp_input, sizeof(temp_input));
}
// INCREMENT
void F1KeyLogic(void) {
    cnt_idle = MAX_IDLE_TIME;
    if (modo == AJUSTANDO_MINUTOS_ACTUAL || modo == AJUSTANDO_MINUTOS_ALARMA) {
        // le paso el puntero a los dos digitos menos significativos
        IncrementarBCD(&temp_input[2], limite_min);
    } else if (modo == AJUSTANDO_HORAS_ACTUAL || modo == AJUSTANDO_HORAS_ALARMA) {
        IncrementarBCD(temp_input, limite_hs);
    }
    DisplayWriteBCD(board->display, temp_input, sizeof(temp_input));
}

static void KeyTask(void * object) {
    // board_t board = object;
    uint16_t events, current_state, last_state, changes;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50));

        if (DigitalInputGetState(board->accept)) {
            current_state |= EVENT_ACCEPT_ON;
        }
        if (DigitalInputGetState(board->cancel)) {
            current_state |= EVENT_CANCEL_ON;
        }
        if (DigitalInputGetState(board->set_time)) {
            current_state |= EVENT_F1_ON;
        }
        if (DigitalInputGetState(board->set_alarm)) {
            current_state |= EVENT_F2_ON;
        }
        if (DigitalInputGetState(board->decrement)) {
            current_state |= EVENT_F3_ON;
        }
        if (DigitalInputGetState(board->increment)) {
            current_state |= EVENT_F4_ON;
        }

        changes = current_state ^ last_state;
        last_state = current_state;
        events = ((changes & !current_state) << 6) | (changes & current_state);
        xEventGroupSetBits(key_group_handle, events);
        current_state = 0;
    }
}

static void ModeTask(void * object) {
    key_t options = object;
    while (1) {
        xEventGroupWaitBits(key_group_handle, options->key_bit, TRUE, FALSE, portMAX_DELAY);

        options->funcion();
    }
}

static void RefreshTask(void * object) {
    int tick;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1));
        tick = RelojNuevoTick(reloj);
        if (modo <= MOSTRANDO_HORA) {
            if (tick == 0) {
                xEventGroupSetBits(clock_group_handle, BIT_0);
            } else if (tick == TICKS_PER_SECOND / 2) {
                xEventGroupSetBits(clock_group_handle, BIT_1);
            }
        }

        DisplayRefresh(board->display);
    }
}

static void DisplayTask(void * object) {
    uint8_t hora[RES_DISPLAY_RELOJ];
    EventBits_t ux_bits;

    while (true) {
        ux_bits =
            xEventGroupWaitBits(clock_group_handle, BIT_0 | BIT_1, TRUE, FALSE, portMAX_DELAY);
        if ((ux_bits & BIT_0) != 0) {
            (void)GetClockTime(reloj, hora, RES_DISPLAY_RELOJ);
            DisplayWriteBCD(board->display, hora, sizeof(hora));
            DisplayToggleDot(board->display, 1);
        } else if ((ux_bits & BIT_1) != 0) {
            DisplayToggleDot(board->display, 1);
        }
    }
}

/* === Public function implementation ========================================================= */
/*Falta completar con las pulsaciones largas y la alarma*/

int main(void) {
    board = BoardCreate();
    reloj = ClockCreate(TICKS_PER_SECOND, ActivarAlarma);
    mode_mutex = xSemaphoreCreateMutex();
    key_group_handle = xEventGroupCreate();
    clock_group_handle = xEventGroupCreate();
    modo = SIN_CONFIGURAR;
    DisplayToggleDot(board->display, 1);
    DisplayFlashDigits(board->display, 0, 3, 250);

    static struct key_s key[6];
    key[0].key_bit = EVENT_F1_ON;
    key[0].funcion = F1KeyLogic;
    key[1].key_bit = EVENT_F2_ON;
    key[1].funcion = F2KeyLogic;
    key[2].key_bit = EVENT_F3_ON;
    key[2].funcion = F3KeyLogic;
    key[3].key_bit = EVENT_F4_ON;
    key[3].funcion = F4KeyLogic;
    key[4].key_bit = EVENT_ACCEPT_ON;
    key[4].funcion = AcceptKeyLogic;
    key[5].key_bit = EVENT_CANCEL_ON;
    key[5].funcion = CancelKeyLogic;

    if (key_group_handle == NULL) {
        while (1) {
        };
    }

    xTaskCreate(KeyTask, "KeysReading", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenF1", 256, &key[0], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenF2", 256, &key[1], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenF3", 256, &key[2], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenF4", 256, &key[3], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenAccept", 256, &key[4], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ModeTask, "ChangeModeWhenCancel", 256, &key[5], tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(RefreshTask, "RefreshDisplay", 512, NULL, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(DisplayTask, "WriteDisplay", 512, NULL, tskIDLE_PRIORITY + 3, NULL);

    vTaskStartScheduler();

    while (true) {
    }
    return 0;
}

/* === End of documentation ====================================================================*/

/** @} End of module definition for doxygen */
