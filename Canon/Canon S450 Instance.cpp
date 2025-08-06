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
#include "Canon_S450_Instance.hpp"
#include "defines.hpp"

#include <iostream>
#include <sstream>

const static bool fTestNoCompression = true;

typedef enum {
   CANON_MEDIA_ID_PLAIN     =  0,
   CANON_MEDIA_ID_COATED    =  1,
   CANON_MEDIA_ID_TRANS     =  2,
   CANON_MEDIA_ID_BACKPRINT =  3,
   CANON_MEDIA_ID_CLOTH     =  4,
   CANON_MEDIA_ID_GLOSSY    =  5,
   CANON_MEDIA_ID_GLOSSFILM =  6,
   CANON_MEDIA_ID_HIGHRES   =  7,
   CANON_MEDIA_ID_ENVELOPE  =  8,
   CANON_MEDIA_ID_POSTCARD  =  9,
   CANON_MEDIA_ID_THICK     =  9,
   CANON_MEDIA_ID_FULLXXXXX = 10,
   CANON_MEDIA_ID_BANNER    = 11,
   CANON_MEDIA_ID_OTHER     = 15
} CANON_MEDIA_ID;

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new Canon_S450_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

Canon_S450_Instance::
Canon_S450_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::Canon_S450_Instance ()" << std::endl;
#endif

   ptlPrintHead_d.x    = 0;
   ptlPrintHead_d.y    = 0;
   fHaveInitialized_d  = false;
   fHaveSetupPrinter_d = false;
}

Canon_S450_Instance::
~Canon_S450_Instance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::~Canon_S450_Instance ()" << std::endl;
#endif
}

void Canon_S450_Instance::
initializeInstance (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::initializeInstance ()" << std::endl;
#endif

   if (fHaveInitialized_d)
      return;

   fHaveInitialized_d = true;
}

void Canon_S450_Instance::
setupPrinter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter ()" << std::endl;
#endif

   if (fHaveSetupPrinter_d)
      return;

   fHaveSetupPrinter_d = true;

   setPrintColor ();

   DeviceCommand *pCommands = getCommands ();
   DeviceData    *pData     = getDeviceData ();
   BinaryData    *pCmd      = 0;

   /* The default is:
   **    <deviceData name="cmdSetPageMode" type="binary">_ESC_ "(a" HEX2S(00,01) HEX(01)</deviceData>
   */
   if (pData)
   {
      // search for: <deviceData name="cmdSetPageMode" type="binary">_ESC_ "(a" HEX2S(00,01) HEX(01)</deviceData>
      if (pData->getBinaryData ("cmdSetPageMode", &pCmd))
      {
         sendBinaryDataToDevice (pCmd);
      }
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter () Error: There is no device data for this device!" << std::endl;
#endif
   }

   pCmd = pCommands->getCommandData ("cmdSetPageID");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter () cmdPageID = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter () Error: There is no cmdPageID defined for this device!" << std::endl;
#endif
   }

   HardCopyCap *pHCC = getCurrentForm ()->getHardCopyCap ();
   int          cx   = pHCC->getCx ();
   int          cy   = pHCC->getCy ();

   /* The default is:
   **    <deviceData name="cmdSetPageMargins" type="binary">_ESC_ "(g" HEX2S(00,03) "%c" HEX(01) "%c"</deviceData>
   **    <deviceData name="minPageLength" type="integer">220</deviceData>
   **    <deviceData name="minRightMargin" type="integer">80</deviceData>
   */
   if (pData)
   {
      // search for: <deviceData name="cmdSetPageMargins" type="binary">_ESC_ "(g" HEX2S(00,03) "%c" HEX(01) "%c"</deviceData>
      if (pData->getBinaryData ("cmdSetPageMargins", &pCmd))
      {
         // convert from hundreds of a millimeter to tens of an inch
         int iPageLength     = cy / 254;
         int iRightMargin    = cx / 254;
         int iMinPageLength  = 0;
         int iMinRightMargin = 0;

         if (  pData->getIntData ("minPageLength", &iMinPageLength)
            && pData->getIntData ("minRightMargin", &iMinRightMargin)
            )
         {
            sendPrintfToDevice (pCmd,
                                std::min (iMinPageLength, iPageLength),
                                std::min (iMinRightMargin, iRightMargin));
         }
         else
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter: Error: could not find minPageLength or minPageLength" << std::endl;
#endif
         }
      }
      // search for: <deviceData name="cmdSetPageMargins2" type="binary">_ESC_ "(p" HEX2S(00,08) "%d%d"</deviceData>
      else if (pData->getBinaryData ("cmdSetPageMargins2", &pCmd))
      {
         // convert from hundreds of a millimeter to sixtys of an inch
         int iPageLength     = (6 * cy) / 254;
         int iRightMargin    = (6 * cx) / 254;
         int iMaxRightMargin = 0;

         // search for: <deviceData name="maxRightMargin" type="integer">480</deviceData>
         if (pData->getIntData ("maxRightMargin", &iMaxRightMargin))
         {
            sendPrintfToDevice (pCmd,
                                std::min (1380, iPageLength),
                                std::min (iMaxRightMargin, iRightMargin));
         }
         else
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter: Error: could not find maxRightMargin" << std::endl;
#endif
         }
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter: Error: could not find cmdSetPageMargins or cmdSetPageMargins2" << std::endl;
#endif
      }
   }

   DeviceResolution *pDR = getCurrentResolution ();

   // Select the resolution
   sendBinaryDataToDevice (pDR);

   /* The default is:
   **    <deviceData name="cmdSetImage" type="binary">_ESC_ "(t" HEX2S(00,03) "%c" HEX(00) "%c"</deviceData>
   **    <deviceData name="setImageParm1" type="integer">1</deviceData>
   **    <deviceData name="setImageParm2" type="integer">0</deviceData>
   */
   if (pData)
   {
      bool fHackCmdSetImage = false;
      byte bParm1           = 0;
      byte bParm2           = 0;
      byte bParm3           = 0;

      // search for: <deviceData name="hackCmdSetImage" type="boolean">true</deviceData>
      pData->getBooleanData ("hackCmdSetImage", &fHackCmdSetImage);

      // search for: <deviceData name="cmdSetImage" type="binary">_ESC_ "(t" HEX2S(00,03) "%c" HEX(00) "%c"</deviceData>
      if (  pData->getBinaryData ("cmdSetImage", &pCmd) )
      {
         if (pDR->getDstBitsPerPel()==2) {
            bParm1=0x02; bParm2=0x80;
         } else if (pDR->getXRes() == 1440 ) {
            // enable offset transmission for 1440
            bParm1=0x01; bParm2=0x04;
         } else {
            bParm1=0x01; bParm2=0x00;
         }
         if ( pDR->getYRes()==720 && pDR->getXRes() == 720 ) {
            bParm3=0x09;
         } else if (pDR->getYRes()==180 && pDR->getXRes()==180) {
            bParm3=0x01;
         } else {
               bParm3=0x09;
         }

         sendPrintfToDevice (pCmd,bParm1,bParm2,bParm3);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setupPrinter: Error: could not find cmdSetImage or ..." << std::endl;
#endif
      }
   }

   DeviceTray *pDT = getCurrentTray ();

   if (pDR->getXRes() == 1440 )
   {
      pCmd = pCommands->getCommandData ("cmdSkipNumLines");
      if (pCmd)
         sendPrintfToDevice(pCmd,0x0071);
      else
         DebugOutput::getErrorStream () << "Couldn't find cmdSkipNumLines\n";
   }

   /* The default is:
   **    The command attached to the tray
   */
   if (pData)
   {
      // search for: <deviceData name="cmdSetTray" type="binary">_ESC_ "(l" HEX2S(00,02) "%c%c"</deviceData>
      if (pData->getBinaryData ("cmdSetTray", &pCmd))
      {
         BinaryData  *pBD          = pDT->getData ();
         std::string *pstringMedia = 0;
         byte         bParm1       = 32 + pBD->getData ()[5]; // @TBD UGLY HACK
         byte         bParm2       = 0;

         pstringMedia = getCurrentMedia ()->getMedia ();

         if (pstringMedia)
         {
            if (0 == pstringMedia->compare ("MEDIA_PLAIN"))
            {
               bParm2 = CANON_MEDIA_ID_PLAIN;
            }
            else if (0 == pstringMedia->compare ("MEDIA_COATED"))
            {
               bParm2 = CANON_MEDIA_ID_COATED;
            }
            else if (0 == pstringMedia->compare ("MEDIA_TRANSPARENCY"))
            {
               bParm2 = CANON_MEDIA_ID_TRANS;
            }
            else if (0 == pstringMedia->compare ("MEDIA_BACKPRINT"))
            {
               bParm2 = CANON_MEDIA_ID_BACKPRINT;
            }
            else if (0 == pstringMedia->compare ("MEDIA_CLOTH"))
            {
               bParm2 = CANON_MEDIA_ID_CLOTH;
            }
            else if (0 == pstringMedia->compare ("MEDIA_GLOSSY"))
            {
               bParm2 = CANON_MEDIA_ID_GLOSSY;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_GLOSS_FILM"))
            {
               bParm2 = CANON_MEDIA_ID_GLOSSFILM;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_RESOLUTION"))
            {
               bParm2 = CANON_MEDIA_ID_HIGHRES;
            }
            else if (0 == pstringMedia->compare ("MEDIA_ENVELOPE"))
            {
               bParm2 = CANON_MEDIA_ID_ENVELOPE;
            }
            else if (0 == pstringMedia->compare ("MEDIA_POSTCARD"))
            {
               bParm2 = CANON_MEDIA_ID_POSTCARD;
            }
            else if (0 == pstringMedia->compare ("MEDIA_THICK"))
            {
               bParm2 = CANON_MEDIA_ID_THICK;
            }
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm2 = CANON_MEDIA_ID_FULLXXXXX;
////////////}
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm2 = CANON_MEDIA_ID_BANNER;
////////////}
            else if (0 == pstringMedia->compare ("MEDIA_OTHER"))
            {
               bParm2 = CANON_MEDIA_ID_OTHER;
            }

            delete pstringMedia;

            sendPrintfToDevice (pCmd,
                                bParm1,
                                bParm2);
         }
      }
      else
      {
         sendBinaryDataToDevice (pDT);
      }
   }
   else
   {
      sendBinaryDataToDevice (pDT);
   }
}

void Canon_S450_Instance::
setPrintColor ()
{
   DevicePrintMode *pDPM  = getCurrentPrintMode ();
   DeviceData      *pData = getDeviceData ();
   BinaryData      *pCmd  = 0;

   if (!pData)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setPrintColor () Error: There is no device data for this device!" << std::endl;
#endif

      return;
   }

   bool fHackCmdSetMono = false;

   // search for: <deviceData name="hackCmdSetMono" type="boolean">true</deviceData>
   pData->getBooleanData ("hackCmdSetMono", &fHackCmdSetMono);

   if (DevicePrintMode::COLOR_TECH_K == pDPM->getColorTech ())
   {
      // Monochrome default
      // search for: <deviceData name="cmdSetMono" type="binary">_ESC_ "(c" HEX2S(00,01) HEX(01)</deviceData>
      if (pData->getBinaryData ("cmdSetMono", &pCmd))
      {
         sendBinaryDataToDevice (pCmd);
      }
      // search for: <deviceData name="cmdSetMono2" type="binary">_ESC_ "(c" HEX2S(00,03) "%c%c%c"</deviceData>
      else if (pData->getBinaryData ("cmdSetMono2", &pCmd))
      {
         DeviceResolution *pDR          = getCurrentResolution ();
         std::string      *pstringMedia = 0;
         byte              bParm1       = 49;
         byte              bParm2       = 0;
         byte              bParm3       = 0;

         pstringMedia = getCurrentMedia ()->getMedia ();

         if (pstringMedia)
         {
            if (0 == pstringMedia->compare ("MEDIA_PLAIN"))
            {
               bParm2 = CANON_MEDIA_ID_PLAIN;
            }
            else if (0 == pstringMedia->compare ("MEDIA_COATED"))
            {
               bParm2 = CANON_MEDIA_ID_COATED;
            }
            else if (0 == pstringMedia->compare ("MEDIA_TRANSPARENCY"))
            {
               bParm2 = CANON_MEDIA_ID_TRANS;
            }
            else if (0 == pstringMedia->compare ("MEDIA_BACKPRINT"))
            {
               bParm2 = CANON_MEDIA_ID_BACKPRINT;
            }
            else if (0 == pstringMedia->compare ("MEDIA_CLOTH"))
            {
               bParm2 = CANON_MEDIA_ID_CLOTH;
            }
            else if (0 == pstringMedia->compare ("MEDIA_GLOSSY"))
            {
               bParm2 = CANON_MEDIA_ID_GLOSSY;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_GLOSS_FILM"))
            {
               bParm2 = CANON_MEDIA_ID_GLOSSFILM;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_RESOLUTION"))
            {
               bParm2 = CANON_MEDIA_ID_HIGHRES;
            }
            else if (0 == pstringMedia->compare ("MEDIA_ENVELOPE"))
            {
               bParm2 = CANON_MEDIA_ID_ENVELOPE;
            }
            else if (0 == pstringMedia->compare ("MEDIA_POSTCARD"))
            {
               bParm2 = CANON_MEDIA_ID_POSTCARD;
            }
            else if (0 == pstringMedia->compare ("MEDIA_THICK"))
            {
               bParm2 = CANON_MEDIA_ID_THICK;
            }
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm2 = CANON_MEDIA_ID_FULLXXXXX;
////////////}
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm2 = CANON_MEDIA_ID_BANNER;
////////////}
            else if (0 == pstringMedia->compare ("MEDIA_OTHER"))
            {
               bParm2 = CANON_MEDIA_ID_OTHER;
            }

            delete pstringMedia;

            if (300 > pDR->getYRes ())
            {
               bParm3 = 0;
            }
            else if (600 < pDR->getYRes ())
            {
               bParm3 = 0x02;
            }
            else
            {
               bParm3 = 0x02;
            }
            if (fHackCmdSetMono)
            {
               bParm3 = 0;
            }

            sendPrintfToDevice (pCmd,
                                bParm1,
                                bParm2,
                                bParm3);
         }
      }
      // search for: <deviceData name="cmdSetMono3" type="binary">_ESC_ "(c" HEX2S(00,02) HEX(11) HEX(00)</deviceData>
      else if (pData->getBinaryData ("cmdSetMono3", &pCmd))
      {
         sendBinaryDataToDevice (pCmd);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::setPrintColor () Error: There is no cmdSetMono or cmdSetMono2 for this device!" << std::endl;
#endif
      }
   }
   else
   {
      // Color
      // search for: <deviceData name="cmdSetColor" type="binary">_ESC_ "(c" HEX2S(00,03) HEX(10) HEX(11) HEX(10)</deviceData>
      if (pData->getBinaryData ("cmdSetColor", &pCmd))
      {
         sendBinaryDataToDevice (pCmd);
      }
      // search for: <deviceData name="cmdSetColor2" type="binary">_ESC_ "(c" HEX2S(00,03) HEX(30) "%c%c"</deviceData>
      else if (pData->getBinaryData ("cmdSetColor2", &pCmd))
      {
         DeviceResolution *pDR          = getCurrentResolution ();
         std::string      *pstringMedia = 0;
         byte              bParm1       = 0;
         byte              bParm2       = 0;

         pstringMedia = getCurrentMedia ()->getMedia ();

         if (pstringMedia)
         {
            if (0 == pstringMedia->compare ("MEDIA_PLAIN"))
            {
               bParm1 = CANON_MEDIA_ID_PLAIN;
            }
            else if (0 == pstringMedia->compare ("MEDIA_COATED"))
            {
               bParm1 = CANON_MEDIA_ID_COATED;
            }
            else if (0 == pstringMedia->compare ("MEDIA_TRANSPARENCY"))
            {
               bParm1 = CANON_MEDIA_ID_TRANS;
            }
            else if (0 == pstringMedia->compare ("MEDIA_BACKPRINT"))
            {
               bParm1 = CANON_MEDIA_ID_BACKPRINT;
            }
            else if (0 == pstringMedia->compare ("MEDIA_CLOTH"))
            {
               bParm1 = CANON_MEDIA_ID_CLOTH;
            }
            else if (0 == pstringMedia->compare ("MEDIA_GLOSSY"))
            {
               bParm1 = CANON_MEDIA_ID_GLOSSY;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_GLOSS_FILM"))
            {
               bParm1 = CANON_MEDIA_ID_GLOSSFILM;
            }
            else if (0 == pstringMedia->compare ("MEDIA_HIGH_RESOLUTION"))
            {
               bParm1 = CANON_MEDIA_ID_HIGHRES;
            }
            else if (0 == pstringMedia->compare ("MEDIA_ENVELOPE"))
            {
               bParm1 = CANON_MEDIA_ID_ENVELOPE;
            }
            else if (0 == pstringMedia->compare ("MEDIA_POSTCARD"))
            {
               bParm1 = CANON_MEDIA_ID_POSTCARD;
            }
            else if (0 == pstringMedia->compare ("MEDIA_THICK"))
            {
               bParm1 = CANON_MEDIA_ID_THICK;
            }
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm1 = CANON_MEDIA_ID_FULLXXXXX;
////////////}
////////////else if (0 == pstringMedia->compare (""))
////////////{
////////////   bParm1 = CANON_MEDIA_ID_BANNER;
////////////}
            else if (0 == pstringMedia->compare ("MEDIA_OTHER"))
            {
               bParm1 = CANON_MEDIA_ID_OTHER;
            }

            delete pstringMedia;

            if (300 > pDR->getYRes ())
            {
               bParm2 = 0;
            }
            else if (600 < pDR->getYRes ())
            {
               bParm2 = 0x02;
            }
            else
            {
               bParm2 = 0x02;
            }

            sendPrintfToDevice (pCmd,
                                bParm1,
                                bParm2);
         }
      }
      // search for: <deviceData name="cmdSetColor3" type="binary">_ESC_ "(c" HEX2S(00,02) HEX(10) HEX(00)</deviceData>
      else if (pData->getBinaryData ("cmdSetColor3", &pCmd))
      {
         sendBinaryDataToDevice (pCmd);
      }
   }
}

bool Canon_S450_Instance::
beginJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::beginJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdInit");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::beginJob () cmdInit = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::beginJob () Error: There is no cmdInit defined for this device!" << std::endl;
#endif
   }

   // @TBD
   // Japanese models cannot receive "Compression Mode" command after control
   // commands.

   return true;
}

bool Canon_S450_Instance::
beginJob (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::beginJob (with props)" << std::endl;
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

bool Canon_S450_Instance::
newFrame ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::newFrame ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::newFrame () cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   ditherNewFrame ();

   ptlPrintHead_d.x = 0;
   ptlPrintHead_d.y = 0;

   return true;
}

bool Canon_S450_Instance::
newFrame (bool fJobPropertiesChanged)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::newFrame (with props)" << std::endl;
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

bool Canon_S450_Instance::
endJob ()
{
   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::endJob ()" << std::endl;
#endif

   pCmd = pCommands->getCommandData ("cmdPageEject");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::endJob () cmdPageEject = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   pCmd = pCommands->getCommandData ("cmdTerm");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::endJob () cmdTerm = " << *pCmd << std::endl;
#endif

      sendBinaryDataToDevice (pCmd);
   }

   return true;
}

bool Canon_S450_Instance::
abortJob ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Canon_S450_Instance::endJob ()" << std::endl;
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

void Canon_S450_Instance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Canon_S450_Instance::
toString (std::ostringstream& oss)
{
   oss << "{Canon_S450_Instance: "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Canon_S450_Instance& const_self)
{
   Canon_S450_Instance& self = const_cast<Canon_S450_Instance&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}
