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

// gcc -o ConvertFromDTDtoSchema -I .. -g ConvertFromDTDtoSchema.cpp -lstdc++ && ./ConvertFromDTDtoSchema /home/Omni/schema/XMLParser/ /home/Omni/autotools/XMLParser/ && make test
// gcc -o ConvertFromDTDtoSchema -I .. -g ConvertFromDTDtoSchema.cpp -lstdc++ && ./ConvertFromDTDtoSchema /home/Omni/schema/Epson /home/Omni/autotools/Epson
// (DEVICES=HPDeskJet; make DEVICES=$DEVICES SUBDIRS=XMLParser; cd ../$DEVICES/; make)

#define dimof(a) (sizeof (a)/sizeof (a[0]))

//#define USE_OMNI_NAMESPACE 1
//#define USE_NO_NAMESPACE   1
//#define USE_WINDOWS        1

void
syntaxIndent (const char *pszLine, int *piLevel)
{
   const static bool fDebug = false;

   std::string            stringLine (pszLine);
   std::string::size_type index                = 0;
   std::string::size_type indexLT              = 0;
   std::string::size_type indexGT              = 0;

   while (std::string::npos != index)
   {
      indexLT = stringLine.find ("<", index);
      indexGT = std::string::npos;
      if (std::string::npos != indexLT)
      {
         indexGT = stringLine.find (">", indexLT);
      }
      if (std::string::npos != indexGT)
      {
         std::string            stringElement;
         std::string::size_type indexSlash;

         stringElement = stringLine.substr (indexLT, indexGT - indexLT + 1);
         indexSlash    = stringElement.find ("/");

if (fDebug) std::cout << "stringElement = " << stringElement << std::endl;
         if (std::string::npos != indexSlash)
         {
            if (1 == indexSlash)
            {
               (*piLevel)--;
if (fDebug) std::cout << "--" << std::endl;
            }
            else
            {
if (fDebug) std::cout << "0" << std::endl;
            }
         }
         else
         {
            (*piLevel)++;
if (fDebug) std::cout << "++" << std::endl;
         }

         index = indexGT + 1;
if (fDebug) std::cout << "indexSlash    = " << indexSlash << std::endl;
      }
      else
      {
         index = std::string::npos;
      }
if (fDebug) std::cout << "indexLT       = " << indexLT << std::endl;
if (fDebug) std::cout << "indexGT       = " << indexGT << std::endl;
if (fDebug) std::cout << "index         = " << index << std::endl;
   }
}

void
bufferHeader (std::vector<std::string>& vectorFile)
{
   vectorFile.push_back ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
   vectorFile.push_back ("<!--");
   vectorFile.push_back ("     IBM Omni driver");
   vectorFile.push_back ("     Copyright (c) International Business Machines Corp., 2000");
   vectorFile.push_back ("");
   vectorFile.push_back ("     This library is free software; you can redistribute it and/or modify");
   vectorFile.push_back ("     it under the terms of the GNU Lesser General Public License as published");
   vectorFile.push_back ("     by the Free Software Foundation; either version 2.1 of the License, or");
   vectorFile.push_back ("     (at your option) any later version.");
   vectorFile.push_back ("");
   vectorFile.push_back ("     This library is distributed in the hope that it will be useful,");
   vectorFile.push_back ("     but WITHOUT ANY WARRANTY; without even the implied warranty of");
   vectorFile.push_back ("     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See");
   vectorFile.push_back ("     the GNU Lesser General Public License for more details.");
   vectorFile.push_back ("");
   vectorFile.push_back ("     You should have received a copy of the GNU Lesser General Public License");
   vectorFile.push_back ("     along with this library; if not, write to the Free Software");
   vectorFile.push_back ("     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA");
   vectorFile.push_back ("-->");
   vectorFile.push_back ("");
}

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

std::string *
convertToOmniNamespace (std::string *pstringIn)
{
#ifdef USE_OMNI_NAMESPACE

   std::ostringstream oss;
   char               ch1,
                      ch2;

   for (int i = 0; i < pstringIn->length (); i++)
   {
      ch1 = (*pstringIn)[i];
      ch2 = (*pstringIn)[i + 1];

      if (  '<' == ch1
         && '/' != ch2
         )
      {
         oss << "<omni:";
      }
      else if (  '<' == ch1
              && '/' == ch2
              )
      {
         oss << "</omni:";
         i++;
      }
      else
      {
         oss << ch1;
      }
   }

   delete pstringIn;

   return new std::string (oss.str ());

#else

   std::string *pstringOut = new std::string (*pstringIn);

   delete pstringIn;

   return pstringOut;

#endif
}

void
bufferLine (std::vector<std::string>&  vectorFile,
            std::string               *pstringLine,
            int                        iLevel)
{
   std::ostringstream oss;

   for (int i = 0; i < iLevel; i++)
   {
      oss << "   ";
   }
   oss << *pstringLine;

   vectorFile.push_back (oss.str ());
}

bool
processFile (const char *pszFileName)
{
   const static bool fDebug = false;

   typedef enum _State {
      STATE_PRELUDE,
      STATE_BLANKLINES,
      STATE_DATA_START,
      STATE_DATA
   } STATE;
   char                      achLine[512];             // @TBD
   std::ifstream             ifstreamIn (pszFileName);
   STATE                     eState                   = STATE_PRELUDE;
   int                       iLevel                   = 0;
   std::string              *pstringLine              = 0;
   std::vector<std::string>  vectorFile;

   if (!ifstreamIn.good ())
   {
      return false;
   }

   while (0 < ifstreamIn.getline (achLine, sizeof (achLine)))
   {
if (fDebug) std::cout << "Read[" << eState << "]: " << achLine << std::endl;
      if (STATE_PRELUDE == eState)
      {
         if (0 != strstr (achLine, "-->"))
         {
            eState = STATE_BLANKLINES;
            continue;
         }
      }
      if (STATE_BLANKLINES == eState)
      {
         char *pszLine = achLine;

         while (*pszLine)
         {
if (fDebug) std::cout << (int)*pszLine;
            if (  '\t' == *pszLine
               || '\n' == *pszLine
               || '\r' == *pszLine
               )
            {
               pszLine++;
            }
            else
            {
               break;
            }
         }
if (fDebug) std::cout << " -- " << (int)*pszLine << std::endl;

         if (*pszLine)
         {
            eState = STATE_DATA_START;
         }
      }
      if (STATE_DATA_START == eState)
      {
         bufferHeader (vectorFile);

         pstringLine = convertToOmniNamespace (trim (achLine));

         std::string stringInsert[] = {
            pstringLine->substr (0, pstringLine->length () - 1),
#ifdef USE_OMNI_NAMESPACE
            "   xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"",
            "   xmlns:omni=\"http://www.ibm.com/linux/ltc/projects/omni/\"",
            "   xs:schemaLocation=\"http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd\">"
#else
#ifdef USE_NO_NAMESPACE
            "   xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"",
            "   xs:noNamespaceSchemaLocation=\"../OmniDevice.xsd\">"
#else
#ifdef USE_WINDOWS
#if 1 // @TBD
            "   xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"",
            "   xs:noNamespaceSchemaLocation=\"..\\OmniDevice.xsd\">"
#else
            "   xmlns=\"http://www.ibm.com/linux/ltc/projects/omni/\"",
            "   xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"",
            "   xs:schemaLocation=\"http://www.ibm.com/linux/ltc/projects/omni/ ..\\OmniDevice.xsd\">"
#endif
#else
            "   xmlns=\"http://www.ibm.com/linux/ltc/projects/omni/\"",
            "   xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"",
            "   xs:schemaLocation=\"http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd\">"
#endif
#endif
#endif
         };

         for (int i = 0; i < (int)dimof (stringInsert); i++)
         {
if (fDebug) std::cout << "Adding: " << stringInsert[i] << std::endl;
            bufferLine (vectorFile,
                        &stringInsert[i],
                        iLevel);
         }

         syntaxIndent (achLine,
                       &iLevel);

         delete pstringLine;

         eState = STATE_DATA;
         continue;
      }
      if (STATE_DATA == eState)
      {
         int iNewLevel = iLevel;

         pstringLine = convertToOmniNamespace (trim (achLine));

         syntaxIndent (achLine,
                       &iNewLevel);
if (fDebug) std::cout << "Adding: " << *pstringLine << std::endl;
         bufferLine (vectorFile,
                     pstringLine,
                     (iNewLevel < iLevel) ? iNewLevel : iLevel);

         delete pstringLine;

         iLevel = iNewLevel;
         eState = STATE_DATA;
         continue;
      }
   }

   ifstreamIn.close ();

   std::ofstream ofstreamOut (pszFileName);

   for ( std::vector<std::string>::iterator iter = vectorFile.begin ();
         iter != vectorFile.end ();
         iter++)
   {
      ofstreamOut << *iter << std::endl;
      std::cout << *iter << std::endl;
   }

   ofstreamOut.close ();

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
#if 0

   const char *pszFrom = "/home/Omni/autotools/HP DeskJet/HP DeskJet 1100C Commands.xml";
   const char *pszTo   = "/home/Omni/schema/HP DeskJet/HP DeskJet 1100C Commands.xml";

   copyFile (pszFrom, pszTo);

   processFile (pszTo);

#else

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
      // Call succeded
      for (int i = 0; i < (int)globbuf.gl_pathc; i++)
      {
         std::cout << globbuf.gl_pathv[i] << std::endl;

         if (3 == argc)
         {
            std::string stringFullFileName = globbuf.gl_pathv[i];
            std::string stringFileName     = stringFullFileName.substr (stringFullFileName.find_last_of ("/") + 1);
            std::string stringFromFileName = argv[2];

            stringFromFileName += "/";
            stringFromFileName += stringFileName;

            copyFile (stringFromFileName.c_str (), globbuf.gl_pathv[i]);
         }

         processFile (globbuf.gl_pathv[i]);
      }
   }

   globfree (&globbuf);

#endif

   return 0;
}
