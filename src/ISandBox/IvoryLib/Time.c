#ifndef IVY_WINDOWS
	#include <unistd.h>
#endif

static ISandBox_Value
nv_sleep_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

#ifndef IVY_WINDOWS
	usleep(args[0].int_value * 1000); 
#endif

    return ret;
}

static ISandBox_Value
nv_usleep_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

#ifndef IVY_WINDOWS
	usleep(args[0].int_value); 
#endif

    return ret;
}

void
ISandBox_add_native_functions_time(ISandBox_VirtualMachine *ISandBox)
{
    /* Time.ivh */
    ISandBox_add_native_function(ISandBox, "Ivory.Time", "sleep", nv_sleep_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.Time", "usleep", nv_usleep_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
