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
#include "DeviceGamma.hpp"
#include "DebugOutput.hpp"

DeviceGamma::
DeviceGamma (int iCGamma,
             int iMGamma,
             int iYGamma,
             int iKGamma,
             int iCBias,
             int iMBias,
             int iYBias,
             int iKBias)
{
   iCGamma_d = iCGamma;
   iMGamma_d = iMGamma;
   iYGamma_d = iYGamma;
   iKGamma_d = iKGamma;
   iCBias_d  = iCBias;
   iMBias_d  = iMBias;
   iYBias_d  = iYBias;
   iKBias_d  = iKBias;
}

int DeviceGamma:: getCGamma () { return iCGamma_d; }
int DeviceGamma:: getMGamma () { return iMGamma_d; }
int DeviceGamma:: getYGamma () { return iYGamma_d; }
int DeviceGamma:: getKGamma () { return iKGamma_d; }
int DeviceGamma:: getCBias  () { return iCBias_d;  }
int DeviceGamma:: getMBias  () { return iMBias_d;  }
int DeviceGamma:: getYBias  () { return iYBias_d;  }
int DeviceGamma:: getKBias  () { return iKBias_d;  }

#ifndef RETAIL

void DeviceGamma::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceGamma::
toString (std::ostringstream& oss)
{
   oss << "{DeviceGamma: "
       << "iCGamma_d = " << iCGamma_d
       << ", iMGamma_d = " << iMGamma_d
       << ", iYGamma_d = " << iYGamma_d
       << ", iKGamma_d = " << iKGamma_d
       << ", iCBias_d = " << iCBias_d
       << ", iMBias_d = " << iMBias_d
       << ", iYBias_d = " << iYBias_d
       << ", iKBias_d = " << iKBias_d
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceGamma& const_self)
{
   DeviceGamma&       self = const_cast<DeviceGamma&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
