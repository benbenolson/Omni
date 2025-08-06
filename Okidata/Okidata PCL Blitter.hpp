/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2001
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
#ifndef _Okidata_PCL_Blitter
#define _Okidata_PCL_Blitter

#include "Device.hpp"

extern "C" {
   DeviceBlitter *createBlitter (PrintDevice   *pDevice);
   void           deleteBlitter (DeviceBlitter *pBlitter);
};

class Okidata_PCL_Blitter : public DeviceBlitter, GplCompressionCallback
{
public:
                        Okidata_PCL_Blitter   (PrintDevice                   *pDevice);
   virtual             ~Okidata_PCL_Blitter   ();

   void                 initializeInstance        ();

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
                                                   const Okidata_PCL_Blitter& device);

private:
   bool            hp_laserjetMonoRasterize       (PBYTE                          pbBits,
                                                   PBITMAPINFO2                   pbmi,
                                                   PRECTL                         prectlPageLocation,
                                                   BITBLT_TYPE                    eType);
   bool            hp_laserjetColorRasterize      (PBYTE                          pbBits,
                                                   PBITMAPINFO2                   pbmi,
                                                   PRECTL                         prectlPageLocation,
                                                   BITBLT_TYPE                    eType);
   bool            moveToYPosition                (int                            iWorldY,
                                                   bool                           fAbsolute);

   bool               fHaveInitialized_d;
   bool               fInstanceInitialized_d;
   bool               fGraphicsHaveBeenSent_d;
   int                iCurrentWidth_d;
};

#endif
