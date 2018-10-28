/**
 *  @filename   :   epd2in7b.cpp
 *  @brief      :   Implements for Dual-color e-paper library
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

#include <stdlib.h>
#include "mgos.h"
#include "mgos_spi.h"
#include "epd2in7b.h"

int Epd_Init(struct epd_display *epd){
    mgos_gpio_set_mode(epd->reset_pin, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(epd->dc_pin, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(epd->cs_pin, MGOS_GPIO_MODE_OUTPUT);
    mgos_gpio_set_mode(epd->busy_pin, MGOS_GPIO_MODE_INPUT);
    
    Epd_Reset(epd);
    
    Epd_SendCommand(epd, POWER_ON);
    Epd_WaitUntilIdle(epd);
    
    Epd_SendCommand(epd, PANEL_SETTING);
    Epd_SendData(epd, 0xaf);        //KW-BF   KWR-AF    BWROTP 0f
    
    Epd_SendCommand(epd, PLL_CONTROL);
    Epd_SendData(epd, 0x3a);       //3A 100HZ   29 150Hz 39 200HZ    31 171HZ
    
    Epd_SendCommand(epd, POWER_SETTING);
    Epd_SendData(epd, 0x03);                  // VDS_EN, VDG_EN
    Epd_SendData(epd, 0x00);                  // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    Epd_SendData(epd, 0x2b);                  // VDH
    Epd_SendData(epd, 0x2b);                  // VDL
    Epd_SendData(epd, 0x09);                  // VDHR
    
    Epd_SendCommand(epd, BOOSTER_SOFT_START);
    Epd_SendData(epd, 0x07);
    Epd_SendData(epd, 0x07);
    Epd_SendData(epd, 0x17);
    
    // Power optimization
    Epd_SendCommand(epd, 0xF8);
    Epd_SendData(epd, 0x60);
    Epd_SendData(epd, 0xA5);
    
    // Power optimization
    Epd_SendCommand(epd, 0xF8);
    Epd_SendData(epd, 0x89);
    Epd_SendData(epd, 0xA5);
    
    // Power optimization
    Epd_SendCommand(epd, 0xF8);
    Epd_SendData(epd, 0x90);
    Epd_SendData(epd, 0x00);
    
    // Power optimization
    Epd_SendCommand(epd, 0xF8);
    Epd_SendData(epd, 0x93);
    Epd_SendData(epd, 0x2A);
    
    // Power optimization
    Epd_SendCommand(epd, 0xF8);
    Epd_SendData(epd, 0x73);
    Epd_SendData(epd, 0x41);
    
    Epd_SendCommand(epd, VCM_DC_SETTING_REGISTER);
    Epd_SendData(epd, 0x12);                   
    Epd_SendCommand(epd, VCOM_AND_DATA_INTERVAL_SETTING);
    Epd_SendData(epd, 0x87);        // define by OTP
    
    Epd_SetLut(epd);
    
    Epd_SendCommand(epd, PARTIAL_DISPLAY_REFRESH);
    Epd_SendData(epd, 0x00);  
    /* EPD hardware init end */
    
    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void Epd_SendCommand(struct epd_display *epd, unsigned char command) {
    mgos_gpio_write(epd->dc_pin, false);
    Epd_SpiTransfer(epd, command);
}

void Epd_SpiTransfer(struct epd_display *epd, unsigned char data) {
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
    txn.hd.tx_len = 1;
    txn.hd.tx_data = &data;
    txn.hd.dummy_len = 0;
    txn.hd.rx_len = 0;
    txn.hd.rx_data = NULL;
    mgos_gpio_write(epd->cs_pin, false);
    if (!mgos_spi_run_txn(spi, false /* full_duplex */, &txn)) {
        LOG(LL_ERROR, ("SPI transaction failed"));
        return;
    }
    mgos_gpio_write(epd->cs_pin, true);
}

/**
 *  @brief: basic function for sending data
 */
void Epd_SendData(struct epd_display *epd, unsigned char data) {
    mgos_gpio_write(epd->dc_pin, true);
    Epd_SpiTransfer(epd, data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void Epd_WaitUntilIdle(struct epd_display *epd) {
    while(mgos_gpio_read(epd->busy_pin) == 0) {      //0: busy, 1: idle
        mgos_msleep(100);
    }
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
bool Epd_IsBusy(struct epd_display *epd) {
    if(mgos_gpio_read(epd->busy_pin) == 0) return true;
    else return false;
}

/**
 *  @brief: module reset. 
 *          often used to awaken the module in deep sleep, 
 *          see Epd::Sleep();
 */
void Epd_Reset(struct epd_display *epd) {
    mgos_gpio_write(epd->reset_pin, false);
    mgos_msleep(200);
    mgos_gpio_write(epd->reset_pin, true);
    mgos_msleep(200);   
}

/**
 *  @brief: set the look-up tables
 */
void Epd_SetLut(struct epd_display *epd) {
    unsigned int count;     
    Epd_SendCommand(epd, LUT_FOR_VCOM);                            //vcom
    for(count = 0; count < 44; count++) {
        Epd_SendData(epd, lut_vcom_dc[count]);
    }
    
    Epd_SendCommand(epd, LUT_WHITE_TO_WHITE);                      //ww --
    for(count = 0; count < 42; count++) {
        Epd_SendData(epd, lut_ww[count]);
    }   
    
    Epd_SendCommand(epd, LUT_BLACK_TO_WHITE);                      //bw r
    for(count = 0; count < 42; count++) {
        Epd_SendData(epd, lut_bw[count]);
    } 

    Epd_SendCommand(epd, LUT_WHITE_TO_BLACK);                      //wb w
    for(count = 0; count < 42; count++) {
        Epd_SendData(epd, lut_bb[count]);
    } 

    Epd_SendCommand(epd, LUT_BLACK_TO_BLACK);                      //bb b
    for(count = 0; count < 42; count++) {
        Epd_SendData(epd, lut_wb[count]);
    } 
}

/**
 *  @brief: transmit partial data to the SRAM
 */
void Epd_TransmitPartial(struct epd_display *epd, const unsigned char* buffer_black, const unsigned char* buffer_red, int x, int y, int w, int l) {   
    if (buffer_black != NULL) {
        Epd_SendCommand(epd, PARTIAL_DATA_START_TRANSMISSION_1);
        Epd_SendData(epd, x >> 8);
        Epd_SendData(epd, x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, y >> 8);        
        Epd_SendData(epd, y & 0xff);
        Epd_SendData(epd, w >> 8);
        Epd_SendData(epd, w & 0xf8);     // w (width) should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, l >> 8);        
        Epd_SendData(epd, l & 0xff);
        mgos_msleep(2);
        for(int i = 0; i < w  / 8 * l; i++) {
            Epd_SendData(epd, buffer_black[i]);
        }
        mgos_msleep(2);
    }
    if (buffer_red != NULL) {
        Epd_SendCommand(epd, PARTIAL_DATA_START_TRANSMISSION_2);
        Epd_SendData(epd, x >> 8);
        Epd_SendData(epd, x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, y >> 8);        
        Epd_SendData(epd, y & 0xff);
        Epd_SendData(epd, w >> 8);
        Epd_SendData(epd, w & 0xf8);     // w (width) should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, l >> 8);        
        Epd_SendData(epd, l & 0xff);
        mgos_msleep(2);
        for(int i = 0; i < w  / 8 * l; i++) {
            Epd_SendData(epd, buffer_red[i]);
        }
        mgos_msleep(2);
    }
}

/**
 *  @brief: transmit partial data to the black part of SRAM
 */
void Epd_TransmitPartialBlack(struct epd_display *epd, const unsigned char* buffer_black, int x, int y, int w, int l) {
    if (buffer_black != NULL) {
        Epd_SendCommand(epd, PARTIAL_DATA_START_TRANSMISSION_1);
        Epd_SendData(epd, x >> 8);
        Epd_SendData(epd, x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, y >> 8);        
        Epd_SendData(epd, y & 0xff);
        Epd_SendData(epd, w >> 8);
        Epd_SendData(epd, w & 0xf8);     // w (width) should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, l >> 8);        
        Epd_SendData(epd, l & 0xff);
        mgos_msleep(2);
        for(int i = 0; i < w  / 8 * l; i++) {
            Epd_SendData(epd, buffer_black[i]);  
        }  
        mgos_msleep(2);                  
    }
}

/**
 *  @brief: transmit partial data to the red part of SRAM
 */
void Epd_TransmitPartialRed(struct epd_display *epd, const unsigned char* buffer_red, int x, int y, int w, int l) {
    if (buffer_red != NULL) {
        Epd_SendCommand(epd, PARTIAL_DATA_START_TRANSMISSION_2);
        Epd_SendData(epd, x >> 8);
        Epd_SendData(epd, x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, y >> 8);        
        Epd_SendData(epd, y & 0xff);
        Epd_SendData(epd, w >> 8);
        Epd_SendData(epd, w & 0xf8);     // w (width) should be the multiple of 8, the last 3 bit will always be ignored
        Epd_SendData(epd, l >> 8);        
        Epd_SendData(epd, l & 0xff);
        mgos_msleep(2);
        for(int i = 0; i < w  / 8 * l; i++) {
            Epd_SendData(epd, buffer_red[i]);  
        }  
        mgos_msleep(2);                  
    }
}

/**
 * @brief: refreshes a specific part of the display
 */
void Epd_RefreshPartial(struct epd_display *epd, int x, int y, int w, int l) {
    Epd_SendCommand(epd, PARTIAL_DISPLAY_REFRESH); 
    Epd_SendData(epd, x >> 8);
    Epd_SendData(epd, x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
    Epd_SendData(epd, y >> 8);        
    Epd_SendData(epd, y & 0xff);
    Epd_SendData(epd, w >> 8);
    Epd_SendData(epd, w & 0xf8);     // w (width) should be the multiple of 8, the last 3 bit will always be ignored
    Epd_SendData(epd, l >> 8);        
    Epd_SendData(epd, l & 0xff);
}

/**
 * @brief: refresh and displays the frame
 */
void Epd_DisplayFrame(struct epd_display *epd, const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red) {
    Epd_SendCommand(epd, TCON_RESOLUTION);
    Epd_SendData(epd, epd->width >> 8);
    Epd_SendData(epd, epd->width & 0xff);        //176      
    Epd_SendData(epd, epd->height >> 8);        
    Epd_SendData(epd, epd->height & 0xff);         //264

    if (frame_buffer_black != NULL) {
        Epd_SendCommand(epd, DATA_START_TRANSMISSION_1);           
        mgos_msleep(2);
        for(int i = 0; i < epd->width * epd->height / 8; i++) {
            Epd_SendData(epd, frame_buffer_black[i]);  
        }  
        mgos_msleep(2);
    }
    if (frame_buffer_red != NULL) {
        Epd_SendCommand(epd, DATA_START_TRANSMISSION_2);
        mgos_msleep(2);
        for(int i = 0; i < epd->width * epd->height / 8; i++) {
            Epd_SendData(epd, frame_buffer_red[i]);  
        }  
        mgos_msleep(2);
    }
    Epd_SendCommand(epd, DISPLAY_REFRESH);
}

/**
 * @brief: clear the frame data from the SRAM, this won't refresh the display
 */
void Epd_ClearFrame(struct epd_display *epd) {
    Epd_SendCommand(epd, TCON_RESOLUTION);
    Epd_SendData(epd, epd->width >> 8);
    Epd_SendData(epd, epd->width & 0xff);        //176      
    Epd_SendData(epd, epd->height >> 8);        
    Epd_SendData(epd, epd->height & 0xff);         //264

    Epd_SendCommand(epd, DATA_START_TRANSMISSION_1);           
    mgos_msleep(2);
    for(int i = 0; i < epd->width * epd->height / 8; i++) {
        Epd_SendData(epd, 0x00);  
    }  
    mgos_msleep(2);
    Epd_SendCommand(epd, DATA_START_TRANSMISSION_2);           
    mgos_msleep(2);
    for(int i = 0; i < epd->width * epd->height / 8; i++) {
        Epd_SendData(epd, 0x00);  
    }  
    mgos_msleep(2);
}

/**
 * @brief: This displays the frame data from SRAM
 */
void Epd_DisplaySRAM(struct epd_display *epd) {
    Epd_SendCommand(epd, DISPLAY_REFRESH); 
}

/**
 * @brief: After this command is transmitted, the chip would enter the deep-sleep mode to save power. 
 *         The deep sleep mode would return to standby by hardware reset. The only one parameter is a 
 *         check code, the command would be executed if check code = 0xA5. 
 *         You can use Epd::Reset() to awaken and use Epd::Init() to initialize.
 */
void Epd_Sleep(struct epd_display *epd) {
  Epd_SendCommand(epd, DEEP_SLEEP);
  Epd_SendData(epd, 0xa5);
}

struct epd_display epd_global;

bool mgos_epd2in7b_init(){
    epd_global.width = EPD_WIDTH;
    epd_global.height = EPD_HEIGHT;
    
    epd_global.reset_pin = mgos_sys_config_get_epd_rst_gpio();
    epd_global.dc_pin = mgos_sys_config_get_epd_dc_gpio();
    epd_global.cs_pin = mgos_sys_config_get_epd_cs_gpio();
    epd_global.busy_pin = mgos_sys_config_get_epd_busy_gpio();
    return true;
}

struct epd_display* mgos_epd2in7b_get_global(){
    return &epd_global;
}

const unsigned char lut_vcom_dc[] =
{
0x00, 0x00,
0x00, 0x1A, 0x1A, 0x00, 0x00, 0x01,        
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,        
0x00, 0x0E, 0x01, 0x0E, 0x01, 0x10,        
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,        
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,        
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,        
0x00, 0x23, 0x00, 0x00, 0x00, 0x01    
};

//R21H
const unsigned char lut_ww[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x40, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R22H    r
const unsigned char lut_bw[] =
{
0xA0, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x00, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x90, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0xB0, 0x04, 0x10, 0x00, 0x00, 0x05,
0xB0, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0xC0, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R23H    w
const unsigned char lut_bb[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x40, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x80, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

//R24H    b
const unsigned char lut_wb[] =
{
0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01,
0x20, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10,
0x10, 0x0A, 0x0A, 0x00, 0x00, 0x08,
0x00, 0x04, 0x10, 0x00, 0x00, 0x05,
0x00, 0x03, 0x0E, 0x00, 0x00, 0x0A,
0x00, 0x23, 0x00, 0x00, 0x00, 0x01
};

/* END OF FILE */


