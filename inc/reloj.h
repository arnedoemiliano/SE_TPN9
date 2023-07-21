/************************************************************************************************
Copyright (c) 2023, Emiliano Arnedo <emiarnedo@gmail.com>
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
SPDX-License-Identifier: MIT
*************************************************************************************************/

#ifndef RELOJ_H
#define RELOJ_H

/* === Headers files inclusions ================================================================ */
#include <stdbool.h>
#include "stdint.h"
/* === Cabecera C++ ============================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =============================================================== */

#define TICKS_PER_SECOND 1000 // Cuantos ticks debe contar el reloj para sumar un segundo

/* === Public data type declarations =========================================================== */

typedef struct reloj_s * reloj_t;
typedef void (*callback_disparar)(reloj_t reloj,
                                  bool act_desact); // funcion de callback que facilita el testing

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

reloj_t ClockCreate(int ticks_por_segundo, callback_disparar funcion_de_disparo);

bool GetClockTime(reloj_t reloj, uint8_t * hora, int size);

bool SetClockTime(reloj_t reloj, const uint8_t * hora, int size);

int RelojNuevoTick(reloj_t reloj);

bool SetAlarmTime(reloj_t reloj, const uint8_t * alarma);

bool GetAlarmTime(reloj_t reloj, uint8_t * alarma);

void VerificarAlarma(reloj_t reloj);

void ToggleHabAlarma(reloj_t reloj);

void PosponerAlarma(reloj_t reloj, uint8_t tiempo);

void CancelarAlarma(reloj_t reloj);

/* === End of documentation ==================================================================== */

#ifdef __cplusplus
}
#endif

/** @} End of module definition for doxygen */

#endif /* RELOJ_H */
