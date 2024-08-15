# 指定编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -O2

# 源文件目录
SRC_DIR = src

# 目标文件目录
OBJ_DIR = obj

# 可执行文件目录
BIN_DIR = bin

# 获取所有的源文件
SRCS = $(wildcard $(SRC_DIR)/*.c)

# 生成所有的目标文件
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# 可执行文件的名称
TARGET = $(BIN_DIR)/my_program

# 默认目标
all: $(TARGET)

# 规则，生成目标文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 规则，生成可执行文件
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# 清理目标
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
