#include <iostream>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "binary.h"
#include "register.h"

enum operand_type
{
  OP_IMMEDIATE,
  OP_REGISTER,
  OP_MEMORY_LOCATION,
  OP_ADDRESS,
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
};

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
      printf ("[ %s ", RM_FIELD_NAMES[op.rm]);
      int displacement = op.displacement;
      if (displacement != 0)
        {
          if (displacement < 0)
            {
              displacement *= -1;
              sign = '-';
            }
          printf("%c %u", sign, displacement);
        }
      printf("]");
    }
  else if (op.type == OP_IMMEDIATE)
    {
      printf("%i", op.value);
    }
  else if (op.type == OP_ADDRESS)
    {
      printf("[ %u ]", op.address);
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
      if (source.value)
        {
          printf("word");
        }
      else
        {
          printf("byte");
        }
    }

  print_operand(source);

  printf("\n");
}

int 
main (int argc, char** argv)
{
  if (argc != 2)
    {
      return 1;
    }

  char* filename = argv[1];
  FILE *fp = fopen (filename, "rb");

  if ( fp == NULL)
    {
      return 1;
    }

  printf("bits 16\n"); // compatibility with source

  byte first_byte;
  while (fread (&first_byte, sizeof(byte), 1, fp)) 
    {
      // instruction line layout:
      // OPER  DW   MOREGRM 
      // OOOOOODW   MMRRRMMM
      // OPER ... 6 bit operator
      // D ... order of register fields
      // W ... 16 bit or 8bit 
      // MO ... MOD
      // REG ... REGISTER
      // RM ... Register or Memory

      // check operations
      if (mask(first_byte, 0b11111100) == MOV_ADR_TO_ADR)
        {
          byte second_byte;
          fread(&second_byte, sizeof(byte), 1, fp);

          byte d =   mask((first_byte >> 1), 0b00000001);
          byte w =   mask((first_byte >> 0), 0b00000001);

          byte mod = mask((second_byte >> 6), 0b00000011);
          byte reg = mask((second_byte >> 3), 0b00000111);
          byte rm =  mask((second_byte >> 0), 0b00000111);

          operand reg_operand;
          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          operand rm_operand;

          if (mod == MOV_MOD_REG_TO_REG) // register to register move
            {
              rm_operand.type = OP_REGISTER;
              rm_operand.reg = rm;
              rm_operand.wide = w;
            }
          else if (mod == MOV_MOD_MEM_MODE)
            {
              if (rm == 0b110) // special case direct address
                {
                  word address = 0;
                  fread (&address, sizeof(address), 1, fp);
                  rm_operand.type = OP_ADDRESS;
                  rm_operand.address = address;
                }
              else
                {
                  rm_operand.type = OP_MEMORY_LOCATION;
                  rm_operand.rm = rm;
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
            {
              byte displacement;
              fread (&displacement, sizeof(displacement), 1, fp);
              rm_operand.type = OP_MEMORY_LOCATION;
              rm_operand.rm = rm;
              rm_operand.wide = false;
              rm_operand.displacement = displacement;
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
            {
              word displacement;
              fread (&displacement, sizeof(displacement), 1, fp);
              rm_operand.type = OP_MEMORY_LOCATION;
              rm_operand.rm = rm;
              rm_operand.wide = false;
              rm_operand.displacement = displacement;
            }
          else
            {
              printf ("; NOT IMPLEMENTED %s %s\n", 
                      byte_to_binary_string(first_byte),
                      byte_to_binary_string(second_byte));
            }
          
          if (d)
            {
              print_mov(reg_operand, rm_operand);
            }
          else
            {
              print_mov(rm_operand, reg_operand);
            }
        }
      else if (mask(first_byte, 0b11111110) == MOV_MEM_TO_ACCUMULATOR)
        {
          uword memory;
          fread (&memory, sizeof(memory), 1, fp);

          byte w = first_byte & 1;

          operand reg;
          reg.type = OP_REGISTER;
          reg.reg = 0;
          reg.wide = w;

          operand addr;
          addr.type = OP_ADDRESS;
          addr.address = memory;

          print_mov(reg, addr);
        }
      else if (mask(first_byte, 0b11111110) == MOV_ACCUMULATOR_TO_MEM)
        {
          uword memory;
          fread (&memory, sizeof(memory), 1, fp);

          byte w = first_byte & 1;

          operand reg;
          reg.type = OP_REGISTER;
          reg.reg = 0;
          reg.wide = w;

          operand addr;
          addr.type = OP_ADDRESS;
          addr.address = memory;

          print_mov(addr, reg);
        }
      else if (mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
        {
          byte w = (first_byte >> 3) & 1;
          byte reg = mask (first_byte, 0b00000111);
          
          int immediate_value = 0;;
          if (w == 1)
            {
              word immediate_value_word; 
              fread (&immediate_value_word, sizeof(word), 1, fp);
              immediate_value = immediate_value_word;
            }
          else
            {
              byte immediate_value_byte;
              fread (&immediate_value_byte, sizeof(byte), 1, fp);
              immediate_value = immediate_value_byte;
            }

          operand reg_operand;
          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          operand immediate;
          immediate.type = OP_IMMEDIATE;
          immediate.value = immediate_value;

          print_mov(reg_operand, immediate);
        }
      else if (mask(first_byte, 0b11111110) == MOV_IMMEDIATE_TO_MEM_OR_REG)
        {
          byte w = first_byte & 1;

          byte second_byte;
          fread (&second_byte, sizeof(second_byte), 1, fp);

          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte rm = mask (second_byte, 0b00000111);

          if (mod == MOV_MOD_REG_TO_REG) // register to register move
            {
              printf("; THIS SHOULD NEVER HAPPEN");
              if (w)
                {
                  byte data;
                  fread( &data, sizeof(data), 1, fp);
                  printf("mov %s, byte %i\n", get_register_name(rm, w), data);
                }
              else
                {
                  word data;
                  fread( &data, sizeof(data), 1, fp);
                  printf("mov %s, word %i\n", get_register_name(rm, w), data);
                }
            }
          else if (mod == MOV_MOD_MEM_MODE)
            {
              if (w)
                {
                  word data;
                  fread( &data, sizeof(data), 1, fp);
                  printf("mov [%s], word %i\n", RM_FIELD_NAMES[rm], data);
                }
              else
                {
                  byte data;
                  fread( &data, sizeof(data), 1, fp);
                  printf("mov [%s], byte %i\n", RM_FIELD_NAMES[rm], data);
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
            {
              byte displacement;
              fread( &displacement, sizeof(displacement), 1, fp);
              char sign = '+';
              if (displacement < 0)
                {
                  sign = '-';
                  displacement *= -1;
                }
              
              if (w)
                {
                  word data;
                  fread( &data, sizeof(data), 1, fp);
                  printf( "mov [%s %c %u], word %i\n", 
                          RM_FIELD_NAMES[rm], 
                          sign,
                          displacement,
                          data);
                }
              else
                {
                  byte data;
                  fread( &data, sizeof(data), 1, fp);
                  printf( "mov [%s %c %u], byte %i\n", 
                          RM_FIELD_NAMES[rm], 
                          sign,
                          displacement,
                          data);
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
            {
              word displacement;
              fread( &displacement, sizeof(displacement), 1, fp);
              char sign = '+';
              if (displacement < 0)
                {
                  sign = '-';
                  displacement *= -1;
                }
              
              if (w)
                {
                  word data;
                  fread( &data, sizeof(data), 1, fp);
                  printf( "mov [%s %c %u], word %i\n", 
                          RM_FIELD_NAMES[rm], 
                          sign,
                          displacement,
                          data);
                }
              else
                {
                  byte data;
                  fread( &data, sizeof(data), 1, fp);
                  printf( "mov [%s %c %u], byte %i\n", 
                          RM_FIELD_NAMES[rm], 
                          sign,
                          displacement,
                          data);
                }
            }
          else
            {
              printf ("; NOT IMPLEMENTED %s %s\n", 
                      byte_to_binary_string(first_byte),
                      byte_to_binary_string(second_byte));
            }

        }
      else
        {
          printf ("; NOT IMPLEMENTED %s\n",byte_to_binary_string(first_byte));
        }
    }

  fclose (fp);

  return 0;
}

