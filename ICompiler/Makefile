TARGET = compiler.o
ENCODING_FLAG = -DEUC_SOURCE
CFLAGS = -c -g -Wall -ansi -pedantic -Wswitch-enum -DDEBUG $(ENCODING_FLAG)
OBJS = \
  lex.yy.o\
  y.tab.o\
  interface.o\
  create.o\
  fix_tree.o\
  generate.o\
  string.o\
  wchar.o\
  util.o\
  error.o\
  error_message.o

INCLUDES = \
  -I../Includes

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

clean:
	rm -f *.o lex.yy.c y.tab.c y.tab.h y.output *~
y.tab.h : Ivory.y
	yacc -dv Ivory.y
y.tab.c : Ivory.y
	yacc -dv Ivory.y
lex.yy.c : Ivory.l Ivory.y y.tab.h
	lex Ivory.l
y.tab.o: y.tab.c Ivoryc.h ../Includes/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
lex.yy.o: lex.yy.c Ivoryc.h ../Includes/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
############################################################
create.o: create.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h \
  ../Includes/ISandBox_dev.h
Ivory.tab.o: Ivory.tab.c Ivoryc.h ../Includes/MEM.h ../Includes/Ivyc.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_code.h \
  ../Includes/share.h ../Includes/Ivyc.h
error.o: error.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
error_message.o: error_message.c Ivoryc.h ../Includes/MEM.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
fix_tree.o: fix_tree.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
generate.o: generate.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
interface.o: interface.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
string.o: string.c ../Includes/MEM.h Ivoryc.h ../Includes/Ivyc.h \
  ../Includes/ISandBox_code.h ../Includes/ISandBox.h ../Includes/ISandBox_code.h \
  ../Includes/share.h ../Includes/Ivyc.h
util.o: util.c ../Includes/MEM.h ../Includes/DBG.h Ivoryc.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
wchar.o: wchar.c ../Includes/DBG.h Ivoryc.h ../Includes/MEM.h \
  ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox_code.h ../Includes/share.h ../Includes/Ivyc.h
