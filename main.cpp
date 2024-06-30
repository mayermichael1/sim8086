#include <iostream>
#include <unistd.h>
#include <string.h>

typedef char unsigned byte;

static const byte MOV_OPERATION = 0b00100010;
//static const byte W_MASK = 0b00000001;
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
  byte mask = (byte)(1 << ((sizeof (input) * 8) - 1));
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

  byte next_instruction[2];
  while (fread(next_instruction, sizeof(byte), sizeof(next_instruction), fp)) 
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

      byte operation =  (next_instruction[0] >> 2) & 0b00111111;
      byte d =          (next_instruction[0] >> 1) & 0b00000001;
      byte w =          (next_instruction[0] >> 0) & 0b00000001;
      byte mod =        (next_instruction[1] >> 6) & 0b00000011;
      byte reg =        (next_instruction[1] >> 3) & 0b00000111;
      byte rm =         (next_instruction[1] >> 0) & 0b00000111;
      byte destination = 0;
      byte source = 0;

      char operation_string[4] =  "";
      char destination_string[3] = "";
      char source_string[3] = "";

      // operation
      if (operation == MOV_OPERATION)
        {
          strcpy (operation_string, "mov");
          if ( mod != 0b11) 
            {
              continue; // skip for now because operation is not reg to reg
            }
        }

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

      printf ("%s %s, %s\n", operation_string, destination_string, source_string);

    }

  fclose (fp);

  return 0;
}
