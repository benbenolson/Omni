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
#include "HardCopyCap.hpp"
#include "DeviceForm.hpp"

HardCopyCap::
HardCopyCap (int iLeft,
             int iTop,
             int iRight,
             int iBottom)
{
   pForm_d   = 0;
   iLeft_d   = iLeft;
   iTop_d    = iTop;
   iRight_d  = iRight;
   iBottom_d = iBottom;
   iXPels_d  = 0;
   iYPels_d  = 0;
}

void HardCopyCap::
setOwner (DeviceForm *pForm)
{
   pForm_d = pForm;
}

void HardCopyCap::
associateWith (DeviceResolution *pResolution)
{
   float flSizeInInches = 0;
   float flSizeInHmm    = 0;

   flSizeInHmm = getCx ();
   flSizeInHmm -= (iLeft_d + iRight_d);
   flSizeInInches = flSizeInHmm / 25400.0; // (inches per thousandths of a mm)

   iXPels_d = (int)(flSizeInInches * pResolution->getXRes () + 0.5);

   flSizeInHmm = getCy ();
   if (!pForm_d->hasCapability (DeviceForm::FORM_CAPABILITY_ROLL))
   {
      flSizeInHmm -= (iTop_d + iBottom_d);
   }
   flSizeInInches = flSizeInHmm / 25400.0; // (inches per thousandths of a mm)

   iYPels_d = (int)(flSizeInInches * pResolution->getYRes () + 0.5);
}

int HardCopyCap:: getCx         () { return pForm_d ? pForm_d->getCx () : 0; }
int HardCopyCap:: getCy         () { return pForm_d ? pForm_d->getCy () : 0; }
int HardCopyCap:: getLeftClip   () { return iLeft_d; }
int HardCopyCap:: getTopClip    () { return iTop_d; }
int HardCopyCap:: getRightClip  () { return iRight_d; }
int HardCopyCap:: getBottomClip () { return iBottom_d; }
int HardCopyCap:: getXPels      () { return iXPels_d; }
int HardCopyCap:: getYPels      () { return iYPels_d; }

#ifndef RETAIL

void HardCopyCap::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string HardCopyCap::
toString (std::ostringstream& oss)
{
   oss << "{HardCopyCap: pForm_d = " << pForm_d
       << ", iLeft_d = " << iLeft_d
       << ", iTop_d = " << iTop_d
       << ", iRight_d = " << iRight_d
       << ", iBottom_d = " << iBottom_d
       << ", iXPels_d = " << iXPels_d
       << ", iYPels_d = " << iYPels_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const HardCopyCap& const_self)
{
   HardCopyCap&       self = const_cast<HardCopyCap&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
