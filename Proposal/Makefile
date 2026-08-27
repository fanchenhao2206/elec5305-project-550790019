TEXFILES := $(wildcard *.tex)

main.pdf: $(TEXFILES) references.bib 
	pdflatex main.tex
	bibtex main
	pdflatex main.tex
	pdflatex main.tex
	open main.pdf

