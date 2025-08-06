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
#ifndef _DeviceGamma_hpp
#define _DeviceGamma_hpp

#include <iostream>
#include <sstream>
#include <string>

class DeviceGamma
{
public:
                        DeviceGamma (int iCGamma,
                                     int iMGamma,
                                     int iYGamma,
                                     int iKGamma,
                                     int iCBias,
                                     int iMBias,
                                     int iYBias,
                                     int iKBias);

   int                  getCGamma   ();
   int                  getMGamma   ();
   int                  getYGamma   ();
   int                  getKGamma   ();
   int                  getCBias    ();
   int                  getMBias    ();
   int                  getYBias    ();
   int                  getKBias    ();

#ifndef RETAIL
   void                 outputSelf  ();
#endif
   virtual std::string  toString    (std::ostringstream& oss);
   friend std::ostream& operator<<  (std::ostream&       os,
                                     const DeviceGamma&  self);

private:
   int iCGamma_d;
   int iMGamma_d;
   int iYGamma_d;
   int iKGamma_d;
   int iCBias_d;
   int iMBias_d;
   int iYBias_d;
   int iKBias_d;
};

#endif
