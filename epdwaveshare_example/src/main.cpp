#include "mgos.h"
#include "epd2in7b.h"
#include "epdpaint.hpp"
#include "imagedata.h"

#define COLORED     1
#define UNCOLORED   0

enum STATE {
    INIT,
    TRANSMIT_1,
    TRANSMIT_2,
    WAIT,
    SLEEP,
    END
};

enum STATE state = INIT, nextstate;

struct epd_display epd;

Paint *paint;    //width should be the multiple of 8 
unsigned char image[1024];

void epd_cb(void *ud){
    struct epd_display *epd = mgos_epd2in7b_get_global();
    switch(state){
        case WAIT:
        LOG(LL_ERROR, ("WAIT"));
        if(Epd_IsBusy(epd)){
            LOG(LL_ERROR, ("EPD BUSY"));
        } else {
            state = nextstate;
        }
        break;
        case INIT:
        LOG(LL_ERROR, ("INIT"));
        Epd_Init(epd);
        Epd_ClearFrame(epd);
        LOG(LL_ERROR, ("FRAME CLEARED"));
        state = TRANSMIT_1;
        break;
        case TRANSMIT_1:
        LOG(LL_ERROR, ("TRANSMIT_1"));
        paint->Clear(UNCOLORED);
        paint->DrawStringAt(0, 0, "e-Paper Demo", &Font16, COLORED);
        Epd_TransmitPartialBlack(epd, paint->GetImage(), 16, 32, paint->GetWidth(), paint->GetHeight());

        paint->Clear(COLORED);
        paint->DrawStringAt(2, 2, "Hello world!", &Font20, UNCOLORED);
        Epd_TransmitPartialRed(epd, paint->GetImage(), 0, 64, paint->GetWidth(), paint->GetHeight());
          
        paint->SetWidth(64);
        paint->SetHeight(64);

        paint->Clear(UNCOLORED);
        paint->DrawRectangle(0, 0, 40, 50, COLORED);
        paint->DrawLine(0, 0, 40, 50, COLORED);
        paint->DrawLine(40, 0, 0, 50, COLORED);
        Epd_TransmitPartialBlack(epd, paint->GetImage(), 10, 130, paint->GetWidth(), paint->GetHeight());
          
        paint->Clear(UNCOLORED);
        paint->DrawCircle(32, 32, 30, COLORED);
        Epd_TransmitPartialBlack(epd, paint->GetImage(), 90, 120, paint->GetWidth(), paint->GetHeight());

        paint->Clear(UNCOLORED);
        paint->DrawFilledRectangle(0, 0, 40, 50, COLORED);
        Epd_TransmitPartialRed(epd, paint->GetImage(), 10, 200, paint->GetWidth(), paint->GetHeight());

        paint->Clear(UNCOLORED);
        paint->DrawFilledCircle(32, 32, 30, COLORED);
        Epd_TransmitPartialRed(epd, paint->GetImage(), 90, 190, paint->GetWidth(), paint->GetHeight());
        nextstate = TRANSMIT_2;
        state = WAIT;
        Epd_DisplaySRAM(epd);
        break;
        case TRANSMIT_2:
        LOG(LL_ERROR, ("TRANSMIT_2"));
        Epd_DisplayFrame(epd, IMAGE_BLACK, IMAGE_RED);
        nextstate = SLEEP;
        state = WAIT;
        break;
        case SLEEP:
        LOG(LL_ERROR, ("SLEEPING"));
        Epd_Sleep(epd);
        state = END;
        break;
        case END:
        LOG(LL_ERROR, ("FINISHED"));
        break;
    }
}

enum mgos_app_init_result mgos_app_init(void) {
    paint = new Paint(image, 176, 24);
    mgos_set_timer(1000, true, epd_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}