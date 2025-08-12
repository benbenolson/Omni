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
#include "Epson_High_Res_ESCP2_Instance.hpp"
#include "defines.hpp"

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new Epson_High_Res_ESCP2_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

Epson_High_Res_ESCP2_Instance::
Epson_High_Res_ESCP2_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::Epson_High_Res_ESCP2_Instance ()" << std::endl;
#endif

   fHaveSetupPrinter_d = false;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
   fHaveInitialized_d  = false;
}

Epson_High_Res_ESCP2_Instance::
~Epson_High_Res_ESCP2_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::~Epson_High_Res_ESCP2_Instance ()" << std::endl;
#endif

}

void Epson_High_Res_ESCP2_Instance::
initializeInstance (PSZCRO pszJobProperties)
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

}

bool Epson_High_Res_ESCP2_Instance::
deviceOptionValid (PSZRO pszDeviceOption)
{
   if (0 == strcmp (pszDeviceOption, "USB_PORT_SUPPORT"))
   {
      return true;
   }

   return false;
}

void Epson_High_Res_ESCP2_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;


   DeviceResolution *pDR       = getCurrentResolution ();
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;
   DeviceData       *pData     = 0;  //@@PAZ
   int               iXRes;

   pData = getDeviceData ();  //@@PAZ

#ifndef RETAIL
   if(!pData) if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << ">>>> getDeviceData Failed " << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdBeginRasterGraphics defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetResolution");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd,
                          1440 / pDR->getYRes (),
                          1440 / pDR->getYRes (),
                          1440 / pDR->getXRes (), // Horiz Res (X).
                          1440);

   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetResolution defined for this device!" << std::endl;
#endif
   }

   DeviceForm *pForm = getCurrentForm ();

   pCmd = pCommands->getCommandData ("cmdSetFormSize");
   if (pCmd)
   {
      int iNozzles = 48;
      int iPageLength = 0 ;
      int iTemp;
      int iUnits = pForm->getHardCopyCap ()->getYPels ();
      float fHeight = (pForm->getHardCopyCap ()->getCy ()/25400.0)* pDR->getYRes ();

      // @TBD - for some reason the printer thinks that we are beyond the end
      // of the page if we have gone past the end of the page counting the bottom
      // line and the number of nozzles in the head.  To stop this, we are adding
      // the number of nozzles to the end of the page to stop the extra page eject

      if(pData)
         pData->getIntData ("Nozzle_Number", &iNozzles);
      iTemp = (iUnits+iNozzles*2) < (int)fHeight ? (iUnits+iNozzles*2) : (int)fHeight;

      iPageLength |= (iTemp & 0x00FF) << 8;
      iPageLength |= (iTemp & 0xFF00) >> 8;

      sendPrintfToDevice (pCmd, iPageLength);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetFormSize defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetPageSize");
   if (pCmd)
   {
      int iWidth, iHeight;
      iWidth = iHeight = 0;

      float fWidth = (pForm->getHardCopyCap ()->getCx ()/25400.0)* pDR->getXRes ();
      float fHeight = (pForm->getHardCopyCap ()->getCy ()/25400.0)* pDR->getYRes ();

      // flip the integer data around

      iWidth |= ((int)fWidth & 0x00FF) << 8 ;
      iWidth |= ((int)fWidth & 0xFF00) >> 8;
      iHeight |= ((int)fHeight & 0x00FF) << 8 ;
      iHeight |= ((int)fHeight & 0xFF00) >> 8;

//////sendPrintfToDevice (pCmd,
//////                    (int)fWidth,
//////                    (int)fHeight);

      sendPrintfToDevice (pCmd,
                          iWidth,
                          iHeight);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetPageSize defined for this device!" << std::endl;
#endif
   }

   // @TBD
   fUseMicroweave_d = false;

   if (  720 == pDR->getXRes ()
      && 720 == pDR->getYRes ()
      )
   {
      if (pDR->getDstBitsPerPel () == 1)
      {
#ifndef RETAIL
          if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "DstBitsPerPel = " << pDR->getDstBitsPerPel () << std::endl;
#endif
          fUseMicroweave_d = true;
      }
   }

   pCmd = pCommands->getCommandData ("cmdBidi");
   if (pCmd)
   {
      char chState;

      if (fUseMicroweave_d)
         // Turn off bidirectional for microweaving
         chState = '0';
      else
         chState = '1';
      sendPrintfToDevice (pCmd, chState);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdBidi defined for this device!" << std::endl;
#endif
   }

   // @TBD
///sendBinaryDataToDevice (getCurrentTray ());

   pCmd = pCommands->getCommandData ("cmdSetHardwareMicroweave");
   if (pCmd)
   {
      if (fUseMicroweave_d)
         sendPrintfToDevice (pCmd, 1);
      else
         sendPrintfToDevice (pCmd, 0);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetHardwareMicroweave defined for this device!" << std::endl;
#endif
   }

      iXRes = pDR->getXRes();

      if(pData)
      {
          int iDotValue;
          pCmd = pCommands->getCommandData ("cmdSetDotSizeInt");
          if(pCmd)
          {
              switch(iXRes)
              {
              case 1440:
                  pData->getIntData ("DotSize1440", &iDotValue);
                  sendPrintfToDevice (pCmd, iDotValue);
                  break;

              case 720:
                  pData->getIntData ("DotSize720", &iDotValue);
                  sendPrintfToDevice (pCmd, iDotValue);
                  break;

              case 360:
                  pData->getIntData ("DotSize360", &iDotValue);
                  sendPrintfToDevice (pCmd, iDotValue);
              }
          }
          else
          {
#ifndef RETAIL
              if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetDotSizeInt defined for this device!" << std::endl;
#endif
          }
      }
      else
      {
          pCmd = pCommands->getCommandData ("cmdSetDotSize");
          if(pCmd)
          {
              switch(iXRes)
              {
              case 1440:
                  sendPrintfToDevice (pCmd, 0x1000);
                  break;

              case 720:
                  sendPrintfToDevice (pCmd, 0x1100);
                  break;

              case 360:
                  sendPrintfToDevice (pCmd, 0x1200);
              }
          }
          else
          {
#ifndef RETAIL
              if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetDotSize defined for this device!" << std::endl;
#endif
          }
      }

   pCmd = pCommands->getCommandData ("cmdSetSpecialResolution");
   if (pCmd)
   {
      if(iXRes < 720)   // low resolution
      {
          sendPrintfToDevice (pCmd,
                              14400,
                              14400 / pDR->getYRes (),
                              14400 / iXRes );  //sets correct value for X positioning
      }
      else
      {
          int iNozzleValue = 2;
          int iXPos = 0;
          bool bRet = false;

          if(pData)
          {
              bRet =  pData->getIntData ("Nozzle_Spacing", &iNozzleValue);
              bRet =  pData->getIntData ("Positioning_x", &iXPos);
          }

          sendPrintfToDevice (pCmd,
                              14400,
                              (14400 / pDR->getYRes ())* iNozzleValue,
                              14400 / iXPos );  //sets correct value for X positioning
      }
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetDotSize defined for this device!" << std::endl;
#endif
   }
}

bool Epson_High_Res_ESCP2_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::beginJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdUSBInit");

   if (  hasDeviceOption ("USB_PORT_SUPPORT")
      && pCmd
      )
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdUSBInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdRemote");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdRemote = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool Epson_High_Res_ESCP2_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::beginJob (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return beginJob ();
}

bool Epson_High_Res_ESCP2_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::newFrame ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   ditherNewFrame ();

   ptlPrintHead_d.x = 0;
   ptlPrintHead_d.y = 0;

   return true;
}

bool Epson_High_Res_ESCP2_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::newFrame (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;
#endif

   // @TBD - reinitialize with new job properties

   // Call common code
   return newFrame ();
}

bool Epson_High_Res_ESCP2_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::endJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdTerm");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdTerm = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool Epson_High_Res_ESCP2_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Instance::endJob ()" << std::endl;
#endif

   int cx = getCurrentForm ()->getHardCopyCap ()->getXPels ();

   // The largest size is 24 lines
   int iSize = 24 * (cx + 7) / 8;

   PBYTE abData = new BYTE [iSize];

   memset (abData, 0, sizeof (abData));

   BinaryData data (abData, sizeof (abData));

   sendBinaryDataToDevice (&data);

   delete []abData;

   return true;
}

#ifndef RETAIL

void Epson_High_Res_ESCP2_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Epson_High_Res_ESCP2_Instance::
toString (std::ostringstream& oss)
{
   oss << "{Epson_High_Res_ESCP2_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Epson_High_Res_ESCP2_Instance& const_self)
{
   Epson_High_Res_ESCP2_Instance& self = const_cast<Epson_High_Res_ESCP2_Instance&>(const_self);
   std::ostringstream             oss;

   os << self.toString (oss);

   return os;
}
