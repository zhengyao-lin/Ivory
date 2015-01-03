#include <stdio.h>
#include <string.h>
#include "DBG.h"
#include "share.h"

extern OpcodeInfo ISandBox_opcode_info[];

static void
dump_constant_pool(int constant_pool_count, ISandBox_ConstantPool *constant_pool)
{
    int i;

    printf("** constant pool section ***********************************\n");
    for (i = 0; i < constant_pool_count; i++) {
        printf("%05d:", i);
        switch (constant_pool[i].tag) {
        case ISandBox_CONSTANT_INT:
            printf("int %d\n", constant_pool[i].u.c_int);
            break;
        case ISandBox_CONSTANT_DOUBLE:
            printf("double %f\n", constant_pool[i].u.c_double);
            break;
        case ISandBox_CONSTANT_LONG_DOUBLE:
            printf("long double %Lf\n", constant_pool[i].u.c_long_double);
            break;
        case ISandBox_CONSTANT_STRING:
            printf("string ");
            ISandBox_print_wcs_ln(stdout, constant_pool[i].u.c_string);
            break;
        default:
            DBG_assert(0, ("tag..%d\n", constant_pool[i].tag));
        }
    }
}

static void dump_type(ISandBox_Executable *exe, ISandBox_TypeSpecifier *type);

static void
dump_parameter_list(ISandBox_Executable *exe,
                    int parameter_count, ISandBox_LocalVariable *parameter_list)
{
    int i;

    printf("(");
    for (i = 0; i < parameter_count; i++) {
        /*
        dump_type(exe, parameter_list[i].type);
        */
        printf(" %s", parameter_list[i].name);
        if (i < parameter_count-1) {
            printf(", ");
        }
    }
    printf(")");
}

static void
dump_type(ISandBox_Executable *exe, ISandBox_TypeSpecifier *type)
{
    int i;

    switch (type->basic_type) {
    case ISandBox_VOID_TYPE:
        printf("void ");
        break;
    case ISandBox_BOOLEAN_TYPE:
        printf("boolean ");
        break;
    case ISandBox_INT_TYPE:
        printf("int ");
        break;
    case ISandBox_DOUBLE_TYPE:
        printf("double ");
        break;
    case ISandBox_LONG_DOUBLE_TYPE:
        printf("long double ");
        break;
    case ISandBox_STRING_TYPE:
        printf("string ");
        break;
    case ISandBox_OBJECT_TYPE:
        printf("object");
        break;
    case ISandBox_ITERATOR_TYPE:
        printf("ArrayIterator");
        break;
    case ISandBox_NATIVE_POINTER_TYPE:
        printf("native_pointer ");
        break;
    case ISandBox_CLASS_TYPE:
        printf("<%s> ", exe->class_definition[type->u.class_t.index].name);
        break;
    case ISandBox_DELEGATE_TYPE:
        printf("delegate ");
        break;
    case ISandBox_ENUM_TYPE:
        printf("<%s> ", exe->enum_definition[type->u.enum_t.index].name);
        break;
    case ISandBox_NULL_TYPE:
        printf("null ");
        break;
    case ISandBox_BASE_TYPE: /* FALLTHRU */
    case ISandBox_UNSPECIFIED_IDENTIFIER_TYPE:
    default:
        DBG_assert(0, ("basic_type..%d\n", type->basic_type));
    }

    for (i = 0; i < type->derive_count; i++) {
        switch (type->derive[i].tag) {
        case ISandBox_FUNCTION_DERIVE:
            dump_parameter_list(exe,
                                type->derive[i].u.function_d.parameter_count,
                                type->derive[i].u.function_d.parameter);
            break;
        case ISandBox_ARRAY_DERIVE:
            printf("[]");
            break;
        default:
            DBG_assert(0, ("derive_tag..%d\n", type->derive->tag));
        }
    }
}

static void
dump_variable(ISandBox_Executable *exe, int global_variable_count,
              ISandBox_Variable *global_variable)
{
    int i;

    printf("** global variable section *********************************\n");
    for (i = 0; i < global_variable_count; i++) {
        printf("%5d:", i);
        dump_type(exe, global_variable[i].type);
        printf(" %s\n", global_variable[i].name);
    }
}

int
ISandBox_dump_instruction(FILE *fp, ISandBox_Byte *code, int index)
{
    OpcodeInfo *info;
    int value;
    int i;

    info = &ISandBox_opcode_info[code[index]];
    fprintf(fp, "%4d %25s ", index, info->mnemonic);
    for (i = 0; info->parameter[i] != '\0'; i++) {
        switch (info->parameter[i]) {
        case 'b':
            value = code[index+1];
            fprintf(fp, " %d", value);
            index++;
            break;
        case 's': /* FALLTHRU */
        case 'p':
            value = (code[index+1] << 8) + code[index+2];
            fprintf(fp, " %d", value);
            index += 2;
            break;
        default:
            DBG_assert(0, ("param..%s, i..%d", info->parameter, i));
        }
    }
    index++;

    return index;
}

static void
dump_opcode(int code_size, ISandBox_Byte *code)
{
    int index;

    for (index = 0; index < code_size; ) {
        index = ISandBox_dump_instruction(stdout, code, index);
        printf("\n");
    }
}

static void
dump_line_number(int line_number_size, ISandBox_LineNumber *line_number)
{
    int i;

    printf("*** line number ***\n");
    for (i = 0; i < line_number_size; i++) {
        printf("%5d: from %5d size %5d\n",
               line_number[i].line_number,
               line_number[i].start_pc,
               line_number[i].pc_count);
    }
}

static void
dump_function(ISandBox_Executable *exe, int function_count, ISandBox_Function *function)
{
    int i;

    printf("** function section ****************************************\n");
    for (i = 0; i < function_count; i++) {
        printf("*** [%d] %s ***\n", i, function[i].name);

        dump_type(exe, function[i].type);
        printf(" %s ", function[i].name);
        dump_parameter_list(exe,
                            function[i].parameter_count,
                            function[i].parameter);
        printf("\n");
        if (function[i].is_implemented) {
            if (function[i].code_block.code_size > 0) {
                dump_opcode(function[i].code_block.code_size,
                            function[i].code_block.code);
                dump_line_number(function[i].code_block.line_number_size,
                                 function[i].code_block.line_number);
            }
        }
        printf("*** end of %s ***\n", function[i].name);
    }
}

static void
dump_types(ISandBox_Executable *exe, int type_count, ISandBox_TypeSpecifier *types)
{
    int i;

    printf("** type section ********************************************\n");
    for (i = 0; i < type_count; i++) {
        printf("%5d:", i);
        dump_type(exe, &types[i]);
        printf("\n");
    }
}

void
ISandBox_disassemble(ISandBox_Executable *exe)
{
    dump_constant_pool(exe->constant_pool_count, exe->constant_pool);
    dump_variable(exe, exe->global_variable_count, exe->global_variable);
    dump_function(exe, exe->function_count, exe->function);
    dump_types(exe, exe->type_specifier_count, exe->type_specifier);
    printf("** toplevel ********************\n");
    dump_opcode(exe->top_level.code_size, exe->top_level.code);
    dump_line_number(exe->top_level.line_number_size,
                     exe->top_level.line_number);
}
