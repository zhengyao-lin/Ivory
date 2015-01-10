#ifndef PUBLIC_ISandBox_CODE_H_INCLUDED
#define PUBLIC_ISandBox_CODE_H_INCLUDED
#include <stdio.h>
#include <wchar.h>
#include "ISandBox.h"

typedef enum {
    ISandBox_VOID_TYPE = 1,
    ISandBox_BOOLEAN_TYPE,
    ISandBox_INT_TYPE,
    ISandBox_DOUBLE_TYPE,
    ISandBox_LONG_DOUBLE_TYPE,
    ISandBox_OBJECT_TYPE,
    ISandBox_STRING_TYPE,
    ISandBox_CLASS_TYPE,
    ISandBox_DELEGATE_TYPE,
    ISandBox_ENUM_TYPE,
    ISandBox_NULL_TYPE,
    ISandBox_NATIVE_POINTER_TYPE,
    ISandBox_BASE_TYPE,
	ISandBox_ITERATOR_TYPE,
	ISandBox_UNCLEAR_TYPE,
	ISandBox_PLACEHOLDER,
    ISandBox_UNSPECIFIED_IDENTIFIER_TYPE
} ISandBox_BasicType;

typedef struct ISandBox_TypeSpecifier_tag ISandBox_TypeSpecifier;

typedef struct {
    char                *name;
    ISandBox_TypeSpecifier   *type;
} ISandBox_LocalVariable;

typedef enum {
    ISandBox_FUNCTION_DERIVE,
    ISandBox_ARRAY_DERIVE
} ISandBox_DeriveTag;

typedef struct {
    int                 parameter_count;
    ISandBox_LocalVariable   *parameter;
} ISandBox_FunctionDerive;

typedef struct {
    int dummy; /* make compiler happy */
} ISandBox_ArrayDerive;

typedef struct ISandBox_TypeDerive_tag {
    ISandBox_DeriveTag       tag;
    union {
        ISandBox_FunctionDerive      function_d;
    } u;
} ISandBox_TypeDerive;

struct ISandBox_TypeSpecifier_tag {
    ISandBox_BasicType       basic_type;
    union {
        struct {
            int index;
        } class_t;
        struct {
            int dummy;
        } delegate_t;
        struct {
            int index;
        } enum_t;
        struct {
            int type;/*equals to enum ISandBox_BasicType*/
        } object_t;
    } u;
    int                 derive_count;
    ISandBox_TypeDerive      *derive;
};

typedef wchar_t ISandBox_Char;
typedef unsigned char ISandBox_Byte;

typedef enum {
    ISandBox_PUSH_INT_1BYTE = 1,
    ISandBox_PUSH_INT_2BYTE,
    ISandBox_PUSH_INT,
    ISandBox_PUSH_DOUBLE_0,
    ISandBox_PUSH_DOUBLE_1,
    ISandBox_PUSH_DOUBLE,
    /**********/
    ISandBox_PUSH_STRING,
    ISandBox_PUSH_NULL,

    ISandBox_PUSH_LONG_DOUBLE_0,
    ISandBox_PUSH_LONG_DOUBLE_1,
    ISandBox_PUSH_LONG_DOUBLE,
    /**********/
    ISandBox_PUSH_STACK_INT,
    ISandBox_PUSH_STACK_DOUBLE,
    ISandBox_PUSH_STACK_OBJECT,
    ISandBox_PUSH_STACK_LONG_DOUBLE,
    ISandBox_POP_STACK_INT,
    ISandBox_POP_STACK_DOUBLE,
    ISandBox_POP_STACK_OBJECT,
    ISandBox_POP_STACK_LONG_DOUBLE,
    /**********/
    ISandBox_PUSH_STATIC_INT,
    ISandBox_PUSH_STATIC_DOUBLE,
    ISandBox_PUSH_STATIC_OBJECT,
    ISandBox_PUSH_STATIC_LONG_DOUBLE,
    ISandBox_POP_STATIC_INT,
    ISandBox_POP_STATIC_DOUBLE,
    ISandBox_POP_STATIC_OBJECT,
    ISandBox_POP_STATIC_LONG_DOUBLE,
    /**********/
    ISandBox_PUSH_CONSTANT_INT,
    ISandBox_PUSH_CONSTANT_DOUBLE,
    ISandBox_PUSH_CONSTANT_OBJECT,
    ISandBox_PUSH_CONSTANT_LONG_DOUBLE,
    ISandBox_POP_CONSTANT_INT,
    ISandBox_POP_CONSTANT_DOUBLE,
    ISandBox_POP_CONSTANT_OBJECT,
    ISandBox_POP_CONSTANT_LONG_DOUBLE,
    /**********/
    ISandBox_PUSH_ARRAY_INT,
    ISandBox_PUSH_ARRAY_DOUBLE,
    ISandBox_PUSH_ARRAY_OBJECT,
    ISandBox_PUSH_ARRAY_LONG_DOUBLE,
    ISandBox_POP_ARRAY_INT,
    ISandBox_POP_ARRAY_DOUBLE,
    ISandBox_POP_ARRAY_OBJECT,
    ISandBox_POP_ARRAY_LONG_DOUBLE,
    /**********/
    ISandBox_PUSH_CHARACTER_IN_STRING,
    /**********/
    ISandBox_PUSH_FIELD_INT,
    ISandBox_PUSH_FIELD_DOUBLE,
    ISandBox_PUSH_FIELD_OBJECT,
    ISandBox_PUSH_FIELD_LONG_DOUBLE,
    ISandBox_POP_FIELD_INT,
    ISandBox_POP_FIELD_DOUBLE,
    ISandBox_POP_FIELD_OBJECT,
    ISandBox_POP_FIELD_LONG_DOUBLE,
    /**********/
    ISandBox_ADD_INT,
    ISandBox_ADD_DOUBLE,
    ISandBox_ADD_STRING,
    ISandBox_ADD_LONG_DOUBLE,
    ISandBox_SUB_INT,
    ISandBox_SUB_DOUBLE,
    ISandBox_SUB_EMPTY,
    ISandBox_SUB_LONG_DOUBLE,
    ISandBox_MUL_INT,
    ISandBox_MUL_DOUBLE,
    ISandBox_MUL_EMPTY,
    ISandBox_MUL_LONG_DOUBLE,
    ISandBox_DIV_INT,
    ISandBox_DIV_DOUBLE,
    ISandBox_DIV_EMPTY,
    ISandBox_DIV_LONG_DOUBLE,
    ISandBox_MOD_INT,
    ISandBox_MOD_DOUBLE,
    ISandBox_MOD_EMPTY,
    ISandBox_MOD_LONG_DOUBLE,
    ISandBox_BIT_AND,
    ISandBox_BIT_OR,
    ISandBox_BIT_XOR,
    ISandBox_MINUS_INT,
    ISandBox_MINUS_DOUBLE,
    ISandBox_MINUS_EMPTY,
    ISandBox_MINUS_LONG_DOUBLE,
    ISandBox_BIT_NOT,
    ISandBox_INCREMENT,
    ISandBox_DECREMENT,
    ISandBox_CAST_INT_TO_DOUBLE,
    ISandBox_CAST_INT_TO_LONG_DOUBLE,
    ISandBox_CAST_DOUBLE_TO_INT,
    ISandBox_CAST_LONG_DOUBLE_TO_INT,
    ISandBox_CAST_LONG_DOUBLE_TO_DOUBLE,
    ISandBox_CAST_DOUBLE_TO_LONG_DOUBLE,
    ISandBox_CAST_BOOLEAN_TO_STRING,
    ISandBox_CAST_INT_TO_STRING,
    ISandBox_CAST_DOUBLE_TO_STRING,
    ISandBox_CAST_LONG_DOUBLE_TO_STRING,
	ISandBox_CAST_ENUM_TO_STRING,
    ISandBox_CAST_ALL_TO_OBJECT,
    /* object cast */
    ISandBox_CAST_OBJECT_TO_STRING,
    ISandBox_CAST_OBJECT_TO_INT,
    ISandBox_CAST_OBJECT_TO_DOUBLE,
    ISandBox_CAST_OBJECT_TO_LONG_DOUBLE,
    ISandBox_CAST_OBJECT_TO_BOOLEAN,
    ISandBox_CAST_OBJECT_TO_CLASS,
    ISandBox_CAST_OBJECT_TO_DELEGATE,
	ISandBox_CAST_OBJECT_TO_NATIVE_POINTER,
	ISandBox_CAST_OBJECT_TO_ARRAY,
	ISandBox_UNBOX_OBJECT,

    ISandBox_UP_CAST,
    ISandBox_DOWN_CAST,
    ISandBox_EQ_INT,
    ISandBox_EQ_DOUBLE,
    ISandBox_EQ_OBJECT,
    ISandBox_EQ_STRING,
    ISandBox_EQ_LONG_DOUBLE,
    ISandBox_GT_INT,
    ISandBox_GT_DOUBLE,
    ISandBox_GT_STRING,
    ISandBox_GT_LONG_DOUBLE,
    ISandBox_GE_INT,
    ISandBox_GE_DOUBLE,
    ISandBox_GE_STRING,
    ISandBox_GE_LONG_DOUBLE,
    ISandBox_LT_INT,
    ISandBox_LT_DOUBLE,
    ISandBox_LT_STRING,
    ISandBox_LT_LONG_DOUBLE,
    ISandBox_LE_INT,
    ISandBox_LE_DOUBLE,
    ISandBox_LE_STRING,
    ISandBox_LE_LONG_DOUBLE,
    ISandBox_NE_INT,
    ISandBox_NE_DOUBLE,
    ISandBox_NE_OBJECT,
    ISandBox_NE_STRING,
    ISandBox_NE_LONG_DOUBLE,
    ISandBox_LOGICAL_AND,
    ISandBox_LOGICAL_OR,
    ISandBox_LOGICAL_NOT,
    ISandBox_POP,
    ISandBox_DUPLICATE,
    ISandBox_DUPLICATE_OFFSET,
    ISandBox_JUMP,
    ISandBox_JUMP_IF_TRUE,
    ISandBox_JUMP_IF_FALSE,
    /**********/
    ISandBox_PUSH_FUNCTION,
    ISandBox_PUSH_METHOD,
    ISandBox_PUSH_DELEGATE,
    ISandBox_PUSH_METHOD_DELEGATE,
    ISandBox_INVOKE,
    ISandBox_INVOKE_DELEGATE,
    ISandBox_RETURN,
    /**********/
    ISandBox_NEW,
    ISandBox_NEW_ARRAY,
    ISandBox_NEW_ARRAY_LITERAL_INT,
    ISandBox_NEW_ARRAY_LITERAL_DOUBLE,
    ISandBox_NEW_ARRAY_LITERAL_OBJECT,
    ISandBox_NEW_ARRAY_LITERAL_LONG_DOUBLE,
    ISandBox_SUPER,
    ISandBox_INSTANCEOF,
    ISandBox_ISTYPE,
    ISandBox_THROW,
    ISandBox_RETHROW,
    ISandBox_GO_FINALLY,
    ISandBox_FINALLY_END,
    /*ISandBox_LABEL,*/
    ISandBox_GOTO
} ISandBox_Opcode;

typedef enum {
    ISandBox_CONSTANT_INT,
    ISandBox_CONSTANT_DOUBLE,
    ISandBox_CONSTANT_STRING,
    ISandBox_CONSTANT_LONG_DOUBLE
} ISandBox_ConstantPoolTag;

typedef struct {
    ISandBox_ConstantPoolTag tag;
    union {
        int             c_int;
        double          c_double;
        long double     c_long_double;
        ISandBox_Char   *c_string;
    } u;
} ISandBox_ConstantPool;

typedef struct {
    char                *name;
    ISandBox_TypeSpecifier   *type;
} ISandBox_Variable;

typedef struct {
    int line_number;
    int start_pc;
    int pc_count;
} ISandBox_LineNumber;

typedef struct {
    int class_index;
    int start_pc;
    int end_pc;
} ISandBox_CatchClause;

typedef struct {
    int                 try_start_pc;
    int                 try_end_pc;
    int                 catch_count;
    ISandBox_CatchClause     *catch_clause;
    int                 finally_start_pc;
    int                 finally_end_pc;
} ISandBox_Try;

typedef struct {
    int                 code_size;
    ISandBox_Byte            *code;
    int                 line_number_size;
    ISandBox_LineNumber      *line_number;
    int                 try_size;
    ISandBox_Try             *try;
    int                 need_stack_size;
} ISandBox_CodeBlock;

typedef struct {
    ISandBox_TypeSpecifier   *type;
    char                *package_name;
    char                *name;
    int                 parameter_count;
    ISandBox_LocalVariable   *parameter;
    ISandBox_Boolean         is_implemented;
    ISandBox_Boolean         is_method;
    int                 local_variable_count;
    ISandBox_LocalVariable   *local_variable;
    ISandBox_CodeBlock       code_block;
} ISandBox_Function;

typedef enum {
    ISandBox_FILE_ACCESS,
    ISandBox_PUBLIC_ACCESS,
    ISandBox_PRIVATE_ACCESS
} ISandBox_AccessModifier;

typedef struct {
    ISandBox_AccessModifier  access_modifier;
    char                *name;
    ISandBox_TypeSpecifier   *type;
} ISandBox_Field;

typedef struct {
    ISandBox_AccessModifier  access_modifier;
    ISandBox_Boolean         is_abstract;
    ISandBox_Boolean         is_virtual;
    ISandBox_Boolean         is_override;
    char                *name;
} ISandBox_Method;

typedef enum {
    ISandBox_CLASS_DEFINITION,
    ISandBox_INTERFACE_DEFINITION
} ISandBox_ClassOrInterface;

typedef struct {
    char *package_name;
    char *name;
} ISandBox_ClassIdentifier;

typedef struct {
    ISandBox_Boolean                 is_abstract;
    ISandBox_AccessModifier          access_modifier;
    ISandBox_ClassOrInterface        class_or_interface;
    char                        *package_name;
    char                        *name;
    ISandBox_Boolean                 is_implemented;
    ISandBox_ClassIdentifier         *super_class;
    int                         interface_count;
    ISandBox_ClassIdentifier         *interface_;
    int                         field_count;
    ISandBox_Field                   *field;
    int                         method_count;
    ISandBox_Method                  *method;
    ISandBox_CodeBlock               field_initializer;
} ISandBox_Class;

typedef struct {
    char        *package_name;
    char        *name;
    ISandBox_Boolean is_defined;
    int         enumerator_count;
    char        **enumerator;
	int         *value;
} ISandBox_Enum;

/*typedef struct {
    ISandBox_TypeSpecifier *type;
    char        *package_name;
    char        *name;
    ISandBox_Boolean is_defined;
} ISandBox_Constant;*/

struct ISandBox_Executable_tag {
    char                *package_name;
    ISandBox_Boolean         is_usingd;
    char                *path;
    int                 constant_pool_count;
    ISandBox_ConstantPool    *constant_pool;
    int                 global_variable_count;
    ISandBox_Variable        *global_variable;
    int                 function_count;
    ISandBox_Function        *function;
    int                 type_specifier_count;
    ISandBox_TypeSpecifier   *type_specifier;
    int                 class_count;
    ISandBox_Class           *class_definition;
    int                 enum_count;
    ISandBox_Enum            *enum_definition;
    ISandBox_CodeBlock       top_level;
    ISandBox_CodeBlock       constant_initializer;
};

typedef struct ISandBox_ExecutableItem_tag {
    ISandBox_Executable *executable;
    struct ISandBox_ExecutableItem_tag *next;
} ISandBox_ExecutableItem;

struct ISandBox_ExecutableList_tag {
    ISandBox_Executable      *top_level;
    ISandBox_ExecutableItem  *list;
};

#endif /* PUBLIC_ISandBox_CODE_H_INCLUDED */
