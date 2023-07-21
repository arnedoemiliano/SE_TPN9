/************************************************************************************************
Copyright (c) <year>, <copyright holders>
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

#ifndef TEMPLATE_H
#define TEMPLATE_H

/** \brief Modulo: digital
 **
 ** Declaraciones publicas del modulo: digital
 **
 ** \addtogroup digital Modulo digital
 ** \brief -
 ** @{ */

/* === Headers files inclusions ================================================================ */
#include <stdint.h>
#include <stdbool.h>
/* === Cabecera C++ ============================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =============================================================== */

typedef struct digital_output_s * digital_output_t;
typedef struct digital_input_s * digital_input_t;

/* === Public data type declarations =========================================================== */

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */
digital_output_t DigitalOutputCreate(uint8_t port, uint8_t pin);

void DigitalOutputActivate(digital_output_t output);

void DigitalOutputDeactivate(digital_output_t output);

void DigitalOutputToggle(digital_output_t output);

digital_input_t DigitalInputCreate(uint8_t port, uint8_t pin, bool inverted);

bool DigitalInputGetState(digital_input_t input);

bool DigitalInputHasChanged(digital_input_t input);

bool DigitalInputHasActivated(digital_input_t input);

bool DigitalInputHasDeactivated(digital_input_t input);
/* === End of documentation ==================================================================== */

#ifdef __cplusplus
}
#endif

/** @} End of module definition for doxygen */

#endif /* TEMPLATE_H */
