//
// Created by dingjing on 23-11-27.
//

#ifndef GRACEFUL_WM_FLOATING_H
#define GRACEFUL_WM_FLOATING_H
#include "tree.h"
#include "types.h"


void floating_disable(GWMContainer* con);
void floating_raise_con(GWMContainer* con);
void floating_move_to_pointer(GWMContainer* con);
bool floating_maybe_reassign_ws(GWMContainer* con);
void floating_center(GWMContainer* con, GWMRect rect);
bool floating_enable(GWMContainer* con, bool automatic);
bool floating_reposition(GWMContainer* con, GWMRect newRect);
void floating_toggle_floating_mode(GWMContainer* con, bool automatic);
void floating_check_size(GWMContainer* floatingCon, bool preferHeight);
void floating_resize(GWMContainer* floatingCon, uint32_t x, uint32_t y);
void floating_fix_coordinates(GWMContainer* con, GWMRect* oldRect, GWMRect* newRect);
void floating_drag_window(GWMContainer* con, const xcb_button_press_event_t *event, bool useThreshold);
void floating_resize_window(GWMContainer* con, bool proportional, const xcb_button_press_event_t* event);



#endif //GRACEFUL_WM_FLOATING_H
