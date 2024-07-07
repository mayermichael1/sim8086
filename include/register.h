#ifndef REGISTER_H
#define REGISTER_H

#include "types.h"
#include <stdio.h>

static const char* REGISTER_NAMES[] = 
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

static const char* REGISTER_NAMES_WIDE[] = 
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

static const char* RM_FIELD_NAMES[] = 
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

static const byte MOV_ADR_TO_ADR = 0b10001000;
static const byte MOV_MOD_REG_TO_REG = 0b11;
static const byte MOV_MOD_MEM_MODE = 0b00;
static const byte MOV_MOD_MEM_MODE_DISPLACE_1 = 0b01;
static const byte MOV_MOD_MEM_MODE_DISPLACE_2 = 0b10;

static const byte MOV_MEM_TO_ACCUMULATOR = 0b10100000;
static const byte MOV_ACCUMULATOR_TO_MEM = 0b10100010;
static const byte MOV_IMMEDIATE_TO_REGISTER = 0b10110000;
static const byte MOV_IMMEDIATE_TO_MEM_OR_REG = 0b11000110;

inline const char*
get_register_name (byte reg, bool wide)
{
  if (wide)
    {
      return REGISTER_NAMES_WIDE[reg];
    }

  return REGISTER_NAMES[reg];
}

void 
print_mov_register_to_register (byte reg, byte rm, bool destination, bool wide);

void 
print_mov_reg_and_mem ( byte reg, byte rm, bool destination, bool wide, 
                        int displacement);

#endif
