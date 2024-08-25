#ifndef TYPES_H
#define TYPES_H

typedef signed char byte;
typedef signed short word; 

typedef unsigned char ubyte;
typedef unsigned short uword; 

enum operand_type
{
  OP_NONE,
  OP_IMMEDIATE,
  OP_REGISTER,
  OP_MEMORY_LOCATION,
  OP_ADDRESS,
  OP_SEGMENT,
  OP_INSTRUCTION_OFFSET,
};

struct operand
{
  operand_type type;
  bool wide;
  word value;
  byte reg;
  byte rm;
  word displacement;
  word address;
  byte sr;
  byte instruction_offset;
};

#endif
