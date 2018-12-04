#include "sdkconfig.h"

#ifdef CONFIG_MICROPY_USE_TFT

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "rom/ets_sys.h"
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "driver/gpio.h"

#include "tft/tftspi.h"
#include "tft/tft.h"
#include "extmod/vfs_native.h"

#include "m5ui.h"

color_t ui_bg_color;

STATIC mp_obj_t m5ui_set_bg_color(mp_obj_t bg_color_in){
    uint32_t cint = mp_obj_get_int(bg_color_in);
	
    ui_bg_color.r = (cint >> 16) & 0xFF;
	ui_bg_color.g = (cint >> 8) & 0xFF;
	ui_bg_color.b = cint & 0xFF;
    TFT_fillScreen(ui_bg_color);
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(m5ui_set_bg_color_obj, m5ui_set_bg_color);

extern mp_obj_t m5ui_circle_deinit();
extern mp_obj_t m5ui_rect_deinit();
extern mp_obj_t m5ui_text_deinit();

STATIC mp_obj_t m5ui_deinit(){
    m5ui_text_deinit();
    m5ui_rect_deinit();
    m5ui_circle_deinit();
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_0(m5ui_deinit_obj, m5ui_deinit);

STATIC const mp_rom_map_elem_t m5ui_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_clear_bg),       (mp_obj_t)&m5ui_set_bg_color_obj},
    { MP_ROM_QSTR(MP_QSTR_M5UI_Deinit),    (mp_obj_t)&m5ui_deinit_obj},

    { MP_OBJ_NEW_QSTR(MP_QSTR_M5Rect),      MP_ROM_PTR(&m5ui_rect_type)   },
    { MP_OBJ_NEW_QSTR(MP_QSTR_M5Circle),    MP_ROM_PTR(&m5ui_circle_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_M5Title),     MP_ROM_PTR(&m5ui_title_type)  },
    { MP_OBJ_NEW_QSTR(MP_QSTR_M5TextBox),   MP_ROM_PTR(&m5ui_text_type)   },
    { MP_OBJ_NEW_QSTR(MP_QSTR_M5Button),    MP_ROM_PTR(&m5ui_button_type)   },
    { MP_OBJ_NEW_QSTR(MP_QSTR_M5Img),       MP_ROM_PTR(&m5ui_img_type)   },

};

STATIC MP_DEFINE_CONST_DICT(m5ui_module_globals, m5ui_module_globals_table);

//===============================================
const mp_obj_module_t mp_module_um5ui = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&m5ui_module_globals,
};
#endif