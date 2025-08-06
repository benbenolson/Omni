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


#include "OmniDCTUtils.hpp"
#include <string>
#include <vector>

using namespace OmniDeviceCreationTool;

string OmniDCTUtils::leftTrim(string input)

      {
         if (input.empty())
            return input;

         bool fCharFound = false;
         string temp = input;
         string::iterator iter;
         while(!fCharFound && !temp.empty() )
         {
            iter = temp.begin();
            if ((*iter) == ' ' )
            {
               temp.erase(iter);
            }
            else
            {
               fCharFound = true;
            }
         }
         return temp;
      } // end of leftTrim()



/*
      str2vec() converts the string containing comma
      seperated values to a vector of strings
*/
vector<string> OmniDCTUtils::
str2vec(string input)
{
   vector<string> output;

   if (input.empty())
   {
      return output;    // a vector with zero elements
   }

   int index=0;
   string temp;
   while ((!input.empty()) && index != -1)
   {
      index = input.find_first_of(',');
      temp = input.substr(0,index);
      output.push_back(temp);
      input = input.substr(index+1);
   }

   return output;

}


/*
* The method i2str() takes an integer as input
* and returns a string that reprents the integer
* example- int 101 will be returned as string "101"
*/

string OmniDCTUtils::
i2str(int iInput)
{
   int iRemainder;
   char cCharVal;
   string val;
   string num;
   while ( iInput > 9 )
   {
      iRemainder = iInput%10;
      iInput = iInput/10;
      iRemainder = iRemainder + 48;
      cCharVal = iRemainder;
      val = val + cCharVal;
      num = val + num;
      val.erase();
   }

   iRemainder = iInput;
   iRemainder = iRemainder + 48;   // 48 is ascii val of zero
   cCharVal = iRemainder;
   val = val + cCharVal;
   num = val + num;
   return num;
}
