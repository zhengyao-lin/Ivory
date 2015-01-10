%{
#include <stdio.h>
#include "Ivoryc.h"
#define YYDEBUG 1
%}
%union {
    char                *identifier;
    PackageName         *package_name;
    UsingList           *using_list;
    RenameList          *rename_list;
    ParameterList       *parameter_list;
    ArgumentList        *argument_list;
	TypeParameterList   *type_parameter_list;
	TypeArgumentList	*type_argument_list;
    Expression          *expression;
    ExpressionList      *expression_list;
    Statement           *statement;
    StatementList       *statement_list;
    Block               *block;
    Elsif               *elsif;
    CaseList            *case_list;
    CatchClause         *catch_clause;
    AssignmentOperator  assignment_operator;
    TypeSpecifier       *type_specifier;
    ISandBox_BasicType       basic_type_specifier;
    ArrayDimension      *array_dimension;
    ClassOrMemberModifierList class_or_member_modifier;
    ISandBox_ClassOrInterface class_or_interface;
    ExtendsList         *extends_list;
    MemberDeclaration   *member_declaration;
    FunctionDefinition  *function_definition;
    ExceptionList       *exception_list;
    Enumerator          *enumerator;
    Declaration         *declaration;
    DeclarationList     *declaration_list;
    TypeSpecifier       *force_cast_type;
}
%token <expression>     INT_LITERAL
%token <expression>     DOUBLE_LITERAL
%token <expression>     LONG_DOUBLE_LITERAL
%token <expression>     STRING_LITERAL
%token <expression>     REGEXP_LITERAL
%token <identifier>     IDENTIFIER
%token IF ELSE ELSIF SWITCH CASE DEFAULT_T WHILE DO_T FOR FOREACH
        RETURN_T FALL_THROUGH GOTO BREAK CONTINUE NULL_T
        LP RP LC RC LB RB SEMICOLON COLON COMMA ASSIGN_T LOGICAL_AND LOGICAL_OR
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD BIT_AND BIT_OR BIT_XOR BIT_NOT
        TRUE_T FALSE_T EXCLAMATION DOT
        ADD_ASSIGN_T SUB_ASSIGN_T MUL_ASSIGN_T DIV_ASSIGN_T MOD_ASSIGN_T
        INCREMENT DECREMENT TRY CATCH FINALLY THROW THROWS
        VOID_T VARIABLE_ARGS ITERATOR_T VARIABLE_T BASE_T BOOLEAN_T INT_T DOUBLE_T LONG_DOUBLE_T OBJECT_T STRING_T VCLASS_T NATIVE_POINTER_T
        WITH NEW USING RENAME
        CLASS_T INTERFACE_T PUBLIC_T PRIVATE_T VIRTUAL_T OVERRIDE_T
        ABSTRACT_T THIS_T SUPER_T CONSTRUCTOR INSTANCEOF ISTYPE
        DOWN_CAST_BEGIN DOWN_CAST_END WL DELEGATE FINAL ENUM CONST

%type   <package_name> package_name
%type   <using_list> using_list using_declaration
%type   <rename_list> rename_list rename_declaration
%type   <parameter_list> parameter_list
%type   <argument_list> argument_list
%type   <type_parameter_list> type_parameter_list type_parameters
%type	<type_argument_list> type_argument_list type_arguments
%type   <expression> expression expression_opt
        assignment_expression logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression postfix_expression primary_expression
        primary_no_new_array array_literal array_creation
        /*initializer_opt*/ parameter_initializer_opt

/* !!!unstable!!! */
%type   <force_cast_type> force_cast

%type   <expression_list> expression_list case_expression_list
%type   <statement> statement
        if_statement switch_statement
        while_statement for_statement do_while_statement foreach_statement
        return_statement label_statement goto_statement break_statement continue_statement try_statement
        throw_statement declaration_list_statement fall_through_statement

/* !!!unstable!!! */
%type   <declaration> declaration_opt
%type   <declaration_list> declaration_items
/* !!!unstable!!! */

%type   <statement_list> statement_list
%type   <block> block default_opt
%type   <elsif> elsif elsif_list
%type   <case_list> case_list one_case
%type   <catch_clause> catch_clause catch_list
%type   <assignment_operator> assignment_operator
%type   <identifier> identifier_opt label_opt
%type   <type_specifier> type_specifier identifier_type_specifier
        array_type_specifier
%type   <basic_type_specifier> basic_type_specifier
%type   <array_dimension> dimension_expression dimension_expression_list
        dimension_list
%type   <class_or_member_modifier> class_or_member_modifier
        class_or_member_modifier_list access_modifier
%type   <class_or_interface> class_or_interface
%type   <extends_list> extends_list extends
%type   <member_declaration> member_declaration member_declaration_list
        method_member field_member
%type   <function_definition> method_function_definition
        constructor_definition
%type   <exception_list> exception_list throws_clause
%type   <enumerator> enumerator_list
%%
translation_unit
		: initial_declaration
        | initial_declaration definition_or_statement
        | translation_unit definition_or_statement
        ;
initial_declaration
        : /* empty */
        {
            Ivyc_set_using_and_rename_list(NULL, NULL);
        }
        | using_list rename_list
        {
            Ivyc_set_using_and_rename_list($1, $2);
        }
        | using_list
        {
            Ivyc_set_using_and_rename_list($1, NULL);
        }
        | rename_list
        {
            Ivyc_set_using_and_rename_list(NULL, $1);
        }
        ;
using_list
        : using_declaration
        | using_list using_declaration
        {
            $$ = Ivyc_chain_using_list($1, $2);
        }
        ;
using_declaration
        : USING package_name SEMICOLON
        {
            $$ = Ivyc_create_using_list($2);
        }
        ;
package_name
        : IDENTIFIER
        {
            $$ = Ivyc_create_package_name($1);
        }
        | package_name DOT IDENTIFIER
        {
            $$ = Ivyc_chain_package_name($1, $3);
        }
        ;
rename_list
        : rename_declaration
        | rename_list rename_declaration
        {
            $$ = Ivyc_chain_rename_list($1, $2);
        }
        ;
rename_declaration
        : RENAME package_name IDENTIFIER SEMICOLON
        {
            $$ = Ivyc_create_rename_list($2, $3);
        }
        ;
definition_or_statement
		: function_definition
        | class_definition
        | statement
        {
            Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

            compiler->statement_list
                = Ivyc_chain_statement_list(compiler->statement_list, $1);
        }
		| label_statement
		{
			Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

            compiler->statement_list
                = Ivyc_chain_statement_list(compiler->statement_list, $1);
		}
        | delegate_definition
        | enum_definition
		| const_definition
        ;
basic_type_specifier
        : VOID_T
        {
            $$ = ISandBox_VOID_TYPE;
        }
		| BASE_T
		{
			$$ = ISandBox_BASE_TYPE;
		}
        | BOOLEAN_T
        {
            $$ = ISandBox_BOOLEAN_TYPE;
        }
        | INT_T
        {
            $$ = ISandBox_INT_TYPE;
        }
        | LONG_DOUBLE_T
        {
            $$ = ISandBox_LONG_DOUBLE_TYPE;
        }
        | DOUBLE_T
        {
            $$ = ISandBox_DOUBLE_TYPE;
        }
        | OBJECT_T
        {
            $$ = ISandBox_OBJECT_TYPE;
        }
        | ITERATOR_T
        {
            $$ = ISandBox_ITERATOR_TYPE;
        }
        | STRING_T
        {
            $$ = ISandBox_STRING_TYPE;
        }
        | NATIVE_POINTER_T
        {
            $$ = ISandBox_NATIVE_POINTER_TYPE;
        }
        ;
identifier_type_specifier
        : IDENTIFIER
        {
            $$ = Ivyc_create_identifier_type_specifier($1);
        }
		| IDENTIFIER type_arguments
		{
			$$ = Ivyc_create_generic_identifier_type_specifier($1, $2);
		}
        ;
array_type_specifier
        : type_specifier LB RB
        {
            $$ = Ivyc_create_array_type_specifier($1);
        }
        ;
type_argument_list
        : type_specifier
        {
            $$ = Ivyc_create_type_argument_list($1);
        }
        | type_specifier COMMA type_argument_list 
        {
            $$ = Ivyc_chain_type_argument_list($3, $1);
        }
        ;
type_arguments
        : /* NULL */
        {
            $$ = NULL;
        }
        | WITH LT type_argument_list GT
        {
            $$ = $3;
        }
        ;
type_specifier
        : basic_type_specifier
        {
        	$$ = Ivyc_create_type_specifier($1);
        }
        | array_type_specifier
        | identifier_type_specifier
        ;
function_definition
        : type_specifier IDENTIFIER LP parameter_list RP throws_clause block
        {
            Ivyc_function_define($1, $2, $4, $6, $7);
        }
        | type_specifier IDENTIFIER LP RP throws_clause block
        {
            Ivyc_function_define($1, $2, NULL, $5, $6);
        }
        | type_specifier IDENTIFIER LP parameter_list RP throws_clause
          SEMICOLON
        {
            Ivyc_function_define($1, $2, $4, $6, NULL);
        }
        | type_specifier IDENTIFIER LP RP throws_clause SEMICOLON
        {
            Ivyc_function_define($1, $2, NULL, $5, NULL);
        }
        ;
parameter_initializer_opt
		: /* NULL */
		{
			$$ = NULL;
		}
		| ASSIGN_T assignment_expression
		{
			$$ = $2;
		}
		;
parameter_list
        : type_specifier IDENTIFIER parameter_initializer_opt
        {
            $$ = Ivyc_create_parameter($1, $2, $3, ISandBox_FALSE);
        }
		| VARIABLE_ARGS IDENTIFIER
        {
			Expression *num_zero = Ivyc_alloc_expression(INT_EXPRESSION);
			num_zero->u.int_value = 0;
			Expression *init = Ivyc_create_basic_array_creation(ISandBox_OBJECT_TYPE, Ivyc_create_array_dimension(num_zero), NULL);

			TypeSpecifier *basic_type = Ivyc_create_type_specifier(ISandBox_OBJECT_TYPE);
            $$ = Ivyc_create_parameter(Ivyc_create_array_type_specifier(basic_type), $2, init, ISandBox_TRUE);
        }
		| VARIABLE_ARGS
        {
			Expression *num_zero = Ivyc_alloc_expression(INT_EXPRESSION);
			num_zero->u.int_value = 0;
			Expression *init = Ivyc_create_basic_array_creation(ISandBox_OBJECT_TYPE, Ivyc_create_array_dimension(num_zero), NULL);

			TypeSpecifier *basic_type = Ivyc_create_type_specifier(ISandBox_OBJECT_TYPE);
            $$ = Ivyc_create_parameter(Ivyc_create_array_type_specifier(basic_type), UNDEFINED_VARIABLE_ARGS, init, ISandBox_TRUE);
        }
		| type_specifier IDENTIFIER parameter_initializer_opt COMMA parameter_list
        {
            $$ = Ivyc_chain_parameter($5, $1, $2, $3, ISandBox_FALSE);
        }
        ;
argument_list
		: /* empty */
		{
			$$ = NULL;
		}
        | assignment_expression
        {
            $$ = Ivyc_create_argument_list($1);
        }
        | argument_list COMMA assignment_expression
        {
            $$ = Ivyc_chain_argument_list($1, $3);
        }
        ;
statement_list
        : statement
        {
            $$ = Ivyc_create_statement_list($1);
        }
        | label_statement
        {
            $$ = Ivyc_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = Ivyc_chain_statement_list($1, $2);
        }
		| statement_list label_statement
		{
			$$ = Ivyc_chain_statement_list($1, $2);
		}
        ;
expression
        : assignment_expression
        | expression COMMA assignment_expression
        {
            $$ = Ivyc_create_comma_expression($1, $3);
        }
        ;
force_cast
        : LP type_specifier RP
        {
            $$ = $2;
        }
        ;
assignment_expression
        : logical_or_expression
        | primary_expression assignment_operator assignment_expression
        {
            $$ = Ivyc_create_assign_expression(NULL, $1, $2, NULL, $3);
        }
        ;
assignment_operator
        : ASSIGN_T
        {
            $$ = NORMAL_ASSIGN;
        }
        | ADD_ASSIGN_T
        {
            $$ = ADD_ASSIGN;
        }
        | SUB_ASSIGN_T
        {
            $$ = SUB_ASSIGN;
        }
        | MUL_ASSIGN_T
        {
            $$ = MUL_ASSIGN;
        }
        | DIV_ASSIGN_T
        {
            $$ = DIV_ASSIGN;
        }
        | MOD_ASSIGN_T
        {
            $$ = MOD_ASSIGN;
        }
        ;
logical_or_expression
        : logical_and_expression
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = Ivyc_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;
logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = Ivyc_create_binary_expression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;
equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = Ivyc_create_binary_expression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = Ivyc_create_binary_expression(NE_EXPRESSION, $1, $3);
        }
        ;
relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = Ivyc_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = Ivyc_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = Ivyc_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = Ivyc_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = Ivyc_create_binary_expression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = Ivyc_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;
multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = Ivyc_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = Ivyc_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = Ivyc_create_binary_expression(MOD_EXPRESSION, $1, $3);
        }
        | multiplicative_expression BIT_AND unary_expression
        {
            $$ = Ivyc_create_binary_expression(BIT_AND_EXPRESSION, $1, $3);
        }
        | multiplicative_expression BIT_OR unary_expression
        {
            $$ = Ivyc_create_binary_expression(BIT_OR_EXPRESSION, $1, $3);
        }
        | multiplicative_expression BIT_XOR unary_expression
        {
            $$ = Ivyc_create_binary_expression(BIT_XOR_EXPRESSION, $1, $3);
        }
        ;
unary_expression
        : postfix_expression
        | SUB unary_expression
        {
            $$ = Ivyc_create_minus_expression($2);
        }
        | EXCLAMATION unary_expression
        {
            $$ = Ivyc_create_logical_not_expression($2);
        }
        | BIT_NOT unary_expression
        {
            $$ = Ivyc_create_bit_not_expression($2);
        }
        | force_cast unary_expression
        {
            $$  = Ivyc_create_force_cast_expression($1, $2);
        }
        ;
postfix_expression
        : primary_expression
        | primary_expression INCREMENT
        {
            $$ = Ivyc_create_incdec_expression($1, INCREMENT_EXPRESSION);
        }
        | INCREMENT primary_expression
        {
            $$ = Ivyc_create_incdec_expression($2, INCREMENT_EXPRESSION);
        }
        | primary_expression DECREMENT
        {
            $$ = Ivyc_create_incdec_expression($1, DECREMENT_EXPRESSION);
        }
        | DECREMENT primary_expression
        {
            $$ = Ivyc_create_incdec_expression($2, DECREMENT_EXPRESSION);
        }
        | primary_expression INSTANCEOF type_specifier
        {
            $$ = Ivyc_create_instanceof_expression($1, $3);
        }
        | primary_expression ISTYPE type_specifier
        {
            $$ = Ivyc_create_istype_expression($1, $3);
        }
        ;
primary_expression
        : primary_no_new_array
        | array_creation
        | IDENTIFIER
        {
            $$ = Ivyc_create_identifier_expression($1);
        }
        ;
primary_no_new_array
        : primary_no_new_array LB expression RB
        {
            $$ = Ivyc_create_index_expression($1, $3);
        }
        | IDENTIFIER LB expression RB
        {
            Expression *identifier = Ivyc_create_identifier_expression($1);
            $$ = Ivyc_create_index_expression(identifier, $3);
        }
        | primary_expression DOT IDENTIFIER
        {
            $$ = Ivyc_create_member_expression($1, $3);
        }
        | primary_expression LP argument_list RP
        {
            $$ = Ivyc_create_function_call_expression($1, $3);
        }
        | LP expression RP
        {
            $$ = $2;
        }
        | primary_expression DOWN_CAST_BEGIN type_specifier DOWN_CAST_END
        {
            $$ = Ivyc_create_down_cast_expression($1, $3);
        }
        | INT_LITERAL
		| basic_type_specifier
		{
			Expression  *expression = Ivyc_alloc_expression(INT_EXPRESSION);
    		expression->u.int_value = $1;
    		$$ = expression;
		}
        | LONG_DOUBLE_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | REGEXP_LITERAL
        | TRUE_T
        {
            $$ = Ivyc_create_boolean_expression(ISandBox_TRUE);
        }
        | FALSE_T
        {
            $$ = Ivyc_create_boolean_expression(ISandBox_FALSE);
        }
        | NULL_T
        {
            $$ = Ivyc_create_null_expression();
        }
        | array_literal
        | THIS_T
        {
            $$ = Ivyc_create_this_expression();
        }
        | SUPER_T
        {
            $$ = Ivyc_create_super_expression();
        }
        | NEW IDENTIFIER type_arguments LP argument_list RP
        {
            $$ = Ivyc_create_new_expression($2, $3, NULL, $5);
        }
        | NEW IDENTIFIER type_arguments DOT IDENTIFIER LP argument_list RP
        {
            $$ = Ivyc_create_new_expression($2, $3, $5, $7);
        }
        ;
array_literal
        : LC expression_list RC
        {
            $$ = Ivyc_create_array_literal_expression($2);
        }
        | LC expression_list COMMA RC
        {
            $$ = Ivyc_create_array_literal_expression($2);
        }
        ;
/*var_args_list
        : expression_list
        {
            $$ = Ivyc_create_var_args_list_expression($1);
        }
        ;*/
array_creation
        : NEW basic_type_specifier dimension_expression_list
        {
            $$ = Ivyc_create_basic_array_creation($2, $3, NULL);
        }
        | NEW basic_type_specifier dimension_expression_list dimension_list
        {
            $$ = Ivyc_create_basic_array_creation($2, $3, $4);
        }
        | NEW identifier_type_specifier dimension_expression_list
        {
            $$ = Ivyc_create_class_array_creation($2, $3, NULL);
        }
        | NEW identifier_type_specifier dimension_expression_list
            dimension_list
        {
            $$ = Ivyc_create_class_array_creation($2, $3, $4);
        }
        ;
dimension_expression_list
        : dimension_expression
        | dimension_expression_list dimension_expression
        {
            $$ = Ivyc_chain_array_dimension($1, $2);
        }
        ;
dimension_expression
        : LB expression RB
        {
            $$ = Ivyc_create_array_dimension($2);
        }
        ;
dimension_list
        : LB RB
        {
            $$ = Ivyc_create_array_dimension(NULL);
        }
        | dimension_list LB RB
        {
            $$ = Ivyc_chain_array_dimension($1,
                                           Ivyc_create_array_dimension(NULL));
        }
        ;
expression_list
        : /* empty */
        {
            $$ = NULL;
        }
        | assignment_expression
        {
            $$ = Ivyc_create_expression_list($1);
        }
        | expression_list COMMA assignment_expression
        {
            $$ = Ivyc_chain_expression_list($1, $3);
        }
        ;
statement
        : expression SEMICOLON
        {
          $$ = Ivyc_create_expression_statement($1);
        }
        | if_statement
        | switch_statement
        | while_statement
        | for_statement
        | do_while_statement
        | foreach_statement
        | goto_statement
        | return_statement
        | break_statement
        | continue_statement
        | try_statement
        | throw_statement
        | declaration_list_statement
		| fall_through_statement
        ;
fall_through_statement
		: FALL_THROUGH SEMICOLON
		{
			$$ = Ivyc_alloc_statement(FALL_THROUGH_STATEMENT);
		}
		;

if_statement
        : IF LP expression RP block
        {
            $$ = Ivyc_create_if_statement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = Ivyc_create_if_statement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elsif_list
        {
            $$ = Ivyc_create_if_statement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elsif_list ELSE block
        {
            $$ = Ivyc_create_if_statement($3, $5, $6, $8);
        }
        ;
elsif_list
        : elsif
        | elsif_list elsif
        {
            $$ = Ivyc_chain_elsif_list($1, $2);
        }
        ;
elsif
        : ELSE IF LP expression RP block
        {
            $$ = Ivyc_create_elsif($4, $6);
        }
        | ELSIF LP expression RP block
        {
            $$ = Ivyc_create_elsif($3, $5);
        }
        ;
label_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER COLON
        {
            $$ = $1;
        }
        ;
switch_statement
        : SWITCH LP expression RP LC case_list default_opt RC
        {
            $$ = Ivyc_create_switch_statement($3, $6, $7);
        }
        ;
case_list
        : one_case
        | case_list one_case
        {
            $$ = Ivyc_chain_case($1, $2);
        }
        ;
one_case
        : CASE case_expression_list COLON statement_list
        {
            $$ = Ivyc_create_one_case($2, $4);
        }
		| CASE case_expression_list COLON LC statement_list RC
        {
            $$ = Ivyc_create_one_case($2, $5);
        }
		| CASE case_expression_list COLON LC RC
        {
            $$ = Ivyc_create_one_case($2, NULL);
        }
		| CASE case_expression_list COLON
        {
            $$ = Ivyc_create_one_case($2, NULL);
        }
        ;
default_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | DEFAULT_T COLON statement_list
        {
            $$ = Ivyc_close_block(Ivyc_open_block(), $3);
        }
		| DEFAULT_T COLON LC statement_list RC
        {
            $$ = Ivyc_close_block(Ivyc_open_block(), $4);
        }
        ;
case_expression_list
        : assignment_expression
        {
            $$ = Ivyc_create_expression_list($1);
        }
        | expression_list COMMA assignment_expression
        {
            $$ = Ivyc_chain_expression_list($1, $3);
        }
        ;
while_statement
        : label_opt WHILE LP expression RP block
        {
            $$ = Ivyc_create_while_statement($1, $4, $6);
        }
        ;
for_statement
        : label_opt FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = Ivyc_create_for_statement($1, $4, $6, $8, $10);
        }
        ;
do_while_statement
        : label_opt DO_T block WHILE LP expression RP SEMICOLON
        {
            $$ = Ivyc_create_do_while_statement($1, $3, $6);
        }
        ;
foreach_statement
        : label_opt FOREACH LP IDENTIFIER COLON expression RP block
        {
            $$ = Ivyc_create_foreach_statement($1, $4, $6, $8);
        }
        ;
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;
return_statement
        : RETURN_T expression_opt SEMICOLON
        {
            $$ = Ivyc_create_return_statement($2);
        }
        ;
identifier_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER
        ;
break_statement 
        : BREAK identifier_opt SEMICOLON
        {
            $$ = Ivyc_create_break_statement($2);
        }
        ;
label_statement
        : LB IDENTIFIER RB
        {
            $$ = Ivyc_create_label_statement($2);
        }
        ;
goto_statement
        : GOTO IDENTIFIER SEMICOLON
        {
            $$ = Ivyc_create_goto_statement($2);
        }
        ;
continue_statement
        : CONTINUE identifier_opt SEMICOLON
        {
            $$ = Ivyc_create_continue_statement($2);
        }
        ;
try_statement
        : TRY block catch_list FINALLY block
        {
            $$ = Ivyc_create_try_statement($2, $3, $5);
        }
        | TRY block FINALLY block
        {
            $$ = Ivyc_create_try_statement($2, NULL, $4);
        }
        | TRY block catch_list
        {
            $$ = Ivyc_create_try_statement($2, $3, NULL);
        }
        ;
catch_list
        : catch_clause
        | catch_list catch_clause
        {
            $$ = Ivyc_chain_catch_list($1, $2);
        }
        ;
catch_clause
        : CATCH
        {
            $<catch_clause>$ = Ivyc_start_catch_clause();
        }
          LP type_specifier IDENTIFIER RP block
        {
            $<catch_clause>$ = Ivyc_end_catch_clause($<catch_clause>2, $4, $5, $7);
        }
        ;
throw_statement
        : THROW expression SEMICOLON
        {
            $$ = Ivyc_create_throw_statement($2);
        }
        | THROW SEMICOLON
        {
            $$ = Ivyc_create_throw_statement(NULL);
        }
        ;
declaration_list_statement
        : type_specifier declaration_items SEMICOLON
        {
            $$ = Ivyc_retype_declaration_list(ISandBox_FALSE, $1, $2);
        }
        | VARIABLE_T declaration_items SEMICOLON
        {
            $$ = Ivyc_retype_declaration_list(ISandBox_FALSE, NULL, $2);
        }
        | FINAL type_specifier declaration_items SEMICOLON
        {
            $$ = Ivyc_retype_declaration_list(ISandBox_TRUE, $2, $3);
        }
        | FINAL VARIABLE_T declaration_items SEMICOLON
        {
            $$ = Ivyc_retype_declaration_list(ISandBox_TRUE, NULL, $3);
        }
        ;
declaration_items
        : declaration_opt
        {
            $$ = Ivyc_create_declaration_list($1);
        }
        | declaration_items COMMA declaration_opt
        {
            $$ = Ivyc_chain_declaration($1, $3);
        }
        ;
declaration_opt
        : IDENTIFIER
        {
            $$ = Ivyc_create_declaration(ISandBox_FALSE, NULL, $1, NULL);
        }
        | IDENTIFIER ASSIGN_T assignment_expression
        {
            $$ = Ivyc_create_declaration(ISandBox_FALSE, NULL, $1, $3);
        }
        ;
block
        : LC
        {
            $<block>$ = Ivyc_open_block();
        }
          statement_list RC
        {
            $<block>$ = Ivyc_close_block($<block>2, $3);
        }
        | LC RC
        {
            Block *empty_block = Ivyc_open_block();
            $<block>$ = Ivyc_close_block(empty_block, NULL);
        }
		| COLON statement
		{
            $$ = Ivyc_close_block(Ivyc_open_block(), Ivyc_create_statement_list($2));
		}
		| COLON SEMICOLON
		{
            $$ = Ivyc_close_block(Ivyc_open_block(), NULL);
		}
        ;
type_parameter_list
        : IDENTIFIER
        {
            $$ = Ivyc_create_type_parameter($1);
        }
		| IDENTIFIER COMMA type_parameter_list
        {
            $$ = Ivyc_chain_type_parameter($3, $1);
        }
        ;
type_parameters
        : /* NULL */
        {
            $$ = NULL;
        }
		| LT type_parameter_list GT
        {
            $$ = $2;
        }
        ;
class_definition
        : class_or_interface IDENTIFIER type_parameters
          extends LC
        {
            Ivyc_start_class_definition(NULL, $1, $2, $3, $4);
        }
          member_declaration_list RC
        {
            Ivyc_class_define($7);
        }
		/***********************************************************/
        | class_or_member_modifier_list class_or_interface IDENTIFIER type_parameters
          extends LC
        {
            Ivyc_start_class_definition(&$1, $2, $3, $4, $5);
        } member_declaration_list RC
        {
            Ivyc_class_define($8);
        }
		/***********************************************************/
        | class_or_interface IDENTIFIER type_parameters extends LC
        {
            Ivyc_start_class_definition(NULL, $1, $2, $3, $4);
        }
          RC
        {
            Ivyc_class_define(NULL);
        }
		/***********************************************************/
        | class_or_member_modifier_list class_or_interface IDENTIFIER type_parameters
          extends LC
        {
            Ivyc_start_class_definition(&$1, $2, $3, $4, $5);
        }
          RC
        {
            Ivyc_class_define(NULL);
        }
        ;
class_or_member_modifier_list
        : class_or_member_modifier
        | class_or_member_modifier_list class_or_member_modifier
        {
            $$ = Ivyc_chain_class_or_member_modifier($1, $2);
        }
        ;
class_or_member_modifier
        : access_modifier
        | VIRTUAL_T
        {
            $$ = Ivyc_create_class_or_member_modifier(VIRTUAL_MODIFIER);
        }
        | OVERRIDE_T
        {
            $$ = Ivyc_create_class_or_member_modifier(OVERRIDE_MODIFIER);
        }
        | ABSTRACT_T
        {
            $$ = Ivyc_create_class_or_member_modifier(ABSTRACT_MODIFIER);
        }
        ;
class_or_interface
        : CLASS_T
        {
            $$ = ISandBox_CLASS_DEFINITION;
        }
        | INTERFACE_T
        {
            $$ = ISandBox_INTERFACE_DEFINITION;
        }
        ;
extends
        : /* empty */
        {
            $$ = NULL;
        }
        | COLON extends_list
        {
            $$ = $2;
        }
        ;
extends_list
        : IDENTIFIER type_arguments
        {
            $$ = Ivyc_create_extends_list($1, $2);
        }
        | extends_list COMMA IDENTIFIER type_arguments
        {
            $$ = Ivyc_chain_extends_list($1, $3, $4);
        }
        ;
member_declaration_list
        : member_declaration
        | member_declaration_list member_declaration
        {
            $$ = Ivyc_chain_member_declaration($1, $2);
        }
        ;
member_declaration
        : method_member
        | field_member
        ;
method_member
        : method_function_definition
        {
            $$ = Ivyc_create_method_member(NULL, $1, ISandBox_FALSE);
        }
        | class_or_member_modifier_list method_function_definition
        {
            $$ = Ivyc_create_method_member(&$1, $2, ISandBox_FALSE);
        }
        | constructor_definition
        {
            $$ = Ivyc_create_method_member(NULL, $1, ISandBox_TRUE);
        }
        | class_or_member_modifier_list constructor_definition
        {
            $$ = Ivyc_create_method_member(&$1, $2, ISandBox_TRUE);
        }
        ;
method_function_definition
        : type_specifier IDENTIFIER LP parameter_list RP throws_clause block
        {
            $$ = Ivyc_method_function_define($1, $2, $4, $6, $7);
        }
        | type_specifier IDENTIFIER LP RP throws_clause block
        {
            $$ = Ivyc_method_function_define($1, $2, NULL, $5, $6);
        }
        | type_specifier IDENTIFIER LP parameter_list RP throws_clause
          SEMICOLON
        {
            $$ = Ivyc_method_function_define($1, $2, $4, $6, NULL);
        }
        | type_specifier IDENTIFIER LP RP throws_clause SEMICOLON
        {
            $$ = Ivyc_method_function_define($1, $2, NULL, $5, NULL);
        }
        ;
throws_clause
        : /* empty */
        {
            $$ = NULL;
        }
        | THROWS exception_list
        {
            $$ = $2;
        }
        ;
exception_list
        : IDENTIFIER
        {
            $$ = Ivyc_create_throws($1);
        }
        | exception_list COMMA IDENTIFIER
        {
            $$ = Ivyc_chain_exception_list($1, $3);
        }
        ;
constructor_definition
        : CONSTRUCTOR IDENTIFIER LP parameter_list RP throws_clause block
        {
            $$ = Ivyc_constructor_function_define($2, $4, $6, $7);
        }
        | CONSTRUCTOR IDENTIFIER LP RP throws_clause block
        {
            $$ = Ivyc_constructor_function_define($2, NULL, $5, $6);
        }
        | CONSTRUCTOR IDENTIFIER LP parameter_list RP throws_clause SEMICOLON
        {
            $$ = Ivyc_constructor_function_define($2, $4, $6, NULL);
        }
        | CONSTRUCTOR IDENTIFIER LP RP throws_clause SEMICOLON
        {
            $$ = Ivyc_constructor_function_define($2, NULL, $5, NULL);
        }
        ;
access_modifier
        : PUBLIC_T
        {
            $$ = Ivyc_create_class_or_member_modifier(PUBLIC_MODIFIER);
        }
        | PRIVATE_T
        {
            $$ = Ivyc_create_class_or_member_modifier(PRIVATE_MODIFIER);
        }
        ;
/*initializer_opt
        :
        {
            $$ = NULL;
        }
        | ASSIGN_T expression
        {
            $$ = $2;
        }
        ;*/
field_member
        : type_specifier declaration_items SEMICOLON
        {
            $$ = Ivyc_create_field_member(NULL, ISandBox_FALSE, $1, $2);
        }
        | class_or_member_modifier_list type_specifier
          declaration_items SEMICOLON
        {
            $$ = Ivyc_create_field_member(&$1, ISandBox_FALSE, $2, $3);
        }
        | FINAL type_specifier declaration_items SEMICOLON
        {
            $$ = Ivyc_create_field_member(NULL, ISandBox_TRUE, $2, $3);
        }
        | class_or_member_modifier_list
          FINAL type_specifier declaration_items SEMICOLON
        {
            $$ = Ivyc_create_field_member(&$1, ISandBox_TRUE, $3, $4);
        }
        ;
delegate_definition
        : DELEGATE type_specifier IDENTIFIER LP parameter_list RP throws_clause
          SEMICOLON
        {
            Ivyc_create_delegate_definition($2, $3, $5, $7);
        }
        | DELEGATE type_specifier IDENTIFIER LP RP throws_clause SEMICOLON
        {
            Ivyc_create_delegate_definition($2, $3, NULL, $6);
        }
        ;
enum_definition
        : ENUM IDENTIFIER LC enumerator_list RC
        {
            Ivyc_create_enum_definition($2, $4);
        }
        | ENUM IDENTIFIER LC enumerator_list COMMA RC
        {
            Ivyc_create_enum_definition($2, $4);
        }
        ;
enumerator_list
        : IDENTIFIER
        {
            $$ = Ivyc_create_enumerator($1);
        }
        | enumerator_list COMMA IDENTIFIER
        {
            $$ = Ivyc_chain_enumerator($1, $3);
        }
        ;
const_definition
        : CONST IDENTIFIER ASSIGN_T expression SEMICOLON
        {
            Ivyc_create_const_definition(NULL, $2, $4);
        }
        | CONST type_specifier IDENTIFIER ASSIGN_T expression SEMICOLON
        {
            Ivyc_create_const_definition($2, $3, $5);
        }
        ;
%%
