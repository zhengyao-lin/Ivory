#ifndef PUBLIC_Ivyc_H_INCLUDED
#define PUBLIC_Ivyc_H_INCLUDED
#include <stdio.h>
#include "ISandBox_code.h"

typedef struct Ivyc_Compiler_tag Ivyc_Compiler;

Ivyc_Compiler *Ivyc_create_compiler(void);
ISandBox_ExecutableList *Ivyc_compile(Ivyc_Compiler *compiler, FILE *fp, char *path);
void Ivyc_dispose_compiler(Ivyc_Compiler *compiler);

#endif /* PUBLIC_Ivyc_H_INCLUDED */
