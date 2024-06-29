#include <iostream>
#include <unistd.h>

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

  char nextInstruction[2];
  while (fread(nextInstruction, 1, 2, fp)) 
    {
      printf("%i %i ", nextInstruction[0], nextInstruction[1]);
      
    }

  printf("\n");

  fclose(fp);
  return 0;
}
