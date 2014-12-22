#include <stdio.h>
#include <string.h>
#include "Ivyc.h"
#include "MEM.h"
#include "DBG.h"
#include "ISandBox_pri.h"

static void
implement_Ivory_function(ISandBox_VirtualMachine *ISandBox, int dest_idx,
                          ExecutableEntry *ee, int src_idx)
{
    ISandBox->function[dest_idx]->u.Ivory_f.executable
        = ee;
    ISandBox->function[dest_idx]->u.Ivory_f.index = src_idx;
}

static void
add_functions(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_func_count = 0;
    ISandBox_Boolean *new_func_flags;

    new_func_flags = MEM_malloc(sizeof(ISandBox_Boolean)
                                * ee->executable->function_count);

    for (src_idx = 0; src_idx < ee->executable->function_count; src_idx++) {
        for (dest_idx = 0; dest_idx < ISandBox->function_count; dest_idx++) {
            if (!strcmp(ISandBox->function[dest_idx]->name,
                        ee->executable->function[src_idx].name)
                && ISandBox_compare_package_name(ISandBox->function[dest_idx]
                                            ->package_name,
                                            ee->executable->function
                                            [src_idx].package_name)) {
                if (ee->executable->function[src_idx].is_implemented
                    && ISandBox->function[dest_idx]->is_implemented) {
                    char *package_name;

                    if (ISandBox->function[dest_idx]->package_name) {
                        package_name = ISandBox->function[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                FUNCTION_MULTIPLE_DEFINE_ERR,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "name",
                                ISandBox->function[dest_idx]->name,
                                ISandBox_MESSAGE_ARGUMENT_END);
                }
                new_func_flags[src_idx] = ISandBox_FALSE;
                if (ee->executable->function[src_idx].is_implemented) {
                    implement_Ivory_function(ISandBox, dest_idx, ee, src_idx);
                }
                break;
            }
        }
        if (dest_idx == ISandBox->function_count) {
            new_func_flags[src_idx] = ISandBox_TRUE;
            add_func_count++;
        }
    }
    ISandBox->function
        = MEM_realloc(ISandBox->function,
                      sizeof(Function*)
                      * (ISandBox->function_count + add_func_count));

    for (src_idx = 0, dest_idx = ISandBox->function_count;
         src_idx < ee->executable->function_count; src_idx++) {
        if (!new_func_flags[src_idx])
            continue;

        ISandBox->function[dest_idx] = MEM_malloc(sizeof(Function));
        if (ee->executable->function[src_idx].package_name) {
            ISandBox->function[dest_idx]->package_name
                = MEM_strdup(ee->executable->function[src_idx].package_name);
        } else {
            ISandBox->function[dest_idx]->package_name = NULL;
        }
        ISandBox->function[dest_idx]->name
            = MEM_strdup(ee->executable->function[src_idx].name);
        ISandBox->function[dest_idx]->kind = Ivory_FUNCTION;
        ISandBox->function[dest_idx]->is_implemented
            = ee->executable->function[src_idx].is_implemented;
        if (ee->executable->function[src_idx].is_implemented) {
            implement_Ivory_function(ISandBox, dest_idx, ee, src_idx);
        }
        dest_idx++;
    }
    ISandBox->function_count += add_func_count;
    MEM_free(new_func_flags);
}

static void
add_enums(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_enum_count = 0;
    ISandBox_Boolean *new_enum_flags;

    new_enum_flags = MEM_malloc(sizeof(ISandBox_Boolean)
                                * ee->executable->enum_count);

    for (src_idx = 0; src_idx < ee->executable->enum_count; src_idx++) {
        for (dest_idx = 0; dest_idx < ISandBox->enum_count; dest_idx++) {
            if (!strcmp(ISandBox->enums[dest_idx]->name,
                        ee->executable->enum_definition[src_idx].name)
                && ISandBox_compare_package_name(ISandBox->enums[dest_idx]
                                            ->package_name,
                                            ee->executable->enum_definition
                                            [src_idx].package_name)) {
                if (ee->executable->enum_definition[src_idx].is_defined
                    && ISandBox->enums[dest_idx]->is_defined) {
                    char *package_name;

                    if (ISandBox->enums[dest_idx]->package_name) {
                        package_name = ISandBox->enums[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                ENUM_MULTIPLE_DEFINE_ERR,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "name",
                                ISandBox->enums[dest_idx]->name,
                                ISandBox_MESSAGE_ARGUMENT_END);
                }
                new_enum_flags[src_idx] = ISandBox_FALSE;
                break;
            }
        }
        if (dest_idx == ISandBox->enum_count) {
            new_enum_flags[src_idx] = ISandBox_TRUE;
            add_enum_count++;
        }
    }
    ISandBox->enums = MEM_realloc(ISandBox->enums,
                             sizeof(Enum*)
                             * (ISandBox->enum_count + add_enum_count));

    for (src_idx = 0, dest_idx = ISandBox->enum_count;
         src_idx < ee->executable->enum_count; src_idx++) {
        if (!new_enum_flags[src_idx])
            continue;

        ISandBox->enums[dest_idx] = MEM_malloc(sizeof(Enum));
        if (ee->executable->enum_definition[src_idx].package_name) {
            ISandBox->enums[dest_idx]->package_name
                = MEM_strdup(ee->executable
                             ->enum_definition[src_idx].package_name);
        } else {
            ISandBox->enums[dest_idx]->package_name = NULL;
        }
        ISandBox->enums[dest_idx]->name
            = MEM_strdup(ee->executable->enum_definition[src_idx].name);
        ISandBox->enums[dest_idx]->is_defined
            = ee->executable->enum_definition[src_idx].is_defined;
        ISandBox->enums[dest_idx]->ISandBox_enum
            = &ee->executable->enum_definition[src_idx];
        dest_idx++;
    }
    ISandBox->enum_count += add_enum_count;
    MEM_free(new_enum_flags);
}

/*static void
add_constants(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_const_count = 0;
    ISandBox_Boolean *new_const_flags;

    new_const_flags = MEM_malloc(sizeof(ISandBox_Boolean)
                                 * ee->executable->constant_count);

    for (src_idx = 0; src_idx < ee->executable->constant_count; src_idx++) {
        for (dest_idx = 0; dest_idx < ISandBox->constant_count; dest_idx++) {
            if (!strcmp(ISandBox->constant[dest_idx]->name,
                        ee->executable->constant_definition[src_idx].name)
                && ISandBox_compare_package_name(ISandBox->constant[dest_idx]
                                            ->package_name,
                                            ee->executable->constant_definition
                                            [src_idx].package_name)) {
                if (ee->executable->constant_definition[src_idx].is_defined
                    && ISandBox->constant[dest_idx]->is_defined) {
                    char *package_name;

                    if (ISandBox->constant[dest_idx]->package_name) {
                        package_name = ISandBox->constant[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                CONSTANT_MULTIPLE_DEFINE_ERR,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "name",
                                ISandBox->constant[dest_idx]->name,
                                ISandBox_MESSAGE_ARGUMENT_END);
                }
                new_const_flags[src_idx] = ISandBox_FALSE;
                break;
            }
        }
        if (dest_idx == ISandBox->constant_count) {
            new_const_flags[src_idx] = ISandBox_TRUE;
            add_const_count++;
        }
    }
    ISandBox->constant
        = MEM_realloc(ISandBox->constant,
                      sizeof(Constant*)
                      * (ISandBox->constant_count + add_const_count));

    for (src_idx = 0, dest_idx = ISandBox->constant_count;
         src_idx < ee->executable->constant_count; src_idx++) {
        if (!new_const_flags[src_idx])
            continue;

        ISandBox->constant[dest_idx] = MEM_malloc(sizeof(Constant));
        if (ee->executable->constant_definition[src_idx].package_name) {
            ISandBox->constant[dest_idx]->package_name
                = MEM_strdup(ee->executable
                             ->constant_definition[src_idx].package_name);
        } else {
            ISandBox->constant[dest_idx]->package_name = NULL;
        }
        ISandBox->constant[dest_idx]->name
            = MEM_strdup(ee->executable->constant_definition[src_idx].name);
        ISandBox->constant[dest_idx]->is_defined
            = ee->executable->constant_definition[src_idx].is_defined;
        dest_idx++;
    }
    ISandBox->constant_count += add_const_count;
    MEM_free(new_const_flags);
}*/

static void
convert_code(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe,
             ISandBox_Byte *code, int code_size, ISandBox_Function *func)
{
    int i;
    int j;
    OpcodeInfo *info;
    int src_idx;
    unsigned int dest_idx;

    for (i = 0; i < code_size; i++) {
        if (code[i] == ISandBox_PUSH_STACK_INT
            || code[i] == ISandBox_PUSH_STACK_DOUBLE
            || code[i] == ISandBox_PUSH_STACK_LONG_DOUBLE
            || code[i] == ISandBox_PUSH_STACK_OBJECT
            || code[i] == ISandBox_POP_STACK_INT
            || code[i] == ISandBox_POP_STACK_DOUBLE
            || code[i] == ISandBox_POP_STACK_LONG_DOUBLE
            || code[i] == ISandBox_POP_STACK_OBJECT) {
            int parameter_count;

            DBG_assert(func != NULL, ("func == NULL!\n"));
            
            if (func->is_method) {
                parameter_count = func->parameter_count + 1; /* for this */
            } else{
                parameter_count = func->parameter_count;
            }

            src_idx = GET_2BYTE_INT(&code[i+1]);
            if (src_idx >= parameter_count) {
                dest_idx = src_idx + CALL_INFO_ALIGN_SIZE;
            } else {
                dest_idx = src_idx;
            }
            SET_2BYTE_INT(&code[i+1], dest_idx);
        }
        info = &ISandBox_opcode_info[code[i]];
        for (j = 0; info->parameter[j] != '\0'; j++) {
            switch (info->parameter[j]) {
            case 'b':
                i++;
                break;
            case 's': /* FALLTHRU */
            case 'p':
                i += 2;
                break;
            default:
                DBG_assert(0, ("param..%s, j..%d", info->parameter, j));
            }
        }
    }
}

int
ISandBox_search_class(ISandBox_VirtualMachine *ISandBox, char *package_name, char *name)
{
    int i;

    for (i = 0; i < ISandBox->class_count; i++) {
        if (ISandBox_compare_package_name(ISandBox->class[i]->package_name,
                                     package_name)
            && !strcmp(ISandBox->class[i]->name, name)) {
            return i;
        }
    }
    ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC, CLASS_NOT_FOUND_ERR,
                ISandBox_STRING_MESSAGE_ARGUMENT, "name", name,
                ISandBox_MESSAGE_ARGUMENT_END);
    return 0; /* make compiler happy */
}

int
ISandBox_search_function(ISandBox_VirtualMachine *ISandBox, char *package_name, char *name)
{
    int i;

    for (i = 0; i < ISandBox->function_count; i++) {
        if (ISandBox_compare_package_name(ISandBox->function[i]->package_name,
                                     package_name)
            && !strcmp(ISandBox->function[i]->name, name)) {
            return i;
        }
    }
    return FUNCTION_NOT_FOUND;
}

int
ISandBox_search_enum(ISandBox_VirtualMachine *ISandBox, char *package_name, char *name)
{
    int i;

    for (i = 0; i < ISandBox->enum_count; i++) {
        if (ISandBox_compare_package_name(ISandBox->enums[i]->package_name,
                                     package_name)
            && !strcmp(ISandBox->enums[i]->name, name)) {
            return i;
        }
    }
    return ENUM_NOT_FOUND;
}

/*int
ISandBox_search_constant(ISandBox_VirtualMachine *ISandBox, char *package_name, char *name)
{
    int i;

    for (i = 0; i < ISandBox->constant_count; i++) {
        if (ISandBox_compare_package_name(ISandBox->constant[i]->package_name,
                                     package_name)
            && !strcmp(ISandBox->constant[i]->name, name)) {
            return i;
        }
    }
    return CONSTANT_NOT_FOUND;
}*/

static void
add_reference_table(ISandBox_VirtualMachine *ISandBox,
                    ExecutableEntry *entry, ISandBox_Executable *exe)
{
    int i;

    entry->function_table = MEM_malloc(sizeof(int) * exe->function_count);
    for (i = 0; i < exe->function_count; i++) {
        entry->function_table[i]
            = ISandBox_search_function(ISandBox, exe->function[i].package_name,
                                  exe->function[i].name);
    }

    entry->enum_table = MEM_malloc(sizeof(int) * exe->enum_count);
    for (i = 0; i < exe->enum_count; i++) {
        entry->enum_table[i]
            = ISandBox_search_enum(ISandBox, exe->enum_definition[i]
                              .package_name,
                              exe->enum_definition[i].name);
    }


    /*entry->constant_table = MEM_malloc(sizeof(int) * exe->constant_count);
    for (i = 0; i < exe->constant_count; i++) {
        entry->constant_table[i]
            = ISandBox_search_constant(ISandBox, exe->constant_definition[i]
                                  .package_name,
                                  exe->constant_definition[i].name);
    }*/

    entry->class_table = MEM_malloc(sizeof(int) * exe->class_count);
    for (i = 0; i < exe->class_count; i++) {
        entry->class_table[i]
            = ISandBox_search_class(ISandBox, exe->class_definition[i].package_name,
                               exe->class_definition[i].name);
    }
}

static void
add_static_variables(ExecutableEntry *entry, ISandBox_Executable *exe)
{
    int i;

    entry->static_v.variable
        = MEM_malloc(sizeof(ISandBox_Value) * exe->global_variable_count);
    entry->static_v.variable_count = exe->global_variable_count;

    for (i = 0; i < exe->global_variable_count; i++) {
        if (exe->global_variable[i].type->basic_type == ISandBox_STRING_TYPE) {
            entry->static_v.variable[i].object = ISandBox_null_object_ref;
        }
    }
    for (i = 0; i < exe->global_variable_count; i++) {
        ISandBox_initialize_value(exe->global_variable[i].type,
                             &entry->static_v.variable[i]);
    }
}

static ISandBox_Class *
search_class_from_executable(ISandBox_Executable *exe, char *name)
{
    int i;

    for (i = 0; i < exe->class_count; i++) {
        if (!strcmp(exe->class_definition[i].name, name)) {
            return &exe->class_definition[i];
        }
    }
    DBG_panic(("class %s not found.", name));

    return NULL; /* make compiler happy */
}

static int
set_field_types(ISandBox_Executable *exe, ISandBox_Class *pos,
                ISandBox_TypeSpecifier **field_type, int index)
{
    ISandBox_Class *next;
    int i;

    if (pos->super_class) {
        next = search_class_from_executable(exe, pos->super_class->name);
        index = set_field_types(exe, next, field_type, index);
    }
    for (i = 0; i < pos->field_count; i++) {
        field_type[index] = pos->field[i].type;
        index++;
    }

    return index;
}

static void
add_fields(ISandBox_Executable *exe, ISandBox_Class *src, ExecClass *dest)
{
    int field_count = 0;
    ISandBox_Class *pos;

    for (pos = src; ; ) {
        field_count += pos->field_count;
        if (pos->super_class == NULL)
            break;
        pos = search_class_from_executable(exe, pos->super_class->name);
    }
    dest->field_count = field_count;
    dest->field_type = MEM_malloc(sizeof(ISandBox_TypeSpecifier*) * field_count);
    set_field_types(exe, src, dest->field_type, 0);
}

static void
set_v_table(ISandBox_VirtualMachine *ISandBox, ISandBox_Class *class_p,
            ISandBox_Method *src, VTableItem *dest, ISandBox_Boolean set_name)
{
    char *func_name;
    int  func_idx;

    if (set_name) {
        dest->name = MEM_strdup(src->name);
    }
    func_name = ISandBox_create_method_function_name(class_p->name, src->name);
    func_idx = ISandBox_search_function(ISandBox, class_p->package_name, func_name);

    if (func_idx == FUNCTION_NOT_FOUND && !src->is_abstract) {
        ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC, FUNCTION_NOT_FOUND_ERR,
                    ISandBox_STRING_MESSAGE_ARGUMENT, "name", func_name,
                    ISandBox_MESSAGE_ARGUMENT_END);
    }
    MEM_free(func_name);
    dest->index = func_idx;
}

static int
add_method(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe, ISandBox_Class *pos,
           ISandBox_VTable *v_table)
{
    ISandBox_Class   *next;
    int         i;
    int         j;
    int         super_method_count = 0;
    int         method_count;

    if (pos->super_class) {
        next = search_class_from_executable(exe, pos->super_class->name);
        super_method_count = add_method(ISandBox, exe, next, v_table);
    }

    method_count = super_method_count;
    for (i = 0; i < pos->method_count; i++) {
        for (j = 0; j < super_method_count; j++) {
            if (!strcmp(pos->method[i].name, v_table->table[j].name)) {
                set_v_table(ISandBox, pos, &pos->method[i], &v_table->table[j],
                            ISandBox_FALSE);
                break;
            }
        }
        /* if pos->method[i].is_override == true,
         * this method implements interface method.
         */
        if (j == super_method_count && !pos->method[i].is_override) {
            v_table->table
                = MEM_realloc(v_table->table,
                              sizeof(VTableItem) * (method_count + 1));
            set_v_table(ISandBox, pos, &pos->method[i],
                        &v_table->table[method_count], ISandBox_TRUE);
            method_count++;
            v_table->table_size = method_count;
        }
    }

    return method_count;
}

static ISandBox_VTable *
alloc_v_table(ExecClass *exec_class)
{
    ISandBox_VTable *v_table;

    v_table = MEM_malloc(sizeof(ISandBox_VTable));
    v_table->exec_class = exec_class;
    v_table->table = NULL;

    return v_table;
}

static void
add_methods(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe,
            ISandBox_Class *src, ExecClass *dest)
{
    int         method_count;
    ISandBox_VTable  *v_table;
    int         i;
    ISandBox_Class   *interface;
    int         method_idx;

    v_table = alloc_v_table(dest);
    method_count = add_method(ISandBox, exe, src, v_table);
    dest->class_table = v_table;
    dest->interface_count = src->interface_count;
    dest->interface_v_table
        = MEM_malloc(sizeof(ISandBox_VTable*) * src->interface_count);

    for (i = 0; i < src->interface_count; i++) {
        dest->interface_v_table[i] = alloc_v_table(dest);
        interface = search_class_from_executable(exe,
                                                 src->interface_[i].name);
        dest->interface_v_table[i]->table
            = MEM_malloc(sizeof(VTableItem) * interface->method_count);
        dest->interface_v_table[i]->table_size = interface->method_count;
        for (method_idx = 0; method_idx < interface->method_count;
             method_idx++) {
            set_v_table(ISandBox, src, &interface->method[method_idx],
                        &dest->interface_v_table[i]->table[method_idx],
                        ISandBox_TRUE);
        }
    }
}

static void
add_class(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe,
          ISandBox_Class *src, ExecClass *dest)
{
    
    add_fields(exe, src, dest);
    add_methods(ISandBox, exe, src, dest);
}

static void
set_super_class(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *exe,
                int old_class_count)
{
    int class_idx;
    ISandBox_Class *ISandBox_class;
    int super_class_index;
    int if_idx;
    int interface_index;

    for (class_idx = old_class_count; class_idx < ISandBox->class_count;
         class_idx++) {
        ISandBox_class = search_class_from_executable(exe,
                                                 ISandBox->class[class_idx]->name);
        if (ISandBox_class->super_class == NULL) {
            ISandBox->class[class_idx]->super_class = NULL;
        } else {
            super_class_index
                = ISandBox_search_class(ISandBox,
                                   ISandBox_class->super_class
                                   ->package_name,
                                   ISandBox_class->super_class->name);
            ISandBox->class[class_idx]->super_class = ISandBox->class[super_class_index];
        }
        ISandBox->class[class_idx]->interface
            = MEM_malloc(sizeof(ExecClass*) * ISandBox_class->interface_count);
        for (if_idx = 0; if_idx < ISandBox_class->interface_count; if_idx++) {
            interface_index
                = ISandBox_search_class(ISandBox,
                                   ISandBox_class->interface_[if_idx].package_name,
                                   ISandBox_class->interface_[if_idx].name);
            ISandBox->class[class_idx]->interface[if_idx]
                = ISandBox->class[interface_index];
        }
    }
}

static void
add_classes(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee)
{
    int src_idx;
    int dest_idx;
    int add_class_count = 0;
    ISandBox_Boolean *new_class_flags;
    int old_class_count;
    ISandBox_Executable *exe = ee->executable;

    new_class_flags = MEM_malloc(sizeof(ISandBox_Boolean)
                                 * exe->class_count);

    for (src_idx = 0; src_idx < exe->class_count; src_idx++) {
        for (dest_idx = 0; dest_idx < ISandBox->class_count; dest_idx++) {
            if (!strcmp(ISandBox->class[dest_idx]->name,
                        exe->class_definition[src_idx].name)
                && ISandBox_compare_package_name(ISandBox->class[dest_idx]
                                            ->package_name,
                                            exe->class_definition[src_idx]
                                            .package_name)) {
                if (exe->class_definition[src_idx].is_implemented
                    && ISandBox->class[dest_idx]->is_implemented) {
                    char *package_name;

                    if (ISandBox->class[dest_idx]->package_name) {
                        package_name = ISandBox->class[dest_idx]->package_name;
                    } else {
                        package_name = "";
                    }
                    ISandBox_error_i(NULL, NULL, NO_LINE_NUMBER_PC,
                                CLASS_MULTIPLE_DEFINE_ERR,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "package",
                                package_name,
                                ISandBox_STRING_MESSAGE_ARGUMENT, "name",
                                ISandBox->class[dest_idx]->name,
                                ISandBox_MESSAGE_ARGUMENT_END);
                }
                new_class_flags[src_idx] = ISandBox_FALSE;
                if (exe->class_definition[src_idx].is_implemented) {
                    add_class(ISandBox, exe, &exe->class_definition[src_idx],
                              ISandBox->class[dest_idx]);
                }
                break;
            }
        }
        if (dest_idx == ISandBox->class_count) {
            new_class_flags[src_idx] = ISandBox_TRUE;
            add_class_count++;
        }
    }
    ISandBox->class
        = MEM_realloc(ISandBox->class, sizeof(ExecClass*)
                      * (ISandBox->class_count + add_class_count));

    for (src_idx = 0, dest_idx = ISandBox->class_count;
         src_idx < exe->class_count; src_idx++) {
        if (!new_class_flags[src_idx])
            continue;

        ISandBox->class[dest_idx] = MEM_malloc(sizeof(ExecClass));
        ISandBox->class[dest_idx]->ISandBox_class = &exe->class_definition[src_idx];
        ISandBox->class[dest_idx]->executable = ee;
        if (exe->class_definition[src_idx].package_name) {
            ISandBox->class[dest_idx]->package_name
                = MEM_strdup(exe->class_definition[src_idx].package_name);
        } else {
            ISandBox->class[dest_idx]->package_name = NULL;
        }
        ISandBox->class[dest_idx]->name
            = MEM_strdup(exe->class_definition[src_idx].name);
        ISandBox->class[dest_idx]->is_implemented
            = exe->class_definition[src_idx].is_implemented;
        ISandBox->class[dest_idx]->class_index = dest_idx;
        if (exe->class_definition[src_idx].is_implemented) {
            add_class(ISandBox, exe, &exe->class_definition[src_idx],
                      ISandBox->class[dest_idx]);
        }
        dest_idx++;
    }
    old_class_count = ISandBox->class_count;
    ISandBox->class_count += add_class_count;

    set_super_class(ISandBox, exe, old_class_count);

    MEM_free(new_class_flags);
}

static ExecutableEntry *
add_executable_to_ISandBox(ISandBox_VirtualMachine *ISandBox, ISandBox_Executable *executable,
                      ISandBox_Boolean is_top_level)
{
    int i;
    ExecutableEntry *ee_pos;
    ExecutableEntry *new_entry;
    
    new_entry = MEM_malloc(sizeof(ExecutableEntry));
    new_entry->executable = executable;
    new_entry->next = NULL;

    if (ISandBox->executable_entry == NULL) {
        ISandBox->executable_entry = new_entry;
    } else {
        for (ee_pos = ISandBox->executable_entry; ee_pos->next;
             ee_pos = ee_pos->next)
            ;
        ee_pos->next = new_entry;
    }
    
    add_functions(ISandBox, new_entry);
    add_enums(ISandBox, new_entry);
    /*add_constants(ISandBox, new_entry);*/
    add_classes(ISandBox, new_entry);

    convert_code(ISandBox, executable,
                 executable->top_level.code, executable->top_level.code_size,
                 NULL);

    for (i = 0; i < executable->function_count; i++) {
        if (executable->function[i].is_implemented) {
            convert_code(ISandBox, executable,
                         executable->function[i].code_block.code,
                         executable->function[i].code_block.code_size,
                         &executable->function[i]);
        }
    }
    add_reference_table(ISandBox, new_entry, executable);

    add_static_variables(new_entry, executable);

    if (is_top_level) {
        ISandBox->top_level = new_entry;
    }
    return new_entry;
}

/*static void
initialize_constant(ISandBox_VirtualMachine *ISandBox, ExecutableEntry *ee)
{
    ISandBox_Executable *exe = ee->executable;

    ISandBox->current_executable = ee;
    ISandBox->current_function = NULL;
    ISandBox->pc = 0;
    ISandBox_expand_stack(ISandBox, exe->constant_initializer.need_stack_size);
    ISandBox_execute_i(ISandBox, NULL, exe->constant_initializer.code,
                  exe->constant_initializer.code_size, 0);
}*/

void
ISandBox_set_executable(ISandBox_VirtualMachine *ISandBox, ISandBox_ExecutableList *list)
{
    ISandBox_ExecutableItem *pos;
    int old_class_count;
    ExecutableEntry *ee;

    ISandBox->executable_list = list;

    old_class_count = ISandBox->class_count;
    for (pos = list->list; pos; pos = pos->next) {

        ee = add_executable_to_ISandBox(ISandBox, pos->executable,
                                   (pos->executable == list->top_level));
        /*initialize_constant(ISandBox, ee);*/
    }
}

static VTableItem st_array_method_v_table[] = {
    {ARRAY_PREFIX ARRAY_METHOD_SIZE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_RESIZE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_INSERT, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_REMOVE, FUNCTION_NOT_FOUND},
    {ARRAY_PREFIX ARRAY_METHOD_ADD, FUNCTION_NOT_FOUND},
	{ARRAY_PREFIX ARRAY_METHOD_ITERATOR, FUNCTION_NOT_FOUND},
};

static VTableItem st_string_method_v_table[] = {
    {STRING_PREFIX STRING_METHOD_LENGTH, FUNCTION_NOT_FOUND},
    {STRING_PREFIX STRING_METHOD_SUBSTR, FUNCTION_NOT_FOUND},
};

static VTableItem st_iterator_method_v_table[] = {
    {ITERATOR_PREFIX ITERATOR_METHOD_NEXT, FUNCTION_NOT_FOUND},
    {ITERATOR_PREFIX ITERATOR_METHOD_HASNEXT, FUNCTION_NOT_FOUND},
    {ITERATOR_PREFIX ITERATOR_METHOD_CURRENT, FUNCTION_NOT_FOUND},
};

static void
set_built_in_methods(ISandBox_VirtualMachine *ISandBox)
{
    ISandBox_VTable *array_v_table;
    ISandBox_VTable *string_v_table;
    ISandBox_VTable *iterator_v_table;
    int i;

    array_v_table = alloc_v_table(NULL);
    array_v_table->table_size = ARRAY_SIZE(st_array_method_v_table);
    array_v_table->table = MEM_malloc(sizeof(VTableItem)
                                      * array_v_table->table_size);
    for (i = 0; i < array_v_table->table_size; i++) {
        array_v_table->table[i] = st_array_method_v_table[i];
        array_v_table->table[i].index
            = ISandBox_search_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                                  array_v_table->table[i].name);
    }
    ISandBox->array_v_table = array_v_table;

    string_v_table = alloc_v_table(NULL);
    string_v_table->table_size = ARRAY_SIZE(st_string_method_v_table);
    string_v_table->table = MEM_malloc(sizeof(VTableItem)
                                      * string_v_table->table_size);
    for (i = 0; i < string_v_table->table_size; i++) {
        string_v_table->table[i] = st_string_method_v_table[i];
        string_v_table->table[i].index
            = ISandBox_search_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                                  string_v_table->table[i].name);
    }
    ISandBox->string_v_table = string_v_table;

    iterator_v_table = alloc_v_table(NULL);
    iterator_v_table->table_size = ARRAY_SIZE(st_iterator_method_v_table);
    iterator_v_table->table = MEM_malloc(sizeof(VTableItem)
                                      * iterator_v_table->table_size);
    for (i = 0; i < iterator_v_table->table_size; i++) {
        iterator_v_table->table[i] = st_iterator_method_v_table[i];
        iterator_v_table->table[i].index
            = ISandBox_search_function(ISandBox, BUILT_IN_METHOD_PACKAGE_NAME,
                                  iterator_v_table->table[i].name);
    }
    ISandBox->iterator_v_table = iterator_v_table;
}

ISandBox_VirtualMachine *
ISandBox_create_virtual_machine(void)
{
    ISandBox_VirtualMachine *ISandBox;

    ISandBox = MEM_malloc(sizeof(ISandBox_VirtualMachine));
    ISandBox->stack.alloc_size = STACK_ALLOC_SIZE;
    ISandBox->stack.stack = MEM_malloc(sizeof(ISandBox_Value) * STACK_ALLOC_SIZE);
    ISandBox->stack.pointer_flags
        = MEM_malloc(sizeof(ISandBox_Boolean) * STACK_ALLOC_SIZE);
    ISandBox->stack.stack_pointer = 0;
    ISandBox->heap.current_heap_size = 0;
    ISandBox->heap.header = NULL;
    ISandBox->heap.current_threshold = HEAP_THRESHOLD_SIZE;
    ISandBox->current_executable = NULL;
    ISandBox->current_function = NULL;
    ISandBox->current_exception = ISandBox_null_object_ref;
    ISandBox->function = NULL;
    ISandBox->function_count = 0;
    ISandBox->class = NULL;
    ISandBox->class_count = 0;
    ISandBox->enums = NULL;
    ISandBox->enum_count = 0;
    /*ISandBox->constant = NULL;
    ISandBox->constant_count = 0;*/
    ISandBox->executable_list = NULL;
    ISandBox->executable_entry = NULL;
    ISandBox->top_level = NULL;
    ISandBox->current_context = NULL;
    ISandBox->free_context = NULL;

    ISandBox_add_native_functions(ISandBox);
#ifdef IVY_WINDOWS
    ISandBox_add_windows_native_functions(ISandBox);
    ISandBox_add_widget_native_functions(ISandBox);
#endif /* IVY_WINDOWS */

    set_built_in_methods(ISandBox);

    return ISandBox;
}

void
ISandBox_add_native_function(ISandBox_VirtualMachine *ISandBox,
                        char *package_name, char *func_name,
                        ISandBox_NativeFunctionProc *proc, int arg_count,
                        ISandBox_Boolean is_method, ISandBox_Boolean return_pointer)
{
    ISandBox->function
        = MEM_realloc(ISandBox->function,
                      sizeof(Function*) * (ISandBox->function_count + 1));

    ISandBox->function[ISandBox->function_count] = MEM_malloc(sizeof(Function));
    ISandBox->function[ISandBox->function_count]->package_name
        = MEM_strdup(package_name);
    ISandBox->function[ISandBox->function_count]->name = MEM_strdup(func_name);
    ISandBox->function[ISandBox->function_count]->kind = NATIVE_FUNCTION;
    ISandBox->function[ISandBox->function_count]->is_implemented = ISandBox_TRUE;
    ISandBox->function[ISandBox->function_count]->u.native_f.proc = proc;
    ISandBox->function[ISandBox->function_count]->u.native_f.arg_count = arg_count;
    ISandBox->function[ISandBox->function_count]->u.native_f.is_method = is_method;
    ISandBox->function[ISandBox->function_count]->u.native_f.return_pointer
        = return_pointer;
    ISandBox->function_count++;
}

void
ISandBox_dynamic_load(ISandBox_VirtualMachine *ISandBox,
                 ISandBox_Executable *caller_exe, Function *caller, int pc,
                 Function *func)
{
    Ivyc_Compiler *compiler;
    ISandBox_ExecutableItem *pos;
    ISandBox_ExecutableItem *add_top;
    SearchFileStatus status;
    char search_file[FILENAME_MAX];

    compiler = Ivyc_create_compiler();
    if (func->package_name == NULL) {
        ISandBox_error_i(caller_exe, caller, pc,
                    DYNAMIC_LOAD_WITHOUT_PACKAGE_ERR,
                    ISandBox_STRING_MESSAGE_ARGUMENT, "name", func->name,
                    ISandBox_MESSAGE_ARGUMENT_END);
    }
    status = Ivyc_dynamic_compile(compiler, func->package_name,
                                 ISandBox->executable_list, &add_top,
                                 search_file);
    if (status != SEARCH_FILE_SUCCESS) {
        if (status == SEARCH_FILE_NOT_FOUND) {
            ISandBox_error_i(caller_exe, caller, pc,
                        LOAD_FILE_NOT_FOUND_ERR,
                        ISandBox_STRING_MESSAGE_ARGUMENT, "file", search_file,
                        ISandBox_MESSAGE_ARGUMENT_END);
        } else {
            ISandBox_error_i(caller_exe, caller, pc,
                        LOAD_FILE_ERR,
                        ISandBox_INT_MESSAGE_ARGUMENT, "status", (int)status,
                        ISandBox_MESSAGE_ARGUMENT_END);
        }
    }

    for (pos = add_top; pos; pos = pos->next) {
        ExecutableEntry *ee;

        ee = add_executable_to_ISandBox(ISandBox, pos->executable, ISandBox_FALSE);
        /*initialize_constant(ISandBox, ee);*/
    }
    Ivyc_dispose_compiler(compiler);
}
