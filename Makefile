# Define directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

# Define compiler and flags
CC := gcc
CFLAGS := -Wall -g

# Define the target executable
TARGET := $(BUILD_DIR)/calculator

# Find all .c files in src directory
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o files in build/obj directory
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BUILD_DIR)

# Rebuild everything from scratch
rebuild: clean all

.PHONY: all clean rebuild
