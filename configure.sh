#!/bin/sh
if [ -n $1 ]
then
  echo 'lack of module path.'
    exit
    fi

    sed -e '/ngx_module_t \*ngx_modules\[\]/,$ !d ' \
        -e '/ngx_module_t \*ngx_modules\[\]/,$ s/ngx_module_t \*ngx_modules\[\]/static ngx_string ngx_modules_name\[\]/' \
	-e 's/&/ngx_string("/' \
	-e 's/,/"),/'  \
	<$1/objs/ngx_modules.c  \
	>"$PWD/ngx_modules_name.h"
