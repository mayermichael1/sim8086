#include "simulate.h"

static void 
set_flag (byte* registers, FLAG flag)
{
  word value = *(word*)(registers+FLAG_OFFSET);

  value = value | flag;

  byte high_byte = get_high_byte(value);
  byte low_byte = get_low_byte(value);

  registers[FLAG_OFFSET] = low_byte;
  registers[FLAG_OFFSET+1] = high_byte;
}

static bool
read_flag (byte* registers, FLAG flag)
{
  word value = *(word*)(registers+FLAG_OFFSET);

  bool flag_set = value & flag;
  return flag_set;
}

static void 
unset_flag (byte* registers, FLAG flag)
{
  word value = *(word*)(registers+FLAG_OFFSET);

  value = value & ~flag;

  byte high_byte = get_high_byte(value);
  byte low_byte = get_low_byte(value);

  registers[FLAG_OFFSET] = low_byte;
  registers[FLAG_OFFSET+1] = high_byte;
}

word 
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

void 
write_value_to_operand (byte* registers, operand reg, word value)
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

void 
simulate_mov(byte* registers, operand destination, operand source)
{

  if (destination.type == OP_REGISTER || destination.type == OP_SEGMENT)
    {
      word value = read_value_from_operand(registers, source);
      write_value_to_operand(registers, destination, value);
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
      set_flag(registers, FLAG_SIGN);
    }
  else 
    {
      unset_flag(registers, FLAG_SIGN);
    }

  if (result == 0)
    {
      set_flag(registers, FLAG_ZERO);
    }
  else 
    {
      unset_flag(registers, FLAG_ZERO);
    }

  if (destination.type == OP_REGISTER && type != ARITHMETIC_CMP)
    {
      write_value_to_operand(registers, destination, result);
    }
}


void 
simulate_jump ( byte* registers,
                bool jump_if_zero,
                operand instruction_offset )
{
  word ip = read_ip(registers);

  bool zero_flag = read_flag(registers, FLAG_ZERO);

  if (!jump_if_zero)
    {
      zero_flag = !zero_flag;
    }

  if (zero_flag)
    {
      ip += instruction_offset.instruction_offset;
    }
  registers[IP_OFFSET] = get_low_byte(ip);
  registers[IP_OFFSET+1] = get_high_byte(ip);
}

