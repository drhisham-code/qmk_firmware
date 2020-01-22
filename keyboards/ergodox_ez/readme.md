# ErgoDox EZ

![Ergodox EZ](https://www.ergodox.io/img/ErgoDox-original-min.png)

The ErgoDox keyboard is a DIY keyboard project originally developed by “Dox” (Dominic Beauchamp) inspired by the Key64 Keyboard.
The printed circuit board was designed by “bpiphany” (Fredrik Atmer).

The ErgoDox EZ was commercialized and manufactured by Erez Zukerman, Dmitry Slepov, and Yaara Lancet (ZSA Technology Labs)

The ErgoDox EZ supports hot swap switches, reinforced by the case (since rev/batch 5), and supports RGB Light (underglow) in the "Shine" model, and RGB Per Key (RGB Matrix) in the "Glow" model. 

This supports both the original ErgoDox style PCBs, as well as the EZ variant. 

* Keyboard Maintainer: [Drashna](https://github.com/drashna)
* Hardware Supported: The PCBs, controllers supported
* Hardware Availability: 
  * [ErgoDox EZ](https://ergodox-ez.com/) 
  * [Profet Keyboards](http://shop.profetkeyboards.com/product/ergodox-pcbs)
  * [FalbaTech](https://falba.tech/customize-your-keyboard/customize-your-ergodox/)
  * [MechanicalKeyboards](https://mechanicalkeyboards.com/shop/index.php?l=product_detail&p=537)


Make example for this keyboard (after setting up your build environment):

    make ergodox_ez:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).



## ErgoDox EZ Configuration (from Oryx)

### Indicator LEDs

The two front "teeth" LED indicators are PWM controlled.   If you have `ORYX_CONFIGURATOR` defined in your keymap's `config.h`, you can use the `LED_LEVEL` keycode to cycle through preset vales (0, 25%, 50%, 75%, 100%), and will be saved to EEPROM (persistent storage)

Alternatively, you can set the brightness by calling the following functions: 

```c
void ergodox_right_led_1_set(uint8_t n);
void ergodox_right_led_2_set(uint8_t n);
void ergodox_right_led_3_set(uint8_t n);

void ergodox_led_all_set(uint8_t n);
```

These settings are not persistent, so you'd need to reset it every time the board starts. 

These are on a 0-255 scale 

### RGB Matrix Features

If you're using the Smart LED (layer indication) feature from the Oryx Configurator, you want to make sure that you enable these options by adding `#define ORYX_CONFIGURATOR` to your keymap's `config.h`. 

This changes the `RGB_TOG` keycode so that it will toggle the lights on and off, in a way that will allow the Smart LEDs to continue to work, even with the rest of the LEDs turned off. 

Additionally, a new keycode has been added to toggle the Smart LEDs.  Use `TOGGLE_LAYER_COLOR`, if you aren't already.  
