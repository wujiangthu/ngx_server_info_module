/*
 *  Wu Jiang <wujiangthu@gmail.com>
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
  ngx_flag_t enable;
} ngx_http_server_info_loc_conf_t;


static ngx_int_t ngx_http_server_info_init(ngx_conf_t *cf);
static void *ngx_http_server_info_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_server_info_merge_loc_conf(ngx_conf_t *cf);

static ngx_int_t ngx_http_server_info_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_server_info_generate_statistic_file();


static ngx_command_t ngx_http_server_info_commands[] = {

  { ngx_string("server_info"),
    NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
    ngx_conf_set_flag_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_server_info_loc_conf_t, enable),
    NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_server_info_module_ctx = {
  NULL,              /* preconfiguration */
  ngx_http_server_info_init,  /* postconfiguration */

  NULL,              /* create main configuration */
  NULL,              /* init main configuration */

  NULL,              /* create server configuration */
  NULL,              /* merge server configuration */

  ngx_http_server_info_create_loc_conf,    /* create location configuration */
  ngx_http_server_info_merge_loc_conf      /* merge location configuration  */
};

ngx_module_t ngx_http_server_info_module = {
  NGX_MODULE_V1,
  &ngx_http_server_info_module_ctx,     /* module context */
  ngx_http_server_info_commands,        /* module directives */
  NGX_HTTP_MODULE,                      /* module type */
  NULL,                                 /* init master */
  NULL,                                 /* init module */
  NULL,                                 /* init process */
  NULL,                                 /* init thread */
  NULL,                                 /* exit thread  */
  NULL,                                 /* exit process */
  NULL,                                 /* exit master */
  NGX_MODULE_V1_PADDING
};


