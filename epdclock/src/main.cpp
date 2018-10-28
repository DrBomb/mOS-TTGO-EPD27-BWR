#include "mgos.h"
#include "epd2in7b.h"
#include "epdpaint.hpp"
#include "imagedata.h"
#include <time.h>

#define COLORED     1
#define UNCOLORED   0

enum STATE {
    IDLE,
    WAIT,
    SLEEP
};

enum STATE state = IDLE;

struct epd_display epd;

Paint *paint;    //width should be the multiple of 8 
unsigned char *framebuf;

void epd_cb(void *ud){
    struct epd_display *epd = mgos_epd2in7b_get_global();
    switch(state){
        case WAIT:
        if(Epd_IsBusy(epd)){
            LOG(LL_ERROR, ("EPD BUSY"));
        } else {
            state = SLEEP;
        }
        break;
        case SLEEP:
        LOG(LL_ERROR, ("Sleeping"));
        Epd_Sleep(epd);
        state = IDLE;
        break;
        case IDLE:
        break;
    }
}

static void timer_cb(void *ud){
    struct epd_display *epd = mgos_epd2in7b_get_global();
    Epd_Init(epd);
    Epd_ClearFrame(epd);
    paint->Clear(COLORED);
    time_t t = time(NULL);
    struct tm *loct;
    loct = localtime(&t);
    char tstring[17];
    strftime(tstring, 17, "%a %b %d", loct);
    paint->DrawStringAt(0, 0, tstring, &Font16, UNCOLORED);
    strftime(tstring, 17, "%I:%M %p", loct);
    paint->DrawStringAt(0, 17, tstring, &Font16, UNCOLORED);
    Epd_TransmitPartialBlack(epd, paint->GetImage(), 0, (EPD_HEIGHT/2)-8, paint->GetWidth(), 16*2);
    Epd_RefreshPartial(epd, 0, (EPD_HEIGHT/2)-8, paint->GetWidth(), 16*2);
    state = WAIT;
    LOG(LL_ERROR, ("MINUTE"));
}

static void time_change_cb(int ev, void *ev_data, void* ud){
    struct epd_display *epd = mgos_epd2in7b_get_global();
    time_t t = time(NULL);
    struct tm *loct;
    loct = localtime(&t);
    char tstring[17];
    strftime(tstring, 17, "%a %b %d", loct);
    paint->Clear(COLORED); // Clear the whole frame
    paint->DrawStringAt(0, (EPD_HEIGHT/2)-8, tstring, &Font16, UNCOLORED);
    strftime(tstring, 17, "%I:%M %p", loct);
    paint->DrawStringAt(0, (EPD_HEIGHT/2)+8, tstring, &Font16, UNCOLORED);
    Epd_TransmitPartialBlack(epd, paint->GetImage(), 0, 0, paint->GetWidth(), paint->GetHeight());
    Epd_DisplaySRAM(epd);
    state = WAIT;
    printf("UTC:       %s", asctime(gmtime(&t)));
    printf("local:     %s", asctime(localtime(&t)));
}

enum mgos_app_init_result mgos_app_init(void) {
    struct epd_display *epd = mgos_epd2in7b_get_global();
    framebuf = (unsigned char*)malloc((EPD_WIDTH/8)*EPD_HEIGHT);
    paint = new Paint(framebuf, EPD_WIDTH, EPD_HEIGHT);
    Epd_Init(epd);
    Epd_ClearFrame(epd);
    mgos_event_add_handler(MGOS_EVENT_TIME_CHANGED, time_change_cb, NULL);
    mgos_set_timer(60000, true, timer_cb, NULL);
    mgos_set_timer(1000, true, epd_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}