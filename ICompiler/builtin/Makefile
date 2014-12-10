TARGET = builtin.o
CFLAGS = -g -DDEBUG -Wall -ansi -pedantic -Wswitch-enum

$(TARGET):builtin.c
	$(CC) -c $(CFLAGS) -I../../Includes $*.c

BUILTIN_SCRIPT = \
  ../../using/Ivory/lang.ivh\
  ../../using/Ivory/lang.ivy\
  ../../using/Ivory/Math.ivh\
  ../../using/Ivory/IO.ivh

builtin.c:builtin_conv head.txt tail.txt $(BUILTIN_SCRIPT)
	./builtin_conv

CONV_OBJS =\
  ../../Memory/memory.o\
  ../../Debug/debug.o

builtin_conv:builtin_conv.c ../../Includes/ISandBox.h ../Ivoryc.h
	$(CC) $(CFLAGS) -o $@ -I../../Includes builtin_conv.c $(CONV_OBJS)
############################################################
builtin.o: builtin.c ../Ivoryc.h ../../Includes/MEM.h ../../Includes/Ivyc.h \
  ../../Includes/ISandBox_code.h ../../Includes/ISandBox.h ../../Includes/ISandBox_code.h \
  ../../Includes/share.h ../../Includes/Ivyc.h
builtin_conv.o: builtin_conv.c ../../Includes/ISandBox.h ../../Includes/DBG.h
