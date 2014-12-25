#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

void
ISandBox_vstr_clear(VString *v)
{
    v->string = NULL;
}

static int
my_strlen(ISandBox_Char *str)
{
    if (str == NULL) {
        return 0;
    }
    return ISandBox_wcslen(str);
}

void
ISandBox_vstr_append_string(VString *v, ISandBox_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = sizeof(ISandBox_Char) * (old_len + ISandBox_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    ISandBox_wcscpy(&v->string[old_len], str);
}

void
ISandBox_vstr_append_character(VString *v, ISandBox_Char ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string,sizeof(ISandBox_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

void
ISandBox_initialize_value(ISandBox_TypeSpecifier *type, ISandBox_Value *value)
{
    if (type->derive_count > 0) {
        if (type->derive[0].tag == ISandBox_ARRAY_DERIVE) {
            value->object = ISandBox_null_object_ref;
        } else {
            DBG_assert(0, ("tag..%d", type->derive[0].tag));
        }
    } else {
        switch (type->basic_type) {
        case ISandBox_VOID_TYPE:  /* FALLTHRU */
        case ISandBox_BOOLEAN_TYPE: /* FALLTHRU */
        case ISandBox_INT_TYPE: /* FALLTHRU */
        case ISandBox_ENUM_TYPE: /* FALLTHRU */
            value->int_value = 0;
            break;
        case ISandBox_DOUBLE_TYPE:
            value->double_value = 0.0;
            break;
        case ISandBox_LONG_DOUBLE_TYPE:
            value->long_double_value = 0.0;
            break;
        case ISandBox_OBJECT_TYPE: /* FALLTHRU */
		case ISandBox_ITERATOR_TYPE: /* FALLTHRU */
        case ISandBox_STRING_TYPE: /* FALLTHRU */
        case ISandBox_NATIVE_POINTER_TYPE: /* FALLTHRU */
        case ISandBox_CLASS_TYPE: /* FALLTHRU */
		case ISandBox_PLACEHOLDER: /* FALLTHRU */
        case ISandBox_DELEGATE_TYPE:
            value->object = ISandBox_null_object_ref;
            break;
        case ISandBox_NULL_TYPE: /* FALLTHRU */
        case ISandBox_BASE_TYPE: /* FALLTHRU */
        case ISandBox_UNSPECIFIED_IDENTIFIER_TYPE: /* FALLTHRU */
        default:
            DBG_assert(0, ("basic_type..%d", type->basic_type));
        }
    }
}
