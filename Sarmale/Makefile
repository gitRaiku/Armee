all: build

.PHONY: build run debug clean remove tar vdbg

# Directories
TARGET   = sarmale
SRC_DIR  = .
BIN_DIR  = .
LIB_DIR  = ../Libs
OBJ_DIR  = .obj
MOD_DIR  = .mod

# Options
CC       = gcc
CCFLAGS  = -ggdb3 -Og \
					 -Wall -Wpedantic -march=native -mtune=native -fmodulo-sched \
					 -fstack-clash-protection -pthread \
					 -pipe -fkeep-inline-functions \
					 -pthread -D_FORTIFY_SOURCE=2

LINKER   = $(CC)

DEBUGGER = gdb -q
VALGRIND = valgrind --leak-check=summary --vgdb=yes --vgdb-error=1

DATE := $(shell date "+%Y-%m-%d")

LFLAGS = 
C_SRC  = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
C_OBJ  = $(C_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
L_OBJ  = $(wildcard $(LIB_DIR)/$(OBJ_DIR)/*.o)
MOD    = $(wildcard $(MOD_DIR)/*.mod)

build: $(BIN_DIR) $(BIN_DIR)/$(TARGET)

$(BIN_DIR):
	mkdir -p $@

$(C_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) -c $(CCFLAGS) $< -o $@

$(MOD_DIR):
	mkdir -p $@

$(BIN_DIR)/$(TARGET): $(C_OBJ)
	$(LINKER) $(L_OBJ) $(C_OBJ) $(LFLAGS) -o $@

run: build
	./$(BIN_DIR)/$(TARGET)

debug: build
	$(DEBUGGER) $(BIN_DIR)/$(TARGET)

clean:
	rm -f $(C_OBJ)
	rm -f $(MOD)

remove: clean
	rm -f $(BIN_DIR)/$(TARGET)

tar:
	tar -czvf armee-$(DATE).tar.gz Makefile $(SRC_DIR)/* 

