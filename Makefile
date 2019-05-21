EXECUTABLE :=  entropic_testing

BUILD_DIR := ./build
SRC_DIR := ./src

SRC :=  $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ :=  $(patsubst ./src/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEP :=  $(patsubst ./src/%.c,$(BUILD_DIR)/%.d,$(SRC))

CFLAGS := -std=c11 -Wall -Wextra -Og
C := gcc

.PHONY : clean

all: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(C) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(C) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@


print-%  : 
	@echo $* = $($*)

clean:
	@rm -rf $(BUILD_DIR) $(EXECUTABLE)

-include $(DEP)