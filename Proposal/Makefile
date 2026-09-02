TEXFILES := $(wildcard *.tex)
IMAGES := $(wildcard images/*.png images/*.jpg)

main.pdf: $(TEXFILES) $(IMAGES) references.bib 
	pdflatex -shell-escape main.tex
	bibtex main
	pdflatex -shell-escape main.tex
	pdflatex -shell-escape main.tex
	open main.pdf

open:
	open main.pdf

clean:
	rm main.pdf
