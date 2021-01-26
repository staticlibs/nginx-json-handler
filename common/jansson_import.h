/*
 * Copyright 2021, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   jansson_import.h
 * Author: alex
 *
 * Created on January 26, 2021, 11:54 AM
 */

#ifndef JSON_HANDLER_JANSSON_IMPORT_H
#define JSON_HANDLER_JANSSON_IMPORT_H

// create

typedef json_t* (*json_null_type)();
static json_null_type json_null_fun = NULL;
json_t* json_null() {
    return json_null_fun();
}

typedef json_t* (*json_true_type)();
static json_true_type json_true_fun = NULL;
json_t* json_true() {
    return json_true_fun();
}

typedef json_t* (*json_false_type)();
static json_false_type json_false_fun = NULL;
json_t* json_false() {
    return json_false_fun();
}

typedef json_t* (*json_integer_type)(json_int_t value);
static json_integer_type json_integer_fun = NULL;
json_t* json_integer(json_int_t value) {
    return json_integer_fun(value);
}

typedef json_t* (*json_real_type)(double value);
static json_real_type json_real_fun = NULL;
json_t* json_real(double value) {
    return json_real_fun(value);
}

typedef json_t* (*json_string_type)(const char *);
static json_string_type json_string_fun = NULL;
json_t* json_string(const char *value) {
    return json_string_fun(value);
}

typedef json_t* (*json_stringn_type)(const char *, size_t);
static json_stringn_type json_stringn_fun = NULL;
json_t* json_stringn(const char *value, size_t len) {
    return json_stringn_fun(value, len);
}

typedef json_t* (*json_array_type)();
static json_array_type json_array_fun = NULL;
json_t* json_array() {
    return json_array_fun();
}

typedef json_t* (*json_object_type)();
static json_object_type json_object_fun = NULL;
json_t* json_object() {
    return json_object_fun();
}

// value

typedef json_int_t (*json_integer_value_type)(const json_t *integer);
static json_integer_value_type json_integer_value_fun = NULL;
json_int_t json_integer_value(const json_t* integer) {
    return json_integer_value_fun(integer);
}

typedef double (*json_real_value_type)(const json_t*);
static json_real_value_type json_real_value_fun = NULL;
double json_real_value(const json_t* real) {
    return json_real_value_fun(real);
}

typedef const char* (*json_string_value_type)(const json_t*);
static json_string_value_type json_string_value_fun = NULL;
const char* json_string_value(const json_t* string) {
    return json_string_value_fun(string);
}

// array

typedef int (*json_array_append_new_type)(json_t*, json_t*);
static json_array_append_new_type json_array_append_new_fun = NULL;
int json_array_append_new(json_t* array, json_t* value) {
    return json_array_append_new_fun(array, value);
}

typedef size_t (*json_array_size_type)(const json_t*);
static json_array_size_type json_array_size_fun = NULL;
size_t json_array_size(const json_t* array) {
    return json_array_size_fun(array);
}

typedef json_t* (*json_array_get_type)(const json_t*, size_t);
static json_array_get_type json_array_get_fun = NULL;
json_t* json_array_get(const json_t* array, size_t index) {
    return json_array_get_fun(array, index);
}

// object

typedef int (*json_object_set_new_type)(json_t*, const char*, json_t*);
static json_object_set_new_type json_object_set_new_fun = NULL;
int json_object_set_new(json_t* object, const char* key, json_t* value) {
    return json_object_set_new_fun(object, key, value);
}

typedef size_t (*json_object_size_type)(const json_t*);
static json_object_size_type json_object_size_fun = NULL;
size_t json_object_size(const json_t* object) {
    return json_object_size_fun(object);
}

typedef json_t* (*json_object_get_type)(const json_t*, const char*);
static json_object_get_type json_object_get_fun = NULL;
json_t* json_object_get(const json_t* object, const char* key) {
    return json_object_get_fun(object, key);
}

typedef void* (*json_object_iter_type)(json_t*);
static json_object_iter_type json_object_iter_fun = NULL;
void* json_object_iter(json_t* object) {
    return json_object_iter_fun(object);
}

typedef void* (*json_object_key_to_iter_type)(const char*);
static json_object_key_to_iter_type json_object_key_to_iter_fun = NULL;
void* json_object_key_to_iter(const char* key) {
    return json_object_key_to_iter_fun(key);
}

typedef void* (*json_object_iter_next_type)(json_t*, void*);
static json_object_iter_next_type json_object_iter_next_fun = NULL;
void* json_object_iter_next(json_t* object, void* iter) {
    return json_object_iter_next_fun(object, iter);
}

typedef const char* (*json_object_iter_key_type)(void*);
static json_object_iter_key_type json_object_iter_key_fun = NULL;
const char* json_object_iter_key(void* iter) {
    return json_object_iter_key_fun(iter);
}

typedef json_t* (*json_object_iter_value_type)(void*);
static json_object_iter_value_type json_object_iter_value_fun = NULL;
json_t* json_object_iter_value(void* iter) {
    return json_object_iter_value_fun(iter);
}

// serialization

typedef json_t* (*json_load_callback_type)(json_load_callback_t, void*, size_t, json_error_t*);
static json_load_callback_type json_load_callback_fun = NULL;
json_t* json_load_callback(json_load_callback_t callback, void* data, size_t flags, json_error_t* error) {
    return json_load_callback_fun(callback, data, flags, error);
}


typedef char* (*json_dumps_type)(const json_t*, size_t);
static json_dumps_type json_dumps_fun = NULL;
char *json_dumps(const json_t* json, size_t flags) {
    return json_dumps_fun(json, flags);
}

typedef int (*json_dump_callback_type)(const json_t*, json_dump_callback_t, void*, size_t);
static json_dump_callback_type json_dump_callback_fun = NULL;
int json_dump_callback(const json_t* json, json_dump_callback_t callback, void* data, size_t flags) {
    return json_dump_callback_fun(json, callback, data, flags);
}

// delete

typedef void (*json_delete_type)(json_t*);
static json_delete_type json_delete_fun = NULL;
void json_delete(json_t* json) {
    json_delete_fun(json);
}

static int jansson_initialize() {
    void* lib = dyload_library("jansson");
    if (NULL == lib) return -1;

    json_null_fun = dyload_symbol(lib, "json_null");
    if (NULL == json_null_fun) return -1;
    json_true_fun = dyload_symbol(lib, "json_true");
    if (NULL == json_true_fun) return -1;
    json_false_fun = dyload_symbol(lib, "json_false");
    if (NULL == json_false_fun) return -1;
    json_integer_fun = dyload_symbol(lib, "json_integer");
    if (NULL == json_integer_fun) return -1;
    json_real_fun = dyload_symbol(lib, "json_real");
    if (NULL == json_real_fun) return -1;
    json_string_fun = dyload_symbol(lib, "json_string");
    if (NULL == json_string_fun) return -1;
    json_stringn_fun = dyload_symbol(lib, "json_stringn");
    if (NULL == json_stringn_fun) return -1;
    json_array_fun = dyload_symbol(lib, "json_array");
    if (NULL == json_array_fun) return -1;
    json_object_fun = dyload_symbol(lib, "json_object");
    if (NULL == json_object_fun) return -1;

    json_integer_value_fun = dyload_symbol(lib, "json_integer_value");
    if (NULL == json_integer_value_fun) return -1;
    json_real_value_fun = dyload_symbol(lib, "json_real_value");
    if (NULL == json_real_value_fun) return -1;
    json_string_value_fun = dyload_symbol(lib, "json_string_value");
    if (NULL == json_string_value_fun) return -1;

    json_array_append_new_fun = dyload_symbol(lib, "json_array_append_new");
    if (NULL == json_array_append_new_fun) return -1;
    json_array_size_fun = dyload_symbol(lib, "json_array_size");
    if (NULL == json_array_size_fun) return -1;
    json_array_get_fun = dyload_symbol(lib, "json_array_get");
    if (NULL == json_array_get_fun) return -1;

    json_object_set_new_fun = dyload_symbol(lib, "json_object_set_new");
    if (NULL == json_object_set_new_fun) return -1;
    json_object_size_fun = dyload_symbol(lib, "json_object_size");
    if (NULL == json_object_size_fun) return -1;
    json_object_get_fun = dyload_symbol(lib, "json_object_get");
    if (NULL == json_object_get_fun) return -1;
    json_object_iter_fun = dyload_symbol(lib, "json_object_iter");
    if (NULL == json_object_iter_fun) return -1;
    json_object_key_to_iter_fun = dyload_symbol(lib, "json_object_key_to_iter");
    if (NULL == json_object_key_to_iter_fun) return -1;
    json_object_iter_next_fun = dyload_symbol(lib, "json_object_iter_next");
    if (NULL == json_object_iter_next_fun) return -1;
    json_object_iter_key_fun = dyload_symbol(lib, "json_object_iter_key");
    if (NULL == json_object_iter_key_fun) return -1;
    json_object_iter_value_fun = dyload_symbol(lib, "json_object_iter_value");
    if (NULL == json_object_iter_value_fun) return -1;

    json_load_callback_fun = dyload_symbol(lib, "json_load_callback");
    if (NULL == json_load_callback_fun) return -1;
    json_dumps_fun = dyload_symbol(lib, "json_dumps");
    if (NULL == json_dumps_fun) return -1;
    json_dump_callback_fun = dyload_symbol(lib, "json_dump_callback");
    if (NULL == json_dump_callback_fun) return -1;

    json_delete_fun = dyload_symbol(lib, "json_delete");
    if (NULL == json_delete_fun) return -1;

    return 0;
}

#endif /* JSON_HANDLER_JANSSON_IMPORT_H */