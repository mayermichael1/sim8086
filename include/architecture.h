#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include "types.h"
#include "binary.h"

//
//  segment registers register memory order
//
//  CS  16
//  DS  18
//  SS  20
//  ES  22
//
//  IP
//
//
//  segment register instruction order
//
//  ES 00
//  CS 01
//  SS 10
//  DS 11
extern const char* REGISTER_NAMES[];
extern const word REGISTER_OFFSET[];
extern const char* REGISTER_NAMES_WIDE[];
extern const word REGISTER_OFFSET_WIDE[];
extern const char* RM_FIELD_NAMES[];
extern const char* SEGMENT_REGISTER_NAMES[];
extern const word SEGMENT_REGISTER_OFFSET[];

static const byte MOV_ADR_TO_ADR = 0b10001000;
static const byte MOV_MOD_REG_TO_REG = 0b11;
static const byte MOV_MOD_MEM_MODE = 0b00;
static const byte MOV_MOD_MEM_MODE_DISPLACE_1 = 0b01;
static const byte MOV_MOD_MEM_MODE_DISPLACE_2 = 0b10;

static const byte MOV_MEM_TO_ACCUMULATOR = 0b10100000;
static const byte MOV_ACCUMULATOR_TO_MEM = 0b10100010;
static const byte MOV_IMMEDIATE_TO_REGISTER = 0b10110000;
static const byte MOV_IMMEDIATE_TO_MEM_OR_REG = 0b11000110;

static const byte MOV_REG_OR_MEM_TO_SEGMENT = 0b10001110;
static const byte MOV_SEGMENT_TO_REG_OR_MEM     = 0b10001100;

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
