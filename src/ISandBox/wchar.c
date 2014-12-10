#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "DBG.h"
#include "MEM.h"
#include "ISandBox_pri.h"

wchar_t *
ISandBox_mbstowcs_alloc(ISandBox_VirtualMachine *ISandBox, const char *src)
{
    int len;
    wchar_t *ret;

    len = ISandBox_mbstowcs_len(src);
    if (len < 0) {
        if (ISandBox) {
            ISandBox_error_i(ISandBox->current_executable->executable,
                        ISandBox->current_function,
                        ISandBox->pc,
                        BAD_MULTIBYTE_CHARACTER_ERR,
                        ISandBox_MESSAGE_ARGUMENT_END);
        } else {
            ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                        BAD_MULTIBYTE_CHARACTER_ERR,
                        ISandBox_MESSAGE_ARGUMENT_END);
        }
        return NULL;
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    ISandBox_mbstowcs(src, ret);

    return ret;
}

char *
ISandBox_wcstombs(const wchar_t *src)
{
    char *ret;

    ret = ISandBox_wcstombs_alloc(src);

    return ret;
}

ISandBox_Char *
ISandBox_mbstowcs_s(const char *src)
{
    int len;
    ISandBox_Char *ret;

    len = ISandBox_mbstowcs_len(src);
    if (len < 0) {
        return NULL;
    }
    ret = MEM_malloc(sizeof(wchar_t) * (len+1));
    ISandBox_mbstowcs(src, ret);

    return ret;
}
