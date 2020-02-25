/*
Copyright 2018 Jack Humbert <jack.humb@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include "wait.h"
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"
#include "debounce.h"
#include "i2c_master.h"
#include QMK_KEYBOARD_H

#define ROWS_PER_HAND (MATRIX_ROWS / 2)

/*
#define MATRIX_ROW_PINS { B0  B1  B2  B3  D2  D3  C6 } outputs
#define MATRIX_COL_PINS { B5  B4  B3  B2  B1  B0 }   inputs
 */
/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];
static matrix_row_t matrix_debouncing_right[ROWS_PER_HAND];
static bool         debouncing            = false;
static uint16_t     debouncing_time       = 0;
static bool         debouncing_right      = false;
static uint16_t     debouncing_time_right = 0;


extern bool mcp23018_leds[3];
extern bool is_launching;

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

bool           mcp23018_initd = false;
static uint8_t mcp23018_reset_loop;

uint8_t mcp23018_tx[3];
uint8_t mcp23018_rx[1];

void mcp23018_init(void) {
    i2c_init();


    mcp23018_tx[0] = 0x01;        // IODIRB
    mcp23018_tx[1] = 0b00000000;  // B is output
    mcp23018_tx[2] = 0b00111111;  // A is inputs

    if (I2C_STATUS_SUCCESS != i2c_transmit(MCP23018_DEFAULT_ADDRESS << 1, mcp23018_tx, 3, ERGODOX_EZ_I2C_TIMEOUT)) {
        printf("error hori\n");
    } else {
        mcp23018_tx[0] = 0x0D;        // GPPUB
        mcp23018_tx[1] = 0b00000000;  // B is not pulled-up
        mcp23018_tx[2] = 0b00111111;  // A is pulled-up

        if (I2C_STATUS_SUCCESS != i2c_transmit(MCP23018_DEFAULT_ADDRESS << 1, mcp23018_tx, 3, ERGODOX_EZ_I2C_TIMEOUT)) {
            printf("error hori\n");
        } else {
            mcp23018_initd = is_launching = true;
        }
    }
}

void matrix_init(void) {
    printf("matrix init\n");
    // debug_matrix = true;

    // outputs
    setPinInput(B0); writePinLow(B0);
    setPinInput(B1); writePinLow(B1);
    setPinInput(B2); writePinLow(B2);
    setPinInput(B3); writePinLow(B3);
    setPinInput(D2); writePinLow(D2);
    setPinInput(D3); writePinLow(D3);
    setPinInput(C6); writePinLow(C6);

    // inputs
    setPinInputHigh(F0);
    setPinInputHigh(F1);
    setPinInputHigh(F4);
    setPinInputHigh(F5);
    setPinInputHigh(F6);
    setPinInputHigh(F7);

    memset(matrix, 0, MATRIX_ROWS * sizeof(matrix_row_t));
    memset(matrix_debouncing, 0, MATRIX_ROWS * sizeof(matrix_row_t));
    memset(matrix_debouncing_right, 0, ROWS_PER_HAND * sizeof(matrix_row_t));

    mcp23018_init();

    matrix_init_quantum();
}

uint8_t matrix_scan(void) {
    bool changed = false;

    matrix_row_t data = 0;
    // actual matrix
    for (uint8_t row = 0; row < ROWS_PER_HAND; row++) {
        // strobe row
        switch (row) {
            case 0: writePinHigh(B0); break;
            case 1: writePinHigh(B1); break;
            case 2: writePinHigh(B2); break;
            case 3: writePinHigh(B3); break;
            case 4: writePinHigh(D2); break;
            case 5: writePinHigh(D3); break;
            case 6: writePinHigh(C6); break;
        }

        // need wait to settle pin state
        wait_us(20);

        // read col data
        data = (
            (readPin(F0) << 0 ) |
            (readPin(F1) << 1 ) |
            (readPin(F4) << 2 ) |
            (readPin(F5) << 3 ) |
            (readPin(F6) << 4 ) |
            (readPin(F7) << 5 )
        );

        // unstrobe  row
        switch (row) {
            case 0: writePinLow(B0); break;
            case 1: writePinLow(B1); break;
            case 2: writePinLow(B2); break;
            case 3: writePinLow(B3); break;
            case 4: writePinLow(D2); break;
            case 5: writePinLow(D3); break;
            case 6: writePinLow(C6); break;
        }

        if (matrix_debouncing[row] != data) {
            matrix_debouncing[row] = data;
            debouncing             = true;
            debouncing_time        = timer_read();
            changed                = true;
        }
    }

    for (uint8_t row = 0; row <= ROWS_PER_HAND; row++) {
        // right side

        if (!mcp23018_initd) {
            if (++mcp23018_reset_loop == 0) {
                // if (++mcp23018_reset_loop >= 1300) {
                // since mcp23018_reset_loop is 8 bit - we'll try to reset once in 255 matrix scans
                // this will be approx bit more frequent than once per second
                print("trying to reset mcp23018\n");
                mcp23018_init();
                if (!mcp23018_initd) {
                    print("left side not responding\n");
                } else {
                    print("left side attached\n");
#ifdef RGB_MATRIX_ENABLE
                    rgb_matrix_init();
#endif
                }
            }
        }

        // #define MCP23_ROW_PINS { GPB5, GBP4, GBP3, GBP2, GBP1, GBP0 }       outputs
        // #define MCP23_COL_PINS { GPA0, GBA1, GBA2, GBA3, GBA4, GBA5, GBA6 } inputs

        // select row

        mcp23018_tx[0] = 0x13;                                                                   // GPIOB
        mcp23018_tx[1] = (0b00111111 & ~(1 << (row))) | ((uint8_t)!mcp23018_leds[2] << 7);       // activate row
        mcp23018_tx[2] = ((uint8_t)!mcp23018_leds[1] << 6) | ((uint8_t)!mcp23018_leds[0] << 7);  // activate row

        if (I2C_STATUS_SUCCESS != i2c_transmit(MCP23018_DEFAULT_ADDRESS << 1, mcp23018_tx, 3, ERGODOX_EZ_I2C_TIMEOUT)) {
            printf("error hori\n");
            mcp23018_initd = false;
        }

        // read col

        mcp23018_tx[0] = 0x12;  // GPIOA
        if (I2C_STATUS_SUCCESS != i2c_readReg(MCP23018_DEFAULT_ADDRESS << 1, mcp23018_tx[0], &mcp23018_rx[0], 1, ERGODOX_EZ_I2C_TIMEOUT)) {
            printf("error vert\n");
            mcp23018_initd = false;
        }

        data = ~(mcp23018_rx[0] & 0b00111111);
        // data = 0x01;

        if (matrix_debouncing_right[row] != data) {
            matrix_debouncing_right[row] = data;
            debouncing_right             = true;
            debouncing_time_right        = timer_read();
            changed                      = true;
        }
    }

    if (debouncing && timer_elapsed(debouncing_time) > DEBOUNCE) {
        for (int row = 0; row < ROWS_PER_HAND; row++) {
            matrix[row] = matrix_debouncing[row];
        }
        debouncing = false;
    }

    if (debouncing_right && timer_elapsed(debouncing_time_right) > DEBOUNCE && mcp23018_initd) {
        for (int row = 0; row < ROWS_PER_HAND; row++) {
            matrix[(MATRIX_ROWS - 1) - row] = 0;
            for (int col = 0; col < MATRIX_COLS; col++) {
                matrix[(MATRIX_ROWS - 1) - row] |= ((matrix_debouncing_right[(MATRIX_COLS - 1) - col] & (1 << row) ? 1 : 0) << col);
            }
        }
        debouncing_right = false;
    }

    matrix_scan_quantum();

    return (uint8_t)changed;
}

bool matrix_is_on(uint8_t row, uint8_t col) { return (matrix[row] & (1 << col)); }

matrix_row_t matrix_get_row(uint8_t row) { return matrix[row]; }

void matrix_print(void) {
    printf("\nr/c 01234567\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        printf("%X0: ", row);
        matrix_row_t data = matrix_get_row(row);
        for (int col = 0; col < MATRIX_COLS; col++) {
            if (data & (1 << col))
                printf("1");
            else
                printf("0");
        }
        printf("\n");
    }
}
