#include <iostream>
#include <unistd.h>
#include <string.h>

typedef char unsigned byte;

static const byte MOV_OPERATION = 0b10001000;
//static const byte W_MASK = 0b00000001;

char* 
byte_to_binary_string(byte input)
{
  char* binary_string = (char*)malloc(sizeof(input)) + 1;
  byte mask = (byte)(1 << ((sizeof(input) * 8) - 1));
  unsigned int i = 0;

  for( i = 0 ; i < sizeof(input) * 8; i++)
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

int main(int argc, char** argv)
{
  if (argc != 2)
    {
      return 1;
    }

  char* filename = argv[1];
  FILE *fp = fopen(filename, "rb");

  if ( fp == NULL)
    {
      return 1;
    }

  byte next_instruction[2];
  char operation[4] = "";
  //char register_1[4] = "";
  //char register_2[4] = "";
  while (fread(next_instruction, sizeof(byte), sizeof(next_instruction), fp)) 
    {
      printf("mc: %s %s ", 
        byte_to_binary_string(next_instruction[0]), 
        byte_to_binary_string(next_instruction[1]));

      if ((next_instruction[0] & MOV_OPERATION) == MOV_OPERATION)
        {
          strcpy(operation, "mov");
        }

      printf(" to op: %s\n", operation);

    }

  fclose(fp);

  return 0;
}
