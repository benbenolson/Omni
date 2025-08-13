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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glob.h>

// gcc -o ModifyXMLFiles -I .. -g ModifyXMLFiles.cpp -lstdc++ && ./ModifyXMLFiles /home/Omni/schema/HPLaserJet

#define dimof(a) (sizeof (a)/sizeof (a[0]))

std::string *
trim (const char *pszLine)
{
   int indexStart = 0;
   int indexEnd   = strlen (pszLine);

   if (0 < indexEnd)
      indexEnd--;

   while (  indexStart < indexEnd
         && (  ' '  == pszLine[indexStart]
            || '\t' == pszLine[indexStart]
            || '\r' == pszLine[indexStart]
            || '\n' == pszLine[indexStart]
            )
         )
   {
      indexStart++;
   }

   while (  indexEnd > indexStart
         && (  ' '  == pszLine[indexEnd]
            || '\t' == pszLine[indexEnd]
            || '\r' == pszLine[indexEnd]
            || '\n' == pszLine[indexEnd]
            )
         )
   {
      indexEnd--;
   }

   return new std::string (pszLine, indexStart, indexEnd - indexStart + 1);
}

bool
processFile (const char *pszFileName)
{
   const static bool fDebug = false;

   char                      achLine[512];             // @TBD
   std::ifstream             ifstreamIn (pszFileName);
   std::vector<std::string>  vectorFile;
   bool                      fFileModified            = false;

   if (!ifstreamIn.good ())
   {
      return false;
   }

   while (ifstreamIn.getline (achLine, sizeof (achLine)))
   {
      std::string            stringLine = achLine;
      std::string::size_type index      = 0;

if (fDebug) std::cout << "Read: " << stringLine << std::endl;

      if (std::string::npos != (index = stringLine.find ("DeviceOptions2")))
      {
         stringLine.erase (index + 13, 1);

         index = stringLine.find ("DO1_");

         if (std::string::npos != index)
         {
            stringLine.erase (index + 2, 1);
         }

         index = stringLine.find ("DO2_");

         if (std::string::npos != index)
         {
            stringLine.erase (index + 2, 1);
         }

         fFileModified = true;
      }
      else if (std::string::npos != (index = stringLine.find ("DeviceOptions")))
      {
         index = stringLine.find ("DO1_");

         if (std::string::npos != index)
         {
            stringLine.erase (index + 2, 1);
         }

         index = stringLine.find ("DO2_");

         if (std::string::npos != index)
         {
            stringLine.erase (index + 2, 1);
         }

         fFileModified = true;
      }

      vectorFile.push_back (stringLine);
   }

   ifstreamIn.close ();

   if (fFileModified)
   {
      std::ofstream ofstreamOut (pszFileName);

      for ( std::vector<std::string>::iterator iter = vectorFile.begin ();
            iter != vectorFile.end ();
            iter++)
      {
         ofstreamOut << *iter << std::endl;
         std::cout << *iter << std::endl;
      }

      ofstreamOut.close ();
   }

   return true;
}

void
copyFile (const char *pszFrom,
          const char *pszTo)
{
   std::ifstream ifFileFrom (pszFrom, std::ios::in  | std::ios::binary);
   std::ofstream ofFileTo   (pszTo,   std::ios::out | std::ios::binary);

   // copy the file in one line o' code!
   ofFileTo << ifFileFrom.rdbuf ();

   ofFileTo.close ();
}

int
main (int argc, char *argv[])
{
   std::ostringstream oss;
   glob_t             globbuf;
   int                rc;

   if (2 > argc)
   {
      return 1;
   }

   // Call glob
   memset (&globbuf, 0, sizeof (globbuf));

   oss << argv[1]
       << "/*.xml"
       << std::ends;

   rc = glob (oss.str ().c_str (), 0, NULL, &globbuf);

   if (0 == rc)
   {
      if (0 == globbuf.gl_pathc)
      {
         std::cerr << "Error: glob (" << oss.str () << ") returned 0 files!" << std::endl;
      }

      // Call succeded
      for (int i = 0; i < (int)globbuf.gl_pathc; i++)
      {
         std::cout << globbuf.gl_pathv[i] << std::endl;

#if 0
         if (3 == argc)
         {
            std::string stringFullFileName = globbuf.gl_pathv[i];
            std::string stringFileName     = stringFullFileName.substr (stringFullFileName.find_last_of ("/") + 1);
            std::string stringFromFileName = argv[2];

            stringFromFileName += "/";
            stringFromFileName += stringFileName;

            copyFile (stringFromFileName.c_str (), globbuf.gl_pathv[i]);
         }
#endif

         processFile (globbuf.gl_pathv[i]);
      }
   }
   else
   {
      std::cerr << "Error: glob (" << oss.str () << ") failed. rc = " << rc << std::endl;
   }

   globfree (&globbuf);

   return 0;
}
