#!/bin/bash
if [ -z $1 ]
then
  echo 'lack of module path.'
  exit
fi

echo '#include "stdio.h"' > $PWD/ngx_modules_name.h
echo "#include <ngx_config.h>" >> $PWD/ngx_modules_name.h
echo "#include <ngx_core.h>" >> $PWD/ngx_modules_name.h
echo "#include <ngx_http.h>" >> $PWD/ngx_modules_name.h
echo "#include <ngx_conf_file.h>" >> $PWD/ngx_modules_name.h
echo "#define null_NGX_STR {0,0}" >> $PWD/ngx_modules_name.h

sed -e '/ngx_module_t \*ngx_modules\[\]/,$ !d ' \
    -e '/ngx_module_t \*ngx_modules\[\]/,$ s/ngx_module_t \*ngx_modules\[\]/static ngx_str_t ngx_modules_name\[\]/' \
    -e 's/&/ngx_string("/' \
    -e 's/,/"),/'  \
    -e 's/NULL/null_NGX_STR/'  \
    <$1/objs/ngx_modules.c  \
    >>"$PWD/ngx_modules_name.h"
