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
 * File:   hex.h
 * Author: alex
 *
 * Created on January 26, 2021, 10:04 PM
 */

#ifndef JSON_HANDLER_HEX_H
#define JSON_HANDLER_HEX_H

static const char* hex_symbols = "0123456789abcdef";

static char* hex_encode(const unsigned char* plain, size_t plain_len) {
    char* hex = malloc(plain_len * 2 + 1);
    memset(hex, '\0', plain_len * 2 + 1);
    for (size_t i = 0; i < plain_len; i++) {
        unsigned char uch = plain[i];
        hex[i*2] = hex_symbols[(size_t)(uch >> 4)];
        hex[(i*2) + 1] = hex_symbols[(size_t)(uch & 0x0f)];
    }
    return hex;
}

#endif /* JSON_HANDLER_HEX_H */

