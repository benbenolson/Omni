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

// This is the omni2foo implementation.  Take the filename as arg, open it
// Need to add in some error checking, bad file etc...
// Check the input from the file to make sure it agrees with what we are
// looking for.  Right now, its a use-at-your-own-risk. Will work fine with
// tested DB file.
#include "omni2foo.hpp"

#include <iostream>

// Default constructor
omni2foo::
omni2foo ()
{
}

omni2foo::
omni2foo (std::string fileName)
{
   std::ifstream file;
   std::string   fooID;
   std::string   printerName;
   int           iLineNumber = 0;

   file.open (fileName.c_str ());

   while (!file.eof ())
   {
      file >> printerName;
      file >> fooID;
      iLineNumber++;

      // Warning: putting a new line character at the end of the file will
      //          cause a file fail condition.
      if (  file.bad ()
         || file.fail ()
         )
      {
         std::cerr << "Error: Reading Foomatic DB file : \"" << fileName << "\" ";
         if (file.bad ())
            std::cerr << "(bad)";
         if (file.fail ())
            std::cerr << "(fail)";
         std::cerr << " line number " << iLineNumber << std::endl;

         return;
      }

      if (  printerName.length () != 0
         && fooID.length () != 0
         )
         _mPrinterMap[printerName] = fooID;
      else
         std::cerr << "Error: Bad entry in file " << fileName << ", Field1: " << printerName << " Field2: " << fooID << std::endl;
   }
}

omni2foo::
~omni2foo()
{
   // nothing to destroy, its all on the stack
}

std::string omni2foo::
getFooIDFromString (std::string printerName)
{
   // lowercase all characters
   transform (printerName.begin (), printerName.end (),   // source
              printerName.begin (),                       // destination
              ::tolower);                                 // operation

   return _mPrinterMap[printerName];
}

std::string omni2foo::
getPrinterNameFromFooID (std::string fooID)
{
   std::string printerName ("");

   return printerName;
}
