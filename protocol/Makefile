# default variablen in make:
# $<    die erste Abhängigkeit
# $@    Name des targets
# $+    eine Liste aller Abhängigkeiten
# $^    eine Liste aller Abhängigkeiten, wobei allerdings doppelt vorkommende Abhängigkeiten eliminiert wurden.

DOCUMENT = protocol
RMCMND   = rm -f *.pdf.done *.done *.aux *.log *.out *.toc

all: en

clean:
	@echo "make clean leaves generated pdf files untouched. To delete them also, use: \n\tmake distclean\n"
	@for filename in $(DOKUMENT) $(DOCUMENT); do \
		rm -fv $${filename}.tex~; \
		rm -fv $${filename}.aux; \
		rm -fv $${filename}.log; \
		rm -fv $${filename}.out; \
		rm -fv $${filename}.toc; \
		rm -fv $${filename}.pdf.done; \
		rm -fv $${filename}.pdflatex.output; \
	done

distclean: clean
	@rm -fv $(DOCUMENT).pdf

veryclean:
	$(RMCMND)

en: $(DOCUMENT).tex
	pdflatex $(DOCUMENT).tex
	while [ ! -f "$(DOCUMENT).done" ]; do \
		grep -q "Rerun to get cross-references right." $(shell basename $(DOCUMENT).pdf).log || ( touch $(DOCUMENT).done && break ); \
		pdflatex $(DOCUMENT).tex; \
	done
	$(RMCMND)
	@echo "solution document created"
	@echo "------------------------"

# end
