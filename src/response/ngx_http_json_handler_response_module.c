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

static u_char ngx_wilton[] = "gateway async resp 3\n";

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

        if (0 == strncmp("x-nginx-request-handle", (const char*) elts[i].lowcase_key, elts[i].key.len)) {
            if (elts[i].value.len >= 32) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Invalid handle received");
                return NULL;
            }
            char cstr[32];
            memset(cstr, '\0', sizeof(cstr));
            memcpy(cstr, (const char*) elts[i].value.data, elts[i].value.len);
            char* endptr;
            errno = 0;
            long long handle = strtoll(cstr, &endptr, 0);
            if (errno == ERANGE || cstr + elts[i].value.len != endptr) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                        "Cannot parse handle from string, value: [%s]\n", cstr);
                return NULL;
            }
            return (ngx_http_request_t*) handle;
        }
    }
    return NULL;
}

static ngx_int_t send_client_response(ngx_http_request_t* r, ngx_http_request_t* gr) {

    if (r->connection->error) {
        ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0, "Request already finalized %d", r->count);
        ngx_http_finalize_request(r, NGX_ERROR);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    /* Set the Content-Type header. */
    //gr->headers_out.content_type.len = r->headers_in.content_type->value.len;
    //gr->headers_out.content_type.data = r->headers_in.content_type->value.data;

    // todo: body
    ngx_http_discard_request_body(gr);

    /* Allocate a new buffer for sending out the reply. */
    ngx_buf_t* b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    ngx_chain_t out;
    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = ngx_wilton; /* first position in memory of the data */
    b->last = ngx_wilton + sizeof(ngx_wilton) - 1; /* last position in memory of the data */
    b->memory = 1; /* content is in read-only memory */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = sizeof(ngx_wilton) - 1;
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    ngx_http_output_filter(r, &out);
    ngx_http_finalize_request(r, NGX_HTTP_OK);
    ngx_http_finalize_request(r, NGX_HTTP_OK);
    ngx_http_run_posted_requests(r->connection);

    return NGX_HTTP_OK;
}

static void body_handler(ngx_http_request_t* r) {

    if (r->request_body == NULL) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    // todo

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "incoming response");

    ngx_int_t status = NGX_HTTP_OK;

    // client response
    ngx_http_request_t* cr = find_request_handle(r);
    if (NULL != cr) {
        status = send_client_response(cr, r);
    } else {
        status = NGX_HTTP_BAD_REQUEST;
    }

    // own response

    /* Allocate a new buffer for sending out the reply. */
    ngx_buf_t* b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    ngx_chain_t out;
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = 0; /* first position in memory of the data */
    b->last = 0; /* last position in memory of the data */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = status; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = 0;
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    ngx_http_output_filter(r, &out);

    ngx_http_finalize_request(r, NGX_HTTP_OK);
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