#include "register_rm.h"

#include <stdio.h>

void
fill_operand_by_mod (byte mod, byte rm, bool w, byte **cursor, operand *operand)
{
  if (mod == MOV_MOD_REG_TO_REG) // register to register move
    {
      operand->type = OP_REGISTER;
      operand->reg = rm;
      operand->wide = w;
    }
  else if (mod == MOV_MOD_MEM_MODE)
    {
      if (rm == 0b110) // special case direct address
        {
          word address = read_word(cursor);
          
          operand->type = OP_ADDRESS;
          operand->address = address;
        }
      else
        {
          operand->type = OP_MEMORY_LOCATION;
          operand->rm = rm;
          operand->wide = false;
        }
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
    {
      byte displacement = read_byte(cursor);
      operand->type = OP_MEMORY_LOCATION;
      operand->rm = rm;
      operand->wide = false;
      operand->displacement = displacement;
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
    {
      word displacement = read_word(cursor);
      operand->type = OP_MEMORY_LOCATION;
      operand->rm = rm;
      operand->wide = false;
      operand->displacement = displacement;
    }
  else
    {
      printf ("; NOT IMPLEMENTED mod: %i", mod);
    }
}
