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
#include "HP_DeskJet_PCL3GUI_Instance.hpp"
#include "defines.hpp"

#include <iostream>
#include <sstream>

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new HP_DeskJet_PCL3GUI_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

HP_DeskJet_PCL3GUI_Instance::
HP_DeskJet_PCL3GUI_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::HP_DeskJet_PCL3GUI_Instance ()" << std::endl;
#endif

   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
   eInternalRes_d      = IR_UNKNOWN;
}

HP_DeskJet_PCL3GUI_Instance::
~HP_DeskJet_PCL3GUI_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::~HP_DeskJet_PCL3GUI_Instance ()" << std::endl;
#endif
}

void HP_DeskJet_PCL3GUI_Instance::
initializeInstance ()
{
   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;

   DevicePrintMode *pDPM = getCurrentPrintMode ();

   switch (pDPM->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:    eInternalRes_d = IR_MONOCHROME; break;
   case DevicePrintMode::COLOR_TECH_CMY:  eInternalRes_d = IR_CMY;        break;
   case DevicePrintMode::COLOR_TECH_CMYK: eInternalRes_d = IR_CMYK;       break;
   }
}

void HP_DeskJet_PCL3GUI_Instance::
setupPrinter ()
{
   if (fHaveSetupPrinter_d)
      return;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::setupPrinter ()" << std::endl;
#endif

   fHaveSetupPrinter_d = true;

   DeviceResolution *pRes      = getCurrentResolution ();
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetUnitsOfMotion");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd, pRes->getXRes ());
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetUnitsOfMotion defined for this device!" << std::endl;
#endif
   }

   DeviceForm *pForm = getCurrentForm ();

   sendBinaryDataToDevice (pForm->getData ());

   sendPrintfToDevice (pRes->getData (), pRes->getXRes ());

   // @TBD Shingle / Depletion

   pCmd = pCommands->getCommandData ("cmdSetTopMargin");
   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetTopMargin defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetXYPos");
   if (pCmd)
   {
      sendPrintfToDevice (pCmd, 0, 0);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetXYPos defined for this device!" << std::endl;
#endif
   }

   configureRasterData (eInternalRes_d);
}

PSZRO HP_DeskJet_PCL3GUI_Instance::
getInternalResConfiguration (INTERNALRES eInternalRes)
{
   static PSZRO aszEntries[] = {
      "C",   // IR_MONOCHROME
      "CCC", // IR_CMY
      "CCCC" // IR_CMYK
   };

   if (  0 >= eInternalRes
      || eInternalRes > (int)dimof (aszEntries)
      )
      return 0;

   return aszEntries[eInternalRes - IR_MONOCHROME];
}

void HP_DeskJet_PCL3GUI_Instance::
configureRasterData (INTERNALRES eInternalRes)
{
   DeviceCommand    *pCommands  = getCommands ();
   BinaryData       *pCmdHeader = 0;
   BinaryData       *pCmdParam1 = 0;
   BinaryData       *pCmdParam2 = 0;
   DeviceResolution *pDR        = getCurrentResolution ();
   int               iXRes      = pDR->getXRes ();
   int               iYRes      = pDR->getYRes ();
   PSZRO             pszPlane  = 0;

   pszPlane = getInternalResConfiguration (eInternalRes);

   pCmdHeader = pCommands->getCommandData ("cmdConfigureRasterDataHeader");
   pCmdParam1 = pCommands->getCommandData ("cmdConfigureRasterDataParam1");
   pCmdParam2 = pCommands->getCommandData ("cmdConfigureRasterDataParam2");

   sendPrintfToDevice (pCmdHeader, 2 + strlen (pszPlane) * 6);
   sendPrintfToDevice (pCmdParam1,
                       2,                  // format
                       strlen (pszPlane)); // # planes

   while (*pszPlane)
   {
      if ('C' == *pszPlane)
      {
         sendPrintfToDevice (pCmdParam2,
                             iXRes,
                             iYRes,
                             2);
      }
      else if ('c' == *pszPlane)
      {
         sendPrintfToDevice (pCmdParam2,
                             iXRes / 2,
                             iYRes / 2,
                             4);
      }

      pszPlane++;
   }
}

bool HP_DeskJet_PCL3GUI_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::beginJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdEnterLanguage");
   if (pCmd)
   {
      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdSetUnitsOfMotion defined for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "cmdInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool HP_DeskJet_PCL3GUI_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::beginJob (with props)" << std::endl;
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

bool HP_DeskJet_PCL3GUI_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::newFrame ()" << std::endl;
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

bool HP_DeskJet_PCL3GUI_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::newFrame (with props)" << std::endl;
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

bool HP_DeskJet_PCL3GUI_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::endJob ()" << std::endl;
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

bool HP_DeskJet_PCL3GUI_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "HP_DeskJet_PCL3GUI_Instance::endJob ()" << std::endl;
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

void HP_DeskJet_PCL3GUI_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string HP_DeskJet_PCL3GUI_Instance::
toString (std::ostringstream& oss)
{
   oss << "{HP_DeskJet_PCL3GUI_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const HP_DeskJet_PCL3GUI_Instance& const_self)
{
   HP_DeskJet_PCL3GUI_Instance& self = const_cast<HP_DeskJet_PCL3GUI_Instance&>(const_self);
   std::ostringstream           oss;

   os << self.toString (oss);

   return os;
}
