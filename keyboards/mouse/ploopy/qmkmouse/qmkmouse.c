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
#include QMK_KEYBOARD_H

// Debug Options
#define DEBUGMOUSE false  // Slows down scan rate!

// Trackball State
bool     BurstState   = false;  // init burst state for Trackball module
uint16_t MotionStart  = 0;      // Timer for accel, 0 is resting state
uint16_t lastScroll   = 0;      // Previous confirmed wheel event
uint16_t lastMidClick = 0;      // Stops scrollwheel from being read if it was pressed

// Mouse Processing
static void process_mouse(bool bMotion, bool* bBurst) {
    // Read state
    PMWState d        = point_burst_read(bMotion, bBurst);
    bool     isMoving = (d.X != 0) || (d.Y != 0);
    int16_t  x, y;
    
    // Reset timer if stopped moving
    if (!isMoving) {
        if (MotionStart != 0) MotionStart = 0;
        return;
    }

    // Set timer if new motion
    if ((MotionStart == 0) && isMoving) {
        if (DEBUGMOUSE) dprintf("Starting motion.\n");
        MotionStart = timer_read();
    }

    if (DEBUGMOUSE) {
        dprintf("Delt] d: %d t: %u\n", abs(d.X) + abs(d.Y), MotionStart);
    }
    if (DEBUGMOUSE) {
        dprintf("Pre ] X: %d, Y: %d\n", d.X, d.Y);
    }

    // Wrap to HID size
    x = constrain(d.X, -127, 127);
    y = constrain(d.Y, -127, 127);
    
    if (DEBUGMOUSE) dprintf("Cons] X: %d, Y: %d\n", x, y);

    report_mouse_t currentReport = pointing_device_get_report();
    currentReport.x              = (int)x;
    currentReport.y              = (int)y;
    pointing_device_set_report(currentReport);
}

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (IS_MOUSEKEY_BUTTON(keycode)) {
        report_mouse_t currentReport = pointing_device_get_report();
        if (record->event.pressed) {
            if (keycode == KC_MS_BTN1)
                currentReport.buttons |= MOUSE_BTN1;
            else if (keycode == KC_MS_BTN2)
                currentReport.buttons |= MOUSE_BTN2;
            else if (keycode == KC_MS_BTN3) {
                currentReport.buttons |= MOUSE_BTN3;
                // Update Timer to prevent accidental scrolls
                lastMidClick = timer_read();
            }
            else if (keycode == KC_MS_BTN4)
                currentReport.buttons |= MOUSE_BTN4;
            else if (keycode == KC_MS_BTN5)
                currentReport.buttons |= MOUSE_BTN5;
        } else {
            if (keycode == KC_MS_BTN1)
                currentReport.buttons &= ~MOUSE_BTN1;
            else if (keycode == KC_MS_BTN2)
                currentReport.buttons &= ~MOUSE_BTN2;
            else if (keycode == KC_MS_BTN3)
                currentReport.buttons &= ~MOUSE_BTN3;
            else if (keycode == KC_MS_BTN4)
                currentReport.buttons &= ~MOUSE_BTN4;
            else if (keycode == KC_MS_BTN5)
                currentReport.buttons &= ~MOUSE_BTN5;
        }
        
        pointing_device_set_report(currentReport);
    }

    return process_record_user(keycode, record);
}

void process_wheel(void) {
    // If the mouse wheel was just released, do not scroll.
    if ((timer_read() - lastMidClick) < SCROLL_BUTT_DEBOUNCE) return;

    // Limit the number of scrolls per unit time.
    if ((timer_read() - lastScroll) < OPT_DEBOUNCE) return;

    // Don't scroll if the middle button is depressed.
    if (!readPin(MOUSE_MIDDLE_PIN)) return;

    lastScroll  = timer_read();
    
    uint16_t p1 = adc_read(OPT_ENC1_MUX);
    uint16_t p2 = adc_read(OPT_ENC2_MUX);

    uint8_t dir = scroll(p1, p2);

    // Bundle and send if needed
    if (dir == 0) return;
    report_mouse_t cRep = pointing_device_get_report();
    cRep.v += dir * OPT_SCALE;
    pointing_device_set_report(cRep);
}

// Hardware Setup
void keyboard_pre_init_kb(void) {
    
    /* Set me to true to make debug messages show up. */
    //debug_enable = true;
    //debug_matrix = true;
    //debug_mouse  = true;

    // Set up all the hardware
    setPinOutput(SENSOR_CS);

    setPinInputHigh(MOUSE_LEFT_PIN);
    setPinInputHigh(MOUSE_RIGHT_PIN);
    setPinInputHigh(MOUSE_MIDDLE_PIN);
    setPinInputHigh(MOUSE_BACK_PIN);
    setPinInputHigh(MOUSE_FORWARD_PIN);
    
    setPinInputHigh(MOUSE_RR_PIN);
    setPinInputHigh(MOUSE_LL_PIN);
    setPinInputHigh(MOUSE_DPI_PIN);    

    setPinInput(OPT_ENC1);
    setPinInput(OPT_ENC2);

    /* Ground all output pins connected to ground. This provides additional
     * pathways to ground. If you're messing with this, know this: driving ANY
     * of these pins high will cause a short. On the MCU. Ka-blooey.
     */
    setPinOutput(B4);
    writePinLow(B4);
    setPinOutput(D6);
    writePinLow(D6);
    setPinOutput(F1);
    writePinLow(F1);
    setPinOutput(F5);
    writePinLow(F5);
    setPinOutput(F6);
    writePinLow(F6);
    setPinOutput(F7);
    writePinLow(F7);

    // Initialize SPI for MCU
    SPI_Init(SPI_OPTION);
    DDRB |= (1 << SS_TB);
    PORTB |= (1 << SS_TB);  // pull up to diable all comm
    point_init(SS_TB);

    /* Set up the scroll wheel. */
    Scroller();
}

void matrix_scan_kb(void) {
    process_mouse(Motion, &BurstState);
    process_wheel();
}
