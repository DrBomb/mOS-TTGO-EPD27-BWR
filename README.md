# mOS-TTGO-EPD27-BWR

This is a collection of libraries and code that I use to drive the [TTGO ESP32 ePaper 2.7inch display](https://www.aliexpress.com/item/32867880658.html).

It has the display. An SD card, two microphones (!) and a speaker amp.

Out of it, I only have for mOS the libraries required to drive the EPD [HERE](https://github.com/DrBomb/mOS-TTGO-EPD27-BWR/tree/master/epd2in7b) alongside a ported EPDPAINT library [HERE](https://github.com/DrBomb/epdpaint)

All the libraries are based off the sample code provided by the sellers, which in turn,  use the libraries made by [Waveshare](https://www.waveshare.com/) which you should totally take a look at.

# Building

If you wanna build the apps provided here it is easy. Clone this repository and navigate to either the `epdclock` or `epdwaveshare_example` folders and invoke `mos build`.
It should be able to build over the cloud as well
