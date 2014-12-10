TARGET = ISandBox.o
CFLAGS = -c -g -DDEBUG -Wall -ansi -pedantic -Wswitch-enum
OBJS = execute.o \
  load.o \
  heap.o \
  native.o \
  nativeif.o \
  wchar.o \
  util.o \
  error.o \
  error_message.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -I../Includes $*.c
error.o: error.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
error_message.o: error_message.c ISandBox_pri.h ../Includes/ISandBox_code.h \
  ../Includes/ISandBox.h ../Includes/ISandBox_dev.h ../Includes/share.h \
  ../Includes/ISandBox_code.h
execute.o: execute.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
heap.o: heap.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
load.o: load.c ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h ../Includes/ISandBox_code.h \
  ../Includes/ISandBox_dev.h ../Includes/share.h
native.o: native.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
nativeif.o: nativeif.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
util.o: util.c ../Includes/MEM.h ../Includes/DBG.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
wchar.o: wchar.c ../Includes/DBG.h ../Includes/MEM.h ISandBox_pri.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_dev.h \
  ../Includes/share.h ../Includes/ISandBox_code.h
