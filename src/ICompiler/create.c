#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "Ivoryc.h"
#include "ISandBox_dev.h"

Expression *
Ivyc_create_force_cast_expression(TypeSpecifier *type, Expression *operand)
{
    Expression *exp;
    exp = Ivyc_alloc_expression(FORCE_CAST_EXPRESSION);
    exp->u.fcast.type = exp->type = type;
    exp->u.fcast.operand = operand;
    return exp;
}

Expression *
Ivyc_create_var_args_list_expression(ExpressionList *list)
{
	Expression  *exp;

    exp = Ivyc_alloc_expression(ARRAY_LITERAL_EXPRESSION);

	list->expression = Ivyc_create_force_cast_expression(Ivyc_create_type_specifier(ISandBox_OBJECT_TYPE), list->expression);
    exp->u.array_literal = list;

    return exp;
}

Statement *
Ivyc_retype_declaration_list(ISandBox_Boolean is_final, TypeSpecifier *type,
                                               DeclarationList *list)
{
    Statement *st;
    DeclarationList *pos;
    Declaration *space;
    Ivyc_chain_declaration(list, space);
    TypeSpecifier *default_t;

    default_t = Ivyc_create_type_specifier(ISandBox_UNCLEAR_TYPE);

    for (pos = list; pos->next != NULL; pos = pos->next)
    {
        pos->declaration->is_final = is_final;
        pos->declaration->type = (type != NULL ? type : default_t);
    }

    st = Ivyc_alloc_statement(DECLARATION_LIST_STATEMENT);
    st->u.declaration_list_s = list;

    return st;
}

DeclarationList *
Ivyc_create_declaration_list(Declaration *decl)
{
    DeclarationList *new_item;

    new_item = Ivyc_malloc(sizeof(DeclarationList));
    new_item->declaration = decl;
    new_item->next = NULL;

    return new_item;
}

DeclarationList *
Ivyc_chain_declaration(DeclarationList *list, Declaration *decl)
{
    DeclarationList *new_item;
    DeclarationList *pos;

    new_item = Ivyc_malloc(sizeof(DeclarationList));
    new_item->declaration = decl;
    new_item->next = NULL;

    if (list == NULL) {
        return new_item;
    }

    for (pos = list; pos->next != NULL; pos = pos->next)
        ;
    pos->next = new_item;

    return list;
}

Declaration *
Ivyc_alloc_declaration(ISandBox_Boolean is_final, TypeSpecifier *type,
                      char *identifier)
{
    Declaration *decl;

    decl = Ivyc_malloc(sizeof(Declaration));
    decl->name = identifier;
    decl->type = type;
    decl->is_final = is_final;
    decl->variable_index = -1;

    return decl;
}

Declaration *
Ivyc_create_declaration(ISandBox_Boolean is_final, TypeSpecifier *type,
                      char *identifier,
                      Expression *initializer)
{
    Declaration *decl;
    decl = Ivyc_alloc_declaration(is_final, type, identifier);
    decl->initializer = initializer;

    return decl;
}

PackageName *
Ivyc_create_package_name(char *identifier)
{
    PackageName *pn;

    pn = Ivyc_malloc(sizeof(PackageName));
    pn->name = identifier;
    pn->next = NULL;

    return pn;
}

PackageName *
Ivyc_chain_package_name(PackageName *list, char *identifier)
{
    PackageName *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_package_name(identifier);

    return list;
}

UsingList *
Ivyc_create_using_list(PackageName *package_name)
{
    UsingList *rl;
    Ivyc_Compiler *compiler;
    char *current_package_name;
    char *req_package_name;

    compiler = Ivyc_get_current_compiler();

    current_package_name = Ivyc_package_name_to_string(compiler->package_name);
    req_package_name = Ivyc_package_name_to_string(package_name);
    if (ISandBox_compare_string(req_package_name, current_package_name)
        && compiler->source_suffix == IVH_SOURCE) {
        Ivyc_compile_error(compiler->current_line_number,
                          USING_ITSELF_ERR, MESSAGE_ARGUMENT_END);
    }
    MEM_free(current_package_name);
    MEM_free(req_package_name);

    rl = Ivyc_malloc(sizeof(UsingList));
    rl->package_name = package_name;
	rl->source_suffix = IVH_SOURCE;
    rl->line_number = Ivyc_get_current_compiler()->current_line_number;
    rl->next = NULL;

    return rl;
}

UsingList *
Ivyc_chain_using_list(UsingList *list, UsingList *add)
{
    UsingList *pos;
    char buf[LINE_BUF_SIZE];

    for (pos = list; pos->next; pos = pos->next) {
        if (Ivyc_compare_package_name(pos->package_name, add->package_name)) {
            char *package_name;
            package_name = Ivyc_package_name_to_string(add->package_name);
            ISandBox_strncpy(buf, package_name, LINE_BUF_SIZE);
            MEM_free(package_name);
            Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                              USING_DUPLICATE_ERR,
                              STRING_MESSAGE_ARGUMENT, "package_name", buf,
                              MESSAGE_ARGUMENT_END);
        }
    }
    pos->next = add;

    return list;
}

RenameList *
Ivyc_create_rename_list(PackageName *package_name, char *identifier)
{
    RenameList *rl;
    PackageName *pre_tail;
    PackageName *tail;

    pre_tail = NULL;
    for (tail = package_name; tail->next; tail = tail->next) {
        pre_tail = tail;
    }
    if (pre_tail == NULL) {
        Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                          RENAME_HAS_NO_PACKAGED_NAME_ERR,
                          MESSAGE_ARGUMENT_END);
    }
    pre_tail->next = NULL;

    rl = Ivyc_malloc(sizeof(RenameList));
    rl->package_name = package_name;
    rl->original_name = tail->name;
    rl->renamed_name = identifier;
    rl->line_number = Ivyc_get_current_compiler()->current_line_number;
    rl->next = NULL;

    return rl;
}

RenameList *
Ivyc_chain_rename_list(RenameList *list, RenameList *add)
{
    RenameList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

static UsingList *
add_default_package(UsingList *using_list)
{
    UsingList *req_pos;
    ISandBox_Boolean default_package_usingd = ISandBox_FALSE;

    for (req_pos = using_list; req_pos; req_pos = req_pos->next) {
        char *temp_name
            = Ivyc_package_name_to_string(req_pos->package_name);
        if (!strcmp(temp_name, ISandBox_Ivory_DEFAULT_PACKAGE)) {
            default_package_usingd = ISandBox_TRUE;
        }
        MEM_free(temp_name);
    }

    if (!default_package_usingd) {
        PackageName *pn;
        UsingList *req_temp;

        pn = Ivyc_create_package_name(ISandBox_Ivory_DEFAULT_PACKAGE_P1);
        pn = Ivyc_chain_package_name(pn, ISandBox_Ivory_DEFAULT_PACKAGE_P2);
        req_temp = using_list;
        using_list = Ivyc_create_using_list(pn);
        using_list->next = req_temp;
    }
    return using_list;
}

void
Ivyc_set_using_and_rename_list(UsingList *using_list,
                                RenameList *rename_list)
{
    Ivyc_Compiler *compiler;
    char *current_package_name;

    compiler = Ivyc_get_current_compiler();

    current_package_name
        = Ivyc_package_name_to_string(compiler->package_name);

    if (!ISandBox_compare_string(current_package_name,
                            ISandBox_Ivory_DEFAULT_PACKAGE)) {
        using_list = add_default_package(using_list);
    }
    MEM_free(current_package_name);
    compiler->using_list = using_list;
    compiler->rename_list = rename_list;
}

static void
add_function_to_compiler(FunctionDefinition *fd)
{
    Ivyc_Compiler *compiler;
    FunctionDefinition *pos;

    compiler = Ivyc_get_current_compiler();
    if (compiler->function_list) {
        for (pos = compiler->function_list; pos->next; pos = pos->next)
            ;
        pos->next = fd;
    } else {
        compiler->function_list = fd;
    }
}

FunctionDefinition *
Ivyc_create_function_definition(TypeSpecifier *type, char *identifier,
                               ParameterList *parameter_list,
                               ExceptionList *throws, Block *block,
							   ISandBox_Boolean if_add)
{
    FunctionDefinition *fd;
    Ivyc_Compiler *compiler;

    compiler = Ivyc_get_current_compiler();

    fd = Ivyc_malloc(sizeof(FunctionDefinition));
	fd->has_fixed = ISandBox_FALSE;
    fd->type = type;
    fd->package_name = compiler->package_name;
    fd->name = identifier;
    fd->parameter = parameter_list;
    fd->block = block;
    fd->local_variable_count = 0;
    fd->local_variable = NULL;
    fd->class_definition = NULL;
    fd->throws = throws;
    fd->end_line_number = compiler->current_line_number;
    fd->next = NULL;
    if (block) {
        block->type = FUNCTION_BLOCK;
        block->parent.function.function = fd;
    }
	if (if_add) {
    	add_function_to_compiler(fd);
	}

    return fd;
}

void
Ivyc_function_define(TypeSpecifier *type, char *identifier,
                    ParameterList *parameter_list, ExceptionList *throws,
                    Block *block)
{
    FunctionDefinition *fd;

    if (Ivyc_search_function(identifier)
        || Ivyc_search_declaration(identifier, NULL)) {
        Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                          FUNCTION_MULTIPLE_DEFINE_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
        return;
    }
    fd = Ivyc_create_function_definition(type, identifier, parameter_list,
                                        throws, block, ISandBox_TRUE);
}

ParameterList *
Ivyc_create_parameter(TypeSpecifier *type, char *identifier, Expression *initializer, ISandBox_Boolean is_vargs)
{
    ParameterList       *p;

    p = Ivyc_malloc(sizeof(ParameterList));
    p->name = identifier;
    p->type = type;
	p->initializer = initializer;
	p->is_vargs = is_vargs;
	p->has_fixed = ISandBox_FALSE;
    p->line_number = Ivyc_get_current_compiler()->current_line_number;
    p->next = NULL;

    return p;
}

ParameterList *
Ivyc_chain_parameter(ParameterList *list, TypeSpecifier *type,
                    char *identifier, Expression *initializer, ISandBox_Boolean is_vargs)
{
    ParameterList *pos;

    /*for (pos = list; pos->next; pos = pos->next)
        ;*/
    pos = Ivyc_create_parameter(type, identifier, initializer, is_vargs);
	pos->next = list;

    return pos;
}

TypeParameterList *
Ivyc_create_type_parameter(char *identifier)
{
    TypeParameterList       *p;

    p = Ivyc_malloc(sizeof(TypeParameterList));
    p->name = identifier;
	p->target = NULL;
    p->line_number = Ivyc_get_current_compiler()->current_line_number;
    p->next = NULL;

    return p;
}

TypeParameterList *
Ivyc_chain_type_parameter(TypeParameterList *list, char *identifier)
{
    TypeParameterList *pos;

    /*for (pos = list; pos->next; pos = pos->next)
        ;*/
    pos = Ivyc_create_type_parameter(identifier);
	pos->next = list;

    return pos;
}

ArgumentList *
Ivyc_create_argument_list(Expression *expression)
{
    ArgumentList *al;

    al = Ivyc_malloc(sizeof(ArgumentList));
    al->expression = expression;
	al->is_default = ISandBox_FALSE;
    al->next = NULL;

    return al;
}

TypeArgumentList *
Ivyc_create_type_argument_list(TypeSpecifier *type)
{
    TypeArgumentList *al;

    al = Ivyc_malloc(sizeof(TypeArgumentList));
    al->type = type;
    al->next = NULL;

    return al;
}

ArgumentList *
Ivyc_chain_argument_list(ArgumentList *list, Expression *expr)
{
    ArgumentList *pos;

	for (pos = list; pos->next; pos = pos->next)
        ;

    pos->next = Ivyc_create_argument_list(expr);

    return list;
}

TypeArgumentList *
Ivyc_chain_type_argument_list(TypeArgumentList *list, TypeSpecifier *type)
{
    TypeArgumentList *pos;

	for (pos = list; pos->next; pos = pos->next)
        ;

    pos->next = Ivyc_create_type_argument_list(type);

    return list;
}

ExpressionList *
Ivyc_create_expression_list(Expression *expression)
{
    ExpressionList *el;

    el = Ivyc_malloc(sizeof(ExpressionList));
    el->expression = expression;
    el->next = NULL;

    return el;
}

ExpressionList *
Ivyc_chain_expression_list(ExpressionList *list, Expression *expr)
{
    ExpressionList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_expression_list(expr);

    return list;
}

StatementList *
Ivyc_create_statement_list(Statement *statement)
{
    StatementList *sl;

    sl = Ivyc_malloc(sizeof(StatementList));
    sl->statement = statement;
    sl->next = NULL;

    return sl;
}

StatementList *
Ivyc_chain_statement_list(StatementList *list, Statement *statement)
{
    StatementList *pos;

    if (list == NULL)
        return Ivyc_create_statement_list(statement);

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_statement_list(statement);

    return list;
}


TypeSpecifier *
Ivyc_create_type_specifier(ISandBox_BasicType basic_type)
{
    TypeSpecifier *type;

    type = Ivyc_alloc_type_specifier(basic_type);
    type->line_number = Ivyc_get_current_compiler()->current_line_number;

    return type;
}

TypeSpecifier *
Ivyc_create_identifier_type_specifier(char *identifier)
{
    TypeSpecifier *type;

    type = Ivyc_alloc_type_specifier(ISandBox_UNSPECIFIED_IDENTIFIER_TYPE);
	type->is_generic = ISandBox_FALSE;
    type->identifier = identifier;
    type->line_number = Ivyc_get_current_compiler()->current_line_number;

    return type;
}

TypeSpecifier *
Ivyc_create_generic_identifier_type_specifier(char *identifier, TypeArgumentList *list)
{
    TypeSpecifier *type;

    type = Ivyc_alloc_type_specifier(ISandBox_UNSPECIFIED_IDENTIFIER_TYPE);
	type->is_generic = ISandBox_TRUE;
	type->type_argument_list = list;
    type->identifier = identifier;
    type->line_number = Ivyc_get_current_compiler()->current_line_number;

    return type;
}

TypeSpecifier *
Ivyc_create_array_type_specifier(TypeSpecifier *base)
{
    TypeDerive *new_derive;
    
    new_derive = Ivyc_alloc_type_derive(ARRAY_DERIVE);

    if (base->derive == NULL) {
        base->derive = new_derive;
    } else {
        TypeDerive *derive_p;
        for (derive_p = base->derive; derive_p->next != NULL;
             derive_p = derive_p->next)
            ;
        derive_p->next = new_derive;
    }

    return base;
}

Expression *
Ivyc_alloc_expression(ExpressionKind kind)
{
    Expression  *exp;

    exp = Ivyc_malloc(sizeof(Expression));
    exp->type = NULL;
    exp->kind = kind;
    exp->line_number = Ivyc_get_current_compiler()->current_line_number;

    return exp;
}

Expression *
Ivyc_create_comma_expression(Expression *left, Expression *right)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(COMMA_EXPRESSION);
    exp->u.comma.left = left;
    exp->u.comma.right = right;

    return exp;
}

Expression *
Ivyc_create_assign_expression(TypeSpecifier *type_left, Expression *left, AssignmentOperator operator,
                             TypeSpecifier *type_operand, Expression *operand)
{
    Expression *exp;
    Ivyc_Compiler *compiler;

    compiler = Ivyc_get_current_compiler();

    exp = Ivyc_alloc_expression(ASSIGN_EXPRESSION);
    exp->u.assign_expression.operator = operator;

    /* !!!unstable!!! */
    /*if (operator == NORMAL_ASSIGN && type_left != NULL) {
        Ivyc_compile_error(compiler->current_line_number,
                          ASSIGN_EXPRESSION_LEFT_ITEM_FORCE_CAST_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", left->u.identifier.name,
                          MESSAGE_ARGUMENT_END);
    }
    
    if (type_left != NULL)
    {
        Expression *cast_left;
        cast_left = Ivyc_alloc_expression(FORCE_CAST_EXPRESSION);
        cast_left->u.fcast.type = type_left;
        cast_left->u.fcast.operand = left;
        exp->u.assign_expression.operand = cast_left;
    }
    else
    {
        exp->u.assign_expression.left = left;
    }

    if (type_operand != NULL)
    {
        Expression *cast_operand;
        cast_operand = Ivyc_alloc_expression(FORCE_CAST_EXPRESSION);
        cast_operand->u.fcast.type = type_operand;
        cast_operand->u.fcast.operand = operand;
        exp->u.assign_expression.operand = cast_operand;
    }
    else
    {
        exp->u.assign_expression.operand = operand;
    }*/
    exp->u.assign_expression.left = left;
    exp->u.assign_expression.operand = operand;

    return exp;
}

Expression *
Ivyc_create_binary_expression(ExpressionKind operator,
                             Expression *left, Expression *right)
{
    Expression *exp;
    exp = Ivyc_alloc_expression(operator);
    exp->u.binary_expression.left = left;
    exp->u.binary_expression.right = right;
    return exp;
}

Expression *
Ivyc_create_minus_expression(Expression *operand)
{
    Expression  *exp;
    exp = Ivyc_alloc_expression(MINUS_EXPRESSION);
    exp->u.minus_expression = operand;
    return exp;
}

Expression *
Ivyc_create_logical_not_expression(Expression *operand)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(LOGICAL_NOT_EXPRESSION);
    exp->u.logical_not = operand;

    return exp;
}

Expression *
Ivyc_create_bit_not_expression(Expression *operand)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(BIT_NOT_EXPRESSION);
    exp->u.bit_not = operand;

    return exp;
}

Expression *
Ivyc_create_index_expression(Expression *array, Expression *index)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(INDEX_EXPRESSION);
    exp->u.index_expression.array = array;
    exp->u.index_expression.index = index;

    return exp;
}

Expression *
Ivyc_create_incdec_expression(Expression *operand, ExpressionKind inc_or_dec)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(inc_or_dec);
    exp->u.inc_dec.operand = operand;

    return exp;
}

Expression *
Ivyc_create_instanceof_expression(Expression *operand, TypeSpecifier *type)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(INSTANCEOF_EXPRESSION);
    exp->u.instanceof.operand = operand;
    exp->u.instanceof.type = type;

    return exp;
}

Expression *
Ivyc_create_istype_expression(Expression *operand, TypeSpecifier *type)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(ISTYPE_EXPRESSION);
    exp->u.istype.operand = operand;
    exp->u.istype.type = type;

    return exp;
}

Expression *
Ivyc_create_identifier_expression(char *identifier)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(IDENTIFIER_EXPRESSION);
    exp->u.identifier.name = identifier;

    return exp;
}

Expression *
Ivyc_create_function_call_expression(Expression *function,
                                    ArgumentList *argument)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(FUNCTION_CALL_EXPRESSION);
    exp->u.function_call_expression.function = function;
    exp->u.function_call_expression.argument = argument;

    return exp;
}

Expression *
Ivyc_create_down_cast_expression(Expression *operand, TypeSpecifier *type)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(DOWN_CAST_EXPRESSION);
    exp->u.down_cast.operand = operand;
    exp->u.down_cast.type = type;

    return exp;
}

Expression *
Ivyc_create_member_expression(Expression *expression, char *member_name)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(MEMBER_EXPRESSION);
    exp->u.member_expression.expression = expression;
    exp->u.member_expression.member_name = member_name;

    return exp;
}


Expression *
Ivyc_create_boolean_expression(ISandBox_Boolean value)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(BOOLEAN_EXPRESSION);
    exp->u.boolean_value = value;

    return exp;
}

Expression *
Ivyc_create_null_expression(void)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(NULL_EXPRESSION);

    return exp;
}

Expression *
Ivyc_create_this_expression(void)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(THIS_EXPRESSION);

    return exp;
}

Expression *
Ivyc_create_super_expression(void)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(SUPER_EXPRESSION);

    return exp;
}

Expression *
Ivyc_create_new_expression(char *class_name, TypeArgumentList *type_list, char *method_name,
                          ArgumentList *argument)
{
    Expression *exp;

    exp = Ivyc_alloc_expression(NEW_EXPRESSION);
	if (type_list != NULL) {
		exp->u.new_e.is_generic = ISandBox_TRUE;
	} else {
		exp->u.new_e.is_generic = ISandBox_FALSE;
	}
	exp->u.new_e.type_argument_list = type_list;
    exp->u.new_e.class_name = class_name;
    exp->u.new_e.class_definition = NULL;
    exp->u.new_e.method_name = method_name;
    exp->u.new_e.method_declaration = NULL;
    exp->u.new_e.argument = argument;

    return exp;
}

Expression *
Ivyc_create_array_literal_expression(ExpressionList *list)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(ARRAY_LITERAL_EXPRESSION);
    exp->u.array_literal = list;

    return exp;
}

Expression *
Ivyc_create_basic_array_creation(ISandBox_BasicType basic_type,
                                ArrayDimension *dim_expr_list,
                                ArrayDimension *dim_list)
{
    Expression  *exp;
    TypeSpecifier *type;

    type = Ivyc_create_type_specifier(basic_type);
    exp = Ivyc_create_class_array_creation(type, dim_expr_list, dim_list);

    return exp;
}

Expression *
Ivyc_create_class_array_creation(TypeSpecifier *type,
                                ArrayDimension *dim_expr_list,
                                ArrayDimension *dim_list)
{
    Expression  *exp;

    exp = Ivyc_alloc_expression(ARRAY_CREATION_EXPRESSION);
    exp->u.array_creation.type = type;
    exp->u.array_creation.dimension
        = Ivyc_chain_array_dimension(dim_expr_list, dim_list);

    return exp;
}

ArrayDimension *
Ivyc_create_array_dimension(Expression *expr)
{
    ArrayDimension *dim;

    dim = Ivyc_malloc(sizeof(ArrayDimension));
    dim->expression = expr;
    dim->next = NULL;

    return dim;
}

ArrayDimension *
Ivyc_chain_array_dimension(ArrayDimension *list, ArrayDimension *dim)
{
    ArrayDimension *pos;

    for (pos = list; pos->next != NULL; pos = pos->next)
        ;
    pos->next = dim;

    return list;
}

Statement *
Ivyc_alloc_statement(StatementType type)
{
    Statement *st;

    st = Ivyc_malloc(sizeof(Statement));
    st->type = type;
    st->line_number = Ivyc_get_current_compiler()->current_line_number;

    return st;
}

Statement *
Ivyc_create_if_statement(Expression *condition,
                        Block *then_block, Elsif *elsif_list,
                        Block *else_block)
{
    Statement *st;

    st = Ivyc_alloc_statement(IF_STATEMENT);
    st->u.if_s.condition = condition;
    st->u.if_s.then_block = then_block;
    st->u.if_s.elsif_list = elsif_list;
    st->u.if_s.else_block = else_block;

    return st;
}

Elsif *
Ivyc_chain_elsif_list(Elsif *list, Elsif *add)
{
    Elsif *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Elsif *
Ivyc_create_elsif(Expression *expr, Block *block)
{
    Elsif *ei;

    ei = Ivyc_malloc(sizeof(Elsif));
    ei->condition = expr;
    ei->block = block;
    ei->next = NULL;

    return ei;
}

Statement *
Ivyc_create_switch_statement(Expression *expression,
                            CaseList *case_list, Block *default_block)
{
    Statement *st;

    st = Ivyc_alloc_statement(SWITCH_STATEMENT);
    st->u.switch_s.expression = expression;
    st->u.switch_s.case_list = case_list;
    st->u.switch_s.default_block = default_block;
	

    return st;
}

CaseList *
Ivyc_create_one_case(ExpressionList *expression_list, StatementList *list)
{
    CaseList *case_list;
	Block *container;

    container = Ivyc_close_block(Ivyc_open_block(), list);
	container->type = CASE_STATEMENT_BLOCK;

    case_list = Ivyc_malloc(sizeof(CaseList));
    case_list->expression_list = expression_list;
    case_list->block = container;
    case_list->next = NULL;

    return case_list;
}

CaseList *
Ivyc_chain_case(CaseList *list, CaseList *add)
{
    CaseList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}

Statement *
Ivyc_create_while_statement(char *label,
                           Expression *condition, Block *block)
{
    Statement *st;

    st = Ivyc_alloc_statement(WHILE_STATEMENT);
    st->u.while_s.label = label;
    st->u.while_s.condition = condition;
    st->u.while_s.block = block;
    block->type = WHILE_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
Ivyc_create_for_statement(char *label, Expression *init, Expression *cond,
                         Expression *post, Block *block)
{
    Statement *st;

    st = Ivyc_alloc_statement(FOR_STATEMENT);
    st->u.for_s.label = label;
    st->u.for_s.init = init;
    st->u.for_s.condition = cond;
    st->u.for_s.post = post;
    st->u.for_s.block = block;
    block->type = FOR_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
Ivyc_create_do_while_statement(char *label, Block *block,
                              Expression *condition)
{
    Statement *st;

    st = Ivyc_alloc_statement(DO_WHILE_STATEMENT);
    st->u.do_while_s.label = label;
    st->u.do_while_s.block = block;
    st->u.do_while_s.condition = condition;
    block->type = DO_WHILE_STATEMENT_BLOCK;
    block->parent.statement.statement = st;

    return st;
}

Statement *
Ivyc_create_foreach_statement(char *label, char *variable,
                             Expression *collection, Block *block)
{
    Statement *st;

    st = Ivyc_alloc_statement(FOREACH_STATEMENT);
    st->u.foreach_s.label = label;
    st->u.foreach_s.variable = variable;
    st->u.foreach_s.collection = collection;
    st->u.for_s.block = block;

    return st;
}

Block *
Ivyc_alloc_block(void)
{
    Block *new_block;

    new_block = Ivyc_malloc(sizeof(Block));
    new_block->type = UNDEFINED_BLOCK;
    new_block->outer_block = NULL;
    new_block->statement_list = NULL;
    new_block->declaration_list = NULL;

    return new_block;
}

Block *
Ivyc_open_block(void)
{
    Block *new_block;

    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();
    new_block = Ivyc_alloc_block();
    new_block->outer_block = compiler->current_block;
    compiler->current_block = new_block;

    return new_block;
}

Block *
Ivyc_close_block(Block *block, StatementList *statement_list)
{
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    DBG_assert(block == compiler->current_block,
               ("block mismatch.\n"));
    block->statement_list = statement_list;
    compiler->current_block = block->outer_block;

    return block;
}

/*Block *
Ivyc_create_block_from_statement(Statement *statement)
{
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    Block *start = Ivyc_open_block();

    DBG_assert(start == compiler->current_block,
               ("block mismatch.\n"));

    start->statement_list = Ivyc_create_statement_list(statement);
    compiler->current_block = start->outer_block;

    return start;
}*/

Statement *
Ivyc_create_expression_statement(Expression *expression)
{
    Statement *st;

    st = Ivyc_alloc_statement(EXPRESSION_STATEMENT);
    st->u.expression_s = expression;

    return st;
}

Statement *
Ivyc_create_return_statement(Expression *expression)
{
    Statement *st;

    st = Ivyc_alloc_statement(RETURN_STATEMENT);
    st->u.return_s.return_value = expression;

    return st;
}

Statement *
Ivyc_create_break_statement(char *label)
{
    Statement *st;

    st = Ivyc_alloc_statement(BREAK_STATEMENT);
    st->u.break_s.label = label;

    return st;
}

Statement *
Ivyc_create_label_statement(char *label)
{
    Statement *st;

    st = Ivyc_alloc_statement(LABEL_STATEMENT);
    st->u.label_s.label = label;

    return st;
}

Statement *
Ivyc_create_goto_statement(char *target)
{
    Statement *st;

    st = Ivyc_alloc_statement(GOTO_STATEMENT);
    st->u.goto_s.target = target;

    return st;
}

Statement *
Ivyc_create_continue_statement(char *label)
{
    Statement *st;

    st = Ivyc_alloc_statement(CONTINUE_STATEMENT);
    st->u.continue_s.label = label;

    return st;
}

Statement *
Ivyc_create_try_statement(Block *try_block,
                         CatchClause *catch_clause,
                         Block *finally_block)
{
    Statement *st;

    st = Ivyc_alloc_statement(TRY_STATEMENT);
    st->u.try_s.try_block = try_block;
    try_block->type = TRY_CLAUSE_BLOCK;
    st->u.try_s.catch_clause = catch_clause;
    if (finally_block) {
        finally_block->type = FINALLY_CLAUSE_BLOCK;
    }
    st->u.try_s.finally_block = finally_block;

    return st;
}

CatchClause *
Ivyc_create_catch_clause(TypeSpecifier *type, char *variable_name,
                        Block *block)
{
    CatchClause *cc;

    cc = Ivyc_malloc(sizeof(CatchClause));
    cc->type = type;
    cc->variable_name = variable_name;
    cc->block = block;
    block->type = CATCH_CLAUSE_BLOCK;
    cc->next = NULL;

    return cc;
}

CatchClause *
Ivyc_start_catch_clause(void)
{
    CatchClause *cc;

    cc = Ivyc_malloc(sizeof(CatchClause));
    cc->line_number = Ivyc_get_current_compiler()->current_line_number;
    cc->next = NULL;

    return cc;
}

CatchClause *
Ivyc_end_catch_clause(CatchClause *catch_clause, TypeSpecifier *type,
                     char *variable_name, Block *block)
{
    catch_clause->type = type;
    catch_clause->variable_name = variable_name;
    catch_clause->block = block;

    return catch_clause;
}

CatchClause *
Ivyc_chain_catch_list(CatchClause *list, CatchClause *add)
{
    CatchClause *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = add;

    return list;
}


Statement *
Ivyc_create_throw_statement(Expression *expression)
{
    Statement *st;

    st = Ivyc_alloc_statement(THROW_STATEMENT);
    st->u.throw_s.exception = expression;

    return st;
}

Statement *
Ivyc_create_declaration_statement(ISandBox_Boolean is_final, TypeSpecifier *type,
                                 char *identifier,
                                 Expression *initializer)
{
    Statement *st;
    Declaration *decl;
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    if (is_final && initializer == NULL) {
        Ivyc_compile_error(compiler->current_line_number,
                          FINAL_VARIABLE_WITHOUT_INITIALIZER_ERR,
                          STRING_MESSAGE_ARGUMENT, "name", identifier,
                          MESSAGE_ARGUMENT_END);
    }
    st = Ivyc_alloc_statement(DECLARATION_STATEMENT);

    decl = Ivyc_alloc_declaration(is_final, type, identifier);

    decl->initializer = initializer;

    st->u.declaration_s = decl;

    return st;
}

ISandBox_AccessModifier
conv_access_modifier(ClassOrMemberModifierKind src)
{
    if (src == PUBLIC_MODIFIER) {
        return ISandBox_PUBLIC_ACCESS;
    } else if (src == PRIVATE_MODIFIER) {
        return ISandBox_PRIVATE_ACCESS;
    } else {
        DBG_assert(src == NOT_SPECIFIED_MODIFIER, ("src..%d\n", src));
        return ISandBox_FILE_ACCESS;
    }
}

void
Ivyc_start_class_definition(ClassOrMemberModifierList *modifier,
                           ISandBox_ClassOrInterface class_or_interface,
                           char *identifier,
						   TypeParameterList *list,
                           ExtendsList *extends)
{
    ClassDefinition *cd;
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    cd = Ivyc_malloc(sizeof(ClassDefinition));

	if (list) {
		cd->is_generic = ISandBox_TRUE;
	} else {
		cd->is_generic = ISandBox_FALSE;
	}
	cd->type_parameter_list = list;

    cd->is_abstract = (class_or_interface == ISandBox_INTERFACE_DEFINITION);
    cd->access_modifier = ISandBox_FILE_ACCESS;
    if (modifier) {
        if (modifier->is_abstract == ABSTRACT_MODIFIER) {
            cd->is_abstract = ISandBox_TRUE;
        }
        cd->access_modifier = conv_access_modifier(modifier->access_modifier);
    }
    cd->class_or_interface = class_or_interface;
    cd->package_name = compiler->package_name;
    cd->name = identifier;
    cd->extends = extends;
    cd->super_class = NULL;
    cd->interface_list = NULL;
    cd->member = NULL;
    cd->next = NULL;
    cd->line_number = compiler->current_line_number;

    DBG_assert(compiler->current_class_definition == NULL,
               ("current_class_definition is not NULL."));
    compiler->current_class_definition = cd;
}

void Ivyc_class_define(MemberDeclaration *member_list)
{
    Ivyc_Compiler *compiler;
    ClassDefinition *cd;
    ClassDefinition *pos;

    compiler = Ivyc_get_current_compiler();
    cd = compiler->current_class_definition;
    DBG_assert(cd != NULL, ("current_class_definition is NULL."));

	cd->member = member_list;

	if (cd->is_generic == ISandBox_TRUE) {
		if (compiler->template_class_definition_list == NULL) {
        	compiler->template_class_definition_list = cd;
    	} else {
        	for (pos = compiler->template_class_definition_list; pos->next;
             	pos = pos->next)
            	;
        	pos->next = cd;
    	}
	} else {
    	if (compiler->class_definition_list == NULL) {
        	compiler->class_definition_list = cd;
    	} else {
        	for (pos = compiler->class_definition_list; pos->next;
             	pos = pos->next)
            	;
        	pos->next = cd;
    	}
	}

    compiler->current_class_definition = NULL;
}

ExtendsList *
Ivyc_create_extends_list(char *identifier, TypeArgumentList *add_list)
{
    ExtendsList *list;

    list = Ivyc_malloc(sizeof(ExtendsList));
    list->identifier = identifier;
	list->type_argument_list = add_list;
	if (add_list != NULL) {
		list->is_generic = ISandBox_TRUE;
	} else {
		list->is_generic = ISandBox_FALSE;
	}
    list->class_definition = NULL;
    list->next = NULL;

    return list;
}

ExtendsList *
Ivyc_chain_extends_list(ExtendsList *list, char *add, TypeArgumentList *add_list)
{
    ExtendsList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_extends_list(add, add_list);

    return list;
}

ClassOrMemberModifierList
Ivyc_create_class_or_member_modifier(ClassOrMemberModifierKind modifier)
{
    ClassOrMemberModifierList ret;

    ret.is_abstract = NOT_SPECIFIED_MODIFIER;
    ret.access_modifier = NOT_SPECIFIED_MODIFIER;
    ret.is_override = NOT_SPECIFIED_MODIFIER;
    ret.is_virtual = NOT_SPECIFIED_MODIFIER;

    switch (modifier) {
    case ABSTRACT_MODIFIER:
        ret.is_abstract = ABSTRACT_MODIFIER;
        break;
    case PUBLIC_MODIFIER:
        ret.access_modifier = PUBLIC_MODIFIER;
        break;
    case PRIVATE_MODIFIER:
        ret.access_modifier = PRIVATE_MODIFIER;
        break;
    case OVERRIDE_MODIFIER:
        ret.is_override = OVERRIDE_MODIFIER;
        break;
    case VIRTUAL_MODIFIER:
        ret.is_virtual = VIRTUAL_MODIFIER;
        break;
    case NOT_SPECIFIED_MODIFIER: /* FALLTHRU */
    default:
        DBG_assert(0, ("modifier..%d", modifier));
    }

    return ret;
}

ClassOrMemberModifierList
Ivyc_chain_class_or_member_modifier(ClassOrMemberModifierList list,
                                   ClassOrMemberModifierList add)
{
    if (add.is_abstract != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_abstract == ABSTRACT_MODIFIER,
                   ("add.is_abstract..%d", add.is_abstract));
        if (list.is_abstract != NOT_SPECIFIED_MODIFIER) {
            Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                              ABSTRACT_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_abstract = ABSTRACT_MODIFIER;

    } else if (add.access_modifier != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.access_modifier == PUBLIC_MODIFIER
                   || add.access_modifier == PUBLIC_MODIFIER,
                   ("add.access_modifier..%d", add.access_modifier));
        if (list.access_modifier != NOT_SPECIFIED_MODIFIER) {
            Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                              ACCESS_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.access_modifier = add.access_modifier;

    } else if (add.is_override != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_override == OVERRIDE_MODIFIER,
                   ("add.is_override..%d", add.is_override));
        if (list.is_override != NOT_SPECIFIED_MODIFIER) {
            Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                              OVERRIDE_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_override = add.is_override;
    } else if (add.is_virtual != NOT_SPECIFIED_MODIFIER) {
        DBG_assert(add.is_virtual == VIRTUAL_MODIFIER,
                   ("add.is_virtual..%d", add.is_virtual));
        if (list.is_virtual != NOT_SPECIFIED_MODIFIER) {
            Ivyc_compile_error(Ivyc_get_current_compiler()->current_line_number,
                              VIRTUAL_MODIFIER_MULTIPLE_SPECIFIED_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        list.is_virtual = add.is_virtual;
    }
    return list;
}

MemberDeclaration *
Ivyc_chain_member_declaration(MemberDeclaration *list, MemberDeclaration *add)
{
    MemberDeclaration *pos;

    for (pos = list; pos->next; pos = pos->next)
		;
    pos->next = add;

    return list;
}

static MemberDeclaration *
alloc_member_declaration(MemberKind kind,
                         ClassOrMemberModifierList *modifier)
{
    MemberDeclaration *ret;

    ret = Ivyc_malloc(sizeof(MemberDeclaration));
    ret->kind = kind;
    if (modifier) {
        ret->access_modifier = conv_access_modifier(modifier->access_modifier);
    } else {
        ret->access_modifier = ISandBox_FILE_ACCESS;
    }
    ret->line_number = Ivyc_get_current_compiler()->current_line_number;
    ret->next = NULL;

    return ret;
}

MemberDeclaration *
Ivyc_create_method_member(ClassOrMemberModifierList *modifier,
                         FunctionDefinition *function_definition,
                         ISandBox_Boolean is_constructor)
{
    MemberDeclaration *ret;
    Ivyc_Compiler *compiler;

    ret = alloc_member_declaration(METHOD_MEMBER, modifier);
    ret->u.method.is_constructor = is_constructor;
    ret->u.method.is_abstract = ISandBox_FALSE;
    ret->u.method.is_virtual = ISandBox_FALSE;
    ret->u.method.is_override = ISandBox_FALSE;
    if (modifier) {
        if (modifier->is_abstract == ABSTRACT_MODIFIER) {
            ret->u.method.is_abstract = ISandBox_TRUE;
        }
        if (modifier->is_virtual == VIRTUAL_MODIFIER) {
            ret->u.method.is_virtual = ISandBox_TRUE;
        }
        if (modifier->is_override == OVERRIDE_MODIFIER) {
            ret->u.method.is_override = ISandBox_TRUE;
        }
    }
    compiler = Ivyc_get_current_compiler();
    if (compiler->current_class_definition->class_or_interface
        == ISandBox_INTERFACE_DEFINITION) {
        /* BUGBUG error check */
        ret->u.method.is_abstract = ISandBox_TRUE;
        ret->u.method.is_virtual = ISandBox_TRUE;
    }

    ret->u.method.function_definition = function_definition;

    if (ret->u.method.is_abstract) {
        if (function_definition->block) {
            Ivyc_compile_error(compiler->current_line_number,
                              ABSTRACT_METHOD_HAS_BODY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
    } else {
        if (function_definition->block == NULL) {
            Ivyc_compile_error(compiler->current_line_number,
                              CONCRETE_METHOD_HAS_NO_BODY_ERR,
                              MESSAGE_ARGUMENT_END);
        }
    }
    function_definition->class_definition
        = compiler->current_class_definition;

    return ret;
}

FunctionDefinition *
Ivyc_method_function_define(TypeSpecifier *type, char *identifier,
                           ParameterList *parameter_list,
                           ExceptionList *throws, Block *block)
{
    FunctionDefinition *fd;

	Ivyc_Compiler *compiler;

    compiler = Ivyc_get_current_compiler();

    if (compiler->current_class_definition != NULL
		&& compiler->current_class_definition->is_generic == ISandBox_TRUE) {
    	fd = Ivyc_create_function_definition(type, identifier, parameter_list,
                                        	throws, block, ISandBox_FALSE);
	} else {
		fd = Ivyc_create_function_definition(type, identifier, parameter_list,
                                        	throws, block, ISandBox_TRUE);
	}

    return fd;
}

FunctionDefinition *
Ivyc_constructor_function_define(char *identifier,
                                ParameterList *parameter_list,
                                ExceptionList *throws, Block *block)
{
    FunctionDefinition *fd;
    TypeSpecifier *type;

    type = Ivyc_create_type_specifier(ISandBox_VOID_TYPE);
    fd = Ivyc_method_function_define(type, identifier, parameter_list,
                                    throws, block);

    return fd;
}

MemberDeclaration *
Ivyc_create_field_member(ClassOrMemberModifierList *modifier,
                        ISandBox_Boolean is_final, TypeSpecifier *type,
						DeclarationList *list)
{
    MemberDeclaration *ret = NULL;
	MemberDeclaration *item;
	DeclarationList *pos;

	for (pos = list; pos; pos = pos->next) {
		item = alloc_member_declaration(FIELD_MEMBER, modifier);
		item->u.field.name = pos->declaration->name;
		item->u.field.type = type;
		item->u.field.initializer = pos->declaration->initializer;
		item->u.field.is_final = is_final;
		item->next = NULL;

		if (!ret) {
			ret = item;
			continue;
		}
		ret = Ivyc_chain_member_declaration(ret, item);
	}

    return ret;
}

ExceptionList *
Ivyc_create_throws(char *identifier)
{
    ExceptionList *list;

    list = Ivyc_malloc(sizeof(ExceptionList));
    list->ref = Ivyc_malloc(sizeof(ExceptionRef));
    list->ref->identifier = identifier;
    list->ref->class_definition = NULL;
    list->ref->line_number = Ivyc_get_current_compiler()->current_line_number;
    list->next = NULL;

    return list;
}

ExceptionList *
Ivyc_chain_exception_list(ExceptionList *list, char *identifier)
{
    ExceptionList *pos;

    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_throws(identifier);

    return list;
}

void
Ivyc_create_delegate_definition(TypeSpecifier *type, char *identifier,
                               ParameterList *parameter_list,
                               ExceptionList *throws)
{
    DelegateDefinition *dd;
    DelegateDefinition *pos;
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    dd = Ivyc_malloc(sizeof(DelegateDefinition));
    dd->type = type;
    dd->name = identifier;
    dd->parameter_list = parameter_list;
    dd->throws = throws;
    dd->next = NULL;

    if (compiler->delegate_definition_list == NULL) {
        compiler->delegate_definition_list = dd;
    } else {
        for (pos = compiler->delegate_definition_list; pos->next;
             pos = pos->next)
            ;
        pos->next = dd;
    }
}

void
Ivyc_create_enum_definition(char *identifier, Enumerator *enumerator)
{
    EnumDefinition *ed;
    EnumDefinition *pos;
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();
    int value;
    Enumerator *enumerator_pos;

    ed = Ivyc_malloc(sizeof(EnumDefinition));
    ed->package_name = compiler->package_name;
    ed->name = identifier;
    ed->enumerator = enumerator;
    ed->next = NULL;

    value = 0;
    for (enumerator_pos = enumerator; enumerator_pos;
         enumerator_pos = enumerator_pos->next) {
        enumerator_pos->value = value;
        value++;
    }

    if (compiler->enum_definition_list == NULL) {
        compiler->enum_definition_list = ed;
    } else {
        for (pos = compiler->enum_definition_list; pos->next;
             pos = pos->next)
            ;
        pos->next = ed;
    }
}

Enumerator *
Ivyc_create_enumerator(char *identifier)
{
    Enumerator *enumerator;

    enumerator = Ivyc_malloc(sizeof(Enumerator));
    enumerator->name = identifier;
    enumerator->value = UNDEFINED_ENUMERATOR;
    enumerator->next = NULL;

    return enumerator;
}

Enumerator *
Ivyc_chain_enumerator(Enumerator *enumerator, char *identifier)
{
    Enumerator *pos;

    for (pos = enumerator; pos->next; pos = pos->next)
        ;
    pos->next = Ivyc_create_enumerator(identifier);

    return enumerator;
}

void
Ivyc_create_const_definition(TypeSpecifier *type, char *identifier,
                            Expression *initializer)
{
    ConstantDefinition *cd;
    ConstantDefinition *pos;
    Ivyc_Compiler *compiler = Ivyc_get_current_compiler();
    
    cd = Ivyc_malloc(sizeof(ConstantDefinition));
    cd->type = type;
    cd->package_name = compiler->package_name;
    cd->name = identifier;
    cd->initializer = initializer;
    cd->line_number = compiler->current_line_number;
    cd->next = NULL;

    if (compiler->constant_definition_list == NULL) {
        compiler->constant_definition_list = cd;
    } else {
        for (pos = compiler->constant_definition_list; pos->next;
             pos = pos->next)
            ;
        pos->next = cd;
    }
}
