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
static const byte MOV_MEM_TO_ACCUMULATOR = 0b10100000;
static const byte MOV_ACCUMULATOR_TO_MEM = 0b10100010;

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

          byte d =          (first_byte >> 1) & 0b00000001;
          byte w =          (first_byte >> 0) & 0b00000001;

          byte mod =        (second_byte >> 6) & 0b00000011;
          byte reg =        (second_byte >> 3) & 0b00000111;
          byte rm =         (second_byte >> 0) & 0b00000111;
          ubyte destination = 0;
          ubyte source = 0;

          char destination_string[3] = "";
          char source_string[3] = "";

          if (mod == MOV_MOD_REG_TO_REG) // register to register move
            {
              // d bit
              if ( d == 1)
                {
                  destination = reg;
                  source = rm;
                }
              else 
                {
                  destination = rm;
                  source = reg;
                }

              // registers
              if ( w == 1)
                {
                  strcpy( destination_string, REGISTER_NAMES_WIDE[destination]);
                  strcpy( source_string, REGISTER_NAMES_WIDE[source]);
                }
              else
                {
                  strcpy( destination_string, REGISTER_NAMES[destination]);
                  strcpy( source_string, REGISTER_NAMES[source]);
                }

              printf ("mov %s, %s\n", 
                  destination_string, 
                  source_string);
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
    }

  fclose (fp);

  return 0;
}

