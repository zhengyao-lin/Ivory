static ISandBox_Value
nv_fabs_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = fabs(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_pow_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = pow(args[0].double_value,
                           args[1].double_value);

    return ret;
}

static ISandBox_Value
nv_fmod_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = fmod(args[0].double_value,
                            args[1].double_value);

    return ret;
}

static ISandBox_Value
nv_ceil_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = ceil(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_floor_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = floor(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_sqrt_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = sqrt(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_exp_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = exp(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_log10_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = log10(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_log_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = log(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_sin_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = sin(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_cos_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = cos(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_tan_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
            int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = tan(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_asin_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = asin(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_acos_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = acos(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_atan_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = atan(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_atan2_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = atan2(args[0].double_value,
                             args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_sinh_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = sinh(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_cosh_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = cosh(args[0].double_value);

    return ret;
}

static ISandBox_Value
nv_tanh_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
             int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    ret.double_value = tanh(args[0].double_value);

    return ret;
}

void
ISandBox_add_native_functions_math(ISandBox_VirtualMachine *ISandBox)
{
    /* Math.ivh */
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "fabs", nv_fabs_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "pow", nv_pow_proc, 2,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "fmod", nv_fmod_proc, 2,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "ceil", nv_ceil_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "floor", nv_floor_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "sqrt", nv_sqrt_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "exp", nv_exp_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "log10", nv_log10_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "log", nv_log_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "sin", nv_sin_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "cos", nv_cos_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "tan", nv_tan_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "asin", nv_asin_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "acos", nv_acos_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "atan", nv_atan_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "atan2", nv_atan2_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "sinh", nv_sinh_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "cosh", nv_cosh_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Math", "tanh", nv_tanh_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
