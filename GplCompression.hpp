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
#ifndef _GplCompression
#define _GplCompression

struct GplCompression; // Foreward reference to bypass cyclic reference

#include "defines.hpp"
#include "BinaryData.hpp"
#include "Device.hpp"

class GplCompressionCallback
{
public:
   virtual void compressionChanged (int         iNewCompression) = 0;
   virtual void sendData           (int         iLength,
                                    BinaryData *pbdData,
                                    int         iWhichPlane)     = 0;
};

class GplCompression
{
public:
   enum {
      GPLCOMPRESS_ERROR       = -1,
      GPLCOMPRESS_INVALID     = -1,
      GPLCOMPRESS_NONE        =  0,
      GPLCOMPRESS_RLL         =  1,
      GPLCOMPRESS_TIFF        =  2,
      GPLCOMPRESS_DELTAROW    =  4,
      GPLCOMPRESS_RLLDELTAROW =  8
   };

   GplCompression                       (int                     iPrintMode,
                                         int                     iCompressionModesSupported,
                                         int                     iBytesPerRow,
                                         GplCompressionCallback *pCallback);
   ~GplCompression                      ();

   void        resetBlankLineCount      ();
   void        incrementBlankLineCount  ();
   void        incrementBlankLineCount  (int         iBlankLines);
   int         getBlankLineCount        ();

   void        resetCompressionMode     ();
   bool        isCurrentCompressionMode (int         iMode);

   BinaryData *compressKRasterPlane     (BinaryData *pbdKPlane);
   BinaryData *compressCRasterPlane     (BinaryData *pbdCPlane);
   BinaryData *compressLCRasterPlane    (BinaryData *pbdLCPlane);
   BinaryData *compressMRasterPlane     (BinaryData *pbdMPlane);
   BinaryData *compressLMRasterPlane    (BinaryData *pbdLMPlane);
   BinaryData *compressYRasterPlane     (BinaryData *pbdYPlane);
   BinaryData *compressRGBRasterPlane   (BinaryData *pbdRGBPlane);

   void        clearLastLineBuffers     ();

private:
   int         compressRasterPlane      (PBYTE       pbBuffer,
                                         int         iPrinterBytesInArray,
                                         PBYTE       pbLastLine,
                                         PBYTE       pbCompress,
                                         int         cbCompressBuffer,
                                         int         iCompressModeSupported,
                                         PUSHORT     pDelta,
                                         int         iWhichPlane);

   int                     iCompressionModesSupported_d;
   int                     iBytesPerRow_d;
   int                     iBlankLineCount_d;

   int                     cbCompressBuffer_d;
   PBYTE                   pbCompressBuffer_d;
   PUSHORT                 pusDelta_d;

   BinaryData             *pbdKPlane_d;
   BinaryData             *pbdCPlane_d;
   BinaryData             *pbdLCPlane_d;
   BinaryData             *pbdMPlane_d;
   BinaryData             *pbdLMPlane_d;
   BinaryData             *pbdYPlane_d;

   PBYTE                   pbKPlane_d;
   PBYTE                   pbCPlane_d;
   PBYTE                   pbLCPlane_d;
   PBYTE                   pbMPlane_d;
   PBYTE                   pbLMPlane_d;
   PBYTE                   pbYPlane_d;

   PBYTE                   pbLastKPlane_d;
   PBYTE                   pbLastCPlane_d;
   PBYTE                   pbLastLCPlane_d;
   PBYTE                   pbLastMPlane_d;
   PBYTE                   pbLastLMPlane_d;
   PBYTE                   pbLastYPlane_d;

   GplCompressionCallback *pCallback_d;
   int                     iCurrentCompression_d;
};

#endif
