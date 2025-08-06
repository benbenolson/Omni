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
#include "Epson_High_Res_ESCP2_Blitter.hpp"
#include "Epson_High_Res_ESCP2_Instance.hpp"
#include "DeviceOrientation.hpp"

#include <stdio.h>

#define PERF_CHANGE 1

const static bool fTestNoCompression = false;

int  compressEpsonRLE (PBYTE  pbData,
                       int    cBytesInData,
                       PBYTE  pbReturn,
                       int    cBytesInReturn);

void expand1To2Bpp    (PBYTE        pbFrom,
                       PBYTE        pbTo,
                       int          cbDestBytesInPrinter,
                       int          iRemainder,
                       int          iPasses);

void flipBand(PBYTE pbInOut, PBYTE pbOut, int iLinesToCopy, int iLineLength);

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new Epson_High_Res_ESCP2_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

Epson_High_Res_ESCP2_Blitter::
Epson_High_Res_ESCP2_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;
   cbCompress_d            = 0;
   pbCompress_d            = 0;

   pbOldCPlane             = 0;           // @@PAZ
   pbOldMPlane             = 0;
   pbOldYPlane             = 0;
   pbOldKPlane             = 0;
   pbOldLCPlane            = 0;
   pbOldLMPlane            = 0;

//   lCurPageLocation        = 0;
   lCurPageLocation        = -293;
   bNotFirstPass           = false;
   bNotFirstRun            = false;
   lFirstBandLocation      = 0;
   lSecondBandLocation     = 0;
   lSecondPlane_cy         = 0;
   lFirstPlane_cy          = 0;
   bLastBand               = false;
   bStartPage              = true;
   iShiftValue             = -1;
   iMoveRight              = 0;
   bDoShiftRight           = false;
   fInstanceInitialized_d  = false;
   iRealPos                = 0;
   bStillWhite             = true;
   lWhiteSpaceIncrement    = 0;

}

Epson_High_Res_ESCP2_Blitter::
~Epson_High_Res_ESCP2_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   if (pbCompress_d)
   {
      free (pbCompress_d);
      pbCompress_d = 0;
      cbCompress_d = 0;
   }

   if(pbOldCPlane)
   {
       free(pbOldCPlane);
   }
   if(pbOldMPlane)
   {
       free(pbOldMPlane);
   }
   if(pbOldYPlane)
   {
       free(pbOldYPlane);
   }
   if(pbOldKPlane)
   {
       free(pbOldKPlane);
   }

   fGraphicsHaveBeenSent_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void Epson_High_Res_ESCP2_Blitter::
initializeInstance ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   if (fInstanceInitialized_d)
      return;

   fInstanceInitialized_d = true;

   HardCopyCap *pHCC             = getCurrentForm ()->getHardCopyCap ();
   int          iNumDstRowBytes8 = (pHCC->getXPels () + 7) >> 3;

   iNumPageLines = pHCC->getYPels();

   cbCompress_d = iNumDstRowBytes8 * 24;
   cbCompress_d += cbCompress_d / 20;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbCompress_d = " << cbCompress_d << std::endl;
#endif

   pbCompress_d = (PBYTE)malloc (cbCompress_d);

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "pbCompress_d = " << (int)pbCompress_d << std::dec << std::endl;
#endif

   DeviceResolution *pDR  = getCurrentResolution ();
   DevicePrintMode  *pDPM = getCurrentPrintMode ();

   const char *pszDitherID = getCurrentDitherID ();

   if (  DevicePrintMode::COLOR_TECH_CMY    == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CMYK   == pDPM->getColorTech ()
      || DevicePrintMode::COLOR_TECH_CcMmYK == pDPM->getColorTech ()
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

bool Epson_High_Res_ESCP2_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "Epson_High_Res_ESCP2_Blitter::rasterize (0x"
           << (int)pbBits << ", 0x"
           << (int)pbmi2 << ", "
           << std::dec
           << "{" << prectlPageLocation->xLeft << ", " << prectlPageLocation->yBottom << ", " << prectlPageLocation->xRight << ", " << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   Epson_High_Res_ESCP2_Instance *pInstance = dynamic_cast <Epson_High_Res_ESCP2_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   pInstance->setupPrinter ();

   switch (getCurrentPrintMode ()->getColorTech ())
   {
   case DevicePrintMode::COLOR_TECH_K:
   {
      return epsonMonoRasterize (pbBits,
                                 pbmi2,
                                 0, //psizelPage,
                                 prectlPageLocation,
                                 eType);
      break;
   }

   case DevicePrintMode::COLOR_TECH_CMYK:
   case DevicePrintMode::COLOR_TECH_CMY:
   case DevicePrintMode::COLOR_TECH_CcMmYK:
   {
      return epsonColorRasterize (pbBits,
                                  pbmi2,
                                  getCurrentPrintMode ()->getColorTech (),
                                  0, //psizelPage,
                                  prectlPageLocation,
                                  eType);
      break;
   }

   default:
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

/***********************************************************************

  Routine: GetMemory

   This routine will allocate memory for the requestor.  If it fails,
   it has the ability to output an error message stating the failing
   request

   RETURNS:
            False - fail
            TRUE  - success

***********************************************************************/

bool
GetMemory (PBYTE *pbData,
           long   lSize,
           char  *pszLocation)
{
   bool bRet = false;

   *pbData = (PBYTE) malloc(lSize);  // Allocate memory for the old band

   if (!*pbData)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Failed Allocating " << pszLocation << std::endl;
#endif
   }
   else
   {
      memset (*pbData, 0, lSize);
      bRet = true;
   }

   return bRet;
}

void Epson_High_Res_ESCP2_Blitter::
InitializePageValues(int iNumPass)
{

    if(iNumPass > 1)
        lCurPageLocation    = -293;
    else
        lCurPageLocation    = 0;
    bNotFirstPass           = false;
    bLastBand               = false;
    bStartPage              = true;
    lSecondBandLocation     = 0;    //@@PAZ-UPDATE
    lFirstBandLocation      = 0;
    lSecondPlane_cy         = 0;
    lFirstPlane_cy          = 0;
    iShiftValue             = -1;
    iMoveRight              = 0;
    iRealPos                = 0;
    bStillWhite             = true;
    lWhiteSpaceIncrement    = 0;
}

bool Epson_High_Res_ESCP2_Blitter::
epsonMonoRasterize (PBYTE        pbBits,
                    PBITMAPINFO2 pbmi2,
                    PSIZEL       psizelPage,
                    PRECTL       prectlPageLocation,
                    BITBLT_TYPE  eType)
{


#define SETSHIFTVALUE                                       \
    if(iShiftValue < iNozzleSpace-1)                        \
    {                                                       \
        if(iShiftValue == 0)                                \
        {                                                   \
            if(iMoveRight)                                  \
                {                                           \
                  iMoveRight++;                             \
                  if(iMoveRight == iNumberofPasses)         \
                      iMoveRight = 0;                       \
                }                                           \
            else                                            \
                iMoveRight++;                               \
        }                                                   \
        else                                                \
          if(iShiftValue == (iNozzleSpace/2))               \
              if(iMoveRight)                                \
                  {                                         \
                    iMoveRight++;                           \
                    if(iMoveRight == iNumberofPasses)       \
                        iMoveRight = 0;                     \
                  }                                         \
              else                                          \
                  iMoveRight++;                             \
                                                            \
       iShiftValue++;                                       \
                                                            \
    }                                                       \
    else                                                    \
    {                                                       \
        iShiftValue = 0;                                    \
        if(iMoveRight)                                      \
        {                                                   \
           iMoveRight++;                                    \
           if(iMoveRight == iNumberofPasses)                \
               iMoveRight = 0;                              \
        }                                                   \
        else                                                \
           iMoveRight++;                                    \
    };



#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Epson_High_Res_ESCP2_Instance *pInstance = dynamic_cast <Epson_High_Res_ESCP2_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceResolution *pDR           = getCurrentResolution ();
   DeviceData       *pDevData      = getDeviceData(); // get device data for this printer

   int               cy            = pbmi2->cy,
                     cx            = pbmi2->cx;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   PBYTE             pbBuffer;
   int               iRemainder;
   int               iSavecy;

   PBYTE             pbRasterLine = 0;    // use for outputting multipass output lines
   PBYTE             pbBlanks = 0;        // use for blank raster line

   PBYTE             pbCurKPlane = 0;

   PBYTE             pbPrevious = 0;  // used for managing previous plane
   PBYTE             pbCurrentBand = 0;   // used for managing current plane band

   int               iCompressed;
   DeviceCommand    *pCommands     = getCommands ();
   BinaryData       *pCmd          = 0;
   register int      sl;
   int               iStripeSize;
   int               iNumberofPasses = 0;  // number of passes across X axis
   int               iXPos;                       // physical XPos
   int               iNumberofNozzles;
   int               iNozzleSpace;
   bool              bOutputComplete = false;
   long              lBandMemSize = 0;
   bool              bBlankBand = true;
   int               iLineMoveSize;
   int               iStartBufferLines = 0;   //@@START
   long              lStartBandMemSize;     //@@START
   PBYTE             pbExpand = 0;
   iStartPassVal = 0;

   if(!pDevData)                     // make sure we have DeviceData
   {
#ifndef RETAIL
       DebugOutput::getErrorStream () << "Fatal Error -- failure to get printhead data " << std::endl;
#endif
       return false;
   }

                // Get the setup information out of the DeviceData
   pDevData->getIntData("Nozzle_Number", &iNumberofNozzles);
   pDevData->getIntData("Nozzle_Spacing", &iNozzleSpace);
   pDevData->getIntData("Positioning_x", &iXPos);


   iStripeSize = iNozzleSpace * iNumberofNozzles;  // Band size needs to be set to a multiple of the nozzle spacing
                                                   // and not to exceed the size of iNozzleSpace * iNumberofNozzles

   // get the number of passes for this resolution
   iNumberofPasses = getNumberofPasses( pDR->getXRes (),iXPos );

   if(!iNumberofPasses)
       return false;  // we should not be here or routine will fail

   iSavecy = cy;

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      iWorldY    = iNumPageLines - prectlPageLocation->yTop;
   }
   else
   {
      iWorldY    = prectlPageLocation->xRight;
   }

   delete pstringOrientation;

   if(lCurPageLocation != 0)   // reset - we must be at the top of a new page
   {
       if(lCurPageLocation > (long)iWorldY)
       {
           iStartPassVal           = 0;
           InitializePageValues(iNumberofPasses);
       }
   }

   if(bStartPage)
   {
       iStartBufferLines = iStripeSize+5;
   }


#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;

   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

   iScanLineY            = cy - 1;
   iNumScanLines         = omni::min (cy, iWorldY+1);

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iNumScanLines = " << iNumScanLines << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   cbDestBytesInPrinter *=2; // double since we are using 2 bits per pel.

   // setup for 2 bit per pel printing
//   PBYTE pbExpand = (PBYTE)malloc (cbDestBytesInPrinter);
   GetMemory(&pbExpand, cbDestBytesInPrinter, "pbExpand");
   if(!pbExpand)
       goto done;

   pbBuffer = pbBits;

   if (fBlackWhiteReversed)
   {
      for (int x = 0; x < cbSourceBytesInBitmap * cy; x++)
         pbBuffer[x] = ~pbBuffer[x];
   }

   if(iNumberofPasses > 1)  // non-360 X-resolution output
   {
       // the first time though we only want to setup the previous band.  No need to
       // do anything with the data here

       lBandMemSize =  cbSourceBytesInBitmap*cy;    // allocate memory to hold previous band
       lStartBandMemSize =  cbSourceBytesInBitmap *(iStartBufferLines+cy);  //@@START

          // first time in so copy off band
       if(!bNotFirstPass)
       {

          GetMemory(&pbOldKPlane, lStartBandMemSize, "pbOldKPlane");

          if(!pbOldKPlane)
          {
              goto done;
          }

          for(iScanLineY = 0; iScanLineY < cy; iScanLineY++)
          {
              bBlankBand = true;
              pbBuffer = pbBits +  (cy - (iScanLineY +1))  * cbSourceBytesInBitmap;
              for(int i = 0; (i < cbSourceBytesInBitmap) && !bBlankBand; i++)
              {
                 if(*(pbBuffer+i))
                     bBlankBand = false;
              }
              if(!bBlankBand)
              {
                  bStillWhite = false;
                  memcpy(pbOldKPlane + ((iScanLineY+iStartBufferLines) * cbSourceBytesInBitmap),
                          pbBuffer, cbSourceBytesInBitmap);
              }
          }

          bNotFirstPass = true;
          lFirstPlane_cy = cy; //@@START

         // some printers need to have the page top set correctly.

          DeviceResolution *pRes      = getCurrentResolution ();
          DeviceForm *pForm = getCurrentForm ();
          DeviceCommand    *pCommands = getCommands ();
          BinaryData       *pCmd      = 0;

          DeviceData       *pData     = 0;  //@@PAZ

          int iLinesTopMargin = (int)((float) pRes->getYRes () * ((float)pForm->getHardCopyCap ()->getTopClip ()/ 25400.0));
          bool bRet = false;

          pData     = getDeviceData ();  //@@PAZ
          if(pData)
          {
              int iStart = 0;
              bRet =  pData->getIntData ("OffsetStart", &iStart);
              if(bRet && iStart)
              {
                  pCmd = pCommands->getCommandData ("cmdMoveRelativeY");
                  if(pCmd)
                  {
                      sendPrintfToDevice (pCmd,iLinesTopMargin);
                  }
                  else
                  {
#ifndef RETAIL
                      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdMoveRelativeY defined for this device!" << std::endl;
#endif
                  }
              }
          }

          /***********  We only wanted to setup the band on first pass so leave now ******/

         return true;

       }
       else
       {
          // copy off the last band
          // Setup our band data

           GetMemory(&pbCurKPlane, lBandMemSize, "pbCurKPlane");

           if(!pbCurKPlane)
           {
               goto done;
           }

           for(iScanLineY = 0; iScanLineY < cy ; iScanLineY++)
           {
               bBlankBand = true;
               pbBuffer = pbBits +  (cy - (iScanLineY +1))  * cbSourceBytesInBitmap;

               for(int i = 0; (i < cbSourceBytesInBitmap) && bBlankBand; i++)
               {
                  if(*(pbBuffer+i))
                      bBlankBand = false;
               }
               if(!bBlankBand)
               {
                   lWhiteSpaceIncrement = 0;
                   bStillWhite = false;
                   memcpy(pbCurKPlane + (iScanLineY * cbSourceBytesInBitmap),
                                       pbBuffer, cbSourceBytesInBitmap);
               }
               else
               {
                   lWhiteSpaceIncrement++;
               }

           }
         lSecondPlane_cy = cy;
       }

       // set up our blank information here so we can use blank lines and bands when needed
       GetMemory(&pbRasterLine, cbDestBytesInPrinter, "pbRasterLine");
       if(!pbRasterLine)
       {
           goto done;
       }

       GetMemory(&pbBlanks, cbDestBytesInPrinter, "pbBlanks");
       if(!pbBlanks)
       {
           goto done;
       }

       memset (pbBlanks, 0, cbDestBytesInPrinter);

       iNumScanLines = omni::min ((int)(lSecondPlane_cy+lFirstPlane_cy), iWorldY+1);

#ifndef RETAIL
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iScanLineY = " << iScanLineY << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iNumScanLines = " << iNumScanLines << std::endl;
#endif

       // set everything up so we can tell which band we will need to get the
       // data out of

       if(lSecondBandLocation == 0)
       {
           lSecondBandLocation = lFirstPlane_cy ;
       }
       else
       {
           lFirstBandLocation = lSecondBandLocation;
           lSecondBandLocation += lFirstPlane_cy;
       }

       //@@LASTBAND
       if((iSavecy % iStripeSize) || ((lSecondBandLocation+iSavecy)>= iNumPageLines))
       {
           bLastBand = true;
       }

         // set condition so that we know we are at the end of the page and have to change
         // the values of the number of lines that are going down to the printer
#ifndef RETAIL
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= iNumPageLines = "     << iNumPageLines <<std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= lSecondPlane_cy = "      << lSecondPlane_cy <<std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= lSecondBandLocation = " << lSecondBandLocation <<std::endl;
#endif

       while (!bOutputComplete)        // need to check the size and make sure we are not out of the band on the bottom
       {                                   // lines of our output
             // setup a rotate scheme so that we know when we need to shift right the bits

          if(((lCurPageLocation+(long)iStripeSize) < (lSecondBandLocation + lSecondPlane_cy)) || bLastBand)  // check to see if we can do another pass
          {
              if((iNumberofPasses == 2) || (iNumberofPasses == 4))  // 720 or 1440 X resolution
              {
                SETSHIFTVALUE
              }

              pbPrevious = pbOldKPlane;
              pbCurrentBand = pbCurKPlane;

                     // Can't go past the end of the second band bottom
              if(((lCurPageLocation + iStripeSize) <  (lSecondBandLocation+lSecondPlane_cy)) && !bLastBand)
              {

                  if(!bStartPage)
                  {
                      if(!bStillWhite && (lWhiteSpaceIncrement <(3*iStripeSize)))
                      {
                          if(iMoveRight)
                          {
                              pCmd = pCommands->getCommandData ("cmdPositionX1");
                              sendPrintfToDevice (pCmd,iMoveRight,0,0,0);
                          }


                          pCmd = pCommands->getCommandData ("cmdVariRaster");
                          sendPrintfToDevice (pCmd,
                                              0,                     // Color
                                              fTestNoCompression ? 0:1,        // compression
                                              2,                         // Bit width
                                              (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,  // X pels
                                              iNumberofNozzles); // Y Pels


                          for (sl = 0; sl < iNumberofNozzles; sl++)
                          {
                             long lPosition = lCurPageLocation + (long)(sl*iNozzleSpace);
                               // data is coming out of top band
                             memset(pbExpand, 0, cbDestBytesInPrinter);
                             if(lPosition < lSecondBandLocation)
                             {
                                 pbBuffer = pbPrevious + ((lPosition - lFirstBandLocation) * cbSourceBytesInBitmap);
                             }
                             else
                             { // data is coming out of the bottom band
                                 pbBuffer = pbCurrentBand + ((lPosition - lSecondBandLocation) * cbSourceBytesInBitmap);
                             }

                             expand1To2Bpp (pbBuffer, pbExpand, cbDestBytesInPrinter/2, iRemainder, iNumberofPasses);
                             memset(pbRasterLine, 0, cbDestBytesInPrinter);

                             // check to see which set of bits need to be sent down to the printer
                             if(iNumberofPasses == 2)
                             {
                                epsonSplit2for1(pbExpand, pbRasterLine, cbDestBytesInPrinter, iMoveRight % iNumberofPasses);
                             }
                             else
                                 if(iNumberofPasses == 4)
                                 {
                                    epsonSplit4for1(pbExpand, pbRasterLine, cbDestBytesInPrinter, iMoveRight % iNumberofPasses);
                                 }

                             if (!fTestNoCompression)
                             {
                                iCompressed = compressEpsonRLE (pbRasterLine,
                                                                (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,
                                                                pbCompress_d,
                                                                cbCompress_d);

                                BinaryData data (pbCompress_d, iCompressed);
                                sendBinaryDataToDevice (&data);
                             }
                             else
                             {
                                BinaryData data (pbRasterLine, (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses);
                                sendBinaryDataToDevice (&data);
                             }

                          }  // end of send to printer

                          pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                          sendBinaryDataToDevice (pCmd);
                      }
                  }
                  else
                  {
                      int iLinesToSend;
                      if(lCurPageLocation < 0)
                          iLinesToSend = (lCurPageLocation+iStripeSize)/iNozzleSpace;
                      else
                          iLinesToSend = iNumberofNozzles;

                      if(iLinesToSend)
                      {
                          if(!bStillWhite && (lWhiteSpaceIncrement <(3*iStripeSize)))
                          {
                              if(iMoveRight)
                              {
                                  pCmd = pCommands->getCommandData ("cmdPositionX1");
                                  sendPrintfToDevice (pCmd,iMoveRight,0,0,0);

                              }


                              pCmd = pCommands->getCommandData ("cmdVariRaster");
                              sendPrintfToDevice (pCmd,
                                                  0,                     // Color
                                                  fTestNoCompression ? 0:1,        // compression
                                                  2,                         // Bit width
                                                  (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,  // X pels
                                                  iLinesToSend); // Y Pels

                              for (sl = 0; sl < iNumberofNozzles; sl++)
                              // @tbd make sure low res blitter and mono increment the same way
                              {
                                 long lPosition = lCurPageLocation + (long)(sl*iNozzleSpace);

                                   // data is coming out of top band
                                 if(lPosition >= 0)
                                 {
                                     memset(pbExpand, 0, cbDestBytesInPrinter);
                                     if(lPosition < lSecondBandLocation)
                                     {
                                         pbBuffer = pbPrevious + ((lPosition + iStartBufferLines - lFirstBandLocation) * cbSourceBytesInBitmap);
                                     }
                                     else
                                     { // data is coming out of the bottom band
                                         pbBuffer = pbCurrentBand + ((lPosition - lSecondBandLocation) * cbSourceBytesInBitmap);
                                     }

                                     expand1To2Bpp (pbBuffer, pbExpand, cbDestBytesInPrinter/2, iRemainder, iNumberofPasses);
                                     memset(pbRasterLine, 0, cbDestBytesInPrinter);

                                     // check to see which set of bits need to be sent down to the printer
                                     if(iNumberofPasses == 2)
                                     {
                                        epsonSplit2for1(pbExpand, pbRasterLine, cbDestBytesInPrinter, iMoveRight % iNumberofPasses);
                                     }
                                     else
                                         if(iNumberofPasses == 4)
                                         {
                                            epsonSplit4for1(pbExpand, pbRasterLine, cbDestBytesInPrinter, iMoveRight % iNumberofPasses);
                                         }

                                     if (!fTestNoCompression)
                                     {
                                        iCompressed = compressEpsonRLE (pbRasterLine,
                                                                        (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,
                                                                        pbCompress_d,
                                                                        cbCompress_d);

                                        BinaryData data (pbCompress_d, iCompressed);
                                        sendBinaryDataToDevice (&data);
                                     }
                                     else
                                     {
                                        BinaryData data (pbRasterLine, (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses);
                                        sendBinaryDataToDevice (&data);
                                     }
                                 }

                              }  // end of send to printer

                              pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                              sendBinaryDataToDevice (pCmd);
                          }
                      }
                  }
              }
              else
              {
                  if(bLastBand)  // we are finishing out the page
                  {
                      int iNumLines = ((lSecondBandLocation + lSecondPlane_cy)-lCurPageLocation)/iNozzleSpace > iNumberofNozzles ?
                                        iNumberofNozzles : ((lSecondBandLocation + lSecondPlane_cy)-lCurPageLocation)/iNozzleSpace;

#ifndef RETAIL
                      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "CURRENT PAGE LOCATION = " << lCurPageLocation << std::endl;
                      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () <<"lines to send = " << iNumLines << std::endl;
                      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "bottom = " << (lFirstBandLocation + lFirstPlane_cy) << std::endl;
#endif

                      if(!bStillWhite && (lWhiteSpaceIncrement <(3*iStripeSize)))
                      {
                          if(iMoveRight)
                          {
                              pCmd = pCommands->getCommandData ("cmdPositionX1");
                              sendPrintfToDevice (pCmd,iMoveRight,0,0,0);
                          }

                          pCmd = pCommands->getCommandData ("cmdVariRaster");

                          sendPrintfToDevice (pCmd,
                                              0,                     // Color
                                              fTestNoCompression ? 0:1,        // compression
                                              2,                         // Bit width
                                              (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,  // X pels
                                              iNumLines); // Y Pels

                          for (sl = 0; sl < iNumLines; sl++)
                          {
                             long lPosition = lCurPageLocation + (long)(sl*iNozzleSpace);
                               // data is coming out of top band
                             memset(pbExpand, 0, cbDestBytesInPrinter);
                             if( lPosition < lSecondBandLocation )
                             {
                                 pbBuffer = pbPrevious + (lPosition - lFirstBandLocation) * cbSourceBytesInBitmap;
                             }
                             else
                             { // No more data is available in the bands
                                 pbBuffer = pbCurrentBand + (lPosition - lSecondBandLocation) * cbSourceBytesInBitmap;
                             }

                             expand1To2Bpp (pbBuffer, pbExpand, cbDestBytesInPrinter/2, iRemainder, iNumberofPasses);
                             memset(pbRasterLine, 0, cbDestBytesInPrinter);

                             // check to see which set of bits need to be sent down to the printer
                             if(iNumberofPasses == 2)
                             {
                                 epsonSplit2for1(pbExpand, pbRasterLine, cbDestBytesInPrinter,
                                                                          iMoveRight % iNumberofPasses);
                             }
                             else
                                 if(iNumberofPasses == 4)
                                 {
                                     epsonSplit4for1(pbExpand, pbRasterLine, cbDestBytesInPrinter,
                                                                              iMoveRight % iNumberofPasses);
                                 }

                             if (!fTestNoCompression)
                             {
                                iCompressed = compressEpsonRLE (pbRasterLine,
                                                                (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses,
                                                                pbCompress_d,
                                                                cbCompress_d);

                                BinaryData data (pbCompress_d, iCompressed);
                                sendBinaryDataToDevice (&data);
                             }
                             else
                             {
                                BinaryData data (pbRasterLine, (cbDestBytesInPrinter+iNumberofPasses-1)/iNumberofPasses);
                                sendBinaryDataToDevice (&data);
                             }


                          }  // end of send to printer

                          pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                          sendBinaryDataToDevice (pCmd);
                      }

                            // set condition to get out of finishup code
                      if(lCurPageLocation >= iNumPageLines)
                          bOutputComplete = true;


                  }
                  else       // if this is not the last band, get more data
                  {
                      bOutputComplete = true;
                  }
              }
                   // End the raster line

             pCmd = pCommands->getCommandData ("cmdMoveRelativeY");

             // Find out how many lines to the next drawing location

             iLineMoveSize = GetNextRunDrawLocation(iNumberofNozzles,
                                                    iNozzleSpace,
                                                    cbDestBytesInPrinter, iShiftValue, iNumberofPasses);

             lCurPageLocation += (long)iLineMoveSize;  // increment down our page location

             if(iRealPos >= 0)
             {
                 if(lCurPageLocation < 0)
                 {
                     if(iStartPassVal++ % iNumberofPasses)
                     {
                         iLineMoveSize = 0;
                     }
                     else
                     {
                         iLineMoveSize = 1;
                         iRealPos++;
                     }
                 }
                 else
                 {
                     if(lCurPageLocation > (long)iRealPos)
                     {
                         iLineMoveSize = (int)lCurPageLocation - iRealPos ;
                         iRealPos = -1;
                     }
                     else
                     {
                         if(iStartPassVal++ % iNumberofPasses)
                         {
                             iLineMoveSize = 0;
                         }
                         else
                         {
                             iLineMoveSize = 1;
                             iRealPos++;
                         }
                     }
                 }
                 if(iStartPassVal == iNumberofPasses)
                     iStartPassVal = 0;
             }

#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iLineMoveSize = " << iLineMoveSize << std::endl;
#endif

             // @@LASTBAND - don't go past end of page
             if((lCurPageLocation + (long)iLineMoveSize) <(long) iNumPageLines)
                sendPrintfToDevice (pCmd,iLineMoveSize);
             else
                 lCurPageLocation = iNumPageLines;

          }
          else
          {
              bOutputComplete = true;
          }
       }
       if(bStartPage)
           bStartPage = false;
   }
   else
   {   // 360 X-resolution blitter for non-FINE output

       PBYTE pTempBand;
       int iNumLines360Band = iNumberofNozzles;
       iNumLines360Band /=2;

       iScanLineY            = cy - 1;

       iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);

       while (iNumScanLines)
       {
          while (iNumScanLines >= iNumLines360Band)
          {

             pbmi2->cy = iNumLines360Band;

#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "indexing to " << (iScanLineY - iNumLines360Band + 1) * cbSourceBytesInBitmap << std::endl;
#endif

             GetMemory(&pTempBand, iNumLines360Band * cbSourceBytesInBitmap, "pTempBand");
             if(!pTempBand)
             {
                 goto done;
             }

             // We're upside down with we dithered data so we need to switch the band memory around.

             pbBuffer = pbBits + ( iScanLineY
                                 - iNumLines360Band
                                 + 1
                                 ) * cbSourceBytesInBitmap;
             for(int iLineValue = iNumLines360Band-1; iLineValue >=0 ; iLineValue--)
             {
                 memcpy(pTempBand+(iLineValue*cbSourceBytesInBitmap),
                                pbBuffer+(((iNumLines360Band-1)-iLineValue)* cbSourceBytesInBitmap),
                        cbSourceBytesInBitmap);
             }

             for(int k = 0 ; k < iNumLines360Band * cbSourceBytesInBitmap; k++)
             {
                 if(pbBuffer[k])
                 {
                     bBlankBand = false;
                     break;
                 }

             }

             if(!bBlankBand)
             {
                 memcpy(pbBuffer, pTempBand, iNumLines360Band * cbSourceBytesInBitmap );

                 if(pTempBand)
                   free(pTempBand);

                 pTempBand = pbBuffer;
                 // Send raster transfer header
                 pCmd = pCommands->getCommandData ("cmdVariRaster");
                 sendPrintfToDevice (pCmd,
                                     0,                     // Color
                                     fTestNoCompression ? 0:1,        // compression
                                     2,                         // Bit width
                                     cbDestBytesInPrinter,  // X Bytes
                                     iNumLines360Band); // Y Pels

                 for (sl = 0; sl < iNumLines360Band; sl++)
                 {
                    pbBuffer = pTempBand + (sl * cbSourceBytesInBitmap);
                    memset(pbExpand, 0, cbDestBytesInPrinter);
                    expand1To2Bpp (pbBuffer, pbExpand, cbDestBytesInPrinter/2, iRemainder, iNumberofPasses);

                    if (!fTestNoCompression)
                    {
                       iCompressed = compressEpsonRLE (pbExpand,
                                                       cbDestBytesInPrinter,
                                                       pbCompress_d,
                                                       cbCompress_d);

                       BinaryData data (pbCompress_d, iCompressed);
                       sendBinaryDataToDevice (&data);
                    }
                    else
                    {
                       BinaryData data (pbExpand, cbDestBytesInPrinter);
                       sendBinaryDataToDevice (&data);
                    }
                 }
                 // End the raster line
                 pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                 sendBinaryDataToDevice (pCmd);
             }

             pCmd = pCommands->getCommandData ("cmdMoveRelativeY");
             sendPrintfToDevice (pCmd,iNumLines360Band);


             // Done with a block of scan lines
             iNumScanLines -= iNumLines360Band;
             iScanLineY    -= iNumLines360Band;
             iWorldY       += iNumLines360Band;
          }

          // Move to the next scan line block size
          iNumLines360Band = (iNumLines360Band+1)/2;
       } // END of while for walking through Band

   } // END 360 X-Res Pass

done:

   pbmi2->cy= iSavecy;
   if(pbCurKPlane)
   {
       memcpy(pbOldKPlane, pbCurKPlane, lBandMemSize);
       free(pbCurKPlane);
   }

   if (pbExpand)
   {
      free (pbExpand);
      pbExpand = 0;
   }

   if (pbRasterLine)
   {
      free (pbRasterLine);
      pbRasterLine= 0;
   }
   if (pbBlanks)
   {
      free (pbBlanks);
      pbBlanks = 0;
   }

   return true;
}

int Epson_High_Res_ESCP2_Blitter::
GetNextRunDrawLocation(int iNumBandSize,
                       int iHeadSpacing,
                       int iBytesPerLine,
                       int iLocation, int iNumberofPasses)
{

   int iLineMove = iNumBandSize/iNumberofPasses;  // this gives us the size of each center line
   int iHalfSpace = iHeadSpacing/2;

   int iReturn = 0;

#ifndef RETAIL
   if(DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream ()  << "__________________________________ iLocation = " << iLocation << std::endl;
#endif

   if(iLocation % iHeadSpacing)
   {
       if(iLocation == iHeadSpacing-1)
       {
          iReturn = iLineMove - (iHalfSpace+1);
       }
       else
       {
           if(iCurrentValue >= iHalfSpace)
           {
              iReturn = iLineMove - (iCurrentValue - iCurrentLowValue);
              iCurrentHighValue--;
              iCurrentValue = iCurrentLowValue;
           }
           else
           {
               iReturn = iLineMove + (iCurrentHighValue - iCurrentValue);
               iCurrentLowValue++;
               iCurrentValue = iCurrentHighValue ;

           }

       }
   }
   else
   {
       iCurrentHighValue = iHeadSpacing;
       iCurrentLowValue = 1;
       iCurrentValue = iHalfSpace;
       iReturn = iCurrentValue + iLineMove;
   }

   return(iReturn);
}

void flipBand(PBYTE pbInOut, PBYTE pbOut,  int iLinesToCopy, int iLineLength)
{
	for(int iCnt = 0 ; iCnt < iLinesToCopy ; iCnt++)
	{
		memcpy(pbOut+(iCnt*iLineLength), pbInOut +((iLinesToCopy-(iCnt+1))*iLineLength), iLineLength);
	}
}

int Epson_High_Res_ESCP2_Blitter::
getNumberofPasses(int iResolution, int iXPhysRes)
{
    if(iResolution < iXPhysRes)  // must be low res
        return 1;
    else
        return iResolution/iXPhysRes;
}

void Epson_High_Res_ESCP2_Blitter::
epsonSplit8for1(PBYTE pInputPlane,
                    PBYTE pOutLeftPlane,
                    PBYTE pOutRghtPlane,
                    int iNumBytes,
                    int iBitValue)
{

 #define BIT_ONE_MASK   0xC0
 #define BIT_TWO_MASK   0x30
 #define BIT_THREE_MASK 0x0C
 #define BIT_FOUR_MASK  0x03

  int ilocate;
  int iplace = 0;
  int iBitMask = 0;

  switch(iBitValue)
  {
      case 0:
          iBitMask = BIT_ONE_MASK;
          break;

      case 1:
          iBitMask = BIT_TWO_MASK;
          break;

      case 2:
          iBitMask = BIT_THREE_MASK;
          break;

      case 3:
          iBitMask = BIT_FOUR_MASK;
          break;
  } // end of switch

  iBitValue *=2;
  for (ilocate = 0; ilocate < iNumBytes; ilocate+=2)
  {

     if(!(ilocate % 8) && (ilocate != 0))
     {
        iplace++;
     }
     // need to take first 2 bits of each byte
     *(pOutLeftPlane+iplace) |= ((*(pInputPlane+ilocate) & iBitMask) << iBitValue) >> (ilocate % 4)*2 ;
     *(pOutRghtPlane+iplace) |= ((*(pInputPlane+ilocate+1) & iBitMask) << iBitValue) >> (ilocate % 4)*2 ;
  }
}

/***********************************************************************/
/*                                                                     */
/*  Function: epsonSplit4for1                                          */
/*                                                                     */
/*    this function goes through the scan line of the bitmap destined  */
/*    for the printer (two bits per pel) and will break it up into     */
/*    for different lines of data.                                     */
/*                                                                     */
/*    Inputs: pInputPlane - input data plane                           */
/*            pOutPlane  -  output data plane                          */
/*            iNumBytes  -  number of input bytes in the scan line     */
/*            iBitValue  -  this refers to what bits need to be        */
/*                          copied into the output line from the       */
/*                          input                                      */
/*                                                                     */
/*  ||||||||||||||||                                                   */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/


void Epson_High_Res_ESCP2_Blitter::
epsonSplit4for1(PBYTE pInputPlane,
                    PBYTE pOutPlane,
                    int iNumBytes,
                    int iBitValue)
{

 #define BIT_ONE_MASK   0xC0
 #define BIT_TWO_MASK   0x30
 #define BIT_THREE_MASK 0x0C
 #define BIT_FOUR_MASK  0x03

  int ilocate;
  int iplace = 0;
  int iBitMask = 0;

  switch(iBitValue)
  {
      case 0:
          iBitMask = BIT_ONE_MASK;
          break;

      case 1:
          iBitMask = BIT_TWO_MASK;
          break;

      case 2:
          iBitMask = BIT_THREE_MASK;
          break;

      case 3:
          iBitMask = BIT_FOUR_MASK;
          break;
  } // end of switch

  iBitValue *=2;
  for (ilocate = 0; ilocate < iNumBytes; ilocate++)
  {

     if(!(ilocate % 4) && (ilocate != 0))
     {
        iplace++;
     }
     // need to take first 2 bits of each byte
     *(pOutPlane+iplace) |= ((*(pInputPlane+ilocate) & iBitMask) << iBitValue) >> (ilocate % 4)*2 ;
  }
}


void Epson_High_Res_ESCP2_Blitter::
epsonSplit2for1(PBYTE pbInputBuffer,
                       PBYTE pbOutputBuffer,
                       int   iNumBytes,
                       int   iBitValue)
{

 #define BIT_ONE_MASK    0xC0
 #define BIT_TWO_MASK    0x30
 #define BIT_THREE_MASK  0x0C
 #define BIT_FOUR_MASK   0x03

    int iplace = 0;
    int iBitMask1 = 0, iBitMask2 = 0;

    switch(iBitValue)
    {
        case 0:
            iBitMask1 = BIT_ONE_MASK;
            iBitMask2 = BIT_THREE_MASK;
            break;

        case 1:
            iBitMask1 = BIT_TWO_MASK;
            iBitMask2 = BIT_FOUR_MASK;
            break;

        default:  // better not be here
            break;
    }
    for (int ilocate = 0; ilocate < iNumBytes; ilocate++)
    {
        if(!iBitValue)
        {
            *(pbOutputBuffer+iplace) |= *(pbInputBuffer+ilocate) & iBitMask1          ; //11000000    <- 11000000
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate++) & iBitMask2) << 2 ; //00110000    <- 00001100
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate) & iBitMask1)   >> 4 ; //00001100    <- 11000000
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate) & iBitMask2)   >> 2 ; //00000011    <- 00001100
        }
        else
        {
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate) & iBitMask1)   << 2 ; //11000000    <- 00110000
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate++) & iBitMask2) << 4 ; //00110000    <- 00000011
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate) & iBitMask1)   >> 2 ; //00001100    <- 00110000
            *(pbOutputBuffer+iplace) |= (*(pbInputBuffer+ilocate) & iBitMask2)        ; //00000011    <- 00000011
        }
        iplace++;
    }
}



bool Epson_High_Res_ESCP2_Blitter::
epsonColorRasterize (PBYTE        pbBits,
                     PBITMAPINFO2 pbmi2,
                     INT          icolortech,
                     PSIZEL       psizelPage,
                     PRECTL       prectlPageLocation,
                     BITBLT_TYPE  eType)
{

#define HIGH_QUALITY 2
#define STD_HIGH_QUALITY 1
#define PHOTO_VALUE STD_HIGH_QUALITY      // 1 - full head pass  2 - half head pass  4 - 1/4 head pass

#define SETSHIFTVALUE                                       \
    if(iShiftValue < iNozzleSpace-1)                        \
    {                                                       \
        if(iShiftValue == 0)                                \
        {                                                   \
            if(iMoveRight)                                  \
                {                                           \
                  iMoveRight++;                             \
                  if(iMoveRight == iNumberofPasses)         \
                      iMoveRight = 0;                       \
                }                                           \
            else                                            \
                iMoveRight++;                               \
        }                                                   \
        else                                                \
          if(iShiftValue == (iNozzleSpace/2))               \
              if(iMoveRight)                                \
                  {                                         \
                    iMoveRight++;                           \
                    if(iMoveRight == iNumberofPasses)       \
                        iMoveRight = 0;                     \
                  }                                         \
              else                                          \
                  iMoveRight++;                             \
       iShiftValue++;                                       \
    }                                                       \
    else                                                    \
    {                                                       \
        iShiftValue = 0;                                    \
        if(iMoveRight)                                      \
        {                                                   \
            iMoveRight++;                                   \
            if(iMoveRight == iNumberofPasses)               \
                 iMoveRight = 0;                            \
        }                                                   \
        else                                                \
            iMoveRight++;                                   \
    };


#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   Epson_High_Res_ESCP2_Instance *pInstance = dynamic_cast <Epson_High_Res_ESCP2_Instance *>(getInstance ());
   if (!pInstance)
      return false;

   DeviceResolution *pDR           = getCurrentResolution ();
   DeviceData       *pDevData      = getDeviceData();
   DeviceForm       *pForm         = getCurrentForm ();

   int               cy            = pbmi2->cy;

   int               iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter,
                     cbDestBytesInLine;
   int               iSaveCy;
   int               iMaxPass,
                     iLastColor    = -1;   /* Not a valid color to start with */
   PBYTE             pbBuffer;
   int               iCompressed;
   BinaryData       *pbdColorBits  = 0;
   DeviceCommand    *pCommands     = getCommands ();
   BinaryData       *pCmd          = 0;
   register long     sl;

   PBYTE   pbRasterLine = 0;
   PBYTE   pbRasterRtLine = 0;  // used for 2880 support
   PBYTE   pbBlanks = 0;

   PBYTE   pbCurCPlane  = 0;
   PBYTE   pbCurMPlane  = 0;
   PBYTE   pbCurYPlane  = 0;
   PBYTE   pbCurKPlane  = 0;
   PBYTE   pbCurLCPlane = 0;
   PBYTE   pbCurLMPlane = 0;

   // current planes to hold reordered lines

   PBYTE   pbPrevious = 0;

   PBYTE   pbCurrentBand = 0;

   int     iNumScanLines;     // how far down the page we are

   long    lBandMemSize = 0;
   long    lStartBandMemSize;     //@@START

   bool    bOutputComplete = false;

   int     iStripeSize;

   int     iLineMoveSize;   // number of lines to move on the page
   int     iStartBufferLines = 0;   //@@START

   int     iNumberofNozzles;

   int     iXPos;                       // physical XPos

   int     iNumberofPasses;             // number of passes we have to do for the resolution
   int     iNumberofRealPasses;         // number of real H/W passes we have to do for the resolution
   int     iNozzleSpace;
   bool    bBlankBand = false;
   bool    bRet = true;


   if(icolortech == DevicePrintMode::COLOR_TECH_CcMmYK)
   {
       iMaxPass = 19;
   }
   else
   {
       iMaxPass = 5;
   }

   iStartPassVal = 0;

   if(!pDevData)                     // make sure we have DeviceData
   {
#ifndef RETAIL
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Fatal Error -- failure to get printhead data " << std::endl;
#endif
       return false;
   }

                // Get the setup information out of the DeviceData
   pDevData->getIntData("Nozzle_Number", &iNumberofNozzles);
   pDevData->getIntData("Nozzle_Spacing", &iNozzleSpace);
   pDevData->getIntData("Positioning_x", &iXPos);
   iSaveCy = pbmi2->cy;

   iNumberofNozzles /= PHOTO_VALUE;   // lower the number of lines

   iStripeSize = iNozzleSpace * iNumberofNozzles;  // Band size needs to be set to a multiple of the nozzle spacing
                                                 // and not to exceed the size of iNozzleSpace * iNumberofNozzles

   iNumberofRealPasses = getNumberofPasses( pDR->getXRes (),iXPos );   // Get the current resolution
   iNumberofPasses = (iNumberofRealPasses > 4) ? iNumberofRealPasses/2 : iNumberofRealPasses;  //2880

   if(!iNumberofPasses)
       return false;  // we should not be here or routine will fail

   std::string *pstringOrientation = 0;

   pstringOrientation = getCurrentOrientation ()->getRotation ();

   if (  !pstringOrientation
      || 0 == pstringOrientation->compare ("Portrait")
      )
   {
      iWorldY    = iNumPageLines - prectlPageLocation->yTop;
   }
   else
   {
      iWorldY    = prectlPageLocation->xRight; //@tbd need to handle this as in portrait case
   }

   delete pstringOrientation;

   if(lCurPageLocation != 0)   // reset - if iWorldY is lower than lCurPageLocation then we are
   {                           // into another page
       if(lCurPageLocation > (long)iWorldY)
       {
           iStartPassVal           = 0;
           InitializePageValues(iNumberofPasses);
       }
   }

   if(bStartPage)   // setup fake data at the beginning so that we don't have to make the code
   {                // for doing the startup of the page any worse than it already will be
       iStartBufferLines = iStripeSize+5;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;

   cbDestBytesInLine  = ((pbmi2->cx *2 )+ 7) >> 3;
   cbDestBytesInPrinter = cbDestBytesInLine/iNumberofRealPasses;

   if(iNumberofPasses > 1)  // non-360 X-resolution output
   {
	   PBYTE pbTempBand = 0;
       // the first time though we only want to setup the previous band.  No need to
       // do anything with the data here

       if(!bNotFirstPass)  // NOT - NOT first pass so this must be first pass code ;-)
       {
          lStartBandMemSize =  cbDestBytesInLine *(iStartBufferLines+cy);  //@@START

          GetMemory(&pbOldCPlane, lStartBandMemSize, "pbOldCPlane");

          GetMemory(&pbOldMPlane, lStartBandMemSize, "pbOldMPlane");

          GetMemory(&pbOldYPlane, lStartBandMemSize, "pbOldYPlane");

          GetMemory(&pbOldKPlane, lStartBandMemSize, "pbOldKPlane");

          GetMemory(&pbTempBand, iNumberofNozzles*cbSourceBytesInBitmap, "pbTempBand");

          if(!pbOldCPlane || !pbOldMPlane || !pbOldYPlane || !pbOldKPlane || !pbTempBand)
          {
              bRet = false;
              goto done;
          }

          if(icolortech == DevicePrintMode::COLOR_TECH_CcMmYK)
          {

              if(!GetMemory(&pbOldLCPlane, lStartBandMemSize, "pbOldLCPlane"))
              {
                  bRet = false;
                  goto done;
              }

              if(!GetMemory(&pbOldLMPlane, lStartBandMemSize, "pbOldLMPlane"))
              {
                  bRet = false;
                  goto done;
              }
          }

    	  pbmi2->cy = iNumberofNozzles;  // should always be larger than a nozzle pass

    	  for(iScanLineY = iNumberofNozzles; iScanLineY <= cy ; iScanLineY+=iNumberofNozzles)
    	  {
              int iBufLoc = (iScanLineY-iNumberofNozzles+iStartBufferLines) * cbDestBytesInLine;
              int iNumBytes = pbmi2->cy*cbDestBytesInLine;

              /*  |---------------------|
                  |  \ starting band    |
                  |   \                 |<-----+
                  |---------------------|      +    <-data taken from here and flipped
                  |     \               |<----++        Then put here+
                  |      \              |     ++                     +
                  |---------------------|     ++                     +
                  |        \            |<---+++                     +
                  |         \           |    +++                     +
                  |---------------------|    +++                     +
                  |           \         |<--++++                     +
                  |            \        |   ++++                     +
                  |---------------------|   ++++                     +
                  |              \      |<-+++++                     +
                  |               \     |  +++++                     +
                  |---------------------|  +++++                     +
                                           +++++                     +
                  |---------------------|  +++++                     +
                  |  finished band /    |  +++++                     +
                  |               /     |<-+++++                     +
                  |---------------------|   ++++                     +
                  |             /       |<--++++                     +
                  |            /        |    +++                     +
                  |---------------------|    +++                     +
                  |          /          |    +++                     +
                  |         /           |<---+++                     +
                  |---------------------|     ++                     +
                  |       /             |<----++                     +
                  |      /              |      +                     +
                  |---------------------|      +                     +
                  |    /                |<---- +                     +
                  |   /                 |<---------------------------+
                  |---------------------|

                  */

    		  pbBuffer = pbBits + ((cy - iScanLineY)* cbSourceBytesInBitmap);

			  flipBand(pbBuffer, pbTempBand, iNumberofNozzles, cbSourceBytesInBitmap);

			  ditherRGBtoCMYK (pbmi2, pbTempBand);

              if(!ditherAllPlanesBlank ())      //@@SKIP
              {
                  if(bStillWhite)
                  {
                      bStillWhite = false;
                  }
                  if (!ditherCPlaneBlank ())
                  {
                         memcpy(pbOldCPlane + iBufLoc, getCPlane()->getData(), iNumBytes);
                  }

                  if (!ditherMPlaneBlank ())
                  {
                         memcpy(pbOldMPlane + iBufLoc, getMPlane()->getData(), iNumBytes);
                  }

                  if (!ditherYPlaneBlank ())
                  {
                         memcpy(pbOldYPlane + iBufLoc, getYPlane()->getData(), iNumBytes);
                  }

                  if (!ditherKPlaneBlank ())
                  {
                         memcpy(pbOldKPlane + iBufLoc, getKPlane()->getData(), iNumBytes);
                  }

                  if(icolortech == DevicePrintMode::COLOR_TECH_CcMmYK)
                  {
                      if (!ditherLCPlaneBlank ())
                      {
                             memcpy(pbOldLCPlane + iBufLoc, getLCPlane()->getData(), iNumBytes);
                      }

                      if (!ditherLMPlaneBlank ())
                      {
                             memcpy(pbOldLMPlane + iBufLoc, getLMPlane()->getData(), iNumBytes);
                      }
                  }
              }
              else   //@@SKIP
              {
//                  lWhiteSpaceIncrement += pbmi2->cy;
              }

#ifndef RETAIL
              if (DebugOutput::shouldOutputBlitter ())
              {
                 DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << ": OOOOOOOOOOOOOUUUUUUUUUUt" << std::endl;
                 DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << ": bStillWhite = " << bStillWhite << std::endl;
              }
#endif
          }
		  if(pbTempBand)
			  free(pbTempBand);

          bNotFirstPass = true;
          lFirstPlane_cy = iSaveCy ;
          // We need to set the top of page on some devices so we will set it here.
          DeviceResolution *pRes      = getCurrentResolution ();
          DeviceCommand    *pCommands = getCommands ();
          BinaryData       *pCmd      = 0;

          DeviceData       *pData     = 0;  //@@PAZ

          int iLinesTopMargin = (int)((float) pRes->getYRes () * ((float)pForm->getHardCopyCap ()->getTopClip ()/ 25400.0));
          bool bRet = false;
          pData     = getDeviceData ();  //@@PAZ

          /* Some printers need to have the offset of the page sent down at the beginning of the page */

          if(pData)
          {

              int iStart = 0;
              bRet =  pData->getIntData ("OffsetStart", &iStart);
              if(bRet && iStart)
              {
                  pCmd = pCommands->getCommandData ("cmdMoveRelativeY");
                  if(pCmd)
                  {
                      sendPrintfToDevice (pCmd,iLinesTopMargin);
                  }
                  else
                  {
#ifndef RETAIL
                     if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "Error: There is no cmdMoveRelativeY defined for this device!" << std::endl;
#endif
                  }
              }
          }

          /***********  We only wanted to setup the bands on first pass so leave now ******/

         return bRet;

       }
       else
       {
          // copy off the last band
          // Setup our band data
           lBandMemSize =  cbDestBytesInLine*cy;    // allocate memory to hold previous band

           GetMemory(&pbCurCPlane, lBandMemSize, "pbCurCPlane");

           GetMemory(&pbCurMPlane, lBandMemSize, "pbCurMPlane");

           GetMemory(&pbCurYPlane, lBandMemSize, "pbCurYPlane");

           GetMemory(&pbCurKPlane, lBandMemSize, "pbCurKPlane");

           GetMemory(&pbTempBand, cy*cbSourceBytesInBitmap, "pbTempBand");

           if(!pbCurCPlane || !pbCurMPlane || !pbCurYPlane || !pbCurKPlane || !pbTempBand )
           {
               bRet = false;
               goto done;
           }

           if(icolortech == DevicePrintMode::COLOR_TECH_CcMmYK)
           {
               if(!GetMemory(&pbCurLCPlane, lBandMemSize, "pbCurLCPlane"))
               {
                   bRet = false;
                   goto done;
               }

               if(!GetMemory(&pbCurLMPlane, lBandMemSize, "pbCurLMPlane"))
               {
                   bRet = false;
                   goto done;
               }
           }

           pbmi2->cy = cy > iNumberofNozzles ? iNumberofNozzles : cy;  // send down a full head pass

		   //iScanLineY = iNumberofNozzles < cy ? iNumberofNozzles : cy ;

           flipBand(pbBits, pbTempBand, cy, cbSourceBytesInBitmap);

           iScanLineY = 0;

#ifndef RETAIL
           if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << ": bStillWhite = " << bStillWhite << std::endl;
#endif

		   while(iScanLineY < cy)
           {
               int iBufLoc = iScanLineY * cbDestBytesInLine;
               int iNumBytes;

			   if((iScanLineY == cy) || (iNumberofNozzles <= (cy - iScanLineY)))
				   pbmi2->cy = iNumberofNozzles;
			   else
				   pbmi2->cy = cy - iScanLineY;

               iNumBytes = pbmi2->cy*cbDestBytesInLine;

#ifndef RETAIL
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "Epson_High_Res_ESCP2_Blitter::" << __FUNCTION__ << ": iScanLineY " << iScanLineY << " - cy = " << cy << std::endl;
#endif

               pbBuffer = pbTempBand + (iScanLineY * cbSourceBytesInBitmap);

               ditherRGBtoCMYK (pbmi2, pbBuffer);

               if(!ditherAllPlanesBlank ())      //@@SKIP
               {
                   lWhiteSpaceIncrement = 0;
                   if(bStillWhite)
                   {
                       bStillWhite = false;
                   }
                   if (!ditherCPlaneBlank ())
                   {
                       memcpy(pbCurCPlane + iBufLoc, getCPlane()->getData(), iNumBytes);
                   }

                   if (!ditherMPlaneBlank ())
                   {
                       memcpy(pbCurMPlane + iBufLoc, getMPlane()->getData(), iNumBytes);
                   }

                   if (!ditherYPlaneBlank ())
                   {
                       memcpy(pbCurYPlane + iBufLoc, getYPlane()->getData(), iNumBytes);
                   }

                   if (!ditherKPlaneBlank ())
                   {
                       memcpy(pbCurKPlane + iBufLoc, getKPlane()->getData(), iNumBytes);
                   }

                   if(icolortech == DevicePrintMode::COLOR_TECH_CcMmYK)
                   {
                       if (!ditherLCPlaneBlank ())
                       {
                           memcpy(pbCurLCPlane + iBufLoc, getLCPlane()->getData(), iNumBytes);
                       }

                       if (!ditherLMPlaneBlank ())
                       {
                           memcpy(pbCurLMPlane + iBufLoc, getLMPlane()->getData(), iNumBytes);
                       }
                   }
               }
               else   //@@SKIP
               {
                  lWhiteSpaceIncrement += pbmi2->cy;
               }

			   iScanLineY += pbmi2->cy;

           }
		   if(pbTempBand)
			   free(pbTempBand);

           lSecondPlane_cy = iSaveCy;
       }

       // set up our blank information here so we can use blank lines and bands when needed
       if(!GetMemory(&pbRasterLine,cbDestBytesInPrinter, "pbRasterLine"))
       {
           bRet = false;
           goto done;
       }

       if(iNumberofRealPasses > 4)
       {
           if(!GetMemory(&pbRasterRtLine, cbDestBytesInPrinter, "pbRasterRtLine"))
           {
               bRet = false;
               goto done;
           }
       }

       if(!GetMemory(&pbBlanks, cbDestBytesInLine, "pbBlanks"))
       {
           bRet = false;
           goto done;
       }

       iNumScanLines = omni::min((int)(lSecondPlane_cy+lFirstPlane_cy), iWorldY+1);

#ifndef RETAIL
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "cbDestBytesInLine = " << cbDestBytesInLine << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iScanLineY = " << iScanLineY << std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iNumScanLines = " << iNumScanLines << std::endl;
#endif

       // set everything up so we can tell which band we will need to get the
       // data out of

       if(lSecondBandLocation == 0)
       {
           lSecondBandLocation = lFirstPlane_cy;
       }
       else
       {
           lFirstBandLocation = lSecondBandLocation;
           lSecondBandLocation += lFirstPlane_cy;
       }

       //@@LASTBAND
       if((iSaveCy % iStripeSize) || ((lSecondBandLocation+iSaveCy)>= iNumPageLines))
       {
           bLastBand = true;
       }

         // set condition so that we know we are at the end of the page and have to change
         // the values of the number of lines that are going down to the printer
#ifndef RETAIL
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= iNumPageLines = "     << iNumPageLines <<std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= lSecondPlane_cy = "      << lSecondPlane_cy <<std::endl;
       if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "======= lSecondBandLocation = " << lSecondBandLocation <<std::endl;
#endif


       while (!bOutputComplete)        // need to check the size and make sure we are not out of the band on the bottom
       {                                   // lines of our output

             // setup a rotate scheme so that we know when we need to shift right the bits

          if(((lCurPageLocation+(long)iStripeSize) < (lSecondBandLocation + lSecondPlane_cy)) || bLastBand )  // check to see if we can do another pass
          {
              if((iNumberofPasses == 2) || (iNumberofPasses == 4))  // 720 Y resolution
              {
                SETSHIFTVALUE
              }

         /* Loop through the four color passes/planes:
         **     YELLOW .. MAGENTA .. CYAN .. BLACK
         */
             for (int iPass = 0; iPass < iMaxPass; iPass++)
             {
                 switch (iPass)
                 {
                   case 4: /* YELLOW */
                   {
                          pbPrevious = pbOldYPlane;
                          pbCurrentBand = pbCurYPlane;

                      break;
                   }

                   case 1: /* MAGENTA */
                   {
                           pbPrevious = pbOldMPlane;
                           pbCurrentBand = pbCurMPlane;

                      break;
                   }

                   case 2: /* CYAN */
                   {
                           pbPrevious = pbOldCPlane;
                           pbCurrentBand = pbCurCPlane;

                      break;
                   }

                   case 0: /* BLACK */
                   {
                           pbPrevious = pbOldKPlane;
                           pbCurrentBand = pbCurKPlane;

                      break;
                   }

                   case 18: /* Light Cyan */
                   {

                           pbPrevious = pbOldLCPlane;
                           pbCurrentBand = pbCurLCPlane;

                      break;
                   }
                   case 17: /* Light Magenta */
                   {

                           pbPrevious = pbOldLMPlane;
                           pbCurrentBand = pbCurLMPlane;

                      break;
                   }

                   default:
                      continue;
                 }

                 iLastColor = iPass;

                     // Can't go past the end of the second band bottom
                 if(((lCurPageLocation + (long)iStripeSize) <  (lSecondBandLocation+lSecondPlane_cy)) && !bLastBand)
                 {

                     if(!bStartPage)
                     {
                         if(!bStillWhite && (lWhiteSpaceIncrement <(3*iStripeSize)))
                         {
                             if(iMoveRight)
                             {
                                 pCmd = pCommands->getCommandData ("cmdPositionX1");
                                 sendPrintfToDevice (pCmd,iMoveRight,0,0,0);

                             }

                             pCmd = pCommands->getCommandData ("cmdVariRaster");
                             sendPrintfToDevice (pCmd,
                                                 iPass,                     // Color
                                                 fTestNoCompression ? 0:1,        // compression
                                                 2,                         // Bit width
                                                 cbDestBytesInPrinter,  // X pels
                                                 iNumberofNozzles); // Y Pels
    /*
       Since we have to maintain two bands of data because the stripe we wish to send
       could extend across both bands at any given time.  The way we do that is that
       we maintain the current page location - lCurPageLocation and each of the locations
       of the bands we wish to send to the printer.  This allows us to use the band bottom
       as a zero reference.  This coming from subtracting the band location from the
       current location.

       We then loop through the bands breaking up each of the lines and sending what lines we need
       to the printer each time through based on the number of passes needed.

    */

                             for (sl = 0; sl < iNumberofNozzles; sl++)
                             {
                                  // data is coming out of top band
                                if(((long)(sl*iNozzleSpace)+lCurPageLocation) < lSecondBandLocation)
                                {
                                    long lTopOffset = lCurPageLocation - lFirstBandLocation;
                                    pbBuffer = pbPrevious + (( ((long)(sl * iNozzleSpace)) + lTopOffset) * cbDestBytesInLine);
                                }
                                else
                                { // data is coming out of the bottom band
                                    long lLowerOffset = lCurPageLocation - lSecondBandLocation;
                                    pbBuffer = pbCurrentBand + (( ((long)(sl * iNozzleSpace))+lLowerOffset) * cbDestBytesInLine);
                                }

                                memset(pbRasterLine, 0, cbDestBytesInPrinter);

                                if(iNumberofRealPasses > 4)
                                {
                                    memset(pbRasterRtLine, 0, cbDestBytesInPrinter);
                                }

                                // check to see which set of bits need to be sent down to the printer
                                switch(iNumberofRealPasses)
                                {
                                case 2:
                                    epsonSplit2for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight);
                                    break;

                                case 4:
                                    epsonSplit4for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight );
                                    break;

                                case 8:
                                    break;

                                default:
                                    break;
                                }

                                if (!fTestNoCompression)
                                {
                                   // @TBD - rewrite!

                                   iCompressed = compressEpsonRLE (pbRasterLine,
                                                                   cbDestBytesInPrinter,
                                                                   pbCompress_d,
                                                                   cbCompress_d);

                                   BinaryData data (pbCompress_d, iCompressed);
                                   sendBinaryDataToDevice (&data);
                                }
                                else
                                {
                                   BinaryData data (pbRasterLine, cbDestBytesInPrinter);
                                   sendBinaryDataToDevice (&data);
                                }

                             }  // end of send to printer

                             pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                             sendBinaryDataToDevice (pCmd);
                         }

                     }
                     else
                     {
                         int iLinesToSend;
                         if(lCurPageLocation < 0)
                             iLinesToSend = (lCurPageLocation+iStripeSize)/iNozzleSpace;
                         else
                             iLinesToSend = iNumberofNozzles;

                         if(iLinesToSend)
                         {
                             if(!bStillWhite && (lWhiteSpaceIncrement < (3*iStripeSize)))
                             {
                                 if(iMoveRight)
                                 {
                                     pCmd = pCommands->getCommandData ("cmdPositionX1");
                                     sendPrintfToDevice (pCmd,iMoveRight,0,0,0);

                                 }

                                 pCmd = pCommands->getCommandData ("cmdVariRaster");
                                 sendPrintfToDevice (pCmd,
                                                     iPass,                     // Color
                                                     fTestNoCompression ? 0:1,        // compression
                                                     2,                         // Bit width
                                                     cbDestBytesInPrinter,  // X pels
                                                     iLinesToSend); // Y Pels

                                 for (sl = 0; sl < iNumberofNozzles; sl++)
                                 // @tbd make sure low res blitter and mono increment the same way
                                 {
                                      // data is coming out of top band
                                    if(((long)(sl*iNozzleSpace)+lCurPageLocation) >= 0)
                                    {
                                        if(((long)(sl*iNozzleSpace)+lCurPageLocation) < lSecondBandLocation)
                                        {
                                            pbBuffer = pbPrevious + (((lCurPageLocation + iStartBufferLines + (long)(sl * iNozzleSpace)) - lFirstBandLocation) * cbDestBytesInLine);
                                        }
                                        else
                                        { // data is coming out of the bottom band
                                            pbBuffer = pbCurrentBand + (((lCurPageLocation + (long)(sl * iNozzleSpace))- lSecondBandLocation) * cbDestBytesInLine);
                                        }

                                        memset(pbRasterLine, 0, cbDestBytesInPrinter);

                                        // check to see which set of bits need to be sent down to the printer
                                        switch(iNumberofRealPasses)
                                        {
                                        case 2:
                                            epsonSplit2for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight);
                                            break;

                                        case 4:
                                            epsonSplit4for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight );
                                            break;

                                        case 8:
                                            break;

                                        default:
                                            break;
                                        }

                                        if (!fTestNoCompression)
                                        {
                                           // @TBD - rewrite!

                                           iCompressed = compressEpsonRLE (pbRasterLine,
                                                                           cbDestBytesInPrinter,
                                                                           pbCompress_d,
                                                                           cbCompress_d);

                                           BinaryData data (pbCompress_d, iCompressed);
                                           sendBinaryDataToDevice (&data);
                                        }
                                        else
                                        {
                                           BinaryData data (pbRasterLine, cbDestBytesInPrinter);
                                           sendBinaryDataToDevice (&data);
                                        }
                                    }

                                 }  // end of send to printer

                                 pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                                 sendBinaryDataToDevice (pCmd);
                             }
                         }
                     }
                 }
                 else
                 {

                     if(bLastBand)  // we are finishing out the page
                     {
                         int iNumLines = ((lSecondBandLocation + lSecondPlane_cy)-lCurPageLocation)/iNozzleSpace > iNumberofNozzles ?
                                           iNumberofNozzles : ((lSecondBandLocation + lSecondPlane_cy)-lCurPageLocation)/iNozzleSpace;

#ifndef RETAIL
                         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "CURRENT PAGE LOCATION = " << lCurPageLocation << std::endl;
                         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () <<"lines to send = " << iNumLines << std::endl;
                         if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "bottom = " << (lFirstBandLocation + lFirstPlane_cy) << std::endl;
#endif
                         if(!bStillWhite && (lWhiteSpaceIncrement <(3*iStripeSize)))
                         {
                             if(iMoveRight)
                             {
                                 pCmd = pCommands->getCommandData ("cmdPositionX1");
                                 sendPrintfToDevice (pCmd,iMoveRight,0,0,0);
                             }

                             pCmd = pCommands->getCommandData ("cmdVariRaster");
                             sendPrintfToDevice (pCmd,
                                                 iPass,                     // Color
                                                 fTestNoCompression ? 0:1,        // compression
                                                 2,                         // Bit width
                                                 cbDestBytesInPrinter,  // X pels
                                                 iNumLines); // Y Pels

                             for (sl = 0; sl < iNumLines; sl++)
                             {
                                  // data is coming out of top band
                                if(((sl*iNozzleSpace)+lCurPageLocation) < (lSecondBandLocation))
                                {
                                    pbBuffer = pbPrevious + ((lCurPageLocation + (long)(sl * iNozzleSpace) - lFirstBandLocation) * cbDestBytesInLine);
                                }
                                else
                                { // No more data is available in the bands
                                    pbBuffer = pbCurrentBand + (((lCurPageLocation + (long)(sl * iNozzleSpace))- lSecondBandLocation) * cbDestBytesInLine);
                                }

                                memset(pbRasterLine, 0, cbDestBytesInPrinter);

                                // check to see which set of bits need to be sent down to the printer
                                switch(iNumberofRealPasses)
                                {
                                case 2:
                                    epsonSplit2for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight);
                                    break;

                                case 4:
                                    epsonSplit4for1(pbBuffer, pbRasterLine, cbDestBytesInLine, iMoveRight );
                                    break;

                                case 8:
                                    break;

                                default:
                                    break;
                                }


                                if (!fTestNoCompression)
                                {
                                   iCompressed = compressEpsonRLE (pbRasterLine,
                                                                   cbDestBytesInPrinter,
                                                                   pbCompress_d,
                                                                   cbCompress_d);

                                   BinaryData data (pbCompress_d, iCompressed);
                                   sendBinaryDataToDevice (&data);
                                }
                                else
                                {
                                   BinaryData data (pbRasterLine, cbDestBytesInPrinter);
                                   sendBinaryDataToDevice (&data);
                                }


                             }  // end of send to printer

                             pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                             sendBinaryDataToDevice (pCmd);
                         }

                               // set condition to get out of finishup code
                         if((iPass == (iMaxPass-1)) && (lCurPageLocation >= (iNumPageLines-1)))
                         {
                             bOutputComplete = true;
                         }
                     }
                     else       // if this is not the last band, get more data
                     {
                         if(iPass == (iMaxPass-1))
                         {
                             bOutputComplete = true;
                         }
                     }
                 }
                      // End the raster line
             }   // end of BANDS - iPass

             pCmd = pCommands->getCommandData ("cmdMoveRelativeY");

             // if we are starting the page, we need to fill in bands to start off with
             iLineMoveSize = GetNextRunDrawLocation(iNumberofNozzles,
                                                    iNozzleSpace,
                                                    cbDestBytesInPrinter, iShiftValue, iNumberofPasses);

             lCurPageLocation += (long)iLineMoveSize;  // increment down our page location

             // we have to do the beginning of the page stuff here
             // since we need to try and fill the top band on the page we need
             // to make sure that we can sync up with the actual beginning page location
             if(iRealPos >= 0)
             {
                 if(lCurPageLocation < 0)
                 {
                     if(iStartPassVal++ % iNumberofPasses)
                     {
                         iLineMoveSize = 0;
                     }
                     else
                     {
                         iLineMoveSize = 1;
                         iRealPos++;
                     }
                 }
                 else
                 {
                     if(lCurPageLocation > (long)iRealPos)
                     {
                         iLineMoveSize = (int)lCurPageLocation - iRealPos ;
                         iRealPos = -1;
                     }
                     else
                     {
                         if(iStartPassVal++ % iNumberofPasses)
                         {
                             iLineMoveSize = 0;
                         }
                         else
                         {
                             iLineMoveSize = 1;
                             iRealPos++;
                         }
                     }
                 }
                 if(iStartPassVal == iNumberofPasses)
                     iStartPassVal = 0;
             }

#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "iLineMoveSize = " << iLineMoveSize << std::endl;
#endif

             // @@LASTBAND - don't go past end of page
             if((lCurPageLocation + (long)iLineMoveSize) <(long) iNumPageLines)
                 sendPrintfToDevice (pCmd,iLineMoveSize);
             else
                 lCurPageLocation = iNumPageLines;
          }
          else
          {
             bOutputComplete = true;
          }
       }
       if(bStartPage)
           bStartPage = false;
   }
   else
   {   // 360 X-resolution blitter for non-FINE output

       PBYTE pTempBand;
       int iNumLines360Band = iNumberofNozzles;
       iNumLines360Band /=2;

       iScanLineY            = cy - 1;

       iNumScanLines = omni::min (cy, prectlPageLocation->yTop + 1);

       while (iNumScanLines)
       {
          while (iNumScanLines >= iNumLines360Band)
          {

             pbmi2->cy = iNumLines360Band;

#ifndef RETAIL
             if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "indexing to " << (iScanLineY - iNumLines360Band + 1) * cbSourceBytesInBitmap << std::endl;
#endif

             GetMemory(&pTempBand, iNumLines360Band * cbSourceBytesInBitmap, "pTempBand");

             // We're upside down with we dither so we need to switch the band memory around.

             for(int iLineValue = 0; iLineValue < iNumLines360Band; iLineValue++)
             {
                 memcpy(pTempBand+(iLineValue*cbSourceBytesInBitmap),
                                pbBits+((iScanLineY-iLineValue)* cbSourceBytesInBitmap),
                        cbSourceBytesInBitmap);
             }

             pbBuffer = pTempBand;

             ditherRGBtoCMYK (pbmi2, pbBuffer);

             if(pTempBand)
               free(pTempBand);

             if (ditherAllPlanesBlank ())
             {
                incrementBlankLineCount (iNumLines360Band);
                bBlankBand = true;
             }
             else
             {
                // Set Absolute Vertical Print Position only when there is a blank band
                if(bBlankBand)
                {
                    pCmd = pCommands->getCommandData ("cmdSetYPos");

#ifndef RETAIL
                    if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "setting position to " << iWorldY << std::endl;
#endif

                    sendPrintfToDevice (pCmd, iWorldY);
                    bBlankBand = false;
                }

                /* Loop through the four color passes/planes:
                **     YELLOW .. MAGENTA .. CYAN .. BLACK
                */
                for (int iPass = 0; iPass < iMaxPass; iPass++)
                {
                   switch (iPass)
                   {
                       case 0: /* BLACK */
                       {
                          if (ditherKPlaneBlank ())
                             continue;

                          pbdColorBits = getKPlane ();

                          break;
                       }

                       case 1: /* MAGENTA */
                       {
                          if (ditherMPlaneBlank ())
                             continue;

                          pbdColorBits = getMPlane ();

                          break;
                       }

                       case 2: /* CYAN */
                       {
                          if (ditherCPlaneBlank ())
                             continue;

                          pbdColorBits = getCPlane ();

                          break;
                       }

                       case 3: /* NOTHING */
                       {
                             continue;
                       }

                       case 4: /* YELLOW */
                       {
                          if (ditherYPlaneBlank ())
                              continue;

                          pbdColorBits = getYPlane ();

                          break;
                       }
                       case 18: /* Light Cyan */
                       {
                           if (ditherLCPlaneBlank ())
                              continue;

                           pbdColorBits = getLCPlane ();

                          break;
                       }
                       case 17: /* Light Magenta */
                       {
                           if (ditherLMPlaneBlank ())
                              continue;

                           pbdColorBits = getLMPlane ();

                          break;
                       }
                       default:
                           continue;
                   }

                   iLastColor = iPass;
                   // Send raster transfer header
                   pCmd = pCommands->getCommandData ("cmdVariRaster");
                   sendPrintfToDevice (pCmd,
                                       iPass,                     // Color
                                       fTestNoCompression ? 0:1,        // compression
                                       2,                         // Bit width
                                       cbDestBytesInPrinter,  // X Bytes
                                       iNumLines360Band); // Y Pels

                  for (sl = 0; sl < iNumLines360Band; sl++)
                   {
                      pbBuffer = pbdColorBits->getData () + sl * cbDestBytesInPrinter;

                      if (!fTestNoCompression)
                      {
                         iCompressed = compressEpsonRLE (pbBuffer,
                                                         cbDestBytesInPrinter,
                                                         pbCompress_d,
                                                         cbCompress_d);

                         BinaryData data (pbCompress_d, iCompressed);
                         sendBinaryDataToDevice (&data);
                      }
                      else
                      {
                         BinaryData data (pbBuffer, cbDestBytesInPrinter);
                         sendBinaryDataToDevice (&data);
                      }
                   }

                   // End the raster line
                   pCmd = pCommands->getCommandData ("cmdEndRasterGraphicsLine");
                   sendBinaryDataToDevice (pCmd);
                }
                pCmd = pCommands->getCommandData ("cmdMoveRelativeY");
                sendPrintfToDevice (pCmd,iNumLines360Band);
             }


             // Done with a block of scan lines
             iNumScanLines -= iNumLines360Band;
             iScanLineY    -= iNumLines360Band;
             iWorldY       += iNumLines360Band;
          }

          // Move to the next scan line block size
          iNumLines360Band = (iNumLines360Band+1)/2;
       } // END of while for walking through Band

   } // END 360 X-Res Pass

done:
   pbmi2->cy = iSaveCy;

   lFirstPlane_cy = lSecondPlane_cy;

   // free up plane memory
   if(pbCurCPlane)
   {             // copy off the current band to the old band
       memcpy(pbOldCPlane, pbCurCPlane, lBandMemSize);
       free(pbCurCPlane);
   }
   if(pbCurMPlane)
   {
       memcpy(pbOldMPlane, pbCurMPlane, lBandMemSize);
       free(pbCurMPlane);
   }
   if(pbCurYPlane)
   {
       memcpy(pbOldYPlane, pbCurYPlane, lBandMemSize);
       free(pbCurYPlane);
   }
   if(pbCurKPlane)
   {
       memcpy(pbOldKPlane, pbCurKPlane, lBandMemSize);
       free(pbCurKPlane);
   }
   if(pbCurLCPlane)
   {
       memcpy(pbOldLCPlane, pbCurLCPlane, lBandMemSize);
       free(pbCurLCPlane);
   }
   if(pbCurLMPlane)
   {
       memcpy(pbOldLMPlane, pbCurLMPlane, lBandMemSize);
       free(pbCurLMPlane);
   }

   if (pbRasterLine)
   {
      free (pbRasterLine);
      pbRasterLine= 0;
   }
   if (pbRasterRtLine)
   {
      free (pbRasterRtLine);
      pbRasterRtLine= 0;
   }
   if (pbBlanks)
   {
      free (pbBlanks);
      pbBlanks = 0;
   }

   return bRet;
}

#ifndef RETAIL

void Epson_High_Res_ESCP2_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string Epson_High_Res_ESCP2_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{Epson_High_Res_ESCP2_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Epson_High_Res_ESCP2_Blitter& const_self)
{
   Epson_High_Res_ESCP2_Blitter& self = const_cast<Epson_High_Res_ESCP2_Blitter&>(const_self);
   std::ostringstream            oss;

   os << self.toString (oss);

   return os;
}


/****************************************************************************/
/*                                                                          */
/*  PROCEDURE NAME: expand1to2Bpp                                           */
/*                                                                          */
/*  This procedure takes our input scanline and doubles its size copying    */
/*  the set bits to the target scanline.  The mask can be altered to set    */
/*  any particular pel setting.                                             */
/*                                                                          */
/*  This also provides the ability to not set the bits in the target line   */
/*  based on the number of bits that should have not been set in the source.*/
/*                                                                          */
/*                                                                          */
/****************************************************************************/


void
expand1To2Bpp (PBYTE        pbFrom,
               PBYTE        pbTo,
               int          cbDestBytesInPrinter,
               int          iRemainder,
               int          iPasses)
{
   PBYTE         pbCurrent  = pbTo;
   unsigned char uchSrcMask    = 0x80;
   int iBitValue = 8;         // handle bits that should not be set on the outgoing bitmap
   char *achDestPelMask;
   static char   ach720DestPelMask[] = {
//    0xC0, 0x30, 0x0C, 0x03                       //  large
//    0x80, 0x20, 0x08, 0x02                       //  medium
      0x40, 0x10, 0x04, 0x01                       //  small
   };

   static char   ach360DestPelMask[] = {
//    0xC0, 0x30, 0x0C, 0x03                       //  large
      0x80, 0x20, 0x08, 0x02                       //  medium
//    0x40, 0x10, 0x04, 0x01                       //  small
   };

   if(iPasses == 1)
       achDestPelMask = ach360DestPelMask;
   else
       achDestPelMask = ach720DestPelMask;

   memset (pbTo, 0, 2 * cbDestBytesInPrinter);

   for (int j = 0; j < cbDestBytesInPrinter; j++)
   {
      uchSrcMask = 0x80;

      if(j == cbDestBytesInPrinter-1)
          iBitValue = iRemainder;

      for (int i = 0; i < iBitValue; i++)
      {
         if(pbFrom[j] & uchSrcMask)
             *pbCurrent |= achDestPelMask[i%4];   // This mask will set everything
                                                // to a specific pel size
         if (3 == (i % 4))
         {
            pbCurrent++;
         }

         uchSrcMask >>= 1;
      }
   }
}

// @TBD Move to GplCompress
/****************************************************************************/
/* PROCEDURE NAME : CompressEpsonRLE                                        */
/* AUTHOR         : Mark Hamzy                                              */
/* DATE WRITTEN   : 4/10/94                                                 */
/* DESCRIPTION    :                                                         */
/*                  Data is organized as counter bytes followed by          */
/*                  data bytes.  Two types of counter bytes can be          */
/*                  used: repeat counters and data-length counters.         */
/*                                                                          */
/*                  repeat counters - specify the number of times (minus 1) */
/*                                    to repeat the following single byte   */
/*                                    of data.                              */
/*                                    -1 <= repeat counter <= -127          */
/*                                                                          */
/*                  data-length counters - specify the number of            */
/*                                    bytes (minus 1) of print data         */
/*                                    data following the counter.           */
/*                                    This data is only printed once.       */
/*                                                                          */
/*                           0 <= data-length counter <= 127                */
/*                                                                          */
/*     The first byte of compressed data must be a counter.                 */
/*     EX: -3 0 1 60 61 -4 15 expands into 0 0 0 0 60 61 15 15 15 15 15     */
/*                                                                          */
/* PARAMETERS:                                                              */
/*                pbData,        // raster data to compress                 */
/*                pbReturn,      // compressed data will be written         */
/*                iTotalBytes )  // count of bytes in                       */
/*                                                                          */
/* RETURN VALUES:                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* CHANGE/MODIFICATION LOG :                                                */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/****************************************************************************/
int
compressEpsonRLE (PBYTE  pbData,
                  int    cBytesInData,
                  PBYTE  pbReturn,
                  int    cBytesInReturn)
{
   int            iFrom = 0,
                  iTo   = 0,
                  iDups = 0;
   unsigned char  uchData;
   unsigned char *puchCount;

   /* Test for a special cases
   */
   if (0 > cBytesInData)
   {
      // Negative count
      return 0;
   }

   // loop through the buffer
   while (iFrom < cBytesInData)
   {
      /* There have to be two or more chars to examine
      ** cBytesInData is 1 based and iFrom is 0 based
      */
      if (1 == cBytesInData - iFrom)
      {
         // just one character to compress
         *(pbReturn + iTo) = (unsigned char)'\0';
         iTo++;
         *(pbReturn + iTo) = (unsigned char)pbData[iFrom];
         iTo++;

         return iTo;
      }

      uchData = pbData[iFrom];         // seed start point
      iFrom++;                         // look at the next char

      // is the next byte equal?
      if (uchData == pbData[iFrom])
      {
         iFrom++;                      // point to the next data byte
         iDups = 2;                    // initial count of dups

         // count dups and not go off the end
         while (  iFrom < cBytesInData
               && uchData == pbData[iFrom]
               && iDups < 128
               )
         {
            iDups++;
            iFrom++;
         }

         *(pbReturn + iTo) = (unsigned char)(256 - iDups + 1); // return count
         iTo++;
         *(pbReturn + iTo) = (unsigned char)uchData;           // return data
         iTo++;
      }
      else
      {
         iDups = 1;                    // initial count of non dups
         puchCount = pbReturn + iTo;   // Remember initial location of count
         iTo++;                        // Reserve its space

         *(pbReturn + iTo) = (unsigned char)uchData; // copy the data
         iTo++;                                      // move to next output

          // count non dups and not go off the end
         while (  iFrom < cBytesInData
               && iDups < 128
               )
         {
            if (  iFrom == cBytesInData - 1          // We are at the end
               || pbData[iFrom] != pbData[iFrom + 1] // Still different
               )
            {
               // copy the data
               *(pbReturn + iTo) = (unsigned char)*(pbData + iFrom);
               iTo++;
               iDups++;
            }
            else
               // Not not the same... ok not different
               break;

            iFrom++;
         }

         *puchCount = (unsigned char)(iDups - 1);      // update count
      }
   }

   return iTo;
}
