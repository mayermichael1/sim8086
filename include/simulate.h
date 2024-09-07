#ifndef SIMULATE_h
#define SIMULATE_h

#include "types.h"
#include "architecture.h"

void 
simulate_mov(cpu_architecture *cpu, operand destination, operand source);

void 
simulate_arithmetic ( cpu_architecture *cpu, 
                      operand destination,
                      operand source, 
                      ARITHMETIC_TYPES type );

void 
simulate_jump ( cpu_architecture *cpu,
                bool jump_if_zero,
                operand instruction_offset );

word
read_value_from_operand (cpu_architecture *cpu, operand reg);

void 
write_value_to_operand (cpu_architecture *cpu, operand reg, word value);

bool
read_flag (byte* registers, FLAG flag);

#endif
