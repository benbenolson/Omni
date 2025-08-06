/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "Capability.hpp"
#include "defines.hpp"

#include <cstring>

typedef struct _ReservedMap {
   const char *pszName;
   int         iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "COLOR",             0x0000000000000001 },
   { "MONOCHROME",        0x0000000000000002 },
   { "MIRROR",            0x0000000000000004 },
   { "HARDWARE_COPIES",   0x0000000000000008 }
};

bool Capability::
isReservedKeyword (const char *pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return true;
   }

   return false;
}

int Capability::
getReservedValue (const char *pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return vaReservedKeywords[i].iValue;
   }

   return 0;
}
