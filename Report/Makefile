LATEX = xelatex

comma:=,
empty:=
space:= $(empty) $(empty)

ifeq ($(OS),Windows_NT)
	RM = rm 
	FixPath = $(subst $(space),$(comma),$1)
else
	ifeq ($(shell uname), Linux)
		RM = rm -f 
		FixPath = $1
	endif
endif

cleanFilesLin:= *.aux *.lof *.log *.lot *.out *.toc *.bbl *.blg
cleanFilesWin:= *.aux, *.lof, *.log, *.lot, *.out, *.toc, *.bbl, *.blg
cleanCmd= $(RM) $(call FixPath,$(cleanFilesLin))

.PHONY: clean cleanAll noBib updateFonts

all:
	$(LATEX) thesis
	bibtex thesis
	$(LATEX) thesis
	$(LATEX) thesis

noBib:
	$(LATEX) thesis

clean:
	$(RM) $(call FixPath,$(cleanFilesLin))

cleanAll:
	make clean
	rm *.pdf

updateFonts:
	initexmf --admin --mkmaps

print:
	$(info $$OS os [$(OS)])
	$(info $(cleanCmd))