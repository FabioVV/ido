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
	@echo "=> linking executable"
	@$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "=> compiling object files..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	@echo "=> creating obj folder"
	@mkdir -p $(OBJ_DIR)

# Clean build files
clean:
	@echo "=> cleaning files"
	@rm -rf $(OBJ_DIR) $(TARGET)

# Run the program
run: $(TARGET)
	@echo "=> running IDO"
	@./$(TARGET)

# Debug the program
debug: $(TARGET)
	@echo "=> running IDO on debug mode"
	@gdb ./$(TARGET)
