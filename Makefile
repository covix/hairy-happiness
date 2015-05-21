CC	=	gcc -std=gnu99 -pthread
SRC = src/project.c src/client.c src/server.c
VPATH = src

.PHONY: clean bin


all: 
	@echo "This is the 2014/2015 project of os lab"
	@echo "possible target:"
	@echo "\tbin"
	@echo "\tassets"
	@echo "\ttest"
	@echo "\tclean"


bin: project.c client.c server.c common.c
	@echo "compiling inside the bin directory"
	@rm -rf bin
	@mkdir -p bin
	$(CC) src/project.c src/server.c src/client.c src/common.c -o bin/project
	@echo "executable compilated"
	@echo "use './bin/project' to play"


assets:
	@echo "creating assets"
	@mkdir -p assets
	@rm -rf assets/*
	@echo "alpha\n100\n52\n37\n46\n51\n131\n123\n87\n42" > assets/1
	@echo "beta\n98\n44\n16\n198\n156\n87\n45\n54\n13" > assets/2
	@echo "finished"

test: bin assets
	@echo "executing some test"
	bin/project.out server --max 10 --win 10 &
	bin/project.out client < assets/2 &
	bin/project.out client < assets/1 &
	@echo "all went good"


clean: 
	@echo "removing all the generated files"
	rm -rf assets bin
	@echo "cleaned"