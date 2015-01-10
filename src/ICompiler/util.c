#include <stdio.h>
#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "Ivoryc.h"

static Ivyc_Compiler *st_current_compiler;

Ivyc_Compiler *
Ivyc_get_current_compiler(void)
{
    return st_current_compiler;
}

void
Ivyc_set_current_compiler(Ivyc_Compiler *compiler)
{
    st_current_compiler = compiler;
}

void *
Ivyc_malloc(size_t size)
{
    void *p;
    Ivyc_Compiler *compiler;

    compiler = Ivyc_get_current_compiler();
    p = MEM_storage_malloc(compiler->compile_storage, size);

    return p;
}

char *
Ivyc_strdup(char *src)
{
    char *p;
    Ivyc_Compiler *compiler;

    compiler = Ivyc_get_current_compiler();
    p = MEM_storage_malloc(compiler->compile_storage, strlen(src)+1);
    strcpy(p, src);

    return p;
}

TypeSpecifier *
Ivyc_alloc_type_specifier(ISandBox_BasicType type)
{
    TypeSpecifier *ts = Ivyc_malloc(sizeof(TypeSpecifier));

	ts->is_placeholder = ISandBox_FALSE;
	ts->is_generic = ISandBox_FALSE;
    ts->basic_type = type;
    ts->line_number = 0;
    ts->derive = NULL;
    if (type == ISandBox_CLASS_TYPE) {
        ts->identifier = NULL;
        ts->u.class_ref.class_definition = NULL;
    }
	ts->orig_identifier = NULL;

    return ts;
}

TypeDerive *
Ivyc_alloc_type_derive(DeriveTag derive_tag)
{
    TypeDerive *td = Ivyc_malloc(sizeof(TypeDerive));
    td->tag = derive_tag;
    td->next = NULL;

    return td;
}

TypeSpecifier *
Ivyc_alloc_type_specifier2(TypeSpecifier *src)
{
    TypeSpecifier *ts = Ivyc_malloc(sizeof(TypeSpecifier));

    *ts = *src;

    return ts;
}

ISandBox_Boolean
Ivyc_compare_parameter(ParameterList *param1, ParameterList *param2)
{
    ParameterList *pos1;
    ParameterList *pos2;

    for (pos1 = param1, pos2 = param2; pos1 && pos2;
         pos1 = pos1->next, pos2 = pos2->next) {
        if (strcmp(pos1->name, pos2->name) != 0) {
            return ISandBox_FALSE;
        }
        if (!Ivyc_compare_type(pos1->type, pos2->type)) {
            return ISandBox_FALSE;
        }
    }
    if (pos1 || pos2) {
        return ISandBox_FALSE;
    }

    return ISandBox_TRUE;
}

ISandBox_Boolean
Ivyc_compare_type_argument_list(TypeArgumentList *list1, TypeArgumentList *list2)
{
	TypeArgumentList *pos1;
	TypeArgumentList *pos2;

	for (pos1 = list1, pos2 = list2; pos1 && pos2; pos1 = pos1->next, pos2 = pos2->next) {
		if (!Ivyc_compare_type(pos1->type, pos2->type)) {
			return ISandBox_FALSE;
		}
	}
	if (pos1 || pos2) {
		return ISandBox_FALSE;
	}
	return ISandBox_TRUE;
}

ISandBox_Boolean
Ivyc_compare_enum(Enumerator *enum1, Enumerator *enum2)
{
	for (; enum1 && enum2; enum1 = enum1->next, enum2 = enum2->next)
	{
		if (enum1->value != enum2->value) {
			return ISandBox_FALSE;
		}
	}
	if (enum1 || enum2) {
		return ISandBox_FALSE;
	}
	return ISandBox_TRUE;
}

ISandBox_Boolean
Ivyc_compare_type(TypeSpecifier *type1, TypeSpecifier *type2)
{
    TypeDerive *d1;
    TypeDerive *d2;

    if (type1->basic_type != type2->basic_type) {
        return ISandBox_FALSE;
    }

	if (type1->is_generic && type2->is_generic) {
		if (Ivyc_compare_type_argument_list(type1->type_argument_list, type2->type_argument_list)) {
			return ISandBox_TRUE;
		} else {
			return ISandBox_FALSE;
		}
	} else if (type1->is_generic || type2->is_generic) {
		return ISandBox_FALSE;
	}

    if (type1->basic_type == ISandBox_CLASS_TYPE) {
        if (type1->u.class_ref.class_definition
            != type2->u.class_ref.class_definition) {
            return ISandBox_FALSE;
        }
    }

    if (type1->basic_type == ISandBox_DELEGATE_TYPE) {
        if (type1->u.delegate_ref.delegate_definition
            != type2->u.delegate_ref.delegate_definition) {
            return ISandBox_FALSE;
        }
    }

    if (type1->basic_type == ISandBox_ENUM_TYPE) {
        if (!Ivyc_compare_enum(type1->u.enum_ref.enum_definition->enumerator,
							  type2->u.enum_ref.enum_definition->enumerator)) {
            return ISandBox_FALSE;
        }
    }

    for (d1 = type1->derive, d2 = type2->derive;
         d1 && d2; d1 = d1->next, d2 = d2->next) {
        if (d1->tag != d2->tag) {
            return ISandBox_FALSE;
        }
        if (d1->tag == FUNCTION_DERIVE) {
            if (!Ivyc_compare_parameter(d1->u.function_d.parameter_list,
                                       d2->u.function_d.parameter_list)) {
                return ISandBox_FALSE;
            }
        }
    }
    if (d1 || d2) {
        return ISandBox_FALSE;
    }

    return ISandBox_TRUE;
}

ISandBox_Boolean
Ivyc_compare_package_name(PackageName *p1, PackageName *p2)
{
    PackageName *pos1;
    PackageName *pos2;

    for (pos1 = p1, pos2 = p2; pos1 && pos2;
         pos1 = pos1->next, pos2 = pos2->next) {
        if (strcmp(pos1->name, pos2->name) != 0) {
            return ISandBox_FALSE;
        }
    }
    if (pos1 || pos2) {
        return ISandBox_FALSE;
    }

    return ISandBox_TRUE;
}

static FunctionDefinition *
search_renamed_function(Ivyc_Compiler *compiler, RenameList *rename)
{
    FunctionDefinition *func_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!Ivyc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (func_pos = comp_pos->compiler->function_list; func_pos;
             func_pos = func_pos->next) {
            if (!strcmp(func_pos->name, rename->original_name)
                && func_pos->class_definition == NULL) {
                return func_pos;
            }
        }
    }
    return NULL;
}

FunctionDefinition *
Ivyc_search_function(char *name)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    FunctionDefinition *func_pos;

    compiler = Ivyc_get_current_compiler();

    for (func_pos = compiler->function_list; func_pos;
         func_pos = func_pos->next) {
        if (!strcmp(func_pos->name, name)
            && func_pos->class_definition == NULL) {
            return func_pos;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, name)) {
            FunctionDefinition * fd
                = search_renamed_function(compiler, ren_pos);
            if (fd) {
                return fd;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (func_pos = comp_pos->compiler->function_list; func_pos;
             func_pos = func_pos->next) {
            if (!strcmp(func_pos->name, name)
                && func_pos->class_definition == NULL) {
                return func_pos;
            }
        }
    }

    return NULL;
}

Declaration *
Ivyc_search_declaration(char *identifier, Block *block)
{
    Block *b_pos;
    DeclarationList *d_pos;
    Ivyc_Compiler *compiler;

    for (b_pos = block; b_pos; b_pos = b_pos->outer_block) {
        for (d_pos = b_pos->declaration_list; d_pos; d_pos = d_pos->next) {
            if (!strcmp(identifier, d_pos->declaration->name)) {
                return d_pos->declaration;
            }
        }
    }

    compiler = Ivyc_get_current_compiler();

    for (d_pos = compiler->declaration_list; d_pos; d_pos = d_pos->next) {
        if (!strcmp(identifier, d_pos->declaration->name)) {
            return d_pos->declaration;
        }
    }

    return NULL;
}

static ConstantDefinition *
search_renamed_constant(Ivyc_Compiler *compiler, RenameList *rename)
{
    ConstantDefinition *cd_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!Ivyc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (cd_pos = comp_pos->compiler->constant_definition_list;
             cd_pos; cd_pos = cd_pos->next) {
            if (!strcmp(cd_pos->name, rename->original_name)) {
                return cd_pos;
            }
        }
    }
    return NULL;
}


ConstantDefinition *
Ivyc_search_constant(char *identifier)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    ConstantDefinition *cd_pos;

    compiler = Ivyc_get_current_compiler();
    for (cd_pos = compiler->constant_definition_list;
         cd_pos; cd_pos = cd_pos->next) {
        if (!strcmp(cd_pos->name, identifier)) {
            return cd_pos;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            cd_pos = search_renamed_constant(compiler, ren_pos);
            if (cd_pos) {
                return cd_pos;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (cd_pos = comp_pos->compiler->constant_definition_list;
             cd_pos; cd_pos = cd_pos->next) {
            if (!strcmp(cd_pos->name, identifier)) {
                return cd_pos;
            }
        }
    }

    return NULL;
}

static ClassDefinition *
search_renamed_class(Ivyc_Compiler *compiler, RenameList *rename)
{
    ClassDefinition *class_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!Ivyc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (class_pos = comp_pos->compiler->class_definition_list; class_pos;
             class_pos = class_pos->next) {
            if (!strcmp(class_pos->name, rename->original_name)) {
                return class_pos;
            }
        }
    }
    return NULL;
}

ClassDefinition *
Ivyc_search_class(char *identifier)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    ClassDefinition *class_def;

    compiler = Ivyc_get_current_compiler();
    for (class_def = compiler->class_definition_list;
         class_def; class_def = class_def->next) {
        if (!strcmp(class_def->name, identifier)) {
            return class_def;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            class_def = search_renamed_class(compiler, ren_pos);
            if (class_def) {
                return class_def;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (class_def = comp_pos->compiler->class_definition_list;
             class_def; class_def = class_def->next) {
            if (!strcmp(class_def->name, identifier)) {
                return class_def;
            }
        }
    }

    return NULL;
}

ClassDefinition *
Ivyc_search_template_class(char *identifier)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    ClassDefinition *class_def;

    compiler = Ivyc_get_current_compiler();
    for (class_def = compiler->template_class_definition_list;
         class_def; class_def = class_def->next) {
        if (strcmp(class_def->name, identifier) == 0) {
            return class_def;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            class_def = search_renamed_class(compiler, ren_pos);
            if (class_def) {
                return class_def;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (class_def = comp_pos->compiler->template_class_definition_list;
             class_def; class_def = class_def->next) {
            if (!strcmp(class_def->name, identifier)) {
                return class_def;
            }
        }
    }

    return NULL;
}

static DelegateDefinition *
search_renamed_delegate(Ivyc_Compiler *compiler, RenameList *rename)
{
    DelegateDefinition *delegate_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!Ivyc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (delegate_pos = comp_pos->compiler->delegate_definition_list;
             delegate_pos; delegate_pos = delegate_pos->next) {
            if (!strcmp(delegate_pos->name, rename->original_name)) {
                return delegate_pos;
            }
        }
    }
    return NULL;
}

DelegateDefinition *
Ivyc_search_delegate(char *identifier)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    DelegateDefinition *delegate_def;

    compiler = Ivyc_get_current_compiler();
    for (delegate_def = compiler->delegate_definition_list;
         delegate_def; delegate_def = delegate_def->next) {
        if (!strcmp(delegate_def->name, identifier)) {
            return delegate_def;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            delegate_def = search_renamed_delegate(compiler, ren_pos);
            if (delegate_def) {
                return delegate_def;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (delegate_def = comp_pos->compiler->delegate_definition_list;
             delegate_def; delegate_def = delegate_def->next) {
            if (!strcmp(delegate_def->name, identifier)) {
                return delegate_def;
            }
        }
    }

    return NULL;
}

static EnumDefinition *
search_renamed_enum(Ivyc_Compiler *compiler, RenameList *rename)
{
    EnumDefinition *enum_pos;
    CompilerList *comp_pos;

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        if (!Ivyc_compare_package_name(rename->package_name,
                                      comp_pos->compiler->package_name)) {
            continue;
        }
        for (enum_pos = comp_pos->compiler->enum_definition_list;
             enum_pos; enum_pos = enum_pos->next) {
            if (!strcmp(enum_pos->name, rename->original_name)) {
                return enum_pos;
            }
        }
    }
    return NULL;
}

EnumDefinition *
Ivyc_search_enum(char *identifier)
{
    Ivyc_Compiler *compiler;
    RenameList *ren_pos;
    CompilerList *comp_pos;
    EnumDefinition *enum_def;

    compiler = Ivyc_get_current_compiler();
    for (enum_def = compiler->enum_definition_list;
         enum_def; enum_def = enum_def->next) {
        if (!strcmp(enum_def->name, identifier)) {
            return enum_def;
        }
    }

    for (ren_pos = compiler->rename_list; ren_pos; ren_pos = ren_pos->next) {
        if (!strcmp(ren_pos->renamed_name, identifier)) {
            enum_def = search_renamed_enum(compiler, ren_pos);
            if (enum_def) {
                return enum_def;
            }
        }
    }

    for (comp_pos = compiler->usingd_list; comp_pos;
         comp_pos = comp_pos->next) {
        for (enum_def = comp_pos->compiler->enum_definition_list;
             enum_def; enum_def = enum_def->next) {
            if (!strcmp(enum_def->name, identifier)) {
                return enum_def;
            }
        }
    }

    return NULL;
}

ISandBox_Boolean
Ivyc_is_initializable(TypeSpecifier *type)
{
	if (Ivyc_is_array(type)) {
		return ISandBox_FALSE;
	}
	if (Ivyc_is_int(type)
		|| Ivyc_is_enum(type)
		|| Ivyc_is_boolean(type)
		|| Ivyc_is_double(type)
		|| Ivyc_is_long_double(type)
		|| Ivyc_is_delegate(type)
		/* || Ivyc_is_string(type) */) {
		return ISandBox_TRUE;
	}

	return ISandBox_FALSE;
}

ISandBox_Boolean
Ivyc_is_castable(TypeSpecifier *type1, TypeSpecifier *type2)
{
	if (Ivyc_compare_type(type1, type2)) {
        return ISandBox_TRUE;
    }

    if (Ivyc_is_object(type2)
        && type1->basic_type == ISandBox_NULL_TYPE) {
        DBG_assert(type1->derive == NULL, ("derive != NULL"));
        return ISandBox_TRUE;
    }

    if (Ivyc_is_class_object(type1) && Ivyc_is_class_object(type2)) {
        return ISandBox_TRUE;
    }

    if (Ivyc_is_function(type1) && Ivyc_is_delegate(type2)) {
        return ISandBox_TRUE;
    }

    if (Ivyc_is_int(type1) && Ivyc_is_double(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_int(type1) && Ivyc_is_long_double(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_double(type1) && Ivyc_is_int(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_long_double(type1) && Ivyc_is_int(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_double(type1) && Ivyc_is_long_double(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_long_double(type1) && Ivyc_is_double(type2)) {
        return ISandBox_TRUE;
    } else if (Ivyc_is_string(type2)) {
    	if (Ivyc_is_boolean(type1)) {
			return ISandBox_TRUE;
		} else if (Ivyc_is_int(type1)) {
			return ISandBox_TRUE;
		} else if (Ivyc_is_double(type1)) {
			return ISandBox_TRUE;
		} else if (Ivyc_is_long_double(type1)) {
			return ISandBox_TRUE;
		} else if (Ivyc_is_enum(type1)) {
			return ISandBox_TRUE;
		}
    } else if (Ivyc_is_type_object(type2)) {
        return ISandBox_TRUE;
    }

    return ISandBox_FALSE;
}

ISandBox_Boolean
Ivyc_compare_arguments(ParameterList *param, ArgumentList *args)
{
	ParameterList *pos1;
	ArgumentList *pos2;

	for (pos1 = param, pos2 = args;
		pos1 && pos2; pos1 = pos1->next, pos2 = pos2->next) {
		/*if (!Ivyc_is_castable(pos2->expression->type, pos1->type)) {
			return ISandBox_FALSE;
		}*/
	}

	if (pos1 != NULL && pos2 == NULL) {
		for (; pos1 && pos1->initializer; pos1 = pos1->next)
			;
	}

	if (pos1 || pos2) {
		return ISandBox_FALSE;
	}

	return ISandBox_TRUE;
}

MemberDeclaration *
Ivyc_search_initialize(ClassDefinition *class_def, ArgumentList *args)
{
    MemberDeclaration *member;
    ExtendsList *extends_p;
    
    for (member = class_def->member; member;
         member = member->next) {
        if (member->kind == METHOD_MEMBER
			&& member->u.method.is_constructor
			&& Ivyc_compare_arguments(member->u.method.function_definition->parameter, args)) {
			break;
		}
    }
    if (member) {
        return member;
    }

    if (class_def->super_class) {
        member = Ivyc_search_initialize(class_def->super_class, args);
    }
    if (member) {
        return member;
    }

    for (extends_p = class_def->interface_list;
         extends_p;
         extends_p = extends_p->next) {
        member = Ivyc_search_initialize(extends_p->class_definition, args);
        if (member) {
            return member;
        }
    }

    return NULL;
}

MemberDeclaration *
Ivyc_search_member(ClassDefinition *class_def,
                  char *member_name)
{
    MemberDeclaration *member;
    ExtendsList *extends_p;
    
    for (member = class_def->member; member;
         member = member->next) {
        if (member->kind == METHOD_MEMBER) {
            if (!strcmp(member->u.method.function_definition->name,
                        member_name)) {
                break;
            }
        } else {
            DBG_assert(member->kind == FIELD_MEMBER,
                       ("member..%d", member->kind));
            if (!strcmp(member->u.field.name, member_name)) {
                break;
            }
        }
    }
    if (member) {
        return member;
    }
    if (class_def->super_class) {
        member = Ivyc_search_member(class_def->super_class, member_name);
    }
    if (member) {
        return member;
    }
    for (extends_p = class_def->interface_list;
         extends_p;
         extends_p = extends_p->next) {
        member = Ivyc_search_member(extends_p->class_definition, member_name);
        if (member) {
            return member;
        }
    }

    return NULL;
}

void
Ivyc_vstr_clear(VString *v)
{
    v->string = NULL;
}

static int
my_strlen(char *str)
{
    if (str == NULL) {
        return 0;
    }
    return strlen(str);
}

void
Ivyc_vstr_append_string(VString *v, char *str)
{
    int new_size;
    int old_len;

    old_len = my_strlen(v->string);
    new_size = old_len + strlen(str)  + 1;
    v->string = MEM_realloc(v->string, new_size);
    strcpy(&v->string[old_len], str);
}

void
Ivyc_vstr_append_character(VString *v, int ch)
{
    int current_len;
    
    current_len = my_strlen(v->string);
    v->string = MEM_realloc(v->string, current_len + 2);
    v->string[current_len] = ch;
    v->string[current_len+1] = '\0';
}

void
Ivyc_vwstr_clear(VWString *v)
{
    v->string = NULL;
}

static int
my_wcslen(ISandBox_Char *str)
{
    if (str == NULL) {
        return 0;
    }
    return ISandBox_wcslen(str);
}

void
Ivyc_vwstr_append_string(VWString *v, ISandBox_Char *str)
{
    int new_size;
    int old_len;

    old_len = my_wcslen(v->string);
    new_size = sizeof(ISandBox_Char) * (old_len + ISandBox_wcslen(str)  + 1);
    v->string = MEM_realloc(v->string, new_size);
    ISandBox_wcscpy(&v->string[old_len], str);
}

void
Ivyc_vwstr_append_character(VWString *v, int ch)
{
    int current_len;
    
    current_len = my_wcslen(v->string);
    v->string = MEM_realloc(v->string,sizeof(ISandBox_Char) * (current_len + 2));
    v->string[current_len] = ch;
    v->string[current_len+1] = L'\0';
}

char *
Ivyc_get_basic_type_name(ISandBox_BasicType type)
{
    switch (type) {
    case ISandBox_VOID_TYPE:
        return "void";
        break;
    case ISandBox_BOOLEAN_TYPE:
        return "boolean";
        break;
    case ISandBox_INT_TYPE:
        return "int";
        break;
    case ISandBox_DOUBLE_TYPE:
        return "double";
        break;
    case ISandBox_LONG_DOUBLE_TYPE:
        return "long double";
        break;
    case ISandBox_OBJECT_TYPE:
        return "object";
        break;
    case ISandBox_ITERATOR_TYPE:
        return "iterator";
        break;
    case ISandBox_STRING_TYPE:
        return "string";
        break;
    case ISandBox_NATIVE_POINTER_TYPE:
        return "native_pointer";
        break;
    case ISandBox_CLASS_TYPE:
        return "class";
        break;
    case ISandBox_NULL_TYPE:
        return "null";
        break;
	case ISandBox_BASE_TYPE: /* FALLTHRU */
		return "base";
		break;
	case ISandBox_PLACEHOLDER: /* FALLTHRU */
		return "placeholder";
		break;
    case ISandBox_DELEGATE_TYPE: /* FALLTHRU */
    case ISandBox_ENUM_TYPE: /* FALLTHRU */
    case ISandBox_UNSPECIFIED_IDENTIFIER_TYPE: /* FALLTHRU */
    default:
        DBG_assert(0, ("bad case. type..%d\n", type));
    }
    return NULL;
}

static void
function_type_to_string(VString *vstr, TypeDerive *derive)
{
    ParameterList *param_pos;
    ExceptionList *e_pos;

    Ivyc_vstr_append_string(vstr, "(");
    for (param_pos = derive->u.function_d.parameter_list; param_pos;
         param_pos = param_pos->next) {
        char *type_name = Ivyc_get_type_name(param_pos->type);
        Ivyc_vstr_append_string(vstr, type_name);
        Ivyc_vstr_append_string(vstr, " ");
        Ivyc_vstr_append_string(vstr, param_pos->name);
        if (param_pos->next) {
            Ivyc_vstr_append_string(vstr, ", ");
        }
    }
    Ivyc_vstr_append_string(vstr, ")");

    if (derive->u.function_d.throws) {
        Ivyc_vstr_append_string(vstr, " throws ");
        for (e_pos = derive->u.function_d.throws; e_pos;
             e_pos = e_pos->next) {
            Ivyc_vstr_append_string(vstr, e_pos->ref->identifier);
            if (e_pos->next) {
                Ivyc_vstr_append_string(vstr, ", ");
            }
        }
    }
}

char *
Ivyc_get_type_name(TypeSpecifier *type)
{
    VString     vstr;
    TypeDerive  *derive_pos;

    Ivyc_vstr_clear(&vstr);

    if (type->basic_type == ISandBox_CLASS_TYPE
        || type->basic_type == ISandBox_DELEGATE_TYPE
        || type->basic_type == ISandBox_ENUM_TYPE) {
        Ivyc_vstr_append_string(&vstr, type->identifier);
    } else {
        Ivyc_vstr_append_string(&vstr,
                               Ivyc_get_basic_type_name(type->basic_type));
    }

    for (derive_pos = type->derive; derive_pos;
         derive_pos = derive_pos->next) {
        switch (derive_pos->tag) {
        case FUNCTION_DERIVE:
            function_type_to_string(&vstr, derive_pos);
            break;
        case ARRAY_DERIVE:
            Ivyc_vstr_append_string(&vstr, "[]");
            break;
        default:
            DBG_assert(0, ("derive_tag..%d\n", derive_pos->tag));
        }
    }

    return vstr.string;
}

ISandBox_Char *
Ivyc_expression_to_string(Expression *expr)
{
    char        buf[LINE_BUF_SIZE];
    ISandBox_Char    wc_buf[LINE_BUF_SIZE];
    int         len;
    ISandBox_Char    *new_str;

    if (expr->kind == BOOLEAN_EXPRESSION) {
        if (expr->u.boolean_value) {
            ISandBox_mbstowcs("true", wc_buf);
        } else {
            ISandBox_mbstowcs("false", wc_buf);
        }
    } else if (expr->kind == INT_EXPRESSION) {
        sprintf(buf, "%d", expr->u.int_value);
        ISandBox_mbstowcs(buf, wc_buf);
    } else if (expr->kind == DOUBLE_EXPRESSION) {
        sprintf(buf, "%f", expr->u.double_value);
        ISandBox_mbstowcs(buf, wc_buf);
    } else if (expr->kind == LONG_DOUBLE_EXPRESSION) {
        sprintf(buf, "%Lf", expr->u.long_double_value);
        ISandBox_mbstowcs(buf, wc_buf);
    } else if (expr->kind == STRING_EXPRESSION) {
        return expr->u.string_value;
    } else {
        return NULL;
    }
    len = ISandBox_wcslen(wc_buf);
    new_str = MEM_malloc(sizeof(ISandBox_Char) * (len + 1));
    ISandBox_wcscpy(new_str, wc_buf);

    return new_str;
}

char *
Ivyc_package_name_to_string(PackageName *src)
{
    int len = 0;
    PackageName *pos;
    char *dest;

    if (src == NULL) {
        return NULL;
    }
    for (pos = src; pos; pos = pos->next) {
        len += strlen(pos->name) + 1;
    }


    dest = MEM_malloc(len);
    dest[0] = '\0';
    for (pos = src; pos; pos = pos->next) {
        strcat(dest, pos->name);
        if (pos->next) {
            strcat(dest, ".");
        }
    }

    return dest;
}

char *
Ivyc_get_folder_by_path(char *path)
{
	int length;
	int i;
	char *ret;

	length = strlen(path);
	for (i = length - 1;
		path[i] != '\\'
		&& path[i] != '/'
		&& i >= 0; i--);
	if (i != 0) {
		ret = (char *)malloc(sizeof(char) * (i + 1));
		strncpy(ret, path, i);
		ret[i] = NULL;
	} else {
		ret = ".";
	}
	return ret;
}
