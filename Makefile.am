bin_PROGRAMS = chpp

chpp_SOURCES = avl.c avl.h bcoutput.c bcoutput.h bytecode.c bytecode.h precompile.c precompile.h byteorder.h types.h chash.c chash.h jump.c jump.h commands.c commands.h depends.c depends.h dynstring.c dynstring.h environment.c environment.h error.c error.h filler.c filler.h getopt.c getopt.h getopt1.c input.c input.h internals.c internals.h list.c list.h main.c main.h memory.c memory.h output.c output.h parser.c parser.h recorder.c recorder.h regex.c regex.h value.c value.h arith.h arithparse.y arithscan.l

INCLUDES = -DCHPP_INCLUDEDIR=\"@chppincludedir@\"
chpp_LDADD = builtins/libbuiltins.a builtins/database/libdatabase.a gc-7.2alpha6/.libs/libgc.a @MYSQL_LIBS@ @MSQL_LIBS@ @LIBOBJS@ -lm

SUBDIRS = builtins doc include

AM_YFLAGS = -d

gc-7.2alpha6/.libs/libgc.a :
	( cd gc-7.2alpha6 ; ./configure --disable-threads ; make )

EXTRA_DIST = arithparse.h configure chpp.spec build.sh debian/changelog debian/compat debian/control debian/copyright debian/dirs debian/docs debian/files debian/rules

dist-hook :
	( cd gc-7.2alpha6 ; ./configure --disable-threads ; make distclean )
	cp -dR gc-7.2alpha6 $(distdir)
	rm -rf `find $(distdir)/gc -name .svn`
	mkdir $(distdir)/test
	cp -dR test/regress $(distdir)/test
	cp -dR test/tautogen $(distdir)/test
	cp -dR test/website $(distdir)/test
	rm -rf `find $(distdir)/test -name .svn`
