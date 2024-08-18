#include <iostream>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "binary.h"
#include "architecture.h"
#include "print.h"
#include "simulate.h"

#define KiB 1024
#define MiB (KiB * KiB)
#define GiB (KiB * KiB)

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
      if ((ubyte)mask(first_byte, 0b11111100) == MOV_ADR_TO_ADR)
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

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);
          
          if (d)
            {
              print_operation("MOV",reg_operand, rm_operand);
              simulate_mov(registers,reg_operand, rm_operand);
            }
          else
            {
              print_operation("MOV",rm_operand, reg_operand);
              simulate_mov(registers,rm_operand, reg_operand);
            }
        }
      else if ((ubyte)mask(first_byte, 0b11111110) == MOV_MEM_TO_ACCUMULATOR)
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

          print_operation("MOV",reg, addr);
          simulate_mov(registers,reg, addr);
        }
      else if ((ubyte)mask(first_byte, 0b11111110) == MOV_ACCUMULATOR_TO_MEM)
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

          print_operation("MOV",addr, reg);
          simulate_mov(registers,addr, reg);
        }
      else if ((ubyte)mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
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

          print_operation("MOV",reg_operand, immediate);
          simulate_mov(registers,reg_operand, immediate);
        }
      else if ((ubyte)mask(first_byte, 0b11111110) == MOV_IMMEDIATE_TO_MEM_OR_REG)
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

          fill_operand_by_mod(mod, rm, w, &cursor, &destination);
          
          if (immediate.wide)
            {
              immediate.value = read_word(&cursor);
            }
          else 
            {
              immediate.value = read_byte(&cursor);
            }

          print_operation("MOV",destination, immediate);
          simulate_mov(registers,destination, immediate);

        }
      else if ((ubyte)first_byte == MOV_REG_OR_MEM_TO_SEGMENT)
        {
          byte second_byte = read_byte(&cursor);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;

          fill_operand_by_mod(mod, rm, true, &cursor, &source);

          destination.type = OP_SEGMENT;
          destination.sr = sr;

          print_operation("MOV",destination, source);
          simulate_mov(registers, destination, source);
        }
      else if ((ubyte)first_byte == MOV_SEGMENT_TO_REG_OR_MEM)
        {    
          byte second_byte = read_byte(&cursor);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;
          
          fill_operand_by_mod(mod, rm, true, &cursor, &destination);

          source.type = OP_SEGMENT;
          source.sr = sr;

          print_operation("MOV",destination, source);
          simulate_mov(registers, destination, source);
        }
      else if ( 
        (ubyte)mask(first_byte, 0b11111100) == ADD_REG_OR_MEM_PLUS_REG_TO_EITHER
              )
        {
          byte second_byte = read_byte(&cursor);

          byte d = mask(first_byte >> 1, 0b00000001);
          byte w = mask(first_byte, 0b00000001);

          byte mod  = mask(second_byte >> 6, 0b00000011);
          byte reg =  mask(second_byte >> 3, 0b00000111);
          byte rm =  mask(second_byte >> 0, 0b00000111);

          operand rm_operand;
          operand reg_operand;

          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);

          operand destination, source;

          if (d)
            {
              destination = reg_operand;
              source = rm_operand;
            }
          else
            {
              source = reg_operand;
              destination = rm_operand;
            }
          print_operation("ADD", destination, source);
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == ARITHMETIC_IMMEDIATE_TO_REG_OR_MEM
              )
        {
          byte second_byte = read_byte(&cursor);

          byte s = mask(first_byte >> 1, 0b00000001);
          byte w = mask(first_byte >> 0, 0b00000001);
          
          byte mod =  mask(second_byte >> 6, 0b00000011);
          byte type = mask(second_byte >> 3, 0b00000111);
          byte rm =   mask(second_byte >> 0, 0b00000111);

          operand rm_operand;
          operand immediate;

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);
          immediate.type = OP_IMMEDIATE;
          if (w && !s)
            {
              immediate.value = read_word(&cursor);
            }
          else 
            {
              immediate.value = read_byte(&cursor);
            }

          switch (type)
            {
              case ARITHMETIC_ADD:
                {
                  print_operation("ADD", rm_operand, immediate);
                  break;
                }
              case ARITHMETIC_SUB:
                {
                  print_operation("SUB", rm_operand, immediate);
                  break;
                }
              case ARITHMETIC_CMP:
                {
                  print_operation("CMP", rm_operand, immediate);
                  break;
                }
            }
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == ADD_IMMEDIATE_TO_ACCUMULATOR
              )
        {
          //NOTE: somehow this does not fire
          printf(";debug does this fire?\n");
          byte w = mask(first_byte, 0b00000001);

          operand immediate;
          immediate.type = OP_IMMEDIATE;

          operand accumulator;
          accumulator.type = OP_REGISTER;
          accumulator.reg = 0; accumulator.wide = w;

          if (w)
            {
              immediate.value = read_word(&cursor);
            }
          else
            {
              immediate.value = read_byte(&cursor);
            }

          print_operation("ADD", accumulator, immediate);
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == SUB_REG_OR_MEM_SUB_REG_TO_EITHER
              )
        {
          byte second_byte = read_byte(&cursor);

          byte d = mask(first_byte >> 1, 0b00000001);
          byte w = mask(first_byte, 0b00000001);

          byte mod  = mask(second_byte >> 6, 0b00000011);
          byte reg =  mask(second_byte >> 3, 0b00000111);
          byte rm =  mask(second_byte >> 0, 0b00000111);

          operand rm_operand;
          operand reg_operand;

          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);

          operand destination, source;

          if (d)
            {
              destination = reg_operand;
              source = rm_operand;
            }
          else
            {
              source = reg_operand;
              destination = rm_operand;
            }
          print_operation("SUB", destination, source);

        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == SUB_IMMEDIATE_FROM_ACCUMULATOR
              )
        {
          //NOTE: somehow this does not fire
          printf(";debug does this fire?\n");
          byte w = mask(first_byte, 0b00000001);

          operand immediate;
          immediate.type = OP_IMMEDIATE;

          operand accumulator;
          accumulator.type = OP_REGISTER;
          accumulator.reg = 0;
          accumulator.wide = w;

          if (w)
            {
              immediate.value = read_word(&cursor);
            }
          else
            {
              immediate.value = read_byte(&cursor);
            }

          print_operation("SUB", accumulator, immediate);

        }
      else if ((ubyte)mask(first_byte, 0b11111100) == CMP_REG_OR_MEM_AND_REG)
        {
          byte second_byte = read_byte(&cursor);

          byte d = mask(first_byte >> 1, 0b00000001);
          byte w = mask(first_byte, 0b00000001);

          byte mod  = mask(second_byte >> 6, 0b00000011);
          byte reg =  mask(second_byte >> 3, 0b00000111);
          byte rm =  mask(second_byte >> 0, 0b00000111);

          operand rm_operand;
          operand reg_operand;

          reg_operand.type = OP_REGISTER;
          reg_operand.reg = reg;
          reg_operand.wide = w;

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);

          operand destination, source;

          if (d)
            {
              destination = reg_operand;
              source = rm_operand;
            }
          else
            {
              source = reg_operand;
              destination = rm_operand;
            }
          print_operation("CMP", destination, source);
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == CMP_IMMEDIATE_WITH_ACCUMULATOR
              )
        {
          //NOTE: somehow this does not fire
          printf(";debug does this fire?\n");
          byte w = mask(first_byte, 0b00000001);

          operand immediate;
          immediate.type = OP_IMMEDIATE;

          operand accumulator;
          accumulator.type = OP_REGISTER;
          accumulator.reg = 0;
          accumulator.wide = w;

          if (w)
            {
              immediate.value = read_word(&cursor);
            }
          else
            {
              immediate.value = read_byte(&cursor);
            }

          print_operation("CMP", accumulator, immediate);
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


