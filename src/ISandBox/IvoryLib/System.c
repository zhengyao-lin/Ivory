static ISandBox_Value
nv_abort_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    
    abort();

    return ret;
}

static ISandBox_Value
nv_exit_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    exit(args[0].int_value);

    return ret;
}

static ISandBox_Value
nv_system_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *cmd;

    cmd = MEM_malloc(ISandBox_wcstombs_len(args[0].object.data->u.string.string) + 1);
    ISandBox_wcstombs_i(args[0].object.data->u.string.string, cmd);
    ret.int_value = system(cmd);
    MEM_free(cmd);

    return ret;
}

static ISandBox_Value
nv_getenv_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char *name;
    ISandBox_Char *wc_str;

    name = MEM_malloc(ISandBox_wcstombs_len(args[0].object.data->u.string.string) + 1);
    ISandBox_wcstombs_i(args[0].object.data->u.string.string, name);
    char *target = getenv(name);

    if (target != NULL) {
        wc_str = ISandBox_mbstowcs_s(target);
        ret.object = ISandBox_create_ISandBox_string(ISandBox, context, wc_str);
    } else {
        ret.object = ISandBox_null_object_ref;
    }
    MEM_free(name);

    return ret;
}

void
ISandBox_add_native_functions_system(ISandBox_VirtualMachine *ISandBox)
{
    /* System.ivh */
    ISandBox_add_native_function(ISandBox, "Ivory.System", "exit", nv_exit_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.System", "abort", nv_abort_proc, 0,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.System", "system", nv_system_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.System", "getenv", nv_getenv_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
