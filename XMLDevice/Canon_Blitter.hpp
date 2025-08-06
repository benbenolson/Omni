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
#ifndef _Canon_Blitter
#define _Canon_Blitter

#include "Device.hpp"

extern "C" {
   DeviceBlitter *createBlitter (PrintDevice   *pDevice);
   void           deleteBlitter (DeviceBlitter *pBlitter);
};

class Canon_Blitter : public DeviceBlitter, GplCompressionCallback
{
public:
                         Canon_Blitter                  (PrintDevice         *pDevice);
   virtual              ~Canon_Blitter                  ();

   void                  initializeInstance             ();

   virtual bool          rasterize                      (PBYTE                pbBits,
                                                         PBITMAPINFO2         pbmi,
                                                         PRECTL               prectlPageLocation,
                                                         BITBLT_TYPE          eType);

   // The following two functions are required for the GplCompressionCallback interface
   virtual void          compressionChanged             (int                  iNewCompression);
   virtual void          sendData                       (int                  iLength,
                                                         BinaryData          *pbdData,
                                                         int                  iWhichPlane);

#ifndef RETAIL
   virtual void          outputSelf                     ();
#endif
   virtual std::string   toString                       (std::ostringstream&  oss);
   friend std::ostream&  operator<<                     (std::ostream&        os,
                                                         const Canon_Blitter& device);

private:
   bool                  canonMonoRasterize             (PBYTE                pbBits,
                                                         PBITMAPINFO2         pbmi,
                                                         PRECTL               prectlPageLocation,
                                                         BITBLT_TYPE          eType);
   bool                  canonColorRasterize            (PBYTE                pbBits,
                                                         PBITMAPINFO2         pbmi,
                                                         PRECTL               prectlPageLocation,
                                                         BITBLT_TYPE          eType);
   bool                  setCompression                 (bool                 fCompressed);
   bool                  moveToYPosition                (int                  iWorldY,
                                                         bool                 fAbsolute);

   bool            fGraphicsHaveBeenSent_d;
   bool            fHaveInitialized_d;

   int             iNumDstRowBytes8_d;
};

#endif
