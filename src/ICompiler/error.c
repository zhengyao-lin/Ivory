#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "MEM.h"
#include "DBG.h"
#include "Ivoryc.h"

extern char *yytext;
extern ErrorDefinition Ivyc_error_message_format[];

typedef struct {
    MessageArgumentType type;
    char        *name;
    union {
        int          int_val;
        double       double_val;
        long double  long_double_val;
        char         *string_val;
        void         *pointer_val;
        int          character_val;
    } u;
} MessageArgument;

static void
create_message_argument(MessageArgument *arg, va_list ap)
{
    int index = 0;
    MessageArgumentType type;
    
    while ((type = va_arg(ap, MessageArgumentType))
           != MESSAGE_ARGUMENT_END) {
        arg[index].type = type;
        arg[index].name = va_arg(ap, char*);
        switch (type) {
        case INT_MESSAGE_ARGUMENT:
            arg[index].u.int_val = va_arg(ap, int);
            break;
        case DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.double_val = va_arg(ap, double);
            break;
        case LONG_DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.long_double_val = va_arg(ap, long double);
            break;
        case STRING_MESSAGE_ARGUMENT:
            arg[index].u.string_val = va_arg(ap, char*);
            break;
        case POINTER_MESSAGE_ARGUMENT:
            arg[index].u.pointer_val = va_arg(ap, void*);
            break;
        case CHARACTER_MESSAGE_ARGUMENT:
            arg[index].u.character_val = va_arg(ap, int);
            break;
        case MESSAGE_ARGUMENT_END:
            assert(0);
            break;
        default:
            assert(0);
        }
        index++;
        assert(index < MESSAGE_ARGUMENT_MAX);
    }
}

static void
search_argument(MessageArgument *arg_list,
                char *arg_name, MessageArgument *arg)
{
    int i;

    for (i = 0; arg_list[i].type != MESSAGE_ARGUMENT_END; i++) {
        if (!strcmp(arg_list[i].name, arg_name)) {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void
format_message(int line_number, ErrorDefinition *format, VWString *v,
               va_list ap)
{
    int         i;
    char        buf[LINE_BUF_SIZE];
    ISandBox_Char    wc_buf[LINE_BUF_SIZE];
    int         arg_name_index;
    char        arg_name[LINE_BUF_SIZE];
    MessageArgument     arg[MESSAGE_ARGUMENT_MAX];
    MessageArgument     cur_arg;
    ISandBox_Char    *wc_format;

    create_message_argument(arg, ap);

    wc_format = Ivyc_mbstowcs_alloc(line_number, format->format);
    DBG_assert(wc_format != NULL, ("wc_format is null.\n"));
    
    for (i = 0; wc_format[i] != L'\0'; i++) {
        if (wc_format[i] != L'$') {
            Ivyc_vwstr_append_character(v, wc_format[i]);
            continue;
        }
        assert(wc_format[i+1] == L'(');
        i += 2;
        for (arg_name_index = 0; wc_format[i] != L')';
             arg_name_index++, i++) {
            arg_name[arg_name_index] = ISandBox_wctochar(wc_format[i]);
        }
        arg_name[arg_name_index] = '\0';
        assert(wc_format[i] == L')');

        search_argument(arg, arg_name, &cur_arg);
        switch (cur_arg.type) {
        case INT_MESSAGE_ARGUMENT:
            sprintf(buf, "%d", cur_arg.u.int_val);
            ISandBox_mbstowcs(buf, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%f", cur_arg.u.double_val);
            ISandBox_mbstowcs(buf, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case LONG_DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%lf", cur_arg.u.long_double_val);
            ISandBox_mbstowcs(buf, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case STRING_MESSAGE_ARGUMENT:
            ISandBox_mbstowcs(cur_arg.u.string_val, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case POINTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%p", cur_arg.u.pointer_val);
            ISandBox_mbstowcs(buf, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case CHARACTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%c", cur_arg.u.character_val);
            ISandBox_mbstowcs(buf, wc_buf);
            Ivyc_vwstr_append_string(v, wc_buf);
            break;
        case MESSAGE_ARGUMENT_END:
            assert(0);
            break;
        default:
            assert(0);
        }
    }
    MEM_free(wc_format);
}

static void
self_check()
{
    if (strcmp(Ivyc_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("compile error message format error.\n"));
    }
    if (strcmp(Ivyc_error_message_format
               [COMPILE_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) {
        DBG_panic(("compile error message format error. "
                   "COMPILE_ERROR_COUNT_PLUS_1..%d\n",
                   COMPILE_ERROR_COUNT_PLUS_1));
    }
}

void
Ivyc_compile_error(int line_number, CompileError id, ...)
{
    va_list     ap;
    VWString    message;

    self_check();
    va_start(ap, id);

    Ivyc_vwstr_clear(&message);
    format_message(line_number,
                   &Ivyc_error_message_format[id],
                   &message, ap);
    fprintf(stderr, "%s:%3d: error: ", Ivyc_get_current_compiler()->path,
            line_number);
    ISandBox_print_wcs_ln(stderr, message.string);
    va_end(ap);

    exit(1);
}

void
Ivyc_compile_warning(int line_number, CompileError id, ...)
{
    va_list     ap;
    VWString    message;

    self_check();
    va_start(ap, id);

    Ivyc_vwstr_clear(&message);
    format_message(line_number,
                   &Ivyc_error_message_format[id],
                   &message, ap);
    fprintf(stderr, "%s:%3d: warning: ", Ivyc_get_current_compiler()->path,
            line_number);
    ISandBox_print_wcs_ln(stderr, message.string);
	MEM_free(message.string);
    va_end(ap);
}

int
yyerror(char const *str)
{
    Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                      PARSE_ERR,
                      STRING_MESSAGE_ARGUMENT, "token", yytext,
                      MESSAGE_ARGUMENT_END);

    return 0;
}
