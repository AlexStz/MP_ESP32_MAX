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

#if !defined(max)
#define max(A,B) ( (A) > (B) ? (A):(B))
#endif
#if !defined(min)
#define min(A,B) ( (A) < (B) ? (A):(B))
#endif
typedef struct _m5ui_img_obj_t {
    mp_obj_base_t base;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t hight;
    uint8_t type;
    uint8_t visibility;
    char text[20];
} m5ui_img_obj_t;

STATIC void img_show(m5ui_img_obj_t *self) {
    char fullname[128] = {'\0'};
    physicalPath(self->text, fullname);
    if(self->type == IMAGE_TYPE_BMP) {
        TFT_bmp_image(self->x, self->y, 0, fullname, NULL, 0);
    } else if (self->type == IMAGE_TYPE_JPG) {
        TFT_jpg_image(self->x, self->y, 0, fullname, NULL, 0);
    } else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported image type"));
    }
    self->width = image_width;
    self->hight = image_hight;
    self->visibility = 1;
}

STATIC void m5ui_img_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    m5ui_img_obj_t *self = self_in;
    mp_printf(print, "img size x:%d y:%d", self->width, self->hight);    
}

mp_obj_t m5ui_img_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args){
    //---------------------------------------------------------------------
    const mp_arg_t m5ui_img_init_allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_name,         MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_visibility,                     MP_ARG_INT, {.u_int = 1}},
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(m5ui_img_init_allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(m5ui_img_init_allowed_args), m5ui_img_init_allowed_args, args);

    m5ui_img_obj_t *self = m_new_obj(m5ui_img_obj_t);
    self->base.type = &m5ui_img_type;
    self->x = args[0].u_int;
    self->y = args[1].u_int;
    self->visibility = args[3].u_int;

    char *fname = NULL;
    int img_type = 0;
    char fullname[128] = {'\0'};
    fname = (char *)mp_obj_str_get_str(args[2].u_obj);
    if(strlen(fname) > 19) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported image type"));
    }
    int res = physicalPath(fname, fullname);
    if ((res != 0) || (strlen(fullname) == 0)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Error resolving file name"));
    }

    char upr_fname[128];
    strcpy(upr_fname, fname);
    for (int i=0; i < strlen(upr_fname); i++) {
        upr_fname[i] = toupper((unsigned char) upr_fname[i]);
    }
    if (strstr(upr_fname, ".JPG") != NULL) img_type = IMAGE_TYPE_JPG;
    else if (strstr(upr_fname, ".BMP") != NULL) img_type = IMAGE_TYPE_BMP;
    else {
        FILE *fhndl = fopen(fullname, "r");
        if (fhndl != NULL) {
            uint8_t buf[16];
            if (fread(buf, 1, 11, fhndl) == 11) {
                buf[10] = 0;
                if (strstr((char *)(buf+6), "JFIF") != NULL) img_type = IMAGE_TYPE_JPG;
                else if ((buf[0] = 0x42) && (buf[1] = 0x4d)) img_type = IMAGE_TYPE_BMP;
            }
            fclose(fhndl);
        }
    }

    if (img_type == IMAGE_TYPE_BMP) {
        if(self->visibility) {
            TFT_bmp_image(args[0].u_int, args[1].u_int, 0, fullname, NULL, 0);
        }
    }
    else if (img_type == IMAGE_TYPE_JPG) {
        if(self->visibility) {
            TFT_jpg_image(args[0].u_int, args[1].u_int, 0, fullname, NULL, 0);
        }
    }
    else {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Unsupported image type"));
    }
    sprintf(self->text, "%s", fname);
    self->type = img_type;
    self->width = image_width;
    self->hight = image_hight;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t m5ui_img_hide(mp_obj_t self_in) {
    m5ui_img_obj_t *self = self_in;
    if(self->visibility) {
        TFT_fillRect(self->x, self->y, self->width, self->hight, ui_bg_color);
        self->visibility = 0;
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_img_hide_obj, m5ui_img_hide);

STATIC mp_obj_t m5ui_img_show(mp_obj_t self_in) {
    m5ui_img_obj_t *self = self_in;
    img_show(self);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(m5ui_img_show_obj, m5ui_img_show);

STATIC mp_obj_t m5ui_img_setposition(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    m5ui_img_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,    MP_ARG_INT,     { .u_int = self->x } },
        { MP_QSTR_y,    MP_ARG_INT,     { .u_int = self->y } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    uint16_t x = args[0].u_int;
    uint16_t y = args[1].u_int;
    if(self->x == args[0].u_int && self->y == args[1].u_int) {
        return mp_const_none;
    }
    if((max(self->x, x) > min(self->x, x) + self->width) || (max(self->y, y) > min(self->y, y) + self->hight)) {
        TFT_fillRect(self->x, self->y, self->width, self->hight, ui_bg_color);
    } else {
        TFT_fillRect(self->x > x ? x+self->width : self->x, self->y, abs(self->x - x), self->hight, ui_bg_color);
        TFT_fillRect(self->x, self->y > y ? y + self->hight : self->y, self->width, abs(self->y - y), ui_bg_color);
    }
    self->x = x;
    self->y = y;
    img_show(self);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(m5ui_img_setposition_obj, 1, m5ui_img_setposition);

STATIC mp_obj_t m5ui_img_changeimg(mp_obj_t self_in, mp_obj_t name_in){
    m5ui_img_obj_t *self = self_in;
    const char *fname = mp_obj_str_get_str(name_in);
    uint16_t old_width = self->width;
    uint16_t old_hight = self->hight;

    uint8_t img_type = 0;
    char fullname[128] = {'\0'};
    if(strlen(fname) > 19) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "name too len"));
    }
    int res = physicalPath(fname, fullname);
    if ((res != 0) || (strlen(fullname) == 0)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Error resolving file name"));
    }

    char upr_fname[128];
    strcpy(upr_fname, fname);
    for (int i=0; i < strlen(upr_fname); i++) {
        upr_fname[i] = toupper((unsigned char) upr_fname[i]);
    }
    if (strstr(upr_fname, ".JPG") != NULL) img_type = IMAGE_TYPE_JPG;
    else if (strstr(upr_fname, ".BMP") != NULL) img_type = IMAGE_TYPE_BMP;
    self->type = img_type;
    sprintf(self->text, "%s", fname);
    img_show(self);
    if(self->width < old_width){
        TFT_fillRect(self->x+self->width, self->y, old_width - self->width, old_hight, ui_bg_color);
    } 
    if(self->hight < old_hight){
        TFT_fillRect(self->x, self->y+self->hight, old_hight, old_hight - self->hight, ui_bg_color);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(m5ui_img_changeimg_obj, m5ui_img_changeimg);


//===============================================================
STATIC const mp_rom_map_elem_t m5ui_img_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_hide),            (mp_obj_t)&m5ui_img_hide_obj},
    { MP_ROM_QSTR(MP_QSTR_show),            (mp_obj_t)&m5ui_img_show_obj},
    { MP_ROM_QSTR(MP_QSTR_setPosition),     (mp_obj_t)&m5ui_img_setposition_obj},
    { MP_ROM_QSTR(MP_QSTR_changeImg),       (mp_obj_t)&m5ui_img_changeimg_obj},

};

STATIC MP_DEFINE_CONST_DICT(m5ui_img_locals_dict, m5ui_img_locals_dict_table);

//====================================
const mp_obj_type_t m5ui_img_type = {
    { &mp_type_type },
    .name = MP_QSTR_M5Img,
    .make_new = m5ui_img_make_new,
    .print = m5ui_img_print,
    .locals_dict = (mp_obj_dict_t*)&m5ui_img_locals_dict,
};