all: build

.PHONY: all build

CC       = gcc
CCFLAGS  = -ggdb3 -Og \
					 -Wall -march=native -mtune=native -fmodulo-sched \
					 -fstack-clash-protection -pthread \
					 -pipe -fkeep-inline-functions \
					 -pthread -D_FORTIFY_SOURCE=2

SRC_DIR  = .
OBJ_DIR  = .obj
MOD_DIR  = .mod

C_SRC  = $(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)
C_OBJ  = $(C_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MOD    = $(wildcard $(MOD_DIR)/*.mod)

$(OBJ_DIR):
	mkdir -p $@

$(C_OBJ): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(CCFLAGS) $< -o $@

build: $(C_OBJ)

clean:
	rm $(OBJ_DIR)/*.o
