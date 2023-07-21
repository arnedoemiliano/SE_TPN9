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
#include "bsp.h"
#include "reloj.h"
#include "chip.h"
#include <stdbool.h>
#include "digital.h"

/* === Macros definitions ====================================================================== */
//#define RES_RELOJ         6    // Cuantos digitos tiene el reloj
#define RES_DISPLAY_RELOJ    4    // Cuantos digitos del reloj se mostrarán
#define INT_PER_SECOND       1000 // interrupciones por segundo del systick
#define DELAY_SET_TIME_ALARM 3 // segundos de delay para que se active el boton set_time o set_alarm
#define MAX_IDLE_TIME        5 // cantidad de segundos antes de cancelar por inactividad
/* === Private data type declarations ========================================================== */

typedef enum {
    SIN_CONFIGURAR,
    MOSTRANDO_HORA,
    AJUSTANDO_MINUTOS_ACTUAL,
    AJUSTANDO_HORAS_ACTUAL,
    AJUSTANDO_MINUTOS_ALARMA,
    AJUSTANDO_HORAS_ALARMA,
} modo_t;

/* === Private variable declarations =========================================================== */
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

/* === Public variable definitions ============================================================= */
modo_t modo;
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
    modo = valor;

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

/* === Public function implementation ========================================================= */

int main(void) {

    reloj = ClockCreate(TICKS_PER_SECOND, ActivarAlarma);
    board = BoardCreate();
    modo = SIN_CONFIGURAR;
    SisTick_Init(INT_PER_SECOND);
    DisplayToggleDot(board->display, 1);
    DisplayFlashDigits(board->display, 0, 3, 250); // cuando inicia el reloj los digitos parpadean

    while (1) {
    }
}

void SysTick_Handler(void) {

    uint8_t hora[RES_DISPLAY_RELOJ];

    int tick = RelojNuevoTick(reloj);
    if (modo <= MOSTRANDO_HORA) {
        if (tick == 0) {
            if (flag_set_time_alarm) {
                if (cnt_set_time_alarm) {
                    cnt_set_time_alarm--;
                }
            } else {
                // Si la bandera esta en false se reinicia la cuenta así no se acumulan los tiempos
                // de falsas pulsaciones
                cnt_set_time_alarm = DELAY_SET_TIME_ALARM;
            }

            (void)GetClockTime(reloj, hora, RES_DISPLAY_RELOJ);
            DisplayWriteBCD(board->display, hora, sizeof(hora));
            DisplayToggleDot(board->display, 1);
            VerificarAlarma(reloj);
        }
        if (tick == TICKS_PER_SECOND / 2) {
            DisplayToggleDot(board->display, 1);
        }
    }
    if (modo >= AJUSTANDO_MINUTOS_ACTUAL && modo <= AJUSTANDO_HORAS_ALARMA) {
        if (tick == 0) {
            if (flag_idle) {
                if (cnt_idle) {
                    cnt_idle--;
                }
            }
        }
    }
    DisplayRefresh(board->display);
}

/* === End of documentation ====================================================================*/

/** @} End of module definition for doxygen */
