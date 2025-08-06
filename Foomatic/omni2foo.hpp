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

// This file is a simple map made from a file of space separated strings.
// The file has a printername and a foomatic printer id
//
// Example,
//
// epson_stylus_color_740 62112
//
// This object encapsulates the reading and creating of a simple
// associative array that maps printernames to the fooid they need
// the getFooIDFromString will take a string and lower case the
// argument and get the value using the new string as the key.
// the resulting value is returned.

#ifndef _omni2foo_
#define _omni2foo_

// Includes needed for this object
#include <sstream>         // For file input/output
#include <fstream>
#include <string>          // for the strings from the file
#include <map>             // used for an associative array, string,string
#include <algorithm>       // used for the to lower transform
#include <cctype>

class omni2foo
{
public:
                           omni2foo                ();
                           omni2foo                (std::string fileName);
      virtual             ~omni2foo                ();
      virtual std::string  getFooIDFromString      (std::string printername);
      virtual std::string  getPrinterNameFromFooID (std::string fooid);

private:
   typedef std::map<std::string,std::string> printerMap;

   // Should turn off copy constructor and assignment operator
   printerMap _mPrinterMap;
};
#endif
