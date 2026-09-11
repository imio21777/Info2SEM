# GNU/Linux Makefile for the cloud paper

default: paper

paper:
	pdflatex main
	bibtex main
	pdflatex main
	pdflatex main
	@$(MAKE) clean
	@echo "Building paper done. ;)"

clean:
	@rm -rf *.aux *.bbl *.blg *.log *.out *~


help:
	@echo "The 'make' will make paper"
	@echo "Please run 'make paper' to make paper."

