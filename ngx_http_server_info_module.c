/*
 *  Wu Jiang <wujiangthu@gmail.com>
 */

//#include "/home/usher/nginx/objs/ngx_modules.c"
#include <stdio.h>
#include <ngx_config.h>
#include <ngx_conf_file.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_modules_name.h"

//extern ngx_module_t *ngx_modules[];
typedef struct {
  ngx_flag_t enable;
} ngx_http_server_info_loc_conf_t;


static ngx_int_t ngx_http_server_info_init(ngx_conf_t *cf);
static void *ngx_http_server_info_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_server_info_merge_loc_conf(ngx_conf_t *cf,
  void *parent, void *child);

static ngx_int_t ngx_http_server_info_handler(ngx_http_request_t *r);
static char *ngx_http_server_info_generate_statistic_file(ngx_str_t *output_path);
static ngx_int_t get_commands_num(ngx_command_t *commands);

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


static ngx_str_t path = ngx_string("statistics");

static char * 
ngx_http_server_info_generate_statistic_file(ngx_str_t *output_path)
{
  FILE *fp;
  fp = fopen((char *)output_path->data, "wt+");
  if(!fp){
    printf("failed on file"); 
    return NGX_CONF_ERROR;
  }

 // int modules_num = sizeof(ngx_modules) / sizeof(void *) - 1;
  int dcount = 0;
  int directives_num = 0;
  int mcount;
  for(mcount = 0 ; ngx_modules[mcount] ; mcount++){
    fprintf(fp, "%s:\n", ngx_modules_name[mcount].data);
    dcount = 0;
    //directives_num = sizeof*(ngx_modules[mcount]->commands) / sizeof(ngx_command_t) - 1;
    directives_num = get_commands_num(ngx_modules[mcount]->commands);
    for(; dcount < directives_num; dcount++)
      fprintf(fp, "%s:\n", ngx_modules[mcount]->commands[dcount].name.data);
  }
  fclose(fp);
  return NGX_CONF_OK;
}

static ngx_int_t
get_commands_num(ngx_command_t *commands){
  ngx_command_t *cmd = commands;
  int num = 0;
  while(NULL != cmd->name.data){
    num++;
    cmd = cmd + sizeof(ngx_command_t);
  } 

return num;
}

static ngx_int_t 
ngx_http_server_info_handler(ngx_http_request_t *r){
  return NGX_OK;
}

static void *
ngx_http_server_info_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_server_info_loc_conf_t *conf;

  conf = ngx_palloc(cf->pool, sizeof(ngx_http_server_info_loc_conf_t));
  if(NULL == conf){
    return NULL; 
  }

  conf->enable = NGX_CONF_UNSET;

  return conf;
}

static char *
ngx_http_server_info_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_server_info_loc_conf_t *prev = parent;
  ngx_http_server_info_loc_conf_t *conf = child;

  ngx_conf_merge_value(conf->enable, prev->enable, 0);

  return NGX_CONF_OK;
}

static ngx_int_t 
ngx_http_server_info_init(ngx_conf_t *cf)
{
  ngx_http_core_loc_conf_t *clcf;

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

  clcf->handler = ngx_http_server_info_handler;

  ngx_http_server_info_generate_statistic_file(&path);

  return NGX_OK;
}
