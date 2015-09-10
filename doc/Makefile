
SUBDIRS = img
PROJ = tracy_manual

BASH := $(shell which bash || which bash)

.PHONY: all
.PHONY: check
.PHONY: clean
.PHONY: doc

all: $(PROJ).pdf

#
# Build list of sources and objects to build
SRCS := $(wildcard *.tex)
$(foreach subdir,$(SUBDIRS), \
	$(eval SRCS := $(SRCS) $(wildcard $(subdir)/*.tex)) \
)

check:
	@aspell --mode tex --check $(SRCS)

$(PROJ).pdf: $(SRCS)
	@pdflatex $(SRCS)
	@perl ~/texmf/scripts/makeglossaries $(PROJ)
	@pdflatex $(SRCS)
	@pdflatex $(SRCS)

clean: $(SUBDIRS)
	@rm -f $(PROJ).{aux,log,out,pdf,toc,glg,glo,gls,ist,xdy} *.log

doc:
	rm -rf doc/*
	doxygen Doxyfile
	
