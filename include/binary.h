#ifndef BINARY_H
#define BINARY_H

#include "types.h"
#include "architecture.h"

char* 
byte_to_binary_string(byte input);

inline byte
mask (byte byte_to_mask, byte mask)
{
  return byte_to_mask & mask; 
}

inline byte
read_byte (byte **cursor)
{
  byte value = *(byte*)(*cursor);
  (*cursor)++;
  return value;
}

inline word
read_word (byte **cursor)
{
  word value = *(word*)(*cursor);
  (*cursor)+=2;
  return value;
}

inline uword
read_uword (byte **cursor)
{
  uword value = *(uword*)(*cursor);
  (*cursor)+=2;
  return value;
}

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

inline byte 
read_ip (byte *registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  return *ip_address;
}

inline byte 
read_byte_using_ip (byte* base_ip, byte* registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  byte byte_at_ip = *(base_ip+(*ip_address));
  (*ip_address)++;
  return byte_at_ip;
}

inline word 
read_word_using_ip (byte* base_ip, byte* registers)
{
  word* ip_address = (word*)(registers+IP_OFFSET);
  word word_at_ip = *(word*)(base_ip+(*ip_address));
  (*ip_address)+=2;
  return word_at_ip;
}

#endif
