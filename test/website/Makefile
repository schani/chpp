CHFILES=header.chml footer.chml choicebar.chml choicestrip.chml

all : news.html tips.html tricks.html

%.html : %.csml
	../../chpp -o $@ $<

news.html tricks.html tips.html : $(CHFILES)

clean :
	rm -f news.html tips.html tricks.html
