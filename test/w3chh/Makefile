# chpp Makefile for web pages

HTML = test.html

CHPP = chpp

# --- general ----------------------------------------------------------------

all : $(HTML)

# --- chpp dependencies ------------------------------------------------------

%.d : %.csml
	@echo Generating $@ from @<..
	@echo "$(<:.csml=.html) $(<:.csml=.images): "`$(CHPP) -M $< | \
	  awk '{gsub( "^[^ ]+[.]o: ", " " );split( $$0, ar, " " );\
	        i=1;\
		while( ar[i] ) if( length(ar[i++])>1 ) print ar[i-1];}'\
		|egrep -v "^$<$$"` > $@

# --- image stuff ------------------------------------------------------------



# --- includes ---------------------------------------------------------------

include $(HTML:.html=.d)
