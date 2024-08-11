#ifndef SIMULATE_h
#define SIMULATE_h

#include "types.h"
#include "register_rm.h"

void 
simulate_mov(byte *registers, operand destination, operand source);

inline word
read_value_from_register (byte *registers, operand reg)
{
  word value = 0;
  if (reg.type == OP_REGISTER)
    {
      if (reg.wide)
        {
          byte offset = REGISTER_OFFSET_WIDE[reg.reg];
          value = *(word*)(registers+offset);
        }
      else
        {
          byte offset = REGISTER_OFFSET[reg.reg];
          value = *(registers+offset);
        }
    }
  return value;
}

inline void 
write_value_to_register (byte *registers, operand reg, word value)
{
  if (reg.type == OP_REGISTER)
    {
      byte offset = REGISTER_OFFSET[reg.reg];

      byte lowValue = get_low_byte(value);
      byte highValue = get_high_byte(value);

      if (reg.wide)
        {
          offset = REGISTER_OFFSET_WIDE[reg.reg];
          registers[offset+1] = highValue;
        }

      registers[offset] = lowValue;
    }
}

#endif
