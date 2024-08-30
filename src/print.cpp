#include "print.h"

#include <stdio.h>

#include "architecture.h"
#include "simulate.h"
#include "binary.h"

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
      printf("[");

      if (op.base_register != 0)
        {
          printf ("%s", get_register_name(op.base_register, true));
          if (op.offset_register != 0)
            {
              printf(" + %s", get_register_name(op.offset_register, true));
            }

          char sign = '+';
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
        }
      else  // direct address
      {
        printf("%u", op.displacement);
      }

      printf("]");
    }
  else if (op.type == OP_IMMEDIATE)
    {
      printf("%i", op.value);
    }
  else if (op.type == OP_SEGMENT)
    {
      const char* op_string = SEGMENT_REGISTER_NAMES[op.sr];
      printf("%s", op_string);
    }
  else if (op.type == OP_INSTRUCTION_OFFSET)
    {
      char sign = '+';
      int offset = op.instruction_offset;
      if (op.instruction_offset < 0)
        {
          sign = '-';
          offset *= -1;
        }
      printf("$%c%u", sign, offset); }
}

void 
print_operation (const char* operation, operand destination, operand source)
{

  if (destination.type == OP_IMMEDIATE)
    {
      printf(";operation not permitted");
      return;
    }

  printf("%s ", operation);

  print_operand(destination);

  if (destination.type != OP_NONE)
    {
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
    }

  printf("\n");
}

void 
print_registers (byte* registers)
{
  //TODO: can not easily create a operand refering to a specific register
  //      without knowing the number

  word ax = (word)(*(word*)(registers));
  word bx = (word)(*(word*)(registers+2));
  word cx = (word)(*(word*)(registers+4));
  word dx = (word)(*(word*)(registers+6));

  word sp = (word)(*(word*)(registers+8));
  word bp = (word)(*(word*)(registers+10));
  word si = (word)(*(word*)(registers+12));
  word di = (word)(*(word*)(registers+14));

  word cs = (word)(*(word*)(registers+16));
  word ds = (word)(*(word*)(registers+18));
  word ss = (word)(*(word*)(registers+20));
  word es = (word)(*(word*)(registers+22));

  printf(";registers:\n");

  printf(";al: %i\t", get_low_byte(ax));
  printf(";ah: %i\t", get_high_byte(ax));
  printf(";ax: %i\n", ax);

  printf(";bl: %i\t", get_low_byte(bx));
  printf(";bh: %i\t", get_high_byte(bx));
  printf(";bx: %i\n", bx);

  printf(";cl: %i\t", get_low_byte(cx));
  printf(";ch: %i\t", get_high_byte(cx));
  printf(";cx: %i\n", cx);

  printf(";dl: %i\t", get_low_byte(dx));
  printf(";dh: %i\t", get_high_byte(dx));
  printf(";dx: %i\n", dx);

  printf(";sp: %i\n", sp);
  printf(";bp: %i\n", bp);
  printf(";si: %i\n", si);
  printf(";di: %i\n", di);

  printf(";cs: %i\n", cs);
  printf(";ds: %i\n", ds);
  printf(";ss: %i\n", ss);
  printf(";es: %i\n", es);

  printf("\n");

}

void 
print_flags (byte* registers)
{
  word flag_register = *(registers+FLAG_OFFSET);

  bool zero_flag = flag_register & FLAG_ZERO;
  printf(";zero flag: %i\n", zero_flag);

  bool sign_flag = flag_register & FLAG_SIGN;
  printf(";sign flag: %i\n", sign_flag);

  printf("\n");
}

