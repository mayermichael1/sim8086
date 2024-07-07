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
};

struct operand
{
  operand_type type;
  bool wide;
  word value;
  byte reg;
  byte rm;
  word displacement;
};

void 
print_mov (operand destination, operand source)
{

  if (destination.type == OP_IMMEDIATE)
    {
      printf(";operation not permitted");
      return;
    }

  printf("mov ");

  if (destination.type == OP_REGISTER)
    {
      const char* destination_string = get_register_name(
                            destination.reg, 
                            destination.wide); 
      printf("%s", destination_string);
    }
  else if (destination.type == OP_MEMORY_LOCATION)
    {
      char sign = '+';
      printf ("[ %s", RM_FIELD_NAMES[destination.rm]);
      int displacement = destination.displacement;
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

  if (source.type == OP_REGISTER)
    {
      const char* source_string = get_register_name(
                            source.reg, 
                            source.wide); 
      printf("%s", source_string);
    }
  else if (source.type == OP_MEMORY_LOCATION)
    {
      char sign = '+';
      printf ("[ %s", RM_FIELD_NAMES[source.rm]);
      int displacement = source.displacement;
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
  else if (source.type == OP_IMMEDIATE)
    {
      printf("%i", source.value);
    }

  printf("\n");
}

int 
main (int argc, char** argv)
{
  printf("%i %s\n", argc, argv[0]);

  operand dst;
  operand src;

  dst.type = OP_REGISTER;
  dst.reg = 001;
  dst.wide = true;

  src.type = OP_IMMEDIATE;
  src.value = 1;
  src.wide = true;
  print_mov(dst, src);

  /*
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

          if (mod == MOV_MOD_REG_TO_REG) // register to register move
            {
              print_mov_register_to_register (reg, rm, d, w);
            }
          else if (mod == MOV_MOD_MEM_MODE)
            {
              if (rm == 0b110) // special case direct address
                {
                  word displacement = 0;
                  fread (&displacement, sizeof(displacement), 1, fp);
                  if (d == 1) 
                    {
                      printf ("mov %s, [%u]\n", 
                              get_register_name (reg, w), 
                              displacement);
                    }
                  else
                    {
                      printf ("mov [%u], [%s]\n", 
                              displacement,
                              get_register_name (reg, w)); 
                    }
                }
              else
                {
                  print_mov_reg_and_mem(reg, rm, d, w, 0);
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
            {
              byte displacement;
              fread (&displacement, sizeof(displacement), 1, fp);
              print_mov_reg_and_mem(reg, rm, d, w, displacement);
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
            {
              word displacement;
              fread (&displacement, sizeof(displacement), 1, fp);
              print_mov_reg_and_mem(reg, rm, d, w, displacement);
            }
          else
            {
              printf ("; NOT IMPLEMENTED %s %s\n", 
                      byte_to_binary_string(first_byte),
                      byte_to_binary_string(second_byte));
            }
        }
      else if (mask(first_byte, 0b11111110) == MOV_MEM_TO_ACCUMULATOR)
        {
          uword memory;
          fread (&memory, sizeof(memory), 1, fp);

          byte w = first_byte & 1;
          printf("mov %s, [%i]\n", get_register_name(0, w), memory);
        }
      else if (mask(first_byte, 0b11111110) == MOV_ACCUMULATOR_TO_MEM)
        {
          uword memory;
          fread (&memory, sizeof(memory), 1, fp);

          byte w = first_byte & 1;
          printf("mov [%i], %s\n", memory, get_register_name(0, w));
        }
      else if (mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
        {
          byte w = (first_byte >> 3) & 1;
          byte reg = mask (first_byte, 0b00000111);
          
          int immediate_value = 0;;
          if (w == 1)
            {
              fread (&immediate_value, sizeof(word), 1, fp);
            }
          else
            {
              fread (&immediate_value, sizeof(byte), 1, fp);
            }
          printf("mov %s, %i\n", get_register_name(reg, w), immediate_value);
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

  */
  return 0;
}

