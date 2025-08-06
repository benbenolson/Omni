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
#ifndef _com_ibm_ltc_omni_guitool_OmniDCTUtils_hpp
#define _com_ibm_ltc_omni_guitool_OmniDCTUtils_hpp

/*
* This file contains utility functions
*/

#include <string>
#include <vector>

namespace OmniDeviceCreationTool {

class OmniDCTUtils
{
   public:

      /* the leftTrim() function removes the
      * leading blank spaces in a string
      */
      static string leftTrim(string input);


      /*
      *   str2vec() converts the string containing comma
      *  seperated values to a vector of strings
      */
      static vector<string> str2vec(string);

/*
* The method i2str() takes an integer as input
* and returns a string that reprents the integer
* example- int 101 will be returned as string "101"
*/
      static string i2str(int iInput);

}; // end class definition

}; // end of namespace

#endif
