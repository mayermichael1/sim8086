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
  
  union
  {
    byte reg;
    byte sr;
    byte base_register;
  };
  
  byte offset_register;

  union
  {
    word value;
    word displacement;
    word instruction_offset;
  };
};

#endif
