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
 * File:   json_handler.h
 * Author: alex
 *
 * Created on January 28, 2021, 7:51 PM
 */

#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

int handle_json_request(const char* req_json);

#ifdef __cplusplus
}
#endif

#endif /* JSON_HANDLER_H */

