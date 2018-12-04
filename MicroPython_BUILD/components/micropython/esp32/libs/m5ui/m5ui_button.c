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

#include "driver/gpio.h"

#include "tft/tftspi.h"
#include "tft/tft.h"
#include "extmod/vfs_native.h"
#include "machine_hw_spi.h"
#include "modmachine.h"
#include "m5ui.h"
#endif

#define BUTTONA_X 34
#define BUTTONB_X 126
#define BUTTONC_X 222
#define BUTTON_Y 216
#define BUTTON_W 70
#define BUTTON_H 30

typedef struct _m5ui_button_obj_t {
    mp_obj_base_t base;
    uint8_t visibility;
    uint8_t x;
    char text[10];
} m5ui_button_obj_t;

m5ui_button_obj_t btn_a = {.x = BUTTONA_X};
m5ui_button_obj_t btn_b = {.x = BUTTONB_X};
m5ui_button_obj_t btn_c = {.x = BUTTONC_X};


// STATIC mp_obj_t m5ui_button_hide(mp_obj_t self_in) {
//     m5ui_button_obj_t *self = self_in;
//     TFT_fillRect(self->x, self->y, self->w, self->h, ui_bg_color);
//     return mp_const_none;

// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_button_hide_obj, m5ui_button_hide);

// STATIC mp_obj_t m5ui_button_show(mp_obj_t self_in) {
//     m5ui_button_obj_t *self = self_in;
//     TFT_fillRect(self->x, self->y,self->w, self->h, self->fill_color);
//     TFT_drawRect(self->x, self->y, self->w, self->h, self->draw_color);
//     return mp_const_none;

// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_button_show_obj, m5ui_button_show);

STATIC void m5ui_button_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "hello m5 button...");    
}

mp_obj_t m5ui_button_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_button_init_allowed_args[] = {
        { MP_QSTR_name,            MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_text,            MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_visibility,                        MP_ARG_INT, {.u_int = 0}},
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_button_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_button_init_allowed_args), m5ui_button_init_allowed_args, args);

    const char *tstr = NULL;
    m5ui_button_obj_t *self  = NULL;
    tstr = mp_obj_str_get_str(args[0].u_obj);
    if(strlen(tstr) > 9){
        mp_raise_ValueError("button name too long");
    }
    
    if(strcmp("ButtonA", tstr) == 0){
        self = &btn_a;
    } else if(strcmp("ButtonB", tstr) == 0){
        self = &btn_b;
    } else {
        self = &btn_c;
    } 

    tstr = mp_obj_str_get_str(args[1].u_obj);
    if(strlen(tstr) > 9){
        mp_raise_ValueError("button name too long");
    }

    sprintf(self->text, "%s", tstr);
    self->visibility = args[2].u_int;
    color_t white = { .r = 0xff, .g=0xff, .b=0xff };
    if(self->visibility == 1) {
        TFT_drawRoundRect(self->x, BUTTON_Y, BUTTON_W, BUTTON_H, 5, white);
        color_t old_fg = _fg;
        _fg = white;
        TFT_setFont(0, (char *)NULL);
        TFT_print(self->text, self->x + BUTTON_W/2 - TFT_getStringWidth(self->text)/2, 224);
        _fg = old_fg;
    }
    return MP_OBJ_FROM_PTR(self);
}

//===============================================================
STATIC const mp_rom_map_elem_t m5ui_button_locals_dict_table[] = {
    // { MP_ROM_QSTR(MP_QSTR_hide),            (mp_obj_t)&m5ui_button_hide_obj},
    // { MP_ROM_QSTR(MP_QSTR_show),            (mp_obj_t)&m5ui_button_show_obj},
};

STATIC MP_DEFINE_CONST_DICT(m5ui_button_locals_dict, m5ui_button_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_button_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5Button,
    .make_new = m5ui_button_make_new,
    .print = m5ui_button_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_button_locals_dict,
};