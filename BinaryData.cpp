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
#include "BinaryData.hpp"
#include "DebugOutput.hpp"

#include <ctype.h>

BinaryData::
BinaryData (PBYTE pbData, int iSize)
{
   pbData_d   = pbData;
   iSize_d    = iSize;
   fOutType_d = true;
}

BinaryData::
~BinaryData ()
{
}

PBYTE BinaryData::
getData ()
{
   return pbData_d;
}

int BinaryData::
getLength ()
{
   return iSize_d;
}

int BinaryData::
getPrintfLength ()
{
   int iSize = 0;

   for (int i = 0; i < iSize_d; i++)
   {
      if (  '%' == pbData_d[i]
         && '%' != pbData_d[i+1]
         )
      {
         switch (pbData_d[i+1])
         {
         case 'd':
         case 'D':
            iSize += 4;
            break;
         case 'w':
         case 'W':
            iSize += 2;
            break;
         case 'c':
         case 'C':
            iSize += 1;
            break;
         case 'f':
         case 'F':
         case 'n':
         case 'N':
         default:
            return -1;
         }
         i++;
      }
      else if (  '%' == pbData_d[i]
              && '%' == pbData_d[i+1]
              )
      {
         iSize++;
         i++;
      }
      else
      {
         iSize++;
      }
   }

   return iSize;
}

void BinaryData::
setData (PBYTE pbNewData)
{
   pbData_d = pbNewData;
}

void BinaryData::
setLength (int iNewLength)
{
   iSize_d = iNewLength;
}

void BinaryData::
setDumpString ()
{
   fOutType_d = true;
}

void BinaryData::
setDumpHex ()
{
   fOutType_d = false;
}

#ifndef RETAIL

void BinaryData::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string BinaryData::
toString (std::ostringstream& oss)
{
   oss << "{BinaryData: iSize_d = "
       << std::hex
       << iSize_d
       << ", {";

   for (int i = 0; i < iSize_d; i++)
   {
      if (  fOutType_d
         && isprint (pbData_d[i])
         )
      {
         oss << '\'' << pbData_d[i] << '\'';
      }
      else
      {
         oss << "0x" << std::hex;
         oss.width (2);
         oss.fill ('0');
         oss << (int)pbData_d[i];
      }

      if (i < iSize_d - 1)
         oss << ",";
   }

   oss << std::dec << "}}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const BinaryData& const_self)
{
   BinaryData&        self = const_cast<BinaryData&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

BinaryDataDelete::
BinaryDataDelete (PBYTE pbData, int iSize)
   : BinaryData (pbData, iSize)
{
   pbData_d = pbData;
}

BinaryDataDelete::
~BinaryDataDelete ()
{
   delete[] pbData_d;
}

#ifndef RETAIL

void BinaryDataDelete::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string BinaryDataDelete::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{BinaryDataDelete: "
       << BinaryData::toString (oss2) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const BinaryDataDelete& const_self)
{
   BinaryDataDelete&  self = const_cast<BinaryDataDelete&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
