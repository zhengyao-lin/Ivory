#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

static ISandBox_ErrorStatus
check_array(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
            ISandBox_Executable *exe, Function *func, int pc,
            ISandBox_ObjectRef *exception_p)
{
    if (array.data == NULL) {
        *exception_p
            = ISandBox_create_exception(ISandBox, ISandBox_NULL_POINTER_EXCEPTION_NAME,
                                   NULL_POINTER_ERR,
                                   ISandBox_MESSAGE_ARGUMENT_END);
        return ISandBox_ERROR;
    }
    if (index < 0 || index >= array.data->u.array.size) {
        *exception_p
            = ISandBox_create_exception(ISandBox, ARRAY_INDEX_EXCEPTION_NAME,
                                   INDEX_OUT_OF_BOUNDS_ERR,
                                   ISandBox_INT_MESSAGE_ARGUMENT, "index", index,
                                   ISandBox_INT_MESSAGE_ARGUMENT, "size",
                                   array.data->u.array.size,
                                   ISandBox_MESSAGE_ARGUMENT_END);
        return ISandBox_ERROR;
    }
    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_get_int(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                  int *value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;
    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    *value = array.data->u.array.u.int_array[index];

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_get_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array,
                     int index, double *value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    *value = array.data->u.array.u.double_array[index];

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_get_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array,
                     int index, long double *value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    *value = array.data->u.array.u.long_double_array[index];

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_get_object(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     ISandBox_ObjectRef *value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    *value = array.data->u.array.u.object[index];

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_set_int(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                  int value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    array.data->u.array.u.int_array[index] = value;

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_set_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     double value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    array.data->u.array.u.double_array[index] = value;

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_set_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     long double value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    array.data->u.array.u.long_double_array[index] = value;

    return ISandBox_SUCCESS;
}

ISandBox_ErrorStatus
ISandBox_array_set_object(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     ISandBox_ObjectRef value, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;

    status = check_array(ISandBox, array, index,
                         ISandBox->current_executable->executable,
                         ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }

    array.data->u.array.u.object[index] = value;

    return ISandBox_SUCCESS;
}

int
ISandBox_array_size(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array)
{
    return array->u.array.size;
}

/* This function doesn't update array->u.array.size.
 */
static void
resize_array(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int new_size)
{
    int new_alloc_size;
    ISandBox_Boolean need_realloc;

    DBG_assert(array->type == ARRAY_OBJECT, ("array->type..%d", array->type));

    if (new_size > array->u.array.alloc_size) {
        new_alloc_size = array->u.array.alloc_size * 2;
        if (new_alloc_size < new_size) {
            new_alloc_size = new_size + ARRAY_ALLOC_SIZE;
        } else if (new_alloc_size - array->u.array.alloc_size
                   > ARRAY_ALLOC_SIZE) {
            new_alloc_size = array->u.array.alloc_size + ARRAY_ALLOC_SIZE;
        }
        need_realloc = ISandBox_TRUE;
    } else if (array->u.array.alloc_size - new_size > ARRAY_ALLOC_SIZE) {
        new_alloc_size = new_size;
        need_realloc = ISandBox_TRUE;
    } else {
        need_realloc = ISandBox_FALSE;
    }
    if (need_realloc) {
        ISandBox_check_gc(ISandBox);
        switch (array->u.array.type) {
        case INT_ARRAY:
            array->u.array.u.int_array
                = MEM_realloc(array->u.array.u.int_array,
                              new_alloc_size * sizeof(int));
            ISandBox->heap.current_heap_size
                += (new_alloc_size - array->u.array.alloc_size)
                * sizeof(int);
            break;
        case DOUBLE_ARRAY:
            array->u.array.u.double_array
                = MEM_realloc(array->u.array.u.double_array,
                              new_alloc_size * sizeof(double));
            ISandBox->heap.current_heap_size
                += (new_alloc_size - array->u.array.alloc_size)
                * sizeof(double);
            break;
        case LONG_DOUBLE_ARRAY:
            array->u.array.u.long_double_array
                = MEM_realloc(array->u.array.u.long_double_array,
                              new_alloc_size * sizeof(long double));
            ISandBox->heap.current_heap_size
                += (new_alloc_size - array->u.array.alloc_size)
                * sizeof(long double);
            break;
        case OBJECT_ARRAY:
            array->u.array.u.object
                = MEM_realloc(array->u.array.u.object,
                              new_alloc_size * sizeof(ISandBox_ObjectRef));
            ISandBox->heap.current_heap_size
                += (new_alloc_size - array->u.array.alloc_size)
                * sizeof(ISandBox_ObjectRef);
            break;
        case FUNCTION_INDEX_ARRAY:
        default:
            DBG_panic(("array->u.array.type..%d", array->u.array.type));
        }
        array->u.array.alloc_size = new_alloc_size;
    }
}

void
ISandBox_array_resize(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int new_size)
{
    int i;    

    resize_array(ISandBox, array, new_size);

    switch (array->u.array.type) {
    case INT_ARRAY:
        for (i = array->u.array.size; i < new_size; i++) {
            array->u.array.u.int_array[i] = 0;
        }
        break;
    case DOUBLE_ARRAY:
        for (i = array->u.array.size; i < new_size; i++) {
            array->u.array.u.double_array[i] = 0;
        }
        break;
    case LONG_DOUBLE_ARRAY:
        for (i = array->u.array.size; i < new_size; i++) {
            array->u.array.u.long_double_array[i] = 0;
        }
        break;
    case OBJECT_ARRAY:
        for (i = array->u.array.size; i < new_size; i++) {
            array->u.array.u.object[i] = ISandBox_null_object_ref;
        }
        break;
    case FUNCTION_INDEX_ARRAY:
    default:
        DBG_panic(("array->u.array.type..%d", array->u.array.type));
    }

    array->u.array.size = new_size;
}

void
ISandBox_array_insert(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int pos,
                 ISandBox_Value value)
{
    resize_array(ISandBox, array, array->u.array.size + 1);

    switch (array->u.array.type) {
    case INT_ARRAY:
        memmove(&array->u.array.u.int_array[pos+1],
                &array->u.array.u.int_array[pos],
                sizeof(int) * (array->u.array.size - pos));
        array->u.array.u.int_array[pos] = value.int_value;
        break;
    case DOUBLE_ARRAY:
        memmove(&array->u.array.u.double_array[pos+1],
                &array->u.array.u.double_array[pos],
                sizeof(double) * (array->u.array.size - pos));
        array->u.array.u.double_array[pos] = value.double_value;
        break;
    case LONG_DOUBLE_ARRAY:
        memmove(&array->u.array.u.long_double_array[pos+1],
                &array->u.array.u.long_double_array[pos],
                sizeof(long double) * (array->u.array.size - pos));
        array->u.array.u.long_double_array[pos] = value.long_double_value;
        break;
    case OBJECT_ARRAY:
        memmove(&array->u.array.u.object[pos+1],
                &array->u.array.u.object[pos],
                sizeof(ISandBox_ObjectRef) * (array->u.array.size - pos));
        array->u.array.u.object[pos] = value.object;
        break;
    case FUNCTION_INDEX_ARRAY:
    default:
        DBG_panic(("array->u.array.type..%d", array->u.array.type));
    }

    array->u.array.size++;
}

void
ISandBox_array_remove(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int pos)
{

    switch (array->u.array.type) {
    case INT_ARRAY:
        memmove(&array->u.array.u.int_array[pos],
                &array->u.array.u.int_array[pos+1],
                sizeof(int) * (array->u.array.size - pos - 1));
        break;
    case DOUBLE_ARRAY:
        memmove(&array->u.array.u.double_array[pos],
                &array->u.array.u.double_array[pos+1],
                sizeof(double) * (array->u.array.size - pos - 1));
        break;
    case LONG_DOUBLE_ARRAY:
        memmove(&array->u.array.u.long_double_array[pos],
                &array->u.array.u.long_double_array[pos+1],
                sizeof(long double) * (array->u.array.size - pos - 1));
        break;
    case OBJECT_ARRAY:
        memmove(&array->u.array.u.object[pos],
                &array->u.array.u.object[pos+1],
                sizeof(ISandBox_ObjectRef) * (array->u.array.size - pos - 1));
        break;
    case FUNCTION_INDEX_ARRAY:
    default:
        DBG_panic(("array->u.array.type..%d", array->u.array.type));
    }

    resize_array(ISandBox, array, array->u.array.size - 1);
    array->u.array.size--;
}

int
ISandBox_string_length(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *string)
{
    return ISandBox_wcslen(string->u.string.string);
}

ISandBox_Char *
ISandBox_string_get_string(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *string)
{
    return string->u.string.string;
}

static ISandBox_ErrorStatus
check_string_index(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef str, int index,
                   ISandBox_Executable *exe, Function *func, int pc,
                   ISandBox_ObjectRef *exception_p)
{
    if (is_object_null(str)) {
        *exception_p
            = ISandBox_create_exception(ISandBox, ISandBox_NULL_POINTER_EXCEPTION_NAME,
                                   NULL_POINTER_ERR,
                                   ISandBox_MESSAGE_ARGUMENT_END);
        return ISandBox_ERROR;
    }
    if (index < 0 || index >= str.data->u.string.length) {
        *exception_p
            = ISandBox_create_exception(ISandBox, STRING_INDEX_EXCEPTION_NAME,
                                   INDEX_OUT_OF_BOUNDS_ERR,
                                   ISandBox_INT_MESSAGE_ARGUMENT, "index", index,
                                   ISandBox_INT_MESSAGE_ARGUMENT, "size",
                                   str.data->u.string.length,
                                   ISandBox_MESSAGE_ARGUMENT_END);
        return ISandBox_ERROR;
    }
    return ISandBox_SUCCESS;
}


ISandBox_ErrorStatus
ISandBox_string_get_character(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef string,
                         int index, ISandBox_Char *ch, ISandBox_ObjectRef *exception_p)
{
    ISandBox_ErrorStatus status;
    status = check_string_index(ISandBox, string, index,
                                ISandBox->current_executable->executable,
                                ISandBox->current_function, ISandBox->pc, exception_p);
    if (status != ISandBox_SUCCESS) {
        return status;
    }
    *ch = string.data->u.string.string[index];

    return ISandBox_SUCCESS;
}

ISandBox_Value
ISandBox_string_substr(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *str,
                  int pos, int len)
{
    ISandBox_Char *new_str;
    ISandBox_Value ret;

    new_str = MEM_malloc(sizeof(ISandBox_Char) * (len+1));
    ISandBox_wcsncpy(new_str, str->u.string.string + pos, len);
    new_str[len] = L'\0';
    ret.object = ISandBox_create_ISandBox_string_i(ISandBox, new_str);

    return ret;
}

static int
get_field_index_sub(ExecClass *ec, char *field_name, int *super_count)
{
    int i;
    int index;

    if (ec->super_class) {
        index = get_field_index_sub(ec->super_class, field_name, super_count);
        if (index != FIELD_NOT_FOUND) {
            return index;
        }
    }
    for (i = 0; i < ec->ISandBox_class->field_count; i++) {
        if (!strcmp(ec->ISandBox_class->field[i].name, field_name)) {
            return i + *super_count;
        }
    }
    *super_count += ec->ISandBox_class->field_count;

    return FIELD_NOT_FOUND;
}

int
ISandBox_get_field_index(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef obj,
                    char *field_name)
{
    int super_count = 0;

    return get_field_index_sub(obj.v_table->exec_class, field_name,
                               &super_count);
}

ISandBox_Value
ISandBox_get_field(ISandBox_ObjectRef obj, int index)
{
    return obj.data->u.class_object.field[index];
}

void
ISandBox_set_field(ISandBox_ObjectRef obj, int index, ISandBox_Value value)
{
    obj.data->u.class_object.field[index] = value;
}

void *
ISandBox_object_get_native_pointer(ISandBox_Object *obj)
{
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT,
               ("obj->type..%d\n", obj->type));
    return obj->u.native_pointer.pointer;
}

ISandBox_Boolean
ISandBox_check_native_pointer_type(ISandBox_Object *native_pointer,
                              ISandBox_NativePointerInfo *info)
{
    ISandBox_NativePointerInfo *info_p;

    for (info_p = native_pointer->u.native_pointer.info;
	 info_p; info_p = info_p->super_class) {
	if (info_p == info) {
	    return ISandBox_TRUE;
	}
    }
    return ISandBox_FALSE;
}


void
ISandBox_object_set_native_pointer(ISandBox_Object *obj, void *p)
{
    DBG_assert(obj->type == NATIVE_POINTER_OBJECT,
               ("obj->type..%d\n", obj->type));
    obj->u.native_pointer.pointer = p;
}

void
ISandBox_set_exception(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                  ISandBox_NativeLibInfo *lib_info,
                  char *package_name, char *class_name,
                  int error_id, ...)
{
    int class_index;
    ISandBox_ObjectRef obj;
    VString     message;
    va_list     ap;
    int message_index;
    int stack_trace_index;

    va_start(ap, error_id);
    class_index = ISandBox_search_class(ISandBox, package_name, class_name);
    obj = ISandBox_create_class_object(ISandBox, context, class_index);

    ISandBox_format_message(lib_info->message_format, error_id,
                       &message, ap);
    va_end(ap);

    message_index
        = ISandBox_get_field_index(ISandBox, obj, "message");
    obj.data->u.class_object.field[message_index].object
        = ISandBox_create_ISandBox_string_i(ISandBox, message.string);

    stack_trace_index
        = ISandBox_get_field_index(ISandBox, obj, "stack_trace");
    obj.data->u.class_object.field[stack_trace_index].object
        = ISandBox_create_array_object_i(ISandBox, 0);

    ISandBox->current_exception = obj;
}

void
ISandBox_set_null(ISandBox_Value *value)
{
    value->object.v_table = NULL;
    value->object.data = NULL;
}

ISandBox_ObjectRef
ISandBox_up_cast(ISandBox_ObjectRef obj, int target_index)
{
    ISandBox_ObjectRef ret;

    ret = obj;
    ret.v_table = obj.v_table->exec_class->interface_v_table[target_index];

    return ret;
}

ISandBox_Value
ISandBox_check_exception(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Value ret;

    ret.object = ISandBox->current_exception;

    return ret;
}
