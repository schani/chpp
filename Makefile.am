bin_PROGRAMS = chpp

chpp_SOURCES = avl.c avl.h bcoutput.c bcoutput.h bytecode.c bytecode.h chash.c chash.h jump.c jump.h commands.c commands.h depends.c depends.h dynstring.c dynstring.h environment.c environment.h error.c error.h filler.c filler.h getopt.c getopt.h getopt1.c input.c input.h internals.c internals.h list.c list.h main.c main.h memory.c memory.h output.c output.h parser.c parser.h recorder.c recorder.h regex.c regex.h value.c value.h arith.h arithparse.y arithscan.l

INCLUDES = -DCHPP_INCLUDEDIR=\"@chppincludedir@\"
chpp_LDADD = builtins/libbuiltins.a builtins/database/libdatabase.a gc/gc.a @MYSQL_LIBS@ @MSQL_LIBS@ @LIBOBJS@ -lm

SUBDIRS = builtins doc include

YFLAGS = -d

gc/gc.a :
	( cd gc ; make gc.a )

EXTRA_DIST = arithparse.h configure

dist-hook :
	( cd gc ; make clean )
	cp -dR gc $(distdir)
	rm -rf `find $(distdir)/gc -name CVS`
	mkdir $(distdir)/test
	cp -dR test/regress $(distdir)/test
	cp -dR test/tautogen $(distdir)/test
	cp -dR test/website $(distdir)/test
	rm -rf `find $(distdir)/test -name CVS`

clean-gc :
	( cd gc ; make clean )

clean : clean-gc

mostlyclean : clean-gc

distclean : clean-gc
