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

#ifndef PANTALLA_H
#define PANTALLA_H

/** \brief Brief description of the file
 **
 ** Full file description
 **
 ** \addtogroup name Module denomination
 ** \brief Brief description of the module
 ** @{ */

/* === Headers files inclusions ================================================================ */
#include <stdint.h>
/* === Cabecera C++ ============================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =============================================================== */

// Def de bits asociados a cada segmento
#define SEGMENT_A (1 << 0)
#define SEGMENT_B (1 << 1)
#define SEGMENT_C (1 << 2)
#define SEGMENT_D (1 << 3)
#define SEGMENT_E (1 << 4)
#define SEGMENT_F (1 << 5)
#define SEGMENT_G (1 << 6)
#define SEGMENT_P (1 << 7)
#define DOT_0     (1 << 0) // el de mas a la izquierda
#define DOT_1     (1 << 1)
#define DOT_2     (1 << 2)
#define DOT_3     (1 << 3)
#define DOT_MASK  (DOT_0 | DOT_1 | DOT_2 | DOT_3)

/* === Public data type declarations =========================================================== */

//! puntero a la estructura display_s
typedef struct display_s * display_t;

//! Funcion de callback para apagar los digitos y segmentos de la pantalla
typedef void (*display_screen_off_t)(void);

//! Funcion de callback para prender los segmentos de la pantalla
typedef void (*display_segments_on_t)(uint8_t segments);

//! Funcion de callback para prender un digito
typedef void (*display_digit_on_t)(uint8_t digits);

//! "Interfaz. Coleccion de metodos que deben estar presentes si o si en la "clase" display.
typedef struct display_driver_s {

    display_screen_off_t ScreenTurnOff;
    display_segments_on_t SegmentsTurnOn;
    display_digit_on_t DigitTurnOn;

} const * const display_driver_t; // puntero constante a la estructura: no puedo modificar ninguno
                                  // de los miembros de la estructura con ese puntero ni puedo
                                  // apuntar a otra direccion de memoria.

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

display_t DisplayCreate(uint8_t digits, display_driver_t driver);

void DisplayWriteBCD(display_t display, uint8_t * numbers, uint8_t size);

void DisplayRefresh(display_t display);

void DisplayToggleDot(display_t display, uint8_t digit_dot);

void DisplaySetDot(display_t display, uint8_t digit_dot);

void DisplayClearDot(display_t display, uint8_t digit_dot);

/**
 * @brief Funcion para parpadear digitos.
 *
 * @param display puntero a la estructura display_s
 * @param from desde que digito se hará parpadear
 * @param to hasta que digito se hará parpadear
 * @param factor ciclo util de encedido del digito en porcentaje
 */
void DisplayFlashDigits(display_t display, uint8_t from, uint8_t to, uint16_t factor);

/* === End of documentation ==================================================================== */

#ifdef __cplusplus
}
#endif

/** @} End of module definition for doxygen */

#endif /* PANTALLA_H */
