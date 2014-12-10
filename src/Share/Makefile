TARGET = share.o
CFLAGS = -c -g -I../Includes -DDEBUG -Wall -ansi -pedantic -Wswitch-enum
OBJS = dispose.o \
  opcode.o \
  disassemble.o \
  wchar.o \
  util.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -I../Includes $*.c
disassemble.o: disassemble.c ../Includes/DBG.h ../Includes/share.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h
dispose.o: dispose.c ../Includes/MEM.h ../Includes/DBG.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h
opcode.o: opcode.c ../Includes/share.h ../Includes/ISandBox_code.h \
  ../Includes/ISandBox.h
util.o: util.c ../Includes/DBG.h ../Includes/MEM.h ../Includes/share.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h
wchar.o: wchar.c ../Includes/DBG.h ../Includes/MEM.h ../Includes/ISandBox.h
