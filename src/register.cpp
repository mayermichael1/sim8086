#include "register.h"

void 
print_mov_register_to_register (byte reg, byte rm, bool destination, bool wide)
{ 
  byte destination_register;
  byte source_register;
  if ( destination)
    {
      destination_register = reg;
      source_register = rm;
    }
  else
    {
      destination_register = rm;
      source_register = reg;
    }

  printf ("mov %s, %s\n", 
      get_register_name (destination_register, wide), 
      get_register_name (source_register, wide));
}

void 
print_mov_reg_and_mem ( byte reg, byte rm, bool destination, bool wide, 
                        int displacement)
{
  if ( displacement != 0 )
    {
      char sign = '+';
      if (displacement < 0)
        {
          sign = '-';
          displacement *= -1;
        }

      if (destination) // register is destination (load)
        {
          printf ("mov %s, [%s %c %u]\n", 
                  get_register_name (reg, wide), 
                  RM_FIELD_NAMES[rm],
                  sign,
                  displacement);
        }
      else // register is source (store)
        {
          printf ("mov [%s %c %u], %s\n", 
                  RM_FIELD_NAMES[rm],
                  sign,
                  displacement,
                  get_register_name (reg, wide));
        }
    }
  else
    {
      if (destination) // register is destination (load)
        {
          printf ("mov %s, [%s]\n", 
                  get_register_name (reg, wide), 
                  RM_FIELD_NAMES[rm]);
        }
      else // register is source (store)
        {
          printf ("mov [%s], %s\n", 
                  RM_FIELD_NAMES[rm],
                  get_register_name (reg, wide));
        }
    }
}

