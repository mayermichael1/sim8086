#  use alias bake to generate compile_commands.json
CXX := g++
CXXFLAGS := -Wall -Wextra -Werror
BUILD_DIR := ./build
LDFLAGS :=
LDLIBS := 
CPPFLAGS :=  -Iinclude/

OBJECTS := $(BUILD_DIR)/main.o
# add objects here like this:
# OBJECTS += $(BUILD_DIR)/object.o

CXXFLAGS += -O0 -g
#CXXFLAGS += -O2

$(BUILD_DIR)/sim8086: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $(BUILD_DIR)/sim8086 $(LDFLAGS) $(LDLIBS)

# program entry point
$(BUILD_DIR)/main.o: main.cpp include/types.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c main.cpp -o $(BUILD_DIR)/main.o

# Build all Objects here
# $(BUILD_DIR)/object.o: src/object.cpp include/object.h
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/object.cpp -o $(BUILD_DIR)/object.o

.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)/*
	rm ./compile_commands.json

