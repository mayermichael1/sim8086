#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "types.h"

extern const char* REGISTER_NAMES[];
extern const word REGISTER_OFFSET[];
extern const char* REGISTER_NAMES_WIDE[];
extern const word REGISTER_OFFSET_WIDE[];
extern const char* SEGMENT_REGISTER_NAMES[];
extern const word SEGMENT_REGISTER_OFFSET[];
extern const word FLAG_OFFSET;
extern const word IP_OFFSET;

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

  ADD_REG_OR_MEM_PLUS_REG_TO_EITHER = 0b00000000,
  ADD_IMMEDIATE_TO_ACCUMULATOR = 0b00010100,

  SUB_REG_OR_MEM_SUB_REG_TO_EITHER = 0b00101000,
  SUB_IMMEDIATE_FROM_ACCUMULATOR = 0b00101100,

  CMP_REG_OR_MEM_AND_REG = 0b00111000,
  CMP_IMMEDIATE_WITH_ACCUMULATOR = 0b00111100,

  ARITHMETIC_IMMEDIATE_TO_REG_OR_MEM = 0b10000000,

  JE_OR_JZ = 0b01110100,
  JNE_OR_JNZ = 0b01110101,
};

enum ARITHMETIC_TYPES
{
  ARITHMETIC_ADD = 0b000,
  ARITHMETIC_SUB = 0b101,
  ARITHMETIC_CMP = 0b111,
};

enum FLAG
{
  FLAG_SIGN = 0b0000000010000000,
  FLAG_ZERO = 0b0000000001000000,
  FLAG_COUNT,
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

struct cpu_architecture
{
  byte registers[14*2] = {0};
  byte memory[MiB] = {0};
};

byte
get_register_by_name (const char *name);

void
fill_operand_by_mod (cpu_architecture *cpu, byte mod, byte rm, bool w, operand *operand);

#endif
