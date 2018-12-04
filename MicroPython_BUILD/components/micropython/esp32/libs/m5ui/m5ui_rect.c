#include "sdkconfig.h"

#ifdef CONFIG_MICROPY_USE_TFT

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "driver/gpio.h"

#include "tft/tftspi.h"
#include "tft/tft.h"
#include "extmod/vfs_native.h"
#include "machine_hw_spi.h"
#include "modmachine.h"
#include "libs/qrcode.h"

#include "m5ui.h"
#endif

#if !defined(max)
#define max(A,B) ( (A) > (B) ? (A):(B))
#endif
#if !defined(min)
#define min(A,B) ( (A) < (B) ? (A):(B))
#endif

typedef struct _m5ui_rect_obj_t {
    mp_obj_base_t base;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    color_t fill_color;
    color_t draw_color;
} m5ui_rect_obj_t;

typedef struct _rect_list_t {
    uint8_t head;
    m5ui_rect_obj_t *list[20];
} rect_list_t;

rect_list_t rect_list = {
    .head = 0,
};

STATIC color_t intToColor(uint32_t cint)
{
	color_t cl = {0,0,0};
	cl.r = (cint >> 16) & 0xFF;
	cl.g = (cint >> 8) & 0xFF;
	cl.b = cint & 0xFF;
	return cl;
}

STATIC void m5ui_rect_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "hello m5 rect...");
}

mp_obj_t m5ui_rect_deinit() {
    for(uint8_t i = 0; i < rect_list.head; i++) {
        // m_del_obj(m5ui_rect_obj_t, rect_list.list[i]);
        rect_list.list[i] = NULL;
    }
    rect_list.head = 0;
    return mp_const_none;
}

// STATIC MP_DEFINE_CONST_FUN_OBJ_0(m5ui_rect_deinit_obj, m5ui_rect_deinit);

STATIC mp_obj_t m5ui_rect_resize(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    m5ui_rect_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_width,    MP_ARG_INT,     { .u_int = self->w } },
        { MP_QSTR_height,   MP_ARG_INT,     { .u_int = self->h } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint16_t width = args[0].u_int;
    uint16_t hight = args[1].u_int;
    
    if(width == self->w && hight == self->h) {
        return mp_const_none;
    }

    if(width > self->w && hight > self->h) {
        self->w = width;
        self->h = hight;
        TFT_fillRect(self->x, self->y,self->w, self->h, self->fill_color);
        TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
        return mp_const_none;
    }

    if(width > self->w) {
        if(self->w > 0) { self->w -= 1; }
        TFT_fillRect(self->x + self->w, self->y, width - self->w, self->h, self->fill_color);
    } else if(width < self->w) {
        TFT_fillRect(self->x + width, self->y, self->w - width, self->h, ui_bg_color);
    }
    self->w = width;

    if(hight > self->h){
        if(self->h > 0) { self->h -= 1; }
        TFT_fillRect(self->x, self->y + self->h, self->w, hight - self->h, self->fill_color);
    } else if(hight < self->h) {
        TFT_fillRect(self->x, self->y + hight, self->w + 1, self->h - hight + 1, ui_bg_color);
    }
    self->h = hight;
   
    TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    return mp_const_none;

}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(m5ui_rect_resize_obj, 1, m5ui_rect_resize);

STATIC mp_obj_t m5ui_rect_setposition(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    m5ui_rect_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,    MP_ARG_INT,     { .u_int = self->x } },
        { MP_QSTR_y,    MP_ARG_INT,     { .u_int = self->y } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if(args[0].u_int != self->x || args[1].u_int != self->y) {
        TFT_fillRect(self->x, self->y,self->w, self->h, ui_bg_color);
        self->x = args[0].u_int;   
        self->y = args[1].u_int;
    }
    TFT_fillRect(self->x, self->y,self->w, self->h, self->fill_color);
    TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(m5ui_rect_setposition_obj, 1, m5ui_rect_setposition);


STATIC mp_obj_t m5ui_rect_hide(mp_obj_t self_in) {
    m5ui_rect_obj_t *self = self_in;
    TFT_fillRect(self->x, self->y, self->w, self->h, ui_bg_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_rect_hide_obj, m5ui_rect_hide);

STATIC mp_obj_t m5ui_rect_show(mp_obj_t self_in) {
    m5ui_rect_obj_t *self = self_in;
    TFT_fillRect(self->x, self->y,self->w, self->h, self->fill_color);
    TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_rect_show_obj, m5ui_rect_show);

STATIC mp_obj_t m5ui_rect_setbgcolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_rect_obj_t *self = self_in;
    self->fill_color = intToColor(mp_obj_get_int(color_in));
    TFT_fillRect(self->x, self->y,self->w, self->h, self->fill_color);
    TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_rect_setbgcolor_obj, m5ui_rect_setbgcolor);

STATIC mp_obj_t m5ui_rect_setbordercolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_rect_obj_t *self = self_in;
    self->draw_color = intToColor(mp_obj_get_int(color_in));
    TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_rect_setbordercolor_obj, m5ui_rect_setbordercolor);

STATIC mp_obj_t m5ui_rect_isoverlap(mp_obj_t self_in, mp_obj_t rect_in) {
    m5ui_rect_obj_t *self = self_in;
    m5ui_rect_obj_t *rect = rect_in;
    
    bool result = false;

    result = (max(self->x, rect->x) > min(self->x + self->w, rect->x + rect->w))
          || (max(self->y, rect->y) > min(self->y + self->h, rect->y + rect->h));

    result = !result;
    return mp_obj_new_bool(result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_rect_isoverlap_obj, m5ui_rect_isoverlap);

mp_obj_t m5ui_rect_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    enum { ARG_x, ARG_y, ARG_w, ARG_h, ARG_fillcolor, ARG_drawcolor };
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_rect_init_allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_width,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_height,       MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_fillcolor,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_color,                          MP_ARG_INT, {.u_int = -1}},
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_rect_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_rect_init_allowed_args), m5ui_rect_init_allowed_args, args);

    m5ui_rect_obj_t *self = m_new_obj(m5ui_rect_obj_t);
    self->base.type = &m5ui_rect_type;
    self->x = args[ARG_x].u_int;
    self->y = args[ARG_y].u_int;
    self->w = args[ARG_w].u_int;
    self->h = args[ARG_h].u_int;
    self->fill_color = intToColor(args[ARG_fillcolor].u_int);
    self->draw_color = self->fill_color;
    TFT_fillRect(self->x, self->y, self->w, self->h, self->fill_color);
    if(args[ARG_drawcolor].u_int >= 0){
        self->draw_color = intToColor(args[ARG_drawcolor].u_int);
        TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
    }
    rect_list.list[rect_list.head++] = self;
    rect_list.head = rect_list.head > 19 ? 19 : rect_list.head;
    return MP_OBJ_FROM_PTR(self);
}

//===============================================================
STATIC const mp_rom_map_elem_t m5ui_rect_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_setSize),         (mp_obj_t)&m5ui_rect_resize_obj},
    { MP_ROM_QSTR(MP_QSTR_setBgColor),      (mp_obj_t)&m5ui_rect_setbgcolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setBorderColor),  (mp_obj_t)&m5ui_rect_setbordercolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setPosition),     (mp_obj_t)&m5ui_rect_setposition_obj},
    { MP_ROM_QSTR(MP_QSTR_hide),            (mp_obj_t)&m5ui_rect_hide_obj},
    { MP_ROM_QSTR(MP_QSTR_show),            (mp_obj_t)&m5ui_rect_show_obj},

};

STATIC MP_DEFINE_CONST_DICT(m5ui_rect_locals_dict, m5ui_rect_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_rect_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5Rect,
    .make_new = m5ui_rect_make_new,
    .print = m5ui_rect_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_rect_locals_dict,
};