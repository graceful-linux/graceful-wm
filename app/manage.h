//
// Created by dingjing on 23-11-27.
//

#ifndef GRACEFUL_WM_MANAGE_H
#define GRACEFUL_WM_MANAGE_H
#include "types.h"


void manage_restore_geometry            (void);
void manage_existing_windows            (xcb_window_t root);
GWMContainer* manage_remanage_window    (GWMContainer* con);
void manage_window                      (xcb_window_t window, xcb_get_window_attributes_cookie_t cookie, bool needsToBeMapped);


#endif //GRACEFUL_WM_MANAGE_H
