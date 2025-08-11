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
#include "IBM_Pages_Blitter.hpp"
#include "IBM_Pages_Instance.hpp"
#include "DeviceOrientation.hpp"
#include "DeviceForm.hpp"
#include "DeviceTray.hpp"
#include <CMYKbitmap.hpp>

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new IBM_Pages_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

IBM_Pages_Blitter::
IBM_Pages_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;
}

IBM_Pages_Blitter::
~IBM_Pages_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void IBM_Pages_Blitter::
initializeInstance ()
{
   HardCopyCap      *pHCC = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR  = getCurrentResolution ();
   DevicePrintMode  *pDPM = getCurrentPrintMode ();

   PSZRO       pszDitherID = getCurrentDitherID ();

   if (  DevicePrintMode::COLOR_TECH_CMY  == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ()
      )
   {
      int  iNumDstRowBytes8      = (pHCC->getXPels () + 7) >> 3;
      char achDitherOptions[512]; // @TBD

      sprintf (achDitherOptions,
               "fDataInRGB=true "
               "iBlackReduction=%d "
               "iColorTech=%d "
               "iNumDitherRows=%d "
               "iSrcRowPels=%d "
               "iNumDestRowBytes=%d "
               "iDestBitsPerPel=%d",
               /* @TBD iBlackReduction*/0,
               pDPM->getColorTech (),
               pDR->getScanlineMultiple (),
               pHCC->getXPels (),
               iNumDstRowBytes8,
               pDR->getDstBitsPerPel ());

      setDitherInstance (DeviceDither::createDitherInstance (pszDitherID,                 // iDitherType
                                                             pDevice_d,                   // pDevice
                                                             achDitherOptions));          // pszOptions
   }
}

bool IBM_Pages_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "IBM_Pages_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   IBM_Pages_Instance *pInstance = dynamic_cast <IBM_Pages_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return ibmMonoRasterize (pbBits,
                               pbmi2,
                               prectlPageLocation,
                               eType);
      break;
   }

///case DevicePrintMode::COLOR_TECH_CMYK:
///case DevicePrintMode::COLOR_TECH_CMY:
///{
///   return ibmColorRasterize (pbBits,
///                             pbmi2,
///                             prectlPageLocation,
///                             eType);
///   break;
///}

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

bool IBM_Pages_Blitter::
ibmMonoRasterize (PBYTE        pbBits,
                  PBITMAPINFO2 pbmi2,
                  PRECTL       prectlPageLocation,
                  BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_Pages_Instance *pInstance = dynamic_cast <IBM_Pages_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 7 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "IBM_Pages_Blitter::ibmMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
// static BYTE       Mask[8]               = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
// bool              bDirty;
   PBYTE             pbBuffer;
   int               iRemainder;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
// int               iXRes                 = pDR->getXRes ();
   int               iYRes                 = pDR->getYRes ();
   int               iSizeBlock;
   int               iImageHeight;
   int               iBlockY;
   int               iRelativeY;
   int               iResParam;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize = cyPage;
      iWorldY    = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize = cxPage;
      iWorldY    = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "IBM_Pages_Blitter::pagesMonoRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** word.
   ** NOTE: 0 <= iRemainder <= 31
   */
   iRemainder = (cbSourceBytesInBitmap * 8) - cx;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::pagesMonoRasterize iRemainder = " << iRemainder << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   iImageHeight = 1;

   /* set resolution parameter used in rect image command */
   switch (iYRes)
   {
      case 360:   iResParam = 0x01;
                  iBlockY = 48;
                  break;
      case 300:   iResParam = 0x10;
                  iBlockY = 60;
                  break;
      case 600:   iResParam = 0x40;
                  iBlockY = 30;
                  break;
      default:    iResParam = 0x00;
                  iBlockY = 96;
                  break;
   }

   iRelativeY = iBlockY;

   while (iNumScanLines)
   {
      /* need to send rect image command at start of a block */
      if (iRelativeY == iBlockY)
      {
         /* reset relative Y pos in a block */
         iRelativeY = 0;

         /* if this is the last blcok, set proper block height */
         if (iNumScanLines < iBlockY)
            iBlockY = iNumScanLines;

         pCmd = pCommands->getCommandData ("cmdRectImage");
         sendPrintfToDevice (pCmd,
                             cbSourceBytesInBitmap * iBlockY + 5,
                             iResParam, cbSourceBytesInBitmap * 8,
                             iBlockY);
      }

      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;
      iSizeBlock = cbSourceBytesInBitmap * iImageHeight;
      for (int i = 0; i < iImageHeight; ++i) {
         for (int j = 4; j > 0; --j) {
            PBYTE p = pbBuffer + cbSourceBytesInBitmap - j;
            *p = *p & ((0xFFFFFFFF << iRemainder) >> (8 * (j - 1)));
         }
      }
      // transfer image bits
      BinaryData data (pbBuffer, iSizeBlock);
      sendBinaryDataToDevice (&data);

      /* need to send relative Y move at end of a block */
      if (iRelativeY == (iBlockY - 1))
      {
         /* relatively move down Y pos */
         pCmd = pCommands->getCommandData ("cmdSetYPos");
         sendPrintfToDevice (pCmd,
                             iBlockY * 1440 / iYRes);
      }

      // Move down a scan line
      iScanLineY--;
      iNumScanLines--;
      iWorldY++;
      iRelativeY++;
   }

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool IBM_Pages_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   IBM_Pages_Instance *pInstance = dynamic_cast <IBM_Pages_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceCommand    *pCommands   = getCommands ();
   BinaryData       *pCmd        = 0;
   int               iAmount,
                     iResult,
                     iRemainder;
   DeviceResolution *pRes        = getCurrentResolution ();
   int               iColumnSize = pRes->getScanlineMultiple ();

   if (fAbsolute)
   {
      iAmount = iWorldY;
   }
   else
   {
      if (pInstance->ptlPrintHead_d.y == iWorldY)
      {
         // No movement required
         return true;
      }

      if (pInstance->ptlPrintHead_d.y > iWorldY)
      {
         // Wants to move backwards!
         return false;
      }

      iAmount = iWorldY - pInstance->ptlPrintHead_d.y;
   }

   pCmd = pCommands->getCommandData ("cmdSetYPos");

   if (pCmd)
   {
      sendPrintfToDevice (pCmd, iAmount);
   }
   else
   {
      int iSpacing = 0;
      int iDefault = 0;

      /* We have to do it the hard way...
      */
      pCmd = pCommands->getCommandData ("cmdSetLineSpacing216inch");
      if (pCmd)
      {
         iSpacing = 216;
      }
      else
      {
         pCmd = pCommands->getCommandData ("cmdSetLineSpacing180inch");
         if (pCmd)
         {
            iSpacing = 180;
         }
         else
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::moveToYPosition Cannot find a set line spacing command!" << std::endl;
#endif
            return false;
         }
      }

      iDefault = iSpacing * iColumnSize / pRes->getYRes ();

      iResult    = iAmount / iSpacing;
      iRemainder = iAmount - iSpacing * iResult;

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::moveToYPosition iAmount    = " << iAmount << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::moveToYPosition iResult    = " << iResult << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_Pages_Blitter::moveToYPosition iRemainder = " << iRemainder << std::endl;
#endif

      if (  0 < iResult
         || 0 < iRemainder
         )
      {
         register int i;

         BinaryData *pCmdMoveToNextRasterGraphicsLine;
         BinaryData *pCmdEndRasterGraphicsLine;

         pCmdMoveToNextRasterGraphicsLine = pCommands->getCommandData ("cmdMoveToNextRasterGraphicsLine");
         pCmdEndRasterGraphicsLine        = pCommands->getCommandData ("cmdEndRasterGraphicsLine");

         if (0 < iResult)
         {
            sendPrintfToDevice (pCmd, iSpacing);
         }

         for (i = iResult; i; i--)
         {
            // Move down
            if (pCmdMoveToNextRasterGraphicsLine)
               sendBinaryDataToDevice (pCmdMoveToNextRasterGraphicsLine);
            if (pCmdEndRasterGraphicsLine)
               sendBinaryDataToDevice (pCmdEndRasterGraphicsLine);
         }

         if (0 < iRemainder)
         {
            sendPrintfToDevice (pCmd, iRemainder);

            if (pCmdMoveToNextRasterGraphicsLine)
               sendBinaryDataToDevice (pCmdMoveToNextRasterGraphicsLine);
            if (pCmdEndRasterGraphicsLine)
               sendBinaryDataToDevice (pCmdEndRasterGraphicsLine);
         }

         sendPrintfToDevice (pCmd, iDefault);
      }
   }

   return true;
}

#ifndef RETAIL

void IBM_Pages_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_Pages_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{IBM_Pages_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_Pages_Blitter& const_self)
{
   IBM_Pages_Blitter& self = const_cast<IBM_Pages_Blitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
