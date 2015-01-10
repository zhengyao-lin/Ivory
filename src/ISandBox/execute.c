#include <math.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

static ISandBox_ObjectRef
chain_string(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef str1, ISandBox_ObjectRef str2)
{
    int result_len;
    ISandBox_Char    *left;
    ISandBox_Char    *right;
    int         left_len;
    int         right_len;
    ISandBox_Char    *result;
    ISandBox_ObjectRef ret;

    if (str1.data == NULL) {
        left = NULL_STRING;
        left_len = ISandBox_wcslen(NULL_STRING);
    } else {
        left = str1.data->u.string.string;
        left_len = str1.data->u.string.length;
    }
    if (str2.data == NULL) {
        right = NULL_STRING;
        right_len = ISandBox_wcslen(NULL_STRING);
    } else {
        right = str2.data->u.string.string;
        right_len = str2.data->u.string.length;
    }
    result_len = left_len + right_len;
    result = MEM_malloc(sizeof(ISandBox_Char) * (result_len + 1));

    ISandBox_wcscpy(result, left);
    ISandBox_wcscat(result, right);

    ret = ISandBox_create_ISandBox_string_i(ISandBox, result);

    return ret;
}

static ISandBox_Context *
create_context(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Context *ret;

    ret = MEM_malloc(sizeof(ISandBox_Context));
    ret->ref_in_native_method = NULL;

    return ret;
}


ISandBox_Context *
ISandBox_push_context(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Context *ret;

    ret = create_context(ISandBox);
    ret->next = ISandBox->current_context;
    ISandBox->current_context = ret;

    return ret;
}

ISandBox_Context *
ISandBox_create_context(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Context *ret;

    ret = create_context(ISandBox);
    ret->next = ISandBox->free_context;
    ISandBox->free_context = ret;

    return ret;
}

static void
dispose_context(ISandBox_Context *context)
{
    RefInNativeFunc *pos;
    RefInNativeFunc *temp;

    for (pos = context->ref_in_native_method; pos; ) {
        temp = pos->next;
        MEM_free(pos);
        pos = temp;
    }
    MEM_free(context);
}

void
ISandBox_pop_context(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context)
{
    DBG_assert(ISandBox->current_context == context,
               ("context is not current context."
                "current_context..%p, context..%p",
                ISandBox->current_context, context));

    ISandBox->current_context = context->next;
    dispose_context(context);
}

void
ISandBox_dispose_context(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context)
{
    ISandBox_Context *prev = NULL;
    ISandBox_Context *pos;

    for (pos = ISandBox->free_context; pos; pos = pos->next) {
        if (pos == context)
            break;
        prev = pos;
    }
    if (prev == NULL) {
        ISandBox->free_context = pos->next;
    } else {
        prev->next = pos->next;
    }
    dispose_context(pos);
}

static void
invoke_native_function(ISandBox_VirtualMachine *ISandBox,
                       Function *caller, Function *callee,
                       int pc, int *sp_p, int base)
{
    ISandBox_Value   *stack;
    ISandBox_Value   ret;
    int         arg_count;
    ISandBox_Context *context;
    CallInfo *call_info;
    int i;

    (*sp_p)--; /* function index */
    stack = ISandBox->stack.stack;
    DBG_assert(callee->kind == NATIVE_FUNCTION,
               ("callee->kind..%d", callee->kind));

    if (callee->u.native_f.is_method) {
        arg_count = callee->u.native_f.arg_count + 1;
    } else {
        arg_count = callee->u.native_f.arg_count;
    }
    context = ISandBox_push_context(ISandBox);

    call_info = (CallInfo*)&ISandBox->stack.stack[*sp_p];
    call_info->caller = caller;
    call_info->caller_address = pc;
    call_info->base = base;
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        ISandBox->stack.pointer_flags[*sp_p+i] = ISandBox_FALSE;
    }
    *sp_p += CALL_INFO_ALIGN_SIZE;
    ISandBox->current_function = callee;
    ret = callee->u.native_f.proc(ISandBox, context,
                                  callee->u.native_f.arg_count,
                                  &stack[*sp_p - arg_count
                                         - CALL_INFO_ALIGN_SIZE]);
    ISandBox->current_function = caller;
    *sp_p -= arg_count + CALL_INFO_ALIGN_SIZE;
    stack[*sp_p] = ret;
    ISandBox->stack.pointer_flags[*sp_p] = callee->u.native_f.return_pointer;
    (*sp_p)++;
    ISandBox_pop_context(ISandBox, context);
}

static void
initialize_local_variables(ISandBox_VirtualMachine *ISandBox,
                           ISandBox_Function *func, int from_sp)
{
    int i;
    int sp_idx;

    for (i = 0, sp_idx = from_sp; i < func->local_variable_count;
         i++, sp_idx++) {
        ISandBox->stack.pointer_flags[sp_idx] = ISandBox_FALSE;
    }

    for (i = 0, sp_idx = from_sp; i < func->local_variable_count;
         i++, sp_idx++) {
        ISandBox_initialize_value(func->local_variable[i].type,
                             &ISandBox->stack.stack[sp_idx]);
        if (is_pointer_type(func->local_variable[i].type)) {
            ISandBox->stack.pointer_flags[sp_idx] = ISandBox_TRUE;
        }
    }
}

void
ISandBox_expand_stack(ISandBox_VirtualMachine *ISandBox, int need_stack_size)
{
    int revalue_up;
    int rest;

    rest = ISandBox->stack.alloc_size - ISandBox->stack.stack_pointer;
    if (rest <= need_stack_size) {
        revalue_up = ((rest / STACK_ALLOC_SIZE) + 1) * STACK_ALLOC_SIZE;

        ISandBox->stack.alloc_size += revalue_up;
        ISandBox->stack.stack
            = MEM_realloc(ISandBox->stack.stack,
                          ISandBox->stack.alloc_size * sizeof(ISandBox_Value));
        ISandBox->stack.pointer_flags
            = MEM_realloc(ISandBox->stack.pointer_flags,
                          ISandBox->stack.alloc_size * sizeof(ISandBox_Boolean));
    }
}

static void
invoke_Ivory_function(ISandBox_VirtualMachine *ISandBox,
                       Function **caller_p, Function *callee,
                       ISandBox_Byte **code_p, int *code_size_p, int *pc_p,
                       int *sp_p, int *base_p,
                       ExecutableEntry **ee_p, ISandBox_Executable **exe_p)
{
    CallInfo *call_info;
    ISandBox_Function *callee_p;
    int i;

    if (!callee->is_implemented) {
        ISandBox_dynamic_load(ISandBox, *exe_p, *caller_p, *pc_p, callee);
    }
    *ee_p = callee->u.Ivory_f.executable;
    *exe_p = (*ee_p)->executable;
    callee_p = &(*exe_p)->function[callee->u.Ivory_f.index];

    ISandBox_expand_stack(ISandBox,
                     CALL_INFO_ALIGN_SIZE
                     + callee_p->local_variable_count
                     + (*exe_p)->function[callee->u.Ivory_f.index]
                     .code_block.need_stack_size);

    call_info = (CallInfo*)&ISandBox->stack.stack[*sp_p-1];
    call_info->caller = *caller_p;
    call_info->caller_address = *pc_p;
    call_info->base = *base_p;
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        ISandBox->stack.pointer_flags[*sp_p-1+i] = ISandBox_FALSE;
    }

    *base_p = *sp_p - callee_p->parameter_count - 1;
    if (callee_p->is_method) {
        (*base_p)--; /* for this */
    }
    *caller_p = callee;

    initialize_local_variables(ISandBox, callee_p,
                               *sp_p + CALL_INFO_ALIGN_SIZE - 1);

    *sp_p += CALL_INFO_ALIGN_SIZE + callee_p->local_variable_count - 1;
    *pc_p = 0;

    *code_p = (*exe_p)->function[callee->u.Ivory_f.index].code_block.code;
    *code_size_p
        = (*exe_p)->function[callee->u.Ivory_f.index].code_block.code_size;
}

/* This function returns ISandBox_TRUE if this function was called from native.
 */
static ISandBox_Boolean
do_return(ISandBox_VirtualMachine *ISandBox, Function **func_p,
          ISandBox_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
          ExecutableEntry **ee_p, ISandBox_Executable **exe_p)
{
    CallInfo *call_info;
    ISandBox_Function *caller_p;
    ISandBox_Function *callee_p;
    int arg_count;

    callee_p = &(*exe_p)->function[(*func_p)->u.Ivory_f.index];

    arg_count = callee_p->parameter_count;
    if (callee_p->is_method) {
        arg_count++; /* for this */
    }
    call_info = (CallInfo*)&ISandBox->stack.stack[*base_p + arg_count];

    if (call_info->caller) {
        *ee_p = call_info->caller->u.Ivory_f.executable;
        *exe_p = (*ee_p)->executable;
        if (call_info->caller->kind == Ivory_FUNCTION) {
            caller_p
                = &(*exe_p)->function[call_info->caller->u.Ivory_f.index];
            *code_p = caller_p->code_block.code;
            *code_size_p = caller_p->code_block.code_size;
        }
    } else {
        *ee_p = ISandBox->top_level;
        *exe_p = ISandBox->top_level->executable;
        *code_p = ISandBox->top_level->executable->top_level.code;
        *code_size_p = ISandBox->top_level->executable->top_level.code_size;
    }
    *func_p = call_info->caller;

    ISandBox->stack.stack_pointer = *base_p;
    *pc_p = call_info->caller_address + 1;
    *base_p = call_info->base;

    return call_info->caller_address == CALL_FROM_NATIVE;
}

/* This function returns ISandBox_TRUE if this function was called from native.
 */
static ISandBox_Boolean
return_function(ISandBox_VirtualMachine *ISandBox, Function **func_p,
                ISandBox_Byte **code_p, int *code_size_p, int *pc_p, int *base_p,
                ExecutableEntry **ee_p, ISandBox_Executable **exe_p)
{
    ISandBox_Value return_value;
    ISandBox_Boolean ret;
    ISandBox_Function *callee_func;

    return_value = ISandBox->stack.stack[ISandBox->stack.stack_pointer-1];
    ISandBox->stack.stack_pointer--;
    callee_func = &(*exe_p)->function[(*func_p)->u.Ivory_f.index];

    ret = do_return(ISandBox, func_p, code_p, code_size_p, pc_p, base_p,
                    ee_p, exe_p);

    ISandBox->stack.stack[ISandBox->stack.stack_pointer] = return_value;
    ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer]
        = is_pointer_type(callee_func->type);
    ISandBox->stack.stack_pointer++;

    return ret;
}

#define ST(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)])

#define STI(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].int_value)
#define STD(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].double_value)
#define STLD(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].long_double_value)
#define STO(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].object)
#define STOD(ISandBox, sp) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].object.data)

#define STI_I(ISandBox, sp) \
  ((ISandBox)->stack.stack[(sp)].int_value)
#define STD_I(ISandBox, sp) \
  ((ISandBox)->stack.stack[(sp)].double_value)
#define STLD_I(ISandBox, sp) \
  ((ISandBox)->stack.stack[(sp)].long_double_value)
#define STO_I(ISandBox, sp) \
  ((ISandBox)->stack.stack[(sp)].object)

#define STI_WRITE(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].int_value = r,\
   (ISandBox)->stack.pointer_flags[(ISandBox)->stack.stack_pointer+(sp)] = ISandBox_FALSE)
#define STD_WRITE(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].double_value = r, \
   (ISandBox)->stack.pointer_flags[(ISandBox)->stack.stack_pointer+(sp)] = ISandBox_FALSE)
#define STLD_WRITE(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].long_double_value = r, \
   (ISandBox)->stack.pointer_flags[(ISandBox)->stack.stack_pointer+(sp)] = ISandBox_FALSE)
#define STO_WRITE(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(ISandBox)->stack.stack_pointer+(sp)].object = r, \
   (ISandBox)->stack.pointer_flags[(ISandBox)->stack.stack_pointer+(sp)] = ISandBox_TRUE)

#define STI_WRITE_I(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(sp)].int_value = r,\
   (ISandBox)->stack.pointer_flags[(sp)] = ISandBox_FALSE)
#define STD_WRITE_I(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(sp)].double_value = r, \
   (ISandBox)->stack.pointer_flags[(sp)] = ISandBox_FALSE)
#define STLD_WRITE_I(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(sp)].long_double_value = r, \
   (ISandBox)->stack.pointer_flags[(sp)] = ISandBox_FALSE)
#define STO_WRITE_I(ISandBox, sp, r) \
  ((ISandBox)->stack.stack[(sp)].object = r, \
   (ISandBox)->stack.pointer_flags[(sp)] = ISandBox_TRUE)

ISandBox_ObjectRef
create_array_sub(ISandBox_VirtualMachine *ISandBox, int dim, int dim_index,
                 ISandBox_TypeSpecifier *type)
{
    ISandBox_ObjectRef ret;
    int size;
    int i;
    ISandBox_ObjectRef exception_dummy;

    size = STI(ISandBox, -dim);

    if (dim_index == type->derive_count-1) {
        switch (type->basic_type) {
        case ISandBox_VOID_TYPE:
            DBG_panic(("creating void array"));
            break;
        case ISandBox_BOOLEAN_TYPE: /* FALLTHRU */
        case ISandBox_INT_TYPE:
        case ISandBox_ENUM_TYPE:
            ret = ISandBox_create_array_int_i(ISandBox, size);
            break;
        case ISandBox_DOUBLE_TYPE:
            ret = ISandBox_create_array_double_i(ISandBox, size);
            break;
        case ISandBox_LONG_DOUBLE_TYPE:
            ret = ISandBox_create_array_long_double_i(ISandBox, size);
            break;
		case ISandBox_OBJECT_TYPE: /* FALLTHRU */
		case ISandBox_ITERATOR_TYPE: /* FALLTHRU */
        case ISandBox_STRING_TYPE: /* FALLTHRU */
        case ISandBox_NATIVE_POINTER_TYPE:
        case ISandBox_CLASS_TYPE:
        case ISandBox_DELEGATE_TYPE:
            ret = ISandBox_create_array_object_i(ISandBox, size);
            break;
        case ISandBox_NULL_TYPE: /* FALLTHRU */
        case ISandBox_BASE_TYPE: /* FALLTHRU */
        case ISandBox_UNSPECIFIED_IDENTIFIER_TYPE: /* FALLTHRU */
        default:
            DBG_assert(0, ("type->basic_type..%d\n", type->basic_type));
            break;
        }
    } else if (type->derive[dim_index].tag == ISandBox_FUNCTION_DERIVE) {
        DBG_panic(("Function type in array literal.\n"));
    } else {
        ret = ISandBox_create_array_object_i(ISandBox, size);
        if (dim_index < dim - 1) {
            STO_WRITE(ISandBox, 0, ret);
            ISandBox->stack.stack_pointer++;
            for (i = 0; i < size; i++) {
                ISandBox_ObjectRef child;
                child = create_array_sub(ISandBox, dim, dim_index+1, type);
                ISandBox_array_set_object(ISandBox, ret, i, child, &exception_dummy);
            }
            ISandBox->stack.stack_pointer--;
        }
    }
    return ret;
}

ISandBox_ObjectRef
create_array(ISandBox_VirtualMachine *ISandBox, int dim, ISandBox_TypeSpecifier *type)
{
    return create_array_sub(ISandBox, dim, 0, type);
}

ISandBox_ObjectRef
create_array_literal_int(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef array;
    int i;

    array = ISandBox_create_array_int_i(ISandBox, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.int_array[i] = STI(ISandBox, -size+i);
    }

    return array;
}

ISandBox_ObjectRef
create_array_literal_double(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef array;
    int i;

    array = ISandBox_create_array_double_i(ISandBox, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.double_array[i] = STD(ISandBox, -size+i);
    }

    return array;
}

ISandBox_ObjectRef
create_array_literal_long_double(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef array;
    int i;

    array = ISandBox_create_array_long_double_i(ISandBox, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.long_double_array[i] = STLD(ISandBox, -size+i);
    }

    return array;
}

ISandBox_ObjectRef
create_array_literal_object(ISandBox_VirtualMachine *ISandBox, int size)
{
    ISandBox_ObjectRef array;
    int i;

    array = ISandBox_create_array_object_i(ISandBox, size);
    for (i = 0; i < size; i++) {
        array.data->u.array.u.object[i] = STO(ISandBox, -size+i);
    }

    return array;
}

static void
restore_pc(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee,
           Function *func, int pc)
{
    ISandBox->current_executable = ee;
    ISandBox->current_function = func;
    ISandBox->pc = pc;
}

#define is_null_pointer(obj) (((obj)->data == NULL))

static ISandBox_Boolean
check_instanceof_i(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef *obj,
                   int target_idx,
                   ISandBox_Boolean *is_interface, int *interface_idx)
{
    ExecClass *pos;
    int i;

    for (pos = obj->v_table->exec_class->super_class; pos;
         pos = pos->super_class) {
        if (pos->class_index == target_idx) {
            *is_interface = ISandBox_FALSE;
            return ISandBox_TRUE;
        }
    }

    for (i = 0; i < obj->v_table->exec_class->interface_count; i++) {
        if (obj->v_table->exec_class->interface[i]->class_index
            == target_idx) {
            *is_interface = ISandBox_TRUE;
            *interface_idx = i;
            return ISandBox_TRUE;
        }
    }
    return ISandBox_FALSE;
}

static ISandBox_Boolean
check_instanceof(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef *obj,
                 int target_idx)
{
    ISandBox_Boolean is_interface_dummy;
    int interface_idx_dummy;

    return check_instanceof_i(ISandBox, obj, target_idx,
                              &is_interface_dummy, &interface_idx_dummy);
}

static ISandBox_ErrorStatus
check_down_cast(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef *obj, int target_idx,
                ISandBox_Boolean *is_same_class,
                ISandBox_Boolean *is_interface, int *interface_index)
{
    if (obj->v_table->exec_class->class_index == target_idx) {
        *is_same_class = ISandBox_TRUE;
        return ISandBox_SUCCESS;
    }
    *is_same_class = ISandBox_FALSE;

    if (!check_instanceof_i(ISandBox, obj, target_idx,
                            is_interface, interface_index)) {
        return ISandBox_ERROR;
    }

    return ISandBox_SUCCESS;
}

static void
reset_stack_pointer(ISandBox_Function *ISandBox_func, int *sp_p, int base)
{
    if (ISandBox_func) {
        *sp_p = base + ISandBox_func->parameter_count
            + (ISandBox_func->is_method ? 1 : 0)
            + CALL_INFO_ALIGN_SIZE
            + ISandBox_func->local_variable_count;
    } else {
        *sp_p = 0;
    }
}

/* This function returns ISandBox_TRUE, if exception happen in try or catch clause.
 */
static ISandBox_Boolean
throw_in_try(ISandBox_VirtualMachine *ISandBox,
             ISandBox_Executable *exe, ExecutableEntry *ee, Function *func,
             int *pc_p, int *sp_p, int base)
{
    ISandBox_CodeBlock *cb;
    int try_idx;
    int catch_idx;
    ISandBox_Boolean throw_in_try = ISandBox_FALSE;
    ISandBox_Boolean throw_in_catch = ISandBox_FALSE;
    int exception_idx;
    ISandBox_Function *ISandBox_func = NULL;

    if (func) {
        cb = &(func->u.Ivory_f.executable->executable->function
               [func->u.Ivory_f.index]).code_block;
        ISandBox_func = &(func->u.Ivory_f.executable->executable->function
                     [func->u.Ivory_f.index]);
    } else {
        cb = &exe->top_level;
    }

    exception_idx = ISandBox->current_exception.v_table->exec_class->class_index;

    for (try_idx = 0; try_idx < cb->try_size; try_idx++) {
        if ((*pc_p) >= cb->try[try_idx].try_start_pc
            && (*pc_p) <= cb->try[try_idx].try_end_pc) {
            throw_in_try = ISandBox_TRUE;
            break;
        }
        for (catch_idx = 0; catch_idx < cb->try[try_idx].catch_count;
             catch_idx++) {
            if ((*pc_p) >= cb->try[try_idx].catch_clause[catch_idx].start_pc
                && ((*pc_p)
                    <= cb->try[try_idx].catch_clause[catch_idx].end_pc)) {
                throw_in_catch = ISandBox_TRUE;
                break;
            }
        }
    }

    if (try_idx == cb->try_size)
        return ISandBox_FALSE;

    DBG_assert(throw_in_try || throw_in_catch, ("bad flags"));

    if (throw_in_try) {
        for (catch_idx = 0; catch_idx < cb->try[try_idx].catch_count;
             catch_idx++) {
            int class_idx_in_exe
                = cb->try[try_idx].catch_clause[catch_idx].class_index;
            int class_idx_in_ISandBox = ee->class_table[class_idx_in_exe];
            if (exception_idx == class_idx_in_ISandBox
                || check_instanceof(ISandBox, &ISandBox->current_exception,
                                    class_idx_in_ISandBox)) {
                *pc_p = cb->try[try_idx].catch_clause[catch_idx].start_pc;
                reset_stack_pointer(ISandBox_func, sp_p, base);
                STO_WRITE(ISandBox, 0, ISandBox->current_exception);
                ISandBox->stack.stack_pointer++;
                ISandBox->current_exception = ISandBox_null_object_ref;
                return ISandBox_TRUE;
            }
        }
    }
    *pc_p = cb->try[try_idx].finally_start_pc;
    reset_stack_pointer(ISandBox_func, sp_p, base);
    
    return ISandBox_TRUE;
}

static void
add_stack_trace(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe,
                Function *func, int pc)
{
    int line_number;
    int class_index;
    ISandBox_ObjectRef stack_trace;
    int stack_trace_index;
    ISandBox_Object *stack_trace_array;
    int array_size;
    int line_number_index;
    int file_name_index;
    ISandBox_Value value;
    ISandBox_Char *wc_str;
    char *func_name;
    int func_name_index;

    line_number = ISandBox_conv_pc_to_line_number(exe, func, pc);
    class_index = ISandBox_search_class(ISandBox,
                                   ISandBox_Ivory_DEFAULT_PACKAGE,
                                   Ivory_STACK_TRACE_CLASS);
    stack_trace = ISandBox_create_class_object_i(ISandBox, class_index);
    STO_WRITE(ISandBox, 0, stack_trace);
    ISandBox->stack.stack_pointer++;
    
    line_number_index
        = ISandBox_get_field_index(ISandBox, stack_trace, "line_number");
    stack_trace.data->u.class_object.field[line_number_index].int_value
        = line_number;

    file_name_index
        = ISandBox_get_field_index(ISandBox, stack_trace, "file_name");
    wc_str = ISandBox_mbstowcs_alloc(ISandBox, exe->path);
    stack_trace.data->u.class_object.field[file_name_index].object
        = ISandBox_create_ISandBox_string_i(ISandBox, wc_str);

    func_name_index
        = ISandBox_get_field_index(ISandBox, stack_trace, "function_name");
    if (func) {
        func_name = exe->function[func->u.Ivory_f.index].name;
    } else {
        func_name = "top level";
    }
    wc_str = ISandBox_mbstowcs_alloc(ISandBox, func_name);
    stack_trace.data->u.class_object.field[func_name_index].object
        = ISandBox_create_ISandBox_string_i(ISandBox, wc_str);

    stack_trace_index
        = ISandBox_get_field_index(ISandBox, ISandBox->current_exception,
                              "stack_trace");
    stack_trace_array = ISandBox->current_exception.data
        ->u.class_object.field[stack_trace_index].object.data;
    array_size = ISandBox_array_size(ISandBox, stack_trace_array);
    value.object = stack_trace;
    ISandBox_array_insert(ISandBox, stack_trace_array, array_size, value);

    ISandBox->stack.stack_pointer--;
}

ISandBox_Value ISandBox_execute_i(ISandBox_VirtualMachine *ISandBox, Function *func,
                        ISandBox_Byte *code, int code_size, int base);

static ISandBox_Value
invoke_Ivory_function_from_native(ISandBox_VirtualMachine *ISandBox,
                                   Function *callee, ISandBox_ObjectRef obj,
                                   ISandBox_Value *args)
{
    int base;
    int i;
    CallInfo *call_info;
    ISandBox_Executable *ISandBox_exe;
    ISandBox_Function *ISandBox_func;
    ExecutableEntry *current_executable_backup;
    Function *current_function_backup;
    int current_pc_backup;
    ISandBox_Byte *code;
    int code_size;
    ISandBox_Value ret;

    current_executable_backup = ISandBox->current_executable;
    current_function_backup = ISandBox->current_function;
    current_pc_backup = ISandBox->pc;

    ISandBox_exe = callee->u.Ivory_f.executable->executable;
    ISandBox_func = &ISandBox_exe->function[callee->u.Ivory_f.index];

    base = ISandBox->stack.stack_pointer;
    for (i = 0; i < ISandBox_func->parameter_count; i++) {
        ISandBox->stack.stack[ISandBox->stack.stack_pointer] = args[i];
        ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer]
            = is_pointer_type(ISandBox_func->parameter[i].type);
        ISandBox->stack.stack_pointer++;
    }
    if (!is_null_pointer(&obj)) {
        STO_WRITE(ISandBox, 0, obj);
        ISandBox->stack.stack_pointer++;
    }
    call_info = (CallInfo*)&ISandBox->stack.stack[ISandBox->stack.stack_pointer];
    call_info->caller = ISandBox->current_function;
    call_info->caller_address = CALL_FROM_NATIVE;
    call_info->base = 0; /* dummy */
    for (i = 0; i < CALL_INFO_ALIGN_SIZE; i++) {
        ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer + i] = ISandBox_FALSE;
        ISandBox->pc = 0;
        ISandBox->current_executable = callee->u.Ivory_f.executable;
    }
    ISandBox->stack.stack_pointer += CALL_INFO_ALIGN_SIZE;
    initialize_local_variables(ISandBox, ISandBox_func, ISandBox->stack.stack_pointer);
    ISandBox->stack.stack_pointer += ISandBox_func->local_variable_count;
    code = ISandBox_exe->function[callee->u.Ivory_f.index].code_block.code;
    code_size = ISandBox_exe->function[callee->u.Ivory_f.index]
        .code_block.code_size;

    ret = ISandBox_execute_i(ISandBox, callee, code, code_size, base);
    ISandBox->stack.stack_pointer--;

    current_executable_backup = ISandBox->current_executable;
    current_function_backup = ISandBox->current_function;
    current_pc_backup = ISandBox->pc;

    return ret;
}

/* This function returns ISandBox_TRUE if this function was called from native.
 */
static ISandBox_Boolean
do_throw(ISandBox_VirtualMachine *ISandBox,
         Function **func_p, ISandBox_Byte **code_p, int *code_size_p, int *pc_p,
         int *base_p, ExecutableEntry **ee_p, ISandBox_Executable **exe_p,
         ISandBox_ObjectRef *exception)
{
    ISandBox_Boolean in_try;

    ISandBox->current_exception = *exception;

    for (;;) {
        in_try = throw_in_try(ISandBox, *exe_p, *ee_p, *func_p, pc_p,
                              &ISandBox->stack.stack_pointer, *base_p);
        if (in_try)
            break;

        if (*func_p) {
            add_stack_trace(ISandBox, *exe_p, *func_p, *pc_p);
            if (do_return(ISandBox, func_p, code_p, code_size_p, pc_p,
                          base_p, ee_p, exe_p)) {
                return ISandBox_TRUE;
            }
        } else {
            int func_index
                = ISandBox_search_function(ISandBox,
                                      ISandBox_Ivory_DEFAULT_PACKAGE,
                                      Ivory_PRINT_STACK_TRACE_FUNC);
            add_stack_trace(ISandBox, *exe_p, *func_p, *pc_p);

            invoke_Ivory_function_from_native(ISandBox, ISandBox->function[func_index],
                                               ISandBox->current_exception, NULL);
            exit(1);
        }
    }
    return ISandBox_FALSE;
}

ISandBox_ObjectRef
ISandBox_create_exception(ISandBox_VirtualMachine *ISandBox, char *class_name,
                     RuntimeError id, ...)
{
    int class_index;
    ISandBox_ObjectRef obj;
    VString     message;
    va_list     ap;
    int message_index;
    int stack_trace_index;

    va_start(ap, id);
    class_index = ISandBox_search_class(ISandBox, ISandBox_Ivory_DEFAULT_PACKAGE,
                                   class_name);
    obj = ISandBox_create_class_object_i(ISandBox, class_index);

    STO_WRITE(ISandBox, 0, obj);
    ISandBox->stack.stack_pointer++;

    ISandBox_format_message(ISandBox_error_message_format, (int)id, &message, ap);
    va_end(ap);

    message_index
        = ISandBox_get_field_index(ISandBox, obj, "message");
    obj.data->u.class_object.field[message_index].object
        = ISandBox_create_ISandBox_string_i(ISandBox, message.string);

    stack_trace_index
        = ISandBox_get_field_index(ISandBox, obj, "stack_trace");
    obj.data->u.class_object.field[stack_trace_index].object
        = ISandBox_create_array_object_i(ISandBox, 0);

    ISandBox->stack.stack_pointer--;

    return obj;
}

/* This function returns ISandBox_TRUE if this function was called from native.
 */
static ISandBox_Boolean
throw_null_pointer_exception(ISandBox_VirtualMachine *ISandBox, Function **func_p,
                             ISandBox_Byte **code_p, int *code_size_p, int *pc_p,
                             int *base_p,
                             ExecutableEntry **ee_p, ISandBox_Executable **exe_p)
{
    ISandBox_ObjectRef ex;

    ex = ISandBox_create_exception(ISandBox, ISandBox_NULL_POINTER_EXCEPTION_NAME,
                              NULL_POINTER_ERR, ISandBox_MESSAGE_ARGUMENT_END);
    STO_WRITE(ISandBox, 0, ex); /* BUGBUG? irane? */
    ISandBox->stack.stack_pointer++;
    return do_throw(ISandBox, func_p, code_p, code_size_p, pc_p, base_p,
                    ee_p, exe_p, &ex);
}

static void
clear_stack_trace(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef *ex)
{
    int stack_trace_index
        = ISandBox_get_field_index(ISandBox, *ex, "stack_trace");

    ex->data->u.class_object.field[stack_trace_index].object
        = ISandBox_create_array_object_i(ISandBox, 0);
}

/*****************************************************************************************************************************
typedef struct {
    int label_address;
	int mark;
} Label_i;

typedef struct chain_tag {
    Label_i *label;
	struct chain_tag *next;
} LabelChain;

LabelChain *Label_Chain;

static Label_i *
alloc_label_i(int address, int mark)
{
	Label_i *l;
	l = malloc(sizeof(Label_i));
	l->label_address = address;
	l->mark = mark;
	return l;
}

static void
add_label_chain(Label_i *l)
{
	LabelChain *pos;
	LabelChain *new_item;

	new_item = malloc(sizeof(LabelChain));
	new_item->label = l;
	new_item->next = NULL;

	for (pos = Label_Chain; pos->next; pos = pos->next)
    { }
	pos->next = new_item;
}

static int
get_label_address(int mark)
{
	LabelChain *pos;
	for (pos = Label_Chain; pos; pos = pos->next)
	{
		if (pos->label->mark == mark)	
		{
			return pos->label->label_address;
		}
	}
	return -1;
}
*****************************************************************************************************************************/

ISandBox_Value
ISandBox_execute_i(ISandBox_VirtualMachine *ISandBox, Function *func,
              ISandBox_Byte *code, int code_size, int base)
{
    ExecutableEntry *ee;
    ISandBox_Executable *exe;
    int         pc, i;
    ISandBox_Value   ret;

    pc = ISandBox->pc;
    ee = ISandBox->current_executable;
    exe = ISandBox->current_executable->executable;

    while (pc < code_size) {

        /*
        ISandBox_dump_instruction(stderr, code, pc);
        fprintf(stderr, "\tsp(%d)\n", ISandBox->stack.stack_pointer);
		*/
		/*ISandBox_check_gc(ISandBox);*/
        switch ((ISandBox_Opcode)code[pc]) {
        case ISandBox_PUSH_INT_1BYTE:
            /*printf("%d: ISandBox_PUSH_INT_1BYTE\n", code[pc-1]);*/
            STI_WRITE(ISandBox, 0, code[pc+1]);
            ISandBox->stack.stack_pointer++;
            pc += 2;
            break;
        case ISandBox_PUSH_INT_2BYTE:
            STI_WRITE(ISandBox, 0, GET_2BYTE_INT(&code[pc+1]));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_INT:
            STI_WRITE(ISandBox, 0,
                      exe->constant_pool[GET_2BYTE_INT(&code[pc+1])]
                      .u.c_int);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_DOUBLE_0:
            STD_WRITE(ISandBox, 0, 0.0);
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        case ISandBox_PUSH_DOUBLE_1:
            STD_WRITE(ISandBox, 0, 1.0);
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        case ISandBox_PUSH_DOUBLE:
            STD_WRITE(ISandBox, 0, 
                      exe->constant_pool[GET_2BYTE_INT(&code[pc+1])]
                      .u.c_double);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STRING:
            STO_WRITE(ISandBox, 0,
                      ISandBox_literal_to_ISandBox_string_i(ISandBox,
                                                  exe->constant_pool
                                                  [GET_2BYTE_INT(&code[pc+1])]
                                                  .u.c_string));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_NULL:
            STO_WRITE(ISandBox, 0, ISandBox_null_object_ref);
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        /******************************************************/
        case ISandBox_PUSH_LONG_DOUBLE_0:
            STLD_WRITE(ISandBox, 0, 0.0);
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        case ISandBox_PUSH_LONG_DOUBLE_1:
            STLD_WRITE(ISandBox, 0, 1.0);
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        case ISandBox_PUSH_LONG_DOUBLE:
            STLD_WRITE(ISandBox, 0, 
                      exe->constant_pool[GET_2BYTE_INT(&code[pc+1])]
                      .u.c_long_double);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        /******************************************************/
        case ISandBox_PUSH_STACK_INT:
            STI_WRITE(ISandBox, 0,
                      STI_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1])));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STACK_DOUBLE:
            STD_WRITE(ISandBox, 0,
                      STD_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1])));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STACK_LONG_DOUBLE:
            STLD_WRITE(ISandBox, 0,
                      STLD_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1])));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STACK_OBJECT:
            STO_WRITE(ISandBox, 0,
                      STO_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1])));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_POP_STACK_INT:
            STI_WRITE_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1]),
                        STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STACK_DOUBLE:
            STD_WRITE_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1]),
                        STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STACK_LONG_DOUBLE:
            STLD_WRITE_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1]),
                        STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STACK_OBJECT:
            STO_WRITE_I(ISandBox, base + GET_2BYTE_INT(&code[pc+1]),
                        STO(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_PUSH_STATIC_INT:
            STI_WRITE(ISandBox, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .int_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STATIC_DOUBLE:
            STD_WRITE(ISandBox, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .double_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STATIC_LONG_DOUBLE:
            STLD_WRITE(ISandBox, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .long_double_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_PUSH_STATIC_OBJECT:
            STO_WRITE(ISandBox, 0,
                      ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                      .object);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        case ISandBox_POP_STATIC_INT:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])].int_value
                = STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STATIC_DOUBLE:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                .double_value
                = STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STATIC_LONG_DOUBLE:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])]
                .long_double_value
                = STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        case ISandBox_POP_STATIC_OBJECT:
            ee->static_v.variable[GET_2BYTE_INT(&code[pc+1])].object
                = STO(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        /*case ISandBox_PUSH_CONSTANT_INT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STI_WRITE(ISandBox, 0,
                      ISandBox->constant[ee->constant_table[idx_in_exe]]
                      ->value.int_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_PUSH_CONSTANT_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STD_WRITE(ISandBox, 0,
                      ISandBox->constant[ee->constant_table[idx_in_exe]]
                      ->value.double_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_PUSH_CONSTANT_LONG_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STLD_WRITE(ISandBox, 0,
                      ISandBox->constant[ee->constant_table[idx_in_exe]]
                      ->value.long_double_value);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_PUSH_CONSTANT_OBJECT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STO_WRITE(ISandBox, 0,
                      ISandBox->constant[ee->constant_table[idx_in_exe]]
                      ->value.object);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_POP_CONSTANT_INT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            ISandBox->constant[ee->constant_table[idx_in_exe]]->value.int_value
                = STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case ISandBox_POP_CONSTANT_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            ISandBox->constant[ee->constant_table[idx_in_exe]]->value.double_value
                = STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case ISandBox_POP_CONSTANT_LONG_DOUBLE:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            ISandBox->constant[ee->constant_table[idx_in_exe]]->value.long_double_value
                = STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        }
        case ISandBox_POP_CONSTANT_OBJECT:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            ISandBox->constant[ee->constant_table[idx_in_exe]]->value.object
                = STO(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc += 3;
            break;
        }*/
        case ISandBox_PUSH_ARRAY_INT:
        {
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            int int_value;
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);

            status = ISandBox_array_get_int(ISandBox, array, index,
                                       &int_value, &exception);
            if (status == ISandBox_SUCCESS) {
                STI_WRITE(ISandBox, -2, int_value);
                ISandBox->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_PUSH_ARRAY_DOUBLE:
        {
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            double double_value;
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status = ISandBox_array_get_double(ISandBox, array, index,
                                          &double_value, &exception);
            if (status == ISandBox_SUCCESS) {
                STD_WRITE(ISandBox, -2, double_value);
                ISandBox->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_PUSH_ARRAY_LONG_DOUBLE:
        {
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            long double long_double_value;
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status = ISandBox_array_get_long_double(ISandBox, array, index,
                                          &long_double_value, &exception);
            if (status == ISandBox_SUCCESS) {
                STLD_WRITE(ISandBox, -2, long_double_value);
                ISandBox->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_PUSH_ARRAY_OBJECT:
        {
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ObjectRef object;
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status = ISandBox_array_get_object(ISandBox, array, index,
                                          &object, &exception);
            if (status == ISandBox_SUCCESS) {
                STO_WRITE(ISandBox, -2, object);
                ISandBox->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_POP_ARRAY_INT:
        {
            int value = STI(ISandBox, -3);
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status = ISandBox_array_set_int(ISandBox, array, index, value, &exception);
            if (status == ISandBox_SUCCESS) {
                ISandBox->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_POP_ARRAY_DOUBLE:
        {
            double value = STD(ISandBox, -3);
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status
                = ISandBox_array_set_double(ISandBox, array, index, value, &exception);
            if (status == ISandBox_SUCCESS) {
                ISandBox->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_POP_ARRAY_LONG_DOUBLE:
        {
            long double value = STLD(ISandBox, -3);
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status
                = ISandBox_array_set_long_double(ISandBox, array, index, value, &exception);
            if (status == ISandBox_SUCCESS) {
                ISandBox->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_POP_ARRAY_OBJECT:
        {
            ISandBox_ObjectRef value = STO(ISandBox, -3);
            ISandBox_ObjectRef array = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            restore_pc(ISandBox, ee, func, pc);
            status
                = ISandBox_array_set_object(ISandBox, array, index, value, &exception);
            if (status == ISandBox_SUCCESS) {
                ISandBox->stack.stack_pointer -= 3;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_PUSH_CHARACTER_IN_STRING:
        {
            ISandBox_ObjectRef str = STO(ISandBox, -2);
            int index = STI(ISandBox, -1);
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;
            ISandBox_Char ch;

            restore_pc(ISandBox, ee, func, pc);
            status = ISandBox_string_get_character(ISandBox, str, index,
                                              &ch, &exception);
            if (status == ISandBox_SUCCESS) {
                STI_WRITE(ISandBox, -2, ch);
                ISandBox->stack.stack_pointer--;
                pc++;
            } else {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            }
            break;
        }
        case ISandBox_PUSH_FIELD_INT:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STI_WRITE(ISandBox, -1,
                          obj.data->u.class_object.field[index].int_value);
                pc += 3;
            }
            break;
        }
        case ISandBox_PUSH_FIELD_DOUBLE:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STD_WRITE(ISandBox, -1,
                          obj.data->u.class_object.field[index].double_value);
                pc += 3;
            }
            break;
        }
        case ISandBox_PUSH_FIELD_LONG_DOUBLE:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STLD_WRITE(ISandBox, -1,
                          obj.data->u.class_object.field[index].long_double_value);
                pc += 3;
            }
            break;
        }
        case ISandBox_PUSH_FIELD_OBJECT:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STO_WRITE(ISandBox, -1,
                          obj.data->u.class_object.field[index].object);
                pc += 3;
            }
            break;
        }
        case ISandBox_POP_FIELD_INT:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].int_value
                    = STI(ISandBox, -2);
                ISandBox->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case ISandBox_POP_FIELD_DOUBLE:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].double_value
                    = STD(ISandBox, -2);
                ISandBox->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case ISandBox_POP_FIELD_LONG_DOUBLE:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].long_double_value
                    = STLD(ISandBox, -2);
                ISandBox->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case ISandBox_POP_FIELD_OBJECT:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.data->u.class_object.field[index].object = STO(ISandBox, -2);
                ISandBox->stack.stack_pointer -= 2;
                pc += 3;
            }
            break;
        }
        case ISandBox_ADD_INT:
            STI(ISandBox, -2) = STI(ISandBox, -2) + STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_ADD_DOUBLE:
            STD(ISandBox, -2) = STD(ISandBox, -2) + STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_ADD_LONG_DOUBLE:
            STLD(ISandBox, -2) = STLD(ISandBox, -2) + STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_ADD_STRING:
            STO(ISandBox, -2) = chain_string(ISandBox,
                                        STO(ISandBox, -2),
                                        STO(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_SUB_INT:
            STI(ISandBox, -2) = STI(ISandBox, -2) - STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_SUB_DOUBLE:
            STD(ISandBox, -2) = STD(ISandBox, -2) - STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_SUB_LONG_DOUBLE:
            STLD(ISandBox, -2) = STLD(ISandBox, -2) - STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MUL_INT:
            STI(ISandBox, -2) = STI(ISandBox, -2) * STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MUL_DOUBLE:
            STD(ISandBox, -2) = STD(ISandBox, -2) * STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MUL_LONG_DOUBLE:
            STLD(ISandBox, -2) = STLD(ISandBox, -2) * STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_DIV_INT:
            if (STI(ISandBox, -1) == 0) {
                ISandBox_ObjectRef exception;

                exception
                    = ISandBox_create_exception(ISandBox,
                                           DIVISION_BY_ZERO_EXCEPTION_NAME,
                                           DIVISION_BY_ZERO_ERR,
                                           ISandBox_MESSAGE_ARGUMENT_END);
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &exception)) {
                    goto EXECUTE_END;
                }
            } else {
                STI(ISandBox, -2) = STI(ISandBox, -2) / STI(ISandBox, -1);
                ISandBox->stack.stack_pointer--;
                pc++;
            }
            break;
        case ISandBox_DIV_DOUBLE:
            STD(ISandBox, -2) = STD(ISandBox, -2) / STD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_DIV_LONG_DOUBLE:
            STLD(ISandBox, -2) = STLD(ISandBox, -2) / STLD(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MOD_INT:
            STI(ISandBox, -2) = STI(ISandBox, -2) % STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MOD_DOUBLE:
            STD(ISandBox, -2) = fmod(STD(ISandBox, -2), STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MOD_LONG_DOUBLE:
            STLD(ISandBox, -2) = fmod(STLD(ISandBox, -2), STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_BIT_AND:
            STI(ISandBox, -2) = STI(ISandBox, -2) & STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_BIT_OR:
            STI(ISandBox, -2) = STI(ISandBox, -2) | STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_BIT_XOR:
            STI(ISandBox, -2) = STI(ISandBox, -2) ^ STI(ISandBox, -1);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_MINUS_INT:
            STI(ISandBox, -1) = -STI(ISandBox, -1);
            pc++;
            break;
        case ISandBox_MINUS_DOUBLE:
            STD(ISandBox, -1) = -STD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_MINUS_LONG_DOUBLE:
            STLD(ISandBox, -1) = -STLD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_BIT_NOT:
            STI(ISandBox, -1) = ~STI(ISandBox, -1);
            pc++;
            break;
        case ISandBox_INCREMENT:
            STI(ISandBox, -1)++;
            pc++;
            break;
        case ISandBox_DECREMENT:
            STI(ISandBox, -1)--;
            pc++;
            break;
        case ISandBox_CAST_INT_TO_DOUBLE:
            STD(ISandBox, -1) = (double)STI(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_INT_TO_LONG_DOUBLE:
            STLD(ISandBox, -1) = (long double)STI(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_DOUBLE_TO_INT:
            STI(ISandBox, -1) = (int)STD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_LONG_DOUBLE_TO_INT:
            STI(ISandBox, -1) = (int)STLD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_LONG_DOUBLE_TO_DOUBLE:
            STD(ISandBox, -1) = (double)STLD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_DOUBLE_TO_LONG_DOUBLE:
            STLD(ISandBox, -1) = (long double)STD(ISandBox, -1);
            pc++;
            break;
        case ISandBox_CAST_BOOLEAN_TO_STRING:
            if (STI(ISandBox, -1)) {
                STO_WRITE(ISandBox, -1,
                          ISandBox_literal_to_ISandBox_string_i(ISandBox, TRUE_STRING));
            } else {
                STO_WRITE(ISandBox, -1,
                          ISandBox_literal_to_ISandBox_string_i(ISandBox, FALSE_STRING));
            }
            pc++;
            break;
        case ISandBox_CAST_INT_TO_STRING:
        {
            char buf[LINE_BUF_SIZE];
            ISandBox_Char *wc_str;

            sprintf(buf, "%d", STI(ISandBox, -1));
            restore_pc(ISandBox, ee, func, pc);
            wc_str = ISandBox_mbstowcs_alloc(ISandBox, buf);
            STO_WRITE(ISandBox, -1,
                      ISandBox_create_ISandBox_string_i(ISandBox, wc_str));
            pc++;
            break;
        }
        case ISandBox_CAST_DOUBLE_TO_STRING:
        {
            char buf[LINE_BUF_SIZE];
            ISandBox_Char *wc_str;

            sprintf(buf, "%lf", STD(ISandBox, -1));
            restore_pc(ISandBox, ee, func, pc);
            wc_str = ISandBox_mbstowcs_alloc(ISandBox, buf);
            STO_WRITE(ISandBox, -1,
                      ISandBox_create_ISandBox_string_i(ISandBox, wc_str));
            pc++;
            break;
        }
        case ISandBox_CAST_LONG_DOUBLE_TO_STRING:
        {
            char buf[LINE_BUF_SIZE];
            ISandBox_Char *wc_str;

            sprintf(buf, "%.12Lf", STLD(ISandBox, -1)); /**********************************************/
            restore_pc(ISandBox, ee, func, pc);
            wc_str = ISandBox_mbstowcs_alloc(ISandBox, buf);
            STO_WRITE(ISandBox, -1,
                      ISandBox_create_ISandBox_string_i(ISandBox, wc_str));
            pc++;
            break;
        }
        case ISandBox_CAST_ENUM_TO_STRING:
        {
            ISandBox_Char *wc_str;
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int enum_index = ee->enum_table[idx_in_exe];
            int enum_value = STI(ISandBox, -1);

            restore_pc(ISandBox, ee, func, pc);
            wc_str = ISandBox_mbstowcs_alloc(ISandBox,
                                        ISandBox->enums[enum_index]->ISandBox_enum
                                        ->enumerator[enum_value]);
            STO_WRITE(ISandBox, -1,
                      ISandBox_create_ISandBox_string_i(ISandBox, wc_str));
            pc += 3;
            break;
        }
        case ISandBox_UP_CAST:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                obj.v_table
                    = obj.v_table->exec_class->interface_v_table[index];
                STO_WRITE(ISandBox, -1, obj);
                pc += 3;
            }
            break;
        }
        case ISandBox_DOWN_CAST:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int index = ee->class_table[idx_in_exe];
            ISandBox_Boolean is_same_class;
            ISandBox_Boolean is_interface;
            int interface_idx;
            ISandBox_ErrorStatus status;
            ISandBox_ObjectRef exception;

            do {
                if (is_null_pointer(&obj)) {
                    if (throw_null_pointer_exception(ISandBox, &func,
                                                     &code, &code_size,
                                                     &pc, &base, &ee, &exe)) {
                        goto EXECUTE_END;
                    }
                    break;
                }
                status = check_down_cast(ISandBox, &obj, index,
                                         &is_same_class,
                                         &is_interface, &interface_idx);
                if (status != ISandBox_SUCCESS) {
                    exception
                        = ISandBox_create_exception(ISandBox,
                                               CLASS_CAST_EXCEPTION_NAME,
                                               CLASS_CAST_ERR,
                                               ISandBox_STRING_MESSAGE_ARGUMENT,
                                               "org",
                                               obj.v_table->exec_class->name,
                                               ISandBox_STRING_MESSAGE_ARGUMENT,
                                               "target",
                                               ISandBox->class[index]->name,
                                               ISandBox_MESSAGE_ARGUMENT_END);
                    if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                                 &base, &ee, &exe, &exception)) {
                        goto EXECUTE_END;
                    }
                    break;
                }
                if (!is_same_class) {
                    if (is_interface) {
                        obj.v_table
                            = obj.v_table->exec_class
                            ->interface_v_table[interface_idx];
                    } else {
                        obj.v_table = obj.v_table->exec_class->class_table;
                    }
                }
                STO_WRITE(ISandBox, -1, obj);
                pc += 3;
            } while (0);
            break;
        }
        case ISandBox_EQ_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) == STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_EQ_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) == STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_EQ_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) == STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_EQ_OBJECT:
            STI_WRITE(ISandBox, -2, STO(ISandBox, -2).data == STO(ISandBox, -1).data);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_EQ_STRING:
            STI_WRITE(ISandBox, -2,
                      !ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                  STO(ISandBox, -1).data->u.string.string));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GT_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) > STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GT_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) > STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GT_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) > STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GT_STRING:
            STI_WRITE(ISandBox, -2,
                      ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                 STO(ISandBox, -1).data->u.string.string) > 0);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GE_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) >= STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GE_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) >= STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GE_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) >= STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_GE_STRING:
            STI_WRITE(ISandBox, -2,
                      ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                 STO(ISandBox, -1).data->u.string.string) >= 0);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LT_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) < STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LT_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) < STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LT_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) < STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LT_STRING:
            STI_WRITE(ISandBox, -2,
                      ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                 STO(ISandBox, -1).data->u.string.string) < 0);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LE_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) <= STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LE_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) <= STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LE_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) <= STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LE_STRING:
            STI_WRITE(ISandBox, -2,
                      ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                 STO(ISandBox, -1).data->u.string.string) <= 0);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_NE_INT:
            STI(ISandBox, -2) = (STI(ISandBox, -2) != STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_NE_DOUBLE:
            STI(ISandBox, -2) = (STD(ISandBox, -2) != STD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_NE_LONG_DOUBLE:
            STI(ISandBox, -2) = (STLD(ISandBox, -2) != STLD(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_NE_OBJECT:
            STI_WRITE(ISandBox, -2, STO(ISandBox, -2).data != STO(ISandBox, -1).data);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_NE_STRING:
            STI_WRITE(ISandBox, -2,
                      ISandBox_wcscmp(STO(ISandBox, -2).data->u.string.string,
                                 STO(ISandBox, -1).data->u.string.string) != 0);
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LOGICAL_AND:
            STI(ISandBox, -2) = (STI(ISandBox, -2) && STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LOGICAL_OR:
            STI(ISandBox, -2) = (STI(ISandBox, -2) || STI(ISandBox, -1));
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_LOGICAL_NOT:
            STI(ISandBox, -1) = !STI(ISandBox, -1);
            pc++;
            break;
        case ISandBox_POP:
            ISandBox->stack.stack_pointer--;
            pc++;
            break;
        case ISandBox_DUPLICATE:
            ISandBox->stack.stack[ISandBox->stack.stack_pointer]
                = ISandBox->stack.stack[ISandBox->stack.stack_pointer-1];
            ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer]
                = ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer-1];
            ISandBox->stack.stack_pointer++;
            pc++;
            break;
        case ISandBox_DUPLICATE_OFFSET:
            {
                int offset = GET_2BYTE_INT(&code[pc+1]);
                ISandBox->stack.stack[ISandBox->stack.stack_pointer]
                    = ISandBox->stack.stack[ISandBox->stack.stack_pointer-1-offset];
                ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer]
                    = ISandBox->stack.pointer_flags[ISandBox->stack.stack_pointer-1
                                               -offset];
                ISandBox->stack.stack_pointer++;
                pc += 3;
                break;
            }
        case ISandBox_JUMP:
            pc = GET_2BYTE_INT(&code[pc+1]);
            break;
        case ISandBox_JUMP_IF_TRUE:
            if (STI(ISandBox, -1)) {
                pc = GET_2BYTE_INT(&code[pc+1]);
            } else {
                pc += 3;
            }
            ISandBox->stack.stack_pointer--;
            break;
        case ISandBox_JUMP_IF_FALSE:
            if (!STI(ISandBox, -1)) {
                pc = GET_2BYTE_INT(&code[pc+1]);
            } else {
                pc += 3;
            }
            ISandBox->stack.stack_pointer--;
            break;
        case ISandBox_PUSH_FUNCTION:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            STI_WRITE(ISandBox, 0, ee->function_table[idx_in_exe]);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_PUSH_METHOD:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);

            if (is_null_pointer(&obj)) {
                if (throw_null_pointer_exception(ISandBox, &func, &code, &code_size,
                                                 &pc, &base, &ee, &exe)) {
                    goto EXECUTE_END;
                }
            } else {
                STI_WRITE(ISandBox, 0, obj.v_table->table[index].index);
                ISandBox->stack.stack_pointer++;
                pc += 3;
            }
            break;
        }
        case ISandBox_PUSH_DELEGATE:
        {
            int index = GET_2BYTE_INT(&code[pc+1]);
            int ISandBox_index;
            ISandBox_ObjectRef delegate;

            ISandBox_index = ee->function_table[index];
            delegate = ISandBox_create_delegate(ISandBox, ISandBox_null_object_ref,
                                           ISandBox_index);
            STO_WRITE(ISandBox, 0, delegate);
            ISandBox->stack.stack_pointer++;
            pc += 3;

            break;
        }
        case ISandBox_PUSH_METHOD_DELEGATE:
        {
            ISandBox_ObjectRef obj = STO(ISandBox, -1);
            int index = GET_2BYTE_INT(&code[pc+1]);
            ISandBox_ObjectRef delegate;

            delegate = ISandBox_create_delegate(ISandBox, obj, index);
            STO_WRITE(ISandBox, -1, delegate);
            pc += 3;

            break;
        }
        case ISandBox_INVOKE: /* FALLTHRU */
        case ISandBox_INVOKE_DELEGATE:
        {
            int func_idx;

            if ((ISandBox_Opcode)code[pc] == ISandBox_INVOKE_DELEGATE) {
                ISandBox_ObjectRef delegate = STO(ISandBox, -1);

                if (is_null_pointer(&delegate)) {
                    if (throw_null_pointer_exception(ISandBox, &func,
                                                     &code, &code_size,
                                                     &pc, &base, &ee, &exe)) {
                        goto EXECUTE_END;
                    }
                }
                if (is_null_pointer(&delegate.data->u.delegate.object)) {
                    func_idx = delegate.data->u.delegate.index;
                } else {
                    func_idx
                        = (delegate.data->u.delegate.object.v_table
                           ->table[delegate.data->u.delegate.index].index);
                    STO_WRITE(ISandBox, -1, delegate.data->u.delegate.object);
                    ISandBox->stack.stack_pointer++; /* for func index */
                }
            } else {
                func_idx = STI(ISandBox, -1);
            }
            if (ISandBox->function[func_idx]->kind == NATIVE_FUNCTION) {
                restore_pc(ISandBox, ee, func, pc);
                ISandBox->current_exception = ISandBox_null_object_ref;
                invoke_native_function(ISandBox, func, ISandBox->function[func_idx],
                                       pc, &ISandBox->stack.stack_pointer, base);
                if (!is_object_null(ISandBox->current_exception)) {
                    if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                                 &base, &ee, &exe, &ISandBox->current_exception)) {
                        goto EXECUTE_END;
                    }
                } else {
                    pc++;
                }
            } else {
                invoke_Ivory_function(ISandBox, &func, ISandBox->function[func_idx],
                                       &code, &code_size, &pc,
                                       &ISandBox->stack.stack_pointer, &base,
                                       &ee, &exe);
            }
            break;
        }
        case ISandBox_RETURN:
            if (return_function(ISandBox, &func, &code, &code_size, &pc,
                                &base, &ee, &exe)) {
                ret = ISandBox->stack.stack[ISandBox->stack.stack_pointer-1];
                goto EXECUTE_END;
            }
            break;
        case ISandBox_NEW:
        {
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int class_index = ee->class_table[idx_in_exe];
            STO_WRITE(ISandBox, 0, ISandBox_create_class_object_i(ISandBox, class_index));
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_NEW_ARRAY:
        {
            int dim = code[pc+1];
            ISandBox_TypeSpecifier *type
                = &exe->type_specifier[GET_2BYTE_INT(&code[pc+2])];
            ISandBox_ObjectRef array;

            restore_pc(ISandBox, ee, func, pc);
            array = create_array(ISandBox, dim, type);
            ISandBox->stack.stack_pointer -= dim;
            STO_WRITE(ISandBox, 0, array);
            ISandBox->stack.stack_pointer++;
            pc += 4;
            break;
        }
        case ISandBox_NEW_ARRAY_LITERAL_INT:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            ISandBox_ObjectRef array;

            restore_pc(ISandBox, ee, func, pc);
            array = create_array_literal_int(ISandBox, size);
            ISandBox->stack.stack_pointer -= size;
            STO_WRITE(ISandBox, 0, array);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_NEW_ARRAY_LITERAL_DOUBLE:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            ISandBox_ObjectRef array;

            restore_pc(ISandBox, ee, func, pc);
            array = create_array_literal_double(ISandBox, size);
            ISandBox->stack.stack_pointer -= size;
            STO_WRITE(ISandBox, 0, array);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_NEW_ARRAY_LITERAL_LONG_DOUBLE:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            ISandBox_ObjectRef array;

            restore_pc(ISandBox, ee, func, pc);
            array = create_array_literal_long_double(ISandBox, size);
            ISandBox->stack.stack_pointer -= size;
            STO_WRITE(ISandBox, 0, array);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_NEW_ARRAY_LITERAL_OBJECT:
        {
            int size = GET_2BYTE_INT(&code[pc+1]);
            ISandBox_ObjectRef array;

            restore_pc(ISandBox, ee, func, pc);
            array = create_array_literal_object(ISandBox, size);
            ISandBox->stack.stack_pointer -= size;
            STO_WRITE(ISandBox, 0, array);
            ISandBox->stack.stack_pointer++;
            pc += 3;
            break;
        }
        case ISandBox_SUPER:
        {
            ISandBox_ObjectRef* obj = &STO(ISandBox, -1);
            ExecClass *this_class;
            
            this_class = obj->v_table->exec_class;
            obj->v_table = this_class->super_class->class_table;
            pc++;
            break;
        }
        case ISandBox_INSTANCEOF:
        {
            ISandBox_ObjectRef* obj = &STO(ISandBox, -1);
            int idx_in_exe = GET_2BYTE_INT(&code[pc+1]);
            int target_idx = ee->class_table[idx_in_exe];

            if (obj->v_table->exec_class->class_index == target_idx) {
                STI_WRITE(ISandBox, -1, ISandBox_TRUE);
            } else {
                STI_WRITE(ISandBox, -1, check_instanceof(ISandBox, obj, target_idx));
            }
            pc += 3;
            break;
        }
        case ISandBox_ISTYPE:
        {
            ISandBox_ObjectRef* obj = &STO(ISandBox, -1);
            int target_type = GET_2BYTE_INT(&code[pc+1]);
            int origin_type = obj->data->object_type;

            if (origin_type == target_type) {
                STI_WRITE(ISandBox, -1, ISandBox_TRUE);
            } else {
                STI_WRITE(ISandBox, -1, ISandBox_FALSE);
            }
            pc += 3;
            break;
        }
        case ISandBox_THROW:
        {
            ISandBox_ObjectRef* exception = &STO(ISandBox, -1);

            clear_stack_trace(ISandBox, exception);
            if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                         &base, &ee, &exe, exception)) {
                goto EXECUTE_END;
            }
            break;
        }
        case ISandBox_RETHROW:
        {
            ISandBox_ObjectRef* exception = &STO(ISandBox, -1);

            if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                         &base, &ee, &exe, exception)) {
                goto EXECUTE_END;
            }
            break;
        }
        case ISandBox_GO_FINALLY:
            STI_WRITE(ISandBox, 0, pc);
            ISandBox->stack.stack_pointer++;
            pc = GET_2BYTE_INT(&code[pc+1]);
            break;
        case ISandBox_FINALLY_END:
            if (!is_object_null(ISandBox->current_exception)) {
                if (do_throw(ISandBox, &func, &code, &code_size, &pc,
                             &base, &ee, &exe, &ISandBox->current_exception)) {
                    goto EXECUTE_END;
                }
            } else {
                pc = STI(ISandBox, -1) + 3;
                ISandBox->stack.stack_pointer--;
            }
            break;
        case ISandBox_CAST_ALL_TO_OBJECT:
        {
			int origin_type = GET_2BYTE_INT(&code[pc+1]);
            STO_WRITE(ISandBox, -1,
                      ISandBox_create_object_i(ISandBox, ST(ISandBox, -1), origin_type));
            pc += 3;
            break;
        }/* !!!unstable!!! */
        case ISandBox_CAST_OBJECT_TO_STRING:
        {
            ISandBox_Object *object;
            STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
                      /*ISandBox_cast_object_to_string(ISandBox, STO(ISandBox, -1))*/
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_INT:
        {
            STI(ISandBox, -1) = (int)((STO(ISandBox, -1)).data->u.object.int_value);
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_DOUBLE:
        {
            STD(ISandBox, -1) = (double)((STO(ISandBox, -1)).data->u.object.double_value);
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_LONG_DOUBLE:
        {
            STLD(ISandBox, -1) = (long double)((STO(ISandBox, -1)).data->u.object.long_double_value);
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_BOOLEAN:
        {
            STI(ISandBox, -1) = (int)((STO(ISandBox, -1)).data->u.object.int_value);
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_CLASS:
        {
			STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
            /*STO_WRITE(ISandBox, -1,
                      ISandBox_cast_object_to_class(ISandBox, STO(ISandBox, -1)));*/
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_DELEGATE:
        {
            STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
            /*STO_WRITE(ISandBox, -1,
                      ISandBox_cast_object_to_delegate(ISandBox, STO(ISandBox, -1)));*/
            pc++;
            break;
        }
        case ISandBox_CAST_OBJECT_TO_NATIVE_POINTER:
        {
            STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
            /*STO_WRITE(ISandBox, -1,
                      ISandBox_cast_object_to_native_pointer(ISandBox, STO(ISandBox, -1)));*/
            pc++;
            break;
        }
		case ISandBox_CAST_OBJECT_TO_ARRAY:
		{
			STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
            pc++;
            break;
		}
		case ISandBox_UNBOX_OBJECT:
		{
			STO_WRITE(ISandBox, -1, STO(ISandBox, -1).data->u.object.object);
            pc++;
            break;
		}
        case ISandBox_GOTO:
        {
            pc = GET_2BYTE_INT(&code[pc+1]);
            break;
        }
        default:
            DBG_assert(0, ("code[%d]..%d\n", pc, code[pc]));
        }
        /* MEM_check_all_blocks(); */
    }
 EXECUTE_END:
    ;
    return ret;
}

void
ISandBox_push_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Value value)
{
    STO_WRITE(ISandBox, 0, value.object);
    ISandBox->stack.stack_pointer++;
}

ISandBox_Value
ISandBox_pop_object(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Value ret;

    ret.object = STO(ISandBox, -1);
    ISandBox->stack.stack_pointer--;

    return ret;
}

ISandBox_Value
ISandBox_execute(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_Value ret;
    ISandBox->current_executable = ISandBox->top_level;
    ISandBox->current_function = NULL;
    ISandBox->pc = 0;
    ISandBox_expand_stack(ISandBox,
                     ISandBox->top_level->executable->top_level.need_stack_size);
    ISandBox_execute_i(ISandBox, NULL, ISandBox->top_level->executable->top_level.code,
                  ISandBox->top_level->executable->top_level.code_size, 0);

    return ret;
}


ISandBox_Value
ISandBox_invoke_delegate(ISandBox_VirtualMachine *ISandBox, ISandBox_Value delegate,
                    ISandBox_Value *args)
{
    ISandBox_Value ret;
    int func_idx;
    Function *callee;
    ISandBox_ObjectRef del_obj = delegate.object;

    if (is_null_pointer(&del_obj.data->u.delegate.object)) {
        func_idx = del_obj.data->u.delegate.index;
    } else {
        func_idx
            = (del_obj.data->u.delegate.object.v_table
               ->table[del_obj.data->u.delegate.index].index);
    }
    callee = ISandBox->function[func_idx];

    if (callee->kind == Ivory_FUNCTION) {
        invoke_Ivory_function_from_native(ISandBox, callee,
                                           del_obj.data->u.delegate.object,
                                           args);
    } else {
        DBG_assert((callee->kind == NATIVE_FUNCTION),
                   ("kind..%d", callee->kind));
    }

    return ret;
}

void ISandBox_dispose_executable_list(ISandBox_ExecutableList *list)
{
    ISandBox_ExecutableItem *temp;

    while (list->list) {
        temp = list->list;
        list->list = temp->next;
        ISandBox_dispose_executable(temp->executable);
        MEM_free(temp);
    }
    MEM_free(list);
}

static void
dispose_v_table(ISandBox_VTable *v_table)
{
    int i;

    for (i = 0; i < v_table->table_size; i++) {
        MEM_free(v_table->table[i].name);
    }
    MEM_free(v_table->table);
    MEM_free(v_table);
}

void
ISandBox_dispose_virtual_machine(ISandBox_VirtualMachine *ISandBox)
{
    ExecutableEntry *ee_temp;
    int i;
    int j;

    while (ISandBox->executable_entry) {
        ee_temp = ISandBox->executable_entry;
        ISandBox->executable_entry = ee_temp->next;

        MEM_free(ee_temp->function_table);
        MEM_free(ee_temp->class_table);
        MEM_free(ee_temp->enum_table);
        /*MEM_free(ee_temp->constant_table);*/
        MEM_free(ee_temp->static_v.variable);
        MEM_free(ee_temp);
    }

    ISandBox_garbage_collect(ISandBox);
    MEM_free(ISandBox->stack.stack);
    MEM_free(ISandBox->stack.pointer_flags);

    for (i = 0; i < ISandBox->function_count; i++) {
        MEM_free(ISandBox->function[i]->name);
        MEM_free(ISandBox->function[i]->package_name);
        MEM_free(ISandBox->function[i]);
    }
    MEM_free(ISandBox->function);

    for (i = 0; i < ISandBox->class_count; i++) {
        MEM_free(ISandBox->class[i]->package_name);
        MEM_free(ISandBox->class[i]->name);
        dispose_v_table(ISandBox->class[i]->class_table);
        for (j = 0; j < ISandBox->class[i]->interface_count; j++) {
            dispose_v_table(ISandBox->class[i]->interface_v_table[j]);
        }
        MEM_free(ISandBox->class[i]->interface_v_table);
        MEM_free(ISandBox->class[i]->interface);
        MEM_free(ISandBox->class[i]->field_type);
        MEM_free(ISandBox->class[i]);
    }

    for (i = 0; i < ISandBox->enum_count; i++) {
        MEM_free(ISandBox->enums[i]->name);
        MEM_free(ISandBox->enums[i]->package_name);
        MEM_free(ISandBox->enums[i]);
    }
    MEM_free(ISandBox->enums);

    /*for (i = 0; i < ISandBox->constant_count; i++) {
        MEM_free(ISandBox->constant[i]->name);
        MEM_free(ISandBox->constant[i]->package_name);
        MEM_free(ISandBox->constant[i]);
    }
    MEM_free(ISandBox->constant);*/

    MEM_free(ISandBox->array_v_table->table);
    MEM_free(ISandBox->array_v_table);
    MEM_free(ISandBox->string_v_table->table);
    MEM_free(ISandBox->string_v_table);
    MEM_free(ISandBox->iterator_v_table->table);
    MEM_free(ISandBox->iterator_v_table);
    MEM_free(ISandBox->class);

    MEM_free(ISandBox);
}
