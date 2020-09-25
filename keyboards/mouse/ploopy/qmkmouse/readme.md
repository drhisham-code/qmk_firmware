# Ploopy Corporation QMK Mouse

It's a DIY, QMK Powered Mouse!

* "Keyboard" Maintainer: [Drashna Jael're](https://github.com/drashna/), [Germ](https://github.com/germ/), [Ploopy](https://github.com/ploopyco/)
* Hardware Supported: ATMega32u4 8MHz(3.3v)
* Hardware Availability: [Store](https://ploopy.co), [GitHub](https://github.com/ploopyco/qmkmouse)

Make example for this keyboard (after setting up your build environment):

    make mouse/ploopy/qmkmouse:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

You can put mouse into bootloader mode by holding down the "back" button while plugging the mouse in (it's the button on the side that's toward the back of the mouse). The mouse will present as an Atmel DFU USB device, after which you can flash it using the command:

	qmk flash -kb mouse/ploopy/qmkmouse -km <put-your-keymap-name-here>
	
The default keymap gives the buttons some example functions that may or may not work for you; see the keymap readme for details.
