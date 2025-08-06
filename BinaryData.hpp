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
#ifndef _BinaryData
#define _BinaryData

#include "defines.hpp"

#include <iostream>
#include <sstream>
#include <string>

class BinaryData
{
public:
                        BinaryData            (PBYTE                   pbData,
                                               int                     iSize);
   virtual             ~BinaryData            ();

   PBYTE                getData               ();
   int                  getLength             ();
   int                  getPrintfLength       ();

   void                 setData               (PBYTE                   pbNewData);
   void                 setLength             (int                     iNewLength);

   void                 setDumpString         ();
   void                 setDumpHex            ();

#ifndef RETAIL
   void                 outputSelf            ();
#endif
   virtual std::string  toString              (std::ostringstream&     oss);
   friend std::ostream& operator<<            (std::ostream&           os,
                                               const BinaryData&       self);

private:
   PBYTE pbData_d;
   int   iSize_d;
   bool  fOutType_d;
};

class BinaryDataDelete : public BinaryData
{
public:
                        BinaryDataDelete      (PBYTE                   pbData,
                                               int                     iSize);
   virtual             ~BinaryDataDelete      ();

#ifndef RETAIL
   void                 outputSelf            ();
#endif
   virtual std::string  toString              (std::ostringstream&     oss);
   friend std::ostream& operator<<            (std::ostream&           os,
                                               const BinaryDataDelete& self);

private:
   PBYTE pbData_d;
};

#endif
