static ISandBox_Value
nv_gets_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    char buf[LINE_BUF_SIZE];
    char *mb_buf = NULL;
    int ret_len = 0;
    ISandBox_Char *wc_str;

    while (fgets(buf, LINE_BUF_SIZE, stdin)) {
        int new_len;
        new_len = ret_len + strlen(buf);
        mb_buf = MEM_realloc(mb_buf, new_len + 1);
        if (ret_len == 0) {
            strcpy(mb_buf, buf);
        } else {
            strcat(mb_buf, buf);
        }
        ret_len = new_len;
        if (mb_buf[ret_len-1] == '\n')
	    {
	        mb_buf[ret_len-1] = '\0';
            break;
	    }
    }
    if (ret_len > 0) {
        wc_str = ISandBox_mbstowcs_s(mb_buf);
        ret.object = ISandBox_create_ISandBox_string(ISandBox, context, wc_str);
    } else {
        ret.object = ISandBox_null_object_ref;
    }
    MEM_free(mb_buf);

    return ret;
}

static ISandBox_Value
nv_puts_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
    ISandBox_Value ret;
    ISandBox_Char *str;

    ret.int_value = 0;

    if (args[0].object.data == NULL) {
        str = NULL_STRING;
    } else {
        str = args[0].object.data->u.string.string;
    }
    ISandBox_print_wcs(stdout, str);
    /*fflush(stdout);*/

    return ret;
}

static ISandBox_Value
nv_putc_proc(ISandBox_VirtualMachine *ISandBox, ISandBox_Context *context,
              int arg_count, ISandBox_Value *args)
{
	ISandBox_Value ret;
    ret.int_value = 0;

	putc(args[0].int_value, stdout);

    return ret;
}

void
ISandBox_add_native_functions_io(ISandBox_VirtualMachine *ISandBox)
{
    /* IO.ivh */
    ISandBox_add_native_function(ISandBox, "Ivory.IO", "gets", nv_gets_proc, 0,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.IO", "puts", nv_puts_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
    ISandBox_add_native_function(ISandBox, "Ivory.IO", "putc", nv_putc_proc, 1,
                            ISandBox_FALSE, ISandBox_FALSE);
}
