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

#define TITLE_NAME_LEN 20
typedef struct _m5ui_title_obj_t {
    mp_obj_base_t base;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    char msg[TITLE_NAME_LEN];
    color_t fill_color;
    color_t draw_color;
} m5ui_title_obj_t;

m5ui_title_obj_t title;

STATIC color_t intToColor(uint32_t cint) {
	color_t cl = {0,0,0};
	cl.r = (cint >> 16) & 0xFF;
	cl.g = (cint >> 8) & 0xFF;
	cl.b = cint & 0xFF;
	return cl;
}

STATIC void m5ui_title_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "hello m5 title...");
}

STATIC mp_obj_t m5ui_title_show(mp_obj_t self_in) {
    m5ui_title_obj_t *self = self_in;
    TFT_fillRect(0, 0,self->w, self->h, self->fill_color);
    int old_font_transparent = font_transparent;
    color_t old_fg = _fg;
    font_transparent = 1; 
    _fg = self->draw_color;
    TFT_setFont(0, (char *)NULL);
    TFT_print(self->msg, self->x, self->y);
    font_transparent = old_font_transparent;
    _fg = old_fg;
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_title_show_obj, m5ui_title_show);

STATIC mp_obj_t m5ui_title_hide(mp_obj_t self_in) {
    m5ui_title_obj_t *self = self_in;
    TFT_fillRect(0, 0,self->w, self->h, ui_bg_color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_title_hide_obj, m5ui_title_hide);


STATIC mp_obj_t m5ui_title_setbgcolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_title_obj_t *self = self_in;
    self->fill_color = intToColor(mp_obj_get_int(color_in));
    TFT_fillRect(0, 0,self->w, self->h, self->fill_color);
    int old_font_transparent = font_transparent;
    color_t old_fg = _fg;
    font_transparent = 1; 
    _fg = self->draw_color;
    TFT_setFont(0, (char *)NULL);
    TFT_print(self->msg, self->x, self->y);
    font_transparent = old_font_transparent;
    _fg = old_fg;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_title_setbgcolor_obj, m5ui_title_setbgcolor);

STATIC mp_obj_t m5ui_title_setfgcolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_title_obj_t *self = self_in;
    self->draw_color = intToColor(mp_obj_get_int(color_in));
    int old_font_transparent = font_transparent;
    color_t old_fg = _fg;
    font_transparent = 1; 
    _fg = self->draw_color;
    TFT_setFont(0, (char *)NULL);
    TFT_print(self->msg, self->x, self->y);
    font_transparent = old_font_transparent;
    _fg = old_fg;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_title_setfgcolor_obj, m5ui_title_setfgcolor);

mp_obj_t m5ui_title_settitle(mp_obj_t self_in, mp_obj_t text_in) {
    m5ui_title_obj_t *self = self_in;
    char *st = (char *)mp_obj_str_get_str(text_in);
    if(strlen(st) > TITLE_NAME_LEN){
        mp_raise_ValueError("Title too long");
    }
    int old_font_transparent = font_transparent;
    color_t old_fg = _fg;
    font_transparent = 1; 
    _fg = self->fill_color;
    TFT_setFont(0, (char *)NULL);
    TFT_print(self->msg, self->x, self->y);
    _fg = self->draw_color;
    TFT_print(st, self->x, self->y);
    sprintf(self->msg, "%s", st);
    font_transparent = old_font_transparent;
    _fg = old_fg;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_title_settitle_obj, m5ui_title_settitle);


mp_obj_t m5ui_title_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_title_init_allowed_args[] = {
        { MP_QSTR_fgcolor,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1}},
        { MP_QSTR_bgcolor,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1}},
        { MP_QSTR_title,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_h,                          MP_ARG_INT, {.u_int = 20}},
        { MP_QSTR_x,        MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 3} },
        { MP_QSTR_y,        MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 4} },
        { MP_QSTR_w,        MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 320} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_title_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_title_init_allowed_args), m5ui_title_init_allowed_args, args);

    const char *tstr = NULL;
    m5ui_title_obj_t *self = &title;
    memset(self, 0, sizeof(m5ui_title_obj_t));

    self->base.type = &m5ui_title_type;
    self->draw_color = intToColor(args[0].u_int);
    self->fill_color = intToColor(args[1].u_int);
    tstr = mp_obj_str_get_str(args[2].u_obj);
    self->h = args[3].u_int;
    self->x = args[4].u_int;
    self->y = args[5].u_int;
    self->w = args[6].u_int;

    if(strlen(tstr) >= TITLE_NAME_LEN){
        mp_raise_ValueError("Title too long");
    }
    sprintf(self->msg, "%s", tstr);
    TFT_fillRect(0, 0, self->w, self->h, self->fill_color);
    int old_font_transparent = font_transparent;
    color_t old_fg = _fg;
    font_transparent = 1;
    
    _fg = self->draw_color;
    TFT_setFont(0, (char *)NULL);
    TFT_print(self->msg, self->x, self->y);

    _fg = old_fg;
    font_transparent = old_font_transparent;
    return MP_OBJ_FROM_PTR(self);
}

//===============================================================
STATIC const mp_rom_map_elem_t m5ui_title_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_hide),        (mp_obj_t)&m5ui_title_hide_obj},
    { MP_ROM_QSTR(MP_QSTR_show),        (mp_obj_t)&m5ui_title_show_obj},
    { MP_ROM_QSTR(MP_QSTR_setTitle),    (mp_obj_t)&m5ui_title_settitle_obj},
    { MP_ROM_QSTR(MP_QSTR_setFgColor),  (mp_obj_t)&m5ui_title_setfgcolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setBgColor),  (mp_obj_t)&m5ui_title_setbgcolor_obj},
};

STATIC MP_DEFINE_CONST_DICT(m5ui_title_locals_dict, m5ui_title_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_title_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5Title,
    .make_new = m5ui_title_make_new,
    .print = m5ui_title_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_title_locals_dict,
};