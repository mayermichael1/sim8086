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

void 
print_registers (byte* registers)
{
  byte al = (byte)(*(registers));
  byte ah = (byte)(*(registers+1));
  word ax = (word)(*(word*)(registers));

  byte bl = (byte)(*(registers+2));
  byte bh = (byte)(*(registers+3));
  word bx = (word)(*(word*)(registers+2));

  byte cl = (byte)(*(registers+4));
  byte ch = (byte)(*(registers+5));
  word cx = (word)(*(word*)(registers+4));

  byte dl = (byte)(*(registers+6));
  byte dh = (byte)(*(registers+7));
  word dx = (word)(*(word*)(registers+6));

  word sp = (word)(*(word*)(registers+8));
  word bp = (word)(*(word*)(registers+10));
  word si = (word)(*(word*)(registers+12));
  word di = (word)(*(word*)(registers+14));

  printf(";registers:\n");

  printf(";al: %i\t", al);
  printf(";ah: %i\t", ah);
  printf(";ax: %i\n", ax);

  printf(";bl: %i\t", bl);
  printf(";bh: %i\t", bh);
  printf(";bx: %i\n", bx);

  printf(";cl: %i\t", cl);
  printf(";ch: %i\t", ch);
  printf(";cx: %i\n", cx);

  printf(";dl: %i\t", dl);
  printf(";dh: %i\t", dh);
  printf(";dx: %i\n", dx);

  printf(";sp: %i\n", sp);
  printf(";bp: %i\n", bp);
  printf(";si: %i\n", si);
  printf(";di: %i\n", di);

  printf("\n");


}

