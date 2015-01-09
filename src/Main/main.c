#include <stdio.h>
#include <locale.h>
#include "Ivyc.h"
#include "share.h"
#include "ISandBox.h"
#include "MEM.h"

int
main(int argc, char **argv)
{
    Ivyc_Compiler *compiler;
    FILE *fp;
    ISandBox_ExecutableList *list;
    ISandBox_VirtualMachine *ISandBox;

    /*
    argc = 2;
    argv[1] = "/home/neo/桌面/OverStack/Ivory_Next/Main/test/System_tests/test.ivy";
    */

    if (argc < 2) {
        fprintf(stderr, "Ivory interpreter\nusage:%s filename arg1, arg2, ...\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "%s not found.\n", argv[1]);
        exit(1);
    }
	Ivory_set_current_path(ISandBox_get_folder_by_path(argv[1]));

    setlocale(LC_CTYPE, "");
    compiler = Ivyc_create_compiler();
    list = Ivyc_compile(compiler, fp, argv[1]);
    ISandBox = ISandBox_create_virtual_machine();
    ISandBox_set_executable(ISandBox, list);
    Ivyc_dispose_compiler(compiler);

    ISandBox_execute(ISandBox);
    ISandBox_dispose_virtual_machine(ISandBox);
    ISandBox_dispose_executable_list(list);

    MEM_check_all_blocks();
    MEM_dump_blocks(stdout);

    return 0;
}
