#include <iostream>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "binary.h"

static const char* REGISTER_NAMES[] = 
{
  "al",
  "cl",
  "dl",
  "bl",
  "ah",
  "ch",
  "dh",
  "bh"
};

static const char* REGISTER_NAMES_WIDE[] = 
{
  "ax",
  "cx",
  "dx",
  "bx",
  "sp",
  "bp",
  "si",
  "di"
};

static const char* RM_FIELD_NAMES[] = 
{
  "bx + si",
  "bx + di",
  "bp + si",
  "bp + di",
  "si",
  "di",
  "bp",
  "bx",
};

static const byte MOV_ADR_TO_ADR = 0b10001000;
static const byte MOV_MOD_REG_TO_REG = 0b11;
static const byte MOV_MOD_MEM_MODE = 0b00;
static const byte MOV_MOD_MEM_MODE_DISPLACE_1 = 0b01;
static const byte MOV_MOD_MEM_MODE_DISPLACE_2 = 0b10;

static const byte MOV_MEM_TO_ACCUMULATOR = 0b10100000;
static const byte MOV_ACCUMULATOR_TO_MEM = 0b10100010;
static const byte MOV_IMMEDIATE_TO_REGISTER = 0b10110000;
static const byte MOV_IMMEDIATE_TO_MEM_OR_REG = 0b11000110;

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
print_mov_register_to_register (byte reg, byte rm, bool destination, bool wide)
{ 
  byte destination_register;
  byte source_register;
  if ( destination)
    {
      destination_register = reg;
      source_register = rm;
    }
  else
    {
      destination_register = rm;
      source_register = reg;
    }

  printf ("mov %s, %s\n", 
      get_register_name (destination_register, wide), 
      get_register_name (source_register, wide));
}

void 
print_mov_reg_and_mem ( byte reg, byte rm, bool destination, bool wide, 
                        int displacement)
{
  if ( displacement != 0 )
    {
      char sign = '+';
      if (displacement < 0)
        {
          sign = '-';
          displacement *= -1;
        }

      if (destination) // register is destination (load)
        {
          printf ("mov %s, [%s %c %u]\n", 
                  get_register_name (reg, wide), 
                  RM_FIELD_NAMES[rm],
                  sign,
                  displacement);
        }
      else // register is source (store)
        {
          printf ("mov [%s %c %u], %s\n", 
                  RM_FIELD_NAMES[rm],
                  sign,
                  displacement,
                  get_register_name (reg, wide));
        }
    }
  else
    {
      if (destination) // register is destination (load)
        {
          printf ("mov %s, [%s]\n", 
                  get_register_name (reg, wide), 
                  RM_FIELD_NAMES[rm]);
        }
      else // register is source (store)
        {
          printf ("mov [%s], %s\n", 
                  RM_FIELD_NAMES[rm],
                  get_register_name (reg, wide));
        }
    }
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

  return 0;
}

