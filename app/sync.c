//
// Created by dingjing on 23-12-12.
//

#include "sync.h"

#include "val.h"
#include "log.h"


void sync_respond(xcb_window_t window, uint32_t rnd)
{
    DEBUG("[gwm sync protocol] Sending random value %d back to X11 window 0x%08x", rnd, window);

    void *reply = calloc(32, 1);
    xcb_client_message_event_t *ev = reply;

    ev->response_type = XCB_CLIENT_MESSAGE;
    ev->window = window;
    ev->type = A_GWM_SYNC;
    ev->format = 32;
    ev->data.data32[0] = window;
    ev->data.data32[1] = rnd;

    xcb_send_event(gConn, false, window, XCB_EVENT_MASK_NO_EVENT, (char*) ev);
    xcb_flush(gConn);
    FREE(reply);
}
