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

typedef struct _m5ui_text_obj_t {
    mp_obj_base_t base;
    char *text;
    uint8_t text_max_len;
    uint8_t font;
    uint16_t x;
    uint16_t y;
    uint16_t rotate;
    color_t color;
} m5ui_text_obj_t;

typedef struct _text_list_t {
    uint8_t head;
    m5ui_text_obj_t *list[20];
} text_list_t;

text_list_t text_list = {
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

STATIC void m5ui_text_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "hello m5 text...");
}

mp_obj_t m5ui_text_deinit() {
    for(uint8_t i = 0; i < text_list.head; i++) {
        // m_del_obj(m5ui_text_obj_t, text_list.list[i]);
        text_list.list[i] = NULL;
    }
    text_list.head = 0;
    return mp_const_none;
}

// STATIC MP_DEFINE_CONST_FUN_OBJ_0(m5ui_text_deinit_obj, m5ui_text_deinit);

STATIC mp_obj_t m5ui_text_show(mp_obj_t self_in) {
    m5ui_text_obj_t *self = self_in;
    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    
    _fg = self->color;
    font_rotate = self->rotate;
    font_transparent = 1;

    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }    

    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    return mp_const_none;

}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_text_show_obj, m5ui_text_show);

STATIC mp_obj_t m5ui_text_hide(mp_obj_t self_in) {
    m5ui_text_obj_t *self = self_in;
    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    
    _fg = ui_bg_color;
    font_rotate = self->rotate;
    font_transparent = 1;

    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }    

    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_text_hide_obj, m5ui_text_hide);

STATIC mp_obj_t m5ui_text_font(mp_obj_t self_in, mp_obj_t font_in) {
    m5ui_text_obj_t *self = self_in;
    uint8_t font = mp_obj_get_int(font_in);
    if(font == self->font) { return mp_const_none; }

    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    
    _fg = ui_bg_color;
    font_rotate = self->rotate;
    font_transparent = 1;

    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }

    TFT_print(self->text, self->x, self->y);
    _fg = self->color;
    TFT_setFont(font, (char *)NULL);
    TFT_print(self->text, self->x, self->y);

    self->font = font;
    font_now = font;

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_text_font_obj, m5ui_text_font);

STATIC mp_obj_t m5ui_text_rotate(mp_obj_t self_in, mp_obj_t rotate_in) {
    m5ui_text_obj_t *self = self_in;
    uint16_t rotate = mp_obj_get_int(rotate_in);
    if(rotate == self->rotate) { return mp_const_none; }
    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;

    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }

    _fg = ui_bg_color;
    font_rotate = self->rotate;
    font_transparent = 1;
    
    TFT_print(self->text, self->x, self->y);
    _fg = self->color;
    self->rotate = rotate;
    font_rotate = rotate;
    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_text_rotate_obj, m5ui_text_rotate);

STATIC mp_obj_t m5ui_text_setposition(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    m5ui_text_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,    MP_ARG_INT,     { .u_int = self->x } },
        { MP_QSTR_y,    MP_ARG_INT,     { .u_int = self->y } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    uint16_t x = args[0].u_int;
    uint16_t y = args[1].u_int;
    if(x == self->x && y == self->y) { return mp_const_none; }

    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;

    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }

    font_rotate = self->rotate;
    font_transparent = 1;

    _fg = ui_bg_color;
    TFT_print(self->text, self->x, self->y);
    _fg = self->color;
    self->x = x;
    self->y = y;
    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(m5ui_text_setposition_obj, 1, m5ui_text_setposition);


STATIC mp_obj_t m5ui_text_setcolor(mp_obj_t self_in, mp_obj_t color_in) {
    m5ui_text_obj_t *self = self_in;
    self->color = intToColor(mp_obj_get_int(color_in));
    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    
    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }
    
    _fg = self->color;
    font_rotate = self->rotate;
    font_transparent = 1;

    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_text_setcolor_obj, m5ui_text_setcolor);

STATIC mp_obj_t m5ui_text_settext(mp_obj_t self_in, mp_obj_t text_in) {
    m5ui_text_obj_t *self = self_in;
    const char *tstr = mp_obj_str_get_str(text_in);

    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    
    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }
    
    if(strcmp(self->text, tstr) == 0) {
        return mp_const_none;
    }

    _fg = ui_bg_color;
    font_rotate = self->rotate;
    font_transparent = 1;

    TFT_print(self->text, self->x, self->y);

    if(strlen(tstr) > self->text_max_len) {
        self->text_max_len = strlen(tstr);
        char *text_n = m_new(char, self->text_max_len);
        strcpy(text_n, tstr);
        m_free(self->text);
        self->text = text_n;
    } else {
        strcpy(self->text, tstr);
    }

    _fg = self->color;
    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_text_settext_obj, m5ui_text_settext);


mp_obj_t m5ui_text_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_x, ARG_y, ARG_text, ARG_font, ARG_color, ARG_rotate };
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_text_init_allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_text,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_font,                           MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_color,                          MP_ARG_INT, {.u_int = -1}},
        { MP_QSTR_rotate,       MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0}},
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_text_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_text_init_allowed_args), m5ui_text_init_allowed_args, args);

    m5ui_text_obj_t *self = m_new_obj(m5ui_text_obj_t);
    const char *tstr = NULL;

    self->base.type = &m5ui_text_type;
    self->x = args[ARG_x].u_int;
    self->y = args[ARG_y].u_int;
    self->color = intToColor(args[ARG_color].u_int);
    self->font = args[ARG_font].u_int;
    self->rotate = args[ARG_rotate].u_int;
    tstr = (char *)mp_obj_str_get_str(args[ARG_text].u_obj);
    self->text_max_len = strlen(tstr);
    char *text_n = m_new(char, self->text_max_len);
    strcpy(text_n, tstr);
    self->text = text_n;
    
    color_t old_fg = _fg;
    int old_rot = font_rotate;
    int old_transp = font_transparent;
    if(font_now != self->font){
        TFT_setFont(self->font, (char *)NULL);
    }

    _fg = self->color;
    font_rotate = self->rotate;
    font_transparent = 1;
    TFT_print(self->text, self->x, self->y);

    _fg = old_fg;
    font_rotate = old_rot;
    font_transparent = old_transp;
    text_list.list[text_list.head++] = self;
    text_list.head = text_list.head > 19 ? 19 : text_list.head;
    return MP_OBJ_FROM_PTR(self);
}

//===============================================================
STATIC const mp_rom_map_elem_t m5ui_text_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_setColor),         (mp_obj_t)&m5ui_text_setcolor_obj},
    { MP_ROM_QSTR(MP_QSTR_setPosition),      (mp_obj_t)&m5ui_text_setposition_obj},
    { MP_ROM_QSTR(MP_QSTR_setFont),          (mp_obj_t)&m5ui_text_font_obj},
    { MP_ROM_QSTR(MP_QSTR_setRotate),        (mp_obj_t)&m5ui_text_rotate_obj},
    { MP_ROM_QSTR(MP_QSTR_setText),          (mp_obj_t)&m5ui_text_settext_obj},
    { MP_ROM_QSTR(MP_QSTR_hide),             (mp_obj_t)&m5ui_text_hide_obj},
    { MP_ROM_QSTR(MP_QSTR_show),             (mp_obj_t)&m5ui_text_show_obj},

    // { MP_ROM_QSTR(MP_QSTR_deinit),          (mp_obj_t)&m5ui_text_deinit_obj},
};

STATIC MP_DEFINE_CONST_DICT(m5ui_text_locals_dict, m5ui_text_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_text_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5TextBox,
    .make_new = m5ui_text_make_new,
    .print = m5ui_text_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_text_locals_dict,
};