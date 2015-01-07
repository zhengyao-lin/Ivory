#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

/* Ivory Lib */
#include "IvoryLib/IO.c"
#include "IvoryLib/Math.c"
#include "IvoryLib/System.c"
#include "IvoryLib/Type.c"
#include "IvoryLib/Time.c"

static void file_finalizer(ISandBox_VirtualMachine *ISandBox, ISandBox_Object* obj);

static ISandBox_NativePointerInfo st_file_type_info = {
    "Ivory.lang.file_pointer",
    file_finalizer,
    NULL
};

extern ISandBox_ErrorDefinition ISandBox_native_error_message_format[];

static ISandBox_NativeLibInfo st_lib_info = {
    ISandBox_native_error_message_format,
};

typedef enum {
    INSERT_INDEX_OUT_OF_BOUNDS_ERR,
    REMOVE_INDEX_OUT_OF_BOUNDS_ERR,
    STRING_POS_OUT_OF_BOUNDS_ERR,
    STRING_SUBSTR_LEN_ERR,
    FOPEN_1ST_ARG_NULL_ERR,
    FOPEN_2ND_ARG_NULL_ERR,
    FGETS_ARG_NULL_ERR,
    FGETS_FP_BAD_TYPE_ERR,
    FGETS_FP_INVALID_ERR,
    FGETS_BAD_MULTIBYTE_CHARACTER_ERR,
    FPUTS_2ND_ARG_NULL_ERR,
    FPUTS_FP_BAD_TYPE_ERR,
    FPUTS_FP_INVALID_ERR,
    FCLOSE_ARG_NULL_ERR,
    FCLOSE_FP_BAD_TYPE_ERR,
    FCLOSE_FP_INVALID_ERR,
    PARSE_INT_ARG_NULL_ERR,
    PARSE_INT_FORMAT_ERR,
    PARSE_DOUBLE_ARG_NULL_ERR,
    PARSE_DOUBLE_FORMAT_ERR,
    PARSE_LONG_DOUBLE_ARG_NULL_ERR,
    PARSE_LONG_DOUBLE_FORMAT_ERR,
	OUT_OF_ITERATOR_ERR,
	NEW_ITERATOR_ERR,
    NATIVE_RUNTIME_ERROR_COUNT_PLUS_1
} NativeRuntimeError;

ISandBox_Char *
ISandBox_mbstowcs_SEC(const char *src)
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

static void
file_finalizer(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *obj)
{
    FILE *fp;

    fp = (FILE*)ISandBox_object_get_native_pointer(obj);
    if (fp && fp != stdin && fp != stdout && fp != stderr) {
        fclose(fp);
    }
}

static ISandBox_Value
nv_print_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Char *str;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (args[0].object.data == NULL) {
        str = NULL_STRING;
    } else {
        /*if (args[0].object.data->u.string) { printf("hi\n"); }*/
        str = args[0].object.data->u.string.string;
    }
    ISandBox_print_wcs(stdout, str);
    fflush(stdout);

    return ret;
}

static ISandBox_Value
nv_fopen_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *file_name;
    char *mode;
    FILE *fp;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_1ST_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (is_object_null(args[1].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_2ND_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    file_name = ISandBox_wcstombs(args[0].object.data->u.string.string);
    mode = ISandBox_wcstombs(args[1].object.data->u.string.string);
    fp = fopen(file_name, mode);
    MEM_free(file_name);
    MEM_free(mode);

    ret.object = ISandBox_create_native_pointer(ISandBox, context, fp,
                                           &st_file_type_info);

    return ret;
}

static ISandBox_Value
nv_fgets_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    FILE *fp;
    char buf[LINE_BUF_SIZE];
    char *mb_buf = NULL;
    int ret_len = 0;
    ISandBox_Char *wc_str;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FOPEN_1ST_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!ISandBox_check_native_pointer_type(args[0].object.data,
                                       &st_file_type_info)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FGETS_FP_BAD_TYPE_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = ISandBox_object_get_native_pointer(args[0].object.data);
    if (fp == NULL) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FGETS_FP_INVALID_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }

    while (fgets(buf, LINE_BUF_SIZE, fp)) {
        int new_len;
        new_len = ret_len + strlen(buf);
        mb_buf = MEM_realloc(mb_buf, new_len + 1);
        if (ret_len == 0) {
            strcpy(mb_buf, buf);
        } else {
            strcat(mb_buf, buf);
        }
        ret_len = new_len;
        if (mb_buf[ret_len-1] == '\n')
            break;
    }
    if (ret_len > 0) {
        wc_str = ISandBox_mbstowcs_s(mb_buf);
        if (wc_str == NULL) {
            MEM_free(mb_buf);
            ISandBox_set_exception(ISandBox, context, &st_lib_info,
                              ISandBox_Ivory_DEFAULT_PACKAGE,
                              MULTIBYTE_CONVERTION_EXCEPTION_NAME,
                              (int)FGETS_BAD_MULTIBYTE_CHARACTER_ERR,
                              ISandBox_MESSAGE_ARGUMENT_END);
        }
        ret.object = ISandBox_create_ISandBox_string(ISandBox, context, wc_str);
    } else {
        ret.object = ISandBox_null_object_ref;
    }
    MEM_free(mb_buf);

    return ret;
}

static ISandBox_Value
nv_fputs_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Char *str;
    FILE *fp;

    ret.int_value = 0;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    if (args[0].object.data == NULL) {
        str = NULL_STRING;
    } else {
        str = args[0].object.data->u.string.string;
    }
    if (is_object_null(args[1].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_2ND_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!ISandBox_check_native_pointer_type(args[1].object.data,
                                       &st_file_type_info)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_FP_BAD_TYPE_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = ISandBox_object_get_native_pointer(args[1].object.data);
    if (fp == NULL) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FPUTS_FP_INVALID_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    ISandBox_print_wcs(fp, str);

    return ret;
}

static ISandBox_Value
nv_fclose_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
               int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    FILE *fp;

    ret.int_value = 0;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (!ISandBox_check_native_pointer_type(args[0].object.data,
                                       &st_file_type_info)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_FP_BAD_TYPE_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fp = ISandBox_object_get_native_pointer(args[0].object.data);
    if (fp == NULL) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          FCLOSE_FP_INVALID_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    fclose(fp);
    ISandBox_object_set_native_pointer(args[0].object.data, NULL);

    return ret;
}

static ISandBox_Value
nv_randomize_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                  int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    srand(time(NULL));

    return ret;
}

static ISandBox_Value
nv_random_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                  int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    if (args[0].int_value == 0) {
        ret.int_value = 0;
    } else {
        ret.int_value = rand() % args[0].int_value;
    }

    return ret;
}

static ISandBox_Value
nv_parse_int_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                  int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *mb_str;

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          PARSE_INT_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    mb_str = ISandBox_wcstombs(args[0].object.data->u.string.string);

    if (sscanf(mb_str, "%d", &ret.int_value) != 1) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          NUMBER_FORMAT_EXCEPTION_NAME,
                          PARSE_INT_FORMAT_ERR,
                          ISandBox_STRING_MESSAGE_ARGUMENT, "str", mb_str,
                          ISandBox_MESSAGE_ARGUMENT_END);
    }
    MEM_free(mb_str);
    return ret;
}

static ISandBox_Value
nv_parse_double_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                     int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *mb_str;

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          PARSE_DOUBLE_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    mb_str = ISandBox_wcstombs(args[0].object.data->u.string.string);

    if (sscanf(mb_str, "%lf", &ret.double_value) != 1) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          NUMBER_FORMAT_EXCEPTION_NAME,
                          PARSE_DOUBLE_FORMAT_ERR,
                          ISandBox_STRING_MESSAGE_ARGUMENT, "str", mb_str,
                          ISandBox_MESSAGE_ARGUMENT_END);
    }
    MEM_free(mb_str);
    return ret;
}

static ISandBox_Value
nv_parse_long_double_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                     int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *mb_str;

    if (is_object_null(args[0].object)) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ISandBox_NULL_POINTER_EXCEPTION_NAME,
                          PARSE_LONG_DOUBLE_ARG_NULL_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    mb_str = ISandBox_wcstombs(args[0].object.data->u.string.string);

    if (sscanf(mb_str, "%Lf", &ret.long_double_value) != 1) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          NUMBER_FORMAT_EXCEPTION_NAME,
                          PARSE_LONG_DOUBLE_FORMAT_ERR,
                          ISandBox_STRING_MESSAGE_ARGUMENT, "str", mb_str,
                          ISandBox_MESSAGE_ARGUMENT_END);
    }
    MEM_free(mb_str);
    return ret;
}

/*Ivory.Math -> IvoryLib/Math.c*/

/*Ivory.IO -> IvoryLib/IO.c*/

static ISandBox_Value
nv_array_size_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                   int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Object *array;

    DBG_assert(arg_count == 0, ("arg_count..%d", arg_count));

    array = args[0].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    ret.int_value = ISandBox_array_size(ISandBox, array);

    return ret;
}

static ISandBox_Value
nv_array_resize_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                     int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret; /* dummy */
    ISandBox_Object *array;
    int new_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    new_size = args[0].int_value;
    array = args[1].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    ISandBox_array_resize(ISandBox, array, new_size);

    return ret;
}

static ISandBox_Value
nv_array_insert_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                     int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Object *array;
    ISandBox_Value value;
    int pos;
    int array_size;

    pos = args[0].int_value;
    value = args[1];
    array = args[2].object.data;

    array_size = ISandBox_array_size(ISandBox, array);

    if (pos < 0 || pos > array_size) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ARRAY_INDEX_EXCEPTION_NAME,
                          INSERT_INDEX_OUT_OF_BOUNDS_ERR,
                          ISandBox_INT_MESSAGE_ARGUMENT, "pos", pos,
                          ISandBox_INT_MESSAGE_ARGUMENT, "size", array_size,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    ISandBox_array_insert(ISandBox, array, pos, value);

    return ret;
}

ISandBox_Value
nv_array_remove_proc(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret; /* dummy */
    ISandBox_Object *array;
    int pos;
    int array_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));

    pos = args[0].int_value;
    array = args[1].object.data;
    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    array_size = ISandBox_array_size(ISandBox, array);
    if (pos < 0 || pos >= array_size) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          ARRAY_INDEX_EXCEPTION_NAME,
                          REMOVE_INDEX_OUT_OF_BOUNDS_ERR,
                          ISandBox_INT_MESSAGE_ARGUMENT, "pos", pos,
                          ISandBox_INT_MESSAGE_ARGUMENT, "size", array_size,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    ISandBox_array_remove(ISandBox, array, pos);

    return ret;
}

static ISandBox_Value
nv_array_add_proc(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                  int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Object *array;
    ISandBox_Value value;
    int array_size;

    DBG_assert(arg_count == 1, ("arg_count..%d", arg_count));
    value = args[0];
    array = args[1].object.data;

    array_size = ISandBox_array_size(ISandBox, array);

    ISandBox_array_insert(ISandBox, array, array_size, value);

    return ret;
}

static ISandBox_Value
nv_array_iterator_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                   int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

	ret.object = ISandBox_create_iterator(ISandBox, context, args[0].object.data->u.array);

    return ret;
}

static ISandBox_Value
nv_string_length_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Object *str;

    DBG_assert(arg_count == 0, ("arg_count..%d", arg_count));

    str = args[0].object.data;
    DBG_assert(str->type == STRING_OBJECT,
               ("str->type..%d", str->type));

    ret.int_value = ISandBox_string_length(ISandBox, str);

    return ret;
}

static ISandBox_Value
nv_string_substr_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Object *str;
    int pos;
    int len;
    int org_len;

    DBG_assert(arg_count == 2, ("arg_count..%d", arg_count));

    pos = args[0].int_value;
    len = args[1].int_value;
    str = args[2].object.data;
    DBG_assert(str->type == STRING_OBJECT,
               ("str->type..%d", str->type));

    org_len = ISandBox_string_length(ISandBox, str);
    
    if (pos < 0 || pos >= org_len) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          STRING_INDEX_EXCEPTION_NAME,
                          STRING_POS_OUT_OF_BOUNDS_ERR,
                          ISandBox_INT_MESSAGE_ARGUMENT, "len", org_len,
                          ISandBox_INT_MESSAGE_ARGUMENT, "pos", pos,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }
    if (len < 0 || pos + len > org_len) {
        ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          STRING_INDEX_EXCEPTION_NAME,
                          STRING_SUBSTR_LEN_ERR,
                          ISandBox_INT_MESSAGE_ARGUMENT, "len", len,
                          ISandBox_MESSAGE_ARGUMENT_END);
        return ret;
    }

    ret = ISandBox_string_substr(ISandBox, str, pos, len);

    return ret;
}

static ISandBox_Value
nv_iterator_next_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
	Iterator *iterator;

	iterator = &args[0].object.data->u.iterator;
	iterator->cursor = ( iterator->cursor+1 < iterator->array.size ? iterator->cursor+1 : -2 );
	if (iterator->cursor >= 0) {
		ret = ISandBox_get_iterator_currrent(ISandBox, context, args[0].object);
	} else if (iterator->cursor == -1) {
		ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          NEW_ITERATOR_EXCEPTION_NAME,
                          NEW_ITERATOR_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
	} else if (iterator->cursor == -2) {
		ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          OUT_OF_ITERATOR_EXCEPTION_NAME,
                          OUT_OF_ITERATOR_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
	}

    return ret;
}

static ISandBox_Value
nv_iterator_hasNext_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
	Iterator *iterator;

	iterator = &args[0].object.data->u.iterator;
	if (iterator->cursor+1 < iterator->array.size) {
		ret.int_value = ISandBox_TRUE;
	} else {
		ret.int_value = ISandBox_FALSE;
	}

    return ret;
}

static ISandBox_Value
nv_iterator_current_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
	Iterator *iterator;

	iterator = &args[0].object.data->u.iterator;
	if (iterator->cursor >= 0 && iterator->cursor < iterator->array.size) {
		ret = ISandBox_get_iterator_currrent(ISandBox, context, args[0].object);
	} else if (iterator->cursor == -1) {
		ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          NEW_ITERATOR_EXCEPTION_NAME,
                          NEW_ITERATOR_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
	} else if (iterator->cursor == -2) {
		ISandBox_set_exception(ISandBox, context, &st_lib_info,
                          ISandBox_Ivory_DEFAULT_PACKAGE,
                          OUT_OF_ITERATOR_EXCEPTION_NAME,
                          OUT_OF_ITERATOR_ERR,
                          ISandBox_MESSAGE_ARGUMENT_END);
	}

    return ret;
}

/*static ISandBox_Value
nv_string_length_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Value content;

    DBG_assert(arg_count == 0, ("arg_count..%d", arg_count));

	content = args[0].object.data->u.object;

    ret.int_value = args[0].object.data->u.object.object->;

    return ret;
}*/

ISandBox_Value
nv_test_native_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                    int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Value *var;
    /*ISandBox_Value ivy_arg[2];

    ivy_arg[0].int_value = 7;
    ISandBox_invoke_delegate(ISandBox, args[0], ivy_arg);*/
    /*var = (ISandBox_Value *) ISandBox_object_get_native_pointer(args[0].object.data);*/
    /*printf("%f\n", args[0].object.data->u.object);*/
    /*ret.object = args[0].object;*/

    return ret;
}

ISandBox_Value
nv_get_object_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                    int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    /*ret.object = ISandBox_create_object(ISandBox, context, args[0].object);*/

    return ret;
}

void
ISandBox_add_native_functions(ISandBox_VirtualMachine *ISandBox)
{
    /* IO.ivh */
    ISandBox_add_native_functions_io(ISandBox);

    /* System.ivh */
    ISandBox_add_native_functions_system(ISandBox);

    /* Math.ivh */
    ISandBox_add_native_functions_math(ISandBox);

    /* Type.ivh */
    ISandBox_add_native_functions_type(ISandBox);

	/* Time.ivh */
	ISandBox_add_native_functions_time(ISandBox);

    /* lang.ivh */
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "print", nv_print_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "__fopen", nv_fopen_proc, 2,
                            ISandBox_FALSE, ISandBox_TRUE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "__fgets", nv_fgets_proc, 1,
                            ISandBox_FALSE, ISandBox_TRUE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "__fputs", nv_fputs_proc, 2,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "__fclose", nv_fclose_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "randomize",
                            nv_randomize_proc, 0,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "random", nv_random_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "parse_int", nv_parse_int_proc,
                            1, ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "parse_double",
                            nv_parse_double_proc, 1, ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "parse_long_double",
                            nv_parse_long_double_proc, 1, ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.lang", "get_object", nv_get_object_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);

    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_SIZE,
                            nv_array_size_proc, 0, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_RESIZE,
                            nv_array_resize_proc, 1, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_INSERT,
                            nv_array_insert_proc, 2, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_REMOVE,
                            nv_array_remove_proc, 1, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_ADD,
                            nv_array_add_proc, 1, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ARRAY_PREFIX ARRAY_METHOD_ITERATOR,
                            nv_array_iterator_proc, 0, ISandBox_TRUE, ISandBox_FALSE);

	/* iterator */
	ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ITERATOR_PREFIX ITERATOR_METHOD_NEXT,
                            nv_iterator_next_proc, 0, ISandBox_TRUE, ISandBox_FALSE);
	ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ITERATOR_PREFIX ITERATOR_METHOD_HASNEXT,
                            nv_iterator_hasNext_proc, 0, ISandBox_TRUE, ISandBox_FALSE);
	ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            ITERATOR_PREFIX ITERATOR_METHOD_CURRENT,
                            nv_iterator_current_proc, 0, ISandBox_TRUE, ISandBox_FALSE);

    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            STRING_PREFIX STRING_METHOD_LENGTH,
                            nv_string_length_proc, 0, ISandBox_TRUE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                            STRING_PREFIX STRING_METHOD_SUBSTR,
                            nv_string_substr_proc, 2, ISandBox_TRUE, ISandBox_FALSE);

    ISandBox_add_native_function(ISandBox, "Ivory.lang", "test_native",
                            nv_test_native_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
