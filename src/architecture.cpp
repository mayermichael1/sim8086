#include "architecture.h"

#include <stdio.h>

const char* REGISTER_NAMES[] = 
{
  "al",
  "cl",
  "dl",
  "bl",
  "ah",
  "ch",
  "dh",
  "bh"
};

const word REGISTER_OFFSET[] =
{
  0,
  4,
  6,
  2,
  1,
  5,
  7,
  3
};

const char* REGISTER_NAMES_WIDE[] = 
{
  "ax",
  "cx",
  "dx",
  "bx",
  "sp",
  "bp",
  "si",
  "di"
};

const word REGISTER_OFFSET_WIDE[] = 
{
  0,
  4,
  6,
  2,
  8,
  10,
  12,
  14
};

const char* RM_FIELD_NAMES[] = 
{
  "bx + si",
  "bx + di",
  "bp + si",
  "bp + di",
  "si",
  "di",
  "bp",
  "bx",
};

const char* SEGMENT_REGISTER_NAMES[] = 
{
  "ES",
  "CS",
  "SS",
  "DS",
};

const word SEGMENT_REGISTER_OFFSET[] =
{
  22, // ES
  16, // CS
  20, // SS
  18, // DS
};

const word FLAG_OFFSET = 26;

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
