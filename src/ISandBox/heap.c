#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

void
ISandBox_check_gc(ISandBox_VirtualMachine *ISandBox)
{
#if 0
    ISandBox_garbage_collect(ISandBox);
#endif
    if (ISandBox->heap.current_heap_size > ISandBox->heap.current_threshold) {
        /* fprintf(stderr, "garbage collecting..."); */
        ISandBox_garbage_collect(ISandBox);
        /* fprintf(stderr, "done.\n"); */

        ISandBox->heap.current_threshold = ISandBox->heap.current_heap_size + HEAP_THRESHOLD_SIZE;
    }
}

static ISandBox_ObjectRef
alloc_object(ISandBox_VirtualMachine *ISandBox, ObjectType type)
{
    ISandBox_ObjectRef ret;

    ISandBox_check_gc(ISandBox);
    ret.v_table = NULL;
    ret.data = MEM_malloc(sizeof(ISandBox_Object));
    ISandBox->heap.current_heap_size += sizeof(ISandBox_Object);
    ret.data->type = type;
    ret.data->marked = ISandBox_TRUE;
    ret.data->prev = NULL;
    ret.data->next = ISandBox->heap.header;
    ISandBox->heap.header = ret.data;
    if (ret.data->next) {
        ret.data->next->prev = ret.data;
    }

    return ret;
}

static void
add_ref_in_native_method(ISandBox_Context *context, ISandBox_ObjectRef *obj)
{
    RefInNativeFunc *new_ref;

    new_ref = MEM_malloc(sizeof(RefInNativeFunc));
    new_ref->object = *obj;
    new_ref->next = context->ref_in_native_method;
    context->ref_in_native_method = new_ref;
}

void
ISandBox_add_reference_to_context(ISandBox_Context *context, ISandBox_Value value)
{
    add_ref_in_native_method(context, &value.object);
}

ISandBox_ObjectRef
ISandBox_literal_to_ISandBox_string_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Char *str)
{
    ISandBox_ObjectRef ret;

    ret = alloc_object(ISandBox, STRING_OBJECT);
    ret.v_table = ISandBox->string_v_table;
    ret.data->u.string.string = str;
    ret.data->u.string.length = ISandBox_wcslen(str);
    ret.data->u.string.is_literal = ISandBox_TRUE;

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_ISandBox_string_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Char *str)
{
    ISandBox_ObjectRef ret;
    int len;

    len = ISandBox_wcslen(str);

    ret = alloc_object(ISandBox, STRING_OBJECT);
    ret.v_table = ISandBox->string_v_table;
    ret.data->u.string.string = str;
    ISandBox->heap.current_heap_size += sizeof(ISandBox_Char) * (len + 1);
    ret.data->u.string.is_literal = ISandBox_FALSE;
    ret.data->u.string.length = len;

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_ISandBox_string(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      ISandBox_Char *str)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_ISandBox_string_i(ISandBox, str);
    add_ref_in_native_method(context, &ret);

    return ret;
}

ISandBox_ObjectRef
alloc_array(ISandBox_VirtualMachine *ISandBox, ArrayType type, int size)
{
    ISandBox_ObjectRef ret;

    ret = alloc_object(ISandBox, ARRAY_OBJECT);
    ret.data->u.array.type = type;
    ret.data->u.array.size = size;
    ret.data->u.array.alloc_size = size;
    ret.v_table = ISandBox->array_v_table;

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_iterator_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Array array)
{
    ISandBox_ObjectRef ret;
	ISandBox_ObjectRef copy_array;

	ret = alloc_object(ISandBox, ITERATOR_OBJECT);
	ret.data->u.iterator.cursor = -1;
	ret.data->u.iterator.array = array;
	ret.v_table = ISandBox->iterator_v_table;

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_iterator(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     ISandBox_Array array)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_iterator_i(ISandBox, array);

    return ret;
}

ISandBox_Value
ISandBox_get_iterator_currrent(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     ISandBox_ObjectRef iter)
{
    ISandBox_Value ret;
	Iterator iterator = iter.data->u.iterator;

    switch (iterator.array.type) {
		case INT_ARRAY:
			ret.int_value = iterator.array.u.int_array[iterator.cursor];
			break;
		case DOUBLE_ARRAY:
			ret.double_value = iterator.array.u.double_array[iterator.cursor];
			break;
		case LONG_DOUBLE_ARRAY:
			ret.long_double_value = iterator.array.u.long_double_array[iterator.cursor];
			break;
		case OBJECT_ARRAY:
			ret.object = iterator.array.u.object[iterator.cursor];
			break;
		default:
			DBG_panic(("iterator.array.type..%d", iterator.array.type));
	}

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_int_i(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef ret;
    int i;

    ret = alloc_array(ISandBox, INT_ARRAY, size);
    ret.data->u.array.u.int_array = MEM_malloc(sizeof(int) * size);
    ISandBox->heap.current_heap_size += sizeof(int) * size;
    for (i = 0; i < size; i++) {
        ret.data->u.array.u.int_array[i] = 0;
    }

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_int(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     int size)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_array_int_i(ISandBox, size);

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_double_i(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef ret;
    int i;

    ret = alloc_array(ISandBox, DOUBLE_ARRAY, size);
    ret.data->u.array.u.double_array = MEM_malloc(sizeof(double) * size);
    ISandBox->heap.current_heap_size += sizeof(double) * size;
    for (i = 0; i < size; i++) {
        ret.data->u.array.u.double_array[i] = 0.0;
    }

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_long_double_i(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef ret;
    int i;

    ret = alloc_array(ISandBox, LONG_DOUBLE_ARRAY, size);
    ret.data->u.array.u.long_double_array = MEM_malloc(sizeof(long double) * size);
    ISandBox->heap.current_heap_size += sizeof(long double) * size;
    for (i = 0; i < size; i++) {
        ret.data->u.array.u.long_double_array[i] = 0.0;
    }

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_double(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int size)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_array_double_i(ISandBox, size);
    add_ref_in_native_method(context, &ret);

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int size)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_array_long_double_i(ISandBox, size);
    add_ref_in_native_method(context, &ret);

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_object_i(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef ret;
    int i;

    ret = alloc_array(ISandBox, OBJECT_ARRAY, size);
    ret.data->u.array.u.object = MEM_malloc(sizeof(ISandBox_ObjectRef) * size);
    ISandBox->heap.current_heap_size += sizeof(ISandBox_ObjectRef) * size;
    for (i = 0; i < size; i++) {
        ret.data->u.array.u.object[i] = ISandBox_null_object_ref;
    }

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_array_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int size)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_array_object_i(ISandBox, size);
    add_ref_in_native_method(context, &ret);

    return ret;
}

static void
initialize_fields(ISandBox_VirtualMachine *ISandBox, ExecClass *ec, ISandBox_ObjectRef obj)
{
    ISandBox_Value value;

    value.object = obj;
    ISandBox_push_object(ISandBox, value);

    ISandBox->current_executable = ec->executable;
    ISandBox->current_function = NULL;
    ISandBox->pc = 0;
    ISandBox_expand_stack(ISandBox, ec->ISandBox_class->field_initializer.need_stack_size);
    ISandBox_execute_i(ISandBox, NULL, ec->ISandBox_class->field_initializer.code,
                  ec->ISandBox_class->field_initializer.code_size, 0);

    ISandBox_pop_object(ISandBox);
}

ISandBox_ObjectRef
ISandBox_create_class_object_i(ISandBox_VirtualMachine *ISandBox, int class_index)
{
    ExecClass *ec;
    ISandBox_ObjectRef obj;
    int i;

    obj = alloc_object(ISandBox, CLASS_OBJECT);

    ec = ISandBox->class[class_index];
    
    obj.v_table = ec->class_table;

    obj.data->u.class_object.field_count = ec->field_count;
    obj.data->u.class_object.field
        = MEM_malloc(sizeof(ISandBox_Value) * ec->field_count);
    for (i = 0; i < ec->field_count; i++) {
        ISandBox_initialize_value(ec->field_type[i],
                             &obj.data->u.class_object.field[i]);
    }
    initialize_fields(ISandBox, ec, obj);

    return obj;
}

ISandBox_ObjectRef
ISandBox_create_class_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int class_index)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_class_object_i(ISandBox, class_index);
    add_ref_in_native_method(context, &ret);

    return ret;
}

/* newly added */
ISandBox_ObjectRef
ISandBox_cast_object_to_string(ISandBox_VirtualMachine *ISandBox,
                       ISandBox_ObjectRef object)
{
    /*ISandBox_ObjectRef ret;

    switch (toType) {
        case ISandBox_STRING_TYPE:
            ret = alloc_object(ISandBox, OBJECT);
            ret = object;
            ret.data->type = STRING_OBJECT;
            break;
        case ISandBox_INT_TYPE:
            ret = alloc_object(ISandBox, OBJECT);
            ret = object;
            ret.data->type = STRING_OBJECT;
            break;
    }
    ret = alloc_object(ISandBox, STRING_OBJECT);
    ret = object;*/

    return object.data->u.object.object;
}

ISandBox_ObjectRef
ISandBox_cast_object_to_class(ISandBox_VirtualMachine *ISandBox,
                       ISandBox_ObjectRef object)
{
    /*ISandBox_ObjectRef ret;*/

    /*switch (toType) {
        case ISandBox_STRING_TYPE:
            ret = alloc_object(ISandBox, OBJECT);
            ret = object;
            ret.data->type = STRING_OBJECT;
            break;
        case ISandBox_INT_TYPE:
            ret = alloc_object(ISandBox, OBJECT);
            ret = object;
            ret.data->type = STRING_OBJECT;
            break;
    }*/
    /*ret = alloc_object(ISandBox, CLASS_OBJECT);*/
    /*ret = object.data->u.object.object;*/

    return object.data->u.object.object;
}

ISandBox_ObjectRef
ISandBox_cast_object_to_delegate(ISandBox_VirtualMachine *ISandBox,
                       ISandBox_ObjectRef object)
{
    /*ISandBox_ObjectRef ret;

    ret = object.data->u.object.object;*/

    return object.data->u.object.object;
}

ISandBox_ObjectRef
ISandBox_cast_object_to_native_pointer(ISandBox_VirtualMachine *ISandBox,
                       ISandBox_ObjectRef object)
{
    /*ISandBox_ObjectRef ret;

    ret = object.data->u.object.object;*/

    return object.data->u.object.object;
}

/* newly added */
ISandBox_ObjectRef
ISandBox_create_object_i(ISandBox_VirtualMachine *ISandBox,
                        ISandBox_Value object, int from_type)
{
	ISandBox_ObjectRef ret;
	ret = alloc_object(ISandBox, OBJECT);
	ret.data->u.object = object;
	ret.data->object_type = from_type;

    /*ISandBox_ObjectRef ret;

    ret = alloc_object(ISandBox, OBJECT);
    ret.data->u.object = object;
    ret.v_table = NULL;*/

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                          ISandBox_Value object, int from_type)
{
    ISandBox_ObjectRef ret;

    ret = ISandBox_create_object_i(ISandBox, object, from_type);
    add_ref_in_native_method(context, &ret);

    return ret;
}


static ISandBox_ObjectRef
create_native_pointer_i(ISandBox_VirtualMachine *ISandBox, void *pointer,
			ISandBox_NativePointerInfo *info)
{
    ISandBox_ObjectRef ret;

    ret = alloc_object(ISandBox, NATIVE_POINTER_OBJECT);
    ret.data->u.native_pointer.pointer = pointer;
    ret.data->u.native_pointer.info = info;
    ret.v_table = NULL;

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_native_pointer(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                          void *pointer, ISandBox_NativePointerInfo *info)
{
    ISandBox_ObjectRef ret;

    ret = create_native_pointer_i(ISandBox, pointer, info);
    add_ref_in_native_method(context, &ret);

    return ret;
}

ISandBox_ObjectRef
ISandBox_create_delegate(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef object,
                    int index)
{
    ISandBox_ObjectRef ret;

    ret = alloc_object(ISandBox, DELEGATE_OBJECT);
    ret.data->u.delegate.object = object;
    ret.data->u.delegate.index = index;
    ret.v_table = NULL;

    return ret;
}

static ISandBox_Boolean
is_reference_type(ISandBox_TypeSpecifier *type)
{
    if (((type->basic_type == ISandBox_STRING_TYPE
          ||type->basic_type == ISandBox_CLASS_TYPE
          || type->basic_type == ISandBox_DELEGATE_TYPE
          || type->basic_type == ISandBox_NATIVE_POINTER_TYPE)
         && type->derive_count == 0)
        || (type->derive_count > 0
            && type->derive[0].tag == ISandBox_ARRAY_DERIVE)) {
        return ISandBox_TRUE;
    }
    return ISandBox_FALSE;
}

static void
gc_mark(ISandBox_ObjectRef *ref)
{
    int i;

    if (ref->data == NULL)
        return;

    if (ref->data->marked)
        return;

    ref->data->marked = ISandBox_TRUE;

    if (ref->data->type == OBJECT) {
        gc_mark(&ref->data->u.object.object);
    } else if (ref->data->type == ARRAY_OBJECT
        && ref->data->u.array.type == OBJECT_ARRAY) {
        for (i = 0; i < ref->data->u.array.size; i++) {
            gc_mark(&ref->data->u.array.u.object[i]);
        }
    } else if (ref->data->type == CLASS_OBJECT) {
        ExecClass *ec = ref->v_table->exec_class;
        for (i = 0; i < ec->field_count; i++) {
#if 0
            if (ec->field_type[i]->basic_type == ISandBox_STRING_TYPE
                || ec->field_type[i]->basic_type == ISandBox_CLASS_TYPE
                || (ec->field_type[i]->derive_count > 0
                    && ec->field_type[i]->derive[0].tag == ISandBox_ARRAY_DERIVE)) {
#endif
            if (is_reference_type(ec->field_type[i])) {
                gc_mark(&ref->data->u.class_object.field[i].object);
            }
        }
    } else if (ref->data->type == DELEGATE_OBJECT) {
        gc_mark(&ref->data->u.delegate.object);
    }
}

static void
gc_mark_ref_in_native_method(ISandBox_Context *context)
{
    RefInNativeFunc *ref;

    if (context == NULL)
        return;

    for (ref = context->ref_in_native_method; ref; ref = ref->next) {
        gc_mark(&ref->object);
    }
}

static void
gc_reset_mark(ISandBox_Object *obj)
{
    obj->marked = ISandBox_FALSE;
}

static void
gc_mark_objects(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Object *obj;
    ExecutableEntry *ee_pos;
    int i;
    ISandBox_Context *context_pos;

    for (obj = ISandBox->heap.header; obj; obj = obj->next) {
        gc_reset_mark(obj);
    }

    for (ee_pos = ISandBox->executable_entry; ee_pos; ee_pos = ee_pos->next) {
        for (i = 0; i < ee_pos->static_v.variable_count; i++) {
            if (is_reference_type(ee_pos->executable->global_variable[i].type)) {
                gc_mark(&ee_pos->static_v.variable[i].object);
            }
            /*gc_mark(&ee_pos->static_v.variable[i].object);*/
        }
    }

    for (i = 0; i < ISandBox->stack.stack_pointer; i++) {
        if (ISandBox->stack.pointer_flags[i]) {
            gc_mark(&ISandBox->stack.stack[i].object);
        }
    }
	/*for (i = 0; i < ISandBox->constant_count; i++) {
        gc_mark(&ISandBox->constant[i]->value.object);
    }*/
    gc_mark(&ISandBox->current_exception);
    for (context_pos = ISandBox->current_context; context_pos;
         context_pos = context_pos->next) {
        gc_mark_ref_in_native_method(context_pos);
    }
    for (context_pos = ISandBox->free_context; context_pos;
         context_pos = context_pos->next) {
        gc_mark_ref_in_native_method(context_pos);
    }
}

static ISandBox_Boolean
gc_dispose_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *obj)
{
    ISandBox_Boolean call_finalizer = ISandBox_FALSE;

    switch (obj->type) {
    case OBJECT:/* !!!unstable!!! */
        /*ISandBox->heap.current_heap_size
            -= sizeof(obj->u.object.object.data);*/
		/*ISandBox->heap.current_heap_size
            -= sizeof(ISandBox_Value);
        MEM_free(&obj->u.object);*/
        break;
    case STRING_OBJECT:
        if (!obj->u.string.is_literal) {
            ISandBox->heap.current_heap_size
                -= sizeof(ISandBox_Char) * (ISandBox_wcslen(obj->u.string.string) + 1);
            MEM_free(obj->u.string.string);
        }
        break;
    case ARRAY_OBJECT:
        switch (obj->u.array.type) {
        case INT_ARRAY:
            ISandBox->heap.current_heap_size
                -= sizeof(int) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.int_array);
            break;
        case DOUBLE_ARRAY:
            ISandBox->heap.current_heap_size
                -= sizeof(double) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.double_array);
            break;
        case LONG_DOUBLE_ARRAY:
            ISandBox->heap.current_heap_size
                -= sizeof(long double) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.long_double_array);
            break;
        case OBJECT_ARRAY:
            ISandBox->heap.current_heap_size
                -= sizeof(ISandBox_Object*) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.object);
            break;
        case FUNCTION_INDEX_ARRAY:
            ISandBox->heap.current_heap_size
                -= sizeof(int) * obj->u.array.alloc_size;
            MEM_free(obj->u.array.u.function_index);
        default:
            DBG_assert(0, ("array.type..%d", obj->u.array.type));
        }
        break;
    case CLASS_OBJECT:/*********************************************************************************************************/
        ISandBox->heap.current_heap_size
            -= sizeof(ISandBox_Value) * obj->u.class_object.field_count;
        MEM_free(obj->u.class_object.field);
        break;
    case NATIVE_POINTER_OBJECT:
        if (obj->u.native_pointer.info->finalizer) {
            obj->u.native_pointer.info->finalizer(ISandBox, obj);
            call_finalizer = ISandBox_TRUE;
        }
        break;
    case DELEGATE_OBJECT:
        break;
    case ITERATOR_OBJECT:
        break;
    case OBJECT_TYPE_COUNT_PLUS_1:
    default:
        DBG_assert(0, ("bad type..%d\n", obj->type));
    }

	ISandBox->heap.current_heap_size -= sizeof(ISandBox_Object);
    MEM_free(obj);

    return call_finalizer;
}

static ISandBox_Boolean
gc_sweep_objects(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Object *obj;
    ISandBox_Object *tmp;
    ISandBox_Boolean call_finalizer = ISandBox_FALSE;

    for (obj = ISandBox->heap.header; obj; ) {
        if (!obj->marked) {
            if (obj->prev) {
                obj->prev->next = obj->next;
            } else {
                ISandBox->heap.header = obj->next;
            }
            if (obj->next) {
                obj->next->prev = obj->prev;
            }
            tmp = obj->next;
            if (gc_dispose_object(ISandBox, obj)) {/* ops...something wrong here... */
                call_finalizer = ISandBox_TRUE;
            }
            obj = tmp;
        } else {
            obj = obj->next;
        }
    }
    return call_finalizer;
}

void
ISandBox_garbage_collect(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Boolean call_finalizer;
    do {
        gc_mark_objects(ISandBox);
        call_finalizer = gc_sweep_objects(ISandBox);
    } while(call_finalizer);
}
