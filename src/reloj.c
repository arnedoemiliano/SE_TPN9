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

/* === Headers files inclusions =============================================================== */

#include "reloj.h"
#include <string.h>

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

typedef struct reloj_s {

    uint8_t hora_actual[6];
    bool hora_valida : 1;
    int ticks; // cantidad de interrupciones antes de aumentar un segundo
    int tick_actual;
    /***********************/
    uint8_t alarma[4];
    bool alarma_habilitada : 1;
    callback_disparar disparar_alarma;
    uint32_t snooze_offset; // tiempo que se suma a alarma al momento de compararse con hora_actual

} reloj_s;
/* === Private variable declarations =========================================================== */

// uint8_t hora_actual[] = {0};

/* === Private function declarations =========================================================== */
void NuevoSegundo(reloj_t reloj);

uint32_t DataTimeASeg(uint8_t * data_time);
/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */
void NuevoSegundo(reloj_t reloj) {

    reloj->hora_actual[5]++; // incrementa en 1 los segundos
                             // false: si se superaron las 10 horas

    if (reloj->hora_actual[5] == 10) {
        reloj->hora_actual[5] = 0;
        reloj->hora_actual[4]++; // incrementan las decenas de segundos
        if (reloj->hora_actual[4] == 6) {
            reloj->hora_actual[4] = 0;
            reloj->hora_actual[3]++; // incrementan los minutos
            if (reloj->hora_actual[3] == 10) {
                reloj->hora_actual[3] = 0;
                reloj->hora_actual[2]++; // incrementan las decenas de minutos
                if (reloj->hora_actual[2] == 6) {
                    reloj->hora_actual[2] = 0;
                    reloj->hora_actual[1]++; // incrementan las horas
                    if (reloj->hora_actual[1] == 10 && reloj->hora_actual[0] < 2) {
                        reloj->hora_actual[1] = 0;
                        reloj->hora_actual[0]++; // incrementan las decenas
                    } else if (reloj->hora_actual[1] == 4 && reloj->hora_actual[0] == 2) {
                        reloj->hora_actual[1] = 0;
                        reloj->hora_actual[0] = 0;
                    }
                }
            }
        }
    }
}
// Convierte cualquier array de 4 bytes a un entero sin signo
uint32_t DataTimeASeg(uint8_t * data_time) {

    // data_time debe tener 4 elementos como minimo, corregir para que no haya comportamiento
    // indefinido si se pasaran menos elementos
    uint32_t data_time_seg = (data_time[0] * 10 * 3600) + (data_time[1] * 3600) +
                             (data_time[2] * 10 * 60) + (data_time[3] * 60);
    return data_time_seg;
}
/* === Public function implementation ========================================================== */

reloj_t ClockCreate(int ticks_por_segundo, callback_disparar funcion_de_disparo) {

    static reloj_s self[1];
    memset(self, 0, sizeof(self));
    self->ticks = ticks_por_segundo;
    self->disparar_alarma = funcion_de_disparo;
    return self;
}

bool GetClockTime(reloj_t reloj, uint8_t * hora, int size) {

    // memset(hora, 0, size);
    memcpy(hora, reloj->hora_actual, size);

    return reloj->hora_valida;
}

bool SetClockTime(reloj_t reloj, const uint8_t * hora_nueva, int size) {

    memcpy(reloj->hora_actual, hora_nueva, size);
    reloj->hora_valida = true; // indica que la hora actual del reloj es válida

    return true; // hace falta retornar una confirmacion?
}

// En principio esta funcion es la que llama el systick en cada interrupcion
int RelojNuevoTick(reloj_t reloj) {

    if ((reloj->tick_actual >= (reloj->ticks - 1))) { // if ((reloj->tick_actual >= reloj->ticks)) {
        if (reloj->hora_valida == true) {
            NuevoSegundo(reloj);
            VerificarAlarma(reloj);
        }
        reloj->tick_actual = 0;
    } else {
        reloj->tick_actual++;
    }
    return reloj->tick_actual;
}

bool SetAlarmTime(reloj_t reloj, const uint8_t * alarma) {
    // No me debería dejar setear una alarma si nunca se configuró la hora

    memcpy(reloj->alarma, alarma, 4);
    reloj->alarma_habilitada = true;
    return true;
}

bool GetAlarmTime(reloj_t reloj, uint8_t * alarma) {

    memcpy(alarma, reloj->alarma, 4);
    return reloj->alarma_habilitada;
}

// La funcion pasa hora_actual y alarma a segundos, esto se hace para facilitar el offset de tiempo
// que se agrega cuando se pospone la alarma, y compara cada minuto si hay coincidencia.
// modificacion: Se hace que la funcion devuelva un booleando para saber si la alarma está sonando o
// no. en funcion del parametro 'mode' la funcion disparará la alarma o avisará que está sonando al
// encontrar coincidencia
// mode = 1 : al haber coincidencia dispara la alarma y retorna 1
// mode = 0 : al haber coincidencia no dispara y retorna 1 y si no hubo coincidencia retorna 0
void VerificarAlarma(reloj_t reloj) {
    // Necesito verificar alarma solo cada minuto:

    if ((reloj->hora_actual[4] == 0) && (reloj->hora_actual[5] == 0)) {

        // Convierto hora_actual y alarma a segundos, sumo el snooze a alarma y comparo.
        uint32_t hora_actual = DataTimeASeg(reloj->hora_actual);
        uint32_t alarma_actual = DataTimeASeg(reloj->alarma);
        uint32_t nueva_alarma = alarma_actual + reloj->snooze_offset;

        // Verifico que nueva_alarma no supere las 24 horas
        if (nueva_alarma >= 86400) {
            nueva_alarma -= 86400;
        }
        if ((hora_actual == nueva_alarma) & (reloj->alarma_habilitada)) {
            reloj->disparar_alarma(reloj, true);
        }
    }
}

void ToggleHabAlarma(reloj_t reloj) {

    reloj->alarma_habilitada ^= 1;
    reloj->snooze_offset = 0;
}

void PosponerAlarma(reloj_t reloj, uint8_t minutos) {

    reloj->snooze_offset += minutos * 60;
    reloj->disparar_alarma(reloj, false);
}

void CancelarAlarma(reloj_t reloj) {
    reloj->snooze_offset = 0;
    reloj->disparar_alarma(reloj, false);
}

/*
void DeshabilitarAlarma(reloj_t reloj) {

    reloj->alarma_habilitada = false;
    reloj->snooze_offset = 0;
}
*/

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */

/*
int RelojNuevoTick(reloj_t reloj) {
    bool act_tick;

    if ((reloj->tick_actual >= (reloj->ticks - 1))) { // if ((reloj->tick_actual >= reloj->ticks)) {
        if (reloj->hora_valida == true) {
            NuevoSegundo(reloj);
            VerificarAlarma(reloj);
        }
        reloj->tick_actual = 0;
    } else {
        reloj->tick_actual++;
    }
    return reloj->tick_actual;
}

*/