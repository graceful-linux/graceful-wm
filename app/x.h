//
// Created by dingjing on 23-11-24.
//

#ifndef GRACEFUL_WM_X_H
#define GRACEFUL_WM_X_H

#include "types.h"


void x_container_init           (GWMContainer* con);
void x_container_kill           (GWMContainer* con);
void x_container_reframe        (GWMContainer* con);

void x_move_window              (GWMContainer* src, GWMContainer* destination);
void x_reparent_child           (GWMContainer* con, GWMContainer* old);
void x_reinit                   (GWMContainer* con);

bool x_window_supports_protocol (xcb_window_t window, xcb_atom_t atom);
void x_window_kill              (xcb_window_t window, GWMKillWindow killWindow);

void x_draw_decoration          (GWMContainer* con);

void x_decoration_recurse       (GWMContainer* con);
void x_push_node                (GWMContainer* con);
void x_push_changes             (GWMContainer* con);
void x_raise_container          (GWMContainer* con);

void x_set_name                 (GWMContainer* con, const char* name);
void x_set_gwm_atoms            (void);
void x_set_warp_to              (GWMRect* rect);
void x_set_shape                (GWMContainer* con, xcb_shape_sk_t kind, bool enable);
void x_mask_event_mask          (uint32_t mask);
void x_update_shmlog_atom       (void);

#endif //GRACEFUL_WM_X_H
