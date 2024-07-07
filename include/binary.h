#ifndef BINARY_H
#define BINARY_H

#include "types.h"

char* 
byte_to_binary_string(byte input);

inline byte
mask (byte byte_to_mask, byte mask)
{
  return byte_to_mask & mask; 
}

#endif
