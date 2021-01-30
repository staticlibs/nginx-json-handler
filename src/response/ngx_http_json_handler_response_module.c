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

#define RESPONSE_HEADER_PREFIX "x-response-"

static ngx_int_t send_buffer(ngx_http_request_t* r, ngx_buf_t* buf) {
    // send headers
    ngx_int_t err_headers = ngx_http_send_header(r);
    if (NGX_OK != err_headers) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Error sending headers");
        ngx_http_finalize_request(r, NGX_ERROR);
        return err_headers;
    }

    // send data
    ngx_chain_t chain;
    chain.buf = buf;
    chain.next = NULL;
    ngx_int_t err_filters = ngx_http_output_filter(r, &chain);
    if (NGX_OK != err_filters) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Error sending data");
        ngx_http_finalize_request(r, NGX_ERROR);
        return err_filters;
    }

    // release request
    ngx_http_finalize_request(r, NGX_HTTP_OK);

    return NGX_OK;
}

static ngx_http_request_t* find_request_handle(ngx_http_request_t* r) {
    ngx_list_part_t* part = &r->headers_in.headers.part;
    ngx_table_elt_t* elts = part->elts;

    for (size_t i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            elts = part->elts;
            i = 0;
        }

        ngx_table_elt_t* h = &elts[i];

        if (0 == strncmp("x-nginx-request-handle", (const char*) h->lowcase_key, h->key.len)) {
            if (h->value.len >= 32) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Invalid handle received");
                return NULL;
            }
            char cstr[32];
            memset(cstr, '\0', sizeof(cstr));
            memcpy(cstr, (const char*) h->value.data, h->value.len);
            char* endptr;
            errno = 0;
            long long handle = strtoll(cstr, &endptr, 0);
            if (errno == ERANGE || cstr + h->value.len != endptr) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "Cannot parse handle from string, value: [%s]", cstr);
                return NULL;
            }
            return (ngx_http_request_t*) handle;
        }
    }
    return NULL;
}

static ngx_int_t copy_single_header(ngx_http_request_t* r, ngx_table_elt_t* hin, size_t prefix_len) {

    // copy key
    ngx_str_t key;
    key.len = hin->key.len - prefix_len;
    key.data = ngx_pcalloc(r->pool, key.len);
    if (NULL == key.data) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "Pool allocation error, size: [%l]", key.len);
        return NGX_ERROR;
    }
    memcpy(key.data, hin->key.data + prefix_len, key.len);

    // copy value
    ngx_str_t value;
    value.len = hin->value.len;
    value.data = ngx_pcalloc(r->pool, value.len);
    if (NULL == value.data) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "Pool allocation error, size: [%l]", value.len);
        return NGX_ERROR;
    }
    memcpy(value.data, hin->value.data, value.len);

    // set header
    ngx_table_elt_t* hout = ngx_list_push(&r->headers_out.headers);
    if (hout == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Header allocation error");
        return NGX_ERROR;
    }
    hout->key = key;
    hout->value = value;
    hout->hash = 1;

    return NGX_OK;
}

static ngx_int_t copy_headers(ngx_http_request_t* r, ngx_http_request_t* hr) {
    ngx_list_part_t* part = &hr->headers_in.headers.part;
    ngx_table_elt_t* elts = part->elts;

    for (size_t i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            elts = part->elts;
            i = 0;
        }

        ngx_table_elt_t* hin = &elts[i];

        size_t len = sizeof(RESPONSE_HEADER_PREFIX) - 1;
        if (hin->key.len > len &&
                0 == strncmp(RESPONSE_HEADER_PREFIX, (const char*) hin->lowcase_key, len)) {
            ngx_int_t err_copy = copy_single_header(r, hin, len);
            if (NGX_OK != err_copy) {
                return err_copy;
            }
        }
    }

    return NGX_OK;
}

static ngx_buf_t* copy_body(ngx_http_request_t* r, ngx_http_request_t* hr) {
    ngx_buf_t* buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (NULL == buf) {
        ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0, "Error allocating buffer struct");
        return NULL;
    }
    buf->last_buf = 1;

    if (NULL == hr->request_body->temp_file) {
        ngx_chain_t* in = hr->request_body->bufs;
        if (NULL != in) {
            size_t len = in->buf->last - in->buf->pos;
            buf->pos = ngx_pcalloc(r->pool, len);
            if (NULL == buf->pos) {
                ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0,
                        "Error allocating buffer, size: [%l]", len);
                return NULL;
            }
            memcpy(buf->pos, in->buf->pos, len);
            buf->last = buf->pos + len;
            buf->start = buf->pos;
            buf->end = buf-> last;
            buf->memory = 1;
        } else { // empty body
            // no-op
        }
    } else { // file
        // todo
    }
    return buf;
}

static ngx_int_t send_client_response(ngx_http_request_t* r, ngx_http_request_t* hr) {

    if (r->connection->error) {
        ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0,
                "Request already finalized, counter: [%d]", r->count);
        ngx_http_finalize_request(r, NGX_ERROR);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // headers
    ngx_int_t err_headers = copy_headers(r, hr);
    if (NGX_OK != err_headers) {
        ngx_http_finalize_request(r, NGX_ERROR);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // body
    if (NULL == hr->request_body) {
        ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0, "Request body access error");
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_buf_t* buf = copy_body(r, hr);
    if (NULL == buf) {
        ngx_http_finalize_request(r, NGX_ERROR);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // send
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = buf->last - buf->pos;

    ngx_int_t err_send = send_buffer(r, buf);
    if (NGX_OK == err_send) {
        ngx_http_run_posted_requests(r->connection);
        return NGX_HTTP_OK;
    } else {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
}

static void body_handler(ngx_http_request_t* r) {

    if (r->request_body == NULL) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    ngx_int_t status = NGX_HTTP_OK;

    // client response
    ngx_http_request_t* cr = find_request_handle(r);
    if (NULL != cr) {
        status = send_client_response(cr, r);
    } else {
        status = NGX_HTTP_BAD_REQUEST;
    }

    // own response
    ngx_buf_t* buf = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    buf->pos = 0;
    buf->last = 0;
    buf->last_buf = 1;

    r->headers_out.status = status;
    r->headers_out.content_length_n = 0;

    send_buffer(r, buf);
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

static char* conf_json_handler_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    /* Install the handler. */
    ngx_http_core_loc_conf_t* clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = request_handler;
    return NGX_CONF_OK;
}

static ngx_command_t conf_desc[] = {

    { ngx_string("json_handler_response"), /* directive */
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS, /* location context and arguments count*/
      conf_json_handler_response, /* configuration setup function */
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

ngx_module_t ngx_http_json_handler_response_module = {
    NGX_MODULE_V1,
    &module_ctx, /* module context */
    conf_desc, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};