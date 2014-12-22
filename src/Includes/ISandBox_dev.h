#ifndef ISandBox_DEV_H_INCLUDED
#define ISandBox_DEV_H_INCLUDED
#include "ISandBox.h"
#include "ISandBox_code.h"

typedef struct ISandBox_Array_tag ISandBox_Array;
typedef struct ISandBox_Context_tag ISandBox_Context;

typedef enum {
    ISandBox_SUCCESS = 1,
    ISandBox_ERROR
} ISandBox_ErrorStatus;

#define ISandBox_is_null(value) ((value).object.data == NULL)
#define ISandBox_is_null_object(obj) ((obj).data == NULL)

typedef ISandBox_Value ISandBox_NativeFunctionProc(ISandBox_VirtualMachine *ISandBox,
                                         ISandBox_Context *context,
                                         int arg_count, ISandBox_Value *args);

typedef void ISandBox_NativePointerFinalizeProc(ISandBox_VirtualMachine *ISandBox,
                                           ISandBox_Object *obj);

typedef struct ISandBox_NativePointerInfo_tag {
    char                                *name;
    ISandBox_NativePointerFinalizeProc       *finalizer;
    struct ISandBox_NativePointerInfo_tag	*super_class;
} ISandBox_NativePointerInfo;

typedef enum {
    ISandBox_INT_MESSAGE_ARGUMENT = 1,
    ISandBox_DOUBLE_MESSAGE_ARGUMENT,
    ISandBox_LONG_DOUBLE_MESSAGE_ARGUMENT,
    ISandBox_STRING_MESSAGE_ARGUMENT,
    ISandBox_CHARACTER_MESSAGE_ARGUMENT,
    ISandBox_POINTER_MESSAGE_ARGUMENT,
    ISandBox_MESSAGE_ARGUMENT_END
} ISandBox_MessageArgumentType;

typedef struct {
    char *format;
} ISandBox_ErrorDefinition;

typedef struct {
    ISandBox_ErrorDefinition *message_format;
} ISandBox_NativeLibInfo;

#define ISandBox_Ivory_DEFAULT_PACKAGE_P1  "Ivory"
#define ISandBox_Ivory_DEFAULT_PACKAGE_P2  "lang"
#define ISandBox_Ivory_DEFAULT_PACKAGE \
 (ISandBox_Ivory_DEFAULT_PACKAGE_P1 "." ISandBox_Ivory_DEFAULT_PACKAGE_P2)
#define ISandBox_NULL_POINTER_EXCEPTION_NAME ("NullPointerException")

/* load.c */
int ISandBox_search_class(ISandBox_VirtualMachine *ISandBox, char *package_name, char *name);

/* execute.c */
ISandBox_Context *ISandBox_push_context(ISandBox_VirtualMachine *ISandBox);
void ISandBox_pop_context(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context);
ISandBox_Context *ISandBox_create_context(ISandBox_VirtualMachine *ISandBox);
void ISandBox_dispose_context(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context);
void ISandBox_add_native_function(ISandBox_VirtualMachine *ISandBox,
                             char *package_name, char *func_name,
                             ISandBox_NativeFunctionProc *proc, int arg_count,
                             ISandBox_Boolean is_method,
                             ISandBox_Boolean return_pointer);
ISandBox_Value ISandBox_invoke_delegate(ISandBox_VirtualMachine *ISandBox, ISandBox_Value delegate,
                              ISandBox_Value *args);
/* nativeif.c */
ISandBox_ErrorStatus
ISandBox_array_get_int(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                  int *value, ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_get_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array,
                     int index, double *value,
                     ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_get_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array,
                     int index, long double *value,
                     ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_get_object(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     ISandBox_ObjectRef *value, ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_set_int(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                  int value, ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_set_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     double value, ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_set_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     long double value, ISandBox_ObjectRef *exception_p);
ISandBox_ErrorStatus
ISandBox_array_set_object(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef array, int index,
                     ISandBox_ObjectRef value, ISandBox_ObjectRef *exception_p);
int ISandBox_array_size(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array);
void
ISandBox_array_resize(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int new_size);
void ISandBox_array_insert(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int pos,
                      ISandBox_Value value);
void ISandBox_array_remove(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *array, int pos);
int ISandBox_string_length(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *string);
ISandBox_Char *ISandBox_string_get_string(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *string);
ISandBox_ErrorStatus
ISandBox_string_get_character(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef string,
                         int index, ISandBox_Char *ch, ISandBox_ObjectRef *exception);
ISandBox_Value ISandBox_string_substr(ISandBox_VirtualMachine *ISandBox, ISandBox_Object *str,
                            int pos, int len);
ISandBox_Value ISandBox_object_type(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef *obj);
int ISandBox_get_field_index(ISandBox_VirtualMachine *ISandBox, ISandBox_ObjectRef obj,
                        char *field_name);
ISandBox_Value ISandBox_get_field(ISandBox_ObjectRef obj, int index);
void ISandBox_set_field(ISandBox_ObjectRef obj, int index, ISandBox_Value value);

void ISandBox_set_exception(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                       ISandBox_NativeLibInfo *lib_info,
                       char *package_name, char *class_name,
                       int error_id, ...);
void ISandBox_set_null(ISandBox_Value *value);
ISandBox_ObjectRef ISandBox_up_cast(ISandBox_ObjectRef obj, int target_index);

ISandBox_Value ISandBox_check_exception(ISandBox_VirtualMachine *ISandBox);

/* heap.c */
void ISandBox_check_gc(ISandBox_VirtualMachine *ISandBox);
void ISandBox_add_reference_to_context(ISandBox_Context *context, ISandBox_Value value);
ISandBox_ObjectRef
ISandBox_create_iterator_i(ISandBox_VirtualMachine *ISandBox, ISandBox_Array array);
ISandBox_ObjectRef
ISandBox_create_iterator(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     ISandBox_Array array);
ISandBox_Value
ISandBox_get_iterator_currrent(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                     ISandBox_ObjectRef iter);
ISandBox_ObjectRef
ISandBox_create_array_int(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                     int size);
ISandBox_ObjectRef
ISandBox_create_array_double(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int size);
ISandBox_ObjectRef
ISandBox_create_array_long_double(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int size);
ISandBox_ObjectRef
ISandBox_create_array_object(ISandBox_VirtualMachine *ISandBox,  ISandBox_Context *context,
                        int size);
void *ISandBox_object_get_native_pointer(ISandBox_Object *obj);
ISandBox_Boolean ISandBox_check_native_pointer_type(ISandBox_Object *native_pointer,
                                          ISandBox_NativePointerInfo *info);
void ISandBox_object_set_native_pointer(ISandBox_Object *obj, void *p);
ISandBox_ObjectRef
ISandBox_create_ISandBox_string(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                      ISandBox_Char *str);
ISandBox_ObjectRef
ISandBox_create_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context, ISandBox_Value object, int from_type);
ISandBox_ObjectRef
ISandBox_create_class_object(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                        int class_index);
ISandBox_ObjectRef
ISandBox_create_native_pointer(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                          void *pointer, ISandBox_NativePointerInfo *info);

/* wchar.c */
char *ISandBox_wcstombs(const wchar_t *src);
ISandBox_Char *ISandBox_mbstowcs_s(const char *src);

#endif /* ISandBox_DEV_H_INCLUDED */
