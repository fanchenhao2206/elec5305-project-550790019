
TEXFILES := $(wildcard *.tex)
IMAGES := $(wildcard images/*.png images/*.jpg images/*.jpeg)
BIBFILE := references.bib

.PHONY: all open clean

all: main.pdf
	open main.pdf

main.pdf: $(TEXFILES) $(IMAGES) $(BIBFILE)
	pdflatex -shell-escape main.tex
	bibtex main || true
	pdflatex -shell-escape main.tex
	pdflatex -shell-escape main.tex

open:
	open main.pdf

clean:
	rm main.pdf main.aux main.bbl main.blg main.log main.out main.toc
