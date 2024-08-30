#ifndef SIMULATE_h
#define SIMULATE_h

#include "types.h"
#include "architecture.h"

void 
simulate_mov(byte *registers, operand destination, operand source);

void 
simulate_arithmetic ( byte* reigsters, 
                      operand destination,
                      operand source, 
                      ARITHMETIC_TYPES type );

void 
simulate_jump ( byte* registers,
                bool jump_if_zero,
                operand instruction_offset );

word
read_value_from_operand (byte *registers, operand reg);

void 
write_value_to_operand (byte *registers, operand reg, word value);

#endif
