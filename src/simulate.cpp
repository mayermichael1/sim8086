#include "simulate.h"

void 
simulate_mov(byte* registers, operand destination, operand source)
{

  if (destination.type == OP_REGISTER)
    {
      if (source.type == OP_IMMEDIATE)
        {
          write_value_to_register(registers, destination, source.value);
       }
      else if (source.type == OP_REGISTER)
        {
          word value = read_value_from_register(registers, source);
          write_value_to_register(registers, destination, value);
        }
    }
}
