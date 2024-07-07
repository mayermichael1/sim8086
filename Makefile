#  use alias bake to generate compile_commands.json
CXX := g++
CXXFLAGS := -Wall -Wextra -Werror
BUILD_DIR := ./build
LDFLAGS :=
LDLIBS := 
CPPFLAGS :=  -Iinclude/

OBJECTS := $(BUILD_DIR)/main.o
# add objects here like this:
OBJECTS += $(BUILD_DIR)/binary.o

CXXFLAGS += -O0 -g
#CXXFLAGS += -O2

$(BUILD_DIR)/sim8086: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $(BUILD_DIR)/sim8086 $(LDFLAGS) $(LDLIBS)

# program entry point
$(BUILD_DIR)/main.o: main.cpp include/types.h include/binary.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c main.cpp -o $(BUILD_DIR)/main.o 

$(BUILD_DIR)/binary.o: src/binary.cpp include/types.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/binary.cpp -o $(BUILD_DIR)/binary.o

.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)/*
	rm ./compile_commands.json

