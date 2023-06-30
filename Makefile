all: build

.PHONY: build client server

CC = gcc
DATE := $(shell date "+%Y-%m-%d")
COMPILE_FLAGS = -Og -g -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
# COMPILE_FLAGS = -Ofast -ggdb3 -march=native -mtune=native -Wall -D_FORTIFY_SOURCE=2 -fmodulo-sched
INCLUDE_FLAGS = 
LIBRARY_FLAGS = -lncursesw -lpanelw
 DEFINE_FLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600
PWD := $(shell pwd)

build:
	$(CC) $(COMPILE_FLAGS) $(DEFINE_FLAGS) $(LIBRARY_FLAGS) -o armee armee.c

install: build

STRING = "Der Alte würfelt nicht"
STRING = "Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht Der Alte würfelt nicht"
run: build
	./armee $(STRING) "/tmp/mata"

debug: build
	$(CC) $(COMPILE_FLAGS) -DNO_INITSCR $(DEFINE_FLAGS) $(LIBRARY_FLAGS) -o armee armee.c
	gdb -q --args ./armee $(STRING) "/tmp/mata"

