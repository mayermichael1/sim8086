#include "architecture.h"

#include <stdio.h>

#include <string.h>

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

const word IP_OFFSET = 24;

byte
get_register_by_name (const char *name)
{
  int register_count = sizeof(REGISTER_NAMES)/sizeof(char*);

  for (byte i = 0; i < register_count; ++i)
    {
      if (strcmp(name, get_register_name(i, false)) == 0)
        {
          return i;
        }
    }

  int register_wide_count = sizeof(REGISTER_NAMES_WIDE)/sizeof(char*);
  for (byte i = 0; i < register_wide_count; ++i)
    {
      if (strcmp(name, get_register_name(i, true)) == 0)
        {
          return i;
        }
    }
  return 0;
}

static void
convert_rm_field_to_mem_location (operand* operand, byte rm)
{
  switch (rm)
  {
    case 0b000:
      {
        operand->base_register = get_register_by_name("bx");
        operand->offset_register = get_register_by_name("si");
        break;
      }
    case 0b001:
      {
        operand->base_register = get_register_by_name("bx");
        operand->offset_register = get_register_by_name("di");
        break;
      }
    case 0b010:
      {
        operand->base_register = get_register_by_name("bp");
        operand->offset_register = get_register_by_name("si");
        break;
      }
    case 0b011:
      {
        operand->base_register = get_register_by_name("bp");
        operand->offset_register = get_register_by_name("di");
        break;
      }
    case 0b100:
      {
        operand->base_register = get_register_by_name("si");
        break;
      }
    case 0b101:
      {
        operand->base_register = get_register_by_name("di");
        break;
      }
    case 0b110:
      {
        operand->base_register = get_register_by_name("bp");
        break;
      }
    case 0b111:
      {
        operand->base_register = get_register_by_name("bx");
        break;
      }
    default:
      {
        break;
      }

  }
}

void
fill_operand_by_mod ( byte *memory, byte *registers, byte mod, byte rm, bool w,
  operand *operand )
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
          word address = read_word_using_ip(memory, registers);
          
          operand->type = OP_MEMORY_LOCATION;
          operand->displacement = address;
        }
      else
        {
          operand->type = OP_MEMORY_LOCATION;
          convert_rm_field_to_mem_location(operand, rm);
          operand->wide = false;
        }
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
    {
      byte displacement = read_byte_using_ip(memory, registers);
      operand->type = OP_MEMORY_LOCATION;
      convert_rm_field_to_mem_location(operand, rm);
      operand->wide = false;
      operand->displacement = displacement;
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
    {
      word displacement = read_word_using_ip(memory, registers);
      operand->type = OP_MEMORY_LOCATION;
      convert_rm_field_to_mem_location(operand, rm);
      operand->wide = false;
      operand->displacement = displacement;
    }
  else
    {
      printf ("; NOT IMPLEMENTED mod: %i", mod);
    }
}
