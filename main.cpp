#include <iostream>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "binary.h"
#include "register_rm.h"
#include "print.h"

#define KiB 1024
#define MiB (KiB * KiB)
#define GiB (KiB * KiB)

byte registers[8*2] = {0};
byte memory[MiB] = {0};

inline byte
get_low_byte (word value)
{
  word masked = value & 0b11111111;
  return masked;
}

inline byte
get_high_byte (word value)
{
  word shift = value >> 8;
  word masked = shift & 0b11111111;
  return masked;
}

inline word
read_value_from_register (byte *registers, operand reg)
{
  word value = 0;
  if (reg.type == OP_REGISTER)
    {
      if (reg.wide)
        {
          byte offset = REGISTER_OFFSET_WIDE[reg.reg];
          value = *(word*)(registers+offset);
        }
      else
        {
          byte offset = REGISTER_OFFSET[reg.reg];
          value = *(registers+offset);
        }
    }
  return value;
}

inline void 
write_value_to_register (byte *registers, operand reg, word value)
{
  if (reg.type == OP_REGISTER)
    {
      byte offset = REGISTER_OFFSET[reg.reg];

      byte lowValue = get_low_byte(value);
      byte highValue = get_high_byte(value);

      if (reg.wide)
        {
          offset = REGISTER_OFFSET_WIDE[reg.reg];
          registers[offset+1] = highValue;
        }

      registers[offset] = lowValue;
    }
}

void 
simulate_mov(operand destination, operand source)
{

  if (destination.type == OP_REGISTER)
    {
      if (source.type == OP_IMMEDIATE)
        {
          write_value_to_register(registers, destination, source.value);
       }
      else if (source.type == OP_REGISTER)
        {
          word value = read_value_from_register(registers, source);
          write_value_to_register(registers, destination, value);
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

  fread(&memory, sizeof(byte), MiB, fp);
  fclose(fp);

#if 0
  operand a, b;
  a.type = OP_REGISTER;
  a.wide = true;
  a.reg = 0;

  b.type = OP_IMMEDIATE;
  b.value = 257;

  simulate_mov(a, b);
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
                  word address = read_word(&cursor);
                  
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
              byte displacement = read_byte(&cursor);
              rm_operand.type = OP_MEMORY_LOCATION;
              rm_operand.rm = rm;
              rm_operand.wide = false;
              rm_operand.displacement = displacement;
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
            {
              word displacement = read_word(&cursor);
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
              simulate_mov(reg_operand, rm_operand);
            }
          else
            {
              print_mov(rm_operand, reg_operand);
              simulate_mov(rm_operand, reg_operand);
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
          simulate_mov(reg, addr);
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
          simulate_mov(addr, reg);
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
          simulate_mov(reg_operand, immediate);
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

          if (mod == MOV_MOD_REG_TO_REG) // register to register move
            {
              printf("; THIS SHOULD NEVER HAPPEN");
            }
          else if (mod == MOV_MOD_MEM_MODE)
            {
              if (w)
                {
                  word data = read_word(&cursor);
                  immediate.value = data;
                }
              else
                {
                  byte data = read_byte(&cursor);
                  immediate.value = data;
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_1)
            {
              byte displacement = read_byte(&cursor);
              destination.displacement = displacement;
              
              if (w)
                {
                  word data = read_word(&cursor);
                  immediate.value = data;
                }
              else
                {
                  byte data = read_byte(&cursor);
                  immediate.value = data;
                }
            }
          else if (mod == MOV_MOD_MEM_MODE_DISPLACE_2)
            {
              word displacement = read_word(&cursor);
              destination.displacement = displacement;
              
              if (w)
                {
                  word data = read_word(&cursor);
                  immediate.value = data;
                }
              else
                {
                  byte data = read_byte(&cursor);
                  immediate.value = data;
                }
            }
          else
            {
              printf ("; NOT IMPLEMENTED %s %s\n", 
                      byte_to_binary_string(first_byte),
                      byte_to_binary_string(second_byte));
            }
          print_mov(destination, immediate);
          simulate_mov(destination, immediate);

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


