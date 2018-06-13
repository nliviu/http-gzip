/*
 * Copyright 2018 Liviu Nicolescu <nliviu@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <mgos.h>
#include "mgos_http_server.h"

static struct mg_serve_http_opts s_http_server_opts;

static void handle_gzip(struct mg_connection *nc, struct http_message* msg,
                        struct mg_serve_http_opts* opts) {
  struct mg_str uri = mg_mk_str_n(msg->uri.p, msg->uri.len);
  bool gzip = strncmp(uri.p + uri.len - 3, ".gz", 3) == 0;
  if (gzip) {
    opts->extra_headers = "Content-Encoding: gzip";
  }
}

static void http_msg_print(const struct http_message* msg) {
  LOG(LL_INFO, ("     message: \"%.*s\"\n", msg->message.len, msg->message.p));
  LOG(LL_INFO, ("      method: \"%.*s\"\n", msg->method.len, msg->method.p));
  LOG(LL_INFO, ("         uri: \"%.*s\"\n", msg->uri.len, msg->uri.p));
}

static void root_handler(struct mg_connection *nc, int ev, void *p, void *user_data) {
  (void) user_data;
  if (ev != MG_EV_HTTP_REQUEST) {
    return;
  }
  LOG(LL_INFO, ("root_handler"));
  struct http_message* msg = (struct http_message*) (p);
  http_msg_print(msg);

  struct mg_serve_http_opts opts;
  memcpy(&opts, &s_http_server_opts, sizeof (opts));
  handle_gzip(nc, msg, &opts);
  mg_serve_http(nc, msg, opts);
}

enum mgos_app_init_result mgos_app_init(void) {
  memset(&s_http_server_opts, 0, sizeof (s_http_server_opts));
  s_http_server_opts.document_root = mgos_sys_config_get_http_document_root();
  /*
   * add mime types for css.gz and js.gz
   */
  s_http_server_opts.custom_mime_types =
    ".js.gz=application/javascript; charset=utf-8,.css.gz=text/css; charset=utf-8";
  /*
   * more options can be filled in
   */

  mgos_register_http_endpoint("/", root_handler, NULL);

  return MGOS_APP_INIT_SUCCESS;
}
