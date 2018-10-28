/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include "mgos.h"
#include "mgos_spi.h"

EpdIf::EpdIf() {
};

EpdIf::~EpdIf() {
};

void EpdIf::DigitalWrite(int pin, int value) {
    mgos_gpio_write(pin, value);
}

int EpdIf::DigitalRead(int pin) {
    return mgos_gpio_read(pin);
}

void EpdIf::DelayMs(unsigned int delaytime) {
    mgos_msleep(delaytime);
}

void EpdIf::SpiTransfer(unsigned char data) {
    struct mgos_spi *spi;
    /* Global SPI instance is configured by the `spi` config section. */
    spi = mgos_spi_get_global();
    if (spi == NULL) {
        LOG(LL_ERROR, ("SPI is not configured, make sure spi.enable is true"));
        return;
    }
    struct mgos_spi_txn txn;
    txn.cs = 0;
    txn.mode = 0;
    txn.freq = 0;
    txd.hd.tx_len = 1;
    txd.hd.tx_data = &data;
    txn.hd.dummy_len = 0;
    txn.hd.rx_len = 0;
    txn.hd.rx_data = NULL;
    if (!mgos_spi_run_txn(spi, false /* full_duplex */, &txn)) {
        LOG(LL_ERROR, ("SPI transaction failed"));
        return;
    }
}

int EpdIf::IfInit(void) {
    mgos_gpio_set_mode(mgos_sys_config_get_epd_rst_gpio(), MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(mgos_sys_config_get_epd_dc_gpio(), MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(mgos_sys_config_get_epd_busy_gpio(), MGOS_GPIO_MODE_INPUT); 
    return 0;
}

