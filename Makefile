# chpp Makefile for web pages

HTML = Main.html News.html Authors.html Manual.html Wizard.html Download.html Links.html

# --- settings ---------------------------------------------------------------

CHPP = chpp

# --- general ----------------------------------------------------------------

all : $(HTML)

clean :
	rm -f *.d *.jpgd *.gifd *.pngd *.html core *~

# --- chpp dependencies ------------------------------------------------------

%.html : %.csml
	chpp $< > $@

%.d : %.csml
	chpp -M -o $(<:.csml=.html) $< > $@

# --- includes ---------------------------------------------------------------

-include $(HTML:.html=.d)
