/* Copyright 2019 Drashna Jael're (@drashna)
 * Copyright 2019 Sunjun Kim
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <math.h>
#include "quantum.h"
#include <LUFA/Drivers/Peripheral/SPI.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <string.h>
#include <print.h>
#include <stdio.h>
#include "spi.h"
#include "analog.h"
#include "pointing_device.h"
#include "scroller.h"

// Helpers
#define degToRad(angleInDegrees) ((angleInDegrees)*M_PI / 180.0)
#define radToDeg(angleInRadians) ((angleInRadians)*180.0 / M_PI)
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Hardware Pin Defs
#define SPI_OPTION (SPI_SPEED_FCPU_DIV_2 | SPI_SCK_LEAD_FALLING | SPI_SAMPLE_TRAILING | SPI_ORDER_MSB_FIRST | SPI_MODE_MASTER)

/* I/O pin assignments for the buttons, each of which is directly
   attached to a GPIO pin. */
#define MOUSE_LEFT_PIN D4
#define MOUSE_RIGHT_PIN E6
#define MOUSE_MIDDLE_PIN D2
#define MOUSE_BACK_PIN B5
#define MOUSE_FORWARD_PIN D7

#define MOUSE_RR_PIN C6
#define MOUSE_LL_PIN C7
#define MOUSE_DPI_PIN B7

#define SENSOR_CS B0
#define SS_TB 0

// Sensor defs
#define OPT_ENC1 F0
#define OPT_ENC2 F4
#define OPT_ENC1_MUX 0
#define OPT_ENC2_MUX 4

#ifndef SCROLL_BUTT_DEBOUNCE
#	 define SCROLL_BUTT_DEBOUNCE 100 // ms      Delay after middle click is released before you can scroll.
#endif

#ifndef OPT_DEBOUNCE
#    define OPT_DEBOUNCE 1  // (ms) 			Time between scroll events
#endif

#ifndef OPT_SCALE
#    define OPT_SCALE 1  //                     Multiplier for wheel
#endif

#define LAYOUT(BL, BM, BR, B4, B5, B6, B7, B8) \
    { {BL, BM, BR, B4, B5, B6, B7, B8}, }

void process_wheel(void);
