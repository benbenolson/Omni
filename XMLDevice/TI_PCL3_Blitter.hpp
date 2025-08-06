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
#ifndef _TI_PCL3_Blitter
#define _TI_PCL3_Blitter

#include "Device.hpp"

extern "C" {
   DeviceBlitter *createBlitter (PrintDevice   *pDevice);
   void           deleteBlitter (DeviceBlitter *pBlitter);
};

class TI_PCL3_Blitter : public DeviceBlitter, GplCompressionCallback
{
public:
                        TI_PCL3_Blitter   (PrintDevice                   *pDevice);
   virtual             ~TI_PCL3_Blitter   ();

   virtual void         initializeInstance        ();

   virtual bool         rasterize                 (PBYTE                          pbBits,
                                                   PBITMAPINFO2                   pbmi,
                                                   PRECTL                         prectlPageLocation,
                                                   BITBLT_TYPE                    eType);

   // The following two functions are required for the GplCompressionCallback interface
   virtual void         compressionChanged        (int                            iNewCompression);
   virtual void         sendData                  (int                            iLength,
                                                   BinaryData                    *pbdData,
                                                   int                            iWhichPlane);

#ifndef RETAIL
   virtual void         outputSelf                ();
#endif
   virtual std::string  toString                  (std::ostringstream&            oss);
   friend std::ostream& operator<<                (std::ostream&                  os,
                                                   const TI_PCL3_Blitter& device);

private:
   bool                 deskjetMonoRasterize      (PBYTE                          pbBits,
                                                   PBITMAPINFO2                   pbmi,
                                                   PRECTL                         prectlPageLocation,
                                                   BITBLT_TYPE                    eType);
   bool                 deskjetColorRasterize     (PBYTE                          pbBits,
                                                   PBITMAPINFO2                   pbmi,
                                                   PRECTL                         prectlPageLocation,
                                                   BITBLT_TYPE                    eType);
   bool                 moveToYPosition           (int                            iWorldY,
                                                   bool                           fAbsolute);

   bool               fInstanceInitialized_d;
   bool               fGraphicsHaveBeenSent_d;
};

#endif
