#include <string.h>
#include "ISandBox_pri.h"

ISandBox_ErrorDefinition ISandBox_error_message_format[] = {
    {"dummy"},
    {"error: invalid multibyte characters"},
    {"error: failed to find function $(name)"},
    {"error: the function $(package)#$(name) has been defined already"},
    {"error: array range error!($(index))the size of array is $(size)"},
    {"error: integer division by zero"},
    {"error: value null can't be instantiated"},
    {"error: failed to find file $(file)"},
    {"error: file upload error ($(status))"},
    {"error: the class $(package)#$(name) has been defined already"},
    {"error: failed to find class $(name)"},
    {"error: $(org) is this class.$(target) can't be casted."},
    {"error: the enumeration $(package)#$(name) has been defined already"},
    {"errpr: the constance $(package)#$(name) has been defined already"},
    {"error: the function $(name) can not be found in any used package"},
    {"dummy"}
};

ISandBox_ErrorDefinition ISandBox_native_error_message_format[] = {
    {"native_error: array range error.the size of array is $(size), but position $(pos) is going to insert."},
    {"native_error: array range error.the size of array is $(size), but position $(pos) is going to remove."},
    {"native_error: string range error.the length of string is $(len), but position $(pos) is given."},
    {"native_error: in function substr():\n\tstring range error.the second parameter is longer than the length of string.($(len))"},
    {"native_error: the first argument of fopen is null"},
    {"native_error: the second argument of fopen is null"},
    {"native_error: fgets: the target file has a null pointer"},
    {"fgetsの引数の型が不正です。"},
    {"fgetsに渡されているファイルポインタが無効です。"
     "たぶんclose()されています。"},
    {"fgetsで読み込んだマルチバイト文字列を内部表現に変換できません。"},
    {"fputsの第2引数に渡されているファイルポインタがnullです。"},
    {"fputsの引数の型が不正です。"},
    {"fputsに渡されているファイルポインタが無効です。"
     "たぶんclose()されています。"},
    {"fcloseの引数にnullが渡されています。"},
    {"fcloseの引数の型が不正です。"},
    {"fcloseに渡されているファイルポインタが無効です。"
     "たぶん既にclose()されています。"},
    {"parse_int()の引数がnullです。"},
    {"parse_int()のフォーマットエラー。元の文字列:「$(str)」"},
    {"parse_double()の引数がnullです。"},
    {"parse_double()のフォーマットエラー。元の文字列:「$(str)」"},
    {"parse_long_double(): the parameter is NULL"},
    {"parse_long_double(): wrong format of string: \"$(str)\""},
	{"out of iterator"},
	{"used new iterator without initializing"},
};
