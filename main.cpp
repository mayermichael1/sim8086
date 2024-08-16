#include <iostream>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "binary.h"
#include "register_rm.h"
#include "print.h"
#include "simulate.h"

#define KiB 1024
#define MiB (KiB * KiB)
#define GiB (KiB * KiB)

void
create_register_by_mod (byte mod, byte rm, bool w, byte **cursor, 
  operand *operand)
{
  if (mod == MOV_MOD_REG_TO_REG) // register to register move
    {
      operand->type = OP_REGISTER;
      operand->reg = rm;
      operand->wide = w;
    }
  else if (mod == MOV_MOD_MEM_MODE)
    {
      if (rm == 0b110) // special case direct address
        {
          word address = read_word(cursor);
          
          operand->type = OP_ADDRESS;
          operand->address = address;
        }
      else
        {
          operand->type = OP_MEMORY_LOCATION;
          operand->rm = rm;
          operand->wide = false;
        }
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
    {
      byte displacement = read_byte(cursor);
      operand->type = OP_MEMORY_LOCATION;
      operand->rm = rm;
      operand->wide = false;
      operand->displacement = displacement;
    }
  else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
    {
      word displacement = read_word(cursor);
      operand->type = OP_MEMORY_LOCATION;
      operand->rm = rm;
      operand->wide = false;
      operand->displacement = displacement;
    }
  else
    {
      printf ("; NOT IMPLEMENTED mod: %i", mod);
    }
}

byte registers[12*2] = {0};
byte memory[MiB] = {0};

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

  fread(&memory, sizeof(byte), MiB, fp);
  fclose(fp);

#if 0
  operand a, b;
  a.type = OP_REGISTER;
  a.wide = true;
  a.reg = 0;

  b.type = OP_IMMEDIATE;
  b.value = 257;

  simulate_mov(registers,a, b);
  print_registers(registers);
  printf("ax: %i", read_value_from_register(registers, a));
#endif


#if 1
  printf("bits 16\n"); // compatibility with source

  byte *cursor = memory;
  
  while ((*cursor)) 
    {
      byte first_byte = read_byte(&cursor);

      // check operations
      if (mask(first_byte, 0b11111100) == MOV_ADR_TO_ADR)
        {
          byte second_byte = read_byte(&cursor);

          byte d =   mask((first_byte >> 1), 0b00000001);
          byte w =   mask((first_byte >> 0), 0b00000001);

          byte mod = mask((second_byte >> 6), 0b00000011);
          byte reg = mask((second_byte >> 3), 0b00000111);
          byte rm =  mask((second_byte >> 0), 0b00000111);

          operand reg_operand = {};
          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          operand rm_operand = {};

          create_register_by_mod(mod, rm, w, &cursor, &rm_operand);
          
          if (d)
            {
              print_mov(reg_operand, rm_operand);
              simulate_mov(registers,reg_operand, rm_operand);
            }
          else
            {
              print_mov(rm_operand, reg_operand);
              simulate_mov(registers,rm_operand, reg_operand);
            }
        }
      else if (mask(first_byte, 0b11111110) == MOV_MEM_TO_ACCUMULATOR)
        {
          uword memory = read_uword(&cursor);

          byte w = first_byte & 1;

          operand reg = {};
          reg.type = OP_REGISTER;
          reg.reg = 0;
          reg.wide = w;

          operand addr = {};
          addr.type = OP_ADDRESS;
          addr.address = memory;

          print_mov(reg, addr);
          simulate_mov(registers,reg, addr);
        }
      else if (mask(first_byte, 0b11111110) == MOV_ACCUMULATOR_TO_MEM)
        {
          uword memory = read_uword(&cursor);

          byte w = first_byte & 1;

          operand reg = {};
          reg.type = OP_REGISTER;
          reg.reg = 0;
          reg.wide = w;

          operand addr = {};
          addr.type = OP_ADDRESS;
          addr.address = memory;

          print_mov(addr, reg);
          simulate_mov(registers,addr, reg);
        }
      else if (mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
        {
          byte w = (first_byte >> 3) & 1;
          byte reg = mask (first_byte, 0b00000111);
          
          int immediate_value = 0;;
          if (w == 1)
            {
              word immediate_value_word = read_word(&cursor);
              immediate_value = immediate_value_word;
            }
          else
            {
              byte immediate_value_byte = read_byte(&cursor);
              immediate_value = immediate_value_byte;
            }

          operand reg_operand = {};
          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          operand immediate = {};
          immediate.type = OP_IMMEDIATE;
          immediate.value = immediate_value;

          print_mov(reg_operand, immediate);
          simulate_mov(registers,reg_operand, immediate);
        }
      else if (mask(first_byte, 0b11111110) == MOV_IMMEDIATE_TO_MEM_OR_REG)
        {
          byte w = first_byte & 1;

          byte second_byte = read_byte(&cursor);

          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte rm = mask (second_byte, 0b00000111);

          operand immediate = {};
          immediate.type = OP_IMMEDIATE;
          immediate.wide = w;

          operand destination = {};
          destination.type = OP_MEMORY_LOCATION;
          destination.rm = rm;

          create_register_by_mod(mod, rm, w, &cursor, &destination);
          
          if (immediate.wide)
            {
              immediate.value = read_word(&cursor);
            }
          else 
            {
              immediate.value = read_byte(&cursor);
            }

          print_mov(destination, immediate);
          simulate_mov(registers,destination, immediate);

        }
      else if (first_byte == MOV_REG_OR_MEM_TO_SEGMENT)
        {
          byte second_byte = read_byte(&cursor);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;

          create_register_by_mod(mod, rm, true, &cursor, &source);

          destination.type = OP_SEGMENT;
          destination.sr = sr;

          print_mov(destination, source);
          simulate_mov(registers, destination, source);
        }
      else if (first_byte == MOV_SEGMENT_TO_REG_OR_MEM)
        {    
          byte second_byte = read_byte(&cursor);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;
          
          create_register_by_mod(mod, rm, true, &cursor, &destination);

          source.type = OP_SEGMENT;
          source.sr = sr;

          print_mov(destination, source);
          simulate_mov(registers, destination, source);
        }
      else
        {
          printf ("; NOT IMPLEMENTED %s\n",byte_to_binary_string(first_byte));
        }
    }
#endif
    print_registers(registers);
  return 0;
}


