#include <string.h>
#include "MEM.h"
#include "DBG.h"
#include "Ivoryc.h"

static CompilerList *st_compiler_list = NULL;
extern BuiltinScript Ivyc_builtin_script[];

typedef struct {
    char *name;
    ISandBox_BasicType type;
} BuiltInMethodParameter;

typedef struct {
    char *name;
    ISandBox_BasicType return_type;
    BuiltInMethodParameter *parameter;
    int parameter_count;
} BuiltInMethod;

static BuiltInMethodParameter st_array_resize_arg[] = {
    {"new_size", ISandBox_INT_TYPE}
};
static BuiltInMethodParameter st_array_insert_arg[] = {
    {"index", ISandBox_INT_TYPE},
    {"new_item", ISandBox_BASE_TYPE}
};
static BuiltInMethodParameter st_array_remove_arg[] = {
    {"index", ISandBox_INT_TYPE}
};
static BuiltInMethodParameter st_array_add_arg[] = {
    {"new_item", ISandBox_BASE_TYPE}
};

static BuiltInMethod st_array_method[] = {
    {ARRAY_METHOD_SIZE, ISandBox_INT_TYPE, NULL, 0},
    {ARRAY_METHOD_RESIZE, ISandBox_VOID_TYPE, st_array_resize_arg,
     ARRAY_SIZE(st_array_resize_arg)},
    {ARRAY_METHOD_INSERT, ISandBox_VOID_TYPE, st_array_insert_arg,
     ARRAY_SIZE(st_array_insert_arg)},
    {ARRAY_METHOD_REMOVE, ISandBox_VOID_TYPE, st_array_remove_arg,
     ARRAY_SIZE(st_array_remove_arg)},
    {ARRAY_METHOD_ADD, ISandBox_VOID_TYPE, st_array_add_arg,
     ARRAY_SIZE(st_array_add_arg)},
    {ARRAY_METHOD_ITERATOR, ISandBox_ITERATOR_TYPE, NULL, 0},
};

static BuiltInMethodParameter st_string_substr_arg[] = {
    {"start", ISandBox_INT_TYPE},
    {"length", ISandBox_INT_TYPE}
};

static BuiltInMethod st_string_method[] = {
    {"length", ISandBox_INT_TYPE, NULL, 0},
    {"substr", ISandBox_STRING_TYPE, st_string_substr_arg,
     ARRAY_SIZE(st_string_substr_arg)},
};

/* iterator method
 */
static BuiltInMethod st_iterator_method[] = {
    {"next", ISandBox_BASE_TYPE, NULL, 0},
    {"hasNext", ISandBox_BOOLEAN_TYPE, NULL, 0},
    {"current", ISandBox_BASE_TYPE, NULL, 0},
};

static FunctionDefinition *
create_built_in_method(BuiltInMethod *src, int method_count)
{
    int i;
    int param_idx;
    ParameterList *param_list;
    FunctionDefinition *fd_array;

    fd_array = Ivyc_malloc(sizeof(FunctionDefinition) * method_count);
    for (i = 0; i < method_count; i++) {
        fd_array[i].name = src[i].name;
        fd_array[i].type = Ivyc_alloc_type_specifier(src[i].return_type);
        param_list = NULL;
        for (param_idx = 0; param_idx < src[i].parameter_count; param_idx++) {
            TypeSpecifier *type
                = Ivyc_alloc_type_specifier(src[i].parameter[param_idx].type);
            if (param_list) {
                param_list
                    = Ivyc_chain_parameter(param_list, type,
                                          src[i].parameter[param_idx].name, NULL, ISandBox_FALSE);
            } else {
                param_list
                    = Ivyc_create_parameter(type,
                                           src[i].parameter[param_idx].name, NULL, ISandBox_FALSE);
            }
        }
        fd_array[i].parameter = param_list;
        fd_array[i].throws = NULL;
    }
    return fd_array;
}

Ivyc_Compiler *
Ivyc_create_compiler(void)
{
    MEM_Storage storage;
    Ivyc_Compiler *compiler;
    Ivyc_Compiler *compiler_backup;

    compiler_backup = Ivyc_get_current_compiler();

    storage = MEM_open_storage(0);
    compiler = MEM_storage_malloc(storage,
                                  sizeof(struct Ivyc_Compiler_tag));
    Ivyc_set_current_compiler(compiler);
    compiler->compile_storage = storage;
    compiler->package_name = NULL;
    compiler->source_suffix = IVY_SOURCE;
    compiler->using_list = NULL;
    compiler->rename_list = NULL;
    compiler->function_list = NULL;
    compiler->ISandBox_function_count = 0;
    compiler->ISandBox_function = NULL;
    compiler->ISandBox_enum_count = 0;
    compiler->ISandBox_enum = NULL;
    /*compiler->ISandBox_constant_count = 0;
    compiler->ISandBox_constant = NULL;*/
    compiler->ISandBox_class_count = 0;
    compiler->ISandBox_class = NULL;
    compiler->declaration_list = NULL;
    compiler->statement_list = NULL;
    compiler->class_definition_list = NULL;
    compiler->template_class_definition_list = NULL;
    compiler->delegate_definition_list = NULL;
    compiler->enum_definition_list = NULL;
    compiler->constant_definition_list = NULL;
    compiler->current_block = NULL;
    compiler->current_line_number = 1;
    compiler->current_class_definition = NULL;
    compiler->current_catch_clause = NULL;
	compiler->current_function_is_constructor = ISandBox_FALSE;
    compiler->input_mode = FILE_INPUT_MODE;
    compiler->usingd_list = NULL;
    compiler->array_method_count = ARRAY_SIZE(st_array_method);
    compiler->array_method
        = create_built_in_method(st_array_method,
                                 ARRAY_SIZE(st_array_method));
    compiler->string_method_count = ARRAY_SIZE(st_string_method);
    compiler->string_method
        = create_built_in_method(st_string_method,
                                 ARRAY_SIZE(st_string_method));
    compiler->iterator_method_count = ARRAY_SIZE(st_iterator_method);/* iterator */
    compiler->iterator_method
        = create_built_in_method(st_iterator_method,
                                 ARRAY_SIZE(st_iterator_method));

#ifdef EUC_SOURCE
    compiler->source_encoding = EUC_ENCODING;
#else
#ifdef SHIFT_JIS_SOURCE
    compiler->source_encoding = SHIFT_JIS_ENCODING;
#else
#ifdef UTF_8_SOURCE
    compiler->source_encoding = UTF_8_ENCODING;
#else
    DBG_panic(("source encoding is not defined.\n"));
#endif
#endif
#endif

    Ivyc_set_current_compiler(compiler_backup);

    return compiler;
}

static CompilerList *
add_compiler_to_list(CompilerList *list, Ivyc_Compiler *compiler)
{
    CompilerList *pos;
    CompilerList *new_item;

    new_item = MEM_malloc(sizeof(CompilerList));
    new_item->compiler = compiler;
    new_item->next = NULL;

    if (list == NULL) {
        return new_item;
    }
    for (pos = list; pos->next; pos = pos->next)
        ;
    pos->next = new_item;

    return list;
}

static Ivyc_Compiler *
search_compiler(CompilerList *list, PackageName *package_name)
{
    CompilerList *pos;

    for (pos = list; pos; pos = pos->next) {
        if (Ivyc_compare_package_name(pos->compiler->package_name,
                                     package_name))
            break;
    }

    if (pos) {
        return pos->compiler;
    } else {
        return NULL;
    }
}

static ISandBox_Boolean
search_buitin_source(char *package_name, SourceSuffix suffix,
                     SourceInput *input)
{
    int i;

    for (i = 0; Ivyc_builtin_script[i].source_string != NULL; i++) {
        if (ISandBox_compare_string(package_name,
                               Ivyc_builtin_script[i].package_name)
            && Ivyc_builtin_script[i].suffix == suffix) {
            input->input_mode = STRING_INPUT_MODE;
            input->u.string.lines = Ivyc_builtin_script[i].source_string;
            return ISandBox_TRUE;
        }
    }
    return ISandBox_FALSE;
}

static void
make_search_path(int line_number, PackageName *package_name, char *buf, char *tail)
{
    PackageName *pos;
    int len = 0;
    int prev_len = 0;
    int suffix_len;

    suffix_len = strlen(tail);
    buf[0] = '\0';
    for (pos = package_name; pos; pos = pos->next) {
        prev_len = len;
        len += strlen(pos->name);
        if (len > FILENAME_MAX - (2 + suffix_len)) {
            Ivyc_compile_error(line_number,
                              PACKAGE_NAME_TOO_LONG_ERR,
                              MESSAGE_ARGUMENT_END);
        }
        strcpy(&buf[prev_len], pos->name);
        if (pos->next) {
            buf[strlen(buf)] = FILE_SEPARATOR;
            len++;
        }
    }
    strcpy(&buf[len], tail);
}

static void
make_search_path_impl(char *package_name, char *buf)
{
    int suffix_len;
    int package_len;
    int i;

    suffix_len = strlen(Ivory_IMPLEMENTATION_SUFFIX);
    package_len = strlen(package_name);

    DBG_assert(package_len <= FILENAME_MAX - (2 + suffix_len),
               ("package name is too long(%s)", package_name));

    for (i = 0; package_name[i] != '\0'; i++) {
        if (package_name[i] == '.') {
            buf[i] = FILE_SEPARATOR;
        } else {
            buf[i] = package_name[i];
        }
    }
    buf[i] = '\0';
    strcat(buf, Ivory_IMPLEMENTATION_SUFFIX);
}

static void
get_using_input(UsingList *req, char *found_path,
                  SourceInput *source_input)
{
    char *search_path;
    char search_file[FILENAME_MAX];
    char research_file[FILENAME_MAX];
    FILE *fp;
    FILE *fp2;
    char *package_name;
    SearchFileStatus status;
    SearchFileStatus status2;
	Ivyc_Compiler *compiler = Ivyc_get_current_compiler();

    package_name = Ivyc_package_name_to_string(req->package_name);

    if (search_buitin_source(package_name, IVH_SOURCE, source_input)) {
        MEM_free(package_name);
        found_path[0] = '\0';
        return;
    }

    MEM_free(package_name);

    search_path = getenv(Ivory_USING_FILE_DEFAULT_PATH);
    if (search_path == NULL) {
        search_path = Ivyc_get_folder_by_path(compiler->path);
    }

    make_search_path(req->line_number, req->package_name, search_file, Ivory_USING_SUFFIX);
    make_search_path(req->line_number, req->package_name, research_file, Ivory_IMPLEMENTATION_SUFFIX);
    status = ISandBox_search_file(search_path, search_file, found_path, &fp);
	status2 = ISandBox_search_file(search_path, research_file, found_path, &fp2);
    
    if (status != SEARCH_FILE_SUCCESS && status2 != SEARCH_FILE_SUCCESS) {
        if (status == SEARCH_FILE_NOT_FOUND && status2 == SEARCH_FILE_NOT_FOUND) {
	        Ivyc_compile_error(req->line_number,
	                          USING_FILE_NOT_FOUND_ERR,
	                          STRING_MESSAGE_ARGUMENT, "file1", search_file,
	                          STRING_MESSAGE_ARGUMENT, "file2", research_file,
	                          MESSAGE_ARGUMENT_END);
        } else {
            Ivyc_compile_error(req->line_number,
                              USING_FILE_ERR,
                              INT_MESSAGE_ARGUMENT, "status", (int)status,
                              MESSAGE_ARGUMENT_END);
        }
    }

	source_input->input_mode = FILE_INPUT_MODE;
	if (status == SEARCH_FILE_SUCCESS) {
    	source_input->u.file.fp = fp;
		req->source_suffix = IVH_SOURCE;
	} else {
    	source_input->u.file.fp = fp2;
		req->source_suffix = IVY_SOURCE;
	}
}

static ISandBox_Boolean
add_exe_to_list(ISandBox_Executable *exe, ISandBox_ExecutableList *list)
{
    ISandBox_ExecutableItem *new_item;
    ISandBox_ExecutableItem *pos;
    ISandBox_ExecutableItem *tail;

    for (pos = list->list; pos; pos = pos->next) {
        if (ISandBox_compare_package_name(pos->executable->package_name,
                                     exe->package_name)
            && pos->executable->is_usingd == exe->is_usingd) {
            return ISandBox_FALSE;
        }
        tail = pos;
    }

    new_item = MEM_malloc(sizeof(ISandBox_ExecutableItem));
    new_item->executable = exe;
    new_item->next = NULL;

    if (list->list == NULL) {
        list->list = new_item;
    } else {
        tail->next = new_item;
    }

    return ISandBox_TRUE;
}

static void
set_path_to_compiler(Ivyc_Compiler *compiler, char *path)
{
    compiler->path = MEM_storage_malloc(compiler->compile_storage,
                                        strlen(path) + 1);
    strcpy(compiler->path, path);
}

static SourceSuffix
get_source_suffix(char *path)
{
	if (path[strlen(path)-1] == 'h') {
		return IVH_SOURCE;
	}
	return IVY_SOURCE;
}

static ISandBox_Executable *
do_compile(Ivyc_Compiler *compiler, ISandBox_ExecutableList *list,
           char *path, ISandBox_Boolean is_usingd)
{
    extern FILE *yyin;
    extern int yyparse(void);
    UsingList *req_pos;
    Ivyc_Compiler *req_comp;
    ISandBox_Executable *exe;
    ISandBox_Executable *req_exe;
    char found_path[FILENAME_MAX];
    Ivyc_Compiler *compiler_backup;
    SourceInput source_input;

    compiler_backup = Ivyc_get_current_compiler();
    Ivyc_set_current_compiler(compiler);
    if (yyparse()) {
        fprintf(stderr, "Serious Panic Error!\n");
        exit(1);
    }

    for (req_pos = compiler->using_list; req_pos;
         req_pos = req_pos->next) {
        req_comp = search_compiler(st_compiler_list, req_pos->package_name);
        if (req_comp) {
            compiler->usingd_list
                = add_compiler_to_list(compiler->usingd_list, req_comp);
            continue;
        }

        req_comp = Ivyc_create_compiler();

        /* BUGBUG req_comp references parent compiler's MEM_storage */
        req_comp->package_name = req_pos->package_name;
		req_comp->source_suffix = req_pos->source_suffix;

        compiler->usingd_list
            = add_compiler_to_list(compiler->usingd_list, req_comp);
        st_compiler_list = add_compiler_to_list(st_compiler_list, req_comp);

        get_using_input(req_pos, found_path, &source_input);

        set_path_to_compiler(req_comp, found_path);
        req_comp->input_mode = source_input.input_mode;
        if (source_input.input_mode == FILE_INPUT_MODE) {
            yyin = source_input.u.file.fp;
        } else {
            Ivyc_set_source_string(source_input.u.string.lines);
        }
        req_exe = do_compile(req_comp, list, found_path, ISandBox_TRUE);
    }
    Ivyc_fix_tree(compiler);
    exe = Ivyc_generate(compiler);
    if (path) {
        exe->path = MEM_strdup(path);
    } else {
        exe->path = NULL;
    }
    /*ISandBox_disassemble(exe);*/

    exe->is_usingd = is_usingd;
    if (!add_exe_to_list(exe, list)) {
        ISandBox_dispose_executable(exe);
    }

    Ivyc_set_current_compiler(compiler_backup);

    return exe;
}

static void
dispose_compiler_list(void)
{
    CompilerList *temp;

    while (st_compiler_list) {
        temp = st_compiler_list;
        st_compiler_list = temp->next;
        MEM_free(temp);
    }
}


ISandBox_ExecutableList *
Ivyc_compile(Ivyc_Compiler *compiler, FILE *fp, char *path)
{
    extern FILE *yyin;
    ISandBox_ExecutableList *list;
    ISandBox_Executable *exe;
    DBG_assert(st_compiler_list == NULL,
               ("st_compiler_list != NULL(%p)", st_compiler_list));
    set_path_to_compiler(compiler, path);
    compiler->input_mode = FILE_INPUT_MODE;

    yyin = fp;
    list = MEM_malloc(sizeof(ISandBox_ExecutableList));
    list->list = NULL;
   
    exe = do_compile(compiler, list, NULL, ISandBox_FALSE);
    exe->path = MEM_strdup(path);
    list->top_level = exe;

    /* ISandBox_disassemble(exe);*/
    dispose_compiler_list();
    Ivyc_reset_string_literal_buffer();

    return list;
}

PackageName *
create_one_package_name(Ivyc_Compiler *compiler,
                        char *str, int start_idx, int to_idx)
{
    PackageName *pn;
    int i;

    MEM_Storage storage = compiler->compile_storage;
    pn = MEM_storage_malloc(storage, sizeof(PackageName));
    pn->name = MEM_storage_malloc(storage, to_idx - start_idx + 1);

    for (i = 0; i < to_idx - start_idx; i++) {
        pn->name[i] = str[start_idx + i];
    }
    pn->name[i] = '\0';
    pn->next = NULL;

    return pn;
}

static PackageName *
string_to_package_name(Ivyc_Compiler *compiler, char *str)
{
    int start_idx;
    int i;
    PackageName *pn;
    PackageName *top;
    PackageName *tail = NULL;

    for (start_idx = i = 0; str[i] != '\0'; i++) {
        if (str[i] == '.') {
            pn = create_one_package_name(compiler, str, start_idx, i);
            start_idx = i + 1;
            if (tail) {
                tail->next = pn;
            } else {
                top = pn;
            }
            tail = pn;
        }
    }
    pn = create_one_package_name(compiler, str, start_idx, i);
    if (tail) {
        tail->next = pn;
    } else {
        top = pn;
    }

    return top;
}

SearchFileStatus
get_dynamic_load_input(char *package_name, char *found_path,
                       char *search_file, SourceInput *source_input)
{
    SearchFileStatus status;
    char *search_path;
    FILE *fp;

    if (search_buitin_source(package_name, IVY_SOURCE, source_input)) {
        found_path[0] = '\0';
        return SEARCH_FILE_SUCCESS;
    }

    search_path = getenv("IVY_LOAD_SEARCH_PATH");
    if (search_path == NULL) {
        search_path = Ivory_get_current_path();
    }
    make_search_path_impl(package_name, search_file);
    status = ISandBox_search_file(search_path, search_file, found_path, &fp);

    if (status != SEARCH_FILE_SUCCESS) {
        return status;
    }
    source_input->input_mode = FILE_INPUT_MODE;
    source_input->u.file.fp = fp;

    return SEARCH_FILE_SUCCESS;
}

SearchFileStatus
Ivyc_dynamic_compile(Ivyc_Compiler *compiler, char *package_name,
                    ISandBox_ExecutableList *list, ISandBox_ExecutableItem **add_top,
                    char *search_file)
{
    SearchFileStatus status;
    extern FILE *yyin;
    ISandBox_ExecutableItem *tail;
    ISandBox_Executable *exe;
    SourceInput source_input;
    char found_path[FILENAME_MAX];
        
    status = get_dynamic_load_input(package_name, found_path,
                                    search_file, &source_input);
    if (status != SEARCH_FILE_SUCCESS) {
        return status;
    }
    DBG_assert(st_compiler_list == NULL,
               ("st_compiler_list != NULL(%p)", st_compiler_list));

    for (tail = list->list; tail->next; tail = tail->next)
        ;

    compiler->package_name = string_to_package_name(compiler, package_name);
    set_path_to_compiler(compiler, found_path);

    compiler->input_mode = source_input.input_mode;
    if (source_input.input_mode == FILE_INPUT_MODE) {
        yyin = source_input.u.file.fp;
    } else {
        Ivyc_set_source_string(source_input.u.string.lines);
    }
    exe = do_compile(compiler, list, found_path, ISandBox_FALSE);

    dispose_compiler_list();
    Ivyc_reset_string_literal_buffer();

    *add_top = tail->next;

    return SEARCH_FILE_SUCCESS;
}

static CompilerList *
traversal_compiler(CompilerList *list, Ivyc_Compiler *compiler)
{
    CompilerList *list_pos;
    CompilerList *req_pos;

    for (list_pos = list; list_pos; list_pos = list_pos->next) {
        if (list_pos->compiler == compiler)
            break;
    }
    if (list_pos == NULL) {
        list = add_compiler_to_list(list, compiler);
    }
    for (req_pos = compiler->usingd_list; req_pos; req_pos = req_pos->next) {
        list = traversal_compiler(list, req_pos->compiler);
    }

    return list;
}

void
Ivyc_dispose_compiler(Ivyc_Compiler *compiler)
{
    CompilerList *list = NULL;
    CompilerList *pos;
    FunctionDefinition *fd_pos;
	ConstantDefinition *cd_pos;
    CompilerList *temp;

    list = traversal_compiler(list, compiler);

    for (pos = list; pos; ) {
        for (fd_pos = pos->compiler->function_list; fd_pos;
             fd_pos = fd_pos->next) {
            MEM_free(fd_pos->local_variable);
        }
        for (cd_pos = pos->compiler->constant_definition_list; cd_pos;
             cd_pos = cd_pos->next) {
			if (cd_pos->initializer->type->basic_type == ISandBox_STRING_TYPE
				&& cd_pos->initializer->kind != CAST_EXPRESSION
				&& cd_pos->initializer->kind != FORCE_CAST_EXPRESSION) {
            	MEM_free(cd_pos->initializer->u.string_value);
			}
        }
        while (pos->compiler->usingd_list) {
            temp = pos->compiler->usingd_list;
            pos->compiler->usingd_list = temp->next;
            MEM_free(temp);
        }
        MEM_dispose_storage(pos->compiler->compile_storage);
        temp = pos->next;

        MEM_free(pos);
        pos = temp;
    }
}
