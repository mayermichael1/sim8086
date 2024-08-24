#include "simulate.h"

void 
simulate_mov(byte* registers, operand destination, operand source)
{

  if (destination.type == OP_REGISTER || destination.type == OP_SEGMENT)
    {
      word value = read_value_from_operand(registers, source);
      write_value_to_register(registers, destination, value);
    }
}

void 
simulate_arithmetic ( byte* registers, 
                      operand destination,
                      operand source, 
                      ARITHMETIC_TYPES type )
{
  word value1 = read_value_from_operand(registers, destination);;
  word value2 = read_value_from_operand(registers, source);

  word result = 0;

  switch (type)
    {
      case ARITHMETIC_ADD:
        {
          result = value1 + value2;
          break;
        }
      case ARITHMETIC_SUB:
        {
          result = value1 - value2;
          break;
        }
      case ARITHMETIC_CMP:
        {
          result = value1 - value2;
          break;
        }
    }

  if (result < 0) 
    {
      // sign flag here
    }
  if (result == 0)
    {
      // zero flag here
    }

  if (destination.type == OP_REGISTER && type != ARITHMETIC_CMP)
    {
      write_value_to_register(registers, destination, result);
    }
}
