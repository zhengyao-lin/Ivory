#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <limits.h>
#include "DBG.h"
#include "MEM.h"
#include "ISandBox.h"

size_t
ISandBox_wcslen(wchar_t *str)
{
    return wcslen(str);
}

wchar_t *
ISandBox_wcscpy(wchar_t *dest, wchar_t *src)
{
    return wcscpy(dest, src);
}

wchar_t *
ISandBox_wcsncpy(wchar_t *dest, wchar_t *src, size_t n)
{
    return wcsncpy(dest, src, n);
}

int
ISandBox_wcscmp(wchar_t *s1, wchar_t *s2)
{
    return wcscmp(s1, s2);
}

wchar_t *
ISandBox_wcscat(wchar_t *s1, wchar_t *s2)
{
    return wcscat(s1, s2);
}

int
ISandBox_mbstowcs_len(const char *src)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(NULL, &src[src_idx], MB_LEN_MAX, &ps);
        if (status < 0) {
            return status;
        }
        dest_idx++;
        src_idx += status;
    }

    return dest_idx;
}

void
ISandBox_mbstowcs(const char *src, wchar_t *dest)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = mbrtowc(&dest[dest_idx], &src[src_idx],
                         MB_LEN_MAX, &ps);
        dest_idx++;
        src_idx += status;
    }
    dest[dest_idx] = L'\0';
}

int
ISandBox_wcstombs_len(const wchar_t *src)
{
    int src_idx, dest_idx;
    int status;
    char dummy[MB_LEN_MAX];
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != L'\0'; ) {
        status = wcrtomb(dummy, src[src_idx], &ps);
        src_idx++;
        dest_idx += status;
    }

    return dest_idx;
}

void
ISandBox_wcstombs_i(const wchar_t *src, char *dest)
{
    int src_idx, dest_idx;
    int status;
    mbstate_t ps;

    memset(&ps, 0, sizeof(mbstate_t));
    for (src_idx = dest_idx = 0; src[src_idx] != '\0'; ) {
        status = wcrtomb(&dest[dest_idx], src[src_idx], &ps);
        src_idx++;
        dest_idx += status;
    }
    dest[dest_idx] = '\0';
}

char *
ISandBox_wcstombs_alloc(const wchar_t *src)
{
    int len;
    char *ret;

    len = ISandBox_wcstombs_len(src);
    ret = MEM_malloc(len + 1);
    ISandBox_wcstombs_i(src, ret);

    return ret;
}

char
ISandBox_wctochar(wchar_t src)
{
    mbstate_t ps;
    int status;
    char dest;

    memset(&ps, 0, sizeof(mbstate_t));
    status = wcrtomb(&dest, src, &ps);
    DBG_assert(status == 1, ("wcrtomb status..%d\n", status));

    return dest;
}

int
ISandBox_print_wcs(FILE *fp, wchar_t *str)
{
    char *tmp;
    int result;

    tmp = ISandBox_wcstombs_alloc(str);
    result = fprintf(fp, "%s", tmp);
    MEM_free(tmp);

    return result;
}


int
ISandBox_print_wcs_ln(FILE *fp, wchar_t *str)
{
    int result;

    result = ISandBox_print_wcs(fp, str);
    fprintf(fp, "\n");

    return result;
}

ISandBox_Boolean
ISandBox_iswdigit(wchar_t ch)
{
    return ch >= L'0' && ch <= L'9';
}
