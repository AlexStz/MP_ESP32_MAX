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

typedef struct _m5ui_circle_obj_t {
    mp_obj_base_t base;
    uint16_t x;
    uint16_t y;
    uint16_t r;
    color_t fill_color;
    color_t draw_color;
} m5ui_circle_obj_t;

typedef struct _circle_list_t {
    uint8_t head;
    m5ui_circle_obj_t *list[20];
} circle_list_t;

circle_list_t circle_list = {
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

STATIC void m5ui_circle_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "hello m5 circle...");
}

mp_obj_t m5ui_circle_deinit() {
    for(uint8_t i = 0; i < circle_list.head; i++) {
        if(circle_list.list[i] != NULL) {
            // m_del_obj(m5ui_circle_obj_t, circle_list.list[i]);
            circle_list.list[i] = NULL;
        }
    }
    circle_list.head = 0;
    return mp_const_none;
}

// STATIC MP_DEFINE_CONST_FUN_OBJ_0(m5ui_circle_deinit_obj, m5ui_circle_deinit);

STATIC mp_obj_t m5ui_circle_resize(mp_obj_t self_in, mp_obj_t r_in) {
    m5ui_circle_obj_t *self = self_in;
    uint16_t r = mp_obj_get_int(r_in);
    if( r == self->r) {
        return mp_const_none;
    } else if(r > self->r) {
        TFT_fillCircle(self->x, self->y, r, self->fill_color);
    } else {
        TFT_fillCircle(self->x, self->y, self->r + 1, ui_bg_color);
        TFT_fillCircle(self->x, self->y, r, self->fill_color);    
    }
    TFT_drawCircle(self->x, self->y, r, self->draw_color);
    self->r = r;
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_circle_resize_obj, m5ui_circle_resize);

STATIC mp_obj_t m5ui_circle_setposition(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    m5ui_circle_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,    MP_ARG_INT,     { .u_int = self->x } },
        { MP_QSTR_y,    MP_ARG_INT,     { .u_int = self->y } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    if(self->x != args[0].u_int || self->y != args[1].u_int){
        TFT_fillCircle(self->x, self->y, self->r + 1, ui_bg_color);
        self->x = args[0].u_int;   
        self->y = args[1].u_int;
    }
    TFT_fillCircle(self->x, self->y,self->r, self->fill_color);
    TFT_drawCircle(self->x, self->y, self->r, self->draw_color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(m5ui_circle_setposition_obj, 1, m5ui_circle_setposition);


STATIC mp_obj_t m5ui_circle_setbgcolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_circle_obj_t *self = self_in;
    self->fill_color = intToColor(mp_obj_get_int(color_in));
    TFT_fillCircle(self->x, self->y,self->r, self->fill_color);
    TFT_drawCircle(self->x, self->y, self->r, self->draw_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_circle_setbgcolor_obj, m5ui_circle_setbgcolor);

STATIC mp_obj_t m5ui_circle_setbordercolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_circle_obj_t *self = self_in;
    self->draw_color = intToColor(mp_obj_get_int(color_in));
    TFT_drawCircle(self->x, self->y, self->r, self->draw_color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_circle_setbordercolor_obj, m5ui_circle_setbordercolor);

STATIC mp_obj_t m5ui_circle_show(mp_obj_t self_in) {
    m5ui_circle_obj_t *self = self_in;
    TFT_fillCircle(self->x, self->y,self->r, self->fill_color);
    TFT_drawCircle(self->x, self->y, self->r, self->draw_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_circle_show_obj, m5ui_circle_show);

STATIC mp_obj_t m5ui_circle_hide(mp_obj_t self_in) {
    m5ui_circle_obj_t *self = self_in;
    TFT_fillCircle(self->x, self->y,self->r, ui_bg_color);
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_circle_hide_obj, m5ui_circle_hide);

mp_obj_t m5ui_circle_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    enum { ARG_x, ARG_y, ARG_r, ARG_fillcolor, ARG_drawcolor };
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_circle_init_allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_r,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_fillcolor,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_color,                          MP_ARG_INT, {.u_int = -1}},
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_circle_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_circle_init_allowed_args), m5ui_circle_init_allowed_args, args);

    m5ui_circle_obj_t *self = m_new_obj(m5ui_circle_obj_t);
    self->base.type = &m5ui_circle_type;
    self->x = args[ARG_x].u_int;
    self->y = args[ARG_y].u_int;
    self->r = args[ARG_r].u_int;
    self->fill_color = intToColor(args[ARG_fillcolor].u_int);
    self->draw_color = self->fill_color;
    TFT_fillCircle(self->x, self->y, self->r, self->fill_color);
    if(args[ARG_drawcolor].u_int >= 0){
        self->draw_color = intToColor(args[ARG_drawcolor].u_int);
        TFT_drawCircle(self->x, self->y, self->r, self->draw_color);
    }
    circle_list.list[circle_list.head++] = self;
    circle_list.head = circle_list.head > 19 ? 19 : circle_list.head;
    return MP_OBJ_FROM_PTR(self);
}

//===============================================================
STATIC const mp_rom_map_elem_t m5ui_circle_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_setSize),         (mp_obj_t)&m5ui_circle_resize_obj},
    { MP_ROM_QSTR(MP_QSTR_setBgColor),      (mp_obj_t)&m5ui_circle_setbgcolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setBorderColor),  (mp_obj_t)&m5ui_circle_setbordercolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setPosition),     (mp_obj_t)&m5ui_circle_setposition_obj},
    { MP_ROM_QSTR(MP_QSTR_hide),            (mp_obj_t)&m5ui_circle_hide_obj},
    { MP_ROM_QSTR(MP_QSTR_show),            (mp_obj_t)&m5ui_circle_show_obj},

    // { MP_ROM_QSTR(MP_QSTR_deinit),          (mp_obj_t)&m5ui_circle_deinit_obj},
};

STATIC MP_DEFINE_CONST_DICT(m5ui_circle_locals_dict, m5ui_circle_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_circle_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5Circle,
    .make_new = m5ui_circle_make_new,
    .print = m5ui_circle_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_circle_locals_dict,
};