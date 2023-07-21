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

/** \brief Brief description of the file
 **
 ** Full file description
 **
 ** \addtogroup name Module denomination
 ** \brief Brief description of the module
 ** @{ */

/* === Headers files inclusions =============================================================== */

#include "digital.h"
#include "chip.h"

/* === Macros definitions ====================================================================== */
// Si no esta definido OUTPUT_INSTANCES en algun otro archivo h, se lo define aqui.
#ifndef OUTPUT_INSTANCES
    #define OUTPUT_INSTANCES 4
#endif
#ifndef INPUT_INSTANCES
    #define INPUT_INSTANCES 6
#endif

/* === Private data type declarations ========================================================== */
struct digital_output_s {
    uint8_t port;
    uint8_t pin;
    bool allocated : 1;
};

struct digital_input_s {
    uint8_t port;
    uint8_t pin;
    bool allocated : 1;
    bool inverted : 1;
    bool last_change : 1;
};
/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */
digital_output_t DigitalOutputAllocate(void);
digital_input_t DigitalInputAllocate(void);
/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */
// Funcion interna del DigitalOutputCreate(), solo esta funcion puede acceder a ella.
digital_output_t DigitalOutputAllocate() {
    static struct digital_output_s instances[OUTPUT_INSTANCES] = {0};
    digital_output_t output = NULL;

    for (int i = 0; i < OUTPUT_INSTANCES; i++) {
        if (instances[i].allocated == false) {
            output = &instances[i];
            instances[i].allocated = true;
            break;
        }
    }
    return output;
}

digital_input_t DigitalInputAllocate() {
    static struct digital_input_s instances[INPUT_INSTANCES] = {0};
    digital_input_t input = NULL;
    for (int i = 0; i < INPUT_INSTANCES; i++) {
        if (instances[i].allocated == false) {
            input = &instances[i];
            instances[i].allocated = true;
            break;
        }
    }
    return input;
}

/* === Public function implementation ========================================================== */
/* --------------------------SALIDAS-------------------------- */
digital_output_t DigitalOutputCreate(uint8_t port, uint8_t pin) {

    digital_output_t output = DigitalOutputAllocate();
    output->port = port;
    output->pin = pin;
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, output->port, output->pin, false);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, output->port, output->pin, true);

    return output;
}

void DigitalOutputToggle(digital_output_t output) {

    Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, output->port, output->pin);
}

void DigitalOutputActivate(digital_output_t output) {

    Chip_GPIO_SetPinState(LPC_GPIO_PORT, output->port, output->pin, true);
}

void DigitalOutputDeactivate(digital_output_t output) {

    Chip_GPIO_SetPinState(LPC_GPIO_PORT, output->port, output->pin, false);
}

/* --------------------------ENTRADAS-------------------------- */

digital_input_t DigitalInputCreate(uint8_t port, uint8_t pin, bool inverted) {

    digital_input_t input = DigitalInputAllocate();
    if (input) { // si es una direccion valida será true, si es NULL false
        input->pin = pin;
        input->port = port;
        input->inverted = inverted;
        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, input->port, input->pin, false);
    }

    return input;
}

bool DigitalInputGetState(digital_input_t input) {

    return input->inverted ^ Chip_GPIO_GetPinState(LPC_GPIO_PORT, input->port, input->pin);
}

bool DigitalInputHasChanged(digital_input_t input) {

    bool current_state = DigitalInputGetState(input);
    bool result = current_state != input->last_change;
    input->last_change = current_state;
    return result;
}

bool DigitalInputHasActivated(digital_input_t input) {

    // verifica solo si la entrada fue activada
    bool current_state = DigitalInputGetState(input);
    bool result = current_state && !input->last_change;
    input->last_change = current_state;
    return result;
}

bool DigitalInputHasDeactivated(digital_input_t input) {

    bool current_state = DigitalInputGetState(input);
    bool result = !current_state && input->last_change;
    input->last_change = current_state;
    return result;
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */