TARGET = ../bin/Ivory
OBJS = \
  ../ICompiler/compiler.o\
  ../ICompiler/builtin/builtin.o\
  ../ISandBox/ISandBox.o\
  ../Share/share.o\
  ../Memory/mem.o\
  ../Debug/dbg.o\
  ./main.o

CFLAGS = -c -g -Wall -ansi -Wswitch-enum -DDEBUG

INCLUDES = \
  -I/usr/local/include\
  -I../Includes

$(TARGET):$(OBJS)
	rm -f ../ICompiler/builtin/builtin_conv;
	cd ../ICompiler/builtin; $(MAKE) builtin_conv;
	cd ../ICompiler; $(MAKE);
	cd ../ISandBox; $(MAKE);
	cd ../Share; $(MAKE);
	cd ../Memory; $(MAKE);
	cd ../Debug; $(MAKE);
	cd ../ICompiler/builtin; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm

clean:
	rm -f *.o *~
all-clean:
	rm -f $(TARGET)
	rm -f ../ICompiler/builtin/builtin_conv
	find ../ -name "*.o"  | xargs rm -f
	cd ../ICompiler; $(MAKE) clean;
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
../Memory/mem.o:
	cd ../Memory; $(MAKE);
../Debug/dbg.o:
	cd ../Debug; $(MAKE);
../Share/share.o:
	cd ../Share; $(MAKE);
../ICompiler/compiler.o:
	cd ../ICompiler; $(MAKE);
../ISandBox/ISandBox.o:
	cd ../ISandBox; $(MAKE);
############################################################
main.o: main.c ../Includes/Ivyc.h ../Includes/ISandBox_code.h ../Includes/ISandBox.h \
  ../Includes/ISandBox.h ../Includes/MEM.h
