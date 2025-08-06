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
#include "IBM_ESC_Blitter.hpp"
#include "IBM_ESC_Instance.hpp"

#include <DeviceOrientation.hpp>
#include <CMYKbitmap.hpp>
#include <defines.hpp>

#define BUMP_TO_NEXT_MODULUS(n,m) ((((n)+(m)-1)/(m))*(m))

int GrabPrintHeadBand (PBYTE pbBits,
                       PBYTE pbBuffer,
                       int   iMaxX,
                       int   iCurrentY,
                       int   iBytesPerColumn,
                       int   iBytesInScanLine,
                       bool  fInterleaved,
                       bool  fBlackWhiteReversed,
                       int  *piMaxRight);

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new IBM_ESC_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

IBM_ESC_Blitter::
IBM_ESC_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;
   pbBuffer_d              = 0;
}

IBM_ESC_Blitter::
~IBM_ESC_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

   if (pbBuffer_d)
   {
      free (pbBuffer_d);
      pbBuffer_d = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void IBM_ESC_Blitter::
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

bool IBM_ESC_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "IBM_ESC_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   IBM_ESC_Instance *pInstance = dynamic_cast <IBM_ESC_Instance *>(getInstance ());
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

   case DevicePrintMode::COLOR_TECH_CMYK:
   case DevicePrintMode::COLOR_TECH_CMY:
   {
      return ibmColorRasterize (pbBits,
                                  pbmi2,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

bool IBM_ESC_Blitter::
ibmMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_ESC_Instance *pInstance = dynamic_cast <IBM_ESC_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "IBM_ESC_Blitter::ibmMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iScanLineY,
                     iWorldY,
                     iColumnSize,
                     iLinesPerBuffer,
                     iNumBlocks,
                     iBytesPerColumn,
                     iMaxDataX,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   bool              fAllZero              = true,
                     bBlankLines           = false,
                     bStartofBand          = true;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize = cyPage;
      iWorldY    = cyPage - prectlPageLocation->yTop - 1;
   }
   else
   {
      // @TBD
      int cxPage = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize = cxPage;
      iWorldY    = cxPage - prectlPageLocation->xRight - 1;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;
   iColumnSize           = pDR->getScanlineMultiple ();
   iLinesPerBuffer       = BUMP_TO_NEXT_MODULUS (cy, iColumnSize);
   iNumBlocks            = iLinesPerBuffer / iColumnSize;
   iBytesPerColumn       = iColumnSize / 8;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize cbDestBytesInPrinter  = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iScanLineY            = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iColumnSize           = " << iColumnSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iLinesPerBuffer       = " << iLinesPerBuffer << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iNumBlocks            = " << iNumBlocks << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize iBytesPerColumn       = " << iBytesPerColumn << std::endl;
#endif

   if (0 == pbBuffer_d)
   {
      pbBuffer_d = (PBYTE)malloc (cx * iBytesPerColumn);
      if (pbBuffer_d)
      {
         memset (pbBuffer_d, 0, cx * iBytesPerColumn);
      }
   }

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize fBlackWhiteReversed = " << fBlackWhiteReversed << std::endl;
#endif

   for (; iNumBlocks && iWorldY >= 0; iNumBlocks--)
   {
      /* Move bits for the print so they are in the correct format for the
      ** printer.
      */
      fAllZero = GrabPrintHeadBand (pbBits,
                                    pbBuffer_d,
                                    cx,
                                    iScanLineY,
                                    iBytesPerColumn,
                                    cbSourceBytesInBitmap,
                                    false,
                                    fBlackWhiteReversed,
                                    &iMaxDataX);

      if (fAllZero)
      {
         bBlankLines = true;
      }
      else
      {
         if (  bBlankLines    // output leading blank lines
            || bStartofBand   // check for motion for start of band
            )
         {
            // Set Absolute Vertical Print Position
#ifndef RETAIL
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize setting position to " << iWorldY << std::endl;
#endif

            moveToYPosition (iWorldY, false);

            bBlankLines  = false;
            bStartofBand = false;
         }

         // Send raster transfer header
         pCmd = pDR->getData ();
         sendPrintfToDevice (pCmd, iMaxDataX);

         BinaryData data (pbBuffer_d, iMaxDataX * iBytesPerColumn);
         sendBinaryDataToDevice (&data);

         // End the raster line
         pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
         sendBinaryDataToDevice (pCmd);
         pCmd = pCommands->getCommandData ("cmdMoveToNextRasterGraphicsLine");
         sendBinaryDataToDevice (pCmd);

         if (fDumpOutgoingBitmaps)
         {
            for (int y = 0; y < iColumnSize; y++)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmMonoRasterize bitmap y pos = " << iScanLineY - y << std::endl;
#endif

               outgoingBitmap.addScanLine (pbBits + (iScanLineY - y) * cbSourceBytesInBitmap,
                                           1,
                                           cy - 1 - iScanLineY + y,
                                           CMYKBitmap::BLACK);
            }
         }

         pInstance->ptlPrintHead_d.y = iWorldY + iColumnSize;
      }

      iScanLineY -= iColumnSize;
      iWorldY    += iColumnSize;
   }

   return true;
}

bool IBM_ESC_Blitter::
ibmColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_ESC_Instance *pInstance = dynamic_cast <IBM_ESC_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   PSZRO       pszDumpEnvironmentVar = getenv ("OMNI_DUMP_OUTGOING_BITMAPS");
   bool        fDumpOutgoingBitmaps  = false;
   static int  iNum                  = 0;
   char        achName[4 + 1 + 3 + 1];

   sprintf (achName, "%04dOUT.bmp", iNum++);

   CMYKBitmap outgoingBitmap (achName, pbmi2->cx, pbmi2->cy);

   if (pszDumpEnvironmentVar)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "IBM_ESC_Blitter::ibmColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iScanLineY,
                     iWorldY,
                     iColumnSize,
                     iLinesPerBuffer,
                     iNumBlocks,
                     iBytesPerColumn,
                     iMaxDataX,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   int               saveCy;
   int               iMaxPass              = 4,
                     iLastColor            = -1;   /* Not a valid color to start with */
   bool              fAllZero              = true,
                     bBlankLines           = false,
                     bStartofBand          = true;
   PBYTE             pbBuffer;
   BinaryData       *pbdColorBits          = 0;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      ulPageSize = cyPage;
      iWorldY    = cyPage - prectlPageLocation->yTop - 1;
   }
   else
   {
      // @TBD
      int cxPage = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      ulPageSize = cxPage;
      iWorldY    = cxPage - prectlPageLocation->xRight - 1;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;
   iColumnSize           = pDR->getScanlineMultiple ();
   iLinesPerBuffer       = BUMP_TO_NEXT_MODULUS (cy, iColumnSize);
   iNumBlocks            = iLinesPerBuffer / iColumnSize;
   iBytesPerColumn       = iColumnSize / 8;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize cbDestBytesInPrinter  = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iScanLineY            = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iColumnSize           = " << iColumnSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iLinesPerBuffer       = " << iLinesPerBuffer << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iNumBlocks            = " << iNumBlocks << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize iBytesPerColumn       = " << iBytesPerColumn << std::endl;
#endif

   if (0 == pbBuffer_d)
   {
      pbBuffer_d = (PBYTE)malloc (cx * iBytesPerColumn);
      if (pbBuffer_d)
      {
         memset (pbBuffer_d, 0, cx * iBytesPerColumn);
      }
   }

   saveCy = pbmi2->cy;

   for (; iNumBlocks && iWorldY >= 0; iNumBlocks--)
   {
      pbBuffer = pbBits + ( iScanLineY
                          - iColumnSize
                          + 1
                          ) * cbSourceBytesInBitmap;

      pbmi2->cy = iColumnSize;
      ditherRGBtoCMYK (pbmi2, pbBuffer);

      if (ditherAllPlanesBlank ())
      {
         incrementBlankLineCount (iColumnSize);
      }
      else
      {
         /* Loop through the four color passes/planes:
         **     YELLOW .. MAGENTA .. CYAN .. BLACK
         */
         for (int iPass = 0; iPass < iMaxPass; iPass++)
         {
            pCmd = pCommands->getCommandData ("cmdSetColor");

            switch (iPass)
            {
            case 0: /* YELLOW */
            {
               if (ditherYPlaneBlank ())
                  continue;

               pbdColorBits = getYPlane ();

               // Need to change color modes?
               if (iLastColor != iPass)
                  sendPrintfToDevice (pCmd, 4);
               break;
            }

            case 1: /* MAGENTA */
            {
               if (ditherMPlaneBlank ())
                  continue;

               pbdColorBits = getMPlane ();

               // Need to change color modes?
               if (iLastColor != iPass)
                  sendPrintfToDevice (pCmd, 1);
               break;
            }

            case 2: /* CYAN */
            {
               if (ditherCPlaneBlank ())
                  continue;

               pbdColorBits = getCPlane ();

               // Need to change color modes?
               if (iLastColor != iPass)
                  sendPrintfToDevice (pCmd, 2);
               break;
            }

            case 3: /* BLACK */
            {
               if (ditherKPlaneBlank ())
                  continue;

               pbdColorBits = getKPlane ();

               // Need to change color modes?
               if (iLastColor != iPass)
                  sendPrintfToDevice (pCmd, 0);
               break;
            }
            }

            iLastColor = iPass;

            /* Move bits for the print so they are in the correct format for the
            ** printer.
            */
            fAllZero = GrabPrintHeadBand (pbdColorBits->getData (),
                                          pbBuffer_d,
                                          cx,
                                          iColumnSize - 1,
                                          iBytesPerColumn,
                                          cbDestBytesInPrinter,
                                          false,
                                          false,
                                          &iMaxDataX);

            if (fAllZero)
            {
               bBlankLines = true;
            }
            else
            {
               if (fDumpOutgoingBitmaps)
               {
                  CMYKBitmap::PLANE eWhichPlane = CMYKBitmap::BLACK;

                  switch (iPass)
                  {
                  case 0: /* YELLOW */  eWhichPlane = CMYKBitmap::YELLOW;  break;
                  case 1: /* MAGENTA */ eWhichPlane = CMYKBitmap::MAGENTA; break;
                  case 2: /* CYAN */    eWhichPlane = CMYKBitmap::CYAN;    break;
                  case 3: /* BLACK */   eWhichPlane = CMYKBitmap::BLACK;   break;
                  }

                  outgoingBitmap.addScanLine (pbdColorBits->getData (),
                                              iColumnSize,
                                              ( cy
                                              - iScanLineY
                                              + iColumnSize
                                              - 2
                                              ),
                                              eWhichPlane);
               }

               if (  bBlankLines    // output leading blank lines
                  || bStartofBand   // check for motion for start of band
                  )
               {
                  // Set Absolute Vertical Print Position
#ifndef RETAIL
                  if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::ibmColorRasterize setting position to " << iWorldY << std::endl;
#endif

                  moveToYPosition (iWorldY, false);

                  bBlankLines  = false;
                  bStartofBand = false;
               }

               // Send raster transfer header
               pCmd = pDR->getData ();
               sendPrintfToDevice (pCmd, iMaxDataX);

               BinaryData data (pbBuffer_d, iMaxDataX * iBytesPerColumn);
               sendBinaryDataToDevice (&data);

               // End the raster line
               pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
               sendBinaryDataToDevice (pCmd);

               pInstance->ptlPrintHead_d.y = iWorldY + iColumnSize;
            }
         }

         pCmd = pCommands->getCommandData ("cmdMoveToNextRasterGraphicsLine");
         sendBinaryDataToDevice (pCmd);
      }

      iScanLineY -= iColumnSize;
      iWorldY    += iColumnSize;
   }

   pbmi2->cy = saveCy;

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool IBM_ESC_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   IBM_ESC_Instance *pInstance = dynamic_cast <IBM_ESC_Instance *>(getInstance ());
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
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::moveToYPosition Cannot find a set line spacing command!" << std::endl;
#endif
            return false;
         }
      }

      iDefault = iSpacing * iColumnSize / pRes->getYRes ();

      iResult    = iAmount / iSpacing;
      iRemainder = iAmount - iSpacing * iResult;

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::moveToYPosition iAmount    = " << iAmount << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::moveToYPosition iResult    = " << iResult << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_ESC_Blitter::moveToYPosition iRemainder = " << iRemainder << std::endl;
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

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int
GrabPrintHeadBand (PBYTE pbBits,
                   PBYTE pbBuffer,
                   int   iMaxX,
                   int   iCurrentY,
                   int   iBytesPerColumn,
                   int   iBytesInScanLine,
                   bool  fInterleaved,
                   bool  fBlackWhiteReversed,
                   int  *piMaxRight)
{
   static BYTE  bMaxBit[256] = {
                              0, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              1, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              2, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              3, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8,
                              4, 8, 7, 8, 6, 8, 7, 8, 5, 8, 7, 8, 6, 8, 7, 8
   };
   static BYTE  bBitMask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
   static BYTE  bRemMask[8] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
   bool         fAllZero    = true;
   BYTE         currentbyte,
                columnbyte0, columnbyte1, columnbyte2, columnbyte3,
                columnbyte4, columnbyte5, columnbyte6, columnbyte7;
   int          iPosInArray,
                iBytesInCX,
                iRemX,
                iMaxRight;
   int          offset2, offset3, offset4, offset5, offset6, offset7;
   register int i, j, x;

   iMaxRight = 0;

   // How many bytes are there for the iMaxX bits?
   iBytesInCX = BUMP_TO_NEXT_MODULUS (iMaxX, 8) / 8;

   /* The for loop and the other tests expect the number to be one less than
   ** the real number.
   */
   iBytesInCX--;

   // How many bits are there in the last byte?
   iRemX = iMaxX & 7;
   if (0 == iRemX)
      /* Since cx is divisible by 8, the "end of the byte" is in effect and
      ** you want to address every bit in that byte.
      */
      iRemX = 8;

   // Calculate the offsets in advance
   offset2  = iBytesPerColumn << 1;
   offset3  = offset2 + iBytesPerColumn;
   offset4  = offset3 + iBytesPerColumn;
   offset5  = offset4 + iBytesPerColumn;
   offset6  = offset5 + iBytesPerColumn;
   offset7  = offset6 + iBytesPerColumn;

   /* Loop through every pel in the X direction one byte at a time.
   ** 8-bit move along the x axis.
   */
   for (x = 0; x <= iBytesInCX; x++)
   {
      /* Calculate the offset into the bitmap array (OS/2 PM defined).
      ** This is a 1-bpp bitmap format.  Note that x (in pels) runs in
      ** multiples of 8, so to access the bitmap data, you need to divide
      ** by 8.
      */
      iPosInArray = x + iCurrentY * iBytesInScanLine;

      /* Loop through the number of bytes in the printer head.
      ** 8-bit move along the y axis.
      */
      for (j = 0; j < iBytesPerColumn; j++)
      {
         // Clear out the column bits
         columnbyte0 = columnbyte1 = columnbyte2 = columnbyte3 =
         columnbyte4 = columnbyte5 = columnbyte6 = columnbyte7 = 0;

         if (iPosInArray >= 0)
         {
            /* Construct the 90 deg rotated bytes from column of bits.
            ** Loop through each scanline in the 8-bit byte of the printer head.
            ** 1-bit move along the y axis.
            */
            for (i = 0; i <= 7; i++)
            {
               // Grab the byte of bitmap data
               currentbyte = pbBits[iPosInArray];

               if (fBlackWhiteReversed)
                  currentbyte = ~currentbyte;

               if (x == iBytesInCX)
                  /* If this is the last byte then, only look at the remaining
                  ** bits (mask off extraneous bits).
                  */
                  currentbyte &= bRemMask[iRemX - 1];

               // Is there something there?
               if (currentbyte)
               {
                  fAllZero = false;

                  /* Keep track of the maximum bit that was set for this
                  ** byte of bitmap data.
                  */
                  iMaxRight = omni::max (((x << 3) + bMaxBit[currentbyte]),
                                         iMaxRight);

                  /* Set the corresponding rotated bits in the column bytes.
                  */
                  if (currentbyte & 0x80) columnbyte0 |= bBitMask[i];
                  if (currentbyte & 0x40) columnbyte1 |= bBitMask[i];
                  if (currentbyte & 0x20) columnbyte2 |= bBitMask[i];
                  if (currentbyte & 0x10) columnbyte3 |= bBitMask[i];
                  if (currentbyte & 0x08) columnbyte4 |= bBitMask[i];
                  if (currentbyte & 0x04) columnbyte5 |= bBitMask[i];
                  if (currentbyte & 0x02) columnbyte6 |= bBitMask[i];
                  if (currentbyte & 0x01) columnbyte7 |= bBitMask[i];
               }

               /* Decrement the scan line pointer.  This has the effect of
               ** keeping x the same and moving one less for y.
               */
               iPosInArray -= iBytesInScanLine;
               if (fInterleaved)
                  iPosInArray -= iBytesInScanLine;

               if (iPosInArray < 0)
                  // We are outside the bitmap array.  Leave!
                  break;
            }
         }

         // Are we on the last byte?
         if (x == iBytesInCX)
         {
            /* Check the number of bits remaining before writing out the data.
            ** We are guarenteed at least one.
            */
            *(pbBuffer + j) = columnbyte0;
            if (1 < iRemX)
               *(pbBuffer + iBytesPerColumn + j) = columnbyte1;
            if (2 < iRemX)
               *(pbBuffer + offset2         + j) = columnbyte2;
            if (3 < iRemX)
               *(pbBuffer + offset3         + j) = columnbyte3;
            if (4 < iRemX)
               *(pbBuffer + offset4         + j) = columnbyte4;
            if (5 < iRemX)
               *(pbBuffer + offset5         + j) = columnbyte5;
            if (6 < iRemX)
               *(pbBuffer + offset6         + j) = columnbyte6;
            if (7 < iRemX)
               *(pbBuffer + offset7         + j) = columnbyte7;
         }
         else
         {
            /* Write the 8 column block of bitmap data.
            */
            *(pbBuffer +                   j) = columnbyte0;
            *(pbBuffer + iBytesPerColumn + j) = columnbyte1;
            *(pbBuffer + offset2         + j) = columnbyte2;
            *(pbBuffer + offset3         + j) = columnbyte3;
            *(pbBuffer + offset4         + j) = columnbyte4;
            *(pbBuffer + offset5         + j) = columnbyte5;
            *(pbBuffer + offset6         + j) = columnbyte6;
            *(pbBuffer + offset7         + j) = columnbyte7;
         }
      }

      /* We have done an 8 byte block for however many byte columns there
      ** are.  Increment the output buffer pointer...
      */
      pbBuffer += (iBytesPerColumn << 3);
   }

   // Return the maximum bit that was set in the x axis.
   if (piMaxRight)
      *piMaxRight = iMaxRight;

   // Return if this band is empty or not.
   return fAllZero;
}

#ifndef RETAIL

void IBM_ESC_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_ESC_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{IBM_ESC_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_ESC_Blitter& const_self)
{
   IBM_ESC_Blitter& self = const_cast<IBM_ESC_Blitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
