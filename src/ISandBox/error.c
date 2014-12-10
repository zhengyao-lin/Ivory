#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

typedef struct {
    ISandBox_MessageArgumentType type;
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
    ISandBox_MessageArgumentType type;
    
    while ((type = va_arg(ap, ISandBox_MessageArgumentType))
           != ISandBox_MESSAGE_ARGUMENT_END) {
        arg[index].type = type;
        arg[index].name = va_arg(ap, char*);
        switch (type) {
        case ISandBox_INT_MESSAGE_ARGUMENT:
            arg[index].u.int_val = va_arg(ap, int);
            break;
        case ISandBox_DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.double_val = va_arg(ap, double);
            break;
        case ISandBox_LONG_DOUBLE_MESSAGE_ARGUMENT:
            arg[index].u.long_double_val = va_arg(ap, long double);
            break;
        case ISandBox_STRING_MESSAGE_ARGUMENT:
            arg[index].u.string_val = va_arg(ap, char*);
            break;
        case ISandBox_POINTER_MESSAGE_ARGUMENT:
            arg[index].u.pointer_val = va_arg(ap, void*);
            break;
        case ISandBox_CHARACTER_MESSAGE_ARGUMENT:
            arg[index].u.character_val = va_arg(ap, int);
            break;
        case ISandBox_MESSAGE_ARGUMENT_END:
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

    for (i = 0; arg_list[i].type != ISandBox_MESSAGE_ARGUMENT_END; i++) {
        if (!strcmp(arg_list[i].name, arg_name)) {
            *arg = arg_list[i];
            return;
        }
    }
    assert(0);
}

static void
format_message(ISandBox_ErrorDefinition *format, VString *v,
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

    wc_format = ISandBox_mbstowcs_alloc(NULL, format->format);
    DBG_assert(wc_format != NULL, ("wc_format is null.\n"));
    
    for (i = 0; wc_format[i] != L'\0'; i++) {
        if (wc_format[i] != L'$') {
            ISandBox_vstr_append_character(v, wc_format[i]);
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
        case ISandBox_INT_MESSAGE_ARGUMENT:
            sprintf(buf, "%d", cur_arg.u.int_val);
            ISandBox_mbstowcs(buf, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%f", cur_arg.u.double_val);
            ISandBox_mbstowcs(buf, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_LONG_DOUBLE_MESSAGE_ARGUMENT:
            sprintf(buf, "%lf", cur_arg.u.long_double_val);
            ISandBox_mbstowcs(buf, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_STRING_MESSAGE_ARGUMENT:
            ISandBox_mbstowcs(cur_arg.u.string_val, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_POINTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%p", cur_arg.u.pointer_val);
            ISandBox_mbstowcs(buf, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_CHARACTER_MESSAGE_ARGUMENT:
            sprintf(buf, "%c", cur_arg.u.character_val);
            ISandBox_mbstowcs(buf, wc_buf);
            ISandBox_vstr_append_string(v, wc_buf);
            break;
        case ISandBox_MESSAGE_ARGUMENT_END:
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
    if (strcmp(ISandBox_error_message_format[0].format, "dummy") != 0) {
        DBG_panic(("runtime error message format error.\n"));
    }
    if (strcmp(ISandBox_error_message_format
               [RUNTIME_ERROR_COUNT_PLUS_1].format,
               "dummy") != 0) {
        DBG_panic(("runtime error message format error. "
                   "RUNTIME_ERROR_COUNT_PLUS_1..%d\n",
                   RUNTIME_ERROR_COUNT_PLUS_1));
    }
}

int
ISandBox_conv_pc_to_line_number(ISandBox_Executable *exe, Function *func, int pc)
{
    ISandBox_LineNumber *line_number;
    int line_number_size;
    int i;
    int ret;

    if (func) {
        line_number
            = exe->function[func->u.Ivory_f.index].code_block.line_number;
        line_number_size
            = exe->function[func->u.Ivory_f.index]
            .code_block.line_number_size;
    } else {
        line_number = exe->top_level.line_number;
        line_number_size = exe->top_level.line_number_size;
    }

    for (i = 0; i < line_number_size; i++) {
        if (pc >= line_number[i].start_pc
            && pc < line_number[i].start_pc + line_number[i].pc_count) {
            ret = line_number[i].line_number;
        }
    }

    return ret;
}

static void
error_v(ISandBox_Executable *exe, Function *func, int pc, RuntimeError id,
        va_list ap)
{
    VString     message;
    int         line_number;

    ISandBox_vstr_clear(&message);
    format_message(&ISandBox_error_message_format[id],
                   &message, ap);

    if (pc != NO_LINE_NUMBER_PC) {
        line_number = ISandBox_conv_pc_to_line_number(exe, func, pc);
        fprintf(stderr, "%s:%d:", exe->path, line_number);
    }
    ISandBox_print_wcs_ln(stderr, message.string);
}

void
ISandBox_error_i(ISandBox_Executable *exe, Function *func, int pc,
            RuntimeError id, ...)
{
    va_list     ap;

    self_check();
    va_start(ap, id);
    error_v(exe, func, pc, id, ap);
    va_end(ap);

    exit(1);
}

void
ISandBox_error_n(ISandBox_VirtualMachine *ISandBox, RuntimeError id, ...)
{
    va_list     ap;

    self_check();
    va_start(ap, id);
    error_v(ISandBox->current_executable->executable,
            ISandBox->current_function, ISandBox->pc, id, ap);
    va_end(ap);

    exit(1);
}

void
ISandBox_format_message(ISandBox_ErrorDefinition *error_definition,
                   int id, VString *message, va_list ap)
{
    ISandBox_vstr_clear(message);
    format_message(&error_definition[id],
                   message, ap);
}
