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

inline word
read_value_from_operand (byte *registers, operand reg)
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
  else if (reg.type == OP_SEGMENT)
    {
      byte offset = SEGMENT_REGISTER_OFFSET[reg.sr];
      value = *(word*)(registers+offset);
    }
  else if (reg.type == OP_IMMEDIATE)
    {
      return reg.value;
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
  else if (reg.type == OP_SEGMENT)
    {
      byte offset = SEGMENT_REGISTER_OFFSET[reg.sr];

      byte lowValue = get_low_byte(value);
      byte highValue = get_high_byte(value);

      registers[offset+1] = highValue;
      registers[offset] = lowValue;
    }
}

#endif
