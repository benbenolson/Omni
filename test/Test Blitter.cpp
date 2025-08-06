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
#include "Test_Blitter.hpp"
#include "Test_Instance.hpp"
#include "DeviceOrientation.hpp"

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Test_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Test_Blitter::
Test_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Test_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif
}

Test_Blitter::
~Test_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Test_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Test_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Test_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Test_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif
}

bool Test_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Test_Blitter::rasterize (0x"
           << (int)pbBits << ", 0x"
           << (int)pbmi2 << ", "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Test_Instance *pInstance = dynamic_cast <Test_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (pDevice_d->getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      break;
   }

   case DevicePrintMode::COLOR_TECH_CMYK:
   case DevicePrintMode::COLOR_TECH_CMY:
   case DevicePrintMode::COLOR_TECH_CcMmYK:
   {
      break;
   }

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec <<  "Error: unknown color tech " << pDevice_d->getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return false;
}

std::string Test_Blitter::
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
operator<< (std::ostream& os, const Test_Blitter& const_self)
{
   Test_Blitter&      self = const_cast<Test_Blitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
