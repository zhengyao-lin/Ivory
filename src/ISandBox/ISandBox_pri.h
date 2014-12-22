#ifndef ISandBox_PRI_H_INCLUDED
#define ISandBox_PRI_H_INCLUDED
#include "ISandBox_code.h"
#include "ISandBox_dev.h"
#include "share.h"
#include <stdarg.h>

/************************************THRESHOLD*****************************************/
#define STACK_ALLOC_SIZE (4096)
#define HEAP_THRESHOLD_SIZE	(1024 * 256)
#define ARRAY_ALLOC_SIZE (256)
/************************************THRESHOLD*****************************************/

#define NULL_STRING (L"null")
#define TRUE_STRING (L"true")
#define FALSE_STRING (L"false")
#define BUILT_IN_METHOD_PACKAGE_NAME ("$built_in")

#define NO_LINE_NUMBER_PC (-1)
#define FUNCTION_NOT_FOUND (-1)
#define ENUM_NOT_FOUND (-1)
#define CONSTANT_NOT_FOUND (-1)
#define FIELD_NOT_FOUND (-1)
#define CALL_FROM_NATIVE (-1)

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE (1024)

#define GET_2BYTE_INT(p) (((p)[0] << 8) + (p)[1])
#define SET_2BYTE_INT(p, value) \
  (((p)[0] = (value) >> 8), ((p)[1] = value & 0xff))

#define is_pointer_type(type) \
  ((type)->basic_type == ISandBox_STRING_TYPE \
   || (type)->basic_type == ISandBox_CLASS_TYPE \
   || (type)->basic_type == ISandBox_NULL_TYPE \
   || (type)->basic_type == ISandBox_DELEGATE_TYPE \
   || ((type)->derive_count > 0 \
       && (type)->derive[0].tag == ISandBox_ARRAY_DERIVE))

#define is_object_null(obj) ((obj).data == NULL)

typedef enum {
    BAD_MULTIBYTE_CHARACTER_ERR = 1,
    FUNCTION_NOT_FOUND_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    INDEX_OUT_OF_BOUNDS_ERR,
    DIVISION_BY_ZERO_ERR,
    NULL_POINTER_ERR,
    LOAD_FILE_NOT_FOUND_ERR,
    LOAD_FILE_ERR,
    CLASS_MULTIPLE_DEFINE_ERR,
    CLASS_NOT_FOUND_ERR,
    CLASS_CAST_ERR,
    ENUM_MULTIPLE_DEFINE_ERR,
    CONSTANT_MULTIPLE_DEFINE_ERR,
    DYNAMIC_LOAD_WITHOUT_PACKAGE_ERR,
    RUNTIME_ERROR_COUNT_PLUS_1
} RuntimeError;

typedef struct {
    ISandBox_Char    *string;
} VString;

typedef enum {
    NATIVE_FUNCTION,
    Ivory_FUNCTION
} FunctionKind;

typedef struct {
    ISandBox_NativeFunctionProc *proc;
    int arg_count;
    ISandBox_Boolean is_method;
    ISandBox_Boolean return_pointer;
} NativeFunction;

typedef struct ExecutableEntry_tag ExecutableEntry;

typedef struct {
    ExecutableEntry     *executable;
    int                 index;
} IvoryFunction;

typedef struct {
    char                *package_name;
    char                *name;
    FunctionKind        kind;
    ISandBox_Boolean         is_implemented;
    union {
        NativeFunction native_f;
        IvoryFunction Ivory_f;
    } u;
} Function;

typedef struct {
    Function    *caller;
    int         caller_address;
    int         base;
} CallInfo;

#define revalue_up_align(val)   ((val) ? (((val) - 1) / sizeof(ISandBox_Value) + 1)\
                                 : 0)
#define CALL_INFO_ALIGN_SIZE    (revalue_up_align(sizeof(CallInfo)))

typedef union {
    CallInfo    s;
    ISandBox_Value   u[CALL_INFO_ALIGN_SIZE];
} CallInfoUnion;

typedef struct {
    int         alloc_size;
    int         stack_pointer;
    ISandBox_Value   *stack;
    ISandBox_Boolean *pointer_flags;
} Stack;

typedef enum {
    OBJECT = 1,
    STRING_OBJECT,
    ARRAY_OBJECT,
    CLASS_OBJECT,
    NATIVE_POINTER_OBJECT,
    DELEGATE_OBJECT,
    ITERATOR_OBJECT,
    OBJECT_TYPE_COUNT_PLUS_1
} ObjectType;

struct ISandBox_String_tag {
    ISandBox_Boolean is_literal;
    int         length;
    ISandBox_Char    *string;
};

typedef enum {
    INT_ARRAY = 1,
    DOUBLE_ARRAY,
    LONG_DOUBLE_ARRAY,
    OBJECT_ARRAY,
    FUNCTION_INDEX_ARRAY
} ArrayType;

struct ISandBox_Array_tag {
    ArrayType   type;
    int         size;
    int         alloc_size;
    union {
        int                  *int_array;
        double               *double_array;
        long double          *long_double_array;
        ISandBox_ObjectRef   *object;
        int                  *function_index;
    } u;
};

typedef struct {
    int         field_count;
    ISandBox_Value   *field;
} ISandBox_ClassObject;

typedef struct {
	int cursor;
	ISandBox_Array		array;
} Iterator;

typedef struct {
    void                        *pointer;
    ISandBox_NativePointerInfo       *info;
} NativePointer;

typedef struct {
    ISandBox_ObjectRef       object;
    int                 index; /* if object is null, function index,
                                  else, method index*/
} Delegate;

struct ISandBox_Object_tag {
    ObjectType  type;
	int object_type;
    unsigned int        marked:1;
    union {
        ISandBox_Value                  object;
        ISandBox_String                 string;
        ISandBox_Array                  array;
        ISandBox_ClassObject            class_object;
        NativePointer                   native_pointer;
        Delegate                        delegate;
        Iterator                        iterator;
    } u;
    struct ISandBox_Object_tag *prev;
    struct ISandBox_Object_tag *next;
};

typedef struct {
    int         current_heap_size;
    int         current_threshold;
    ISandBox_Object  *header;
} Heap;

typedef struct {
    int         variable_count;
    ISandBox_Value   *variable;
} Static;

typedef struct ExecClass_tag {
    ISandBox_Class           *ISandBox_class;
    ExecutableEntry     *executable;
    char                *package_name;
    char                *name;
    ISandBox_Boolean         is_implemented;
    int                 class_index;
    struct ExecClass_tag *super_class;
    ISandBox_VTable          *class_table;
    int                 interface_count;
    struct ExecClass_tag **interface;
    ISandBox_VTable          **interface_v_table;
    int                 field_count;
    ISandBox_TypeSpecifier   **field_type;
} ExecClass;

typedef struct {
    char        *name;
    int         index;
} VTableItem;

struct ISandBox_VTable_tag {
    ExecClass   *exec_class;
    int         table_size;
    VTableItem  *table;
};

struct ExecutableEntry_tag {
    ISandBox_Executable *executable;
    int         *function_table;
    int         *class_table;
    int         *enum_table;
    int         *constant_table;
    Static      static_v;
    struct ExecutableEntry_tag *next;
};

typedef struct {
    char        *package_name;
    char        *name;
    ISandBox_Boolean is_defined;
    ISandBox_Enum    *ISandBox_enum;
} Enum;

/*typedef struct {
    char        *package_name;
    char        *name;
    ISandBox_Boolean is_defined;
    ISandBox_Value   value;
} Constant;*/

struct ISandBox_VirtualMachine_tag {
    Stack       stack;
    Heap        heap;
    ExecutableEntry     *current_executable;
    Function    *current_function;
    ISandBox_ObjectRef current_exception;
    int         pc;
    Function    **function;
    int         function_count;
    ExecClass   **class;
    int         class_count;
    Enum        **enums;
    int         enum_count;
    /*Constant    **constant;
    int         constant_count;*/
    ISandBox_ExecutableList  *executable_list;
    ExecutableEntry     *executable_entry;
    ExecutableEntry     *top_level;
    ISandBox_VTable  *array_v_table;
    ISandBox_VTable  *string_v_table;
    ISandBox_VTable  *iterator_v_table;
    ISandBox_Context *current_context;
    ISandBox_Context *free_context;
};

typedef struct RefInNativeFunc_tag {
    ISandBox_ObjectRef  object;
    struct RefInNativeFunc_tag *next;
} RefInNativeFunc;

struct ISandBox_Context_tag {
    RefInNativeFunc     *ref_in_native_method;
    struct ISandBox_Context_tag *next;
};

/* execute.c */
void
ISandBox_expand_stack(ISandBox_VirtualMachine *ISandBox, int need_stack_size);
ISandBox_ObjectRef
ISandBox_create_exception(ISandBox_VirtualMachine *ISandBox, char *class_name,
                     RuntimeError id, ...);
ISandBox_Value
ISandBox_execute_i(ISandBox_VirtualMachine *ISandBox, Function *func,
              ISandBox_Byte *code, int code_size, int base);
void ISandBox_push_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Value value);
ISandBox_Value ISandBox_pop_object(ISandBox_VirtualMachine *ISandBox);

/* heap.c */
ISandBox_ObjectRef
ISandBox_literal_to_ISandBox_string_i(ISandBox_VirtualMachine *inter, ISandBox_Char *str);
ISandBox_ObjectRef
ISandBox_create_ISandBox_string_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Char *str);
ISandBox_ObjectRef ISandBox_create_array_int_i(ISandBox_VirtualMachine *ISandBox, int size);
ISandBox_ObjectRef ISandBox_create_array_double_i(ISandBox_VirtualMachine *ISandBox, int size);
ISandBox_ObjectRef ISandBox_create_array_long_double_i(ISandBox_VirtualMachine *ISandBox, int size);
ISandBox_ObjectRef ISandBox_create_array_object_i(ISandBox_VirtualMachine *ISandBox, int size);
ISandBox_ObjectRef ISandBox_create_class_object_i(ISandBox_VirtualMachine *ISandBox,
                                        int class_index);
ISandBox_ObjectRef ISandBox_cast_object_to_string(ISandBox_VirtualMachine *ISandBox,
                                                 ISandBox_ObjectRef object);
ISandBox_ObjectRef ISandBox_cast_object_to_class(ISandBox_VirtualMachine *ISandBox,
                                                 ISandBox_ObjectRef object);
ISandBox_ObjectRef ISandBox_cast_object_to_delegate(ISandBox_VirtualMachine *ISandBox,
                                                 ISandBox_ObjectRef object);
ISandBox_ObjectRef ISandBox_cast_object_to_native_pointer(ISandBox_VirtualMachine *ISandBox,
                                                 ISandBox_ObjectRef object);
ISandBox_ObjectRef ISandBox_create_object_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Value object, int from_type);
ISandBox_ObjectRef ISandBox_create_delegate(ISandBox_VirtualMachine *ISandBox,
                                  ISandBox_ObjectRef object, int index);
void ISandBox_garbage_collect(ISandBox_VirtualMachine *ISandBox);
/* native.c */
void ISandBox_add_native_functions(ISandBox_VirtualMachine *ISandBox);
/* load.c*/
int ISandBox_search_function(ISandBox_VirtualMachine *ISandBox,
                        char *package_name, char *name);
void ISandBox_dynamic_load(ISandBox_VirtualMachine *ISandBox,
                      ISandBox_Executable *caller_exe, Function *caller, int pc,
                      Function *func);
/* util.c */
void ISandBox_vstr_clear(VString *v);
void ISandBox_vstr_append_string(VString *v, ISandBox_Char *str);
void ISandBox_vstr_append_character(VString *v, ISandBox_Char ch);
void ISandBox_initialize_value(ISandBox_TypeSpecifier *type, ISandBox_Value *value);

/* error.c */
void ISandBox_error_i(ISandBox_Executable *exe, Function *func,
                 int pc, RuntimeError id, ...);
void ISandBox_error_n(ISandBox_VirtualMachine *ISandBox, RuntimeError id, ...);
int ISandBox_conv_pc_to_line_number(ISandBox_Executable *exe, Function *func, int pc);
void ISandBox_format_message(ISandBox_ErrorDefinition *error_definition,
                        int id, VString *message, va_list ap);
#ifdef IVY_WINDOWS
/* windows.c */
void ISandBox_add_windows_native_functions(ISandBox_VirtualMachine *ISandBox);
#endif /* IVY_WINDOWS */

extern OpcodeInfo ISandBox_opcode_info[];
extern ISandBox_ObjectRef ISandBox_null_object_ref;
extern ISandBox_ErrorDefinition ISandBox_error_message_format[];

#endif /* ISandBox_PRI_H_INCLUDED */
