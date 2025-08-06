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
#include "PluggableBlitter.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

PluggableBlitter::
PluggableBlitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "PluggableBlitter::" << __FUNCTION__ << " ()" << std::endl;
#endif
}

PluggableBlitter::
~PluggableBlitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "PluggableBlitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "PluggableBlitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void PluggableBlitter::
initializeInstance ()
{
}

bool PluggableBlitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream ()
           << "PluggableBlitter::rasterize (0x"
           << std::hex << (int)pbBits << std::dec << ", {"
           << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   PluggableInstance *pInstance;

   pInstance = dynamic_cast <PluggableInstance *>(getInstance ());

   if (pInstance)
   {
      return pInstance->rasterize (pbBits,
                                   pbmi2,
                                   prectlPageLocation,
                                   eType);
   }
   else
   {
      return false;
   }
}

std::string PluggableBlitter::
toString (std::ostringstream& oss)
{
   oss << "{ "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const PluggableBlitter& const_self)
{
   PluggableBlitter&  self = const_cast<PluggableBlitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
