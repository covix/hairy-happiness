CC	=	gcc

.PHONY: clean


all: 
	@echo "This is the 2014/2015 project of lab os"
	@echo "possible target:"
	@echo "\tbin"
	@echo "\tassets"
	@echo "\ttest"
	@echo "\tbin"


bin: client.c server.c
	@echo "compiling inside the bin directory"
	mkdir -p bin
	$(CC) client.c -o bin/client.out
	$(CC) server.c -o bin/server.out


assets:
	@echo "fileing"
	if [ ! -d "assets" ]; then
		mkdir assets
	fi
	touch assets/1 assets/2


test: bin assets
	@echo "testing"


clean: 
	@echo "clean"
