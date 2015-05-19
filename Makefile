CC	=	gcc
SRC = src/project.c src/client.c src/server.c
VPATH = src

.PHONY: clean


all: 
	@echo "This is the 2014/2015 project of os lab"
	@echo "possible target:"
	@echo "\tbin"
	@echo "\tassets"
	@echo "\ttest"
	@echo "\tclean"


bin: project.c client.c server.c
	@echo "compiling inside the bin directory"
	mkdir -p bin
	$(CC) src/project.c -o bin/project.out


assets:
	@echo "fileing"
	mkdir -p assets
	rm assets/*
	touch assets/1 assets/2


test: bin assets
	@echo "testing"


clean: 
	@echo "clean"
	rm -rf assets bin
