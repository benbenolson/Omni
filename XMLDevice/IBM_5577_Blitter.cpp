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
#include "IBM_5577_Blitter.hpp"
#include "IBM_5577_Instance.hpp"
#include "DeviceOrientation.hpp"
#include <CMYKbitmap.hpp>

DeviceBlitter *
createBlitter (PrintDevice *pDevice)
{
   return new IBM_5577_Blitter (pDevice);
}

void
deleteBlitter (DeviceBlitter *pBlitter)
{
   delete pBlitter;
}

IBM_5577_Blitter::
IBM_5577_Blitter (PrintDevice *pDevice)
   : DeviceBlitter (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   fHaveInitialized_d      = false;
   fGraphicsHaveBeenSent_d = false;

}

IBM_5577_Blitter::
~IBM_5577_Blitter ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::~" << __FUNCTION__ << " () enter" << std::endl;
#endif

   fGraphicsHaveBeenSent_d = false;

   if (restPbBitBak != NULL)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::free( restPbBitBak ) " << std::endl;
#endif

      free (restPbBitBak);
      restPbBit = restPbBitBak = NULL;
      restNumScanLines = restScanLineY = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::~" << __FUNCTION__ << " () exit" << std::endl;
#endif
}

void IBM_5577_Blitter::
initializeInstance ()
{
   HardCopyCap      *pHCC = getCurrentForm ()->getHardCopyCap ();
   DeviceResolution *pDR  = getCurrentResolution ();
   DevicePrintMode  *pDPM = getCurrentPrintMode ();

   PSZRO       pszDitherID = getCurrentDitherID ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "IBM_5577_Blitter::initializeInstance() " << std::endl;
#endif

   restPbBit = restPbBitBak = NULL;
   restNumScanLines = restScanLineY = 0;

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

bool IBM_5577_Blitter::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi2,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ())
      DebugOutput::getErrorStream () << std::hex
           << "IBM_5577_Blitter::rasterize (0x"
           << (int)pbBits << ", {"
           << std::dec << pbmi2->cx << ", "
           << pbmi2->cy << ", "
           << pbmi2->cPlanes << ", "
           << pbmi2->cBitCount << "}, "
           << "{" << prectlPageLocation->xLeft << ", "
           << prectlPageLocation->yBottom << ", "
           << prectlPageLocation->xRight << ", "
           << prectlPageLocation->yTop << "})"
           << std::endl;
#endif

   IBM_5577_Instance *pInstance = dynamic_cast <IBM_5577_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::rasterize Error: unknown color tech " << getCurrentPrintMode ()->getColorTech () << std::endl;
#endif
      break;
   }

   return true;
}

bool IBM_5577_Blitter::
ibmMonoRasterize (PBYTE        pbBits,
                  PBITMAPINFO2 pbmi2,
                  PRECTL       prectlPageLocation,
                  BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::" << __FUNCTION__ << " ()" << std::endl;
#endif

   IBM_5577_Instance *pInstance = dynamic_cast <IBM_5577_Instance *>(getInstance ());
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
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::hex << "IBM_5577_Blitter::ibmMonoRasterize (out)pszDumpEnvironmentVar = " << (int)pszDumpEnvironmentVar << std::dec << std::endl;
#endif

      if (*pszDumpEnvironmentVar)
         fDumpOutgoingBitmaps = true;
   }

   DeviceResolution *pDR                   = getCurrentResolution ();
   int               cy                    = pbmi2->cy,
                     cx                    = pbmi2->cx,
                     ulPageSize;
   int               iNumScanLines,
                     iScanLineY,
                     iWorldY,
                     cbSourceBytesInBitmap,
                     cbDestBytesInPrinter;
   PBYTE             pbBuffer;
   int               iRemainder;
   DeviceCommand    *pCommands             = getCommands ();
   BinaryData       *pCmd                  = 0;
   int               iXRes                 = pDR->getXRes ();
   int               iYRes                 = pDR->getYRes ();
   int               iSizeBlock;
   int               iImageHeight;
   int               iBlockY;
   int               iRelativeY;
   int               iTransY;                        // Trans mode for 5577 (default : 3 byte)
   BYTE              imgBuffSrc[8];                  // 8x8 matrix soruce
   BYTE              imgBuffDst[8];                  // 8x8 matrix conversion
   BYTE              imgBuff5577[MAX_CX_BYTES_5577]; // 13.6 [inch] x 180 [dpi] / 8 * 24 lines = 7344 byte
   int               n1, n2;                         // high and low byte
   int               idx_X, idx_Y, idx_T;            // some indexes

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
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << std::dec << "IBM_5577_Blitter::5577Rasterize ulPageSize = " << ulPageSize << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iWorldY = " << iWorldY << std::endl;
#endif

   cbSourceBytesInBitmap = ((pbmi2->cBitCount * pbmi2->cx + 31) >> 5) << 2;
   cbDestBytesInPrinter  = (pbmi2->cx + 7) >> 3;

   iScanLineY            = cy - 1;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize cbSourceBytesInBitmap = " << cbSourceBytesInBitmap << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize cbDestBytesInPrinter = " << cbDestBytesInPrinter << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iScanLineY = " << iScanLineY << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iNumScanLines = " << iNumScanLines << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iXRes = " << iXRes << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iYRes = " << iYRes << std::endl;
#endif

   /* We want to keep iRemainder of the left-most bits of the last
   ** byte.
   ** NOTE: 0 <= iRemainder <= 7
   */
   iRemainder = cx - (cbDestBytesInPrinter - 1) * 8;
   if (8 == iRemainder)
      iRemainder = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iRemainder = " << iRemainder << std::endl;
#endif

   bool fBlackWhiteReversed = false;

   if (  0x00 == pbmi2->argbColor[0].bRed
      && 0x00 == pbmi2->argbColor[0].bGreen
      && 0x00 == pbmi2->argbColor[0].bBlue
      )
      fBlackWhiteReversed = true;

   iImageHeight = 1;

   iTransY    = 3;          /* 3-bytes transfer mode for 5577, 2-bytes mode is TBD */
   iBlockY    = 8*iTransY;  /* 24-bit per head impact */

   // for 5577 image command
   n1 = ( (pbmi2->cx) & 0xff00) >> 8;
   n2 = ( (pbmi2->cx) & 0x00ff);

#ifndef RETAIL
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize n1 = " << n1 << std::endl;
   if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize n2 = " << n2 << std::endl;
#endif

   while (iNumScanLines>0)
   {
      iRelativeY = 0;

      memset( imgBuff5577, 0x00, MAX_CX_BYTES_5577);

#ifndef RETAIL
//    iScanLineY == iNumScanLines - 1
//
//    if (DebugOutput::shouldOutputBlitter ())
//      DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iScanLineY    = " << iScanLineY << std::endl;
//    if (DebugOutput::shouldOutputBlitter ())
//	     DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iNumScanLines = " << iNumScanLines << std::endl;
#endif

      if ( iNumScanLines < iBlockY ) { // too short data lines for iBlockY
#ifndef RETAIL
	if (DebugOutput::shouldOutputBlitter ())
	  DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize iNumScanLines shortage! " << std::endl;
#endif
        if ( prectlPageLocation->yBottom ) {  // banding data continues, so stocking data as temporarily
          if ( ( restPbBitBak = restPbBit = (PBYTE)malloc(iNumScanLines*cbSourceBytesInBitmap) ) != NULL ) {

	    memcpy( restPbBit, pbBits, iNumScanLines*cbSourceBytesInBitmap ); // rest on upper banding
	    restNumScanLines = iNumScanLines;
       restScanLineY    = iScanLineY;

#ifndef RETAIL
	    if (DebugOutput::shouldOutputBlitter ())
	      DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize restNumScanLines = " << restNumScanLines << std::endl;
	    if (DebugOutput::shouldOutputBlitter ())
	      DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize restScanLineY    = " << restScanLineY << std::endl;
	    if (DebugOutput::shouldOutputBlitter ())
	      DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize skip and wait for continuous data..." << std::endl;
#endif
	  }
	  else
	    DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize malloc fail! " << std::endl;

  	  return true;

	} else {
#ifndef RETAIL
	  if (DebugOutput::shouldOutputBlitter ())
	    DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize force rasterizing with null(0x00) data..." << std::endl;
#endif
	}
	for ( idx_X=0; idx_X < cbDestBytesInPrinter; idx_X++ ) {
	  for ( idx_T=0; idx_T < iTransY; idx_T++ ) {
	    for ( idx_Y=0; idx_Y < 8; idx_Y++ ) {
	      if ( idx_T*8 + idx_Y < iNumScanLines ) { // you have to pay attention! (T.Irie, 01/31/2003)
		imgBuffSrc[idx_Y] = pbBits[(iScanLineY-idx_T*8-idx_Y)*cbSourceBytesInBitmap+idx_X];
	      }
	      else
		imgBuffSrc[idx_Y] = 0x00;     // filling buffer with null
	    }
	    transparentMatrix(imgBuffSrc, imgBuffDst, 8, 8); // rotate data
	    for ( idx_Y=0; idx_Y < 8; idx_Y++ ) {
	      imgBuff5577[(idx_X*8+idx_Y)*iTransY+idx_T] = imgBuffDst[idx_Y];
	      iRelativeY ++;
	    }
	  }
	}
      }
      else { // enough data lines of banding remains
	for ( idx_X=0; idx_X < cbDestBytesInPrinter; idx_X++ ) {
	  for ( idx_T=0; idx_T < iTransY; idx_T++ ) {
	    for ( idx_Y=0; idx_Y < 8; idx_Y++ ) {

              if ( ! restNumScanLines )
		imgBuffSrc[idx_Y] = pbBits[(iScanLineY-idx_T*8-idx_Y)*cbSourceBytesInBitmap+idx_X];
	      else if ( restNumScanLines > idx_T*8+idx_Y )  // -> data line on previous banding
		imgBuffSrc[idx_Y] = restPbBit[(restScanLineY - idx_T*8 - idx_Y)*cbSourceBytesInBitmap+idx_X];
              else   // restNumScanLines <= idx_T*8+idx_Y      -> data line on this banding
		imgBuffSrc[idx_Y] = pbBits[(iScanLineY - (idx_T*8+idx_Y-restNumScanLines))*cbSourceBytesInBitmap+idx_X];
	    }
	    transparentMatrix(imgBuffSrc, imgBuffDst, 8, 8);
	    for ( idx_Y=0; idx_Y < 8; idx_Y++ ) {
	      imgBuff5577[(idx_X*8+idx_Y)*iTransY+idx_T] = imgBuffDst[idx_Y];
	      iRelativeY ++;
	    }
	  }
	}
      }

      pCmd = pCommands->getCommandData ("cmd5577Image");

      sendPrintfToDevice (pCmd,n1,n2);

      pbBuffer = imgBuff5577;
      iSizeBlock = pbmi2->cx * iTransY;

      // transfer image bits
      BinaryData data (pbBuffer, iSizeBlock);
      sendBinaryDataToDevice (&data);

      // transfer Carridge Return Code
      pCmd = pCommands->getCommandData ("cmd5577CR");
      sendPrintfToDevice (pCmd);

      // transfer Variable Feed Command
      pCmd = pCommands->getCommandData ("cmd5577MovDY");
      sendPrintfToDevice (pCmd,0x00,0x10);    // feed 16/120 inch

      // Move down a scan line
      if ( restNumScanLines ) {
	iScanLineY    -= iBlockY - restNumScanLines;
	iNumScanLines -= iBlockY - restNumScanLines;
	iWorldY       += iBlockY - restNumScanLines;

	if ( restPbBitBak != NULL ) {
#ifndef RETAIL
	  if ( DebugOutput::shouldOutputBlitter () ) DebugOutput::getErrorStream () << "IBM_5577_Blitter::5577Rasterize free( restPbBitBak )" << std::endl;
#endif
	  free ( restPbBitBak );
	}
	
        restPbBit = restPbBitBak = NULL;
	restNumScanLines = restScanLineY = 0;
      } else {
	iScanLineY    -= iBlockY;
	iNumScanLines -= iBlockY;
	iWorldY       += iBlockY;
      }
   }

   return true;
}

void IBM_5577_Blitter::
transparentMatrix ( PBYTE src, PBYTE dst, int width, int height )
{
  int i,j;
  BYTE temp;

  memset( dst, 0x00, height );

  for ( i=0; i<width; i++ ) {
    temp = 0x80>>i;
    for ( j=0; j<height; j++)
      dst[i] |= (src[j] & temp)? 0x80>>j : 0x00;
  }

 #if 0
  printf("Y:   src  ->   dst\n");
  for ( i=0; i<height; i++ )
    printf("%d:  0x%02x  ->  0x%02x\n",i,src[i],dst[i]);
 #endif
} // of transparentMatrix()


/****************************************************************************/
/* NOTE: assumes iWorldY increases                                          */
/****************************************************************************/
bool IBM_5577_Blitter::
moveToYPosition (int  iWorldY,
                 bool fAbsolute)
{
   IBM_5577_Instance *pInstance = dynamic_cast <IBM_5577_Instance *>(getInstance ());
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
            if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::moveToYPosition Cannot find a set line spacing command!" << std::endl;
#endif
            return false;
         }
      }

      iDefault = iSpacing * iColumnSize / pRes->getYRes ();

      iResult    = iAmount / iSpacing;
      iRemainder = iAmount - iSpacing * iResult;

#ifndef RETAIL
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::moveToYPosition iAmount    = " << iAmount << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::moveToYPosition iResult    = " << iResult << std::endl;
      if (DebugOutput::shouldOutputBlitter ()) DebugOutput::getErrorStream () << "IBM_5577_Blitter::moveToYPosition iRemainder = " << iRemainder << std::endl;
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

void IBM_5577_Blitter::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string IBM_5577_Blitter::
toString (std::ostringstream& oss)
{
   oss << "{IBM_5577_Blitter: "
       << DeviceBlitter::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const IBM_5577_Blitter& const_self)
{
   IBM_5577_Blitter&  self = const_cast<IBM_5577_Blitter&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
