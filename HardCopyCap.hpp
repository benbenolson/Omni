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
#ifndef _HardCopyCap
#define _HardCopyCap

#include "DeviceResolution.hpp"

struct DeviceForm; // Foreward reference to bypass cyclic reference

class HardCopyCap
{
public:
                         HardCopyCap   (int                 iLeft,
                                        int                 iTop,
                                        int                 iRight,
                                        int                 iBottom);

   void                  setOwner      (DeviceForm         *pForm);
   void                  associateWith (DeviceResolution   *pResolution);
   // The following units are in thousands of a millimeter (1/1000 mm)
   // This is for the   full form
   int                   getCx         ();
   int                   getCy         ();
   // This is for each unprintable margin
   int                   getLeftClip   ();
   int                   getTopClip    ();
   int                   getRightClip  ();
   int                   getBottomClip ();
   // The following units are in pels
   // This is for the   printable area
   int                   getXPels      ();
   int                   getYPels      ();

#ifndef RETAIL
   void                  outputSelf    ();
#endif
   virtual std::string   toString      (std::ostringstream& oss);
   friend std::ostream&  operator<<    (std::ostream&       os,
                                        const HardCopyCap&  self);

private:
   DeviceForm *pForm_d;
   int         iLeft_d;
   int         iTop_d;
   int         iRight_d;
   int         iBottom_d;
   int         iXPels_d;
   int         iYPels_d;
};

#endif
