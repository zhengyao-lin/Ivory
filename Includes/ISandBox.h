#ifndef PUBLIC_ISandBox_H_INCLUDED
#define PUBLIC_ISandBox_H_INCLUDED

typedef struct ISandBox_Executable_tag ISandBox_Executable;
typedef struct ISandBox_ExecutableList_tag ISandBox_ExecutableList;
typedef struct ISandBox_VirtualMachine_tag ISandBox_VirtualMachine;

typedef struct ISandBox_Object_tag ISandBox_Object;
typedef struct ISandBox_String_tag ISandBox_String;

typedef enum {
    ISandBox_FALSE = 0,
    ISandBox_TRUE = 1
} ISandBox_Boolean;

typedef struct ISandBox_VTable_tag ISandBox_VTable;

typedef struct {
    ISandBox_VTable *v_table;
    ISandBox_Object *data;
} ISandBox_ObjectRef;

typedef union {
    int                      int_value;
    double                   double_value;
    long double              long_double_value;
    ISandBox_ObjectRef       object;
} ISandBox_Value;

ISandBox_VirtualMachine *ISandBox_create_virtual_machine(void);
void ISandBox_set_executable(ISandBox_VirtualMachine *ISandBox, ISandBox_ExecutableList *list);
ISandBox_Value ISandBox_execute(ISandBox_VirtualMachine *ISandBox);
void ISandBox_dispose_executable_list(ISandBox_ExecutableList *list);
void ISandBox_dispose_virtual_machine(ISandBox_VirtualMachine *ISandBox);

#endif /* PUBLIC_ISandBox_CODE_H_INCLUDED */
