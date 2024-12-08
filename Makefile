CC = gcc
CFLAGS = -g -Wall

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Source and Object Files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Target Executable
TARGET = ido

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug the program
debug: $(TARGET)
	gdb ./$(TARGET)
