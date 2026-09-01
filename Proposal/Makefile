# TEXFILES := $(wildcard *.tex)
TEXFILES := $(wildcard *) 

main.pdf: $(TEXFILES) references.bib 
	pdflatex -shell-escape main.tex
	bibtex main
	pdflatex -shell-escape main.tex
	pdflatex -shell-escape main.tex
	open main.pdf

open:
	open main.pdf

clean:
	rm main.pdf