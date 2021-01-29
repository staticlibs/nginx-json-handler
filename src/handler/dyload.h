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
 * File:   dyload.h
 * Author: alex
 *
 * Created on January 26, 2021, 11:54 AM
 */

#ifndef JSON_HANDLER_DYLOAD_H
#define JSON_HANDLER_DYLOAD_H

#ifdef _WIN32

#include <windows.h>

#define dlsym GetProcAddress

static void* dyload_platform(const char* libname) {
    char buf[1024];

    int len_libname = snprintf(buf, sizeof(buf), "%s.dll", libname);
    if (len_libname < 0 || len_libname >= sizeof(buf)) {
        return NULL;
    }

    // note: LoadLibraryW should not be necessary because only name is used
    HANDLE lib = LoadLibraryA(buf);
    if (NULL == lib) {
        return NULL;
    }

    return lib;
}

#else // !_WIN32

#include <dlfcn.h>

static void* dyload_platform(const char* libname) {
    char buf[1024];

    int len_libname = snprintf(buf, sizeof(buf), "lib%s.so", libname);
    if (len_libname < 0 || len_libname >= (int) sizeof(buf)) {
        return NULL;
    }

    void* lib = dlopen(buf, RTLD_LAZY);
    if (NULL == lib) {
        return NULL;
    }

    return lib;
}

#endif // _WIN32

static void* dyload_library(const char* libname) {
    if (NULL == libname) {
        return NULL;
    }
    return dyload_platform(libname);
}

static void* dyload_symbol(void* lib, const char* symbol) {
    if (NULL == lib) {
        return NULL;
    }
    if (NULL == symbol) {
        return NULL;
    }
    return dlsym(lib, symbol);
}

#endif /* JSON_HANDLER_DYLOAD_H */