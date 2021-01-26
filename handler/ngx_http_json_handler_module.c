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

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "../common/dyload.h"
#include "../common/hex.h"
#include "../common/jansson_import.h"


static ngx_int_t initialize(ngx_cycle_t* cycle) {
    int err = jansson_initialize();
    return 0 == err ? NGX_OK : NGX_ERROR;
}

static json_t* read_headers(ngx_http_headers_in_t* headers_in) {
    ngx_list_part_t* part = &headers_in->headers.part;
    ngx_table_elt_t* elts = part->elts;

    json_t* res = json_object();

    for (size_t i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            elts = part->elts;
            i = 0;
        }

        ngx_str_t key = elts[i].key;
        ngx_str_t value = elts[i].value;

        json_t* key_json = json_stringn((const char*) key.data, key.len);
        if (NULL != key_json) {
            const char* key_st = json_string_value(key_json);
            json_t* value_json = json_stringn((const char*) value.data, value.len);
            if (NULL != value_json) {
                json_object_set_new(res, key_st, value_json);
            }
            json_decref(key_json);
        }
    }

    return res;
}

static void json_set_ngx_string(json_t* obj, const char* key, ngx_str_t str) {
    json_t* jst = json_stringn((const char*) str.data, str.len);
    if (NULL != jst) {
        json_object_set_new(obj, key, jst);
    } else {
        json_t* empty = json_stringn("", 0);
        json_object_set_new(obj, key, empty);
    }
}

static json_t* read_meta(ngx_http_request_t* r) {
    json_t* res = json_object();
    json_set_ngx_string(res, "uri", r->uri);
    json_set_ngx_string(res, "args", r->args);
    json_set_ngx_string(res, "unparsedUri", r->unparsed_uri);
    json_set_ngx_string(res, "method", r->method_name);
    json_set_ngx_string(res, "protocol", r->http_protocol);
    return res;
}

static json_t* read_data(ngx_http_request_t* r) {
    json_t* res = json_object();

    if (NULL == r->request_body->temp_file) {
        json_object_set_new(res, "file", json_null());
        ngx_chain_t* in = r->request_body->bufs;
        if (NULL != in) {
            ngx_buf_t* buf = in->buf;
            size_t plain_len = buf->last - buf->pos;
            char* data_hex = hex_encode(buf->pos, plain_len);
            json_t* data_hex_json = json_stringn(data_hex, plain_len * 2);
            json_object_set_new(res, "hex", data_hex_json);
            free(data_hex);
        } else {
            json_t* empty_json = json_stringn("", 0);
            json_object_set_new(res, "hex", empty_json);
        }
    } else {
        json_object_set_new(res, "hex", json_null());
        ngx_str_t path = r->request_body->temp_file->file.name;
        json_t* path_json = json_stringn((const char*) path.data, path.len);
        if (NULL != path_json) {
            json_object_set_new(res, "file", path_json);
        } else {
            json_t* empty_json = json_stringn("", 0);
            json_object_set_new(res, "file", empty_json);
        }
    }

    return res;
}

static void body_handler(ngx_http_request_t* r) {

    if (r->request_body == NULL) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    json_t* meta = read_meta(r);
    json_t* headers = read_headers(&r->headers_in);
    json_t* data = read_data(r);
    json_t* req = json_object();
    json_object_set_new(req, "meta", meta);
    json_object_set_new(req, "headers", headers);
    json_object_set_new(req, "data", data);

    char* dumped = json_dumps(req, JSON_INDENT(4));
    json_decref(req);
    fprintf(stderr, "%s\n", dumped);
    free(dumped);

    r->main->count++;
}

static ngx_int_t request_handler(ngx_http_request_t *r) {

    // http://mailman.nginx.org/pipermail/nginx/2007-August/001559.html
    r->request_body_in_single_buf = 1;
    r->request_body_in_persistent_file = 1;
    r->request_body_in_clean_file = 1;
    r->request_body_file_log_level = 0;

    ngx_int_t rc = ngx_http_read_client_request_body(r, body_handler);

    if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
        return rc;
    }

    return NGX_DONE;
}

static char* conf_json_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    /* Install the handler. */
    ngx_http_core_loc_conf_t* clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = request_handler;
    return NGX_CONF_OK;
}

static ngx_command_t conf_desc[] = {

    { ngx_string("json_handler"), /* directive */
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and arguments count*/
      conf_json_handler, /* configuration setup function */
      NGX_HTTP_LOC_CONF_OFFSET, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL},

    ngx_null_command /* command termination */
};

static ngx_http_module_t module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};

ngx_module_t ngx_http_json_handler_module = {
    NGX_MODULE_V1,
    &module_ctx, /* module context */
    conf_desc, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    initialize, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};