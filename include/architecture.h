#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "types.h"
#include "binary.h"

extern const char* REGISTER_NAMES[];
extern const word REGISTER_OFFSET[];
extern const char* REGISTER_NAMES_WIDE[];
extern const word REGISTER_OFFSET_WIDE[];
extern const char* RM_FIELD_NAMES[];
extern const char* SEGMENT_REGISTER_NAMES[];
extern const word SEGMENT_REGISTER_OFFSET[];

enum OPERATION_CODES
{
  MOV_ADR_TO_ADR = 0b10001000,
  MOV_MOD_REG_TO_REG = 0b11,
  MOV_MOD_MEM_MODE = 0b00,
  MOV_MOD_MEM_MODE_DISPLACE_1 = 0b01,
  MOV_MOD_MEM_MODE_DISPLACE_2 = 0b10,

  MOV_MEM_TO_ACCUMULATOR = 0b10100000,
  MOV_ACCUMULATOR_TO_MEM = 0b10100010,
  MOV_IMMEDIATE_TO_REGISTER = 0b10110000,
  MOV_IMMEDIATE_TO_MEM_OR_REG = 0b11000110,

  MOV_REG_OR_MEM_TO_SEGMENT = 0b10001110,
  MOV_SEGMENT_TO_REG_OR_MEM     = 0b10001100,
};

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
fill_operand_by_mod (byte mod, byte rm, bool w, byte **cursor, operand *operand);

#endif
