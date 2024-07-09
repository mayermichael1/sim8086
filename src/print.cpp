#include "print.h"

#include <stdio.h>

#include "register_rm.h"

void 
print_operand (operand op)
{
  if (op.type == OP_REGISTER)
    {
      const char* op_string = get_register_name(
                            op.reg, 
                            op.wide); 
      printf("%s", op_string);
    }
  else if (op.type == OP_MEMORY_LOCATION)
    {
      char sign = '+';
      printf ("[%s", RM_FIELD_NAMES[op.rm]);
      int displacement = op.displacement;
      if (displacement != 0)
        {
          if (displacement < 0)
            {
              displacement *= -1;
              sign = '-';
            }
          printf(" %c %u", sign, displacement);
        }
      printf("]");
    }
  else if (op.type == OP_IMMEDIATE)
    {
      printf("%i", op.value);
    }
  else if (op.type == OP_ADDRESS)
    {
      printf("[%u]", op.address);
    }
}

void 
print_mov (operand destination, operand source)
{

  if (destination.type == OP_IMMEDIATE)
    {
      printf(";operation not permitted");
      return;
    }

  printf("mov ");

  print_operand(destination);

  printf(", ");

  if (destination.type == OP_MEMORY_LOCATION && source.type == OP_IMMEDIATE)
    {
      if (source.wide)
        {
          printf("word ");
        }
      else
        {
          printf("byte ");
        }
    }

  print_operand(source);

  printf("\n");
}

