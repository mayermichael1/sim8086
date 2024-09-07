#ifndef PRINT_H
#define PRINT_H

#include "types.h"


void 
print_operand (operand op);

void 
print_operation (const char* operation, operand destination, operand source);

unsigned int 
print_cycles (byte* registers, const char* operation, operand destination, operand source);

void 
print_registers (byte* registers);

void 
print_flags (byte* registers);

#endif //PRINT_H
