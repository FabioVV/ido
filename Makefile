CC = gcc
CFLAGS = -g -Wall

SRC_DIR = src
OBJ_DIR = obj

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = ido
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: $(EXEC)
	./$(EXEC)

debug: $(EXEC)
	gdb ./$(EXEC)
