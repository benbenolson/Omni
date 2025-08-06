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
#ifndef _DeviceBlitter
#define _DeviceBlitter

#include "defines.hpp"

#include <iostream>
#include <sstream>
#include <string>

typedef enum {
   BITBLT_BITMAP,
   BITBLT_AREA,
   BITBLT_TEXT
} BITBLT_TYPE;

class DeviceBlitter
{
public:
                           DeviceBlitter            (PrintDevice         *pDevice);
   virtual                ~DeviceBlitter            ();

   virtual void            initializeInstance       ();

   virtual bool            hasError                 ();

   DeviceInstance         *getInstance              ();
   DeviceDither           *getDitherInstance        ();

   virtual bool            rasterize                (PBYTE                pbBits,
                                                     PBITMAPINFO2         pbmi,
                                                     PRECTL               prectlPageLocation,
                                                     BITBLT_TYPE          eType);

   DeviceOrientation      *getCurrentOrientation    ();
   PSZCRO                  getCurrentDitherID       ();
   DeviceForm             *getCurrentForm           ();
   DeviceTray             *getCurrentTray           ();
   DeviceMedia            *getCurrentMedia          ();
   DeviceResolution       *getCurrentResolution     ();
   DeviceCommand          *getCommands              ();
   DeviceData             *getDeviceData            ();
   DevicePrintMode        *getCurrentPrintMode      ();
   DeviceGamma            *getCurrentGamma          ();

   void                    ditherRGBtoCMYK          (PBITMAPINFO2         pbmi2,
                                                     PBYTE                pbStart);
   bool                    ditherAllPlanesBlank     ();
   bool                    ditherCPlaneBlank        ();
   bool                    ditherMPlaneBlank        ();
   bool                    ditherYPlaneBlank        ();
   bool                    ditherKPlaneBlank        ();
   bool                    ditherLMPlaneBlank       ();
   bool                    ditherLCPlaneBlank       ();
   BinaryData             *getCPlane                ();
   BinaryData             *getMPlane                ();
   BinaryData             *getYPlane                ();
   BinaryData             *getKPlane                ();
   BinaryData             *getLMPlane               ();
   BinaryData             *getLCPlane               ();
   void                    ditherNewFrame           ();

   void                    resetBlankLineCount      ();
   void                    incrementBlankLineCount  ();
   void                    incrementBlankLineCount  (int                  iBlankLines);
   int                     getBlankLineCount        ();
   void                    resetCompressionMode     ();
   bool                    isCurrentCompressionMode (int                  iMode);
   BinaryData             *compressKRasterPlane     (BinaryData          *pbdKPlane);
   BinaryData             *compressCRasterPlane     (BinaryData          *pbdCPlane);
   BinaryData             *compressLCRasterPlane    (BinaryData          *pbdLCPlane);
   BinaryData             *compressMRasterPlane     (BinaryData          *pbdMPlane);
   BinaryData             *compressLMRasterPlane    (BinaryData          *pbdLMPlane);
   BinaryData             *compressYRasterPlane     (BinaryData          *pbdYPlane);
   BinaryData             *compressRGBRasterPlane   (BinaryData          *pbdRGBPlane);
   void                    clearLastLineBuffers     ();

   bool                    sendBinaryDataToDevice   (BinaryData          *pData);
   bool                    sendBinaryDataToDevice   (DeviceForm          *pForm);
   bool                    sendBinaryDataToDevice   (DeviceTray          *pTray);
   bool                    sendBinaryDataToDevice   (DeviceMedia         *pMedia);
   bool                    sendBinaryDataToDevice   (DeviceResolution    *pResolution);
   bool                    sendBinaryDataToDevice   (PBYTE                pbData,
                                                     int                  iLength);

   bool                    sendPrintfToDevice       (BinaryData          *pData,
                                                                          ...);
   bool                    sendVPrintfToDevice      (BinaryData          *pData,
                                                     va_list              list);

#ifndef RETAIL
   void                    outputSelf               ();
#endif
   virtual std::string     toString                 (std::ostringstream&  oss);
   friend std::ostream&    operator<<               (std::ostream&        os,
                                                     const DeviceBlitter& self);

protected:
   void                    setDitherInstance        (DeviceDither        *pDitherInstance);
   void                    setCompressionInstance   (GplCompression      *pCompressionInstance);

   PrintDevice       *pDevice_d;

private:
   DeviceDither      *pDitherInstance_d;
   GplCompression    *pCompressionInstance_d;
};

#endif
