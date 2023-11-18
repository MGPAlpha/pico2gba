SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
LIB_DIR := lib
RES_DIR := res

BIN_RES_DIR := $(BIN_DIR)/res

CORE_O := $(OBJ_DIR)/core.o
CLI_O := $(OBJ_DIR)/cli.o

LIB := 
RES := 
EXE := $(BIN_DIR)/pico2gba
CLI := $(BIN_DIR)/pico2gbacli
CORE := $(BIN_DIR)/libpico2gba.a
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall
LDFLAGS  := -Llib
LDLIBS   := -lm -ljson-c

.PHONY: all clean

all: $(CLI)

$(CORE): $(CORE_O) $(LIB) | $(BIN_DIR)
		$(AR) -rc $@ $?

$(CLI): $(CLI_O) $(CORE) $(RES) $(LIB) | $(BIN_DIR)
		$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
		$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(RES_DIR) $(BIN_RES_DIR):
		mkdir -p $@
		
clean:
		@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

deep-clean: clean
		@cd $(LIB_DIR)/luaARM && make clean

# $(LIBLUA_A) : | $(RES_DIR)
# 		cd $(LIB_DIR)/luaARM && make liblua.a
# 		cp $(LIB_DIR)/luaARM/liblua.a $(RES_DIR)/liblua.a

-include $(OBJ:.o=.d)
