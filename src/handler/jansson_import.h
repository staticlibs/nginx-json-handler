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

typedef json_t* (*json_integer_type)(json_int_t value);
static json_integer_type json_integer_fun = NULL;
json_t* json_integer(json_int_t value) {
    return json_integer_fun(value);
}

typedef json_t* (*json_stringn_type)(const char *, size_t);
static json_stringn_type json_stringn_fun = NULL;
json_t* json_stringn(const char *value, size_t len) {
    return json_stringn_fun(value, len);
}

typedef json_t* (*json_object_type)();
static json_object_type json_object_fun = NULL;
json_t* json_object() {
    return json_object_fun();
}

// value

typedef const char* (*json_string_value_type)(const json_t*);
static json_string_value_type json_string_value_fun = NULL;
const char* json_string_value(const json_t* string) {
    return json_string_value_fun(string);
}

// object

typedef int (*json_object_set_new_type)(json_t*, const char*, json_t*);
static json_object_set_new_type json_object_set_new_fun = NULL;
int json_object_set_new(json_t* object, const char* key, json_t* value) {
    return json_object_set_new_fun(object, key, value);
}

// serialization

typedef char* (*json_dumps_type)(const json_t*, size_t);
static json_dumps_type json_dumps_fun = NULL;
char *json_dumps(const json_t* json, size_t flags) {
    return json_dumps_fun(json, flags);
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
    json_integer_fun = dyload_symbol(lib, "json_integer");
    if (NULL == json_integer_fun) return -1;
    json_stringn_fun = dyload_symbol(lib, "json_stringn");
    if (NULL == json_stringn_fun) return -1;
    json_object_fun = dyload_symbol(lib, "json_object");
    if (NULL == json_object_fun) return -1;

    json_string_value_fun = dyload_symbol(lib, "json_string_value");
    if (NULL == json_string_value_fun) return -1;

    json_object_set_new_fun = dyload_symbol(lib, "json_object_set_new");
    if (NULL == json_object_set_new_fun) return -1;

    json_dumps_fun = dyload_symbol(lib, "json_dumps");
    if (NULL == json_dumps_fun) return -1;

    json_delete_fun = dyload_symbol(lib, "json_delete");
    if (NULL == json_delete_fun) return -1;

    return 0;
}

#endif /* JSON_HANDLER_JANSSON_IMPORT_H */