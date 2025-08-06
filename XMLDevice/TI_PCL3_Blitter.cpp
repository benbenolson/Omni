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
#include "TI_PCL3_Blitter.hpp"
#include "TI_PCL3_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

const static bool fTestNoCompression = false;

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new TI_PCL3_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

TI_PCL3_Blitter::
TI_PCL3_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fInstanceInitialized_d  = false;
   fGraphicsHaveBeenSent_d = false;
}

TI_PCL3_Blitter::
~TI_PCL3_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void TI_PCL3_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fInstanceInitialized_d)
      return;

   fInstanceInitialized_d = true;

   HardCopyCap     *pHCC             = getCurrentForm ()->getHardCopyCap ();
   DevicePrintMode *pDPM             = getCurrentPrintMode ();
   int              iNumDstRowBytes8 = (pHCC->getXPels () + 7) >> 3;

   iNumDstRowBytes8 = (pHCC->getXPels () + 7) >> 3;

   setCompressionInstance (new GplCompression (pDPM->getColorTech (),
                                               GplCompression::GPLCOMPRESS_TIFF,
                                               iNumDstRowBytes8,
                                               this));

   DeviceResolution *pDR  = getCurrentResolution ();

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

bool TI_PCL3_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "TI_PCL3_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   TI_PCL3_Instance *pInstance = dynamic_cast <TI_PCL3_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (pInstance->eInternalRes_d)
   {
   case TI_PCL3_Instance::IR_MONOCHROME:
   {
      return deskjetMonoRasterize (pbBits,
                                   pbmi2,
                                   prectlPageLocation,
                                   eType);
      break;
   }

   case TI_PCL3_Instance::IR_CMY:
   case TI_PCL3_Instance::IR_CMYK:
   {
      return deskjetColorRasterize (pbBits,
                                    pbmi2,
                                    prectlPageLocation,
                                    eType);
      break;
   }

   default:
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif

      break;
   }
   }

   return true;
}

void TI_PCL3_Blitter::
compressionChanged (int iNewCompression)
{
   DeviceCommand *pCommands = getCommands ();
   BinaryData    *pCmd      = 0;

   pCmd = pCommands->getCommandData ("cmdSetCompression");
   if (pCmd)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::compressionChanged cmdSetCompression = " << *pCmd << ", new compression is " << iNewCompression << std::endl;
#endif

      sendPrintfToDevice (pCmd, iNewCompression);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::compressionChanged Error: There is no cmdSetCompression defined for this device!" << std::endl;
#endif
   }
}

void TI_PCL3_Blitter::
sendData (int         iLength,
          BinaryData *pbdData,
          int         iWhichPlane)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::sendData ("
                          << iLength
                          << ", "
                          << std::hex << (int)pbdData << std::dec
                          << ", "
                          << iWhichPlane
                          << ")"
                          << std::endl;
#endif

   DeviceCommand    *pCommands = getCommands ();
   BinaryData       *pCmd      = 0;
   DevicePrintMode  *pDPM      = getCurrentPrintMode ();

   bool fLastPlane = false;

   if (  DevicePrintMode::COLOR_TECH_CMY  == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CMYK == pDPM->getColorTech ()
      )
   {
      if (DevicePrintMode::COLOR_PLANE_YELLOW == iWhichPlane)
      {
         fLastPlane = true;
      }
   }
   else // if (pDPM->isID (DevicePrintMode::PRINT_MODE_1_ANY))
   {
      fLastPlane = true;
   }

   if (fLastPlane)
   {
      pCmd = pCommands->getCommandData ("cmdTransferRasterBlock");
   }
   else
   {
      pCmd = pCommands->getCommandData ("cmdTransferRasterPlane");
   }

   if (pCmd)
   {
      sendPrintfToDevice (pCmd, iLength);
      sendBinaryDataToDevice (pbdData);
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::sendData Error: There is no cmdMoveToNextRasterGraphicsLine or cmdEndRasterGraphicsLine defined for this device!" << std::endl;
#endif
   }
}

bool TI_PCL3_Blitter::
deskjetMonoRasterize (PBYTE        pbBits,
                      PBITMAPINFO2 pbmi2,
                      PRECTL       prectlPageLocation,
                      BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize TI_PCL3_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   TI_PCL3_Instance *pInstance = dynamic_cast <TI_PCL3_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "TI_PCL3_Blitter::deskjetMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   static BYTE       Mask[8]               = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   bool              bDirty;
   PBYTE             pbBuffer;
   int               iRemainder;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
   bool              fGraphicsHaveBeenSent = false;
   bool              fHaveMoved            = false;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize iRemainder = " << iRemainder << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   for (int i = 0; i < iNumScanLines; i++)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      if (fBlackWhiteReversed)
      {
         for (int x = 0; x < cbSourceBytesInBitmap; x++)
            pbBuffer[x] = ~pbBuffer[x];
      }

      /* See if this block of raster has some bits set in it!
      */
      bDirty = false;

      // Check all but the last byte
      for (int x = 0; x <= cbDestBytesInPrinter - 2 && !bDirty; x++)
         bDirty |= pbBuffer[x];
      // Check the remaining bits
      bDirty |= pbBits[(iScanLineY + 1) * cbSourceBytesInBitmap - 1] & Mask[iRemainder];

      if (bDirty)
      {
         if (!fGraphicsHaveBeenSent)
         {
            fGraphicsHaveBeenSent = true;

            pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
            sendBinaryDataToDevice (pCmd);
         }

         fGraphicsHaveBeenSent_d = true;

#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetMonoRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY * cbSourceBytesInBitmap << std::endl;
#endif

         if (!fHaveMoved)
         {
            moveToYPosition (iWorldY, false);

            fHaveMoved = true;
         }

         if (fDumpOutgoingBitmaps)
         {
            outgoingBitmap.addScanLine (pbBits,
                                        1,
                                        ( cy
                                        - iScanLineY
                                        - 1
                                        ),
                                        CMYKBitmap::BLACK);
         }

         // Make sure that the extraneous bits are set to 0.
         pbBuffer[cbDestBytesInPrinter - 1] &= Mask[iRemainder];

         BinaryData data (pbBuffer, cbDestBytesInPrinter);

         compressKRasterPlane (&data);
         // Note: TI_PCL3_Blitter::sendData () is called because of the
         //       compression!  Also, TI_PCL3_Blitter::compressionChanged ()
         //       can be called.

         // Printing a scan line moves the printer down a scan line.  Update the
         // new current position.
         pInstance->ptlPrintHead_d.y = iWorldY + 1;
      }
      else
      {
         fHaveMoved = false;
      }

      // Move down a scan line
      iScanLineY--;
      iWorldY++;
   }

   if (fGraphicsHaveBeenSent)
   {
      pCmd = pCommands->getCommandData ("cmdEndRasterGraphics");
      sendBinaryDataToDevice (pCmd);

      // Ending the raster graphics block will force a start raster graphics
      // block which will force a zeroing out of the seed line.  Hence, make
      // the compression instance start from scratch.
      resetCompressionMode ();
   }
   else
   {
      if (fDumpOutgoingBitmaps)
      {
         outgoingBitmap.addScanLine (0, 0, 0, CMYKBitmap::BLACK);
      }
   }

   return true;
}

bool TI_PCL3_Blitter::
deskjetColorRasterize (PBYTE        pbBits,
                       PBITMAPINFO2 pbmi2,
                       PRECTL       prectlPageLocation,
                       BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize TI_PCL3_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   TI_PCL3_Instance *pInstance = dynamic_cast <TI_PCL3_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "TI_PCL3_Blitter::deskjetColorRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   PBYTE             pbBuffer;
   int               iRemainder;
   int               saveCy;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
   bool              fGraphicsHaveBeenSent = false;
   bool              fHaveMoved            = false;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      int cyPage    = getCurrentForm ()->getHardCopyCap ()->getYPels ();
      iWorldY       = cyPage - prectlPageLocation->yTop - 1;
      iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);
   }
   else
   {
      // @TBD
      int cxPage    = getCurrentForm ()->getHardCopyCap ()->getXPels ();
      iWorldY       = cxPage - prectlPageLocation->xRight - 1;
      iNumScanLines = 0;
   }

   delete pstringOrientation;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;
   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize iRemainder = " << iRemainder << std::endl;
#endif

   saveCy = pbmi2->cy;

   for (int i = 0; i < iNumScanLines; i++)
   {
      pbBuffer = pbBits + iScanLineY * cbSourceBytesInBitmap;

      pbmi2->cy = 1;
      ditherRGBtoCMYK (pbmi2, pbBuffer);

      if (!ditherAllPlanesBlank ())
      {
         if (!fGraphicsHaveBeenSent)
         {
            fGraphicsHaveBeenSent = true;

            pCmd = pCommands->getCommandData ("cmdBeginRasterGraphics");
            sendBinaryDataToDevice (pCmd);
         }

         fGraphicsHaveBeenSent_d = true;

#ifndef RETAIL
         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3_Blitter::deskjetColorRasterize iScanLineY = " << iScanLineY << ", indexing to " << iScanLineY * cbSourceBytesInBitmap << std::endl;
#endif

         if (!fHaveMoved)
         {
            moveToYPosition (iWorldY, false);

            fHaveMoved = true;
         }

         compressKRasterPlane (getKPlane ());
         compressCRasterPlane (getCPlane ());
         compressMRasterPlane (getMPlane ());
         compressYRasterPlane (getYPlane ());
         // Note: TI_PCL3_Blitter::sendData () is called because of the
         //       compression!  Also, TI_PCL3_Blitter::compressionChanged ()
         //       can be called.

         if (fDumpOutgoingBitmaps)
         {
            int iMaxPass = 4;

            /* Loop through the four color passes/planes:
            **     YELLOW .. MAGENTA .. CYAN .. BLACK
            */
            for (int iPass = 0; iPass < iMaxPass; iPass++)
            {
               switch (iPass)
               {
               case 0: /* YELLOW */
               {
                  if (ditherYPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getYPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::YELLOW);
                  break;
               }

               case 1: /* MAGENTA */
               {
                  if (ditherMPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getMPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::MAGENTA);
                  break;
               }

               case 2: /* CYAN */
               {
                  if (ditherCPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getCPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::CYAN);
                  break;
               }

               case 3: /* BLACK */
               {
                  if (ditherKPlaneBlank ())
                     continue;

                  outgoingBitmap.addScanLine (getKPlane ()->getData (),
                                              1,
                                              ( cy
                                              - iScanLineY
                                              - 1
                                              ),
                                              CMYKBitmap::BLACK);
                  break;
               }
               }
            }
         }

         // Printing a scan line moves the printer down a scan line.  Update the
         // new current position.
         pInstance->ptlPrintHead_d.y = iWorldY + 1;
      }
      else
      {
         fHaveMoved = false;
      }

      // Move down a scan line
      iScanLineY--;
      iWorldY++;
   }

   if (fGraphicsHaveBeenSent)
   {
      pCmd = pCommands->getCommandData ("cmdEndRasterGraphics");
      sendBinaryDataToDevice (pCmd);

      // Ending the raster graphics block will force a start raster graphics
      // block which will force a zeroing out of the seed line.  Hence, make
      // the compression instance start from scratch.
      resetCompressionMode ();
   }
   else
   {
      if (fDumpOutgoingBitmaps)
      {
         outgoingBitmap.addScanLine (0, 0, 0, CMYKBitmap::BLACK);
      }
   }

   pbmi2->cy = saveCy;

   return true;
}

/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool TI_PCL3_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   TI_PCL3_Instance *pInstance = dynamic_cast <TI_PCL3_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceCommand    *pCommands   = getCommands ();
   BinaryData       *pCmd        = 0;
   int               iAmount;

   if (fAbsolute)
   {
      iAmount = iWorldY;
   }
   else
   {
      iAmount = iWorldY - pInstance->ptlPrintHead_d.y;
   }

   if (0 == iAmount)
   {
      // No where to move!
      return true;
   }

   // Set Absolute Vertical Print Position
   pCmd = pCommands->getCommandData ("cmdSetYPos");

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "TI_PCL3GUI_Blitter::moveToYPosition setting position to " << iAmount << std::endl;
#endif

   sendPrintfToDevice (pCmd, iAmount);

   return true;
}

#ifndef RETAIL

void TI_PCL3_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string TI_PCL3_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{TI_PCL3_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const TI_PCL3_Blitter& const_self)
{
   TI_PCL3_Blitter& self = const_cast<TI_PCL3_Blitter&>(const_self);
   std::ostringstream       oss;

   os << self.toString (oss);

   return os;
}
