#include <iostream>
#include <unistd.h>
#include <string.h>

typedef signed char byte;
typedef signed short word; 

typedef unsigned char ubyte;
typedef unsigned short uword; 

//  6 bit operations
static const byte MOV_ADR_TO_ADR = 0b10001000;
static const byte MOV_MOD_REG_TO_REG = 0b11;
static const byte MOV_MOD_MEM_MODE = 0b00;
//static const byte MOV_MOD_MEM_MODE_DISPLACE_1 = 0b01;
//static const byte MOV_MOD_MEM_MODE_DISPLACE_2 = 0b10;

static const byte MOV_MEM_TO_ACCUMULATOR = 0b10100000;
static const byte MOV_ACCUMULATOR_TO_MEM = 0b10100010;
static const byte MOV_IMMEDIATE_TO_REGISTER = 0b10110000;

/// 7 bit operations
//static const byte MOV_TO_ACCUMULATOR = 0b00100010;
//static const byte MOV_FROM_ACCUMULATOR = 0b00100010;

static const char* REGISTER_NAMES[] = 
{
  "AL",
  "CL",
  "DL",
  "BL",
  "AH",
  "CH",
  "DH",
  "BH"
};

static const char* REGISTER_NAMES_WIDE[] = 
{
  "AX",
  "CX",
  "DX",
  "BX",
  "SP",
  "BP",
  "SI",
  "DI"
};

static const char* RM_FIELD_NAMES[] = 
{
  "BX + SI",
  "BX + DI",
  "BP + SI",
  "BP + DI",
  "SI",
  "DI",
  "BP",
  "BX",
};

char* 
byte_to_binary_string(byte input)
{
  char* binary_string = (char*)malloc (sizeof(input)) + 1;
  ubyte mask = (byte)(1 << ((sizeof (input) * 8) - 1));
  unsigned int i = 0;

  for( i = 0 ; i < sizeof (input) * 8; i++)
    {
      if( input & mask )
        {
          binary_string[i] = '1';
        }
      else
        {
          binary_string[i] = '0';
        }
      mask = mask >> 1;
    }
  binary_string[i] = 0;

  return binary_string;
}

inline byte 
mask (byte byte_to_mask, byte mask)
{
  return byte_to_mask & mask; 
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
              // d bit
              if ( d == 1)
                {
                  if ( w == 1)
                    {
                      printf ("mov %s, %s\n", 
                          REGISTER_NAMES_WIDE[reg], 
                          REGISTER_NAMES_WIDE[rm]);
                    }
                  else
                    {
                      printf ("mov %s, %s\n", 
                          REGISTER_NAMES[reg], 
                          REGISTER_NAMES[rm]);
                    }
                }
              else 
                {
                  if ( w == 1)
                    {
                      printf ("mov %s, %s\n", 
                          REGISTER_NAMES_WIDE[rm], 
                          REGISTER_NAMES_WIDE[reg]);
                    }
                  else
                    {
                      printf ("mov %s, %s\n", 
                          REGISTER_NAMES[rm], 
                          REGISTER_NAMES[reg]);
                    }
                }


            }
          else if (mod == MOV_MOD_MEM_MODE)
            {
              if (d == 1) // register is destination (load)
                {
                  if (w == 1)
                    {
                      printf ("mov %s, [%s]\n", 
                              REGISTER_NAMES_WIDE[reg], 
                              RM_FIELD_NAMES[rm]);
                    }
                  else
                    {
                      printf ("mov %s, [%s]\n", 
                              REGISTER_NAMES[reg], 
                              RM_FIELD_NAMES[rm]);
                    }
                }
              else // register is source (store)
                {
                  if (w == 1)
                    {
                      printf ("mov [%s], %s\n", 
                              RM_FIELD_NAMES[rm], 
                              REGISTER_NAMES_WIDE[reg]);
                    }
                  else
                    {
                      printf ("mov [%s], %s\n", 
                              RM_FIELD_NAMES[rm],
                              REGISTER_NAMES_WIDE[reg]);
                    }
                }
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
          if (w) // wide memory read
            {
              printf("mov AX, [%i]\n", memory);
            }
          else
            {
              printf("mov AL, [%i]", memory);
            }
        }
      else if (mask(first_byte, 0b11111110) == MOV_ACCUMULATOR_TO_MEM)
        {
          uword memory;
          fread (&memory, sizeof(memory), 1, fp);

          byte w = first_byte & 1;
          if (w) // wide memory read
            {
              printf("mov [%i], AX\n", memory);
            }
          else
            {
              printf("mov [%i], AL", memory);
            }
        }
      else if (mask(first_byte, 0b11110000) == MOV_IMMEDIATE_TO_REGISTER)
        {
          byte w = (first_byte >> 3) & 1;
          byte reg = mask (first_byte, 0b00000111);
          
          if (w == 1)
            {
              word immediate_value;
              fread (&immediate_value, sizeof(immediate_value), 1, fp);
              
              printf("mov %s, [%i]\n", REGISTER_NAMES_WIDE[reg], immediate_value);
            }
          else
            {
              byte immediate_value;
              fread (&immediate_value, sizeof(immediate_value), 1, fp);
              
              printf("mov %s, [%i]\n", REGISTER_NAMES[reg], immediate_value);
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

