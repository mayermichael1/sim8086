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

byte registers[14*2] = {0};
byte memory[MiB] = {0};

byte 
read_ip (byte *registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  return *ip_address;
}

byte 
read_byte_using_ip (byte* base_ip, byte* registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  byte byte_at_ip = *(base_ip+(*ip_address));
  (*ip_address)++;
  return byte_at_ip;
}

word 
read_word_using_ip (byte* base_ip, byte* registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  word word_at_ip = *(word*)(base_ip+(*ip_address));
  (*ip_address)+=2;
  return word_at_ip;
}

int 
main (int argc, char** argv)
{
  bool print_every_intruction = false;
  bool print_at_end = true;
  bool simulate = true;

  if (argc < 2)
    {
      return 1;
    }
  
  if (argc > 2)
    {
      for (int i = 2; i < argc; i++)
        {
          if (strcmp(argv[i], "-every") == 0)
            {
              print_every_intruction = true;
            }
          if (strcmp(argv[i], "-noprint") == 0)
            {
              print_every_intruction = false;
              print_at_end = false;
            }
          if (strcmp(argv[i], "-nosim") == 0)
            {
              simulate = false;
            }
        }
    }

  char* filename = argv[1];
  FILE *fp = fopen (filename, "rb");

  if ( fp == NULL)
    {
      return 1;
    }

  int instruction_bytes = fread(&memory, sizeof(byte), MiB, fp);
  fclose(fp);

  printf("bits 16\n"); // compatibility with source

  byte *cursor = memory;
  
  while (read_ip(registers) < instruction_bytes) 
    {
      byte first_byte = read_byte_using_ip(memory, registers);

      // check operations
      if ((ubyte)mask(first_byte, 0b11111100) == MOV_ADR_TO_ADR)
        {
          byte second_byte = read_byte_using_ip(memory, registers);

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
              if (simulate)
                {
                  simulate_mov(registers,reg_operand, rm_operand);
                }
            }
          else
            {
              print_operation("MOV",rm_operand, reg_operand);
              if (simulate)
                {
                  simulate_mov(registers,rm_operand, reg_operand);
                }
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
          if (simulate)
            {
              simulate_mov(registers,reg, addr);
            }
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
          if (simulate)
            {
              simulate_mov(registers,addr, reg);
            }
        }
      else if ((ubyte)mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
        {
          byte w = (first_byte >> 3) & 1;
          byte reg = mask (first_byte, 0b00000111);
          
          int immediate_value = 0;;
          if (w == 1)
            {
              word immediate_value_word = read_word_using_ip(memory, registers);
              immediate_value = immediate_value_word;
            }
          else
            {
              byte immediate_value_byte = read_byte_using_ip(memory, registers);
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
          if (simulate)
            {
              simulate_mov(registers,reg_operand, immediate);
            }
        }
      else if ((ubyte)mask(first_byte, 0b11111110) == MOV_IMMEDIATE_TO_MEM_OR_REG)
        {
          byte w = first_byte & 1;

          byte second_byte = read_byte_using_ip(memory, registers);

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
              immediate.value = read_word_using_ip(memory, registers);
            }
          else 
            {
              immediate.value = read_byte_using_ip(memory, registers);
            }

          print_operation("MOV",destination, immediate);
          if (simulate)
            {
              simulate_mov(registers,destination, immediate);
            }

        }
      else if ((ubyte)first_byte == MOV_REG_OR_MEM_TO_SEGMENT)
        {
          byte second_byte = read_byte_using_ip(memory, registers);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;

          fill_operand_by_mod(mod, rm, true, &cursor, &source);

          destination.type = OP_SEGMENT;
          destination.sr = sr;

          print_operation("MOV",destination, source);
          if (simulate)
            {
              simulate_mov(registers, destination, source);
            }
        }
      else if ((ubyte)first_byte == MOV_SEGMENT_TO_REG_OR_MEM)
        {    
          byte second_byte = read_byte_using_ip(memory, registers);
          byte mod = mask ((second_byte >> 6), 0b00000011);
          byte sr = mask((second_byte >> 3), 0b00000011);
          byte rm = mask(second_byte, 0b00000111); 

          operand destination;
          operand source;
          
          fill_operand_by_mod(mod, rm, true, &cursor, &destination);

          source.type = OP_SEGMENT;
          source.sr = sr;

          print_operation("MOV",destination, source);
          if (simulate)
            {
              simulate_mov(registers, destination, source);
            }
        }
      else if ( 
        (ubyte)mask(first_byte, 0b11111100) == ADD_REG_OR_MEM_PLUS_REG_TO_EITHER
              )
        {
          byte second_byte = read_byte_using_ip(memory, registers);

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
          if (simulate)
            {
              simulate_arithmetic(registers, destination, source, ARITHMETIC_ADD);
            }
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == ARITHMETIC_IMMEDIATE_TO_REG_OR_MEM
              )
        {
          byte second_byte = read_byte_using_ip(memory, registers);

          byte s = mask(first_byte >> 1, 0b00000001);
          byte w = mask(first_byte >> 0, 0b00000001);
          
          byte mod =  mask(second_byte >> 6, 0b00000011);
          ARITHMETIC_TYPES type = 
            (ARITHMETIC_TYPES) mask(second_byte >> 3, 0b00000111);
          byte rm =   mask(second_byte >> 0, 0b00000111);

          operand rm_operand;
          operand immediate;

          fill_operand_by_mod(mod, rm, w, &cursor, &rm_operand);
          immediate.type = OP_IMMEDIATE;
          if (w && !s)
            {
              immediate.value = read_word_using_ip(memory, registers);
            }
          else 
            {
              immediate.value = read_byte_using_ip(memory, registers);
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
          if (simulate)
            {
              simulate_arithmetic(registers, rm_operand, immediate, type);
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
              immediate.value = read_word_using_ip(memory, registers);
            }
          else
            {
              immediate.value = read_byte_using_ip(memory, registers);
            }

          print_operation("ADD", accumulator, immediate);
          if (simulate)
            {
              simulate_arithmetic(registers, accumulator, immediate, ARITHMETIC_ADD);
            }
        }
      else if (
        (ubyte)mask(first_byte, 0b11111100) == SUB_REG_OR_MEM_SUB_REG_TO_EITHER
              )
        {
          byte second_byte = read_byte_using_ip(memory, registers);

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
          if (simulate)
            {
              simulate_arithmetic(registers, destination, source, ARITHMETIC_SUB);
            }

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
              immediate.value = read_word_using_ip(memory, registers);
            }
          else
            {
              immediate.value = read_byte_using_ip(memory, registers);
            }

          print_operation("SUB", accumulator, immediate);
          if (simulate)
            {
              simulate_arithmetic(registers, accumulator, immediate, ARITHMETIC_SUB);
            }

        }
      else if ((ubyte)mask(first_byte, 0b11111100) == CMP_REG_OR_MEM_AND_REG)
        {
          byte second_byte = read_byte_using_ip(memory, registers);

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
          if (simulate)
            {
              simulate_arithmetic(registers, destination, source, ARITHMETIC_CMP);
            }
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
              immediate.value = read_word_using_ip(memory, registers);
            }
          else
            {
              immediate.value = read_byte_using_ip(memory, registers);
            }

          print_operation("CMP", accumulator, immediate);
          if (simulate)
            {
              simulate_arithmetic(registers, accumulator, immediate, ARITHMETIC_CMP);
            }
        }
      else
        {
          printf ("; NOT IMPLEMENTED %s\n",byte_to_binary_string(first_byte));
        }

      if (print_every_intruction)
        {
          print_registers(registers);
          print_flags(registers);
          printf("\n");
        }
    }

  if (print_at_end)
    {
      print_registers(registers);
      print_flags(registers);
      printf("\n");
    }
  return 0;
}


