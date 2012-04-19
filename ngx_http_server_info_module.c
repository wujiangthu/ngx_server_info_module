/*
 * Copyright(c) Wu Jiang <wujiangthu@gmail.com>
 */

#include <ngx_config.h>
#include <ngx_conf_file.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_modules_name.h"


typedef struct {
  ngx_flag_t enable;
  ngx_str_t  path;
} ngx_http_server_info_loc_conf_t;

static ngx_str_t default_path = ngx_string("statistics");

static ngx_int_t ngx_http_server_info_init(ngx_conf_t *cf);
static void *ngx_http_server_info_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_server_info_merge_loc_conf(ngx_conf_t *cf,
  void *parent, void *child);

static char *ngx_http_server_info(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_server_info_handler(ngx_http_request_t *r);
static char *ngx_http_server_info_generate_statistic_file(ngx_str_t *output_path);
static ngx_int_t get_commands_num(ngx_module_t *commands);

static ngx_command_t ngx_http_server_info_commands[] = {

  { ngx_string("modules_info"),
    NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
    ngx_http_server_info,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_server_info_loc_conf_t, enable),
    NULL },

  { ngx_string("info_path"),
    NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_server_info_loc_conf_t, path),
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

  ngx_http_server_info_create_loc_conf,     /* create location configuration  */
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


static char * 
ngx_http_server_info_generate_statistic_file(ngx_str_t *output_path)
{
  FILE *fp;
  fp = fopen((char *)output_path->data, "wt+");

  if (!fp) {
     printf("failed on file"); 
     return NGX_CONF_ERROR;
  }

  int dcount         = 0;
  int directives_num = 0;
  int mcount;
  for (mcount = 0 ; ngx_modules[mcount] ; mcount++) {
      fprintf(fp, "%s:\n", ngx_modules_name[mcount].data);

      dcount         = 0;
      directives_num = get_commands_num(ngx_modules[mcount]);
      for (; dcount < directives_num; dcount++) {
          fprintf(fp, "     %s:\n", 
                  ngx_modules[mcount]->commands[dcount].name.data);
      }

      fprintf(fp, "\n");
  }

  fclose(fp);
  return NGX_CONF_OK;
}

static ngx_int_t
get_commands_num(ngx_module_t *module)
{
  ngx_module_t *m = module;
  int num         = 0;
  while (m->commands && m->commands[num].name.data) {
        num++;
  } 

  return num;
}

static ngx_int_t 
ngx_http_server_info_handler(ngx_http_request_t *r)
{
  ngx_int_t   rc;
  ngx_buf_t   *b;
  ngx_chain_t out;
  ngx_str_t   final_path;
  ngx_http_server_info_loc_conf_t *silcf;

  silcf = ngx_http_get_module_loc_conf(r, ngx_http_server_info_module);
  
  if (!silcf->enable) {
     return NGX_OK;
  }

  if (0 == silcf->path.len) {
     final_path = default_path; 
  } else {
     final_path = default_path; 
    // final_path = silcf->path;
  }

  FILE *fp;
  fp = fopen((char *)final_path.data, "r");
  fseek(fp, 0L, SEEK_END);
  int file_len =  ftell(fp);
  rewind(fp);

  u_char *content = ngx_pcalloc(r->pool, file_len);
  u_char *content_temp =content;
  
  while (!feof(fp)) {
        if (fgets((char *)content_temp,file_len,fp)!=NULL) { 
           content_temp = content_temp+strlen((const char *)content_temp);
       }
          
  } 
  fclose(fp);

  if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
     return NGX_HTTP_NOT_ALLOWED;
  }
  
  rc = ngx_http_discard_request_body(r);  
 
  if (NGX_OK != rc) {
     return rc; 
  }

  r->headers_out.content_type.data = (u_char *) "text/html";
  r->headers_out.content_type.len = sizeof("text/html") - 1;

  if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = file_len;
 
        return ngx_http_send_header(r);
   }

  b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
  if (!b) {
     return NGX_HTTP_INTERNAL_SERVER_ERROR; 
  }
 
  out.buf = b;
  out.next = NULL;

  b->pos = content;
  b->last = content + file_len;

  b->memory = 1;   
  b->last_buf = 1;
 
  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = file_len;

  rc = ngx_http_send_header(r);

  if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
     return rc;
  }

  return ngx_http_output_filter(r, &out);
}

static char *
ngx_http_server_info(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_conf_set_flag_slot(cf, cmd, conf);

    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_server_info_handler; 

    return NGX_CONF_OK;
}

static void *
ngx_http_server_info_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_server_info_loc_conf_t *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_server_info_loc_conf_t));
  if (!conf) {
     return NULL; 
  }

  /*
   * set by ngx_pcalloc():
   *
   *     conf->path = { 0, NULL };
   */

  conf->enable = NGX_CONF_UNSET;

  return conf;
}

static char *
ngx_http_server_info_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_server_info_loc_conf_t *prev = parent;
  ngx_http_server_info_loc_conf_t *conf = child;

  ngx_conf_merge_value(conf->enable, prev->enable, 0);
  ngx_conf_merge_str_value(conf->path, prev->path, 0);

  return NGX_CONF_OK;
}


static ngx_int_t 
ngx_http_server_info_init(ngx_conf_t *cf)
{
  ngx_http_server_info_generate_statistic_file(&default_path);

  return NGX_OK;
}


