#ifndef M5UI_H
#define M5UI_H
#include "sdkconfig.h"

#ifdef CONFIG_MICROPY_USE_TFT

#include "py/obj.h"

#endif

extern color_t ui_bg_color;
extern const mp_obj_type_t m5ui_rect_type;
extern const mp_obj_type_t m5ui_circle_type;
extern const mp_obj_type_t m5ui_title_type;
extern const mp_obj_type_t m5ui_text_type;
extern const mp_obj_type_t m5ui_button_type;
extern const mp_obj_type_t m5ui_img_type;

#endif