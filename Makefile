#  use alias bake to generate compile_commands.json
CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -Wno-error=unused-variable
BUILD_DIR := ./build
LDFLAGS :=
LDLIBS := 
CPPFLAGS :=  -Iinclude/

OBJECTS := $(BUILD_DIR)/main.o
# add objects here like this:
OBJECTS += $(BUILD_DIR)/binary.o
OBJECTS += $(BUILD_DIR)/print.o
OBJECTS += $(BUILD_DIR)/simulate.o

CXXFLAGS += -O0 -g
#CXXFLAGS += -O2

$(BUILD_DIR)/sim8086: $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) -o $(BUILD_DIR)/sim8086 $(LDFLAGS) $(LDLIBS)

# program entry point
$(BUILD_DIR)/main.o: main.cpp include/types.h include/binary.h include/register_rm.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c main.cpp -o $(BUILD_DIR)/main.o 

$(BUILD_DIR)/binary.o: src/binary.cpp include/types.h include/binary.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/binary.cpp -o $(BUILD_DIR)/binary.o

$(BUILD_DIR)/print.o: src/print.cpp include/print.h include/types.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/print.cpp -o $(BUILD_DIR)/print.o

$(BUILD_DIR)/simulate.o: src/simulate.cpp include/simulate.h include/types.h include/register_rm.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c src/simulate.cpp -o $(BUILD_DIR)/simulate.o

.PHONY: clean
clean: 
	rm -rf $(BUILD_DIR)/*
	rm ./compile_commands.json

