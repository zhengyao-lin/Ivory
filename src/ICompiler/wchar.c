#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "DBG.h"
#include "Ivoryc.h"

wchar_t *
Ivyc_mbstowcs_alloc(int line_number, const char *src)
{
    int len;
    wchar_t *ret;

    len = ISandBox_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
        Ivyc_compile_error(line_number,
                          BAD_MULTIBYTE_CHARACTER_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    ISandBox_mbstowcs(src, ret);

    return ret;
}
