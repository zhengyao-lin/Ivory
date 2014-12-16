ISandBox_Value
nv_typeof_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
                    int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;

    /*ret.object = ISandBox_create_object(ISandBox, context, args[0].object);*/
	/*if (args[0].object.data->object_type == (int)ISandBox_INT_TYPE) {
		printf("int!\n");
	} else if (args[0].object.data->object_type == (int)ISandBox_DOUBLE_TYPE) {
		printf("double!\n");
	} else if (args[0].object.data->object_type == (int)ISandBox_STRING_TYPE) {
		printf("string!\n");
	}*/
	ret.int_value = args[0].object.data->object_type;

    return ret;
}

void
ISandBox_add_native_functions_type(ISandBox_VirtualMachine *ISandBox)
{
    /* Math.ivh */
	ISandBox_add_native_function(ISandBox, "Ivory.Type", "__typeof", nv_typeof_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
