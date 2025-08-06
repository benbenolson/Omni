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
#include "XMLDevice.hpp"
#include "XMLDeviceInstance.hpp"
#include "XMLDeviceBlitter.hpp"

XMLDeviceBlitter::
XMLDeviceBlitter (GModule     *hmodLibrary,
                  PrintDevice *pDevice)
   :DeviceBlitter (pDevice)
{
   hmodLibrary_d      = hmodLibrary;
   pfnCreateBlitter_d = 0;
   pfnDeleteBlitter_d = 0;
   pBlitter_d         = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceBlitter ()) DebugOutput::getErrorStream () << "XMLDeviceBlitter::" << __FUNCTION__ << ": hmodLibrary_d = " << hmodLibrary_d << std::endl;
#endif

   if (hmodLibrary_d)
   {
      int rc = 0;

      rc = ::g_module_symbol (hmodLibrary_d,
                              "createBlitter",
                              (void **)&pfnCreateBlitter_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceBlitter ()) DebugOutput::getErrorStream () << "XMLDeviceBlitter::" << __FUNCTION__ << ": pfnCreateBlitter_d = " << (void *)pfnCreateBlitter_d << std::endl;
#endif

      if (!rc)
      {
         std::cerr << "g_module_error returns " << g_module_error () << std::endl;
      }

      rc = ::g_module_symbol (hmodLibrary_d,
                              "deleteBlitter",
                              (void **)&pfnDeleteBlitter_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceBlitter ()) DebugOutput::getErrorStream () << "XMLDeviceBlitter::" << __FUNCTION__ << ": pfnDeleteBlitter_d = " << (void *)pfnDeleteBlitter_d << std::endl;
#endif

      if (!rc)
      {
         std::cerr << "g_module_error returns " << g_module_error () << std::endl;
      }

      if (  pfnCreateBlitter_d
         || pfnDeleteBlitter_d
         )
      {
         pBlitter_d = pfnCreateBlitter_d (pDevice);

#ifndef RETAIL
         if (DebugOutput::shouldOutputXMLDeviceBlitter ()) DebugOutput::getErrorStream () << "XMLDeviceBlitter::" << __FUNCTION__ << ": pBlitter_d = " << pBlitter_d << std::endl;
#endif
      }
   }
}

XMLDeviceBlitter::
~XMLDeviceBlitter ()
{
   if (pBlitter_d)
   {
      pfnDeleteBlitter_d (pBlitter_d);
   }

   if (hmodLibrary_d)
   {
      g_module_close (hmodLibrary_d);
      hmodLibrary_d = 0;
   }
}

void XMLDeviceBlitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputXMLDeviceBlitter ()) DebugOutput::getErrorStream () << "XMLDeviceBlitter::" << __FUNCTION__ << std::endl;
#endif

   if (pBlitter_d)
   {
      pBlitter_d->initializeInstance ();
   }
}

bool XMLDeviceBlitter::
hasError ()
{
   if (pBlitter_d)
      return pBlitter_d->hasError ();
   else
      return false;
}

bool XMLDeviceBlitter::
rasterize (PBYTE                   pbBits,
           PBITMAPINFO2            pbmi,
           PRECTL                  prectlPageLocation,
           BITBLT_TYPE             eType)
{
   if (pBlitter_d)
      return pBlitter_d->rasterize (pbBits,
                                    pbmi,
                                    prectlPageLocation,
                                    eType);
   else
      return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceBlitter * XMLDeviceBlitter::
getDeviceBlitter ()
{
   return pBlitter_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void XMLDeviceBlitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceBlitter::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceBlitter: "
       << "hmodLibrary_d = " << std::hex << hmodLibrary_d
       << ", pfnCreateBlitter_d = " << (int)pfnCreateBlitter_d
       << ", pfnDeleteBlitter_d = " << (int)pfnDeleteBlitter_d << std::dec
       << ", "
       << DeviceBlitter::toString (oss2)
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream&           os,
            const XMLDeviceBlitter& const_self)
{
   XMLDeviceBlitter&  self = const_cast<XMLDeviceBlitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
