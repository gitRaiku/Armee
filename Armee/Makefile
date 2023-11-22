all: build

.PHONY: build run debug clean remove tar vdbg

# Directories
TARGET   = armee
SRC_DIR  = .
BIN_DIR  = .
LIB_DIR  = ../Libs
OBJ_DIR  = .obj

# Options
CC       = gcc
CCFLAGS  = -ggdb3 -Og \
					 -Wall -march=native -mtune=native -fmodulo-sched \
					 -fstack-clash-protection -pthread \
					 -pipe -fkeep-inline-functions \
					 -pthread -D_FORTIFY_SOURCE=2

LINKER   = $(CC)

DEBUGGER = gdb -q
VALGRIND = valgrind --leak-check=summary --vgdb=yes --vgdb-error=1

DATE := $(shell date "+%Y-%m-%d")

LFLAGS = -lncursesw -lpanelw
C_SRC  = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
C_OBJ  = $(C_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
L_OBJ  = $(wildcard $(LIB_DIR)/$(OBJ_DIR)/*.o)

libs:
	make -C ../Libs

build: libs $(BIN_DIR) $(BIN_DIR)/$(TARGET)

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(C_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) -c $(CCFLAGS) $< -o $@

$(MOD_DIR):
	mkdir -p $@

$(BIN_DIR)/$(TARGET): $(C_OBJ)
	$(LINKER) $(L_OBJ) $(C_OBJ) $(LFLAGS) -o $@

install: $(BIN_DIR)/$(TARGET)
	cp -f $(BIN_DIR)/$(TARGET) /usr/local/bin/$(TARGET)

# STRING = "    über Aü krallen Bär Vogel überspitzt kiöppen der Alte würfelt Würfel nicht bo "

# STRING = "derr"
STRING = "Worten es begann ein Wettrüsten zwischen"
run: build
	@./$(BIN_DIR)/$(TARGET) $(STRING) ""

debug: build
	$(DEBUGGER) --args $(BIN_DIR)/$(TARGET) $(STRING) ""

clean:
	rm -rf $(OBJ_DIR)

remove: clean
	rm -f $(BIN_DIR)/$(TARGET)

tar:
	tar -czvf armee-$(DATE).tar.gz Makefile $(SRC_DIR)/* 

