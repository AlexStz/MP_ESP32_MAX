/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "sdkconfig.h"
//#ifdef CONFIG_MICROPY_USE_NVS

#include "py/mpconfig.h"

#include <string.h>
#include <stdlib.h>

#include "py/runtime.h"
#include "py/smallint.h"
#include "py/mphal.h"

#include <nvs_flash.h>

typedef struct _mp_obj_nvs_t {
    mp_obj_base_t base;
    nvs_open_mode open_mode;
    nvs_handle handle;
    mp_obj_t namespace;
} mp_obj_nvs_t;

STATIC mp_obj_t nvs_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, true);
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_namespace,       MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_mode,            MP_ARG_INT, {.u_int = NVS_READWRITE }},
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    mp_obj_t namespace = parsed_args[0].u_obj;
    mp_int_t mode = parsed_args[1].u_int;
    const char* namespace_str = mp_obj_str_get_str(namespace);

    nvs_handle handle;
    esp_err_t err = nvs_open(namespace_str, (nvs_open_mode)mode, &handle);
    if( err != ESP_OK ) {
        if( err == ESP_ERR_NVS_INVALID_NAME ) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Failed to open NVS - invalid namespace"));
        }
        else {
            char message[80];
            sprintf(message, "Failed to open NVS - %d", err);
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, message));
        }
    }

    mp_obj_nvs_t* o = m_new_obj(mp_obj_nvs_t);
    o->base.type = type;
    o->handle = handle;
    o->namespace = namespace;
    o->open_mode = mode;
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t nvs_m_close(mp_obj_t self_in) {
    mp_obj_nvs_t* self = MP_OBJ_TO_PTR(self_in);
    nvs_close(self->handle);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(nvs_close_obj, nvs_m_close);

////
// get methods
//
typedef esp_err_t (*nvs_get_func)(nvs_handle, const char*, void*);
STATIC bool nvs_get_integral_m(mp_obj_t self_in, mp_obj_t key, void* value, nvs_get_func get_func) {
    mp_obj_nvs_t* self = (mp_obj_nvs_t*)self_in;
    const char* key_str = mp_obj_str_get_str(key);
    esp_err_t err = get_func(self->handle, key_str, &value);
    if( err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND ) {
        char message[80];
        sprintf(message, "Failed to get NVS value - %d", err);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, message));
    }
    return err == ESP_OK;
}

STATIC mp_obj_t nvs_m_get_u8(mp_obj_t self_in, mp_obj_t key) {
    uint8_t value;
    bool result = nvs_get_integral_m(self_in, key, &value, (nvs_get_func)&nvs_get_u8);
    return result ? mp_obj_new_int(value) : mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(nvs_get_u8_obj, nvs_m_get_u8);

STATIC mp_obj_t nvs_m_get_u16(mp_obj_t self_in, mp_obj_t key) {
    uint16_t value;
    bool result = nvs_get_integral_m(self_in, key, &value, (nvs_get_func)&nvs_get_u16);
    return result ? mp_obj_new_int(value) : mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(nvs_get_u16_obj, nvs_m_get_u16);

STATIC mp_obj_t nvs_m_get_u32(mp_obj_t self_in, mp_obj_t key) {
    uint32_t value;
    bool result = nvs_get_integral_m(self_in, key, &value, (nvs_get_func)&nvs_get_u32);
    return result ? mp_obj_new_int(value) : mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(nvs_get_u32_obj, nvs_m_get_u32);

STATIC mp_obj_t nvs_m_get_u64(mp_obj_t self_in, mp_obj_t key) {
    uint64_t value;
    bool result = nvs_get_integral_m(self_in, key, &value, (nvs_get_func)&nvs_get_u64);
    return result ? mp_obj_new_int_from_uint(value) : mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(nvs_get_u64_obj, nvs_m_get_u64);

STATIC mp_obj_t nvs_m_get_str(mp_obj_t self_in, mp_obj_t key) {
    mp_obj_nvs_t* self = (mp_obj_nvs_t*)self_in;
    const char* key_str = mp_obj_str_get_str(key);
    size_t required_size;

    esp_err_t err = nvs_get_str(self->handle, key_str, NULL, &required_size);
    if( err == ESP_ERR_NVS_NOT_FOUND ) {
        return mp_const_none;
    }
    if( err != ESP_OK ) {
        char message[80];
        sprintf(message, "Failed to get NVS value - %d", err);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, message));
    }
    char* buffer = malloc(required_size);
    if( buffer == NULL ) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "Failed to allocate buffer."));
    }
    err = nvs_get_str(self->handle, key_str, buffer, &required_size);
    if( err != ESP_OK ) {
        free(buffer);
        char message[80];
        sprintf(message, "Failed to get NVS value - %d", err);
        nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, message));
    }

    mp_obj_t result = mp_obj_new_str(buffer, required_size-1);
    free(buffer);
    return result;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(nvs_get_str_obj, nvs_m_get_str);

STATIC const mp_rom_map_elem_t nvs_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_u8 ), MP_ROM_PTR(&nvs_get_u8_obj ) },
    { MP_ROM_QSTR(MP_QSTR_get_u16), MP_ROM_PTR(&nvs_get_u16_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_u32), MP_ROM_PTR(&nvs_get_u32_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_u64), MP_ROM_PTR(&nvs_get_u64_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_str), MP_ROM_PTR(&nvs_get_str_obj) },
    { MP_ROM_QSTR(MP_QSTR_close  ), MP_ROM_PTR(&nvs_close_obj  ) },
};
STATIC MP_DEFINE_CONST_DICT(nvs_locals_dict, nvs_locals_dict_table);


STATIC void nvs_attr(mp_obj_t self_in, qstr attr, mp_obj_t* dest)
{
    mp_obj_nvs_t* self = MP_OBJ_TO_PTR(self_in);
    bool is_load = dest[0] == MP_OBJ_NULL;

    switch(attr)
    {
        // methods
        case MP_QSTR_get_u8 : if( is_load ) { dest[0] = (mp_obj_t)&nvs_get_u8_obj;  dest[1] = self_in; } break;
        case MP_QSTR_get_u16: if( is_load ) { dest[0] = (mp_obj_t)&nvs_get_u16_obj; dest[1] = self_in; } break;
        case MP_QSTR_get_u32: if( is_load ) { dest[0] = (mp_obj_t)&nvs_get_u32_obj; dest[1] = self_in; } break;
        case MP_QSTR_get_u64: if( is_load ) { dest[0] = (mp_obj_t)&nvs_get_u64_obj; dest[1] = self_in; } break;
        case MP_QSTR_get_str: if( is_load ) { dest[0] = (mp_obj_t)&nvs_get_str_obj; dest[1] = self_in; } break;
        case MP_QSTR_close:   if( is_load ) { dest[0] = (mp_obj_t)&nvs_close_obj;   dest[1] = self_in; } break;

        case MP_QSTR_mode: { // obj.mode
            if( is_load ) {
                dest[0] = mp_obj_new_int(self->open_mode);
            }
            break;
        }
    }
}

STATIC const mp_obj_type_t nvs_type = {
    { &mp_type_type },
    .name = MP_QSTR_nvs,
    .make_new    = nvs_make_new,
    .attr        = nvs_attr,
    .locals_dict = (void*)&nvs_locals_dict,
};

STATIC const mp_rom_map_elem_t mp_module_nvs_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_nvs) },
    { MP_ROM_QSTR(MP_QSTR_NVS), MP_ROM_PTR(&nvs_type) },
};
STATIC MP_DEFINE_CONST_DICT(mp_module_nvs_globals, mp_module_nvs_globals_table);

const mp_obj_module_t mp_module_nvs = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_nvs_globals,
};

//#endif // CONFIG_MICROPY_USE_NVS