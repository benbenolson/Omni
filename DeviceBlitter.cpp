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
#include "Device.hpp"
#include <cstdint>

DeviceBlitter::
DeviceBlitter (PrintDevice *pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceBlitter ()) DebugOutput::getErrorStream () << "DeviceBlitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   pDevice_d              = pDevice;
   pDitherInstance_d      = 0;
   pCompressionInstance_d = 0;
}

DeviceBlitter::
~DeviceBlitter ()
{
   if (pDitherInstance_d)
   {
      delete pDitherInstance_d;
      pDitherInstance_d = 0;
   }

   if (pCompressionInstance_d)
   {
      delete pCompressionInstance_d;
      pCompressionInstance_d = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceBlitter ()) DebugOutput::getErrorStream () << "DeviceBlitter::~" << __FUNCTION__ << " ()" << std::endl;
#endif
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceBlitter::
initializeInstance ()
{
}

bool DeviceBlitter::
hasError ()
{
   return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceInstance * DeviceBlitter::
getInstance ()
{
   DeviceInstance *pInstance = 0;

   if (pDevice_d)
      pInstance = pDevice_d->getDeviceInstance ();

   return pInstance;
}

DeviceDither * DeviceBlitter::
getDitherInstance ()
{
   return pDitherInstance_d;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceBlitter::
setDitherInstance (DeviceDither *pDitherInstance)
{
   pDitherInstance_d = pDitherInstance;
}

void DeviceBlitter::
setCompressionInstance (GplCompression *pCompressionInstance)
{
   pCompressionInstance_d = pCompressionInstance;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceBlitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
   // @TBD - default implementation
   return false;
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

DeviceOrientation * DeviceBlitter::
getCurrentOrientation ()
{
   return pDevice_d->getCurrentOrientation ();
}

PSZCRO DeviceBlitter::
getCurrentDitherID ()
{
   return pDevice_d->getCurrentDitherID ();
}

DeviceForm * DeviceBlitter::
getCurrentForm ()
{
   return pDevice_d->getCurrentForm ();
}

DeviceTray * DeviceBlitter::
getCurrentTray ()
{
   return pDevice_d->getCurrentTray ();
}

DeviceMedia * DeviceBlitter::
getCurrentMedia ()
{
   return pDevice_d->getCurrentMedia ();
}

DeviceResolution * DeviceBlitter::
getCurrentResolution ()
{
   return pDevice_d->getCurrentResolution ();
}

DeviceCommand * DeviceBlitter::
getCommands ()
{
   return pDevice_d->getCommands ();
}

DeviceData * DeviceBlitter::
getDeviceData ()
{
   return pDevice_d->getDeviceData ();
}

DevicePrintMode * DeviceBlitter::
getCurrentPrintMode ()
{
   return pDevice_d->getCurrentPrintMode ();
}

DeviceGamma * DeviceBlitter::
getCurrentGamma ()
{
   return pDevice_d->getCurrentGamma ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceBlitter::
ditherRGBtoCMYK (PBITMAPINFO2 pbmi2, PBYTE pbStart)
{
   if (pDitherInstance_d)
      pDitherInstance_d->ditherRGBtoCMYK (pbmi2, pbStart);
}

bool DeviceBlitter::
ditherAllPlanesBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherAllPlanesBlank ();
   else
      return false;
}

bool DeviceBlitter::
ditherCPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherCPlaneBlank ();
   else
      return true;
}

bool DeviceBlitter::
ditherMPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherMPlaneBlank ();
   else
      return true;
}

bool DeviceBlitter::
ditherYPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherYPlaneBlank ();
   else
      return true;
}

bool DeviceBlitter::
ditherKPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherKPlaneBlank ();
   else
      return true;
}

bool DeviceBlitter::
ditherLMPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherLMPlaneBlank ();
   else
      return true;
}

bool DeviceBlitter::
ditherLCPlaneBlank ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->ditherLCPlaneBlank ();
   else
      return true;
}

BinaryData * DeviceBlitter::
getCPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getCPlane ();
   else
      return new BinaryData (0, 0);
}

BinaryData * DeviceBlitter::
getMPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getMPlane ();
   else
      return new BinaryData (0, 0);
}

BinaryData * DeviceBlitter::
getYPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getYPlane ();
   else
      return new BinaryData (0, 0);
}

BinaryData * DeviceBlitter::
getKPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getKPlane ();
   else
      return new BinaryData (0, 0);
}

BinaryData * DeviceBlitter::
getLCPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getLCPlane ();
   else
      return new BinaryData (0, 0);
}

BinaryData * DeviceBlitter::
getLMPlane ()
{
   if (pDitherInstance_d)
      return pDitherInstance_d->getLMPlane ();
   else
      return new BinaryData (0, 0);
}

void DeviceBlitter::
ditherNewFrame ()
{
   if (pDitherInstance_d)
      pDitherInstance_d->newFrame ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

void DeviceBlitter::
resetBlankLineCount ()
{
   if (pCompressionInstance_d)
      pCompressionInstance_d->resetBlankLineCount ();
}

void DeviceBlitter::
incrementBlankLineCount ()
{
   if (pCompressionInstance_d)
      pCompressionInstance_d->incrementBlankLineCount ();
}

void DeviceBlitter::
incrementBlankLineCount (int iBlankLines)
{
   if (pCompressionInstance_d)
      pCompressionInstance_d->incrementBlankLineCount (iBlankLines);
}

int DeviceBlitter::
getBlankLineCount ()
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->getBlankLineCount ();
   else
      return 0;
}

void DeviceBlitter::
resetCompressionMode ()
{
   if (pCompressionInstance_d)
      pCompressionInstance_d->resetCompressionMode ();
}

bool DeviceBlitter::
isCurrentCompressionMode (int iMode)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->isCurrentCompressionMode (iMode);
   else
      return false;
}

BinaryData * DeviceBlitter::
compressKRasterPlane (BinaryData *pbdKPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressKRasterPlane (pbdKPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressCRasterPlane (BinaryData *pbdCPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressCRasterPlane (pbdCPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressLCRasterPlane (BinaryData *pbdLCPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressLCRasterPlane (pbdLCPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressMRasterPlane (BinaryData *pbdMPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressMRasterPlane (pbdMPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressLMRasterPlane (BinaryData *pbdLMPlane)        //Canon6
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressLMRasterPlane (pbdLMPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressYRasterPlane (BinaryData *pbdYPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressYRasterPlane (pbdYPlane);
   else
      return 0;
}

BinaryData * DeviceBlitter::
compressRGBRasterPlane (BinaryData *pbdRGBPlane)
{
   if (pCompressionInstance_d)
      return pCompressionInstance_d->compressRGBRasterPlane (pbdRGBPlane);
   else
      return 0;
}

void DeviceBlitter::
clearLastLineBuffers ()
{
   if (pCompressionInstance_d)
      pCompressionInstance_d->clearLastLineBuffers ();
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

bool DeviceBlitter::
sendBinaryDataToDevice (BinaryData *pData)
{
   return pDevice_d->sendBinaryDataToDevice (pData);
}

bool DeviceBlitter::
sendBinaryDataToDevice (DeviceForm *pForm)
{
   return pDevice_d->sendBinaryDataToDevice (pForm);
}

bool DeviceBlitter::
sendBinaryDataToDevice (DeviceTray *pTray)
{
   return pDevice_d->sendBinaryDataToDevice (pTray);
}

bool DeviceBlitter::
sendBinaryDataToDevice (DeviceMedia *pMedia)
{
   return pDevice_d->sendBinaryDataToDevice (pMedia);
}

bool DeviceBlitter::
sendBinaryDataToDevice (DeviceResolution *pResolution)
{
   return pDevice_d->sendBinaryDataToDevice (pResolution);
}

bool DeviceBlitter::
sendBinaryDataToDevice (PBYTE pbData,
                        int   iLength)
{
   return pDevice_d->sendBinaryDataToDevice (pbData, iLength);
}

bool DeviceBlitter::
sendPrintfToDevice (BinaryData *pData,
                                ...)
{
   va_list list;
   bool    rc;

   va_start (list, pData);

   rc = sendVPrintfToDevice (pData, list);

   va_end (list);

   return rc;
}

bool DeviceBlitter::
sendVPrintfToDevice (BinaryData *pData,
                     va_list     list)
{
   return pDevice_d->sendVPrintfToDevice (pData, list);
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void DeviceBlitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceBlitter::
toString (std::ostringstream& oss)
{
   oss << "{DeviceBlitter: pDevice_d = "
       << std::hex
       << (uintptr_t)pDevice_d
       << ", pDitherInstance_d = "
       << (uintptr_t)pDitherInstance_d
       << ", pCompressionInstance_d = "
       << (uintptr_t)pCompressionInstance_d
       << std::dec
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceBlitter& const_self)
{
   DeviceBlitter&      self = const_cast<DeviceBlitter&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
