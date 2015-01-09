#ifndef PRIVATE_SHARE_H_INCLUDED
#define PRIVATE_SHARE_H_INCLUDED
#include <stdio.h>
#include "ISandBox_code.h"
#include "Ivyc.h"

typedef struct {
    char        *mnemonic;
    char        *parameter;
    int         stack_increment;
} OpcodeInfo;

typedef enum {
    SEARCH_FILE_SUCCESS,
    SEARCH_FILE_NOT_FOUND,
    SEARCH_FILE_PATH_TOO_LONG
} SearchFileStatus;

#ifdef IVY_WINDOWS
#define FILE_SEPARATOR          ('\\')
#define FILE_PATH_SEPARATOR     (';')
#else
#define FILE_SEPARATOR          ('/')
#define FILE_PATH_SEPARATOR     (':')
#endif

#define Ivory_USING_FILE_DEFAULT_PATH    ("IVY_USING_SEARCH_PATH")
#define Ivory_RUNTIME_PATH_NAME    ("IVY_RUNTIME_PATH")
#define Ivory_RUNTIME_PATH_HEAD    ("IVY_RUNTIME_PATH=")
#define Ivory_USING_SUFFIX    (".ivh")
#define Ivory_IMPLEMENTATION_SUFFIX    (".ivy")
#define Ivory_STACK_TRACE_CLASS ("StackTrace")
#define Ivory_PRINT_STACK_TRACE_FUNC ("Exception#print_stack_trace")
#define EXCEPTION_CLASS_NAME ("Exception")
#define BUG_EXCEPTION_CLASS_NAME ("BugException")
#define RUNTIME_EXCEPTION_CLASS_NAME ("RuntimeException")
#define ARRAY_INDEX_EXCEPTION_NAME ("ArrayIndexOutOfBoundsException")
#define STRING_INDEX_EXCEPTION_NAME ("StringIndexOutOfBoundsException")
#define DIVISION_BY_ZERO_EXCEPTION_NAME ("DivisionByZeroException")
#define CLASS_CAST_EXCEPTION_NAME ("ClassCastException")
#define MULTIBYTE_CONVERTION_EXCEPTION_NAME \
  ("MultibyteCharacterConvertionException")
#define NUMBER_FORMAT_EXCEPTION_NAME ("NumberFormatException")
#define OUT_OF_ITERATOR_EXCEPTION_NAME ("OutOfIteratorException")
#define NEW_ITERATOR_EXCEPTION_NAME ("NewIteratorException")

#define ARRAY_METHOD_SIZE "size"
#define ARRAY_METHOD_RESIZE "resize"
#define ARRAY_METHOD_INSERT "insert"
#define ARRAY_METHOD_REMOVE "remove"
#define ARRAY_METHOD_ADD "add"
#define ARRAY_METHOD_ITERATOR "iterator"
#define ARRAY_PREFIX "array#"

#define STRING_METHOD_LENGTH "length"
#define STRING_METHOD_SUBSTR "substr"
#define STRING_PREFIX "string#"

#define ITERATOR_METHOD_NEXT "next"
#define ITERATOR_METHOD_HASNEXT "hasNext"
#define ITERATOR_METHOD_CURRENT "current"
#define ITERATOR_PREFIX "iterator#"

#define ARRAY_SIZE(array)  (sizeof(array) / sizeof((array)[0]))

/* dispose.c */
void ISandBox_dispose_executable(ISandBox_Executable *exe);
/* wchar.c */
size_t ISandBox_wcslen(wchar_t *str);
wchar_t *ISandBox_wcscpy(wchar_t *dest, wchar_t *src);
wchar_t *ISandBox_wcsncpy(wchar_t *dest, wchar_t *src, size_t n);
int ISandBox_wcscmp(wchar_t *s1, wchar_t *s2);
wchar_t *ISandBox_wcscat(wchar_t *s1, wchar_t *s2);
int ISandBox_mbstowcs_len(const char *src);
void ISandBox_mbstowcs(const char *src, wchar_t *dest);
wchar_t *ISandBox_mbstowcs_alloc(ISandBox_VirtualMachine *ISandBox, const char *src);
int ISandBox_wcstombs_len(const wchar_t *src);
void ISandBox_wcstombs_i(const wchar_t *src, char *dest);
char *ISandBox_wcstombs_alloc(const wchar_t *src);
char ISandBox_wctochar(wchar_t src);
int ISandBox_print_wcs(FILE *fp, wchar_t *str);
int ISandBox_print_wcs_ln(FILE *fp, wchar_t *str);
ISandBox_Boolean ISandBox_iswdigit(wchar_t ch);
/* disassemble.c */
int ISandBox_dump_instruction(FILE *fp, ISandBox_Byte *code, int index);
void ISandBox_disassemble(ISandBox_Executable *exe);
/* util.c */
char *Ivory_get_current_path(void);
void Ivory_set_current_path(char *path);
SearchFileStatus
ISandBox_search_file(char *search_path, char *search_file,
                char *found_path, FILE **fp);
ISandBox_Boolean ISandBox_compare_string(char *str1, char *str2);
ISandBox_Boolean ISandBox_compare_package_name(char *p1, char *p2);
char *
ISandBox_create_method_function_name(char *class_name, char *method_name);
void ISandBox_strncpy(char *dest, char *src, int buf_size);

SearchFileStatus Ivyc_dynamic_compile(Ivyc_Compiler *compiler,
                                     char *package_name,
                                     ISandBox_ExecutableList *list,
                                     ISandBox_ExecutableItem **add_top,
                                     char *search_file);
char *ISandBox_get_folder_by_path(char *path);
char *ISandBox_get_absolute_path(char *relative_path);

#endif  /* PRIVATE_SHARE_H_INCLUDED */
