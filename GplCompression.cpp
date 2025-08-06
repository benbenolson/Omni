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
#include "GplCompression.hpp"
#include "DevicePrintMode.hpp"

#include <cstdlib>

#include <memory>

#define MAXBLOCKSIZE                      127
#define MAX_OFF_COMP                      3
#define MAX_REPL_COMP                     31
#define MAX_OFF_UNC                       15
#define MAX_REPL_UNC                      7
#define MAX_RUN                           256

#define COMPRESS_MODE_NONE                0
#define COMPRESS_MODE_RLL                 1
#define COMPRESS_MODE_TIFF                2
#define COMPRESS_MODE_DELTA_ROW           3
#define COMPRESS_MODE_ENHANCED_DELTA_ROW  9

/* Function prototypes...
*/
int            GplCompressRLL         (PBYTE               pbDataIn,
                                       int                 cbDataIn,
                                       PBYTE               pbDataOut,
                                       int                 cbDataOut);
int            GplCompressTIFF        (PBYTE               pSource,
                                       int                 cbIn,
                                       PBYTE               pDest,
                                       int                 cbOut);
int            GplCompressDeltaRow    (int                 iTotalBytes,
                                       PBYTE               pbData,
                                       PBYTE               pbLastLine,
                                       int                 iMaxReturn,
                                       PBYTE               pbReturn,
                                       unsigned short int *pDeltas);
int            GplCompressRLLDeltaRow (INT                 iTotalBytes,
                                       PBYTE               pbData,
                                       PBYTE               pbLastLine,
                                       INT                 usMaxReturn,
                                       PBYTE               pbReturn,
                                       PUSHORT             pDeltas);
PBYTE          GplpChooseMode9        (SHORT               offset,
                                       USHORT             *pOutBytes,
                                       PBYTE               pbReturn,
                                       PBYTE               pbReplace,
                                       SHORT               ReplaceCount);
int            GplCompressMode9Out    (int                 iTotalBytes,
                                       PBYTE               pbData,
                                       PBYTE               pbLastLine,
                                       PBYTE               pbReturn);
int
GplCompressChooseMode (PBYTE               pbRow,
                       PBYTE               pbLastRow,
                       int                 iRowLength,
                       int                 iCompressModes,
                       unsigned short int *pDelta);

GplCompression::
GplCompression (int                     iPrintMode,
                int                     iCompressionModesSupported,
                int                     iBytesPerRow,
                GplCompressionCallback *pCallback)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ()) DebugOutput::getErrorStream () << "GplCompression::GplCompression ()" << std::endl;
#endif

   iCompressionModesSupported_d = iCompressionModesSupported;
   iBytesPerRow_d               = iBytesPerRow;
   iBlankLineCount_d            = 0;

   cbCompressBuffer_d           = 2 * iBytesPerRow;
   pbCompressBuffer_d           = (PBYTE)calloc (1, cbCompressBuffer_d);
   pusDelta_d                   = (PUSHORT)calloc (1, 2 * (iBytesPerRow + 1));

   pbKPlane_d      = 0;
   pbCPlane_d      = 0;
   pbLCPlane_d     = 0;
   pbMPlane_d      = 0;
   pbLMPlane_d     = 0;
   pbYPlane_d      = 0;
   pbLastKPlane_d  = 0;
   pbLastCPlane_d  = 0;
   pbLastLCPlane_d = 0;
   pbLastMPlane_d  = 0;
   pbLastLMPlane_d = 0;
   pbLastYPlane_d  = 0;

   pbdKPlane_d     = 0;
   pbdCPlane_d     = 0;
   pbdMPlane_d     = 0;
   pbdLCPlane_d    = 0;
   pbdLMPlane_d    = 0;
   pbdYPlane_d     = 0;

   if (  DevicePrintMode::COLOR_TECH_K       == iPrintMode
      || DevicePrintMode::COLOR_TECH_CMYK    == iPrintMode
      || DevicePrintMode::COLOR_TECH_CcMmYK  == iPrintMode
      || DevicePrintMode::COLOR_TECH_CcMmYyK == iPrintMode
      || DevicePrintMode::COLOR_TECH_RGB     == iPrintMode
      )
   {
      pbKPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
      pbLastKPlane_d = (PBYTE)calloc (1, iBytesPerRow);
   }

   if (  DevicePrintMode::COLOR_TECH_CMY     == iPrintMode
      || DevicePrintMode::COLOR_TECH_CMYK    == iPrintMode
      || DevicePrintMode::COLOR_TECH_CcMmYK  == iPrintMode
      || DevicePrintMode::COLOR_TECH_CcMmYyK == iPrintMode
      )
   {
      pbCPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
      pbMPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
      pbYPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
      pbLastCPlane_d = (PBYTE)calloc (1, iBytesPerRow);
      pbLastMPlane_d = (PBYTE)calloc (1, iBytesPerRow);
      pbLastYPlane_d = (PBYTE)calloc (1, iBytesPerRow);
   }

   pbdKPlane_d = new BinaryData (pbKPlane_d, iBytesPerRow);
   pbdCPlane_d = new BinaryData (pbCPlane_d, iBytesPerRow);
   pbdMPlane_d = new BinaryData (pbMPlane_d, iBytesPerRow);
   pbdYPlane_d = new BinaryData (pbYPlane_d, iBytesPerRow);

   if(DevicePrintMode::COLOR_TECH_CcMmYK  == iPrintMode)
   {
       pbLCPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
       pbLMPlane_d     = (PBYTE)calloc (1, iBytesPerRow);
       pbdLCPlane_d    = new BinaryData (pbLCPlane_d, iBytesPerRow);
       pbdLMPlane_d    = new BinaryData (pbLMPlane_d, iBytesPerRow);
       pbLastLCPlane_d = (PBYTE)calloc (1, iBytesPerRow);
       pbLastLMPlane_d = (PBYTE)calloc (1, iBytesPerRow);
   }

   pCallback_d           = pCallback;
   iCurrentCompression_d = GPLCOMPRESS_INVALID;
}

GplCompression::
~GplCompression ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ()) DebugOutput::getErrorStream () << "GplCompression::~GplCompression () enter" << std::endl;
#endif

   if (pbCompressBuffer_d)
   {
      free (pbCompressBuffer_d);
      pbCompressBuffer_d = 0;
   }

   if (pusDelta_d)
   {
      free (pusDelta_d);
      pusDelta_d = 0;
   }

   if (pbdKPlane_d)
   {
      delete pbdKPlane_d;
      pbdKPlane_d = 0;
   }
   if (pbdCPlane_d)
   {
      delete pbdCPlane_d;
      pbdCPlane_d = 0;
   }
   if (pbdLCPlane_d)
   {
      delete pbdLCPlane_d;
      pbdLCPlane_d = 0;
   }
   if (pbdMPlane_d)
   {
      delete pbdMPlane_d;
      pbdMPlane_d = 0;
   }
   if (pbdLMPlane_d)
   {
      delete pbdLMPlane_d;
      pbdLMPlane_d = 0;
   }
   if (pbdYPlane_d)
   {
      delete pbdYPlane_d;
      pbdYPlane_d = 0;
   }

   if (pbKPlane_d)
   {
      free (pbKPlane_d);
      pbKPlane_d = 0;
   }
   if (pbCPlane_d)
   {
      free (pbCPlane_d);
      pbCPlane_d = 0;
   }
   if (pbLCPlane_d)
   {
      free (pbLCPlane_d);
      pbLCPlane_d = 0;
   }
   if (pbMPlane_d)
   {
      free (pbMPlane_d);
      pbMPlane_d = 0;
   }
   if (pbLMPlane_d)
   {
      free (pbLMPlane_d);
      pbLMPlane_d = 0;
   }
   if (pbYPlane_d)
   {
      free (pbYPlane_d);
      pbYPlane_d = 0;
   }

   if (pbLastKPlane_d)
   {
      free (pbLastKPlane_d);
      pbLastKPlane_d = 0;
   }
   if (pbLastCPlane_d)
   {
      free (pbLastCPlane_d);
      pbLastCPlane_d = 0;
   }
   if (pbLastLCPlane_d)
   {
      free (pbLastLCPlane_d);
      pbLastLCPlane_d = 0;
   }
   if (pbLastMPlane_d)
   {
      free (pbLastMPlane_d);
      pbLastMPlane_d = 0;
   }
   if (pbLastLMPlane_d)
   {
      free (pbLastLMPlane_d);
      pbLastLMPlane_d = 0;
   }
   if (pbLastYPlane_d)
   {
      free (pbLastYPlane_d);
      pbLastYPlane_d = 0;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ()) DebugOutput::getErrorStream () << "GplCompression::~GplCompression () exit" << std::endl;
#endif
}

void GplCompression::
resetBlankLineCount ()
{
   iBlankLineCount_d = 0;
}

void GplCompression::
incrementBlankLineCount ()
{
   iBlankLineCount_d++;
}

void GplCompression::
incrementBlankLineCount (int iBlankLines)
{
   iBlankLineCount_d += iBlankLines;
}

int GplCompression::
getBlankLineCount ()
{
   return iBlankLineCount_d;
}

void GplCompression::
resetCompressionMode ()
{
   iCurrentCompression_d = GPLCOMPRESS_INVALID;
}

bool GplCompression::
isCurrentCompressionMode (int iMode)
{
   if (iMode & iCurrentCompression_d)
      return true;
   else
      return false;
}

BinaryData * GplCompression::
compressKRasterPlane (BinaryData *pbdKPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdKPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastKPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_BLACK);

   memcpy (pbKPlane_d, pbCompressBuffer_d, iCompressed);
   pbdKPlane_d->setLength (iCompressed);

   return pbdKPlane_d;
}

BinaryData * GplCompression::
compressCRasterPlane (BinaryData *pbdCPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdCPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastCPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_CYAN);

   memcpy (pbCPlane_d, pbCompressBuffer_d, iCompressed);
   pbdCPlane_d->setLength (iCompressed);

   return pbdCPlane_d;
}

BinaryData * GplCompression::
compressLCRasterPlane (BinaryData *pbdLCPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdLCPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastLCPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_LIGHT_CYAN);

   memcpy (pbLCPlane_d, pbCompressBuffer_d, iCompressed);
   pbdLCPlane_d->setLength (iCompressed);

   return pbdLCPlane_d;
}

BinaryData * GplCompression::
compressMRasterPlane (BinaryData *pbdMPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdMPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastMPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_MAGENTA);

   memcpy (pbMPlane_d, pbCompressBuffer_d, iCompressed);
   pbdMPlane_d->setLength (iCompressed);

   return pbdMPlane_d;
}

BinaryData * GplCompression::
compressLMRasterPlane (BinaryData *pbdLMPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdLMPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastLMPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_LIGHT_MAGENTA);

   memcpy (pbLMPlane_d, pbCompressBuffer_d, iCompressed);
   pbdLMPlane_d->setLength (iCompressed);

   return pbdLMPlane_d;
}

BinaryData * GplCompression::
compressYRasterPlane (BinaryData *pbdYPlaneIn)
{
   int iCompressed;

   iCompressed = compressRasterPlane (pbdYPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastYPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_YELLOW);

   memcpy (pbYPlane_d, pbCompressBuffer_d, iCompressed);
   pbdYPlane_d->setLength (iCompressed);

   return pbdYPlane_d;
}

BinaryData * GplCompression::
compressRGBRasterPlane (BinaryData *pbdRGBPlaneIn)
{
   int iCompressed;

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ()) DebugOutput::getErrorStream () << "GplCompression::compressRGBRasterPlane cbCompressBuffer_d = " << cbCompressBuffer_d << ", data size = " << pbdRGBPlaneIn->getLength ();
#endif

   iCompressed = compressRasterPlane (pbdRGBPlaneIn->getData (),
                                      iBytesPerRow_d,
                                      pbLastKPlane_d,
                                      pbCompressBuffer_d,
                                      cbCompressBuffer_d,
                                      iCompressionModesSupported_d,
                                      pusDelta_d,
                                      DevicePrintMode::COLOR_PLANE_BLACK);

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ())
   {
      DebugOutput::getErrorStream () << ", iCompressed = " << iCompressed << std::endl;
      DebugOutput::getErrorStream ().flush ();
   }
#endif

   memcpy (pbKPlane_d, pbCompressBuffer_d, iCompressed);
   pbdKPlane_d->setLength (iCompressed);

   return pbdKPlane_d;
}

void GplCompression::
clearLastLineBuffers ()
{
   if (pbLastKPlane_d)
      memset (pbLastKPlane_d, 0, iBytesPerRow_d);
   if (pbLastCPlane_d)
      memset (pbLastCPlane_d, 0, iBytesPerRow_d);
   if (pbLastLCPlane_d)
      memset (pbLastLCPlane_d, 0, iBytesPerRow_d);
   if (pbLastMPlane_d)
      memset (pbLastMPlane_d, 0, iBytesPerRow_d);
   if (pbLastLMPlane_d)
      memset (pbLastLMPlane_d, 0, iBytesPerRow_d);
   if (pbLastYPlane_d)
      memset (pbLastYPlane_d, 0, iBytesPerRow_d);
}

int GplCompression::
compressRasterPlane (PBYTE   pbBuffer,
                     int     iPrinterBytesInArray,
                     PBYTE   pbLastLine,
                     PBYTE   pbCompress,
                     int     cbCompressBuffer,
                     int     iCompressModeSupported,
                     PUSHORT pDelta,
                     int     iWhichPlane)
{
   int    iCompressMode;
   int    iCompressed   = 0;

   if (iCompressModeSupported != COMPRESS_MODE_NONE)
      iCompressMode = GplCompressChooseMode (pbBuffer,
                                             pbLastLine,
                                             iPrinterBytesInArray,
                                             iCompressModeSupported,
                                             pDelta);
   else
      iCompressMode = COMPRESS_MODE_NONE;

   if (  iCurrentCompression_d == GPLCOMPRESS_INVALID
      && iCompressMode == COMPRESS_MODE_DELTA_ROW
      )
   {
      // The seed row is not set up yet, switch to RLL if supported
      if (iCompressModeSupported & GPLCOMPRESS_RLL)
      {
         iCompressMode = COMPRESS_MODE_RLL;
      }
      else
      {
         iCompressMode = COMPRESS_MODE_NONE;
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputGplCompression ())
   {
      switch (iCompressMode)
      {
      case COMPRESS_MODE_NONE:               DebugOutput::getErrorStream () << "COMPRESS_MODE_NONE "; break;
      case COMPRESS_MODE_RLL:                DebugOutput::getErrorStream () << "COMPRESS_MODE_RLL "; break;
      case COMPRESS_MODE_TIFF:               DebugOutput::getErrorStream () << "COMPRESS_MODE_TIFF "; break;
      case COMPRESS_MODE_DELTA_ROW:          DebugOutput::getErrorStream () << "COMPRESS_MODE_DELTA_ROW "; break;
      case COMPRESS_MODE_ENHANCED_DELTA_ROW: DebugOutput::getErrorStream () << "COMPRESS_MODE_ENHANCED_DELTA_ROW "; break;
      }
   }
#endif

   switch (iCompressMode)
   {
   case COMPRESS_MODE_NONE:                // no compression
   {
      iCompressed = iPrinterBytesInArray;
      memcpy (pbCompress, pbBuffer, iCompressed);
      break;
   }

   case COMPRESS_MODE_RLL:                 // RLL compression
   {
      iCompressed = GplCompressRLL (pbBuffer,
                                    iPrinterBytesInArray,
                                    pbCompress,
                                    cbCompressBuffer);
      break;
   }

   case COMPRESS_MODE_TIFF:                // TIFF compression
   {
      iCompressed = GplCompressTIFF (pbBuffer,
                                     iPrinterBytesInArray,
                                     pbCompress,
                                     cbCompressBuffer);
      break;
   }

   case COMPRESS_MODE_DELTA_ROW:           // Delta Row compression
   {
      iCompressed = GplCompressDeltaRow (iPrinterBytesInArray,
                                         pbBuffer,
                                         pbLastLine,
                                         cbCompressBuffer,
                                         pbCompress,
                                         pDelta);
      break;
   }

   case COMPRESS_MODE_ENHANCED_DELTA_ROW:  // Enhanced Delta Row compression
   {
      iCompressed = GplCompressRLLDeltaRow (iPrinterBytesInArray,
                                            pbBuffer,
                                            pbLastLine,
                                            cbCompressBuffer,
                                            pbCompress,
                                            pDelta);
      break;
   }

   default:
   {
      // should never execute
      break;
   }
   }

   if (  GplCompression::GPLCOMPRESS_ERROR == iCompressed // Default to uncompressed on an error
      || iCompressed > iPrinterBytesInArray               // Use the smaller of the two
      )
   {
      iCompressMode = COMPRESS_MODE_NONE;
      iCompressed   = iPrinterBytesInArray;
      memcpy (pbCompress, pbBuffer, iCompressed);
   }

   // Save the last line for the algorithms that need it
   if (iCompressModeSupported >= GplCompression::GPLCOMPRESS_DELTAROW)
   {
      memcpy (pbLastLine, pbBuffer, iPrinterBytesInArray);
   }

   if (iCurrentCompression_d != iCompressMode)
   {
      if (pCallback_d)
      {
         pCallback_d->compressionChanged (iCompressMode);
      }

      iCurrentCompression_d = iCompressMode;
   }

   BinaryData data (pbCompress, iCompressed);

   if (pCallback_d)
   {
      pCallback_d->sendData (iCompressed, &data, iWhichPlane);
   }

   return iCompressed;
}

/************************************************************************
 *                                                                      *
 * Function:      GplCompressRLL                                        *
 *                                                                      *
 * Description: Run Length Encoding (HP Mode 1)                         *
 *                                                                      *
 * Inputs: pbData            = pointer to input data                    *
 *         cbIn              = count of bytes in                        *
 *         pbReturn          = pointer to output data                   *
 *         cbOut             = count of bytes pointed to by pbReturn    *
 *                                                                      *
 * Returns: signed LONG                                                 *
 *                                                                      *
 * Success: number of bytes in compressed return string                 *
 * Error:   GPLCOMPRESS_ERROR (-1)                                      *
 *                                                                      *
 * The input data is encoded with pairs of bytes.  The first byte is    *
 * the replacement count and the second byte is the data.  A            *
 * replacement count of 0 means the data is not repeated.               *
 *                                                                      *
 * History:  Mike Jones - 02/01/94 Created                              *
 * 17may94monte. do not write past end of output buffer                 *
 *               return 0 in case of error                              *
 *                                                                      *
 ************************************************************************/
int
GplCompressRLL (PBYTE pbDataIn,
                int   cbDataIn,
                PBYTE pbDataOut,
                int   cbDataOut)
{
   byte data;
   int  i    = 0,
        j    = 0,
        dups = 0;

   // it always writes 2 at a time to target buffer, so fudge count of bytes in target
   cbDataOut -= 2;

   while (  i < cbDataIn
         && j < cbDataOut
         )
   {
      dups = 0;                     // initial count of dups
      data = *(pbDataIn + i++);     // seed start point

      while (  i < cbDataIn
            && data == *(pbDataIn + i)
            && dups < 255
            )
      {
         dups++;
         i++;
      }

      *(pbDataOut + j++) = dups;
      *(pbDataOut + j++) = data;
   }

   return j < cbDataOut ? j : -1;
}

/**************************************************************************
 *                                                                        *
 * Function:       GplCompressTIFF                                        *
 *                                                                        *
 * Description:    tagged image file format encoding (HP Mode 2)          *
 *                                                                        *
 * Concept:        This function compresses a scanline.  The compression  *
 *                 is done according to HP PCL Tech Ref Manual.           *
 *                                                                        *
 *                 Once in a repeated run, we stop and emit a block if a  *
 *                 different byte is encountered.                         *
 *                                                                        *
 *                 Once in a literal run, we stop and emit a block if at  *
 *                 least 4 repeated bytes follow: the next block will be  *
 *                 a repeat block.                                        *
 *                                                                        *
 *                 Repeats of 2 or more instances of a byte b are         *
 *                 represented by -(n-1)  b                               *
 *                                                                        *
 *                 Literal runs of different bytes b1 b2 b3 .. bn are     *
 *                 represented by (n-1) b1 b2 b3 .. bn                    *
 *                                                                        *
 *                 Any combination of blocks of these types may be sent   *
 *                 with the normal raster output escape "Esc * b count W".*
 *                                                                        *
 * Input(s):       pSource and cbSource                                   *
 *                 pDest   and cbDest                                     *
 *                                                                        *
 * returns:        count of bytes put in dest, or GPLCOMPRESS_ERROR (-1)  *
 *                                                                        *
 *                                                                        *
 *                    ! ! ! I M P O R T A N T ! ! !                       *
 *                                                                        *
 *  The define BIT16OBJ is set to be equal to the size of a 16 bit        *
 *  primitive type.  For use BIT16OBJ is set equal to short int!          *
 *                                                                        *
 **************************************************************************
 * MODIFICATION HISTORY                                                   *
 * --------------------                                                   *
 * Date      Flag   APAR    Initials   Description of change              *
 * ------    ------ ------  --------   --------------------------------   *
 * 12/20/93                 Mike Jones [IBM] Wrote it                     *
 *                                                                        *
 **************************************************************************/
int
GplCompressTIFF (PBYTE pSource,
                 int   cbIn,
                 PBYTE pDest,
                 int   cbOut)
{
   PBYTE pEndDest     = pDest + cbOut; // 1 past end of destination
   PBYTE pBeginDest   = pDest;         // save original address of destination
   PBYTE pBeginSource = pSource;       // save original address of source
   PBYTE p;                            // working pointer
   PBYTE temp1,
         temp2;                        // temp pointers
   bool  fDone        = false;         // flag
   int   rcResult     = GplCompression::GPLCOMPRESS_ERROR;

   p = pBeginSource;
   while (!fDone)
   {
      /*
       * 4 bytes left?
       * if not, exit out of the loop.
       */
      if ((p + 4 - pBeginSource) > cbIn)
      {
         break;
      }

      /*
       * 2 bytes equal?
       * ... and 4 bytes equal?
       */
      if (  (*p == *(p+1))
         && ( *(short int *)(p) == *(short int *)(p+2))
         )
      {
         /*
          * YES, run of repeating byte.
          * At this point, the first 4 bytes are equal.
          */
         p = p + 3;
         while ((p - pSource) < MAXBLOCKSIZE)       // 127 bytes?
         {
            p++;                                    // advance to next byte

            if ((p - pBeginSource) >= cbIn)         // end of input buffer?
            {
               fDone = true;
               break;
            }
            if (*p != *pSource)                     // still equal?
            {
               break;
            }
         }

         // don't walk off the end - monte
         if (pDest >= pEndDest - 2)
         {
            rcResult = GplCompression::GPLCOMPRESS_ERROR;
            goto depart;
         }

         *pDest++ = -(p - pSource - 1);             // -(n-1)
         *pDest++ = *pSource;                       // copy repeating byte
         pSource = p;                               // advance to next block
      }
      else
      {
         /*
          * NO, literal run of differring bytes
          */
         temp1 = pSource;                             // save start location
         temp2 = pDest++;
         while ((p - temp1) < MAXBLOCKSIZE)           // 127 bytes?
         {
            p++;                                      // advance to next byte

            // don't walk off the end - monte
            if (pDest >= pEndDest)
            {
               rcResult = GplCompression::GPLCOMPRESS_ERROR;
               goto depart;
            }

            if ((p - pBeginSource) >= cbIn)          // end of input buffer?
            {
               fDone = true;
               *pDest++ = *pSource;                  // copy the last byte
               break;                                //   before exit out
            }
            /*
             * Remain in literal block (differing bytes) unless there are
             * at least 4 bytes left and 4 consecutive bytes are equal.
             */
            if (*p == *pSource)                               // 2 bytes equal?
            {
               if ((pSource + 4 - pBeginSource) <= cbIn)      // 4 bytes left?
               {
                  if (*(short int *)(pSource) == *(short int *)(pSource+2))
                  {
                     p--;                                     // 4 bytes are equal
                     break;
                  }
               }
            }

            *pDest++ = *pSource++;                   // copy byte
         }

         *temp2 = p - temp1 - 1;                     // (n-1)
      }
   } // end of while(!fDone)

   /*
    * If not done yet, the block is less than 4 bytes:  literal run
    */
   if (!fDone)
   {
      temp2 = pDest++;
      while ((p - pBeginSource) < cbIn)
      {
         *pDest++ = *p++;
      }
      *temp2 = p - pSource - 1;
   }

   rcResult = (int)(pDest - pBeginDest);

depart:
   return rcResult;
}

/*************************************************************************
 *                                                                       *
 * Function:       GplCompressDeltaRow()                                 *
 *                                                                       *
 * Description: Delta Row Encoding - HP mode 3 compression               *
 *                                                                       *
 * This method replaces only the bytes in a new row that are different   *
 * from the preceding (seed) row.  Unreplaced bytes are replicated       *
 * from the seed row.  The new row then becomes the seed row upon        *
 * being printed.                                                        *
 *                                                                       *
 * The replacement bytes are called delta bytes.  A delta compression    *
 * string consists of the command byte the replacement bytes and         *
 * optionally, offset bytes.                                             *
 *                                                                       *
 *   -----------------------------------------------------------------   *
 *  | command byte | optional offset bytes | 1 to 8 replacement bytes |  *
 *   -----------------------------------------------------------------   *
 *                                                                       *
 * The command byte has two parts: the number of consecutive delta       *
 * bytes that follow, and the left offset from the current byte positon  *
 *                                                                       *
 * Bit 1                   3  4                                   8      *
 *   ---------------------------------------------------------------     *
 *  | # bytes to replace 1-8 | offset from last untreated byte 0-30 |    *
 *   ---------------------------------------------------------------     *
 *                                                                       *
 * The upper 3 bits of the command byte indicate the number of           *
 * consecutive delta bytes that follow + 1 (000=1, 111=8).  The lower    *
 * 5 bits contain the offset from the current untreated byte to the      *
 * next delta byte.  A value of 00000 indicates an offset of 0           *
 * (the current untreated byte is being replaced).  A value of 00001     *
 * indicates an offset of 1 (the byte following the current byte is      *
 * replaced).                                                            *
 *                                                                       *
 * If more than eight delta bytes are needed, additional command byte/   *
 * delta bytes are needed.                                               *
 *                                                                       *
 *                                                                       *
 *      Inputs:    iTotalBytes   - the number of bytes in the row        *
 *                 pbData        - pointer to current raster data row    *
 *                 pbLastLine    - pointer to previous raster data row   *
 *                 usMaxReturn   - Length of output buffer pointed       *
 *                                 by pbReturn                           *
 *                 pbReturn      - pointer to compressed data returned   *
 *                 pDeltas       - pointer to an array of struct {       *
 *                                 int start, int end }  where each      *
 *                                 start, end pair is the index to       *
 *                                 beginning and end of a difference     *
 *                                 fragment.                             *
 *                                                                       *
 *     Outputs:    pbReturn points to the compressed data                *
 *                                                                       *
 *                                                                       *
 *     Returns:    0 if current and previous line are identical else     *
 *                 > 0 = length of data pointed to by pbReturn           *
 *                 -1 = GPLCOMPRESS_ERROR = usMaxOutput not large        *
 *                      enough for the compressed output                 *
 *                                                                       *
 *     History:    07/11/94  mjones wrote it                             *
 *                                                                       *
 *     Copywrite IBM Corporation 1994                                    *
 *                                                                       *
 *************************************************************************/
int
GplCompressDeltaRow (int                 iTotalBytes, // bytes in the scan line
                     PBYTE               pbData,      // raster data to compress
                     PBYTE               pbLastLine,  // previous scanline's raster data
                     int                 iMaxReturn,  // size of output buffer
                     PBYTE               pbReturn,    // compressed data will be written
                     unsigned short int *pDeltas)     // differences indexes = array
                                                      // of start, end pairs.
{
   int    d = 0;             // index into pDeltas (Start,End) pairs
   int    iStartDiff;        // start of difference string
   int    iEndDiff;          // end of differnce string
   int    iLastEnd;          // used for relative iOffset from last difference string
   int    iLengthDiff;       // length of current difference string
   int    iNumCommandBlocks; // number of groups of 8 bytes in difference string
   int    iRemLength;        // remainder bytes
   int    iOffset;           // used to output iOffset byte(s)
   int    i;                 // local counters
   int    l;                 // index into pbReturn = also is return count of bytes in pbReturn
   PBYTE  pbTemp;            // to hold temp pointers that are constant in loops

   if ((iStartDiff = *(pDeltas + d++)) == 0)
      return 0;   // previous and current are the same

   l = iLastEnd = 0;

   /* We will loop over all the start, end pairs pointed to by pDeltas.
    * This array of pairs are terminated by a start of 0.  The first position
    * in the current scan line is given a count of 1.  So a sample array of
    * pDeltas: 3 5 8 10 14 21 0 show that the current scan line differs from
    * previous scan line at index 3 and extends to index 5, etc. Note we start
    * with a index 1 being the 0th byte in the current scan.
    *
    *    1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21
    *    s  s  d  d  d  s  s  d  d   d  s  s  s  d  d  d  d  d  d  d  d
    *
    *  The index for the current scan is shown from 1 to 21 above.
    *  The small letter s says that the current byte and previous scan line
    *  byte is the same.  The small letter d says that the current byte and
    *  previous scan line byte is different at this index.  Note that
    *  the pDeltas 3 and 5 are the start and end for the first difference
    *  string in the current scan line.  Similiarly 8 and 10 are the
    *  second difference string.  While we loop over these pairs we
    *  output the format necessary for the HP Mode 3 Delta Compression Mode.
    */
   do
   {
      iEndDiff    = *(pDeltas + d++);          // get end of difference string
      iOffset     = iStartDiff - iLastEnd - 1; // relative iOffset = iOffset from last end
      iLastEnd    = iEndDiff;                  // set new last end
      iLengthDiff = iEndDiff - iStartDiff + 1; // length of difference of difference bytes

      iNumCommandBlocks = iLengthDiff / 8;     // number of command bytes needed for this difference string
      iRemLength = iLengthDiff % 8;            // length for last command byte

      if (iNumCommandBlocks-- != 0)    // output multiples of 8 and remainder
      {
         if (iOffset >= 31)
         {
            *(pbReturn + l++) = 0xFF;  // command byte = 111 11111
            iOffset -= 31;

            while (iOffset >= 255)      // output additional iOffset bytes
            {
               *(pbReturn + l++) = 0xFF;
               if (l > iMaxReturn)
                  return GplCompression::GPLCOMPRESS_ERROR;
               iOffset -= 255;
            }

            *(pbReturn + l++) = iOffset;
            if (l > iMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }
         else
         {
            *(pbReturn + l++) =  0xE0 | iOffset;
            if (l > iMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }

         // check enough room for following bytes
         if (l + 8 * (iNumCommandBlocks + 1) + iRemLength + 2 > iMaxReturn)
            return GplCompression::GPLCOMPRESS_ERROR;

         pbTemp = pbData + iStartDiff - 1;        // output group of 8 for above iOffset
         for (i = 0; i < 8; i++)
            *(pbReturn + l++) = *(pbTemp + i);

         while (iNumCommandBlocks-- != 0)
         {
            *(pbReturn + l++) = 0xE0;     // output command byte with 0 iOffset
            pbTemp += 8;                  // output 8 at a time
            for (i = 0; i < 8; i++)       // output next group of 8 with 0 iOffset
               *(pbReturn + l++) = *(pbTemp + i);
         }

         if (iRemLength != 0)      // output last command byte for remainder bytes in differnce string
         {
            *(pbReturn + l++) = (iRemLength - 1) << 5;    // just length zero iOffset

            pbTemp = pbData + iEndDiff - iRemLength;
            for (i = 0; i < iRemLength; i++)      // output difference data for the remainder bytes
               *(pbReturn + l++) = *(pbTemp + i);
         }
      }
      else             // output just remainder
      {
         if (iOffset >= 31)
         {
            *(pbReturn + l++) = ((iRemLength - 1) << 5) | 0x1F;
            if ( l > iMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
            iOffset -= 31;

            while (iOffset >= 255)      // output additional iOffset bytes
            {
               *(pbReturn + l++) = 0xFF;
               // check enough room for following bytes
               if (l > iMaxReturn)
                  return GplCompression::GPLCOMPRESS_ERROR;
               iOffset -= 255;
            }

            *(pbReturn + l++) = iOffset;
            if (l > iMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }
         else
         {
            *(pbReturn + l++) = ((iRemLength - 1) << 5) | iOffset;
            if (l > iMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }

         pbTemp = pbData + iEndDiff - iRemLength;

         // check enough room for following bytes
         if (l + iRemLength > iMaxReturn)
            return GplCompression::GPLCOMPRESS_ERROR;

         for (i = 0; i < iRemLength; i++)      // output difference data for the remainder bytes
            *(pbReturn + l++) = *(pbTemp + i);
      }
   } while ((iStartDiff = *(pDeltas + d++)) != 0);

   return l;  // length of compressed data
}

/*************************************************************************
 *                                                                       *
 * Function:       GplCompressRLLDeltaRow()                              *
 *                                                                       *
 * Description: RLL Delta Row Encoding. - HP mode 9 compression          *
 *                                                                       *
 *              Compresses a string of bytes using the PCL Mode 9        *
 *              algorithm.  Returns the compressed string at pbReturn    *
 *              and the new string usLength as its return value.         *
 *              This method replaces only bytes in the current row       *
 *              that are different from the proceeding seed row,         *
 *              similarly to Method 3.  However unlike Method 3,         *
 *              the replacement or delta bytes themselves are encoded.   *
 *              A delta compression string consists of a command byte,   *
 *              optional replacement count bytes and finally the         *
 *              replacement data.                                        *
 *                                                                       *
 *  ------------------------------------------------------------------   *
 * | command byte | offset byte | replacement count bytes | data byte |  *
 *  ------------------------------------------------------------------   *
 *                                                                       *
 * The offset byte and replacement count are optional.  The command      *
 * byte itself has three parts: the control bit to determine the         *
 * encoding of the replacement bytes, the left offset from the current   *
 * byte position, and the number of consecutive bytes to replace.        *
 *                                                                       *
 * The control bit determines the compression method of replacement      *
 * data.  It also determines where the bit boundaries are for the        *
 * next two fields.                                                      *
 *                                                                       *
 *     bit         7         6            3   2            0             *
 *       | control bit = 0 | offset count  | replace count |             *
 *                                                                       *
 * If the control bit is 0 then the replacement data is uncompressed     *
 * and bits 6 through 3 contain the offset count and bits 2 through      *
 * 0 contain the replacement count.  The replacement bytes that follow   *
 * the command byte then replace the replacement count of bytes offset   *
 * from the current position within the seed row by the offset count.    *
 *                                                                       *
 * If the control bit is 0 and the offst count is 0 to 14 then that      *
 * number of bytes are ignored or offset prior to replacing bytes.       *
 *                                                                       *
 * I the control bit is 0 and the offset count is 15 then an additional  *
 * optional offset count byte follows the command byte.  The offset      *
 * count value in the command byte is added to the value in the          *
 * additional offset count byte.  If the additional offset count byte    *
 * is 0, the offset count is 15.  If the additional offset count byte    *
 * is 255, another addtional offset count byte follows.  The last        *
 * additional offset count byte is indicated by a value of less than     *
 * 255.  All of the additional offset count bytes are added to the       *
 * offset count in the command byte to get the total offset count.       *
 *                                                                       *
 * If the control bit is 0 and the replacement count is 0 to 6 then one  *
 * more than the replacement count bytes will be replaced.               *
 *                                                                       *
 * If the control bit is 0 and replacement count is 7 then an            *
 * additional replacement byte follows the command byte and any          *
 * additional offset offset count bytes.  The replacement count value    *
 * in the command byte is added to the value in the additional           *
 * replacement count byte.  If the additional replacement count byte     *
 * is 0, then  8 bytes are replaced as the replacement count is 7.       *
 * If the additional replacement count byte is 255, then another         *
 * additional replacement count byte follows.  The last replacement      *
 * count byte is indicated by a value les than 255.  All the             *
 * additional replacement count bytes are added to the replacement       *
 * count in the command byte to get the total replacement byte count.    *
 * One more than the total replacement byte count will be replaced.      *
 *                                                                       *
 *     bit         7         6            5  4            0              *
 *       | control bit = 1 | offset count  | replace count |             *
 *                                                                       *
 * If the control bit is 1 then the replacement data is run length       *
 * encoded and bits 6 through 5 contain the offset count and bits        *
 * 4 through 0 contain the replacement count.  The replacement bytes     *
 * that follows the command byte then replace the replacement count      *
 * of bytes offset from the current position within the seed row by      *
 * the offset count.                                                     *
 *                                                                       *
 * If the contol bit is 1 and the offset count is 0 to 2 then that       *
 * number of bytes is ignored or offset prior to replacing bytes.        *
 *                                                                       *
 * If the control bit is 1 and the offset count is 3 then an             *
 * additional optional offset count byte follows the command byte.       *
 * The offset count value in the command byte is added to the value      *
 * in the additional offset count bytes.  If the additional offset       *
 * count byte is 0, the offset count is 3.  If the additonal offset      *
 * count byte is 255, another addtional offset count byte follows.       *
 * The last addtitional offset byte is indicated by a value less         *
 * than 256.  All of the additional offset count bytes are added         *
 * to the offset count in the command byte to get the total offset       *
 * count.                                                                *
 *                                                                       *
 * If the control bit is 1 and the replacement is 0 to 30 then two       *
 * more than the replacement count bytes will be replaced.               *
 *                                                                       *
 * If the control bit is 1 and the replacement count is 31 than an       *
 * additional replacement byte follows the command byte and any          *
 * additional offset count bytes.  The replacement count value in the    *
 * command byte is added ot the value in the additional replacement      *
 * count byte.  If the additional replacement count byte is 0, then      *
 * the replacement count is 31. If the additional replacement count      *
 * byte is 255, then another additional replacement count byte           *
 * follows.  The last additional replacement count byte is indicated     *
 * by a value less than 255.  All the additonal replacement count        *
 * bytes are added to the replacement count in the command byte          *
 * to get the total replacement byte count.  Two more than the total     *
 * replacement byte count will be replaced.                              *
 *                                                                       *
 *                                                                       *
 *      Inputs:    iTotalBytes   - the number of bytes in the row        *
 *                 pbData        - pointer to current raster data row    *
 *                 pbLastLine    - pointer to previous raster data row   *
 *                 usMaxReturn   - Length of output buffer pointed       *
 *                                 by pbReturn                           *
 *                 pbReturn      - pointer to compressed data returned   *
 *                 pDeltas       - pointer to an array of struct {       *
 *                                 int start, int end }  where each      *
 *                                 start, end pair is the index to       *
 *                                 beginning and end of a difference     *
 *                                 fragment.                             *
 *                                                                       *
 *     Outputs:    pbReturn points to the compressed data                *
 *                                                                       *
 *                                                                       *
 *     Returns:    0 if current and previous line are identical else     *
 *                 > 0 = length of data pointed to by pbReturn           *
 *                 -1 = GPLCOMPRESS_ERROR = usMaxOutput not large        *
 *                      enough for the compressed output                 *
 *                                                                       *
 *     History:    07/11/94  mjones wrote it                             *
 *                                                                       *
 *     Copywrite IBM Corporation 1994                                    *
 *                                                                       *
 *************************************************************************/
int
GplCompressRLLDeltaRow (INT     iTotalBytes,   // bytes in the scan line
                        PBYTE   pbData,        // raster data to compress
                        PBYTE   pbLastLine,    // previous scanline's raster data
                        INT     usMaxReturn,   // size of output buffer
                        PBYTE   pbReturn,      // compressed data will be written
                        PUSHORT pDeltas)       // differences indexes = array
                                               // of start, end pairs.
{
   INT   dl = 0;        // index into pDeltas (Start,End) pairs
   INT   startdiff;     // start of difference string
   INT   enddiff;       // end of differnce string
   INT   lastend;       // used for relative offset from last difference string
   INT   offset;        // used to output offset byte(s)
   INT   l;             // offset in pbReturn = length of return compressed bytes
   BOOL  dofirstoffset; // Has first offset been set, then other offsets are zero
   INT   lengthdiff;    // length of difference sequence between current line and previous line
   INT   clengthdiff;   // length of difference sequence to keep for copying to output
   INT   i;             // local counter
   INT   d;             // index into difference fragments with length >= 4
   BOOL  flagstartdiff; // flag for difference sequence within difference fragment
   INT   dupstartindex; // start index within a difference fragment for dups
   INT   lengthdups;    // length of dups within a difference fragment
   INT   clengthdups;   // length of dups to keep for copying to output
   PBYTE ptemp;         // for constant pointers in a loop

   if ((startdiff = *(pDeltas + dl++)) == 0)
      return 0;   // previous and current are the same
   l = lastend = 0; //

   do
   {
      enddiff = *(pDeltas + dl++);            // get end of difference string
      offset = startdiff - lastend - 1; // relative offset = offset from last end
      lastend = enddiff;                     // set new last end

      lengthdiff = enddiff - startdiff + 1;

      if (lengthdiff < 4)       // output noncompress command byte
      {
         if (offset >= 15)
         {
            *(pbReturn + l++) = 0x78 | lengthdiff - 1;  // command byte = 0111 1rrr
            if (l > usMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
            offset -= 15;

            while (offset >= 255)      // output additional offset bytes of 255
            {
               *(pbReturn + l++) = 0xFF;
               if (l > usMaxReturn)
                  return GplCompression::GPLCOMPRESS_ERROR;
               offset -= 255;
            }

            *(pbReturn + l++) = offset;   // output last offset byte
            if (l > usMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;

         }
         else
         {
            *(pbReturn + l++) =  offset << 3 | lengthdiff - 1;
            if (l > usMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }

         ptemp = pbData + startdiff - 1;   // output difference bytes

         for (i = 0; i < lengthdiff; i++)
         {
            *(pbReturn + l++) = *(ptemp + i);
            if (l > usMaxReturn)
               return GplCompression::GPLCOMPRESS_ERROR;
         }

         continue;    // do new difference fragment
      }

      flagstartdiff = false;

      startdiff -= 1;
      enddiff -= 1;

      d = startdiff;
      dofirstoffset = true;

      do   // output difference fragment
      {
         if (*(pbData + d) != *(pbData + d + 1))
         {
            if (flagstartdiff == false)
            {
               startdiff = d;              // save start of difference sequence
               flagstartdiff = true;
            }
         }
         else
         {
#ifdef PPC                                             // for Power PC
            if (  *(pbData + d) == *(pbData + d + 2 )  // already know *(pbData + d ) ==
               && *(pbData + d) == *(pbData + d + 3 )  // *(pbData + d + 1 )
               )
#else
            if (*(PUSHORT)(pbData + d) == *(PUSHORT)(pbData + d + 2 ))
#endif
            {
               if (flagstartdiff == true) // output difference sequence first
               {
                  flagstartdiff = false;  // reset single switch

                  clengthdiff = lengthdiff = d - startdiff;   // get length of difference sequence

                  if (dofirstoffset)    // output index from last difference fragment
                  {
                     dofirstoffset = false;

                     if (offset >= 15)
                     {
                        if (lengthdiff > 7)
                        {
                           lengthdiff -= 7;
                           *(pbReturn + l++) = 0x7F;  // command byte = 0111 1111
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                        }
                        else
                        {
                           *(pbReturn + l++) = 0x78 | lengthdiff - 1;  // command byte = 0111 1rrr
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                        }

                        offset -= 15;

                        while (offset >= 255)      // additional offset bytes MUST! follow
                        {
                           *(pbReturn + l++) = 0xFF;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                           offset -= 255;
                        }

                        *(pbReturn + l++) =  offset;   // output last offset byte
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;

                        if (clengthdiff > 7)
                        {
                           lengthdiff -= 1;

                           // additional count byte MUST! follow
                           while (lengthdiff >= 255)
                           {
                              *(pbReturn + l++) = 0xFF;
                              if (l > usMaxReturn)
                                 return GplCompression::GPLCOMPRESS_ERROR;
                              lengthdiff -= 255;
                           }

                           *(pbReturn + l++) = lengthdiff;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                        }
                     }
                     else
                     {
                        if (lengthdiff > 7)
                        {
                           lengthdiff -= 7;
                           *(pbReturn + l++) = offset << 3 | 0x07;  // command byte = 0ooo o111
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                           if (clengthdiff > 7)
                           {
                              lengthdiff -= 1;

                              // additional count byte MUST! follow
                              while (lengthdiff >= 255)
                              {
                                 *(pbReturn + l++) = 0xFF;
                                 if (l > usMaxReturn)
                                    return GplCompression::GPLCOMPRESS_ERROR;
                                 lengthdiff -= 255;
                              }

                              *(pbReturn + l++) = lengthdiff;
                              if (l > usMaxReturn)
                                 return GplCompression::GPLCOMPRESS_ERROR;
                           }
                        }
                        else
                        {
                           *(pbReturn + l++) = offset << 3 | lengthdiff - 1;  // command byte = 0111 1rrr
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                        }
                     }

                     ptemp = pbData + startdiff;   // output difference bytes

                     for (i = 0; i < clengthdiff ; i++)
                     {
                        *(pbReturn + l++) = *(ptemp + i);
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }

                  }
                  else   /* if dofirstoffset */
                  {
                     if (lengthdiff > 7)
                     {
                        lengthdiff -= 7;
                        *(pbReturn + l++) = 0x07;  // command byte = 0000 0111
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;

                        if (clengthdiff > 7)
                        {
                           lengthdiff -= 1;

                           // additional count byte MUST! follow
                           while (lengthdiff >= 255)
                           {
                              *(pbReturn + l++) = 0xFF;
                              if (l > usMaxReturn)
                                 return GplCompression::GPLCOMPRESS_ERROR;
                              lengthdiff -= 255;
                           }

                           *(pbReturn + l++) = lengthdiff;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                        }
                     }
                     else
                     {
                        *(pbReturn + l++) = lengthdiff - 1 ;  // command byte = 0000 0rrr
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }

                     ptemp = pbData + startdiff;   // output difference bytes

                     for (i = 0; i < clengthdiff; i++)
                     {
                        *(pbReturn + l++) = *(ptemp + i);
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }   /* endif dofirstoffset */
               }  /* if flagstartdiff == true */

               /*
                *  Output replicates now
                *
                */
               dupstartindex = d;
               while (*(pbData + d) == * (pbData + d + 1 ) && d < enddiff)
                  ++d;
               // now output dups sequence
               clengthdups = lengthdups = d - dupstartindex + 1;

               if (dofirstoffset)    // output index from last difference fragment
               {
                  dofirstoffset = false;

                  if (offset >= 3)
                  {
                     if (lengthdups > 32)
                     {
                        lengthdups -= 32;
                        *(pbReturn + l++) = 0xFF;  // command byte = 1111 11111
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                     else
                     {
                        *(pbReturn + l++) = 0xE0 | lengthdups - 2 ;  // command byte = 111r rrrr
                                                                     // where rrr is replacecount
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }

                     offset -= 3;

                     while (offset >= 255)      // output additional offset bytes MUST! follow
                     {
                        *(pbReturn + l++) = 0xFF;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                        offset -= 255;
                     }

                     *(pbReturn + l++) =  offset;   // output last offset byte
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;

                     if (clengthdups > 32)
                     {
                        lengthdups -= 1;

                        // additional count byte MUST! follow
                        while (lengthdups >= 255)
                        {
                           *(pbReturn + l++) = 0xFF;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                           lengthdups -= 255;
                        }

                        *(pbReturn + l++) = lengthdups;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }
                  else
                  {
                     if (lengthdups > 32)
                     {
                        lengthdups -= 32;
                        *(pbReturn + l++) = offset << 5 | 0x9F;  // command byte = 1oo1 1111
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;

                        if (clengthdups > 32)
                        {
                           lengthdups -= 1;

                           // additional count byte MUST! follow
                           while (lengthdups >= 255)
                           {
                              *(pbReturn + l++) = 0xFF;
                              if (l > usMaxReturn)
                                 return GplCompression::GPLCOMPRESS_ERROR;

                              lengthdups -= 255;
                           }

                           *(pbReturn + l++) = lengthdups;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                        }
                     }
                     else
                     {
                        *(pbReturn + l++) = 0x80 | offset << 5 | lengthdups - 2;  // command byte = 1oor rrrr
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }  /* endif if offset >= 3 */

                  *(pbReturn + l++) = *(pbData + dupstartindex);  // output dup byte
                  if (l > usMaxReturn)
                     return GplCompression::GPLCOMPRESS_ERROR;
               }
               else     /* if dofirstoffset */
               {
                  if (lengthdups > 32)
                  {
                     lengthdups -= 32;
                     *(pbReturn + l++) = 0x9F;  // command byte = 1001 1111
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;

                     if (clengthdups > 32)
                     {
                        lengthdups -= 1;

                        // additional count byte MUST! follow
                        while (lengthdups >= 255)
                        {
                           *(pbReturn + l++) = 0xFF;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                           lengthdups -= 255;
                        }

                        *(pbReturn + l++) = lengthdups;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }
                  else
                  {
                     *(pbReturn + l++) = 0x80 | lengthdups - 2 ;  // command byte = 1000 0rrr
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;

                  }

                  *(pbReturn + l++) = *(pbData + dupstartindex);  // output dup byte
                  if (l > usMaxReturn)
                     return GplCompression::GPLCOMPRESS_ERROR;
               }
               if (d == enddiff) break;     // difference fragment ended in dups at least 4 dups
            }
            else
            {
               if (flagstartdiff == false)
               {
                  startdiff = d;      // save start of difference sequence
                  flagstartdiff = true;
               }
            }
         }

         if (enddiff - d  < 4)   // output difference sequence within difference fragment
         {
            if (flagstartdiff)     // output with startdiff at beginning
            {
               flagstartdiff = false;  // reset single switch
               clengthdiff = lengthdiff = enddiff - startdiff + 1;   // get length of difference sequence

               if (dofirstoffset)    // output index from last difference fragment
               {
                  dofirstoffset = false;

                  if (offset >= 15)
                  {
                     if (lengthdiff > 7)
                     {
                        lengthdiff -= 7;
                        *(pbReturn + l++) = 0x7F;  // command byte = 0111 1111
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                     else
                     {
                        *(pbReturn + l++) = 0x78 | lengthdiff - 1;  // command byte = 0111 1rrr
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }

                     offset -= 15;

                     while (offset >= 255)      // additional offset bytes MUST! follow
                     {
                        *(pbReturn + l++) = 0xFF;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;

                        offset -= 255;
                     }

                     *(pbReturn + l++) =  offset;   // output last offset byte
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;

                     if (clengthdiff > 7)
                     {
                        lengthdiff -= 1;

                        // additional count byte MUST! follow
                        while (lengthdiff >= 255)
                        {
                           *(pbReturn + l++) = 0xFF;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                           lengthdiff -= 255;
                        }

                        *(pbReturn + l++) = lengthdiff;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }
                  else
                  {
                     if (lengthdiff > 7)
                     {
                        lengthdiff -= 7;
                        *(pbReturn + l++) = offset << 3 | 0x07;  // command byte = 0ooo o111
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;

                        if (clengthdiff > 7)
                        {
                           lengthdiff -= 1;

                           // additional count byte MUST! follow
                           while (lengthdiff >= 255)
                           {
                              *(pbReturn + l++) = 0xFF;
                              if (l > usMaxReturn)
                                 return GplCompression::GPLCOMPRESS_ERROR;

                              lengthdiff -= 255;
                           }

                           *(pbReturn + l++) = lengthdiff;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;
                        }
                     }
                     else
                     {
                        *(pbReturn + l++) = offset << 3 | lengthdiff - 1;  // command byte = 0111 1rrr
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }

                  ptemp = pbData + startdiff;      // output difference bytes

                  for (i = 0; i < clengthdiff; i++)
                  {
                     *(pbReturn + l++) = *(ptemp + i);
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;
                  }
               }
               else   /* if dofirstoffset */
               {
                  if (lengthdiff > 7)
                  {
                     lengthdiff -= 7;
                     *(pbReturn + l++) = 0x07;  // command byte = 0000 0111
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;

                     if (clengthdiff > 7)
                     {
                        lengthdiff -= 1;

                        // additional count MUST! follow
                        while (lengthdiff >= 255)
                        {
                           *(pbReturn + l++) = 0xFF;
                           if (l > usMaxReturn)
                              return GplCompression::GPLCOMPRESS_ERROR;

                           lengthdiff -= 255;
                        }

                        *(pbReturn + l++) = lengthdiff;
                        if (l > usMaxReturn)
                           return GplCompression::GPLCOMPRESS_ERROR;
                     }
                  }
                  else
                  {
                     *(pbReturn + l++) = lengthdiff - 1;  // command byte = 0000 0rrr
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;
                  }

                  ptemp = pbData + startdiff;       // output difference bytes

                  for (i = 0; i < clengthdiff; i++)
                  {
                     *(pbReturn + l++) = *(ptemp + i);
                     if (l > usMaxReturn)
                        return GplCompression::GPLCOMPRESS_ERROR;
                  }

               }   /* endif dofirstoffset */
            }
            else
            {
               /* not flagstartdiff */
               lengthdiff = enddiff - d;

               *(pbReturn + l++) =  lengthdiff - 1;
               if (l > usMaxReturn)
                  return GplCompression::GPLCOMPRESS_ERROR;

               ptemp = pbData + d + 1;          // output difference bytes

               for (i = 0; i < lengthdiff; i++)
               {
                  *(pbReturn + l++) = *(ptemp + i);
                  if (l > usMaxReturn)
                     return GplCompression::GPLCOMPRESS_ERROR;
               }
            }         /* if flagstartdiff */

            break; // difference fragment ended
         }   /* if ( enddiff - d < 4 ) */

      } while (++d < enddiff);

   } while ((startdiff = *(pDeltas + dl++)) != 0);

   return l;  // length of compressed data
}

/************************************************************************
 * Function:      GplpChooseMode9                                       *
 *                                                                      *
 * Description:   Chooses which form of Mode 9 to use                   *
 *                                                                      *
 * Return:        New position in Output Buffer                         *
 *                                                                      *
 * History:       1/12/94 Mike Jones [IBM] - Wrote it                   *
 *                                                                      *
 ************************************************************************/
PBYTE
GplpChooseMode9 (SHORT   offset,
                 USHORT *pOutBytes,
                 PBYTE   pbReturn,
                 PBYTE   pbReplace,
                 SHORT   ReplaceCount)
{
   SHORT         loffset;
   SHORT         loffset1;    /* Offsets from last change */
   SHORT         ReplCnt;
   SHORT         lReplace;
   SHORT         lReplace1;
   BYTE          cmdbyte;
   USHORT        OutBytes;
   int           repeat_seq;
   int           diff_seq;
   int           i;
   int           l;
   unsigned char cdata      = 0;

   /* Make local copy of *pOutBytes for efficiency */
   OutBytes = *pOutBytes;

   /* Initialise counters */

   repeat_seq = diff_seq = 0;

   /* Pre-load 'data' with first graphic image data.  */

   for (i = ReplaceCount; i > 0; /* i changed within loop */)
   {
      // While there is data in the scan line

      /* Collect repeating data sequence.
       * Unlike Tiff Packbits, we use a positive counter
       * and we do not have a limit to the repeat size
       */
      if (i > 3)
      {
         if (  *pbReplace == *(pbReplace + 1)
            && *((short int *)pbReplace) == *((short int *)pbReplace + 1)
            )
         {
            cdata = *pbReplace;
            repeat_seq += 4;
            pbReplace += 4;
            i -= 4;
            while (  (i > 0)
                  && (cdata == *pbReplace)
                  )                               /* find others */
            {
               repeat_seq++;
               pbReplace++;
               i--;
            }
         }

         /* Output repeat sequence as two bytes. */
         if (repeat_seq > 0)
         {
            // Control Bit = 1
            cmdbyte = 0x80;

            // Add offset count to command byte (max 3)
            loffset = offset - MAX_OFF_COMP;

            if (offset > (MAX_OFF_COMP - 1))
               offset = MAX_OFF_COMP;

            loffset1 = loffset - 255;

            if (loffset > 254)
               loffset = 255;

            // We want offset in bits 6..5 (if bit order 7..0)
            offset <<= 5;
            cmdbyte |= offset;

            // Add replacement count to command byte (max 31)
            // We encode 2 less than replacement count
            ReplCnt = repeat_seq - 2;

            lReplace = ReplCnt - MAX_REPL_COMP;

            if (ReplCnt > (MAX_REPL_COMP - 1))
               ReplCnt = MAX_REPL_COMP;

            lReplace1 = lReplace - 255;

            if (lReplace > 254)
               lReplace = 255;

            cmdbyte |= ReplCnt;

            *pbReturn++ = cmdbyte;
            OutBytes++;

            // Using up to 2 additional offset/replacement bytes allows a maximum
            // count of 255+255+(7 or 15) - versus our maximum line length of
            // 8 inches (300 bytes)

            // Now handle any additional offset bytes
            if (loffset >= 0)
            {
               *pbReturn++ = (char)loffset;
               OutBytes++;

               if (loffset1 >= 0)
               {
                  *pbReturn++ = (char)loffset1;
                  OutBytes++;
               }
            }

            // Next, any additional Replacement bytes
            if (lReplace >= 0)
            {
               *pbReturn++ = (char)lReplace;
               OutBytes++;

               if (lReplace1 >= 0)
               {
                  *pbReturn++ = (char)lReplace1;
                  OutBytes++;
               }
            }

            // Finally, add the data byte
            *pbReturn++ = cdata;
            OutBytes++;

            repeat_seq = 0;
            offset = 0;
         }
      }
      /* Collect differing data sequence */

      if (i < 4)       /* Always get bytes up to 3 max as literal */
      {
         diff_seq += i;
         pbReplace += i;
         i = 0;
      }
      else
      {
         while (  *pbReplace != *(pbReplace + 1)
               || *((short int *)pbReplace) != *((short int *)pbReplace + 1)
               )
         {
            ++diff_seq;
            ++pbReplace;
            --i;
            if (i < 4)
            {
               diff_seq += i;
               pbReplace += i;
               i = 0;
               break;
            }
         }
      }

      /* Output (data counter - 1) (data sequence) */
      if (diff_seq > 0)
      {
         // Send replacement bytes uncompressed
         // Control Bit = 0
         cmdbyte = 0x00;

         // Add offset count to command byte (max 15)
         loffset = offset - MAX_OFF_UNC;

         if (offset > (MAX_OFF_UNC - 1))
            offset = MAX_OFF_UNC;

         loffset1 = loffset - 255;

         if (loffset > 254)
            loffset = 255;

         // We want offset in bits 6..3 (if bit order 7..0)
         offset <<= 3;
         cmdbyte |= offset;

         // Add replacement count to command byte (max 7)
         ReplCnt = diff_seq - 1;  // We encode 1 less than replacement count

         lReplace = ReplCnt - MAX_REPL_UNC;

         if (ReplCnt > (MAX_REPL_UNC - 1))
            ReplCnt = MAX_REPL_UNC;

         lReplace1 = lReplace - 255;

         if (lReplace > 254)
            lReplace = 255;

         cmdbyte |= ReplCnt;

         *pbReturn++ = cmdbyte;
         OutBytes++;

         // Using up to 2 additional offset/replacement bytes allows a maximum
         // count of 255+255+(7 or 15) - versus our maximum line length of
         // 8 inches (300 bytes)

         // Now handle any additional offset bytes
         if (loffset >= 0)
         {
            *pbReturn++ = (char)loffset;
            OutBytes++;

            if (loffset1 >= 0)
            {
               *pbReturn++ = (char)loffset1;
               OutBytes++;
            }
         }

         // Next, any additional Replacement bytes
         if (lReplace >= 0)
         {
            *pbReturn++ = (char)lReplace;
            OutBytes++;

            if (lReplace1 >= 0)
            {
               *pbReturn++ = (char)lReplace1;
               OutBytes++;
            }
         }

         // Finally, add the data
         // Move pointer to start of sequence */

         pbReplace -= diff_seq;

         for (l = 0; l < diff_seq; l++)
            *pbReturn++ = *pbReplace++;

         OutBytes += diff_seq;

         /* Reset Counters */
         diff_seq = 0;
         offset = 0;
      }
   }

   // Copy new value for *pOutBytes
   *pOutBytes = OutBytes;

   // Return the new output buffer position
   return pbReturn;
}

/************************************************************************
 *                                                                      *
 * Function:      GplCompressMode9Out                                   *
 *                                                                      *
 * Inputs: int  iTotalBytes  = orginal number of bytes                  *
 *         char * pbData     = pointer to orginal string                *
 *         char * pbLastline = pointer to previous string               *
 *         char * pbReturn   = pointer to return string                 *
 *                                                                      *
 * Returns: int = number of bytes in compressed return string           *
 *                                                                      *
 * Description:   Compresses a string of bytes using the PCL Mode 9     *
 *                algorithm.  Returns the compressed string at pbReturn *
 *                and the new string usLength as its return value.      *
 *                This method replaces only bytes in the current row    *
 *                that are different from the proceeding seed row,      *
 *                similarly to Method 3.  However unlike Method 3,      *
 *                the replacement or delta bytes themselves are encoded.*
 *                A delta compression string consists of a command byte,*
 *                optional replacement count bytes and finally the      *
 *                replacement data.                                     *
 *                                                                      *
 *  ------------------------------------------------------------------  *
 * | command byte | offset byte | replacement count bytes | data byte | *
 *  ------------------------------------------------------------------  *
 *                                                                      *
 * The offset byte and replacement count are optional.  The command     *
 * byte itself has three parts: the control bit to determine the        *
 * encoding of the replacement bytes, the left offset from the current  *
 * byte position, and the number of consecutive bytes to replace.       *
 *                                                                      *
 * The control bit determines the compression method of replacement     *
 * data.  It also determines where the bit boundaries are for the       *
 * next two fields.                                                     *
 *                                                                      *
 *     bit         7         6            3   2            0            *
 *       | control bit = 0 | offset count  | replace count |            *
 *                                                                      *
 * If the control bit is 0 then the replacement data is uncompressed    *
 * and bits 6 through 3 contain the offset count and bits 2 through     *
 * 0 contain the replacement count.  The replacement bytes that follow  *
 * the command byte then replace the replacement count of bytes offset  *
 * from the current position within the seed row by the offset count.   *
 *                                                                      *
 * If the control bit is 0 and the offst count is 0 to 14 then that     *
 * number of bytes are ignored or offset prior to replacing bytes.      *
 *                                                                      *
 * I the control bit is 0 and the offset count is 15 then an additional *
 * optional offset count byte follows the command byte.  The offset     *
 * count value in the command byte is added to the value in the         *
 * additional offset count byte.  If the additional offset count byte   *
 * is 0, the offset count is 15.  If the additional offset count byte   *
 * is 255, another addtional offset count byte follows.  The last       *
 * additional offset count byte is indicated by a value of less than    *
 * 255.  All of the additional offset count bytes are added to the      *
 * offset count in the command byte to get the total offset count.      *
 *                                                                      *
 * If the control bit is 0 and the replacement count is 0 to 6 then one *
 * more than the replacement count bytes will be replaced.              *
 *                                                                      *
 * If the control bit is 0 and replacement count is 7 then an           *
 * additional replacement byte follows the command byte and any         *
 * additional offset offset count bytes.  The replacement count value   *
 * in the command byte is added to the value in the additional          *
 * replacement count byte.  If the additional replacement count byte    *
 * is 0, then  8 bytes are replaced as the replacement count is 7.      *
 * If the additional replacement count byte is 255, then another        *
 * additional replacement count byte follows.  The last replacement     *
 * count byte is indicated by a value les than 255.  All the            *
 * additional replacement count bytes are added to the replacement      *
 * count in the command byte to get the total replacement byte count.   *
 * One more than the total replacement byte count will be replaced.     *
 *                                                                      *
 *     bit         7         6            5  4            0             *
 *       | control bit = 1 | offset count  | replace count |            *
 *                                                                      *
 * If the control bit is 1 then the replacement data is run length      *
 * encoded and bits 6 through 5 contain the offset count and bits       *
 * 4 through 0 contain the replacement count.  The replacement bytes    *
 * that follows the command byte then replace the replacement count     *
 * of bytes offset from the current position within the seed row by     *
 * the offset count.                                                    *
 *                                                                      *
 * If the contol bit is 1 and the offset count is 0 to 2 then that      *
 * number of bytes is ignored or offset prior to replacing bytes.       *
 *                                                                      *
 * If the control bit is 1 and the offset count is 3 then an            *
 * additional optional offset count byte follows the command byte.      *
 * The offset count value in the command byte is added to the value     *
 * in the additional offset count bytes.  If the additional offset      *
 * count byte is 0, the offset count is 3.  If the additonal offset     *
 * count byte is 255, another addtional offset count byte follows.      *
 * The last addtitional offset byte is indicated by a value less        *
 * than 256.  All of the additional offset count bytes are added        *
 * to the offset count in the command byte to get the total offset      *
 * count.                                                               *
 *                                                                      *
 * If the control bit is 1 and the replacement is 0 to 30 then two      *
 * more than the replacement count bytes will be replaced.              *
 *                                                                      *
 * If the control bit is 1 and the replacement count is 31 than an      *
 * additional replacement byte follows the command byte and any         *
 * additional offset count bytes.  The replacement count value in the   *
 * command byte is added ot the value in the additional replacement     *
 * count byte.  If the additional replacement count byte is 0, then     *
 * the replacement count is 31. If the additional replacement count     *
 * byte is 255, then another additional replacement count byte          *
 * follows.  The last additional replacement count byte is indicated    *
 * by a value less than 255.  All the additonal replacement count       *
 * bytes are added to the replacement count in the command byte         *
 * to get the total replacement byte count.  Two more than the total    *
 * replacement byte count will be replaced.                             *
 *                                                                      *
 * Return:        int = size of compressed data in pbReturn             *
 *                                                                      *
 * History:       2/01/94 Mike Jones [IBM] - Wrote it                   *
 *                                                                      *
 ************************************************************************/
int
GplCompressMode9Out (int   iTotalBytes,  // original number of bytes
                     PBYTE pbData,       // pointer to original string
                     PBYTE pbLastLine,   // pointer to previous scanline's string
                     PBYTE pbReturn)     // pointer to return string
{
   SHORT  j;             // index variable
   SHORT  cap       = 0; // Current Active Position (x only)
   USHORT outbytes  = 0; // number of return bytes
   SHORT  temp      = 0; // number of bytes since last change
   SHORT  offset;        // offset from last change
   PBYTE  pbLocLast;     // local copy of pointer to string

   pbLocLast = pbLastLine;

   for (j = 0; j < iTotalBytes; )
   {
      while (  j < iTotalBytes
            && *pbLocLast == *(pbData + j)
            )
      {
         if (temp)
         {
            // if this is the first non-change after at least one change
            // compute offsets from last changed byte
            offset = j - cap - temp;
            cap = j;

            // Encode Compressed/Uncompressed Delta as appropriate
            // and add to the output buffer
            // Replacement bytes are at (pbLoc - temp - 1)

            pbReturn = GplpChooseMode9 (offset,
                                        (USHORT *)&outbytes,
                                        pbReturn,
                                        ((pbData + j + 1) - temp - 1),
                                        temp);

            temp = 0;
         }

         // Copy in Compress() now
         pbLocLast++;

         // Inc j
         j++;
      }

      while (  j < iTotalBytes
            && *pbLocLast != *(pbData + j)
            )
      {
         // Copy in Compress() now
         pbLocLast++;

         // Increment count of replacement bytes
         temp++;

         // Inc counter
         j++;
      }
   }

   if (temp)
   {
      offset = j - cap - temp;

      // Encode Compressed/Uncompressed Delta as appropriate
      // and add to the output buffer
      // Replacement bytes are at (pbLoc - temp)

      pbReturn = GplpChooseMode9 (offset,
                                  (USHORT *)&outbytes,
                                  pbReturn,
                                  ((pbData + j) - temp),
                                  temp);
   }

   return outbytes;
}

/*************************************************************************
 *                                                                       *
 * Function:            GplCompressChooseMode()                          *
 *                                                                       *
 *                                                                       *
 * Description:    Uses a semi-intelligent algorithm to select           *
 *                 the compression mode that will most likely            *
 *                 give the best compression for the row of data.        *
 *                                                                       *
 *                 Mode 9 (RLL Delta Row) is currently chosen            *
 *                 if Mode 3 is the "best" and the printer supports      *
 *                 mode 9 compression.                                   *
 *                                                                       *
 *                 Mode 3 (Delta Row) is chosen if most of the bytes     *
 *                 are the same as the corresponding bytes in the        *
 *                 previous row                                          *
 *                                                                       *
 *                 Mode 2 (TIFF) is chosen if most of the bytes are      *
 *                 repeated within this row                              *
 *                                                                       *
 *                 Mode 1 (RLL) is chosen if most of the bytes are       *
 *                 repeated within this row and the printer does not     *
 *                 support mode 2.                                       *
 *                                                                       *
 *                 Mode 0 (No compression) is chosen of most bytes       *
 *                 are neither repeats within the row nor duplicates     *
 *                 of the previous row.                                  *
 *                                                                       *
 *      Inputs:    pbRow         - a pointer to the current row's data   *
 *                 pbLast_Row    - a pointer to the last row's data      *
 *                 usRow_Length  - the number of bytes in the row        *
 *                 CompressModes - compress modes supported              *
 *                                 This is ULONG with the follow bit     *
 *                                 positions used to represent compress  *
 *                                 modes that this printer supports.     *
 *                                                                       *
 *                                 GPLCOMPRESS_NONE          0           *
 *                                 GPLCOMPRESS_RLL           1           *
 *                                 GPLCOMPRESS_TIFF          2           *
 *                                 GPLCOMPRESS_DELTAROW      4           *
 *                                 GPLCOMPRESS_RLLDELTAROW   8           *
 *                                                                       *
 *                                 Note that you bitwise or the          *
 *                                 compression modes supported.          *
 *                                                                       *
 *                 pDelta        - array of ints to hold the offsets     *
 *                                 from the begining of pbRow that       *
 *                                 differ from pbLast_Row.  This         *
 *                                 data will be used by mode 3 and       *
 *                                 mode 9.  Note that pDelta must have   *
 *                                 a length equal to 2 times             *
 *                                 the smallest even integer greater     *
 *                                 or equal to usRow_Length + 2.         *
 *                                 There is no check on the              *
 *                                 size of pDelta.  For example          *
 *                                 if usRow_Length = 2 bytes then        *
 *                                 the length of pDelta must be at       *
 *                                 at least 2*2+2=6 since 2 is the       *
 *                                 smallest even integer greater than    *
 *                                 or equal to 2.  If usRow_Length is    *
 *                                 3 bytes then the length of pDelta     *
 *                                 must be at least 2*4+2=10 since 4 is  *
 *                                 the smallest even integer greater     *
 *                                 than or equal to 3.                   *
 *                                                                       *
 *     Outputs:    For mode 3 and 9 pDelta is filled with the start      *
 *                 and end of the difference fragments.  Note that a     *
 *                 difference fragment is defined to be the fragments    *
 *                 in the current row that differ with the previous      *
 *                 row.                                                  *
 *                                                                       *
 *                 Example:                                              *
 *                                                                       *
 *         Current Row bytes:                                            *
 *         7,  8,  1,  0,  4,  5,  6,  7,  23,  128, 257,   1,   7,  8   *
 *                                                                       *
 *         Previous Row bytes:                                           *
 *         7,  1,  3,  1,  4,  5,  9,  7,  23,  128,   7,   1,   7,  8   *
 *                                                                       *
 *             ^       ^          ^  ^                 ^   ^             *
 *         indexes                                                       *
 *         1   2   3   4   5   6   7   8   9    10    11   12   13   14  *
 *                                                                       *
 *         There are three difference fragments:                         *
 *         a:  Fragment one starts at index 2 and ends at index 4        *
 *         b:  Fragment two starts at index 9 and ends at index 9        *
 *         c:  Fragment three starts at index 11 and ends at index 12    *
 *                                                                       *
 *         pDelta above will point to a array of the indexes stored to   *
 *         used by GplCompressDeltaRow() and GplCompressRLLDeltaRow()    *
 *                                                                       *
 *                                                                       *
 *     Returns:    The mode that will probably compress the best         *
 *                 0 = no compression                                    *
 *                 1 = RLL ( Run-Length Limited ) compression            *
 *                 2 = TIFF ( Tagged Image File Format) compression      *
 *                 3 = HP's Mode 3 = Delta Row compression               *
 *                 9 = HP's Mode 9 = RLL Delta Row compression           *
 *                                                                       *
 *     History:    10/22/90  pcv created it.                             *
 *                 02/14/91  tw inserted into os/2 driver                *
 *                 01/31/94  mjones compiled to 32-c                     *
 *                 01/31/94  mjones added comments                       *
 *                 07/11/94  mjones wrote to use mode3/mode9/mode 1      *
 *                                                                       *
 *    Copywrite IBM Corporation 1994                                     *
 *                                                                       *
 *                                                                       *
 *************************************************************************/
int
GplCompressChooseMode (PBYTE               pbRow,
                       PBYTE               pbLastRow,
                       int                 iRowLength,
                       int                 iCompressModes,
                       unsigned short int *pDelta)
{
   int  i,                    // loop index
        iMode2Count = 0,      // the # of repetitions within the row
        iMode3Count = 0,      // the # of bytes that are the same as last_row
        iCompress;            // The threshold that must be exceeded for
                              // compression to take place
   bool fStartDiff  = false;  // for a delta change indexes
   int  j           = 0;      // Deltas index
   bool fCurrentPrevious;     // flag to do current and previous line comparison

   if (  (iCompressModes & GplCompression::GPLCOMPRESS_DELTAROW)
      || (iCompressModes & GplCompression::GPLCOMPRESS_RLLDELTAROW)
      )
      fCurrentPrevious = true;
   else
      fCurrentPrevious = false;

   // Our testing has shown this to be a good value
   iCompress = iRowLength >> 2;

   if (fCurrentPrevious)
   {
      // Only check mode3 if we support it
      if (*pbRow == *pbLastRow)
      {
         iMode3Count++;
      }
      else
      {
         *(pDelta + j++) = 1;     // record start of difference
         fStartDiff = true;
      }

      for (i = 1; i < iRowLength; i++)
      {
         if ((*(pbRow + i)) == *(pbLastRow + i))
         {
            if (fStartDiff)    // record end of difference
            {
               *(pDelta + j++) = i;
               fStartDiff = false;
            }

            iMode3Count++;
         }
         else
         {
            if (!fStartDiff)    // record start of difference
            {
               *(pDelta + j++) = i + 1;
               fStartDiff = true;
            }
         }

         if (*(pbRow + i) == *(pbRow + i - 1))      // for TIFF testing
            iMode2Count++;
      }

      if (fStartDiff)    // record end of differnce at boundary
         *(pDelta + j++) = i;

      *(pDelta + j) = 0;    // null terminiate difference string
   }
   else
   {
      for (i = 1; i < iRowLength; i++)
      {
         if (*(pbRow + i) == *(pbRow + i - 1))      // for TIFF testing
            iMode2Count++;
      }
   }

   if (iCompressModes & GplCompression::GPLCOMPRESS_RLLDELTAROW)
   {
      // If we identify mode 3 or 2 as the best and the printer supports
      // Mode 9 - then we use Mode 9 - Approx. RLL plus delta offsets

      if (  iMode3Count > iCompress
         || iMode2Count > iCompress
         )
         return COMPRESS_MODE_ENHANCED_DELTA_ROW;
      else
         return COMPRESS_MODE_NONE;
   }

   if (  (iCompressModes & GplCompression::GPLCOMPRESS_DELTAROW)
      && iMode3Count >= iMode2Count
      )
   {
      if (iMode3Count > iCompress)
         return COMPRESS_MODE_DELTA_ROW;
      else
         return COMPRESS_MODE_NONE;
   }
   else if (iMode2Count > iCompress)
   {
      if (iCompressModes & GplCompression::GPLCOMPRESS_TIFF)
         return COMPRESS_MODE_TIFF;
      if (iCompressModes & GplCompression::GPLCOMPRESS_RLL)
         return COMPRESS_MODE_RLL;
      else
         return COMPRESS_MODE_NONE;
   }
   else
   {
      return COMPRESS_MODE_NONE;
   }
}
