#ifndef PRINT_H
#define PRINT_H

#include "types.h"


void 
print_operand (operand op);

void 
print_mov (operand destination, operand source);

void 
print_registers (byte* registers);

#endif //PRINT_H
