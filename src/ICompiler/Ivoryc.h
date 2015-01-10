#ifndef PRIVATE_IvoryC_H_INCLUDED
#define PRIVATE_IvoryC_H_INCLUDED
#include <stdio.h>
#include <setjmp.h>
#include <wchar.h>
#include "../Includes/MEM.h"
#include "../Includes/Ivyc.h"
#include "../Includes/ISandBox_code.h"
#include "../Includes/share.h"

#define DEFAULT_CONSTRUCTOR_NAME ("initialize")

#define smaller(a, b) ((a) < (b) ? (a) : (b))
#define larger(a, b) ((a) > (b) ? (a) : (b))

#define MESSAGE_ARGUMENT_MAX    (256)
#define LINE_BUF_SIZE           (1024)

#define UTF8_ALLOC_LEN (256)

#define UNDEFINED_LABEL (-1)
#define UNDEFINED_ENUMERATOR (-1)
#define ABSTRACT_METHOD_INDEX (-1)

#define UNDEFINED_VARIABLE_ARGS "__VARGS"

typedef enum {
    INT_MESSAGE_ARGUMENT = 1,
    DOUBLE_MESSAGE_ARGUMENT,
    LONG_DOUBLE_MESSAGE_ARGUMENT,
    STRING_MESSAGE_ARGUMENT,
    CHARACTER_MESSAGE_ARGUMENT,
    POINTER_MESSAGE_ARGUMENT,
    MESSAGE_ARGUMENT_END
} MessageArgumentType;

typedef struct {
    char *format;
} ErrorDefinition;

typedef enum {
    PARSE_ERR = 1,
    CHARACTER_INVALID_ERR,
    FUNCTION_MULTIPLE_DEFINE_ERR,
    BAD_MULTIBYTE_CHARACTER_ERR,
    UNEXPECTED_WIDE_STRING_IN_COMPILE_ERR,
    ARRAY_ELEMENT_CAN_NOT_BE_FINAL_ERR,
    COMPLEX_ASSIGNMENT_OPERATOR_TO_FINAL_ERR,
    PARAMETER_MULTIPLE_DEFINE_ERR,
    VARIABLE_MULTIPLE_DEFINE_ERR,
    IDENTIFIER_NOT_FOUND_ERR,
    FUNCTION_IDENTIFIER_ERR,
    DERIVE_TYPE_CAST_ERR,
    CAST_MISMATCH_ERR,
    MATH_TYPE_MISMATCH_ERR,
    COMPARE_TYPE_MISMATCH_ERR,
    LOGICAL_TYPE_MISMATCH_ERR,
    MINUS_TYPE_MISMATCH_ERR,
    LOGICAL_NOT_TYPE_MISMATCH_ERR,
    INC_DEC_TYPE_MISMATCH_ERR,
    FUNCTION_NOT_IDENTIFIER_ERR, /* BUGBUG not used */
    FUNCTION_NOT_FOUND_ERR,
    ARGUMENT_COUNT_MISMATCH_ERR,
    NOT_LVALUE_ERR,
    LABEL_NOT_FOUND_ERR,
    ARRAY_LITERAL_EMPTY_ERR,
    INDEX_LEFT_OPERAND_NOT_ARRAY_ERR,
    INDEX_NOT_INT_ERR,
    ARRAY_SIZE_NOT_INT_ERR,
    DIVISION_BY_ZERO_IN_COMPILE_ERR,
    PACKAGE_NAME_TOO_LONG_ERR,
    USING_FILE_NOT_FOUND_ERR,
    USING_FILE_ERR,
    USING_DUPLICATE_ERR,
    RENAME_HAS_NO_PACKAGED_NAME_ERR,
    ABSTRACT_MULTIPLE_SPECIFIED_ERR,
    ACCESS_MODIFIER_MULTIPLE_SPECIFIED_ERR,
    OVERRIDE_MODIFIER_MULTIPLE_SPECIFIED_ERR,
    VIRTUAL_MODIFIER_MULTIPLE_SPECIFIED_ERR,
    MEMBER_EXPRESSION_TYPE_ERR,
    MEMBER_NOT_FOUND_ERR,
    PRIVATE_MEMBER_ACCESS_ERR,
    ABSTRACT_METHOD_HAS_BODY_ERR,
    CONCRETE_METHOD_HAS_NO_BODY_ERR,
    MULTIPLE_INHERITANCE_ERR,
    INHERIT_CONCRETE_CLASS_ERR,
    NEW_ABSTRACT_CLASS_ERR,
    RETURN_IN_VOID_FUNCTION_ERR,
    CLASS_NOT_FOUND_ERR,
    CONSTRUCTOR_IS_FIELD_ERR,
    NOT_CONSTRUCTOR_ERR,
    FIELD_CAN_NOT_CALL_ERR,
    ASSIGN_TO_METHOD_ERR,
    NON_VIRTUAL_METHOD_OVERRIDED_ERR,
    NEED_OVERRIDE_ERR,
    ABSTRACT_METHOD_IN_CONCRETE_CLASS_ERR,
    HASNT_SUPER_CLASS_ERR,
    SUPER_NOT_IN_MEMBER_EXPRESSION_ERR,
    FIELD_OF_SUPER_REFERENCED_ERR,
    FIELD_OVERRIDED_ERR,
    FIELD_NAME_DUPLICATE_ERR,
    ARRAY_METHOD_NOT_FOUND_ERR,
    STRING_METHOD_NOT_FOUND_ERR,
    INSTANCEOF_OPERAND_NOT_REFERENCE_ERR,
    INSTANCEOF_TYPE_NOT_REFERENCE_ERR,
    INSTANCEOF_FOR_NOT_CLASS_ERR,
    INSTANCEOF_MUST_RETURN_TRUE_ERR,
    INSTANCEOF_MUST_RETURN_FALSE_ERR,
    INSTANCEOF_INTERFACE_ERR,
    DOWN_CAST_OPERAND_IS_NOT_CLASS_ERR,
    DOWN_CAST_TARGET_IS_NOT_CLASS_ERR,
    DOWN_CAST_DO_NOTHING_ERR,
    DOWN_CAST_TO_SUPER_CLASS_ERR,
    DOWN_CAST_TO_BAD_CLASS_ERR,
    DOWN_CAST_INTERFACE_ERR,
    CATCH_TYPE_IS_NOT_CLASS_ERR,
    CATCH_TYPE_IS_NOT_EXCEPTION_ERR,
    THROW_TYPE_IS_NOT_CLASS_ERR,
    THROW_TYPE_IS_NOT_EXCEPTION_ERR,
    RETHROW_OUT_OF_CATCH_ERR,
    GOTO_STATEMENT_IN_FINALLY_ERR,
    THROWS_TYPE_NOT_FOUND_ERR,
    THROWS_TYPE_IS_NOT_EXCEPTION_ERR,
    EXCEPTION_HAS_TO_BE_THROWN_ERR,
    USING_ITSELF_ERR,
    IF_CONDITION_NOT_BOOLEAN_ERR,
    WHILE_CONDITION_NOT_BOOLEAN_ERR,
    FOR_CONDITION_NOT_BOOLEAN_ERR,
    DO_WHILE_CONDITION_NOT_BOOLEAN_ERR,
    CASE_TYPE_MISMATCH_ERR,
    FINAL_VARIABLE_ASSIGNMENT_ERR,
    FINAL_FIELD_ASSIGNMENT_ERR,
    FINAL_VARIABLE_WITHOUT_INITIALIZER_ERR,
    OVERRIDE_METHOD_ACCESSIBILITY_ERR,
    BAD_PARAMETER_COUNT_ERR,
    BAD_PARAMETER_TYPE_ERR,
    BAD_RETURN_TYPE_ERR,
    BAD_EXCEPTION_ERR,
    CONSTRUCTOR_CALLED_ERR,
    TYPE_NAME_NOT_FOUND_ERR,
    ENUMERATOR_NOT_FOUND_ERR,
    INTERFACE_INHERIT_ERR,
    PACKAGE_MEMBER_ACCESS_ERR,
    PACKAGE_CLASS_ACCESS_ERR,
    THIS_OUT_OF_CLASS_ERR,
    SUPER_OUT_OF_CLASS_ERR,
    BIT_NOT_TYPE_MISMATCH_ERR,
    BIT_BINARY_OPERATOR_TYPE_MISMATCH_ERR,
    EOF_IN_C_COMMENT_ERR,
    EOF_IN_STRING_LITERAL_ERR,
    TOO_LONG_CHARACTER_LITERAL_ERR,
    ASSIGN_EXPRESSION_LEFT_ITEM_FORCE_CAST_ERR,
    ISTYPE_EXPRESSION_OPERAND_MUST_BE_OBJECT_ERR,
	GENERIC_CLASS_WITH_NO_ARGUMENT,
	FALL_THROUGH_ONLY_FOR_SWITCH_ERR,
	DISALLOWED_DEFAULT_PARAMETER_ERR,
	DELEGATE_WITH_DEFAULT_PARAMETER_ERR,
	DIFFERENT_UNBOXING_TYPE_ERR,
	CAST_CLASS_WITH_FORCE_CAST_ERR,
	UNSUPPORT_FORCE_CAST_ERR,
    COMPILE_ERROR_COUNT_PLUS_1
} CompileError;

typedef struct Expression_tag Expression;

typedef enum {
    BOOLEAN_EXPRESSION = 1,
    INT_EXPRESSION,
    DOUBLE_EXPRESSION,
    STRING_EXPRESSION,
    LONG_DOUBLE_EXPRESSION,
    IDENTIFIER_EXPRESSION,
    COMMA_EXPRESSION,
    ASSIGN_EXPRESSION,
    ADD_EXPRESSION,
    SUB_EXPRESSION,
    MUL_EXPRESSION,
    DIV_EXPRESSION,
    MOD_EXPRESSION,
    BIT_AND_EXPRESSION,
    BIT_OR_EXPRESSION,
    BIT_XOR_EXPRESSION,
    EQ_EXPRESSION,
    NE_EXPRESSION,
    GT_EXPRESSION,
    GE_EXPRESSION,
    LT_EXPRESSION,
    LE_EXPRESSION,
    LOGICAL_AND_EXPRESSION,
    LOGICAL_OR_EXPRESSION,
    MINUS_EXPRESSION,
    BIT_NOT_EXPRESSION,
    LOGICAL_NOT_EXPRESSION,
    FUNCTION_CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    NULL_EXPRESSION,
    THIS_EXPRESSION,
    SUPER_EXPRESSION,
    ARRAY_LITERAL_EXPRESSION,
    INDEX_EXPRESSION,
    INCREMENT_EXPRESSION,
    DECREMENT_EXPRESSION,
    INSTANCEOF_EXPRESSION,
    ISTYPE_EXPRESSION,
    DOWN_CAST_EXPRESSION,
    CAST_EXPRESSION,
    FORCE_CAST_EXPRESSION,
    UP_CAST_EXPRESSION,
    NEW_EXPRESSION,
    ARRAY_CREATION_EXPRESSION,
    ENUMERATOR_EXPRESSION,
    EXPRESSION_KIND_COUNT_PLUS_1
} ExpressionKind;

#define Ivyc_is_numeric_type(type)\
  ((type) == Ivyc_INT_VALUE || (type) == Ivyc_DOUBLE_VALUE || (type) == Ivyc_LONG_DOUBLE_VALUE)

#define Ivyc_is_math_operator(operator) \
  ((operator) == ADD_EXPRESSION || (operator) == SUB_EXPRESSION\
   || (operator) == MUL_EXPRESSION || (operator) == DIV_EXPRESSION\
   || (operator) == MOD_EXPRESSION)

#define Ivyc_is_compare_operator(operator) \
  ((operator) == EQ_EXPRESSION || (operator) == NE_EXPRESSION\
   || (operator) == GT_EXPRESSION || (operator) == GE_EXPRESSION\
   || (operator) == LT_EXPRESSION || (operator) == LE_EXPRESSION)

#define Ivyc_is_unknown_type(type) \
  ((type)->basic_type == ISandBox_UNCLEAR_TYPE && (type)->derive == NULL)

#define Ivyc_is_base_type(type) \
  ((type)->basic_type == ISandBox_BASE_TYPE && (type)->derive == NULL)

#define Ivyc_is_int(type) \
  ((type)->basic_type == ISandBox_INT_TYPE && (type)->derive == NULL)

#define Ivyc_is_double(type) \
  ((type)->basic_type == ISandBox_DOUBLE_TYPE && (type)->derive == NULL)

#define Ivyc_is_long_double(type) \
  ((type)->basic_type == ISandBox_LONG_DOUBLE_TYPE && (type)->derive == NULL)

#define Ivyc_is_boolean(type) \
  ((type)->basic_type == ISandBox_BOOLEAN_TYPE && (type)->derive == NULL)

#define Ivyc_is_type_object(type) \
  ((type)->basic_type == ISandBox_OBJECT_TYPE && (type)->derive == NULL)

#define Ivyc_is_iterator(type) \
  ((type)->basic_type == ISandBox_ITERATOR_TYPE && (type)->derive == NULL)

#define Ivyc_is_string(type) \
  ((type)->basic_type == ISandBox_STRING_TYPE && (type)->derive == NULL)

#define Ivyc_is_array(type) \
  ((type)->derive && ((type)->derive->tag == ARRAY_DERIVE))

#define Ivyc_is_class_object(type) \
  ((type)->basic_type == ISandBox_CLASS_TYPE && (type)->derive == NULL)

#define Ivyc_is_native_pointer(type) \
  ((type)->basic_type == ISandBox_NATIVE_POINTER_TYPE && (type)->derive == NULL)

#define Ivyc_is_object(type) \
  (Ivyc_is_string(type) || Ivyc_is_array(type) || Ivyc_is_class_object(type)\
   || Ivyc_is_native_pointer(type) || Ivyc_is_iterator(type))

#define Ivyc_is_enum(type) \
  ((type)->basic_type == ISandBox_ENUM_TYPE && (type)->derive == NULL)

#define Ivyc_is_delegate(type) \
  ((type)->basic_type == ISandBox_DELEGATE_TYPE && (type)->derive == NULL)

#define Ivyc_is_function(type) \
  ((type)->derive && ((type)->derive->tag == FUNCTION_DERIVE))

#define Ivyc_is_logical_operator(operator) \
  ((operator) == LOGICAL_AND_EXPRESSION || (operator) == LOGICAL_OR_EXPRESSION)

typedef struct PackageName_tag {
    char                *name;
    struct PackageName_tag      *next;
} PackageName;

typedef enum {
    IVH_SOURCE,
    IVY_SOURCE
} SourceSuffix;

typedef struct UsingList_tag {
    PackageName *package_name;
	SourceSuffix source_suffix;
    int line_number;
    struct UsingList_tag      *next;
} UsingList;

typedef struct RenameList_tag {
    PackageName *package_name;
    char        *original_name;
    char        *renamed_name;
    int         line_number;
    struct RenameList_tag       *next;
} RenameList;

typedef struct ArgumentList_tag {
    Expression *expression;
	ISandBox_Boolean is_default;
    struct ArgumentList_tag *next;
} ArgumentList;

typedef struct TypeArgumentList_tag {
    struct TypeSpecifier_tag *type;
    struct TypeArgumentList_tag *next;
} TypeArgumentList;

typedef struct TypeSpecifier_tag TypeSpecifier;

typedef struct ParameterList_tag {
    char                *name;
    TypeSpecifier       *type;
	Expression			*initializer;
	ISandBox_Boolean	is_vargs;
	ISandBox_Boolean	has_fixed;
    int                 line_number;
    struct ParameterList_tag *next;
} ParameterList;

typedef struct TypeParameterList_tag {
	char				*name;
	TypeSpecifier       *target;
    int                 line_number;
    struct TypeParameterList_tag *next;
} TypeParameterList;

typedef enum {
    FUNCTION_DERIVE,
    ARRAY_DERIVE
} DeriveTag;

typedef struct ClassDefinition_tag ClassDefinition;

typedef struct {
    char *identifier;
    ClassDefinition *class_definition;
    int         line_number;
} ExceptionRef;

typedef struct ExceptionList_tag {
    ExceptionRef *ref;
    struct ExceptionList_tag *next;
} ExceptionList;

typedef struct {
    ParameterList       *parameter_list;
    ExceptionList       *throws;
} FunctionDerive;

typedef struct {
    int dummy; /* make compiler happy */
} ArrayDerive;

typedef struct TypeDerive_tag {
    DeriveTag   tag;
    union {
        FunctionDerive  function_d;
        ArrayDerive     array_d;
    } u;
    struct TypeDerive_tag       *next;
} TypeDerive;

typedef struct DelegateDefinition_tag DelegateDefinition;
typedef struct EnumDefinition_tag EnumDefinition;

struct TypeSpecifier_tag {
	ISandBox_Boolean		 is_generic;
	ISandBox_Boolean         is_placeholder;
	TypeArgumentList		 *type_argument_list;

    ISandBox_BasicType       basic_type;
    char        *identifier;
	char        *orig_identifier;
    union {
        struct {
            ClassDefinition *class_definition;
            int class_index;
        } class_ref;
        struct {
            DelegateDefinition *delegate_definition;
        } delegate_ref;
        struct {
            EnumDefinition *enum_definition;
            int enum_index;
        } enum_ref;
		struct {
            TypeSpecifier *origin;
        } object_ref;
    } u;
    int                 line_number;
    TypeDerive  *derive;
};

typedef struct FunctionDefinition_tag FunctionDefinition;
typedef struct ConstantDefinition_tag ConstantDefinition;

typedef struct {
    char        *name;
    TypeSpecifier       *type;
    ISandBox_Boolean is_final;
    Expression  *initializer;
    int variable_index;
    ISandBox_Boolean is_local;
} Declaration;

typedef struct DeclarationList_tag {
    Declaration *declaration;
    struct DeclarationList_tag *next;
} DeclarationList;

typedef struct {
    FunctionDefinition *function_definition;
    int function_index;
} FunctionIdentifier;

typedef struct {
    ConstantDefinition *constant_definition;
    int constant_index;
} ConstantIdentifier;

typedef enum {
    VARIABLE_IDENTIFIER,
    FUNCTION_IDENTIFIER,
    CONSTANT_IDENTIFIER
} IdentifierKind;

typedef struct {
    char        *name;
    IdentifierKind kind;
    union {
        FunctionIdentifier function;
        Declaration     *declaration;
        ConstantIdentifier constant;
    } u;
} IdentifierExpression;

typedef struct {
    Expression  *left;
    Expression  *right;
} CommaExpression;

typedef enum {
    NORMAL_ASSIGN = 1,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN
} AssignmentOperator;

typedef struct {
    AssignmentOperator  operator;
    Expression  *left;
    Expression  *operand;
} AssignExpression;

typedef struct {
    Expression  *left;
    Expression  *right;
} BinaryExpression;

typedef struct {
    Expression          *function;
    ArgumentList        *argument;
} FunctionCallExpression;

typedef struct ExpressionList_tag {
    Expression          *expression;
    struct ExpressionList_tag   *next;
} ExpressionList;

typedef struct {
    Expression  *array;
    Expression  *index;
} IndexExpression;

typedef struct MemberDeclaration_tag MemberDeclaration;

typedef struct {
    Expression          *expression;
    char                *member_name;
    MemberDeclaration   *declaration;
    int         method_index; /* use for only array and string */
} MemberExpression;

typedef struct {
    Expression  *operand;
} IncrementOrDecrement;

typedef struct {
    Expression  *operand;
    TypeSpecifier *type;
} InstanceofExpression;

typedef struct {
    Expression  *operand;
    TypeSpecifier *type;
} IsExpression;

typedef struct {
    Expression          *operand;
    TypeSpecifier       *type;
} DownCastExpression;

typedef enum {
    INT_TO_DOUBLE_CAST,
    INT_TO_LONG_DOUBLE_CAST,
    INT_TO_STRING_CAST,
    DOUBLE_TO_INT_CAST,
    DOUBLE_TO_LONG_DOUBLE_CAST,
    DOUBLE_TO_STRING_CAST,
    LONG_DOUBLE_TO_INT_CAST,
    LONG_DOUBLE_TO_DOUBLE_CAST,
    LONG_DOUBLE_TO_STRING_CAST,
    BOOLEAN_TO_STRING_CAST,
    ENUM_TO_STRING_CAST,
	ENUM_TO_INT_CAST,
    FUNCTION_TO_DELEGATE_CAST,
    ALL_TO_OBJECT_CAST
} CastType;

typedef struct {
    CastType    type;
    Expression  *operand;
} CastExpression;

typedef struct {
    TypeSpecifier  *type;
    TypeSpecifier  *from;
    Expression     *operand;
} ForceCastExpression;

typedef struct {
    ClassDefinition     *interface_definition;
    Expression          *operand;
    int                 interface_index;
} UpCastExpression;

typedef struct {
	ISandBox_Boolean 	*is_generic;
	TypeArgumentList	*type_argument_list;

    char                *class_name;
    ClassDefinition     *class_definition;
    int                 class_index;
    char                *method_name;
    MemberDeclaration   *method_declaration;
    ArgumentList        *argument;
} NewExpression;

typedef struct ArrayDimension_tag {
    Expression  *expression;
    struct ArrayDimension_tag   *next;
} ArrayDimension;

typedef struct {
    TypeSpecifier       *type;
    ArrayDimension      *dimension;
} ArrayCreation;

typedef struct Enumerator_tag {
    char        *name;
    int         value;
    struct Enumerator_tag *next;
} Enumerator;

typedef struct {
    EnumDefinition      *enum_definition;
    Enumerator          *enumerator;
} EnumeratorExpression;

struct Expression_tag {
    TypeSpecifier *type;
    ExpressionKind kind;
    int line_number;
    union {
        ISandBox_Boolean        boolean_value;
        int                     int_value;
        double                  double_value;
        long double             long_double_value;
        ISandBox_Char           *string_value;
        IdentifierExpression    identifier;
        CommaExpression         comma;
        AssignExpression        assign_expression;
        BinaryExpression        binary_expression;
        Expression              *minus_expression;
        Expression              *logical_not;
        Expression              *bit_not;
        FunctionCallExpression  function_call_expression;
        MemberExpression        member_expression;
        ExpressionList          *array_literal;
        IndexExpression         index_expression;
        IncrementOrDecrement    inc_dec;
        InstanceofExpression    instanceof;
        IsExpression    		istype;
        DownCastExpression      down_cast;
        CastExpression          cast;
        ForceCastExpression     fcast;
        UpCastExpression        up_cast;
        NewExpression           new_e;
        ArrayCreation           array_creation;
        EnumeratorExpression    enumerator;
    } u;
};

typedef struct Statement_tag Statement;

typedef struct StatementList_tag {
    Statement   *statement;
    struct StatementList_tag    *next;
} StatementList;

typedef enum {
    UNDEFINED_BLOCK = 1,
    FUNCTION_BLOCK,
    WHILE_STATEMENT_BLOCK,
    FOR_STATEMENT_BLOCK,
    DO_WHILE_STATEMENT_BLOCK,
    TRY_CLAUSE_BLOCK,
    CATCH_CLAUSE_BLOCK,
    FINALLY_CLAUSE_BLOCK,
	CASE_STATEMENT_BLOCK
} BlockType;

typedef struct {
    Statement   *statement;
    int         continue_label;
    int         break_label;
	int         fall_through_label;
} StatementBlockInfo;

typedef struct {
    FunctionDefinition  *function;
    int         end_label;
} FunctionBlockInfo;

typedef struct Block_tag {
    BlockType           type;
    struct Block_tag    *outer_block;
    StatementList       *statement_list;
    DeclarationList     *declaration_list;
    union {
        StatementBlockInfo      statement;
        FunctionBlockInfo       function;
    } parent;
} Block;

typedef struct Elsif_tag {
    Expression  *condition;
    Block       *block;
    struct Elsif_tag    *next;
} Elsif;

typedef struct {
    Expression  *condition;
    Block       *then_block;
    Elsif       *elsif_list;
    Block       *else_block;
} IfStatement;

typedef struct CaseList_tag {
    ExpressionList      *expression_list;
    Block               *block;
    struct CaseList_tag *next;
} CaseList;

typedef struct {
    Expression          *expression;
    CaseList            *case_list;
    Block               *default_block;
} SwitchStatement;

typedef struct {
    char        *label;
    Expression  *condition;
    Block       *block;
} WhileStatement;

typedef struct {
    char        *label;
    Expression  *init;
    Expression  *condition;
    Expression  *post;
    Block       *block;
} ForStatement;

typedef struct {
    char        *label;
    Block       *block;
    Expression  *condition;
} DoWhileStatement;

typedef struct {
    char        *label;
    char        *variable;
    Expression  *collection;
    Block       *block;
} ForeachStatement;

typedef struct {
    Expression *return_value;
} ReturnStatement;

typedef struct {
    char        *label;
} BreakStatement;

typedef struct {
    char        *label;
} LabelStatement;

typedef struct {
    char        *target;
} GotoStatement;

typedef struct {
    char        *label;
} ContinueStatement;

typedef struct CatchClause_tag {
    TypeSpecifier       *type;
    char                *variable_name;
    Declaration         *variable_declaration;
    Block               *block;
    int                 line_number;
    struct CatchClause_tag      *next;
} CatchClause;

typedef struct {
    Block       *try_block;
    CatchClause *catch_clause;
    Block       *finally_block;
} TryStatement;

typedef struct {
    Expression  *exception;
    Declaration *variable_declaration;
} ThrowStatement;

typedef enum {
    EXPRESSION_STATEMENT = 1,
    IF_STATEMENT,
    SWITCH_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    DO_WHILE_STATEMENT,
    FOREACH_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    TRY_STATEMENT,
    THROW_STATEMENT,
    DECLARATION_STATEMENT,
    DECLARATION_LIST_STATEMENT,
    LABEL_STATEMENT,
    GOTO_STATEMENT,
	FALL_THROUGH_STATEMENT,
	IVY_BYTE_CODE,
    STATEMENT_TYPE_COUNT_PLUS_1
} StatementType;

struct Statement_tag {
    StatementType       type;
    int                 line_number;
    union {
        Expression      *expression_s;
        IfStatement     if_s;
        SwitchStatement switch_s;
        WhileStatement  while_s;
        ForStatement    for_s;
        DoWhileStatement        do_while_s;
        ForeachStatement        foreach_s;
        BreakStatement  break_s;
        LabelStatement  label_s;
		GotoStatement   goto_s;
        ContinueStatement       continue_s;
        ReturnStatement return_s;
        TryStatement    try_s;
        ThrowStatement  throw_s;
        Declaration     *declaration_s;
        DeclarationList *declaration_list_s;
    } u;
};

struct FunctionDefinition_tag {
	ISandBox_Boolean    has_fixed;
    TypeSpecifier       *type;
    PackageName         *package_name;
    char                *name;
    ParameterList       *parameter;
    Block               *block;
    int                 local_variable_count;
    Declaration         **local_variable;
    ClassDefinition     *class_definition;
    ExceptionList       *throws;
    int                 end_line_number;
    struct FunctionDefinition_tag       *next;
};

typedef enum {
    ABSTRACT_MODIFIER,
    PUBLIC_MODIFIER,
    PRIVATE_MODIFIER,
    OVERRIDE_MODIFIER,
    VIRTUAL_MODIFIER,
    NOT_SPECIFIED_MODIFIER
} ClassOrMemberModifierKind;

typedef struct {
    ClassOrMemberModifierKind   is_abstract;
    ClassOrMemberModifierKind   access_modifier;
    ClassOrMemberModifierKind   is_virtual;
    ClassOrMemberModifierKind   is_override;
} ClassOrMemberModifierList;

typedef struct ExtendsList_tag {
	ISandBox_Boolean is_generic;
    char *identifier;
	TypeArgumentList *type_argument_list;
    ClassDefinition *class_definition;
    struct ExtendsList_tag *next;
} ExtendsList;

typedef enum {
    METHOD_MEMBER,
    FIELD_MEMBER
} MemberKind;

typedef struct {
    ISandBox_Boolean         is_constructor;
    ISandBox_Boolean         is_abstract;
    ISandBox_Boolean         is_virtual;
    ISandBox_Boolean         is_override;
    FunctionDefinition  *function_definition;
    int                 method_index;
} MethodMember;

typedef struct {
    char                *name;
    TypeSpecifier       *type;
    ISandBox_Boolean         is_final;
    Expression          *initializer;
    int                 field_index;
} FieldMember;

struct MemberDeclaration_tag {
    MemberKind  kind;
    ISandBox_AccessModifier  access_modifier;
    union {
        MethodMember method;
        FieldMember  field;
    } u;
    int line_number;
    struct MemberDeclaration_tag *next;
};

typedef struct TypeParameterRequireList_tag {
	TypeSpecifier		*pointer;
	struct TypeParameterRequireList_tag *next;
} TypeParameterRequireList;

struct ClassDefinition_tag {
	/* generic */
    ISandBox_Boolean                 is_generic;
	TypeParameterList				 *type_parameter_list;
	/* generic */

    ISandBox_Boolean is_abstract;
    ISandBox_AccessModifier access_modifier;
    ISandBox_ClassOrInterface class_or_interface;
    PackageName *package_name;
    char *name;
    ExtendsList *extends;
    ClassDefinition *super_class;
    ExtendsList *interface_list;
    MemberDeclaration *member;
    int line_number;
    struct ClassDefinition_tag *next;
};

typedef struct CompilerList_tag {
    Ivyc_Compiler *compiler;
    struct CompilerList_tag *next;
} CompilerList;

typedef enum {
    FILE_INPUT_MODE = 1,
    STRING_INPUT_MODE
} InputMode;

typedef struct {
    InputMode   input_mode;
    union {
        struct {
            FILE        *fp;
        } file;
        struct {
            char        **lines;
        } string;
    } u;
} SourceInput;

struct DelegateDefinition_tag {
    TypeSpecifier       *type;
    char                *name;
    ParameterList       *parameter_list;
    ExceptionList       *throws;
    DelegateDefinition  *next;
};

struct EnumDefinition_tag {
    PackageName *package_name;
    char        *name;
    Enumerator  *enumerator;
    int         index;
    EnumDefinition *next;
};

struct ConstantDefinition_tag {
    TypeSpecifier       *type;
    PackageName         *package_name;
    char        *name;
    int         index;
    Expression  *initializer;
    int         line_number;
    ConstantDefinition *next;
};

typedef enum {
    EUC_ENCODING = 1,
    SHIFT_JIS_ENCODING,
    UTF_8_ENCODING
} Encoding;

struct Ivyc_Compiler_tag {
    MEM_Storage         compile_storage;
    PackageName         *package_name;
    SourceSuffix        source_suffix;
    char                *path;
    UsingList           *using_list;
    RenameList          *rename_list;
    FunctionDefinition  *function_list;
    int                 ISandBox_function_count;
    ISandBox_Function        *ISandBox_function;
    int                 ISandBox_enum_count;
    ISandBox_Enum            *ISandBox_enum;
    int                 ISandBox_class_count;
    ISandBox_Class           *ISandBox_class;
    DeclarationList     *declaration_list;
    StatementList       *statement_list;
    ClassDefinition     *class_definition_list;
    ClassDefinition     *template_class_definition_list;
    DelegateDefinition  *delegate_definition_list;
    EnumDefinition      *enum_definition_list;
    ConstantDefinition  *constant_definition_list;
    int                 current_line_number;
    Block               *current_block;
    ClassDefinition     *current_class_definition;
    TryStatement        *current_try_statement;
    CatchClause         *current_catch_clause;
    int                 current_finally_label;
	ISandBox_Boolean	current_function_is_constructor;
    InputMode           input_mode;
    CompilerList        *usingd_list;
    int                 array_method_count;
    FunctionDefinition  *array_method;
    int                 string_method_count;
    FunctionDefinition  *string_method;
    int                 iterator_method_count;
    FunctionDefinition  *iterator_method;
    Encoding            source_encoding;
};

typedef struct {
    char        *string;
} VString;

typedef struct {
    ISandBox_Char    *string;
} VWString;

typedef struct {
    char *package_name;
    SourceSuffix suffix;
    char **source_string;
} BuiltinScript;

/* Ivory.l */
void Ivyc_set_source_string(char **source);

/* create.c */
Expression * Ivyc_create_var_args_list_expression(ExpressionList *list);
DeclarationList *Ivyc_chain_declaration(DeclarationList *list,
                                       Declaration *decl);
Declaration *Ivyc_alloc_declaration(ISandBox_Boolean is_final, TypeSpecifier *type,
                                   char *identifier);
PackageName *Ivyc_create_package_name(char *identifier);
PackageName *Ivyc_chain_package_name(PackageName *list, char *identifier);
UsingList *Ivyc_create_using_list(PackageName *package_name);
UsingList *Ivyc_chain_using_list(UsingList *list, UsingList *add);
RenameList *Ivyc_create_rename_list(PackageName *package_name,
                                   char *identifier);
RenameList *Ivyc_chain_rename_list(RenameList *list, RenameList *add);
void Ivyc_set_using_and_rename_list(UsingList *using_list,
                                     RenameList *rename_list);
FunctionDefinition *
Ivyc_create_function_definition(TypeSpecifier *type, char *identifier,
                               ParameterList *parameter_list,
                               ExceptionList *throws, Block *block,
							   ISandBox_Boolean if_add);
void Ivyc_function_define(TypeSpecifier *type, char *identifier,
                         ParameterList *parameter_list,
                         ExceptionList *throws, Block *block);
ParameterList *Ivyc_create_parameter(TypeSpecifier *type, char *identifier, Expression *initializer, ISandBox_Boolean is_vargs);
ParameterList *Ivyc_chain_parameter(ParameterList *list, TypeSpecifier *type,
                                   char *identifier, Expression *initializer, ISandBox_Boolean is_vargs);
TypeParameterList *Ivyc_create_type_parameter(char *identifier);
TypeParameterList *Ivyc_chain_type_parameter(TypeParameterList *list, char *identifier);
ArgumentList *Ivyc_create_argument_list(Expression *expression);
TypeArgumentList *Ivyc_create_type_argument_list(TypeSpecifier *type);
TypeArgumentList *Ivyc_chain_type_argument_list(TypeArgumentList *list, TypeSpecifier *type);
ArgumentList *Ivyc_chain_argument_list(ArgumentList *list, Expression *expr);
ExpressionList *Ivyc_create_expression_list(Expression *expression);
ExpressionList *Ivyc_chain_expression_list(ExpressionList *list,
                                          Expression *expr);
StatementList *Ivyc_create_statement_list(Statement *statement);
StatementList *Ivyc_chain_statement_list(StatementList *list,
                                        Statement *statement);
TypeSpecifier *Ivyc_create_type_specifier(ISandBox_BasicType basic_type);
TypeSpecifier *Ivyc_create_identifier_type_specifier(char *identifier);
TypeSpecifier *Ivyc_create_generic_identifier_type_specifier(char *identifier, TypeArgumentList *list);
TypeSpecifier *Ivyc_create_array_type_specifier(TypeSpecifier *base);
Expression *Ivyc_alloc_expression(ExpressionKind type);
Expression *Ivyc_create_comma_expression(Expression *left, Expression *right);
Expression *Ivyc_create_assign_expression(TypeSpecifier *type_left,
                                          Expression *left,
                                          AssignmentOperator operator,
                                          TypeSpecifier *type_operand,
                                          Expression *operand);
Expression *Ivyc_create_binary_expression(ExpressionKind operator,
                                         Expression *left,
                                         Expression *right);
Expression *Ivyc_create_minus_expression(Expression *operand);
Expression *Ivyc_create_logical_not_expression(Expression *operand);
Expression *Ivyc_create_bit_not_expression(Expression *operand);
Expression *Ivyc_create_index_expression(Expression *array, Expression *index);
Expression *Ivyc_create_incdec_expression(Expression *operand,
                                         ExpressionKind inc_or_dec);
Expression *Ivyc_create_instanceof_expression(Expression *operand,
                                             TypeSpecifier *type);
Expression *Ivyc_create_istype_expression(Expression *operand,
                                             TypeSpecifier *type);
Expression *Ivyc_create_identifier_expression(char *identifier);
Expression *Ivyc_create_function_call_expression(Expression *function,
                                                ArgumentList *argument);
Expression *Ivyc_create_down_cast_expression(Expression *operand,
                                            TypeSpecifier *type);
Expression *Ivyc_create_member_expression(Expression *expression,
                                         char *member_name);
Expression *Ivyc_create_boolean_expression(ISandBox_Boolean value);
Expression *Ivyc_create_null_expression(void);
Expression *Ivyc_create_new_expression(char *class_name, TypeArgumentList *type_list, char *method_name,
                                      ArgumentList *argument);
Expression *Ivyc_create_array_literal_expression(ExpressionList *list);
Expression *Ivyc_create_basic_array_creation(ISandBox_BasicType basic_type,
                                            ArrayDimension *dim_expr_list,
                                            ArrayDimension *dim_ilst);
Expression *Ivyc_create_class_array_creation(TypeSpecifier *type,
                                            ArrayDimension *dim_expr_list,
                                            ArrayDimension *dim_ilst);
Expression *Ivyc_create_this_expression(void);
Expression *Ivyc_create_super_expression(void);
ArrayDimension *Ivyc_create_array_dimension(Expression *expr);
ArrayDimension *Ivyc_chain_array_dimension(ArrayDimension *list,
                                          ArrayDimension *dim);
Statement *Ivyc_alloc_statement(StatementType type);
Statement *Ivyc_create_if_statement(Expression *condition,
                                   Block *then_block, Elsif *elsif_list,
                                   Block *else_block);
Elsif *Ivyc_chain_elsif_list(Elsif *list, Elsif *add);
Elsif *Ivyc_create_elsif(Expression *expr, Block *block);
Statement *Ivyc_create_switch_statement(Expression *expression,
                                       CaseList *case_list,
                                       Block *default_block);
CaseList *Ivyc_create_one_case(ExpressionList *expression_list, StatementList *list);
CaseList *Ivyc_chain_case(CaseList *list, CaseList *add);
Statement *Ivyc_create_while_statement(char *label,
                                      Expression *condition, Block *block);
Statement *
Ivyc_create_foreach_statement(char *label, char *variable,
                             Expression *collection, Block *block);
Statement *Ivyc_create_for_statement(char *label,
                                    Expression *init, Expression *cond,
                                    Expression *post, Block *block);
Statement *Ivyc_create_do_while_statement(char *label, Block *block,
                                         Expression *condition);
Block *Ivyc_alloc_block(void);
Block * Ivyc_open_block(void);
Block *Ivyc_close_block(Block *block, StatementList *statement_list);
Statement *Ivyc_create_expression_statement(Expression *expression);
Statement *Ivyc_create_return_statement(Expression *expression);
Statement *Ivyc_create_break_statement(char *label);
Statement *Ivyc_create_label_statement(char *label);
Statement *Ivyc_create_goto_statement(char *target);
Statement *Ivyc_create_continue_statement(char *label);
Statement *Ivyc_create_try_statement(Block *try_block,
                                    CatchClause *catch_clause,
                                    Block *finally_block);
CatchClause *Ivyc_create_catch_clause(TypeSpecifier *type, char *variable_name,
                                     Block *block);
CatchClause *Ivyc_start_catch_clause(void);
CatchClause *Ivyc_end_catch_clause(CatchClause *catch_clause,
                                  TypeSpecifier *type, char *variable_name,
                                  Block *block);
CatchClause *Ivyc_chain_catch_list(CatchClause *list, CatchClause *add);
Statement *Ivyc_create_throw_statement(Expression *expression);
Statement *Ivyc_create_declaration_statement(ISandBox_Boolean is_final,
                                            TypeSpecifier *type,
                                            char *identifier,
                                            Expression *initializer);
void
Ivyc_start_class_definition(ClassOrMemberModifierList *modifier,
                           ISandBox_ClassOrInterface class_or_interface,
                           char *identifier,
						   TypeParameterList *list,
                           ExtendsList *extends);
void Ivyc_class_define(MemberDeclaration *member_list);
ExtendsList *Ivyc_create_extends_list(char *identifier, TypeArgumentList *add_list);
ExtendsList *Ivyc_chain_extends_list(ExtendsList *list, char *add, TypeArgumentList *add_list);
ClassOrMemberModifierList
Ivyc_create_class_or_member_modifier(ClassOrMemberModifierKind modifier);
ClassOrMemberModifierList
Ivyc_chain_class_or_member_modifier(ClassOrMemberModifierList list,
                                   ClassOrMemberModifierList add);
MemberDeclaration *
Ivyc_chain_member_declaration(MemberDeclaration *list, MemberDeclaration *add);
MemberDeclaration *
Ivyc_create_method_member(ClassOrMemberModifierList *modifier,
                         FunctionDefinition *function_definition,
                         ISandBox_Boolean is_constructor);
FunctionDefinition *
Ivyc_method_function_define(TypeSpecifier *type, char *identifier,
                           ParameterList *parameter_list,
                           ExceptionList *throws, Block *block);
FunctionDefinition *
Ivyc_constructor_function_define(char *identifier,
                                ParameterList *parameter_list,
                                ExceptionList *throws, Block *block);
MemberDeclaration *
Ivyc_create_field_member(ClassOrMemberModifierList *modifier,
                        ISandBox_Boolean is_final, TypeSpecifier *type,
						DeclarationList *list);
ExceptionList *Ivyc_create_throws(char *identifier);
ExceptionList *Ivyc_chain_exception_list(ExceptionList *list, char *identifier);
void Ivyc_create_delegate_definition(TypeSpecifier *type, char *identifier,
                                    ParameterList *parameter_list,
                                    ExceptionList *throws);
void Ivyc_create_enum_definition(char *identifier, Enumerator *enumerator);
Enumerator *Ivyc_create_enumerator(char *identifier);
Enumerator *Ivyc_chain_enumerator(Enumerator *enumerator, char *identifier);
void Ivyc_create_const_definition(TypeSpecifier *type, char *identifier,
                                 Expression *initializer);

/* string.c */
char *Ivyc_create_identifier(char *str);
void Ivyc_open_string_literal(void);
void Ivyc_add_string_literal(int letter);
void Ivyc_reset_string_literal_buffer(void);
ISandBox_Char *Ivyc_close_string_literal(void);
int Ivyc_close_character_literal(void);

/* fix_tree.c */
void Ivyc_fix_tree(Ivyc_Compiler *compiler);

/* generate.c */
ISandBox_TypeSpecifier *Ivyc_copy_type_specifier(TypeSpecifier *src);
ISandBox_Executable *Ivyc_generate(Ivyc_Compiler *compiler);

/* util.c */
Ivyc_Compiler *Ivyc_get_current_compiler(void);
void Ivyc_set_current_compiler(Ivyc_Compiler *compiler);
void *Ivyc_malloc(size_t size);
char *Ivyc_strdup(char *src);
TypeSpecifier *Ivyc_alloc_type_specifier(ISandBox_BasicType type);
TypeDerive *Ivyc_alloc_type_derive(DeriveTag derive_tag);
TypeSpecifier *Ivyc_alloc_type_specifier2(TypeSpecifier *src);
ISandBox_Boolean Ivyc_is_castable(TypeSpecifier *type1, TypeSpecifier *type2);
ISandBox_Boolean Ivyc_compare_parameter(ParameterList *param1,
                                  ParameterList *param2);
ISandBox_Boolean Ivyc_compare_type_argument_list(TypeArgumentList *list1, TypeArgumentList *list2);
ISandBox_Boolean Ivyc_compare_enum(Enumerator *enum1, Enumerator *enum2);
ISandBox_Boolean Ivyc_compare_type(TypeSpecifier *type1, TypeSpecifier *type2);
ISandBox_Boolean Ivyc_compare_package_name(PackageName *p1, PackageName *p2);
FunctionDefinition *Ivyc_search_function(char *name);
Declaration *Ivyc_search_declaration(char *identifier, Block *block);
ConstantDefinition *Ivyc_search_constant(char *identifier);
ClassDefinition *Ivyc_search_class(char *identifier);
ClassDefinition *Ivyc_search_template_class(char *identifier);
DelegateDefinition *Ivyc_search_delegate(char *identifier);
EnumDefinition *Ivyc_search_enum(char *identifier);
ISandBox_Boolean Ivyc_is_initializable(TypeSpecifier *type);
ISandBox_Boolean Ivyc_compare_arguments(ParameterList *param, ArgumentList *args);
MemberDeclaration *Ivyc_search_initialize(ClassDefinition *class_def, ArgumentList *args);
MemberDeclaration *Ivyc_search_member(ClassDefinition *class_def,
                                     char *member_name);
void Ivyc_vstr_clear(VString *v);
void Ivyc_vstr_append_string(VString *v, char *str);
void Ivyc_vstr_append_character(VString *v, int ch);
void Ivyc_vwstr_clear(VWString *v);
void Ivyc_vwstr_append_string(VWString *v, ISandBox_Char *str);
void Ivyc_vwstr_append_character(VWString *v, int ch);
char *Ivyc_get_type_name(TypeSpecifier *type);
char *Ivyc_get_basic_type_name(ISandBox_BasicType type);
ISandBox_Char *Ivyc_expression_to_string(Expression *expr);
char *Ivyc_package_name_to_string(PackageName *src);
char *Ivyc_get_folder_by_path(char *path);

/* wchar.c */
size_t Ivyc_wcslen(ISandBox_Char *str);
ISandBox_Char *Ivyc_wcscpy(ISandBox_Char *dest, ISandBox_Char *src);
ISandBox_Char *Ivyc_wcsncpy(ISandBox_Char *dest, ISandBox_Char *src, size_t n);
int Ivyc_wcscmp(ISandBox_Char *s1, ISandBox_Char *s2);
ISandBox_Char *Ivyc_wcscat(ISandBox_Char *s1, ISandBox_Char *s2);
int Ivyc_mbstowcs_len(const char *src);
void Ivyc_mbstowcs(const char *src, ISandBox_Char *dest);
ISandBox_Char *Ivyc_mbstowcs_alloc(int line_number, const char *src);
int Ivyc_wcstombs_len(const ISandBox_Char *src);
void Ivyc_wcstombs(const ISandBox_Char *src, char *dest);
char *Ivyc_wcstombs_alloc(const ISandBox_Char *src);
char Ivyc_wctochar(ISandBox_Char src);
int Ivyc_print_wcs(FILE *fp, ISandBox_Char *str);
int Ivyc_print_wcs_ln(FILE *fp, ISandBox_Char *str);
ISandBox_Boolean Ivyc_iswdigit(ISandBox_Char ch);

/* error.c */
void Ivyc_compile_error(int line_number, CompileError id, ...);
void Ivyc_compile_warning(int line_number, CompileError id, ...);

/* disassemble.c */
void Ivyc_disassemble(ISandBox_Executable *exe);

#endif /* PRIVATE_IvoryC_H_INCLUDED */
