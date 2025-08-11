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
#include <sstream>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "OmniDomParser.hpp"

#ifdef DEBUG
const static bool fDebugOutput = false;
#endif

void convertFilename (char *pszFileName);

extern char *vpszExeName;

OmniDomParser::
OmniDomParser (const char *pszXMLFile,
               DeviceInfo *pdi,
               bool        fAutoconf,
               bool        fAutoconfNoInst)
{
   doc_d             = 0;
   pstrXMLFile_d     = new std::string (pszXMLFile);
   pdi_d             = pdi;
   fErrorCondition_d = false;
   fAutoconf_d       = fAutoconf;
   fAutoconfNoInst_d = fAutoconfNoInst;
   fFileModified_d   = false;
}

OmniDomParser::
~OmniDomParser ()
{
   delete pstrXMLFile_d;

   if (doc_d)
   {
      XMLFreeDoc (doc_d);
   }

   doc_d         = 0;
   pstrXMLFile_d = 0;
   pdi_d         = 0;
}

void OmniDomParser::
setErrorCondition ()
{
   fErrorCondition_d = true;
}

bool OmniDomParser::
fileModified ()
{
   return fFileModified_d;
}

void
convertFilename (char *pszFileName)
{
   while (*pszFileName)
   {
      switch (*pszFileName)
      {
      case '-':
      case '+':
      case '(':
      case ')':
      case ' ':
      case '/':
      case '\\':
         *pszFileName = '_';
      }

      pszFileName++;
   }
}

void
convertCId (char *pszCId)
{
   while (*pszCId)
   {
      switch (*pszCId)
      {
      case '-':
      case '+':
      case '(':
      case ')':
      case ' ':
      case '/':
      case '\\':
         *pszCId = '_';
      }

      pszCId++;
   }
}

std::string * OmniDomParser::
getNameOut ()
{
   int          iDotPos    = pstrXMLFile_d->find_last_of ('.');
   std::string *pstringRet = new std::string (pstrXMLFile_d->substr (0, iDotPos));

   convertFilename (const_cast<char*>(pstringRet->c_str ())); // @HACK?

   return pstringRet;
}

std::string *
converToClassName (const char *pszFileName)
{
   std::string  stringFileName (pszFileName);

   int          iDotPos    = stringFileName.find_last_of ('.');
   std::string *pstringRet = new std::string (stringFileName.substr (0, iDotPos));

   convertFilename (const_cast<char*>(pstringRet->c_str ())); // @HACK?

   return pstringRet;
}

bool OmniDomParser::
shouldCreateFile (std::string strFileName)
{
   if (!vpszExeName)
   {
      return true;
   }

   std::string stringFileNameHPP (strFileName.c_str ());
   std::string stringFileNameCPP (strFileName.c_str ());

   convertFilename (const_cast<char*>(stringFileNameHPP.c_str ())); // @HACK?
   convertFilename (const_cast<char*>(stringFileNameCPP.c_str ())); // @HACK?

   int iDotPos = stringFileNameHPP.find_last_of ('.');

   stringFileNameHPP.replace (iDotPos, 4, ".hpp");

   iDotPos = stringFileNameCPP.find_last_of ('.');

   stringFileNameCPP.replace (iDotPos, 4, ".cpp");

   struct stat statExe;
   struct stat statIn;
   struct stat statOutHPP;
   struct stat statOutCPP;

   if (-1 == stat (vpszExeName, &statExe))
   {
      std::cerr << "Error: File not found \"" << vpszExeName << "\" (could not stat it)!" << std::endl;
      std::cerr.flush ();
      setErrorCondition ();
      abort ();
   }

   if (-1 == stat (strFileName.c_str (), &statIn))
   {
      std::cerr << "Error: File not found \"" << strFileName << "\" (could not stat it)!" << std::endl;
      std::cerr.flush ();
      setErrorCondition ();
      abort ();
   }

#ifdef DEBUG
   if (fDebugOutput)
   {
      std::cerr << "stats for \"" << strFileName << "\"" << std::endl;
      std::cerr << "   atime = " << statIn.st_atime << std::endl;
      std::cerr << "   mtime = " << statIn.st_mtime << std::endl;
      std::cerr << "   ctime = " << statIn.st_ctime << std::endl;
   }
#endif

   if (-1 == stat (stringFileNameHPP.c_str (), &statOutHPP))
      return true;

#ifdef DEBUG
   if (fDebugOutput)
   {
      std::cerr << "stats for \"" << stringFileNameHPP << "\"" << std::endl;
      std::cerr << "   atime = " << statOutHPP.st_atime << std::endl;
      std::cerr << "   mtime = " << statOutHPP.st_mtime << std::endl;
      std::cerr << "   ctime = " << statOutHPP.st_ctime << std::endl;
   }
#endif

   if (-1 == stat (stringFileNameCPP.c_str (), &statOutCPP))
      return true;

#ifdef DEBUG
   if (fDebugOutput)
   {
      std::cerr << "stats for \"" << stringFileNameCPP << "\"" << std::endl;
      std::cerr << "   atime = " << statOutCPP.st_atime << std::endl;
      std::cerr << "   mtime = " << statOutCPP.st_mtime << std::endl;
      std::cerr << "   ctime = " << statOutCPP.st_ctime << std::endl;
   }
#endif

   if (  statExe.st_ctime > statOutHPP.st_ctime
      || statExe.st_ctime > statOutCPP.st_ctime
      || statIn.st_ctime > statOutHPP.st_ctime
      || statIn.st_ctime > statOutCPP.st_ctime
      || 0 == statOutHPP.st_size
      || 0 == statOutCPP.st_size
      )
      return true;
   else
      return false;
}

void OmniDomParser::
copyFile (const char *pszFileNameIn)
{
   std::string stringFileNameOut (pszFileNameIn);

   convertFilename (const_cast<char*>(stringFileNameOut.c_str ())); // @HACK?

   struct stat statIn;
   struct stat statOut;
   bool        fShouldCopy = false;

   if (-1 == stat (pszFileNameIn, &statIn))
   {
      std::cerr << "Error: File not found \"" << pszFileNameIn << "\" (could not stat it)!" << std::endl;
      setErrorCondition ();

      return;
   }

   if (-1 == stat (stringFileNameOut.c_str (), &statOut))
      fShouldCopy = true;

   if (  statIn.st_ctime > statOut.st_ctime
       || 0 == statOut.st_size
      )
      fShouldCopy = true;

   if (fShouldCopy)
   {
      std::cerr << "copying \"" << pszFileNameIn << "\" to \"" << stringFileNameOut << "\"" << std::endl;

      std::ifstream  ifIn  (pszFileNameIn, std::ios::in | std::ios::binary);
      std::ofstream  ofOut (stringFileNameOut.c_str (), std::ios::out | std::ios::binary);

      // copy the file in one line o' code!
      ofOut << ifIn.rdbuf ();
   }

   addToLibraries3 (stringFileNameOut);
}

void OmniDomParser::
addToLibraries3 (const char *pszFileName)
{
   std::string stringFileName = pszFileName;

   addToLibraries3 (stringFileName);
}

void OmniDomParser::
addToLibraries3 (std::string& stringFileName)
{
   if (!fAutoconf_d)
      return;

   if (!(*pdi_d->mapSeenFiles)[stringFileName])
   {
#ifdef DEBUG
      if (fDebugOutput) std::cerr << "adding \"" << stringFileName << "\" to libraries3.mak" << std::endl;
#endif

      (*pdi_d->mapSeenFiles)[stringFileName] = 1;

      std::ofstream ofstreamAM3 ("libraries3.mak", std::ios::app);

      ofstreamAM3 << "CLEANFILES += " << stringFileName << std::endl;

      ofstreamAM3.close ();
   }
}

std::string *
getXMLJobProperties (XmlNodePtr  root,
                     XmlDocPtr   doc,
                     char       *pszXMLNodeName)
{
   if (pszXMLNodeName)
   {
      root = XMLFindEntry (root, pszXMLNodeName, false);
   }

   if (XMLFirstNode (XMLGetChildrenNode (root)))
   {
      std::ostringstream oss;
      bool               fFirst = true;

      root = XMLFirstNode (XMLGetChildrenNode (root));

      while (root != 0)
      {
         if (fFirst)
         {
            fFirst = false;
         }
         else
         {
            oss << ' ';
         }

         PSZCRO pszFormat = XMLGetProp (root, "FORMAT");

         if (pszFormat)
         {
            if (0 == strcmp (pszFormat, "XbyY"))
            {
               XmlNodePtr params      = XMLFirstNode (XMLGetChildrenNode (root));
               bool       fFirstParam = true;

               if (params != 0)
               {
                  oss << XMLGetName (root) << "=";
               }

               while (params != 0)
               {
                  PSZCRO pszContent = XMLNodeListGetString (doc,
                                                            XMLGetChildrenNode (params),
                                                            1);

                  if (pszContent)
                  {
                     if (fFirstParam)
                     {
                        fFirstParam = false;
                     }
                     else
                     {
                        oss << 'X';
                     }

                     oss << pszContent;

                     free ((void *)pszContent);
                  }

                  params = XMLNextNode (params);
               }
            }

            free ((void *)pszFormat);
         }
         else
         {
            PSZCRO pszContent = XMLNodeListGetString (doc,
                                                      XMLGetChildrenNode (root),
                                                      1);

            if (pszContent)
            {
               oss << XMLGetName (root) << "=" << pszContent;

               free ((void *)pszContent);
            }
         }

         root = XMLNextNode (root);
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "getXMLJobProperties returns \"" << oss.str () << "\"" << std::endl;
#endif

      return new std::string (oss.str ());
   }
   else
   {
      PSZCRO pszContent = XMLNodeListGetString (doc,
                                                XMLGetChildrenNode (root),
                                                1);

      if (pszContent)
      {
         std::ostringstream oss;

         oss << XMLGetName (root) << "=" << pszContent;

         free ((void *)pszContent);

#ifdef DEBUG
         if (fDebugOutput) std::cerr << "getXMLJobProperties returns \"" << oss.str () << "\"" << std::endl;
#endif

         return new std::string (oss.str ());
      }
   }

   return 0;
}

bool OmniDomParser::
openOutputFiles (std::string    *pstringName,
                 std::ofstream **ppofstreamHPP,
                 std::ofstream **ppofstreamCPP)
{
   char     achFilename[513]; //@TBD

   strcpy (achFilename, pstringName->c_str ());
   strcat (achFilename, ".hpp");
   convertFilename (achFilename);

   *ppofstreamHPP = new std::ofstream (achFilename);

   addToLibraries3 (achFilename);

   strcpy (achFilename, pstringName->c_str ());
   strcat (achFilename, ".cpp");
   convertFilename (achFilename);

   *ppofstreamCPP = new std::ofstream (achFilename);

   addToLibraries3 (achFilename);

   return true;
}

bool
openMakeFile (std::string    *pstringName,
              std::ofstream **ppofstreamMAK)
{
   char achFilename[512]; //@TBD

   strcpy (achFilename, pstringName->c_str ());
   strcat (achFilename, ".mak");
   convertFilename (achFilename);

   *ppofstreamMAK = new std::ofstream (achFilename);

   return true;
}

void
outputHeader (std::ofstream *pofstream)
{
   *pofstream << "/*" << std::endl;
   *pofstream << " *   IBM Omni driver" << std::endl;
   *pofstream << " *   Copyright (c) International Business Machines Corp., 2000" << std::endl;
   *pofstream << " *" << std::endl;
   *pofstream << " *   This library is free software; you can redistribute it and/or modify" << std::endl;
   *pofstream << " *   it under the terms of the GNU Lesser General Public License as published" << std::endl;
   *pofstream << " *   by the Free Software Foundation; either version 2.1 of the License, or" << std::endl;
   *pofstream << " *   (at your option) any later version." << std::endl;
   *pofstream << " *" << std::endl;
   *pofstream << " *   This library is distributed in the hope that it will be useful," << std::endl;
   *pofstream << " *   but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl;
   *pofstream << " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See" << std::endl;
   *pofstream << " *   the GNU Lesser General Public License for more details." << std::endl;
   *pofstream << " *" << std::endl;
   *pofstream << " *   You should have received a copy of the GNU Lesser General Public License" << std::endl;
   *pofstream << " *   along with this library; if not, write to the Free Software" << std::endl;
   *pofstream << " *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA" << std::endl;
   *pofstream << " */" << std::endl;
}

PSZCRO
bundleStringData (XmlNodePtr nodeItem)
{
   return XMLNodeListGetString (XMLGetDocNode (nodeItem),
                                XMLGetChildrenNode (nodeItem),
                                1);
}

int
bundleIntegerData (XmlNodePtr nodeItem)
{
   PSZCRO pszData = XMLNodeListGetString (XMLGetDocNode (nodeItem),
                                          XMLGetChildrenNode (nodeItem),
                                          1);
   int    iRet    = 0;

   if (1 != sscanf (pszData, "%d", &iRet))
   {
      iRet = 0;
   }

   free ((void *)pszData);

   return iRet;
}

double
bundleDoubleData (XmlNodePtr nodeItem)
{
   PSZCRO pszData = XMLNodeListGetString (XMLGetDocNode (nodeItem),
                                          XMLGetChildrenNode (nodeItem),
                                          1);
   double dRet    = 0;

   if (1 != sscanf (pszData, "%lf", &dRet))
   {
      dRet = 0;
   }

   free ((void *)pszData);

   return dRet;
}

bool
bundleBooleanData (XmlNodePtr nodeItem)
{
   PSZCRO pszData = XMLNodeListGetString (XMLGetDocNode (nodeItem),
                                          XMLGetChildrenNode (nodeItem),
                                          1);
   bool   fRet    = false;

   if (0 == strcasecmp (pszData, "true"))
   {
      fRet = true;
   }
   else if (0 == strcasecmp (pszData, "false"))
   {
      fRet = false;
   }
   else
   {
      // @TBD
   }

   free ((void *)pszData);

   return fRet;
}

void
printNodeName (XmlNodePtr cur)
{
#ifdef DEBUG
   const char *pszNodeName = 0;

   if (!fDebugOutput)
      return;

   pszNodeName = bundleStringData (cur);

   std::cerr << "Node <"
             << XMLGetName (cur);
   if (pszNodeName)
   {
      std::cerr << " "
                << pszNodeName;
   }
   std::cerr << ">"
             << std::endl;

   if (pszNodeName)
   {
      free ((void *)pszNodeName);
   }
#endif
}

bool
parseHexGroup (const char   *pszData,
               unsigned int *puiData)
{
   if (  isxdigit (pszData[0])
      && isxdigit (pszData[1])
      )
   {
      return 1 == sscanf (pszData, "%x", puiData);
   }
   else
   {
      return false;
   }
}

bool OmniDomParser::
parseBinaryData (const char  *pszData,
                 byte       **ppbData,
                 int         *pcbData)
{
   int   cbDataAlloc    = 64;
   int   cbDataLeft     = 64;
   byte *pbData         = new byte [cbDataAlloc];
   byte *pbDataCurrent  = pbData;
   bool  fContinue      = true;
   bool  fInString      = false;
   byte  bChar          = 0;
   byte  bNextChar      = 0;
   bool  fCharFound     = false;
   bool  fNextCharFound = false;

   *ppbData = pbData;
   *pcbData = cbDataAlloc - cbDataLeft;

   while (  *pszData
         && fContinue
         )
   {
      if (fNextCharFound)
      {
         bChar          = bNextChar;
         fNextCharFound = false;
         fCharFound     = true;
      }
      else
      {
         if (fInString)
         {
            switch (*pszData)
            {
            case '\\':
            {
               break;
            }

            case '"':
            {
               pszData++;

               fInString = false;
               continue;
            }

            default:
            {
               bChar      = *pszData++;
               fCharFound = true;
               break;
            }
            }
         }
         else
         {
            while (  '\x0a' == *pszData
                  || '\x0d' == *pszData
                  || ' '    == *pszData
                  )
               pszData++;

            switch (*pszData)
            {
            case '_':
            {
               pszData++;

               if (0 == strncmp (pszData, "ESC_", 4))
               {
                  bChar      = 0x1b;
                  fCharFound = true;
                  pszData += 4;
               }
               else if (0 == strncmp (pszData, "NUL_", 4))
               {
                  bChar      = 0;
                  fCharFound = true;
                  pszData += 4;
               }
               else if (0 == strncmp (pszData, "LF_", 3))
               {
                  bChar      = 0x0a;
                  fCharFound = true;
                  pszData += 3;
               }
               else if (0 == strncmp (pszData, "CR_", 3))
               {
                  bChar      = 0x0d;
                  fCharFound = true;
                  pszData += 3;
               }
               else if (0 == strncmp (pszData, "FF_", 3))
               {
                  bChar      = 0x0c;
                  fCharFound = true;
                  pszData += 3;
               }
               else
               {
                  std::cerr << "Error: Unknown define " << pszData << std::endl;
                  setErrorCondition ();
                  fContinue = false;
               }
               break;
            }

            case 'H':
            {
               pszData++;

               if (0 == strncmp (pszData, "EX2S", 4))
               {
                  pszData += 4;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     unsigned int uiData  = 0;
                     unsigned int uiData2 = 0;

                     if (parseHexGroup (pszData, &uiData))
                     {
                        pszData += 2;

                        while (' ' == *pszData)
                           pszData++;

                        if (',' == *pszData)
                        {
                           pszData++;

                           while (' ' == *pszData)
                              pszData++;

                           if (parseHexGroup (pszData, &uiData2))
                           {
                              pszData += 2;

                              while (' ' == *pszData)
                                 pszData++;

                              if (')' == *pszData)
                              {
                                 pszData++;

                                 bChar          = (byte)uiData2;
                                 bNextChar      = (byte)uiData;
                                 fCharFound     = true;
                                 fNextCharFound = true;
                              }
                              else
                              {
                                 std::cerr << "Error: Expecting ')', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                                 setErrorCondition ();
                                 fContinue = false;
                              }
                           }
                           else
                           {
                              std::cerr << "Error: Expecting 2 hex digits, found '" << *pszData << "' @ " << __LINE__ << std::endl;
                              setErrorCondition ();
                              fContinue = false;
                           }
                        }
                        else
                        {
                           std::cerr << "Error: Expecting ',', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                           setErrorCondition ();
                           fContinue = false;
                        }
                     }
                     else
                     {
                        std::cerr << "Error: Expecting 2 hex digits, found '" << pszData[0] << pszData[1] << "' @ " << __LINE__ << std::endl;
                        setErrorCondition ();
                        fContinue = false;
                     }
                  }
                  else
                  {
                     std::cerr << "Error: Expecting '(', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                     setErrorCondition ();
                     fContinue = false;
                  }
               }
               else if (0 == strncmp (pszData, "EX", 2))
               {
                  pszData += 2;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     unsigned int uiData = 0;

                     if (parseHexGroup (pszData, &uiData))
                     {
                        pszData += 2;

                        while (' ' == *pszData)
                           pszData++;

                        if (')' == *pszData)
                        {
                           pszData++;

                           bChar      = (byte)uiData;
                           fCharFound = true;
                        }
                        else
                        {
                           std::cerr << "Error: Expecting ')', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                           setErrorCondition ();
                           fContinue = false;
                        }
                     }
                     else
                     {
                        std::cerr << "Error: Expecting 2 hex digits, found '" << pszData[0] << pszData[1] << "' @ " << __LINE__ << std::endl;
                        setErrorCondition ();
                        fContinue = false;
                     }
                  }
                  else
                  {
                     std::cerr << "Error: Expecting '(', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                     setErrorCondition ();
                     fContinue = false;
                  }
               }
               break;
            }

            case 'A':
            {
               pszData++;

               if (0 == strncmp (pszData, "SCII", 4))
               {
                  pszData += 4;

                  while (' ' == *pszData)
                     pszData++;

                  if ('(' == *pszData)
                  {
                     pszData++;
                     while (' ' == *pszData)
                        pszData++;

                     bChar      = (byte)*pszData++;
                     fCharFound = true;

                     if (')' == *pszData)
                     {
                        pszData++;
                     }
                     else
                     {
                        std::cerr << "Error: Expecting ')', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                        setErrorCondition ();
                        fContinue = false;
                     }
                  }
                  else
                  {
                     std::cerr << "Error: Expecting '(', found '" << *pszData << "' @ " << __LINE__ << std::endl;
                     setErrorCondition ();
                     fContinue = false;
                  }
               }
               break;
            }

            case '"':
            {
               pszData++;
               fInString = true;
               break;
            }
            }
         }
      }

      if (fCharFound)
      {
/////////if (isprint (bChar))
/////////   std::cerr << "bChar = '" << bChar << "'" << std::endl;
/////////else
/////////   std::cerr << "bChar = 0x" << std::hex << (int)bChar << std::dec << std::endl;

         *pbDataCurrent++ = bChar;
         cbDataLeft--;

         bChar      = 0;
         fCharFound = false;

         if (0 == cbDataLeft)
         {
            byte *pbTemp = new byte [cbDataAlloc + 64];

            if (pbTemp)
            {
               memcpy (pbTemp, pbData, cbDataAlloc);

               delete[] pbData;

               pbData        = pbTemp;
               pbDataCurrent = pbData + cbDataAlloc;
               cbDataAlloc  += 64;
               cbDataLeft    = 64;
            }
            else
            {
               std::cerr << "Error: Allocation of " << (cbDataAlloc + 64) << " bytes failed @ " << __LINE__ << std::endl;
               setErrorCondition ();
               fContinue = false;
            }
         }
      }
      else if (  !fInString
              && *pszData
              )
      {
         delete[] pbData;
         return false;
      }
   }

   if (fContinue)
   {
      *ppbData = pbData;
      *pcbData = cbDataAlloc - cbDataLeft;
   }
   else
   {
      delete[] pbData;
   }

   return fContinue;
}

bool OmniDomParser::
processDeviceCopies (XmlNodePtr deviceCopiesNode)
{
#ifdef INCLUDE_JP_COMMON_COPIES
   if (deviceCopiesNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringCopyClassName = pstringNameOut;

   XmlNodePtr      deviceCopyNode     = XMLFirstNode (XMLGetChildrenNode (deviceCopiesNode));
   XmlNodePtr      nodeItem           = 0;
   std::ofstream  *pofstreamHPP       = 0;
   std::ofstream  *pofstreamCPP       = 0;
   const char     *pszMinimum         = 0;
   const char     *pszMaximum         = 0;
   const char     *pszBinaryData      = 0;
   byte           *pbData             = 0;
   int             cbData             = 0;
   BinaryData     *pbdData            = 0;
   bool           fSimulationRequired = false;
   const char     *pszDeviceID        = 0;
   int             rc                 = false;

   if (deviceCopyNode == 0)
      goto done;

   nodeItem = XMLFirstNode (XMLGetChildrenNode (deviceCopyNode));
   if (nodeItem == 0)
      goto done;

   pszMinimum = bundleStringData (nodeItem);
   if (!pszMinimum)
   {
      std::cerr << "Error: Missing minimum copies" << std::endl;
      setErrorCondition ();
      goto done;
   }

   // Move to the next one
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   pszMaximum = bundleStringData (nodeItem);
   if (!pszMaximum)
   {
      std::cerr << "Error: Missing maximum copies" << std::endl;
      setErrorCondition ();
      goto done;
   }

   // Move to the next one
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   pszBinaryData = bundleStringData (nodeItem);

   if (parseBinaryData (pszBinaryData, &pbData, &cbData))
   {
      pbdData = new BinaryData (pbData, cbData);
   }
   else
   {
      std::cerr << "Error: binary data not understood \""
                << pszBinaryData
                << "\" for "
                << pszMinimum
                << ", "
                << pszMaximum
                << "."
                << std::endl;
      goto done;
   }

   // Move to the next one
   nodeItem = XMLNextNode (nodeItem);

   fSimulationRequired = bundleBooleanData (nodeItem);

   // Move to the next one
   nodeItem = XMLNextNode (nodeItem);

   if (nodeItem != 0)
   {
      pszDeviceID = bundleStringData (nodeItem);
   }

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceCopies.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceCopies" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *data," << std::endl;
      *pofstreamHPP << "                          int          iMinimum," << std::endl;
      *pofstreamHPP << "                          int          iMaximum," << std::endl;
      *pofstreamHPP << "                          bool         fSimulationRequired);" << std::endl;
      *pofstreamHPP << "~" << *pstringNameOut << " ();" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceCopies        *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceCopies *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (pszDeviceID)
      {
         *pofstreamHPP << "   PSZCRO               getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamCPP << "                          int          iMinimum," << std::endl;
      *pofstreamCPP << "                          int          iMaximum," << std::endl;
      *pofstreamCPP << "                          bool         fSimulationRequired)" << std::endl;
      *pofstreamCPP << "      : DeviceCopies (pDevice," << std::endl;
      *pofstreamCPP << "                      pszJobProperties," << std::endl;
      *pofstreamCPP << "                      pbdData," << std::endl;
      *pofstreamCPP << "                      iMinimum," << std::endl;
      *pofstreamCPP << "                      iMaximum," << std::endl;
      *pofstreamCPP << "                      fSimulationRequired)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "~" << *pstringNameOut << "()" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;
      *pofstreamCPP << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static byte vabData[] = {";

      BinaryData *pbdValue = pbdData;
      PBYTE       pbValue  = pbdValue->getData ();
      int         cbValue  = pbdValue->getLength ();

      for (int i = 0; i < cbValue; i++)
      {
         *pofstreamCPP << (int)pbValue[i];
         if (i < cbValue - 1)
            *pofstreamCPP << ",";
      }

      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceCopies * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Copies\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         std::ostringstream oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         oss << pszKey << \"=\" << pszValue;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "oss.str ().c_str (), "
                    << "new BinaryData (vabData, sizeof (vabData)), "
                    << pszMinimum
                    << ", "
                    << pszMaximum
                    << ", "
                    << fSimulationRequired
                    << ");"
                    << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceCopies * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std:: endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std:: endl;
      *pofstreamCPP << "{" << std:: endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std:: endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std:: endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std:: endl;
      *pofstreamCPP << "   {" << std:: endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std:: endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Copies\"))" << std:: endl;
      *pofstreamCPP << "      {" << std:: endl;
      *pofstreamCPP << "         int iCopies = atoi (pszValue);" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "         if (  " << pszMinimum << " <= iCopies" << std:: endl;
      *pofstreamCPP << "            && iCopies <= " << pszMaximum << std:: endl;
      *pofstreamCPP << "            )" << std:: endl;
      *pofstreamCPP << "         {" << std:: endl;
      *pofstreamCPP << "            fRet = true;" << std:: endl;
      *pofstreamCPP << "         }" << std:: endl;
      *pofstreamCPP << "      }" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std:: endl;
      *pofstreamCPP << "   }" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "   delete pEnum;" << std:: endl;
      *pofstreamCPP << std:: endl;
      *pofstreamCPP << "   return fRet;" << std:: endl;
      *pofstreamCPP << "}" << std:: endl;

      if (pszDeviceID)
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return \"" << pszDeviceID << "\";" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << "" << std:: endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << " ::" << std:: endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class CopiesValueEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      CopiesValueEnumerator (int iNumCopies," << std::endl;
      *pofstreamCPP << "                             int iMinimum," << std::endl;
      *pofstreamCPP << "                             int iMaximum," << std::endl;
      *pofstreamCPP << "                             bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         iNumCopies_d        = iNumCopies;" << std::endl;
      *pofstreamCPP << "         iMinimum_d          = iMinimum;" << std::endl;
      *pofstreamCPP << "         iMaximum_d          = iMaximum;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "         fReturnedValue_d    = false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual ~" << std::endl;
      *pofstreamCPP << "      CopiesValueEnumerator ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         iNumCopies_d        = 0;" << std::endl;
      *pofstreamCPP << "         iMinimum_d          = 0;" << std::endl;
      *pofstreamCPP << "         iMaximum_d          = 0;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = false;" << std::endl;
      *pofstreamCPP << "         fReturnedValue_d    = true;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return !fReturnedValue_d;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!fReturnedValue_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            std::ostringstream oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            fReturnedValue_d = true;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            oss << \"Copies=\";" << std::endl;
      *pofstreamCPP << std::endl;
      if (pszDeviceID)
      {
         *pofstreamCPP << "            if (fInDeviceSpecific_d)" << std::endl;
         *pofstreamCPP << "            {" << std::endl;
         *pofstreamCPP << "               oss << \"" << pszDeviceID << "\";" << std::endl;
         *pofstreamCPP << "            }" << std::endl;
         *pofstreamCPP << "            else" << std::endl;
         *pofstreamCPP << "            {" << std::endl;
         *pofstreamCPP << "               oss << \"{\"" << std::endl;
         *pofstreamCPP << "                   << iNumCopies_d" << std::endl;
         *pofstreamCPP << "                   << \",\"" << std::endl;
         *pofstreamCPP << "                   << iMinimum_d" << std::endl;
         *pofstreamCPP << "                   << \",\"" << std::endl;
         *pofstreamCPP << "                   << iMaximum_d" << std::endl;
         *pofstreamCPP << "                   << \"}\";" << std::endl;
         *pofstreamCPP << "            }" << std::endl;
      }
      else
      {
         *pofstreamCPP << "            oss << \"{\"" << std::endl;
         *pofstreamCPP << "                << iNumCopies_d" << std::endl;
         *pofstreamCPP << "                << \",\"" << std::endl;
         *pofstreamCPP << "                << iMinimum_d" << std::endl;
         *pofstreamCPP << "                << \",\"" << std::endl;
         *pofstreamCPP << "                << iMaximum_d" << std::endl;
         *pofstreamCPP << "                << \"}\";" << std::endl;
      }
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            stringReturn_d = oss.str ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (stringReturn_d);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      int         iNumCopies_d;" << std::endl;
      *pofstreamCPP << "      int         iMinimum_d;" << std::endl;
      *pofstreamCPP << "      int         iMaximum_d;" << std::endl;
      *pofstreamCPP << "      bool        fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "      bool        fReturnedValue_d;" << std::endl;
      *pofstreamCPP << "      std::string stringReturn_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new CopiesValueEnumerator (iNumCopies_d, iMinimum_d, iMaximum_d, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceCopiesNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringCopyClassName = 0;
   }

   if (pszMinimum)
   {
      free ((void *)pszMinimum);
   }
   if (pszMaximum)
   {
      free ((void *)pszMaximum);
   }
   if (pszBinaryData)
   {
      free ((void *)pszBinaryData);
   }
   if (pszDeviceID)
   {
      free ((void *)pszDeviceID);
   }

   delete   pbdData;
   delete[] pbData;

   return true;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceForms (XmlNodePtr deviceFormsNode)
{
   if (deviceFormsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringFormClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> FormMapElement;
   typedef std::map <std::string, BinaryData *>  FormMapData1;
   typedef std::map <std::string, byte *>        FormMapData2;
   typedef std::list <std::string>               FormList;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr      deviceFormNode = XMLFirstNode (XMLGetChildrenNode (deviceFormsNode));
   std::ofstream  *pofstreamHPP   = 0;
   std::ofstream  *pofstreamCPP   = 0;
   FormMapElement  mapElement;
   FormMapData1    mapData1;
   FormMapData2    mapData2;
   FormList        listType;
   DeviceIDList    mapDeviceID;
   int             iCount         = 0;
   int             rc             = false;

   if (deviceFormNode == 0)
      goto done;

   while (deviceFormNode != 0)
   {
      XmlNodePtr   nodeItem         = XMLFirstNode (XMLGetChildrenNode (deviceFormNode));
      XmlNodePtr   nodeHCC;
      const char  *pszId            = 0;
      PSZRO        pszLongFormName  = 0;
      PSZRO        pszShortFormName = 0;
      char        *pszShortId       = 0;
      const char  *pszCapabilities  = 0;
      const char  *pszBinaryData    = 0;
      const char  *pszDeviceID      = 0;
      byte        *pbData           = 0;
      int          cbData           = 0;
      BinaryData  *pbdData          = 0;
      int          iLeft            = 0;
      int          iTop             = 0;
      int          iRight           = 0;
      int          iBottom          = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "Form=" << const_cast<char*>(pszId);

         if (!DeviceForm::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceForm \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceForm" << std::endl;
         setErrorCondition ();
         goto done;
      }

      pszLongFormName  = DeviceForm::getLongFormName (pszId);
      pszShortFormName = DeviceForm::getShortFormName (pszId);

      if (pszShortFormName)
      {
         pszShortId = (char *)malloc (strlen (pszShortFormName) + 1);
         if (pszShortId)
         {
            strcpy (pszShortId, pszShortFormName);
            for (int i = 0; pszShortId[i]; i++)
            {
               if (  '-' == pszShortId[i]
                  || '.' == pszShortId[i]
                  )
               {
                  pszShortId[i] = '_';
               }
            }
         }
      }

      pdi_d->listForms.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      pszCapabilities = bundleStringData (nodeItem);

      if (!DeviceForm::isReservedKeyword (pszCapabilities))
      {
         listType.push_back (std::string (pszCapabilities));
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      // Move down into the HardCopyCap tree
      nodeHCC = XMLFirstNode (XMLGetChildrenNode (nodeItem));
      if (nodeHCC == 0)
         goto done;

      iLeft = bundleIntegerData (nodeHCC);

      // Move to the next one
      nodeHCC = XMLNextNode (nodeHCC);
      if (nodeHCC == 0)
         goto done;

      iTop = bundleIntegerData (nodeHCC);

      // Move to the next one
      nodeHCC = XMLNextNode (nodeHCC);
      if (nodeHCC == 0)
         goto done;

      iRight = bundleIntegerData (nodeHCC);

      // Move to the next one
      nodeHCC = XMLNextNode (nodeHCC);
      if (nodeHCC == 0)
         goto done;

      iBottom = bundleIntegerData (nodeHCC);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszShortFormName
          << "\",\""
          << pszLongFormName
          << "\", \"Form="
          << pszLongFormName
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Form="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", DeviceForm::"
          << pszCapabilities
          << ", abData"
          << pszShortId
          << ", "
          << cbData
          << ", "
          << iLeft
          << ","
          << iTop
          << ","
          << iRight
          << ","
          << iBottom
          << " }";

      mapData2[std::string (pszId)]              = pbData;
      mapData1[std::string (pszShortId)]         = pbdData;
      mapElement[std::string (pszShortFormName)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId           = " << pszId           << std::endl;
         std::cerr << "pszCapabilities = " << pszCapabilities << std::endl;
         std::cerr << "pszBinaryData   = " << pszBinaryData   << std::endl;
         std::cerr << "pbdData         = " << *pbdData        << std::endl;
         std::cerr << "   iLeft        = " << iLeft           << std::endl;
         std::cerr << "   iTop         = " << iTop            << std::endl;
         std::cerr << "   iRight       = " << iRight          << std::endl;
         std::cerr << "   iBottom      = " << iBottom         << std::endl;
      }
#endif

      // Get the next deviceForm node
      deviceFormNode = XMLNextNode (deviceFormNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszShortFormName)
      {
         free ((void *)pszShortFormName);
      }
      if (pszLongFormName)
      {
         free ((void *)pszLongFormName);
      }
      if (pszShortId)
      {
         free ((void *)pszShortId);
      }
      if (pszCapabilities)
      {
         free ((void *)pszCapabilities);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listForms.sort ();
   pdi_d->listForms.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceForm.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceForm" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << "   enum {" << std::endl;
      listType.sort ();
      listType.unique ();
      iCount = listType.size ();
      for ( FormList::iterator next = listType.begin () ;
            next != listType.end () ;
            next++, iCount-- )
      {
         *pofstreamHPP << "      " << *next;
         if (1 < iCount)
            *pofstreamHPP << ",";
         *pofstreamHPP << std::endl;
      }
      *pofstreamHPP << "   };" << std::endl;
      *pofstreamHPP << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          int          iCapabilities," << std::endl;
      *pofstreamHPP << "                          BinaryData  *data," << std::endl;
      *pofstreamHPP << "                          HardCopyCap *hcInfo);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceForm          *create          (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceForm   *createS         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported     (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO       getDeviceID     ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration  (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          int          iCapabilities," << std::endl;
      *pofstreamCPP << "                          BinaryData  *data," << std::endl;
      *pofstreamCPP << "                          HardCopyCap *hcInfo)" << std::endl;
      *pofstreamCPP << "      : DeviceForm (pDevice," << std::endl;
      *pofstreamCPP << "                    pszJobProperties," << std::endl;
      *pofstreamCPP << "                    iCapabilities," << std::endl;
      *pofstreamCPP << "                    data," << std::endl;
      *pofstreamCPP << "                    hcInfo)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      for ( FormMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszShortName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszLongName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   int    iCapabilities;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    iLeft;" << std::endl;
      *pofstreamCPP << "   int    iTop;" << std::endl;
      *pofstreamCPP << "   int    iRight;" << std::endl;
      *pofstreamCPP << "   int    iBottom;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( FormMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszFormName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszFormName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      if (DeviceForm::FORM_CAPABILITY_ROLL == aTable[iMid].iCapabilities)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         PSZCRO pszShortTableName = DeviceForm::getShortFormName (aTable[iMid].pszShortName);" << std::endl;
      *pofstreamCPP << "         PSZCRO pszShortFormName  = DeviceForm::getShortFormName (pszFormName);" << std::endl;
      *pofstreamCPP << "" << std::endl;
      *pofstreamCPP << "         if (  pszShortTableName" << std::endl;
      *pofstreamCPP << "            && pszShortFormName" << std::endl;
      *pofstreamCPP << "            )" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            iResult = strcmp (pszShortFormName, pszShortTableName);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return -1;" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "" << std::endl;
      *pofstreamCPP << "         if (pszShortTableName)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            free ((void *)pszShortTableName);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "         if (pszShortFormName)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            free ((void *)pszShortFormName);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         iResult = strcmp (pszFormName, aTable[iMid].pszShortName);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceForm * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Form\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         PSZCRO      pszShortName = DeviceForm::getShortFormName (pszValue);" << std::endl;
      *pofstreamCPP << "         int         iIndex       = -1;" << std::endl;
      *pofstreamCPP << "         DeviceForm *pForm        = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (pszShortName)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            iIndex = findTableEntry (pszShortName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            free ((void *)pszShortName);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (DeviceForm::FORM_CAPABILITY_ROLL == aTable[iIndex].iCapabilities)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               PSZCRO pszLongName = DeviceForm::getLongFormName (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "               if (pszLongName)" << std::endl;
      *pofstreamCPP << "               {" << std::endl;
      *pofstreamCPP << "                  std::ostringstream oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "                  oss << \"Form=\" << pszLongName;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "                  pForm = new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "oss.str ().c_str (), "
                    << "aTable[iIndex].iCapabilities, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData), "
                    << "new HardCopyCap (aTable[iIndex].iLeft, "
                    << "aTable[iIndex].iTop, "
                    << "aTable[iIndex].iRight, "
                    << "aTable[iIndex].iBottom));"
                    << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "                  free ((void *)pszLongName);" << std::endl;
      *pofstreamCPP << "               }" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            if (!pForm)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pForm = new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "aTable[iIndex].iCapabilities, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData), "
                    << "new HardCopyCap (aTable[iIndex].iLeft, "
                    << "aTable[iIndex].iTop, "
                    << "aTable[iIndex].iRight, "
                    << "aTable[iIndex].iBottom));"
                    << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pForm;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceForm * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "static PSZCRO" << std::endl;
         *pofstreamCPP << "findDeviceID (PSZCRO pszForm)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszForm" << std::endl;
         *pofstreamCPP << "      || !*pszForm" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszForm, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszForm, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszForm_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Form\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         PSZCRO pszShortName = DeviceForm::getShortFormName (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (pszShortName)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            fRet = 0 <= findTableEntry (pszShortName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "            free ((void *)pszShortName);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class FormEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      FormEnumerator (Device *pDevice, int cForms, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iForm_d             = 0;" << std::endl;
      *pofstreamCPP << "         cForms_d            = cForms;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iForm_d < cForms_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iForm_d > cForms_d - 1)" << std::endl;
      *pofstreamCPP << "            return pvRet;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iForm_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iForm_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iForm_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iForm_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iForm_d;" << std::endl;
      *pofstreamCPP << "      int           cForms_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new FormEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceFormsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringFormClassName = 0;
   }

   for ( FormMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( FormMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( FormMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceMedias (XmlNodePtr deviceMediasNode)
{
   if (deviceMediasNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringMediaClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> MediaMapElement;
   typedef std::map <std::string, BinaryData *>  MediaMapData1;
   typedef std::map <std::string, byte *>        MediaMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr      deviceMediaNode = XMLFirstNode (XMLGetChildrenNode (deviceMediasNode));
   std::ofstream  *pofstreamHPP    = 0;
   std::ofstream  *pofstreamCPP    = 0;
   MediaMapElement mapElement;
   MediaMapData1   mapData1;
   MediaMapData2   mapData2;
   DeviceIDList    mapDeviceID;
   int             rc              = false;

   if (deviceMediaNode == 0)
      goto done;

   while (deviceMediaNode != 0)
   {
      XmlNodePtr  nodeItem             = XMLFirstNode (XMLGetChildrenNode (deviceMediaNode));
      const char *pszId                = 0;
      const char *pszBinaryData        = 0;
      const char *pszDeviceID          = 0;
      byte       *pbData               = 0;
      int         cbData               = 0;
      BinaryData *pbdData              = 0;
      int         iColorAdjustRequired = 0;
      const char *pszAbsorption        = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "media=" << const_cast<char*>(pszId);

         if (!DeviceMedia::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceMedia \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }

      pdi_d->listMedias.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iColorAdjustRequired = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszAbsorption = bundleStringData (nodeItem);

      // @TBD
      if (  0 != strcmp (pszAbsorption, "MEDIA_NO_ABSORPTION")
         && 0 != strcmp (pszAbsorption, "MEDIA_LIGHT_ABSORPTION")
         && 0 != strcmp (pszAbsorption, "MEDIA_HEAVY_ABSORPTION")
         )
         goto done;

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"media="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"media="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", abData"
          << pszId
          << ", "
          << cbData
          << ", "
          << iColorAdjustRequired
          << ", DeviceMedia::"
          << pszAbsorption
          << "}";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszId)]   = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                = " << pszId                << std::endl;
         std::cerr << "pszBinaryData        = " << pszBinaryData        << std::endl;
         std::cerr << "pbdData              = " << *pbdData             << std::endl;
         std::cerr << "iColorAdjustRequired = " << iColorAdjustRequired << std::endl;
         std::cerr << "pszAbsorption        = " << pszAbsorption        << std::endl;
      }
#endif

      // Get the next deviceMedia node
      deviceMediaNode = XMLNextNode (deviceMediaNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszAbsorption)
      {
         free ((void *)pszAbsorption);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listMedias.sort ();
   pdi_d->listMedias.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceMedia.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceMedia" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                           PSZCRO      pszJobProperties," << std::endl;
      *pofstreamHPP << "                           BinaryData *data," << std::endl;
      *pofstreamHPP << "                           int         iColorAdjustRequired," << std::endl;
      *pofstreamHPP << "                           int         iAbsorption);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceMedia         *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceMedia  *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO       getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                           PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                           BinaryData *data," << std::endl;
      *pofstreamCPP << "                           int         iColorAdjustRequired," << std::endl;
      *pofstreamCPP << "                           int         iAbsorption)" << std::endl;
      *pofstreamCPP << "      : DeviceMedia (pDevice," << std::endl;
      *pofstreamCPP << "                     pszJobProperties," << std::endl;
      *pofstreamCPP << "                     data," << std::endl;
      *pofstreamCPP << "                     iColorAdjustRequired," << std::endl;
      *pofstreamCPP << "                     iAbsorption)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszMedia)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszMedia" << std::endl;
         *pofstreamCPP << "      || !*pszMedia" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszMedia, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszMedia, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszMedia_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( MediaMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    iColorAdjustRequired;" << std::endl;
      *pofstreamCPP << "   int    iAbsorption;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( MediaMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszMediaName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszMediaName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszMediaName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceMedia * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"media\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData), "
                    << "aTable[iIndex].iColorAdjustRequired, "
                    << "aTable[iIndex].iAbsorption); "
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceMedia * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"media\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class MediaEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      MediaEnumerator (Device *pDevice, int cMedias, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iMedia_d            = 0;" << std::endl;
      *pofstreamCPP << "         cMedias_d           = cMedias;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iMedia_d < cMedias_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iMedia_d > cMedias_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iMedia_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iMedia_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iMedia_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iMedia_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iMedia_d;" << std::endl;
      *pofstreamCPP << "      int           cMedias_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new MediaEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceMediasNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringMediaClassName = 0;
   }

   for ( MediaMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( MediaMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( MediaMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceNUps (XmlNodePtr deviceNUpsNode)
{
#ifdef INCLUDE_JP_COMMON_NUP
   if (deviceNUpsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringNUpClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> NUpMapElement;
   typedef std::map <std::string, BinaryData *>  NUpMapData1;
   typedef std::map <std::string, byte *>        NUpMapData2;
   typedef std::list <std::string>               NUpList;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr      deviceNUpNode  = XMLFirstNode (XMLGetChildrenNode (deviceNUpsNode));
   std::ofstream  *pofstreamHPP   = 0;
   std::ofstream  *pofstreamCPP   = 0;
   NUpMapElement   mapElement;
   NUpMapData1     mapData1;
   NUpMapData2     mapData2;
   DeviceIDList    mapDeviceID;
   int             rc             = false;

   if (deviceNUpNode == 0)
      goto done;

   while (deviceNUpNode != 0)
   {
      XmlNodePtr  nodeItem            = XMLFirstNode (XMLGetChildrenNode (deviceNUpNode));
      XmlNodePtr  nodeXY              = XMLFirstNode (XMLGetChildrenNode (nodeItem));
      const char *pszPresentationDir  = 0;
      const char *pszBinaryData       = 0;
      byte       *pbData              = 0;
      int         cbData              = 0;
      BinaryData *pbdData             = 0;
      bool        fSimulationRequired = false;
      const char *pszDeviceID         = 0;
      std::string stringShortId;
      int         iX                  = 0;
      int         iY                  = 0;

      if (nodeItem == 0)
         goto done;

      if (nodeXY == 0)
         goto done;

      iX = bundleIntegerData (nodeXY);
      if (!iX)
      {
         std::cerr << "Error: X = 0" << std::endl;
         goto done;
      }

      // Move to the next one
      nodeXY = XMLNextNode (nodeXY);
      if (nodeXY == 0)
         goto done;

      iY = bundleIntegerData (nodeXY);
      if (!iY)
      {
         std::cerr << "Error: Y = 0" << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszPresentationDir = bundleStringData (nodeItem);

      if (-1 == DeviceNUp::directionIndex (pszPresentationDir))
      {
         std::cerr << "Error: " << pszPresentationDir << " is not a correct word." << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << iX
                   << ", "
                   << iY
                   << ", "
                   << pszPresentationDir
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      fSimulationRequired = bundleBooleanData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         std::ostringstream  oss;

         pszDeviceID = bundleStringData (nodeItem);

         if (pszDeviceID)
         {
            oss << "(0 == strcmp (pszDirection_d, \""
                << pszPresentationDir
                << "\")) && (iX_d == "
                << iX
                << ") && (iY_d == "
                << iY
                << ")";

            mapDeviceID[std::string (pszDeviceID)] = oss.str ();
         }
      }

      std::ostringstream oss;

      oss << pszPresentationDir
          << "_"
          << iX
          << "_"
          << iY;

      stringShortId = oss.str ();

      oss.str ("");
      oss << "NumberUp=" << iX << "X" << iY;
      oss << " NumberUpDirection=" << pszPresentationDir;

      if (!DeviceNUp::isValid (oss.str ().c_str ()))
      {
         std::cerr << "Error: Unknown DeviceNUp \"" << oss.str () << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      oss.str ("");
      oss << "   { \"NumberUp="
          << iX
          << "X"
          << iY
          << " NumberUpDirection="
          << pszPresentationDir
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"NumberUp="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << fSimulationRequired
          << ", abData"
          << stringShortId
          << ", "
          << cbData
          << "}";

      mapData2[stringShortId]   = pbData;
      mapData1[stringShortId]   = pbdData;
      mapElement[stringShortId] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "stringShortId       = " << stringShortId       << std::endl;
         std::cerr << "pszPresentationDir  = " << pszPresentationDir  << std::endl;
         std::cerr << "pszBinaryData       = " << pszBinaryData       << std::endl;
         std::cerr << "pbdData             = " << *pbdData            << std::endl;
         std::cerr << "fSimulationRequired = " << fSimulationRequired << std::endl;
      }
#endif

      // Get the next deviceNUp node
      deviceNUpNode = XMLNextNode (deviceNUpNode);

      if (pszPresentationDir)
      {
         free ((void *)pszPresentationDir);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceNUp.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceNUp" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamHPP << "                          bool         fSimulationRequired);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceNUp           *create        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceNUp    *createS       (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported   (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO       getDeviceID     ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration  (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamCPP << "                          bool         fSimulationRequired)" << std::endl;
      *pofstreamCPP << "      : DeviceNUp (pDevice," << std::endl;
      *pofstreamCPP << "                   pszJobProperties," << std::endl;
      *pofstreamCPP << "                   pbdData," << std::endl;
      *pofstreamCPP << "                   fSimulationRequired)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->first << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( NUpMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   bool   fSimulationRequired;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( NUpMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszNUpName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszNUpName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   for (int i = 0; i < (int)dimof (aTable); i++)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszNUpName, aTable[i].pszJobProperties))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return i;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static std::string *" << std::endl;
      *pofstreamCPP << "getOurProperties (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   std::ostringstream     oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (  0 == strcmp (pszKey, \"NumberUp\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"NumberUpDirection\")" << std::endl;
      *pofstreamCPP << "         )" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (oss.str ().length ())" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            oss << \" \";" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "         oss << pszKey << \"=\" << pszValue;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new std::string (oss.str ());" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceNUp * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int          iIndex    = -1;" << std::endl;
      *pofstreamCPP << "   std::string *pstringJP = 0;" << std::endl;
      *pofstreamCPP << "   DeviceNUp   *pDNUpRet  = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringJP = getOurProperties (pszJobProperties);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pstringJP)" << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pstringJP->c_str ());" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      pDNUpRet = new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData),"
                    << "aTable[iIndex].fSimulationRequired);"
                    << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pstringJP;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return pDNUpRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceNUp * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int          iIndex    = -1;" << std::endl;
      *pofstreamCPP << "   std::string *pstringJP = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringJP = getOurProperties (pszJobProperties);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pstringJP)" << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pstringJP->c_str ());" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pstringJP;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return true;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   else" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return false;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class NUpEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      NUpEnumerator (Device *pDevice, int cNUps, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iNUp_d              = 0;" << std::endl;
      *pofstreamCPP << "         cNUps_d             = cNUps;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iNUp_d < cNUps_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iNUp_d > cNUps_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iNUp_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iNUp_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iNUp_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iNUp_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iNUp_d;" << std::endl;
      *pofstreamCPP << "      int           cNUps_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new NUpEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceNUpsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringNUpClassName = 0;
   }

   for ( NUpMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( NUpMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( NUpMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceOrientations (XmlNodePtr deviceOrientationsNode)
{
   if (deviceOrientationsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringOrientationClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> OrientationMapElement;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr             deviceOrientationNode = XMLFirstNode (XMLGetChildrenNode (deviceOrientationsNode));
   std::ofstream         *pofstreamHPP          = 0;
   std::ofstream         *pofstreamCPP          = 0;
   OrientationMapElement  mapElement;
   DeviceIDList           mapDeviceID;
   int                    rc                    = false;

   if (deviceOrientationNode == 0)
      goto done;

   while (deviceOrientationNode != 0)
   {
      XmlNodePtr  nodeItem            = XMLFirstNode (XMLGetChildrenNode (deviceOrientationNode));
      const char *pszId               = 0;
      bool        fSimulationRequired = false;
      const char *pszDeviceID         = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "Rotation=" << const_cast<char*>(pszId);

         if (!DeviceOrientation::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceOrientation \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceOrientation." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pdi_d->listOrientations.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
      }

      fSimulationRequired = bundleBooleanData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"Rotation="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Rotation="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << fSimulationRequired
          << " }";

      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszId           = " << pszId << std::endl;
#endif

      // Move to the next deviceOrientation node
      deviceOrientationNode = XMLNextNode (deviceOrientationNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <DeviceOrientation.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceOrientation" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          bool         fSimulationRequired);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceOrientation        *create        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceOrientation *createS       (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                      isSupported   (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO            getDeviceID            ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration      *getEnumeration         (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                     PSZCRO  pszJobProperties," << std::endl;
      *pofstreamCPP << "                     bool    fSimulationRequired)" << std::endl;
      *pofstreamCPP << "      : DeviceOrientation (pDevice," << std::endl;
      *pofstreamCPP << "                           pszJobProperties," << std::endl;
      *pofstreamCPP << "                           fSimulationRequired)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszRotation)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszRotation" << std::endl;
         *pofstreamCPP << "      || !*pszRotation" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszRotation, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszRotation, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszRotation_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   bool   fSimulationRequired;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;
      for ( OrientationMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszOrientationName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszOrientationName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszOrientationName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceOrientation * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Rotation\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties,"
                    << "aTable[iIndex].fSimulationRequired);"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceOrientation * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Rotation\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class OrientationEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      OrientationEnumerator (Device *pDevice, int cOrientations, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iOrientation_d      = 0;" << std::endl;
      *pofstreamCPP << "         cOrientations_d     = cOrientations;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iOrientation_d < cOrientations_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iOrientation_d > cOrientations_d - 1)" << std::endl;
      *pofstreamCPP << "            return pvRet;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iOrientation_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iOrientation_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iOrientation_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iOrientation_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iOrientation_d;" << std::endl;
      *pofstreamCPP << "      int           cOrientations_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new OrientationEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceOrientationsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringOrientationClassName = 0;
   }

   for ( OrientationMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceOutputBins (XmlNodePtr deviceOutputBinsNode)
{
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (deviceOutputBinsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringOutputBinClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> OutputBinMapElement;
   typedef std::map <std::string, BinaryData *>  OutputBinMapData1;
   typedef std::map <std::string, byte *>        OutputBinMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr           deviceOutputBinNode = XMLFirstNode (XMLGetChildrenNode (deviceOutputBinsNode));
   std::ofstream       *pofstreamHPP        = 0;
   std::ofstream       *pofstreamCPP        = 0;
   OutputBinMapElement  mapElement;
   OutputBinMapData1    mapData1;
   OutputBinMapData2    mapData2;
   DeviceIDList         mapDeviceID;
   int                  rc                  = false;

   if (deviceOutputBinNode == 0)
      goto done;

   while (deviceOutputBinNode != 0)
   {
      XmlNodePtr  nodeItem      = XMLFirstNode (XMLGetChildrenNode (deviceOutputBinNode));
      const char *pszId         = 0;
      char       *pszCId        = 0;
      const char *pszBinaryData = 0;
      const char *pszDeviceID   = 0;
      byte       *pbData        = 0;
      int         cbData        = 0;
      BinaryData *pbdData       = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "OutputBin=" << const_cast<char*>(pszId);

         if (!DeviceOutputBin::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceOutputBin \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceOutputBin." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pszCId = (char *)malloc (strlen (pszId) + 1);
      if (pszCId)
      {
         strcpy (pszCId, pszId);
         convertCId (pszCId);
      }

      pdi_d->listOutputBins.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"OutputBin="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"OutputBin="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", abData"
          << pszCId
          << ", "
          << cbData
          << "}";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszCId)]  = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                = " << pszId                << std::endl;
         std::cerr << "pszBinaryData        = " << pszBinaryData        << std::endl;
         std::cerr << "pbdData              = " << *pbdData             << std::endl;
      }
#endif

      // Get the next deviceOutputBin node
      deviceOutputBinNode = XMLNextNode (deviceOutputBinNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszCId)
      {
         free ((void *)pszCId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listOutputBins.sort ();
   pdi_d->listOutputBins.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceOutputBin.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceOutputBin" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceOutputBin          *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceOutputBin   *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                      isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO            getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration      *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                     PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                     BinaryData *pbdData)" << std::endl;
      *pofstreamCPP << "      : DeviceOutputBin (pDevice," << std::endl;
      *pofstreamCPP << "                         pszJobProperties," << std::endl;
      *pofstreamCPP << "                         pbdData)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszOutputBin)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszOutputBin" << std::endl;
         *pofstreamCPP << "      || !*pszOutputBin" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszOutputBin, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszOutputBin, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszOutputBin_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( OutputBinMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;
      for ( OutputBinMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszOutputBinName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszOutputBinName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszOutputBinName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceOutputBin * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"OutputBin\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData));"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceOutputBin * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"OutputBin\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class OutputBinEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      OutputBinEnumerator (Device *pDevice, int cOutputBins, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iOutputBin_d        = 0;" << std::endl;
      *pofstreamCPP << "         cOutputBins_d       = cOutputBins;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iOutputBin_d < cOutputBins_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iOutputBin_d > cOutputBins_d - 1)" << std::endl;
      *pofstreamCPP << "            return pvRet;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iOutputBin_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iOutputBin_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iOutputBin_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iOutputBin_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iOutputBin_d;" << std::endl;
      *pofstreamCPP << "      int           cOutputBins_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new OutputBinEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceOutputBinsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringOutputBinClassName = 0;
   }

   for ( OutputBinMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( OutputBinMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( OutputBinMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDevicePrintModes (XmlNodePtr devicePrintModesNode)
{
   if (devicePrintModesNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringPrintModeClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> PrintModeMapElement;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr           devicePrintModeNode = XMLFirstNode (XMLGetChildrenNode (devicePrintModesNode));
   std::ofstream       *pofstreamHPP        = 0;
   std::ofstream       *pofstreamCPP        = 0;
   PrintModeMapElement  mapElement;
   DeviceIDList         mapDeviceID;
   int                  rc                  = false;

   if (devicePrintModeNode == 0)
      goto done;

   while (devicePrintModeNode != 0)
   {
      XmlNodePtr  nodeItem       = XMLFirstNode (XMLGetChildrenNode (devicePrintModeNode));
      const char *pszId          = 0;
      const char *pszDeviceID    = 0;
      int         iPhysicalCount;
      int         iLogicalCount;
      int         iPlanes;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "printmode=" << const_cast<char*>(pszId);

         if (!DevicePrintMode::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DevicePrintMode \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }

      pdi_d->listPrintModes.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      iPhysicalCount = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iLogicalCount = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iPlanes = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"printmode="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"printmode="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << iPhysicalCount
          << ", "
          << iLogicalCount
          << ", "
          << iPlanes
          << " }";

      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId          = " << pszId          << std::endl;
         std::cerr << "iPhysicalCount = " << iPhysicalCount << std::endl;
         std::cerr << "iLogicalCount  = " << iLogicalCount  << std::endl;
         std::cerr << "iPlanes        = " << iPlanes        << std::endl;
      }
#endif

      // Get the next devicePrintMode node
      devicePrintModeNode = XMLNextNode (devicePrintModeNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listPrintModes.sort ();
   pdi_d->listPrintModes.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DevicePrintMode.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DevicePrintMode" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO      pszJobProperties," << std::endl;
      *pofstreamHPP << "                          int         iPhysicalCount," << std::endl;
      *pofstreamHPP << "                          int         iLogicalCount," << std::endl;
      *pofstreamHPP << "                          int         iPlanes);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DevicePrintMode        *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DevicePrintMode *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                    isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO          getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration    *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                          int         iPhysicalCount," << std::endl;
      *pofstreamCPP << "                          int         iLogicalCount," << std::endl;
      *pofstreamCPP << "                          int         iPlanes)" << std::endl;
      *pofstreamCPP << "      : DevicePrintMode (pDevice," << std::endl;
      *pofstreamCPP << "                         pszJobProperties," << std::endl;
      *pofstreamCPP << "                         iPhysicalCount," << std::endl;
      *pofstreamCPP << "                         iLogicalCount," << std::endl;
      *pofstreamCPP << "                         iPlanes)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszPrintMode)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszPrintMode" << std::endl;
         *pofstreamCPP << "      || !*pszPrintMode" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszPrintMode, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszPrintMode, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszPrintMode_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   int    iPhysicalCount;" << std::endl;
      *pofstreamCPP << "   int    iLogicalCount;" << std::endl;
      *pofstreamCPP << "   int    iPlanes;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( PrintModeMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszPrintModeName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszPrintModeName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszPrintModeName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DevicePrintMode * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"printmode\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "aTable[iIndex].iPhysicalCount, "
                    << "aTable[iIndex].iLogicalCount, "
                    << "aTable[iIndex].iPlanes);"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DevicePrintMode * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"printmode\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class PrintModeEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      PrintModeEnumerator (Device *pDevice, int cPrintModes, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iPrintMode_d        = 0;" << std::endl;
      *pofstreamCPP << "         cPrintModes_d       = cPrintModes;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iPrintMode_d < cPrintModes_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iPrintMode_d > cPrintModes_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iPrintMode_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iPrintMode_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iPrintMode_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iPrintMode_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iPrintMode_d;" << std::endl;
      *pofstreamCPP << "      int           cPrintModes_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new PrintModeEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (devicePrintModesNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringPrintModeClassName = 0;
   }

   for ( PrintModeMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceResolutions (XmlNodePtr deviceResolutionsNode)
{
   if (deviceResolutionsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringResolutionClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> ResolutionMapElement;
   typedef std::map <std::string, BinaryData *>  ResolutionMapData1;
   typedef std::map <std::string, byte *>        ResolutionMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr           deviceResolutionNode = XMLFirstNode (XMLGetChildrenNode (deviceResolutionsNode));
   std::ofstream       *pofstreamHPP         = 0;
   std::ofstream       *pofstreamCPP         = 0;
   ResolutionMapElement mapResolution;
   ResolutionMapData1   mapData1;
   ResolutionMapData2   mapData2;
   DeviceIDList         mapDeviceID;
   int                  rc                   = false;

   if (deviceResolutionNode == 0)
      goto done;

   while (deviceResolutionNode != 0)
   {
      XmlNodePtr  nodeItem               = XMLFirstNode (XMLGetChildrenNode (deviceResolutionNode));
      const char *pszId                  = 0;
      int         iXRes;
      int         iYRes;
      int         iXInternalRes          = 0;
      int         iYInternalRes          = 0;
      const char *pszBinaryData          = 0;
      const char *pszDeviceID            = 0;
      byte       *pbData                 = 0;
      int         cbData                 = 0;
      BinaryData *pbdData                = 0;
      int         iCapabilities;
      int         iDestinationBitsPerPel;
      int         iScanlineMultiple;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "Resolution=" << const_cast<char*>(pszId);

         if (!DeviceResolution::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceResolution \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }

      pdi_d->listResolutions.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      iXRes = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iYRes = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "xInternalRes"))
      {
         iXInternalRes = bundleIntegerData (nodeItem);

         // Move to the next one
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;

         iYInternalRes = bundleIntegerData (nodeItem);

         // Move to the next one
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      pszBinaryData = bundleStringData (nodeItem);
      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iCapabilities = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iDestinationBitsPerPel = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iScanlineMultiple = bundleIntegerData (nodeItem);

      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"Resolution="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Resolution="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << iXInternalRes
          << ", "
          << iYInternalRes
          << ", abData"
          << pszId
          << ", "
          << cbData
          << ", "
          << iCapabilities
          << ", "
          << iDestinationBitsPerPel
          << ", "
          << iScanlineMultiple
          << " }";

      mapData2[std::string (pszId)]      = pbData;
      mapData1[std::string (pszId)]      = pbdData;
      mapResolution[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                  = " << pszId                  << std::endl;
         std::cerr << "iXRes                  = " << iXRes                  << std::endl;
         std::cerr << "iYRes                  = " << iYRes                  << std::endl;
         std::cerr << "iXInternalRes          = " << iXInternalRes          << std::endl;
         std::cerr << "iYInternalRes          = " << iYInternalRes          << std::endl;
         std::cerr << "pszBinaryData          = " << pszBinaryData          << std::endl;
         std::cerr << "pbdData                = " << *pbdData               << std::endl;
         std::cerr << "iCapabilities          = " << iCapabilities          << std::endl;
         std::cerr << "iDestinationBitsPerPel = " << iDestinationBitsPerPel << std::endl;
         std::cerr << "iScanlineMultiple      = " << iScanlineMultiple      << std::endl;
      }
#endif

      // Move to the next deviceResolution node
      deviceResolutionNode = XMLNextNode (deviceResolutionNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listResolutions.sort ();
   pdi_d->listResolutions.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceResolution.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceResolution" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO      pszJobProperties," << std::endl;
      *pofstreamHPP << "                          int         iXInternalRes," << std::endl;
      *pofstreamHPP << "                          int         iYInternalRes," << std::endl;
      *pofstreamHPP << "                          BinaryData *data," << std::endl;
      *pofstreamHPP << "                          int         iCapabilities," << std::endl;
      *pofstreamHPP << "                          int         iDestinationBitsPerPel," << std::endl;
      *pofstreamHPP << "                          int         iScanlineMultiple);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceResolution        *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceResolution *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                     isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO           getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration     *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                          int         iXInternalRes," << std::endl;
      *pofstreamCPP << "                          int         iYInternalRes," << std::endl;
      *pofstreamCPP << "                          BinaryData *data," << std::endl;
      *pofstreamCPP << "                          int         iCapabilities," << std::endl;
      *pofstreamCPP << "                          int         iDestinationBitsPerPel," << std::endl;
      *pofstreamCPP << "                          int         iScanlineMultiple)" << std::endl;
      *pofstreamCPP << "      : DeviceResolution (pDevice," << std::endl;
      *pofstreamCPP << "                          pszJobProperties," << std::endl;
      *pofstreamCPP << "                          iXInternalRes," << std::endl;
      *pofstreamCPP << "                          iYInternalRes," << std::endl;
      *pofstreamCPP << "                          data," << std::endl;
      *pofstreamCPP << "                          iCapabilities," << std::endl;
      *pofstreamCPP << "                          iDestinationBitsPerPel," << std::endl;
      *pofstreamCPP << "                          iScanlineMultiple)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszResolution)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszResolution" << std::endl;
         *pofstreamCPP << "      || !*pszResolution" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszResolution, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszResolution, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszResolution_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( ResolutionMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   int    iXInternalRes;" << std::endl;
      *pofstreamCPP << "   int    iYInternalRes;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    iCapabilities;" << std::endl;
      *pofstreamCPP << "   int    iDestinationBitsPerPel;" << std::endl;
      *pofstreamCPP << "   int    iScanLineMultiple;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( ResolutionMapElement::iterator next = mapResolution.begin () ;
            next != mapResolution.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapResolution.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszResolutionName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszResolutionName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszResolutionName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceResolution * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Resolution\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "aTable[iIndex].iXInternalRes, "
                    << "aTable[iIndex].iYInternalRes, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData), "
                    << "aTable[iIndex].iCapabilities, "
                    << "aTable[iIndex].iDestinationBitsPerPel, "
                    << "aTable[iIndex].iScanLineMultiple);"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceResolution * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Resolution\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class ResolutionEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      ResolutionEnumerator (Device *pDevice, int cResolutions, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iResolution_d       = 0;" << std::endl;
      *pofstreamCPP << "         cResolutions_d      = cResolutions;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iResolution_d < cResolutions_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iResolution_d > cResolutions_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iResolution_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iResolution_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iResolution_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iResolution_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iResolution_d;" << std::endl;
      *pofstreamCPP << "      int           cResolutions_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new ResolutionEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceResolutionsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringResolutionClassName = 0;
   }

   for ( ResolutionMapElement::iterator next = mapResolution.begin () ;
         next != mapResolution.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( ResolutionMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( ResolutionMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceScalings (XmlNodePtr deviceScalingsNode)
{
#ifdef INCLUDE_JP_COMMON_SCALING
   if (deviceScalingsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringScalingClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> ScalingMapElement;
   typedef std::map <std::string, BinaryData *>  ScalingMapData1;
   typedef std::map <std::string, byte *>        ScalingMapData2;
   typedef std::list <std::string>               ScalingList;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr         deviceScalingNode = XMLFirstNode (XMLGetChildrenNode (deviceScalingsNode));
   std::ofstream     *pofstreamHPP      = 0;
   std::ofstream     *pofstreamCPP      = 0;
   ScalingMapElement  mapElement;
   ScalingMapData1    mapData1;
   ScalingMapData2    mapData2;
   DeviceIDList       mapDeviceID;
   int                rc                = false;

   if (deviceScalingNode == 0)
      goto done;

   while (deviceScalingNode != 0)
   {
      XmlNodePtr  nodeItem       = XMLFirstNode (XMLGetChildrenNode (deviceScalingNode));
      int         iDefault       = 0;
      double      dMinimum       = 0;
      double      dMaximum       = 0;
      const char *pszAllowedType = 0;
      const char *pszBinaryData  = 0;
      const char *pszDeviceID    = 0;
      byte       *pbData         = 0;
      int         cbData         = 0;
      BinaryData *pbdData        = 0;
      std::string stringShortId;

      if (nodeItem == 0)
         goto done;

      iDefault = bundleIntegerData (nodeItem);

      if (!iDefault)
      {
         std::cerr << "Error: default scaling = 0" << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      dMinimum = bundleDoubleData (nodeItem);

      if (0.0 == dMinimum)
      {
         std::cerr << "Error: minimum scaling = 0" << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      dMaximum = bundleDoubleData (nodeItem);

      if (0.0 == dMaximum)
      {
         std::cerr << "Error: maximum scaling = 0" << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << iDefault
                   << ", "
                   << dMinimum
                   << ", "
                   << dMaximum
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszAllowedType = bundleStringData (nodeItem);

      if (-1 == DeviceScaling::allowedTypeIndex (pszAllowedType))
      {
         std::cerr << "Error: " << pszAllowedType << " is not a correct word." << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         std::ostringstream  oss;

         pszDeviceID = bundleStringData (nodeItem);

         if (pszDeviceID)
         {
            oss << "(0 == strcmp (pszScalingType_d, \""
                << pszAllowedType
                << "\")) && (dMinimumScale_d == "
                << dMinimum
                << ") && (dMaximumScale_d == "
                << dMaximum
                << ")";

            mapDeviceID[std::string (pszDeviceID)] = oss.str ();
         }
      }

      std::ostringstream oss;
      std::ostringstream oss2;

      oss << pszAllowedType
          << "_"
          << dMinimum
          << "_"
          << dMaximum;

      stringShortId = oss.str ();

      oss.str ("");
      oss << "ScalingType=" << pszAllowedType
          << " ScalingPercentage=" << iDefault;

      if (!DeviceScaling::isValid (oss.str ().c_str ()))
      {
         std::cerr << "Error: Unknown DeviceScaling \"" << oss.str () << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      oss.str ("");

      oss << "   { \""
          << pszAllowedType
          << "\", \"ScalingType="
          << pszAllowedType
          << " ScalingPercentage={"
          << iDefault
          << ","
          << dMinimum
          << ","
          << dMaximum
          << "}\", ";
      if (pszDeviceID)
      {
         oss << "\"Scaling="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << iDefault
          << ", "
          << dMinimum
          << ", "
          << dMaximum
          << ", abData"
          << stringShortId
          << ", "
          << cbData
          << " }";

      mapData2[stringShortId]   = pbData;
      mapData1[stringShortId]   = pbdData;
      mapElement[stringShortId] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "stringShortId   = " << stringShortId   << std::endl;
         std::cerr << "pszBinaryData   = " << pszBinaryData   << std::endl;
         std::cerr << "pbdData         = " << *pbdData        << std::endl;
         std::cerr << "iDefault        = " << iDefault        << std::endl;
         std::cerr << "dMinimum        = " << dMinimum        << std::endl;
         std::cerr << "dMaximum        = " << dMaximum        << std::endl;
      }
#endif

      // Get the next deviceScaling node
      deviceScalingNode = XMLNextNode (deviceScalingNode);

      if (pszAllowedType)
      {
         free ((void *)pszAllowedType);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceScaling.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceScaling" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamHPP << "                          double       dMinimumScale," << std::endl;
      *pofstreamHPP << "                          double       dMaximumScale);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceScaling        *create          (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceScaling *createS         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                  isSupported     (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO        getDeviceID     ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration  *getEnumeration  (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamCPP << "                          double       dMinimumScale," << std::endl;
      *pofstreamCPP << "                          double       dMaximumScale)" << std::endl;
      *pofstreamCPP << "      : DeviceScaling (pDevice," << std::endl;
      *pofstreamCPP << "                    pszJobProperties," << std::endl;
      *pofstreamCPP << "                    pbdData," << std::endl;
      *pofstreamCPP << "                    dMinimumScale," << std::endl;
      *pofstreamCPP << "                    dMaximumScale)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->first << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( ScalingMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszAllowedType;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   int    iDefault;" << std::endl;
      *pofstreamCPP << "   double dMinimum;" << std::endl;
      *pofstreamCPP << "   double dMaximum;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( ScalingMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszScalingType);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszScalingType)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   for (int i = 0; i < (int)dimof (aTable); i++)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszScalingType, aTable[i].pszAllowedType))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return i;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceScaling * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   PSZRO                  pszScalingType             = 0;" << std::endl;
      *pofstreamCPP << "   double                 dScalingPercentage         = 0.0;" << std::endl;
      *pofstreamCPP << "   std::ostringstream     oss;" << std::endl;
      *pofstreamCPP << "   DeviceScaling         *pDScalingRet               = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"ScalingType\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (pszScalingType)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            free ((void *)pszScalingType);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         pszScalingType = (PSZRO)malloc (strlen (pszValue) + 1);" << std::endl;
      *pofstreamCPP << "         if (pszScalingType)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            strcpy ((char *)pszScalingType, pszValue);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 == strcmp (pszKey, \"ScalingPercentage\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         sscanf (pszValue, \"%lf\", &dScalingPercentage);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pszScalingType)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pszScalingType);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (-1 != iIndex)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (0.0 == dScalingPercentage)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            dScalingPercentage = aTable[iIndex].iDefault;" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         oss << \"ScalingType=\"" << std::endl;
      *pofstreamCPP << "             << pszScalingType" << std::endl;
      *pofstreamCPP << "             << \" ScalingPercentage={\"" << std::endl;
      *pofstreamCPP << "             << dScalingPercentage" << std::endl;
      *pofstreamCPP << "             << \",\"" << std::endl;
      *pofstreamCPP << "             << aTable[iIndex].dMinimum" << std::endl;
      *pofstreamCPP << "             << \",\"" << std::endl;
      *pofstreamCPP << "             << aTable[iIndex].dMaximum" << std::endl;
      *pofstreamCPP << "             << \"}\";" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         pDScalingRet = new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "oss.str ().c_str (), "
                    << " new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData), "
                    << "aTable[iIndex].dMinimum, "
                    << "aTable[iIndex].dMaximum);"
                    << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return pDScalingRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceScaling * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static std::string *" << std::endl;
      *pofstreamCPP << "getOurProperties (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   std::ostringstream     oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (  0 == strcmp (pszKey, \"ScalingType\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"ScalingPercentage\")" << std::endl;
      *pofstreamCPP << "         )" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (oss.str ().length ())" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            oss << \" \";" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "         oss << pszKey << \"=\" << pszValue;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new std::string (oss.str ());" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int          iIndex    = -1;" << std::endl;
      *pofstreamCPP << "   std::string *pstringJP = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringJP = getOurProperties (pszJobProperties);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pstringJP)" << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pstringJP->c_str ());" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pstringJP;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return true;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   else" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return false;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class ScalingEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      ScalingEnumerator (int cScalings, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         iScaling_d          = 0;" << std::endl;
      *pofstreamCPP << "         cScalings_d         = cScalings;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iScaling_d < cScalings_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iScaling_d > cScalings_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iScaling_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iScaling_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iScaling_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iScaling_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      int           iScaling_d;" << std::endl;
      *pofstreamCPP << "      int           cScalings_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "      std::string   stringReturn_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new ScalingEnumerator (dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceScalingsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringScalingClassName = 0;
   }

   for ( ScalingMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( ScalingMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( ScalingMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceSheetCollates (XmlNodePtr deviceSheetCollatesNode)
{
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (deviceSheetCollatesNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringSheetCollateClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> SheetCollateMapElement;
   typedef std::map <std::string, BinaryData *>  SheetCollateMapData1;
   typedef std::map <std::string, byte *>        SheetCollateMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr              deviceSheetCollateNode = XMLFirstNode (XMLGetChildrenNode (deviceSheetCollatesNode));
   std::ofstream          *pofstreamHPP        = 0;
   std::ofstream          *pofstreamCPP        = 0;
   SheetCollateMapElement  mapElement;
   SheetCollateMapData1    mapData1;
   SheetCollateMapData2    mapData2;
   DeviceIDList            mapDeviceID;
   int                     rc                  = false;

   if (deviceSheetCollateNode == 0)
      goto done;

   while (deviceSheetCollateNode != 0)
   {
      XmlNodePtr  nodeItem      = XMLFirstNode (XMLGetChildrenNode (deviceSheetCollateNode));
      const char *pszId         = 0;
      const char *pszBinaryData = 0;
      const char *pszDeviceID   = 0;
      byte       *pbData        = 0;
      int         cbData        = 0;
      BinaryData *pbdData       = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "SheetCollate=" << const_cast<char*>(pszId);

         if (!DeviceSheetCollate::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceSheetCollate \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceSheetCollate." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pdi_d->listSheetCollates.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"SheetCollate="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"SheetCollate="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", abData"
          << pszId
          << ", "
          << cbData
          << "}";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszId)]   = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                = " << pszId                << std::endl;
         std::cerr << "pszBinaryData        = " << pszBinaryData        << std::endl;
         std::cerr << "pbdData              = " << *pbdData             << std::endl;
      }
#endif

      // Get the next deviceSheetCollate node
      deviceSheetCollateNode = XMLNextNode (deviceSheetCollateNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listSheetCollates.sort ();
   pdi_d->listSheetCollates.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceSheetCollate.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceSheetCollate" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceSheetCollate        *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceSheetCollate *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                       isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO             getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration       *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                     PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                     BinaryData *pbdData)" << std::endl;
      *pofstreamCPP << "      : DeviceSheetCollate (pDevice," << std::endl;
      *pofstreamCPP << "                         pszJobProperties," << std::endl;
      *pofstreamCPP << "                         pbdData)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszSheetCollate)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszSheetCollate" << std::endl;
         *pofstreamCPP << "      || !*pszSheetCollate" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszSheetCollate, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszSheetCollate, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszSheetCollate_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( SheetCollateMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;
      for ( SheetCollateMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszSheelCollateName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszSheetCollateName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszSheetCollateName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceSheetCollate * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"SheetCollate\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData));"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceSheetCollate * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"SheetCollate\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class SheetCollateEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      SheetCollateEnumerator (Device *pDevice, int cSheetCollates, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iSheetCollate_d     = 0;" << std::endl;
      *pofstreamCPP << "         cSheetCollates_d    = cSheetCollates;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iSheetCollate_d < cSheetCollates_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iSheetCollate_d > cSheetCollates_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iSheetCollate_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iSheetCollate_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iSheetCollate_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iSheetCollate_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iSheetCollate_d;" << std::endl;
      *pofstreamCPP << "      int           cSheetCollates_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new SheetCollateEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceSheetCollatesNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringSheetCollateClassName = 0;
   }

   for ( SheetCollateMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( SheetCollateMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( SheetCollateMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceSides (XmlNodePtr deviceSidesNode)
{
#ifdef INCLUDE_JP_COMMON_SIDE
   if (deviceSidesNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringSideClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> SideMapElement;
   typedef std::map <std::string, BinaryData *>  SideMapData1;
   typedef std::map <std::string, byte *>        SideMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr      deviceSideNode = XMLFirstNode (XMLGetChildrenNode (deviceSidesNode));
   std::ofstream  *pofstreamHPP   = 0;
   std::ofstream  *pofstreamCPP   = 0;
   SideMapElement  mapElement;
   SideMapData1    mapData1;
   SideMapData2    mapData2;
   DeviceIDList    mapDeviceID;
   int             rc             = false;

   if (deviceSideNode == 0)
      goto done;

   while (deviceSideNode != 0)
   {
      XmlNodePtr  nodeItem            = XMLFirstNode (XMLGetChildrenNode (deviceSideNode));
      const char *pszId               = 0;
      const char *pszBinaryData       = 0;
      byte       *pbData              = 0;
      int         cbData              = 0;
      BinaryData *pbdData             = 0;
      bool        fSimulationRequired = false;
      const char *pszDeviceID         = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "Sides=" << const_cast<char*>(pszId);

         if (!DeviceSide::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceSide \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceSide." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pdi_d->listSides.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      fSimulationRequired = bundleBooleanData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"Sides="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Sides="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", "
          << fSimulationRequired
          << ", abData"
          << pszId
          << ", "
          << cbData
          << "}";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszId)]   = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                = " << pszId                << std::endl;
         std::cerr << "pszBinaryData        = " << pszBinaryData        << std::endl;
         std::cerr << "pbdData              = " << *pbdData             << std::endl;
         std::cerr << "fSimulationRequired  = " << fSimulationRequired  << std::endl;
      }
#endif

      // Get the next deviceSide node
      deviceSideNode = XMLNextNode (deviceSideNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listSides.sort ();
   pdi_d->listSides.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceSide.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceSide" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData," << std::endl;
      *pofstreamHPP << "                          bool         fSimulationRequired);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceSide          *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceSide   *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO       getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                     PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                     BinaryData *pbdData," << std::endl;
      *pofstreamCPP << "                     bool        fSimulationRequired)" << std::endl;
      *pofstreamCPP << "      : DeviceSide (pDevice," << std::endl;
      *pofstreamCPP << "                         pszJobProperties," << std::endl;
      *pofstreamCPP << "                         pbdData," << std::endl;
      *pofstreamCPP << "                         fSimulationRequired)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszSide)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszSide" << std::endl;
         *pofstreamCPP << "      || !*pszSide" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszSide, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszSide, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszSide_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( SideMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   bool   fSimulationRequired;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;
      for ( SideMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszSideName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszSideName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszSideName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceSide * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Sides\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData),"
                    << "aTable[iIndex].fSimulationRequired);"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceSide * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Sides\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class SideEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      SideEnumerator (Device *pDevice, int cSides, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iSide_d             = 0;" << std::endl;
      *pofstreamCPP << "         cSides_d            = cSides;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iSide_d < cSides_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iSide_d > cSides_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iSide_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iSide_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iSide_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iSide_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iSide_d;" << std::endl;
      *pofstreamCPP << "      int           cSides_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new SideEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceSidesNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringSideClassName = 0;
   }

   for ( SideMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( SideMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( SideMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceStitchings (XmlNodePtr deviceStitchingsNode)
{
#ifdef INCLUDE_JP_COMMON_STITCHING
   if (deviceStitchingsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringStitchingClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> StitchingMapElement;
   typedef std::map <std::string, BinaryData *>  StitchingMapData1;
   typedef std::map <std::string, byte *>        StitchingMapData2;
   typedef std::list <std::string>               StitchingList;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr           deviceStitchingNode = XMLFirstNode (XMLGetChildrenNode (deviceStitchingsNode));
   std::ofstream       *pofstreamHPP        = 0;
   std::ofstream       *pofstreamCPP        = 0;
   StitchingMapElement  mapElement;
   StitchingMapData1    mapData1;
   StitchingMapData2    mapData2;
   DeviceIDList         mapDeviceID;
   int                  rc                  = false;

   if (deviceStitchingNode == 0)
      goto done;

   while (deviceStitchingNode != 0)
   {
      XmlNodePtr  nodeItem           = XMLFirstNode (XMLGetChildrenNode (deviceStitchingNode));
      int         iPosition          = 0;
      const char *pszReferenceEdge   = 0;
      const char *pszType            = 0;
      int         iCount             = 0;
      int         iAngle             = 0;
      const char *pszBinaryData      = 0;
      const char *pszDeviceID        = 0;
      byte       *pbData             = 0;
      int         cbData             = 0;
      BinaryData *pbdData            = 0;
      std::string stringShortId;

      if (nodeItem == 0)
         goto done;

      iPosition = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszReferenceEdge = bundleStringData (nodeItem);

      if (-1 == DeviceStitching::referenceEdgeIndex (pszReferenceEdge))
      {
         std::cerr << "Error: " << pszReferenceEdge << " is not a correct word." << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszType = bundleStringData (nodeItem);

      if (-1 == DeviceStitching::typeIndex (pszType))
      {
         std::cerr << "Error: " << pszType << " is not a correct word." << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iCount = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iAngle = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszReferenceEdge
                   << ", "
                   << pszType
                   << ", "
                   << iCount
                   << ", "
                   << iAngle
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         std::ostringstream  oss;

         pszDeviceID = bundleStringData (nodeItem);

         if (pszDeviceID)
         {
            oss << "(0 == strcmp (pszStitchingReferenceEdge_d, \""
                << pszReferenceEdge
                << "\")) && (0 == strcmp (pszStitchingType_d, \""
                << pszType
                << "\")) && (iStitchingPosition_d == "
                << iPosition
                << ") && (iStitchingCount_d == "
                << iCount
                << ") && (iStitchingAngle_d == "
                << iAngle
                << ")";

            mapDeviceID[std::string (pszDeviceID)] = oss.str ();
         }
      }

      std::ostringstream oss;
      std::ostringstream oss2;

      oss << pszReferenceEdge
          << "_"
          << pszType
          << "_"
          << iPosition
          << "_"
          << iCount
          << "_"
          << iAngle;

      stringShortId = oss.str ();

      oss.str ("");
      oss << "StitchingPosition=" << iPosition
          << " StitchingReferenceEdge=" << pszReferenceEdge
          << " StitchingType=" << pszType
          << " StitchingCount=" << iCount
          << " StitchingAngle=" << iAngle;

      if (!DeviceStitching::isValid (oss.str ().c_str ()))
      {
         std::cerr << "Error: Unknown DeviceStitching \"" << oss.str () << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      oss.str ("");

      oss2 << "StitchingPosition=" << iPosition
           << " StitchingReferenceEdge=" << pszReferenceEdge
           << " StitchingType=" << pszType
           << " StitchingCount=" << iCount
           << " StitchingAngle=" << iAngle;

      oss << "   { \"";

      JobProperties::standarizeJPOrder (oss, oss2.str ());

      oss << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Stitching="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", abData"
          << stringShortId
          << ", "
          << cbData
          << "}";

      mapData2[stringShortId]   = pbData;
      mapData1[stringShortId]   = pbdData;
      mapElement[stringShortId] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "stringShortId   = " << stringShortId   << std::endl;
         std::cerr << "pszBinaryData   = " << pszBinaryData   << std::endl;
         std::cerr << "pbdData         = " << *pbdData        << std::endl;
      }
#endif

      // Get the next deviceStitching node
      deviceStitchingNode = XMLNextNode (deviceStitchingNode);

      if (pszReferenceEdge)
      {
         free ((void *)pszReferenceEdge);
      }
      if (pszType)
      {
         free ((void *)pszType);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceStitching.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceStitching" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceStitching          *create          (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceStitching   *createS         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                      isSupported     (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO            getDeviceID     ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration      *getEnumeration  (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          BinaryData  *pbdData)" << std::endl;
      *pofstreamCPP << "      : DeviceStitching (pDevice," << std::endl;
      *pofstreamCPP << "                    pszJobProperties," << std::endl;
      *pofstreamCPP << "                    pbdData)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if ("
                             << nextDeviceID->second
                             << ")"
                             << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->first << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( StitchingMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( StitchingMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszStitchingName);" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszStitchingName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   for (int i = 0; i < (int)dimof (aTable); i++)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszStitchingName, aTable[i].pszJobProperties))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return i;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static std::string *" << std::endl;
      *pofstreamCPP << "getOurProperties (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   std::ostringstream     oss;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (  0 == strcmp (pszKey, \"StitchingPosition\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"StitchingReferenceEdge\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"StitchingType\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"StitchingCount\")" << std::endl;
      *pofstreamCPP << "         || 0 == strcmp (pszKey, \"StitchingAngle\")" << std::endl;
      *pofstreamCPP << "         )" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (oss.str ().length ())" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            oss << \" \";" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "         oss << pszKey << \"=\" << pszValue;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new std::string (oss.str ());" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceStitching * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int              iIndex         = -1;" << std::endl;
      *pofstreamCPP << "   std::string     *pstringJP      = 0;" << std::endl;
      *pofstreamCPP << "   DeviceStitching *pDStitchingRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringJP = getOurProperties (pszJobProperties);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pstringJP)" << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pstringJP->c_str ());" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      pDStitchingRet = new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData));"
                    << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pstringJP;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return pDStitchingRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceStitching * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int          iIndex    = -1;" << std::endl;
      *pofstreamCPP << "   std::string *pstringJP = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringJP = getOurProperties (pszJobProperties);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pstringJP)" << std::endl;
      *pofstreamCPP << "      iIndex = findTableEntry (pstringJP->c_str ());" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pstringJP;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return true;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   else" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return false;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class StitchingEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      StitchingEnumerator (Device *pDevice, int cStitchings, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iStitching_d        = 0;" << std::endl;
      *pofstreamCPP << "         cStitchings_d       = cStitchings;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iStitching_d < cStitchings_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iStitching_d > cStitchings_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iStitching_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iStitching_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iStitching_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iStitching_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iStitching_d;" << std::endl;
      *pofstreamCPP << "      int           cStitchings_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new StitchingEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceStitchingsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringStitchingClassName = 0;
   }

   for ( StitchingMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( StitchingMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( StitchingMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceTrays (XmlNodePtr deviceTraysNode)
{
   if (deviceTraysNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringTrayClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> TrayMapElement;
   typedef std::map <std::string, BinaryData *>  TrayMapData1;
   typedef std::map <std::string, byte *>        TrayMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr      deviceTrayNode = XMLFirstNode (XMLGetChildrenNode (deviceTraysNode));
   std::ofstream  *pofstreamHPP   = 0;
   std::ofstream  *pofstreamCPP   = 0;
   TrayMapElement  mapElement;
   TrayMapData1    mapData1;
   TrayMapData2    mapData2;
   TrayList        listType;
   DeviceIDList    mapDeviceID;
   int             iCount         = 0;
   int             rc             = false;

   if (deviceTrayNode == 0)
      goto done;

   while (deviceTrayNode != 0)
   {
      XmlNodePtr  nodeItem       = XMLFirstNode (XMLGetChildrenNode (deviceTrayNode));
      const char *pszId          = 0;
      char       *pszCId         = 0;
      const char *pszTrayType    = 0;
      const char *pszBinaryData  = 0;
      const char *pszDeviceID    = 0;
      byte       *pbData         = 0;
      int         cbData         = 0;
      BinaryData *pbdData        = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "InputTray=" << const_cast<char*>(pszId);

         if (!DeviceTray::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceTray \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceTray." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pszCId = (char *)malloc (strlen (pszId) + 1);
      if (pszCId)
      {
         strcpy (pszCId, pszId);
         convertCId (pszCId);
      }

      pdi_d->listTrays.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniName"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
         if (nodeItem == 0)
            goto done;
      }

      pszTrayType = bundleStringData (nodeItem);

      if (!DeviceTray::isReservedKeyword (pszTrayType))
      {
         // @TBD cleanup before exit
      }

      listType.push_back (std::string (pszTrayType));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"InputTray="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"InputTray="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", DeviceTray::"
          << pszTrayType
          << ", abData"
          << pszCId
          << ", "
          << cbData
          << " }";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszCId)]  = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId         = " << pszId         << std::endl;
         std::cerr << "pszTrayType   = " << pszTrayType   << std::endl;
         std::cerr << "pszBinaryData = " << pszBinaryData << std::endl;
         std::cerr << "pbdData       = " << *pbdData      << std::endl;
      }
#endif

      // Get the next deviceTray node
      deviceTrayNode = XMLNextNode (deviceTrayNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszCId)
      {
         free ((void *)pszCId);
      }
      if (pszTrayType)
      {
         free ((void *)pszTrayType);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listTrays.sort ();
   pdi_d->listTrays.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceTray.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceTray" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;

      *pofstreamHPP << "   enum {" << std::endl;
      listType.sort ();
      listType.unique ();
      iCount = listType.size ();
      for ( TrayList::iterator next = listType.begin () ;
            next != listType.end () ;
            next++, iCount-- )
      {
         *pofstreamHPP << "      " << *next;
         if (1 < iCount)
            *pofstreamHPP << ",";
         *pofstreamHPP << std::endl;
      }
      *pofstreamHPP << "   };" << std::endl;

      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO      pszJobProperties," << std::endl;
      *pofstreamHPP << "                          int         iType," << std::endl;
      *pofstreamHPP << "                          BinaryData *data);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceTray          *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceTray   *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                 isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO       getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamCPP << "                          int          iType," << std::endl;
      *pofstreamCPP << "                          BinaryData  *data)" << std::endl;
      *pofstreamCPP << "      : DeviceTray (pDevice," << std::endl;
      *pofstreamCPP << "                    pszJobProperties," << std::endl;
      *pofstreamCPP << "                    iType," << std::endl;
      *pofstreamCPP << "                    data)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszTray)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszTray" << std::endl;
         *pofstreamCPP << "      || !*pszTray" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszTray, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszTray, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszTray_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( TrayMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   int    iType;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;

      for ( TrayMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszTrayName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszTrayName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszTrayName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceTray * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"InputTray\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "aTable[iIndex].iType, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData));"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceTray * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"InputTray\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class TrayEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      TrayEnumerator (Device *pDevice, int cTrays, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iTray_d             = 0;" << std::endl;
      *pofstreamCPP << "         cTrays_d            = cTrays;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iTray_d < cTrays_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iTray_d > cTrays_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iTray_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iTray_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iTray_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iTray_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iTray_d;" << std::endl;
      *pofstreamCPP << "      int           cTrays_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new TrayEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceTraysNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringTrayClassName = 0;
   }

   for ( TrayMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( TrayMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( TrayMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceTrimmings (XmlNodePtr deviceTrimmingsNode)
{
#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (deviceTrimmingsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringTrimmingClassName = pstringNameOut;

   typedef std::map <std::string, std::string *> TrimmingMapElement;
   typedef std::map <std::string, BinaryData *>  TrimmingMapData1;
   typedef std::map <std::string, byte *>        TrimmingMapData2;
   typedef std::map <std::string, std::string>   DeviceIDList;

   XmlNodePtr          deviceTrimmingNode = XMLFirstNode (XMLGetChildrenNode (deviceTrimmingsNode));
   std::ofstream      *pofstreamHPP       = 0;
   std::ofstream      *pofstreamCPP       = 0;
   TrimmingMapElement  mapElement;
   TrimmingMapData1    mapData1;
   TrimmingMapData2    mapData2;
   DeviceIDList        mapDeviceID;
   int                 rc                 = false;

   if (deviceTrimmingNode == 0)
      goto done;

   while (deviceTrimmingNode != 0)
   {
      XmlNodePtr  nodeItem      = XMLFirstNode (XMLGetChildrenNode (deviceTrimmingNode));
      const char *pszId         = 0;
      const char *pszBinaryData = 0;
      const char *pszDeviceID   = 0;
      byte       *pbData        = 0;
      int         cbData        = 0;
      BinaryData *pbdData       = 0;

      if (nodeItem == 0)
         goto done;

      pszId = bundleStringData (nodeItem);
      if (pszId)
      {
         std::ostringstream oss;

         oss << "Trimming=" << const_cast<char*>(pszId);

         if (!DeviceTrimming::isValid (oss.str ().c_str ()))
         {
            std::cerr << "Error: Unknown DeviceTrimming \"" << pszId << "\"" << std::endl;
            setErrorCondition ();
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Missing DeviceTrimming." << std::endl;
         setErrorCondition ();
         goto done;
      }

      pdi_d->listTrimmings.push_back (std::string (pszId));

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      pszBinaryData = bundleStringData (nodeItem);

      if (parseBinaryData (pszBinaryData, &pbData, &cbData))
      {
         pbdData = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszBinaryData
                   << "\" for "
                   << pszId
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);

      if (nodeItem != 0)
      {
         pszDeviceID = bundleStringData (nodeItem);

         mapDeviceID[std::string (pszId)] = std::string (pszDeviceID);
      }

      std::ostringstream oss;

      oss << "   { \""
          << pszId
          << "\", \"Trimming="
          << pszId
          << "\", ";
      if (pszDeviceID)
      {
         oss << "\"Trimming="
             << pszDeviceID
             << "\"";
      }
      else
      {
         oss << "0";
      }
      oss << ", abData"
          << pszId
          << ", "
          << cbData
          << "}";

      mapData2[std::string (pszId)]   = pbData;
      mapData1[std::string (pszId)]   = pbdData;
      mapElement[std::string (pszId)] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszId                = " << pszId                << std::endl;
         std::cerr << "pszBinaryData        = " << pszBinaryData        << std::endl;
         std::cerr << "pbdData              = " << *pbdData             << std::endl;
      }
#endif

      // Get the next deviceTrimming node
      deviceTrimmingNode = XMLNextNode (deviceTrimmingNode);

      if (pszId)
      {
         free ((void *)pszId);
      }
      if (pszBinaryData)
      {
         free ((void *)pszBinaryData);
      }
      if (pszDeviceID)
      {
         free ((void *)pszDeviceID);
      }
   }

   pdi_d->listTrimmings.sort ();
   pdi_d->listTrimmings.unique ();

   rc = true;

   if (shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

      openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

      fFileModified_d = true;

      outputHeader (pofstreamHPP);

      *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
      *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#include <BinaryData.hpp>" << std::endl;
      *pofstreamHPP << "#include <DeviceTrimming.hpp>" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceTrimming" << std::endl;
      *pofstreamHPP << "{" << std::endl;
      *pofstreamHPP << "public:" << std::endl;
      *pofstreamHPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamHPP << "                          PSZCRO       pszJobProperties," << std::endl;
      *pofstreamHPP << "                          BinaryData  *pbdData);" << std::endl;

      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   DeviceTrimming          *create         (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << "   static DeviceTrimming   *createS        (Device *pDevice, PSZCRO pszJobProperties);" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "   bool                     isSupported    (PSZCRO  pszJobProperties);" << std::endl;
      if (0 < mapDeviceID.size ())
      {
         *pofstreamHPP << "   virtual PSZCRO           getDeviceID    ();" << std::endl;
      }
      *pofstreamHPP << "   virtual Enumeration     *getEnumeration (bool    fInDeviceSpecific);" << std::endl;

      *pofstreamHPP << "};" << std::endl;
      *pofstreamHPP << std::endl;
      *pofstreamHPP << "#endif" << std::endl;

      outputHeader (pofstreamCPP);

      *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
      *pofstreamCPP << "#include <JobProperties.hpp>" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << *pstringNameOut << " (Device *pDevice," << std::endl;
      *pofstreamCPP << "                     PSZCRO      pszJobProperties," << std::endl;
      *pofstreamCPP << "                     BinaryData *pbdData)" << std::endl;
      *pofstreamCPP << "      : DeviceTrimming (pDevice," << std::endl;
      *pofstreamCPP << "                         pszJobProperties," << std::endl;
      *pofstreamCPP << "                         pbdData)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      if (0 < mapDeviceID.size ())
      {
         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO" << std::endl;
         *pofstreamCPP << "static findDeviceID (PSZCRO pszTrimming)" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   if (  !pszTrimming" << std::endl;
         *pofstreamCPP << "      || !*pszTrimming" << std::endl;
         *pofstreamCPP << "      )" << std::endl;
         *pofstreamCPP << "    {" << std::endl;
         *pofstreamCPP << "       return 0;" << std::endl;
         *pofstreamCPP << "    }" << std::endl;
         *pofstreamCPP << std::endl;

         for ( DeviceIDList::iterator nextDeviceID = mapDeviceID.begin () ;
               nextDeviceID != mapDeviceID.end () ;
               nextDeviceID++ )
         {
            if (nextDeviceID == mapDeviceID.begin ())
            {
               *pofstreamCPP << "   if (0 == strcmp (pszTrimming, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            else
            {
               *pofstreamCPP << "   else if (0 == strcmp (pszTrimming, \""
                             << nextDeviceID->first
                             << "\"))" << std::endl;
            }
            *pofstreamCPP << "   {" << std::endl;
            *pofstreamCPP << "      return \"" << nextDeviceID->second << "\";" << std::endl;
            *pofstreamCPP << "   }" << std::endl;
         }

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "   return 0;" << std::endl;
         *pofstreamCPP << "}" << std::endl;

         *pofstreamCPP << std::endl;
         *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
         *pofstreamCPP << "getDeviceID ()" << std::endl;
         *pofstreamCPP << "{" << std::endl;
         *pofstreamCPP << "   return findDeviceID (pszTrimming_d);" << std::endl;
         *pofstreamCPP << "}" << std::endl;
      }

      *pofstreamCPP << std::endl;
      for ( TrimmingMapData1::iterator next = mapData1.begin () ;
            next != mapData1.end () ;
            next++ )
      {
         BinaryData *pbdValue = (BinaryData *)next->second;
         PBYTE       pbValue  = pbdValue->getData ();
         int         cbValue  = pbdValue->getLength ();

         *pofstreamCPP << "static byte abData" << next->first << "[] = {";

         for (int i = 0; i < cbValue; i++)
         {
            *pofstreamCPP << (int)pbValue[i];
            if (i < cbValue - 1)
               *pofstreamCPP << ",";
         }

         *pofstreamCPP << "};" << std::endl;
      }

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "typedef struct _MappingTable {" << std::endl;
      *pofstreamCPP << "   PSZCRO pszName;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobProperties;" << std::endl;
      *pofstreamCPP << "   PSZCRO pszJobPropertiesDevice;" << std::endl;
      *pofstreamCPP << "   byte  *pbBinaryData;" << std::endl;
      *pofstreamCPP << "   int    cbBinaryData;" << std::endl;
      *pofstreamCPP << "} MAPPINGTABLE, *PMAPPINGTABLE, **PPMAPPINGTABLE;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static MAPPINGTABLE aTable[] = {" << std::endl;
      for ( TrimmingMapElement::iterator next = mapElement.begin () ;
            next != mapElement.end () ;
          )
      {
         std::string *pElement = (std::string *)next->second;

         *pofstreamCPP << *pElement;

         next++;
         if (next != mapElement.end ())
         {
            *pofstreamCPP << ",";
         }

         *pofstreamCPP << std::endl;
      }
      *pofstreamCPP << "};" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "static int findTableEntry (PSZCRO pszTrimmingName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "int" << std::endl;
      *pofstreamCPP << "findTableEntry (PSZCRO pszTrimmingName)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   int   iLow     = 0;" << std::endl;
      *pofstreamCPP << "   int   iMid     = (int)dimof (aTable) / 2;" << std::endl;
      *pofstreamCPP << "   int   iHigh    = (int)dimof (aTable) - 1;" << std::endl;
      *pofstreamCPP << "   int   iResult;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (iLow <= iHigh)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      iResult = strcmp (pszTrimmingName, aTable[iMid].pszName);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         return iMid;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 > iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 < str2" << std::endl;
      *pofstreamCPP << "         iHigh = iMid - 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else // (0 < iResult)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         // str1 > str2" << std::endl;
      *pofstreamCPP << "         iLow  = iMid + 1;" << std::endl;
      *pofstreamCPP << "         iMid  = iLow + (iHigh - iLow) / 2;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return -1;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceTrimming * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "createS (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Trimming\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         int iIndex = -1;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iIndex = findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (0 <= iIndex)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            return new "
                    << *pstringNameOut
                    << " (pDevice, "
                    << "aTable[iIndex].pszJobProperties, "
                    << "new BinaryData (aTable[iIndex].pbBinaryData, "
                    << "aTable[iIndex].cbBinaryData));"
                    << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return 0;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "DeviceTrimming * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "create (Device *pDevice, PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   return createS (pDevice, pszJobProperties);" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << " ::" << std::endl;
      *pofstreamCPP << "isSupported (PSZCRO pszJobProperties)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   JobProperties          jobProp (pszJobProperties);" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   bool                   fRet                       = false;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Trimming\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         fRet = 0 <= findTableEntry (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return fRet;" << std::endl;
      *pofstreamCPP << "}" << std::endl;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "Enumeration * " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "getEnumeration (bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "{" << std::endl;
      *pofstreamCPP << "   class TrimmingEnumerator : public Enumeration" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "   public:" << std::endl;
      *pofstreamCPP << "      TrimmingEnumerator (Device *pDevice, int cTrimmings, PMAPPINGTABLE aTable, bool fInDeviceSpecific)" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pDevice_d           = pDevice;" << std::endl;
      *pofstreamCPP << "         iTrimming_d         = 0;" << std::endl;
      *pofstreamCPP << "         cTrimmings_d        = cTrimmings;" << std::endl;
      *pofstreamCPP << "         aTable_d            = aTable;" << std::endl;
      *pofstreamCPP << "         fInDeviceSpecific_d = fInDeviceSpecific;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual bool hasMoreElements ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         if (iTrimming_d < cTrimmings_d)" << std::endl;
      *pofstreamCPP << "            return true;" << std::endl;
      *pofstreamCPP << "         else" << std::endl;
      *pofstreamCPP << "            return false;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      virtual void *nextElement ()" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         void *pvRet = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (iTrimming_d > cTrimmings_d - 1)" << std::endl;
      *pofstreamCPP << "            return 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (fInDeviceSpecific_d)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            if (aTable_d[iTrimming_d].pszJobPropertiesDevice)" << std::endl;
      *pofstreamCPP << "            {" << std::endl;
      *pofstreamCPP << "               pvRet = (void *)new JobProperties (aTable_d[iTrimming_d].pszJobPropertiesDevice);" << std::endl;
      *pofstreamCPP << "            }" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         if (!pvRet)" << std::endl;
      *pofstreamCPP << "         {" << std::endl;
      *pofstreamCPP << "            pvRet = (void *)new JobProperties (aTable_d[iTrimming_d].pszJobProperties);" << std::endl;
      *pofstreamCPP << "         }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         iTrimming_d++;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "         return pvRet;" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   private:" << std::endl;
      *pofstreamCPP << "      Device       *pDevice_d;" << std::endl;
      *pofstreamCPP << "      int           iTrimming_d;" << std::endl;
      *pofstreamCPP << "      int           cTrimmings_d;" << std::endl;
      *pofstreamCPP << "      PMAPPINGTABLE aTable_d;" << std::endl;
      *pofstreamCPP << "      bool          fInDeviceSpecific_d;" << std::endl;
      *pofstreamCPP << "   };" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return new TrimmingEnumerator (pDevice_d, dimof (aTable), aTable, fInDeviceSpecific);" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceTrimmingsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringTrimmingClassName = 0;
   }

   for ( TrimmingMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( TrimmingMapData1::iterator next = mapData1.begin () ;
         next != mapData1.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( TrimmingMapData2::iterator next = mapData2.begin () ;
         next != mapData2.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
#else
   return true;
#endif
}

bool OmniDomParser::
processDeviceConnections (XmlNodePtr deviceConnectionsNode)
{
   if (deviceConnectionsNode == 0)
      return false;

   XmlNodePtr deviceConnectionNode = XMLFirstNode (XMLGetChildrenNode (deviceConnectionsNode));
   int        rc                   = false;

   if (deviceConnectionNode == 0)
      goto done;

   while (deviceConnectionNode != 0)
   {
      XmlNodePtr nodeItem = XMLFirstNode (XMLGetChildrenNode (deviceConnectionNode));

      if (nodeItem == 0)
         goto done;

      // name

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      // form

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      // tray

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      // media

      // Get the next deviceConnection node
      deviceConnectionNode = XMLNextNode (deviceConnectionNode);
   }

   rc = true;

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceConnectionsNode), 0);
   }

   // Clean up!

   return rc;
}

bool OmniDomParser::
processDeviceGammas (XmlNodePtr deviceGammasNode)
{
   if (deviceGammasNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringGammaClassName = pstringNameOut;

   if (!shouldCreateFile (*pstrXMLFile_d))
      return true;

   if (!pdi_d->pstringResolutionClassName)
   {
      std::cerr << "Error: DeviceResolution not filled in." << std::endl;
      return false;
   }

   if (!pdi_d->pstringMediaClassName)
   {
      std::cerr << "Error: DeviceMedia not filled in." << std::endl;
      return false;
   }

   if (!pdi_d->pstringPrintModeClassName)
   {
      std::cerr << "Error: DevicePrintMode not filled in." << std::endl;
      return false;
   }

   std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

   typedef std::map <std::string, std::string *> GammaMapElement;

   XmlNodePtr      deviceGammaNode = XMLFirstNode (XMLGetChildrenNode (deviceGammasNode));
   std::ofstream  *pofstreamHPP    = 0;
   std::ofstream  *pofstreamCPP    = 0;
   GammaMapElement mapElement;
   int             iCount          = 0;
   int             rc              = false;

   if (deviceGammaNode == 0)
      goto done;

   while (deviceGammaNode != 0)
   {
      XmlNodePtr          nodeItem            = XMLFirstNode (XMLGetChildrenNode (deviceGammaNode));
      const char         *pszResolutionID     = 0;
      const char         *pszMediaID          = 0;
      const char         *pszPrintModeID      = 0;
      const char         *pszDitherCatagoryID = 0;
      int                 iCGamma;
      int                 iMGamma;
      int                 iYGamma;
      int                 iKGamma;
      int                 iCBias;
      int                 iMBias;
      int                 iYBias;
      int                 iKBias;
      std::ostringstream  oss;

      if (nodeItem == 0)
         goto done;

      pszResolutionID = bundleStringData (nodeItem);
      oss.str ("");
      if (pszResolutionID)
         oss << "Resolution=" << const_cast<char*>(pszResolutionID);

      if (  !pszResolutionID
         || !DeviceResolution::isValid (oss.str ().c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceResolution \"" << pszResolutionID << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniResolution"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
      }

      pszMediaID = bundleStringData (nodeItem);
      oss.str ("");
      if (pszMediaID)
         oss << "media=" << const_cast<char*>(pszMediaID);

      if (  !pszMediaID
         || !DeviceMedia::isValid (oss.str ().c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceMedia \"" << pszMediaID << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniMedia"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
      }
      pszPrintModeID = bundleStringData (nodeItem);
      oss.str ("");
      if (pszPrintModeID)
         oss << "printmode=" << const_cast<char*>(pszPrintModeID);

      if (  !pszPrintModeID
         || !DevicePrintMode::isValid (oss.str ().c_str ())
         )
      {
         std::cerr << "Error: Unknown DevicePrintMode \"" << pszPrintModeID << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 == strcmp (XMLGetName (nodeItem), "omniPrintmode"))
      {
         // Move to the next node
         nodeItem = XMLNextNode (nodeItem);
      }
      if (  !(pszDitherCatagoryID = bundleStringData (nodeItem))
         || !DeviceDither::ditherCatagoryValid (const_cast<char*>(pszDitherCatagoryID))
         )
      {
         std::cerr << "Error: Unknown Dither type \"" << pszDitherCatagoryID << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iCGamma = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iMGamma = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iYGamma = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iKGamma = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iCBias = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iMBias = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iYBias = bundleIntegerData (nodeItem);

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      iKBias = bundleIntegerData (nodeItem);

      oss.str ("");

      oss << "   "
          << "{\""
          << pszResolutionID
          << "\",\""
          << pszMediaID
          << "\",\""
          << pszPrintModeID
          << "\",\""
          << pszDitherCatagoryID
          << "\","
          << iCGamma
          << ","
          << iMGamma
          << ","
          << iYGamma
          << ","
          << iKGamma
          << ","
          << iCBias
          << ","
          << iMBias
          << ","
          << iYBias
          << ","
          << iKBias
          << "}";

      std::ostringstream ossName;

      // Create a unique name
      ossName << pszResolutionID
              << "/"
              << pszMediaID
              << "/"
              << pszPrintModeID
              << "/"
              << pszDitherCatagoryID;

      mapElement[std::string (ossName.str ())] = new std::string (oss.str ());

#ifdef DEBUG
      if (fDebugOutput)
      {
         std::cerr << "pszResolutionID     = " << pszResolutionID     << std::endl;
         std::cerr << "pszMediaID          = " << pszMediaID          << std::endl;
         std::cerr << "pszPrintModeID      = " << pszPrintModeID      << std::endl;
         std::cerr << "pszDitherCatagoryID = " << pszDitherCatagoryID << std::endl;
         std::cerr << "iCGamma             = " << iCGamma             << std::endl;
         std::cerr << "iMGamma             = " << iMGamma             << std::endl;
         std::cerr << "iYGamma             = " << iYGamma             << std::endl;
         std::cerr << "iKGamma             = " << iKGamma             << std::endl;
         std::cerr << "iCBias              = " << iCBias              << std::endl;
         std::cerr << "iMBias              = " << iMBias              << std::endl;
         std::cerr << "iYBias              = " << iYBias              << std::endl;
         std::cerr << "iKBias              = " << iKBias              << std::endl;
      }
#endif

      // Get the next deviceGamma node
      deviceGammaNode = XMLNextNode (deviceGammaNode);

      if (pszResolutionID)
      {
         free ((void *)pszResolutionID);
      }
      if (pszMediaID)
      {
         free ((void *)pszMediaID);
      }
      if (pszPrintModeID)
      {
         free ((void *)pszPrintModeID);
      }
      if (pszDitherCatagoryID)
      {
         free ((void *)pszDitherCatagoryID);
      }
   }

   rc = true;

   openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

   fFileModified_d = true;

   outputHeader (pofstreamHPP);

   *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
   *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#include <DeviceGammaTable.hpp>" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceGammaTable" << std::endl;
   *pofstreamHPP << "{" << std::endl;
   *pofstreamHPP << "public:" << std::endl;

   *pofstreamHPP << "static DeviceGamma *getDeviceGamma (PSZCRO iResolutionID," << std::endl;
   *pofstreamHPP << "                                    PSZCRO iMediaID," << std::endl;
   *pofstreamHPP << "                                    PSZCRO iPrintModeID," << std::endl;
   *pofstreamHPP << "                                    PSZCRO pszDitherID);" << std::endl;
   *pofstreamHPP << "};" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#endif" << std::endl;

   outputHeader (pofstreamCPP);

   *pofstreamCPP << "#include <defines.hpp>" << std::endl;
   *pofstreamCPP << "#include <Device.hpp>" << std::endl;
   *pofstreamCPP << "#include <" << *pdi_d->pstringResolutionClassName << ".hpp>" << std::endl;
   *pofstreamCPP << "#include <" << *pdi_d->pstringMediaClassName << ".hpp>" << std::endl;
   *pofstreamCPP << "#include <" << *pdi_d->pstringPrintModeClassName << ".hpp>" << std::endl;
   *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "typedef struct _GAMMATABLE {" << std::endl;
   *pofstreamCPP << "   PSZCRO             pszResolutionID;" << std::endl;
   *pofstreamCPP << "   PSZCRO             pszMediaID;" << std::endl;
   *pofstreamCPP << "   PSZCRO             pszPrintModeID;" << std::endl;
   *pofstreamCPP << "   PSZCRO             pszDitherCatagoryID;" << std::endl;
   *pofstreamCPP << "   unsigned short int usCGamma;" << std::endl;
   *pofstreamCPP << "   unsigned short int usMGamma;" << std::endl;
   *pofstreamCPP << "   unsigned short int usYGamma;" << std::endl;
   *pofstreamCPP << "   unsigned short int usKGamma;" << std::endl;
   *pofstreamCPP << "   unsigned short int usCBias;" << std::endl;
   *pofstreamCPP << "   unsigned short int usMBias;" << std::endl;
   *pofstreamCPP << "   unsigned short int usYBias;" << std::endl;
   *pofstreamCPP << "   unsigned short int usKBias;" << std::endl;
   *pofstreamCPP << "} GAMMATABLE, *PGAMMATABLE;" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "GAMMATABLE gt" << *pstringNameOut << "[] = {" << std::endl;

   iCount = mapElement.size ();
   for ( GammaMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      std::string *pElement = (std::string *)next->second;

      *pofstreamCPP << *pElement;
      if (1 < iCount)
         *pofstreamCPP << ",";
      *pofstreamCPP << std::endl;
   }

   *pofstreamCPP << "};" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceGamma * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDeviceGamma (PSZCRO pszResolutionID," << std::endl;
   *pofstreamCPP << "                PSZCRO pszMediaID," << std::endl;
   *pofstreamCPP << "                PSZCRO pszPrintModeID," << std::endl;
   *pofstreamCPP << "                PSZCRO pszDitherID)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   PSZCRO pszDitherCatagoryID = DeviceDither::getDitherCatagory (pszDitherID);" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   for (int i = 0; i < (int)dimof (gt" << *pstringNameOut << "); i++)" << std::endl;
   *pofstreamCPP << "   {" << std::endl;
   *pofstreamCPP << "      if (  0 == strcmp (pszResolutionID,     gt" << *pstringNameOut << "[i].pszResolutionID)" << std::endl;
   *pofstreamCPP << "         && 0 == strcmp (pszMediaID,          gt" << *pstringNameOut << "[i].pszMediaID)" << std::endl;
   *pofstreamCPP << "         && 0 == strcmp (pszPrintModeID,      gt" << *pstringNameOut << "[i].pszPrintModeID)" << std::endl;
   *pofstreamCPP << "         && 0 == strcmp (pszDitherCatagoryID, gt" << *pstringNameOut << "[i].pszDitherCatagoryID)" << std::endl;
   *pofstreamCPP << "         )" << std::endl;
   *pofstreamCPP << "      {" << std::endl;
   *pofstreamCPP << "         return new DeviceGamma ((int)gt" << *pstringNameOut << "[i].usCGamma," << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usMGamma," << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usYGamma," << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usKGamma," << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usCBias,"  << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usMBias,"  << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usYBias,"  << std::endl;
   *pofstreamCPP << "                                 (int)gt" << *pstringNameOut << "[i].usKBias);" << std::endl;
   *pofstreamCPP << "      }" << std::endl;
   *pofstreamCPP << "   }" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   return 0;" << std::endl;
   *pofstreamCPP << "}" << std::endl;

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceGammasNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringGammaClassName = 0;
   }

   for ( GammaMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceCommands (XmlNodePtr deviceCommandsNode)
{
   if (deviceCommandsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringCommandClassName = pstringNameOut;

   if (!shouldCreateFile (*pstrXMLFile_d))
      return true;

   std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

   typedef std::map <std::string, BinaryData *> CommandMapElement;
   typedef std::map <std::string, byte *>       CommandMapData;

   XmlNodePtr        deviceCommandNode = XMLFirstNode (XMLGetChildrenNode (deviceCommandsNode));
   std::ofstream    *pofstreamHPP      = 0;
   std::ofstream    *pofstreamCPP      = 0;
   CommandMapElement mapElement;
   CommandMapData    mapData;
   int               rc                = false;

   if (deviceCommandNode == 0)
      goto done;

   while (deviceCommandNode != 0)
   {
      PSZCRO pszCmdName  = XMLGetProp (deviceCommandNode, "name");
      PSZRO  pszCmdValue = 0;
      byte  *pbData      = 0;
      int    cbData      = 0;

      if (pszCmdName == 0)
         goto done;

      pszCmdValue = XMLNodeListGetString (XMLGetDocNode (deviceCommandNode),
                                          XMLGetChildrenNode (deviceCommandNode),
                                          1);

#ifdef DEBUG
      if (fDebugOutput) std::cerr << pszCmdName << " = " << pszCmdValue << std::endl;
#endif

      if (parseBinaryData (pszCmdValue, &pbData, &cbData))
      {
         mapData[std::string (pszCmdName)]    = pbData;
         mapElement[std::string (pszCmdName)] = new BinaryData (pbData, cbData);
      }
      else
      {
         std::cerr << "Error: binary data not understood \""
                   << pszCmdValue
                   << "\" for "
                   << pszCmdName
                   << "."
                   << std::endl;
         goto done;
      }

      // Move to the next one
      deviceCommandNode = XMLNextNode (deviceCommandNode);

      if (pszCmdName)
      {
         free ((void *)pszCmdName);
      }
      if (pszCmdValue)
      {
         free ((void *)pszCmdValue);
      }
   }

   rc = true;

   openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

   fFileModified_d = true;

   outputHeader (pofstreamHPP);

   *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
   *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#include <DeviceCommand.hpp>" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceCommand" << std::endl;
   *pofstreamHPP << "{" << std::endl;
   *pofstreamHPP << "public:" << std::endl;
   *pofstreamHPP << "   " << *pstringNameOut << " ();" << std::endl;
   *pofstreamHPP << "};" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#endif" << std::endl;

   outputHeader (pofstreamCPP);

   *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << *pstringNameOut << "()" << std::endl;
   *pofstreamCPP << "{" << std::endl;

   for ( CommandMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      BinaryData *pbdValue = (BinaryData *)next->second;
      PBYTE       pbValue  = pbdValue->getData ();
      int         cbValue  = pbdValue->getLength ();

      *pofstreamCPP << "   static byte " << next->first << "[] = {";

      for (int i = 0; i < cbValue; i++)
      {
         *pofstreamCPP << (int)pbValue[i];
         if (i < cbValue - 1)
            *pofstreamCPP << ",";
      }

      *pofstreamCPP << "};" << std::endl;

#ifdef DEBUG
//////if (fDebugOutput) std::cerr << next->first << " = " << *(next->second) << std::endl;
#endif
   }

   *pofstreamCPP << std::endl;

   for ( CommandMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      *pofstreamCPP << "   add (\""
                    << next->first
                    << "\", new BinaryData ("
                    << next->first
                    << ", dimof ("
                    << next->first << ")));"
                    << std::endl;
   }

   *pofstreamCPP << "}" << std::endl;

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceCommandsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringCommandClassName = 0;
   }

   for ( CommandMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( CommandMapData::iterator next = mapData.begin () ;
         next != mapData.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceDatas (XmlNodePtr deviceDatasNode)
{
   if (deviceDatasNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringDataClassName = pstringNameOut;

   if (!shouldCreateFile (*pstrXMLFile_d))
      return true;

   std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

   typedef std::map <std::string, BinaryData *> DataMapElement;
   typedef std::map <std::string, byte *>       DataMapData;

   XmlNodePtr      deviceDataNode = XMLFirstNode (XMLGetChildrenNode (deviceDatasNode));
   std::ofstream  *pofstreamHPP   = 0;
   std::ofstream  *pofstreamCPP   = 0;
   DataMapElement  mapElement;
   DataMapData     mapData;
   int             rc             = false;

   if (deviceDataNode == 0)
      goto done;

   while (deviceDataNode != 0)
   {
      const char *pszDataName  = XMLGetProp (deviceDataNode, "name");
      const char *pszDataValue = 0;
      const char *pszTypeName  = XMLGetProp (deviceDataNode, "type");
      byte       *pbData       = 0;
      int         cbData;

      if (pszDataName == 0)
         goto done;

      if (pszTypeName == 0)
         goto done;

      pszDataValue = bundleStringData (deviceDataNode);

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "name  = " << pszDataName  << ", type = " << pszTypeName << std::endl;
      if (fDebugOutput) std::cerr << "value = " << pszDataValue << std::endl;
#endif

      if (0 == strcmp (pszTypeName, "string"))
      {
         cbData = strlen (pszDataValue) + 1;
         pbData = new byte [cbData];

         if (pbData)
         {
            strcpy ((char *)pbData, pszDataValue);
         }
         else
         {
            goto done;
         }
      }
      else if (0 == strcmp (pszTypeName, "boolean"))
      {
         cbData = 4;
         pbData = new byte [cbData];

         if (pbData)
         {
            memset (pbData, 0, cbData);

            if (0 == strcasecmp (pszDataValue, "true"))
            {
               pbData[0] = 1;
            }
            else if (0 == strcasecmp (pszDataValue, "false"))
            {
               pbData[0] = 0;
            }
            else
            {
               delete[] pbData;
               goto done;
            }
         }
         else
         {
            goto done;
         }
      }
      else if (0 == strcmp (pszTypeName, "integer"))
      {
         cbData = 4;
         pbData = new byte [cbData];

         if (pbData)
         {
            if (0 == sscanf (pszDataValue, "%d", (int *)pbData))
            {
               delete[] pbData;
               goto done;
            }
         }
         else
         {
            goto done;
         }
      }
      else if (0 == strcmp (pszTypeName, "byte"))
      {
         cbData = 1;
         pbData = new byte [cbData];

         if (pbData)
         {
            if (0 == sscanf (pszDataValue, "%c", (char *)pbData))
            {
               delete[] pbData;
               goto done;
            }
         }
         else
         {
            goto done;
         }
      }
      else if (0 == strcmp (pszTypeName, "binary"))
      {
         if (!parseBinaryData (pszDataValue, &pbData, &cbData))
         {
            std::cerr << "Error: binary data not understood \""
                      << pszDataValue
                      << "\" for "
                      << pszDataName
                      << "."
                      << std::endl;

            delete[] pbData;
            goto done;
         }
      }
      else if (0 == strcmp (pszTypeName, "bytearray"))
      {
         int iCnt,
             iTempCnt = 0;

         for (iCnt = 0; *(pszDataValue+iCnt); iCnt++)
         {
            if (!isxdigit (*(pszDataValue+iCnt)))
            {
               if (isspace (*(pszDataValue+iCnt)) == 0)
               {
                  std::cerr << "Error: Data in bytearray is not understood \"" << *(pszDataValue+iCnt) << "\" for " << pszDataName << "." << std::endl;
                  goto done;
               }
            }
            else
            {
               iTempCnt++;
            }
         }

         cbData = iTempCnt/2;
         pbData = new byte [cbData];
         if (!pbData)
         {
            goto done;
         }

         int          iEnd = strlen (pszDataValue);
         int          iLoc = 0;
         bool         bOK;
         char         cTemp[2];
         unsigned int iValue;

         iTempCnt = 0;

         for (iCnt = 0; iCnt < iEnd; iCnt++)
         {
            if (isxdigit (*(pszDataValue+iCnt)))
            {
               cTemp[iLoc++] = *(pszDataValue+iCnt);
               if (iLoc == 2)
               {
                  bOK = parseHexGroup ((const char *)cTemp, &iValue);
                  if (!bOK)
                  {
                     std::cerr << "Error: Data in bytearray is not understood \"" << *(pszDataValue+iCnt) << "\" for " << pszDataName << "." << std::endl;
                     delete[] pbData;
                     goto done;
                  }

                  *(pbData+iTempCnt++) = (unsigned char)iValue;
                  iLoc = 0;
               }
            }
         }

         if (iLoc)
         {
            std::cerr << "Error: Data in bytearray is missing bytes\"" << *(pszDataValue+iCnt) << "\" for " << pszDataName << "." << std::endl;
            delete[] pbData;
            goto done;
         }
      }
      else
      {
         std::cerr << "Error: Unknown type " << pszTypeName << std::endl;
         goto done;
      }

      if (!pbData)
      {
         std::cerr << "Error: pbData is not allocated." << std::endl;
         goto done;
      }

      mapData[std::string (pszDataName)]    = pbData;
      mapElement[std::string (pszDataName)] = new BinaryData (pbData, cbData);

      // Move to the next one
      deviceDataNode = XMLNextNode (deviceDataNode);

      if (pszDataValue)
      {
         free ((void *)pszDataValue);
      }
      if (pszDataName)
      {
         free ((void *)pszDataName);
      }
      if (pszTypeName)
      {
         free ((void *)pszTypeName);
      }
   }

   rc = true;

   openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

   fFileModified_d = true;

   outputHeader (pofstreamHPP);

   *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
   *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#include <DeviceData.hpp>" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceData" << std::endl;
   *pofstreamHPP << "{" << std::endl;
   *pofstreamHPP << "public:" << std::endl;
   *pofstreamHPP << "   " << *pstringNameOut << " ();" << std::endl;
   *pofstreamHPP << "};" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#endif" << std::endl;

   outputHeader (pofstreamCPP);

   *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << *pstringNameOut << " ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;

   for ( DataMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      BinaryData *pbdValue = (BinaryData *)next->second;
      PBYTE       pbValue  = pbdValue->getData ();
      int         cbValue  = pbdValue->getLength ();

      *pofstreamCPP << "   static byte " << next->first << "[] = {";

      for (int i = 0; i < cbValue; i++)
      {
         *pofstreamCPP << (int)pbValue[i];
         if (i < cbValue - 1)
            *pofstreamCPP << ",";
      }

      *pofstreamCPP << "};" << std::endl;
   }

   *pofstreamCPP << std::endl;

   for ( DataMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      *pofstreamCPP << "   add (\""
                    << next->first
                    << "\", new BinaryData ("
                    << next->first
                    << ", dimof ("
                    << next->first << ")));"
                    << std::endl;
   }

   *pofstreamCPP << "}" << std::endl;

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceDatasNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringDataClassName = 0;
   }

   for ( DataMapElement::iterator next = mapElement.begin () ;
         next != mapElement.end () ;
         next++ )
   {
      delete (*next).second;
   }
   for ( DataMapData::iterator next = mapData.begin () ;
         next != mapData.end () ;
         next++ )
   {
      delete[] (*next).second;
   }

   return rc;
}

bool OmniDomParser::
processDeviceStrings (XmlNodePtr deviceStringsNode)
{
   if (deviceStringsNode == 0)
      return false;

   std::string *pstringNameOut = getNameOut ();

   pdi_d->pstringStringClassName = pstringNameOut;

   if (!shouldCreateFile (*pstrXMLFile_d))
      return true;

   std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;

   typedef std::map <std::string, std::string>        StringMapElement;
   typedef std::map <std::string, StringMapElement *> StringMapLanguage;

   XmlNodePtr         deviceStringNode = XMLFirstNode (XMLGetChildrenNode (deviceStringsNode));
   std::ofstream     *pofstreamHPP     = 0;
   std::ofstream     *pofstreamCPP     = 0;
   StringMapLanguage  mapLanguage;
   int                rc               = false;

   if (deviceStringNode == 0)
      goto done;

   while (deviceStringNode != 0)
   {
      XmlNodePtr  nodeItem      = XMLFirstNode (XMLGetChildrenNode (deviceStringNode));
      XmlNodePtr  nodeLanguages = 0;
      const char *pszName       = 0;
      std::string stringElement;

      if (nodeItem == 0)
         goto done;

      pszName = bundleStringData (nodeItem);

      stringElement = pszName;

      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      nodeLanguages = XMLFirstNode (XMLGetChildrenNode (nodeItem));

      while (nodeLanguages != 0)
      {
         const char       *pszTranslation      = 0;
         std::string       stringLanguageName;
         std::string       stringTranslation;
         StringMapElement *psme                = 0;

         pszTranslation = bundleStringData (nodeLanguages);

         stringLanguageName = (char *)XMLGetName (nodeLanguages);
         stringTranslation  = pszTranslation;

         psme = mapLanguage[stringLanguageName];

         if (!psme)
         {
            psme = new StringMapElement ();

            mapLanguage[stringLanguageName] = psme;
         }

         (*psme)[stringElement] = stringTranslation;

         // Move to the next one
         nodeLanguages = XMLNextNode (nodeLanguages);

         if (pszTranslation)
         {
            free ((void *)pszTranslation);
         }
      }

      // Move to the next one
      deviceStringNode = XMLNextNode (deviceStringNode);

      if (pszName)
      {
         free ((void *)pszName);
      }
   }

   rc = true;

   openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

   fFileModified_d = true;

   outputHeader (pofstreamHPP);

   *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
   *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#include <DeviceString.hpp>" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "class " << *pstringNameOut << " : public DeviceString" << std::endl;
   *pofstreamHPP << "{" << std::endl;
   *pofstreamHPP << "public:" << std::endl;
   *pofstreamHPP << "   " << *pstringNameOut << " ();" << std::endl;
   *pofstreamHPP << "};" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#endif" << std::endl;

   outputHeader (pofstreamCPP);

   *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << *pstringNameOut << " ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;

   for ( StringMapLanguage::iterator nextLanguage = mapLanguage.begin () ;
         nextLanguage != mapLanguage.end () ;
         nextLanguage++ )
   {
      StringMapElement *pEntries = (StringMapElement *)nextLanguage->second;

      for ( StringMapElement::iterator nextEntry = pEntries->begin () ;
            nextEntry != pEntries->end () ;
            nextEntry++ )
      {
         *pofstreamCPP << "   add (\""
                       << nextLanguage->first
                       << "\", \""
                       << nextEntry->first
                       << "\", \""
                       << nextEntry->second
                       << "\");"
                       << std::endl;
      }
   }

   *pofstreamCPP << "};" << std::endl;

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (deviceStringsNode), 0);
   }

   // Clean up!
   delete pofstreamHPP;
   delete pofstreamCPP;
   if (!rc)
   {
      delete pstringNameOut;
      pdi_d->pstringStringClassName = 0;
   }

   for ( StringMapLanguage::iterator nextLanguage = mapLanguage.begin () ;
         nextLanguage != mapLanguage.end () ;
         nextLanguage++ )
   {
      StringMapElement *pEntries = (StringMapElement *)nextLanguage->second;

      delete pEntries;
   }

   return rc;
}

bool OmniDomParser::
preprocessDriver (XmlNodePtr driverNode)
{
   // This routine does two things.
   //    It fill is pstringInstanceClassName and pstringBlitterClassName
   //       in the DeviceInfo structure.
   //    It sees if it needs to process (copy) the instance and blitter files.

   XmlNodePtr  nodeItem    = XMLFirstNode (XMLGetChildrenNode (driverNode));
   const char *pszNodeName = 0;
   PSZRO       pszExeName  = 0;
   PSZRO       pszData     = 0;
   bool        fRetVal     = false;

   while (  0 != nodeItem
         && !(  0 == strcmp (XMLGetName (nodeItem), "Instance")
             || 0 == strcmp (XMLGetName (nodeItem), "PluggableInstance")
             )
         )
   {
      // Move to the next one
      nodeItem = XMLNextNode (nodeItem);
   }

   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "Instance"))
   {
      pszNodeName = bundleStringData (nodeItem);

      pdi_d->pstringInstanceClassName = converToClassName (pszNodeName);

      copyFile (pszNodeName);

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Instance"))
         goto done;

      if (pszNodeName)
      {
         free ((void *)pszNodeName);
      }

      pszNodeName = bundleStringData (nodeItem);

      copyFile (pszNodeName);

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Blitter"))
         goto done;

      if (pszNodeName)
      {
         free ((void *)pszNodeName);
      }

      pszNodeName = bundleStringData (nodeItem);

      pdi_d->pstringBlitterClassName = converToClassName (pszNodeName);

      copyFile (pszNodeName);

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Blitter"))
         goto done;

      if (pszNodeName)
      {
         free ((void *)pszNodeName);
      }

      pszNodeName = bundleStringData (nodeItem);

      copyFile (pszNodeName);

      fRetVal = true;
   }
   else if (0 == strcmp (XMLGetName (nodeItem), "PluggableInstance"))
   {
      pszExeName = XMLGetProp (nodeItem, "exename");
      pszData    = XMLGetProp (nodeItem, "data");

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "exename = " << pszExeName << std::endl;
      if (fDebugOutput) std::cerr << "data = " << pszData << std::endl;
#endif

      if (pszExeName == 0)
      {
         std::cerr << "Error: exename is missing for PluggableInstance tag." << std::endl;
         goto done;
      }

      pdi_d->pstringExeName = new std::string (pszExeName);
      if (pszData)
         pdi_d->pstringData = new std::string (pszData);

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "PluggableBlitter"))
         goto done;

      fRetVal = true;
   }
   else
   {
      std::cerr << "Error: Expecting either an Instance tag or a PluggableInstance tag." << std::endl;
      goto done;
   }

done:
   if (pszNodeName)
   {
      free ((void *)pszNodeName);
   }

   if (pszExeName)
   {
      free ((void *)pszExeName);
   }
   if (pszData)
   {
      free ((void *)pszData);
   }

   if (!fRetVal)
   {
      std::cerr << "Error: preprocessDriver failed." << std::endl;
      setErrorCondition ();
   }

   return fRetVal;
}

static int
parseSize (char **ppszComma)
{
   int iReturn = 0;

   while (  '0' <= **ppszComma
         && **ppszComma <= '9'
         )
   {
      iReturn = iReturn * 10 + (**ppszComma - '0');

      (*ppszComma)++;
   }

   return iReturn;
}

bool OmniDomParser::
processDriver (XmlNodePtr driverNode)
{
   bool  fFoundItem = false;
   char *pszComma   = 0;

   if (driverNode == 0)
      return false;

   if (!preprocessDriver (driverNode))
      return false;

   if (!shouldCreateFile (*pstrXMLFile_d))
   {
      std::cout << "Parsing:    \"" << *pstrXMLFile_d << "\"" << std::endl;
   }
   else
   {
      std::cout << "Generating: \"" << *pstrXMLFile_d << "\"" << std::endl;
   }

   // Make sure required fields in the Device Info struture are filled in
   if (!pdi_d->pstringCommandClassName)
   {
      std::cerr << "Error: DeviceCommands not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringResolutionClassName)
   {
      std::cerr << "Error: DeviceResolution not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringPrintModeClassName)
   {
      std::cerr << "Error: DevicePrintMode not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringTrayClassName)
   {
      std::cerr << "Error: DeviceTray not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringFormClassName)
   {
      std::cerr << "Error: DeviceForm not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringMediaClassName)
   {
      std::cerr << "Error: DeviceMedia not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }
   if (!pdi_d->pstringOrientationClassName)
   {
      std::cerr << "Error: DeviceOrientation not filled in." << std::endl;
      setErrorCondition ();
      return false;
   }

   if (fAutoconf_d)
   {
      std::ofstream *pofstreamAM1   = 0;
      std::ofstream *pofstreamAM2   = 0;
      std::string   *pstringNameOut = getNameOut ();

      pofstreamAM1 = new std::ofstream ("libraries1.mak", std::ios::app);
      pofstreamAM2 = new std::ofstream ("libraries2.mak", std::ios::app);

      if (fAutoconfNoInst_d)
         *pofstreamAM1 << "noinst_LTLIBRARIES";
      else
         *pofstreamAM1 << "pkglib_LTLIBRARIES";

      *pofstreamAM1 << " += lib" << *pstringNameOut << ".la" << std::endl;

      *pofstreamAM2 << "lib" << *pstringNameOut << "_la_SOURCES = " << *pdi_d->pstringCommandClassName << ".cpp \\" << std::endl;
#ifdef INCLUDE_JP_COMMON_COPIES
      if (pdi_d->pstringCopyClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringCopyClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringFormClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringFormClassName << ".cpp \\" << std::endl;
      }
#ifdef INCLUDE_JP_UPDF_JOGGING
      if (pdi_d->pstringJoggingClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringJoggingClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringMediaClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringMediaClassName << ".cpp \\" << std::endl;
      }
#ifdef INCLUDE_JP_COMMON_NUP
      if (pdi_d->pstringNUpClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringNUpClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringOrientationClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringOrientationClassName << ".cpp \\" << std::endl;
      }
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      if (pdi_d->pstringOutputBinClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringOutputBinClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringPrintModeClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringPrintModeClassName << ".cpp \\" << std::endl;
      }
      if (pdi_d->pstringResolutionClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringResolutionClassName << ".cpp \\" << std::endl;
      }
#ifdef INCLUDE_JP_COMMON_SCALING
      if (pdi_d->pstringScalingClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringScalingClassName << ".cpp \\" << std::endl;
      }
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      if (pdi_d->pstringSheetCollateClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringSheetCollateClassName << ".cpp \\" << std::endl;
      }
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
      if (pdi_d->pstringSideClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringSideClassName << ".cpp \\" << std::endl;
      }
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
      if (pdi_d->pstringStitchingClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringStitchingClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringTrayClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringTrayClassName << ".cpp \\" << std::endl;
      }
#ifdef INCLUDE_JP_COMMON_TRIMMING
      if (pdi_d->pstringTrimmingClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringTrimmingClassName << ".cpp \\" << std::endl;
      }
#endif
      if (pdi_d->pstringGammaClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringGammaClassName << ".cpp \\" << std::endl;
      }
      if (pdi_d->pstringInstanceClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringInstanceClassName << ".cpp \\" << std::endl;
      }
      if (pdi_d->pstringBlitterClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringBlitterClassName << ".cpp \\" << std::endl;
      }
      if (pdi_d->pstringDataClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringDataClassName << ".cpp \\" << std::endl;
      }
      if (pdi_d->pstringStringClassName)
      {
         *pofstreamAM2 << "                        " << *pdi_d->pstringStringClassName << ".cpp \\" << std::endl;
      }
      *pofstreamAM2 << "                        " << *pstringNameOut << ".cpp" << std::endl;
      *pofstreamAM2 << "lib" << *pstringNameOut << "_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@" << std::endl;

      pofstreamAM1->close ();
      pofstreamAM2->close ();

      delete pofstreamAM1;
      delete pofstreamAM2;
      delete pstringNameOut;
   }

   if (!shouldCreateFile (*pstrXMLFile_d))
      return true;

   typedef std::list <std::string> DriverList;

   const char         *pszDeviceName          = 0;
   const char         *pszDriverName          = 0;
   const char         *pszTypeName            = 0;
   const char         *pszMajorPDLLevel       = 0;
   const char         *pszMinorPDLLevel       = 0;
   const char         *pszMajorRevisionLevel  = 0;
   const char         *pszMinorRevisionLevel  = 0;
   const char         *pszCopies              = 0;
   const char         *pszDither              = 0;
   const char         *pszForm                = 0;
   const char         *pszJogging             = 0;
   const char         *pszMedia               = 0;
   const char         *pszNUpDirection        = 0;
   const char         *pszNUpX                = 0;
   const char         *pszNUpY                = 0;
   const char         *pszOrientation         = 0;
   const char         *pszOutputBin           = 0;
   const char         *pszPrintMode           = 0;
   const char         *pszResolution          = 0;
   const char         *pszScalingPercentage   = 0;
   const char         *pszScalingType         = 0;
   const char         *pszSheetCollate        = 0;
   const char         *pszSide                = 0;
   const char         *pszStitchingAngle      = 0;
   const char         *pszStitchingCount      = 0;
   const char         *pszStitchingPosition   = 0;
   const char         *pszStitchingRefEdge    = 0;
   const char         *pszStitchingType       = 0;
   const char         *pszTray                = 0;
   const char         *pszTrimming            = 0;
   std::string         stringCopies;
   std::string         stringForm;
   std::string         stringJogging;
   std::string         stringMedia;
   std::string         stringNUp;
   std::string         stringOrientation;
   std::string         stringOutputBin;
   std::string         stringPrintMode;
   std::string         stringResolution;
   std::string         stringScaling;
   std::string         stringSheetCollate;
   std::string         stringSide;
   std::string         stringStitching;
   std::string         stringTray;
   std::string         stringTrimming;
   XmlNodePtr          nodeItem               = XMLFirstNode (XMLGetChildrenNode (driverNode));
   std::string        *pstringNameOut         = getNameOut ();
   std::ofstream      *pofstreamHPP           = 0;
   std::ofstream      *pofstreamCPP           = 0;
   std::ofstream      *pofstreamMAK           = 0;
   DriverList          listCapabilities;
   DriverList          listRasterCapabilities;
   DriverList          listDeviceOptions;
   int                 iCount                 = 0;
   XmlNodePtr          nodeOther              = 0;
   int                 rc                     = false;
   std::ostringstream  oss;

   pszDeviceName = XMLGetProp (driverNode, "name");

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszDeviceName = " << pszDeviceName << std::endl;
#endif

   pszDriverName = bundleStringData (nodeItem);

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszDriverName = " << pszDriverName << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

#ifdef DEBUG
   if (fDebugOutput) printNodeName (nodeItem);
#endif

   while (  0 != nodeItem
         && 0 == strcmp (XMLGetName (nodeItem), "Capability")
         )
   {
      pszTypeName = XMLGetProp (nodeItem, "type");

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszTypeName = " << pszTypeName << std::endl;
#endif

      listCapabilities.push_back (std::string (pszTypeName));

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);

#ifdef DEBUG
      if (fDebugOutput && nodeItem) printNodeName (nodeItem);
#endif

      if (pszTypeName)
      {
         free ((void *)pszTypeName);
      }
   }

   if (nodeItem == 0)
      goto done;

   while (  0 != nodeItem
         && 0 == strcmp (XMLGetName (nodeItem), "RasterCapabilities")
         )
   {
      pszTypeName = XMLGetProp (nodeItem, "type");

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszTypeName = " << pszTypeName << std::endl;
#endif

      listRasterCapabilities.push_back (std::string (pszTypeName));

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);

#ifdef DEBUG
      if (fDebugOutput && nodeItem) printNodeName (nodeItem);
#endif

      if (pszTypeName)
      {
         free ((void *)pszTypeName);
      }
   }

   if (nodeItem == 0)
      goto done;

   while (  0 != nodeItem
         && 0 == strcmp (XMLGetName (nodeItem), "DeviceOptions")
         )
   {
      pszTypeName = XMLGetProp (nodeItem, "type");

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszTypeName = " << pszTypeName << std::endl;
#endif

      listDeviceOptions.push_back (std::string (pszTypeName));

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);

#ifdef DEBUG
      if (fDebugOutput && nodeItem) printNodeName (nodeItem);
#endif

      if (pszTypeName)
      {
         free ((void *)pszTypeName);
      }
   }

   if (nodeItem == 0)
      goto done;

   if (0 != strcmp (XMLGetName (nodeItem), "PDL"))
   {
      std::cerr << "Error: Could not find PDL tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszMajorPDLLevel = XMLGetProp (nodeItem, "level");
   if (pszMajorPDLLevel == 0)
      goto done;
#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszMajorPDLLevel = " << pszMajorPDLLevel << std::endl;
#endif

   pszMinorPDLLevel = XMLGetProp (nodeItem, "sublevel");
   if (pszMinorPDLLevel == 0)
      goto done;
#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszMinorPDLLevel = " << pszMinorPDLLevel << std::endl;
#endif

   pszMajorRevisionLevel = XMLGetProp (nodeItem, "major");
   if (pszMajorRevisionLevel == 0)
      goto done;
#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszMajorRevisionLevel = " << pszMajorRevisionLevel << std::endl;
#endif

   pszMinorRevisionLevel = XMLGetProp (nodeItem, "minor");
   if (pszMinorRevisionLevel == 0)
      goto done;
#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszMinorRevisionLevel = " << pszMinorRevisionLevel << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

#ifdef DEBUG
   if (fDebugOutput) printNodeName (nodeItem);
#endif

   while (  0 != nodeItem
         && (  0 == strcmp (XMLGetName (nodeItem), "Uses")
            || 0 == strcmp (XMLGetName (nodeItem), "Has")
            )
         )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);

#ifdef DEBUG
      if (fDebugOutput && nodeItem) printNodeName (nodeItem);
#endif
   }

   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "Instance"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Instance"))
      {
         std::cerr << "Error: Could not find Instance tag" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Blitter"))
      {
         std::cerr << "Error: Could not find Blitter tag" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "Blitter"))
      {
         std::cerr << "Error: Could not find Blitter tag" << std::endl;
         setErrorCondition ();
         goto done;
      }
   }
   else if (0 == strcmp (XMLGetName (nodeItem), "PluggableInstance"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;

      if (0 != strcmp (XMLGetName (nodeItem), "PluggableBlitter"))
      {
         std::cerr << "Error: Could not find PluggableBlitter tag" << std::endl;
         setErrorCondition ();
         goto done;
      }

      if (0 != listDeviceOptions.size ())
      {
         std::cerr << "Error: Cannot have <DeviceOptions> with <PluggableInstance> tag" << std::endl;
         setErrorCondition ();
         goto done;
      }
   }
   else
   {
      std::cerr << "Error: Could not find Instance or PluggableInstance tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "DeviceData"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "DefaultJobProperties"))
   {
      std::cerr << "Error: Expecting DefaultJobProperties tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   // Move down into the DefaultJobProperties
   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeItem));
   if (nodeItem == 0)
      goto done;

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "Copies")
      )
   {
#ifdef INCLUDE_JP_COMMON_COPIES

      pszCopies = bundleStringData (nodeItem);
      oss.str ("");
      if (pszCopies)
         oss << "Copies=" << const_cast<char*>(pszCopies);
      stringCopies = oss.str ();

      if (  0 == pszCopies
         || !DeviceCopies::isValid (stringCopies.c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceCopies \"" << pszCopies << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // @TBD
      fFoundItem = true;

      if (!fFoundItem)
      {
         std::cerr << "Error: \"" << pszCopies << "\" is not defined for this device." << std::endl;
         setErrorCondition ();
         goto done;
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszCopies = " << pszCopies << std::endl;
#endif
#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "dither"))
   {
      std::cerr << "Error: Expecting dither tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszDither = bundleStringData (nodeItem);

   if (  0 == pszDither
      || !DeviceDither::ditherNameValid (const_cast<char*>(pszDither))
      )
   {
      std::cerr << "Error: Unknown DeviceDither \"" << pszDither << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszDither = " << pszDither << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 != strcmp (XMLGetName (nodeItem), "Form"))
   {
      std::cerr << "Error: Expecting form tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszForm = bundleStringData (nodeItem);
   oss.str ("");
   if (pszForm)
      oss << "Form=" << const_cast<char*>(pszForm);
   stringForm = oss.str ();

   if (  0 == pszForm
      || !DeviceForm::isValid (stringForm.c_str ())
      )
   {
      std::cerr << "Error: Unknown DeviceForm \"" << pszForm << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( FormList::iterator next = pdi_d->listForms.begin () ;
         next != pdi_d->listForms.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszForm))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszForm << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

   if (pszComma)
   {
      *pszComma = ',';
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszForm = " << pszForm << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "omniForm"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "media"))
   {
      std::cerr << "Error: Expecting media tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszMedia = bundleStringData (nodeItem);
   oss.str ("");
   if (pszMedia)
      oss << "media=" << const_cast<char*>(pszMedia);
   stringMedia = oss.str ();

   if (  0 == pszMedia
      || !DeviceMedia::isValid (stringMedia.c_str ())
      )
   {
      std::cerr << "Error: Unknown DeviceMedia \"" << pszMedia << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( MediaList::iterator next = pdi_d->listMedias.begin () ;
         next != pdi_d->listMedias.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszMedia))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszMedia << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszMedia = " << pszMedia << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "omniMedia"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "NumberUp")
      )
   {
#ifdef INCLUDE_JP_COMMON_NUP

      std::string *pstringTemp = getXMLJobProperties (nodeItem,
                                                      XMLGetDocNode (nodeItem),
                                                      0);

      if (pstringTemp)
      {
         stringNUp = *pstringTemp;

         delete pstringTemp;
      }

#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "Rotation"))
   {
      std::cerr << "Error: Expecting orientation tag" << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszOrientation = bundleStringData (nodeItem);
   oss.str ("");
   if (pszOrientation)
      oss << "Rotation=" << const_cast<char*>(pszOrientation);
   stringOrientation = oss.str ();

   if (  0 == pszOrientation
      || !DeviceOrientation::isValid (stringOrientation.c_str ())
      )
   {
      std::cerr << "Error: Unknown DeviceOrientation \"" << pszOrientation << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( OrientationList::iterator next = pdi_d->listOrientations.begin () ;
         next != pdi_d->listOrientations.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszOrientation))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszOrientation << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszOrientation = " << pszOrientation << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "omniOrientation"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "OutputBin")
      )
   {
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

      pszOutputBin = bundleStringData (nodeItem);
      oss.str ("");
      if (pszOutputBin)
         oss << "OutputBin=" << const_cast<char*>(pszOutputBin);
      stringOutputBin = oss.str ();

      if (  0 == pszOutputBin
         || !DeviceOutputBin::isValid (stringOutputBin.c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceOutputBin \"" << pszOutputBin << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // @TBD
      fFoundItem = true;

      if (!fFoundItem)
      {
         std::cerr << "Error: \"" << pszOutputBin << "\" is not defined for this device." << std::endl;
         setErrorCondition ();
         goto done;
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszOutputBin = " << pszOutputBin << std::endl;
#endif
#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "printmode"))
   {
      std::cerr << "Error: Expecting printmode tag." << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszPrintMode = bundleStringData (nodeItem);
   oss.str ("");
   if (pszPrintMode)
      oss << "printmode=" << const_cast<char*>(pszPrintMode);
   stringPrintMode = oss.str ();

   if (  0 == pszPrintMode
      || !DevicePrintMode::isValid (stringPrintMode.c_str ())
      )
   {
      std::cerr << "Error: Unknown DevicePrintMode \"" << pszPrintMode << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( PrintModeList::iterator next = pdi_d->listPrintModes.begin () ;
         next != pdi_d->listPrintModes.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszPrintMode))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszPrintMode << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszPrintMode = " << pszPrintMode << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "omniPrintmode")
      )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "Resolution"))
   {
      std::cerr << "Error: Expecting resolution tag." << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszResolution = bundleStringData (nodeItem);
   oss.str ("");
   if (pszResolution)
      oss << "Resolution=" << const_cast<char*>(pszResolution);
   stringResolution = oss.str ();

   if (  0 == pszResolution
      || !DeviceResolution::isValid (stringResolution.c_str ())
      )
   {
      std::cerr << "Error: Unknown DeviceResolution \"" << pszResolution << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( ResolutionList::iterator next = pdi_d->listResolutions.begin () ;
         next != pdi_d->listResolutions.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszResolution))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszResolution << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszResolution = " << pszResolution << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);
   if (nodeItem == 0)
      goto done;

   if (0 == strcmp (XMLGetName (nodeItem), "omniResolution"))
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "Scaling")
      )
   {
#ifdef INCLUDE_JP_COMMON_SCALING

      std::string *pstringTemp = getXMLJobProperties (nodeItem,
                                                      XMLGetDocNode (nodeItem),
                                                      0);

      if (pstringTemp)
      {
         stringScaling = *pstringTemp;

         delete pstringTemp;
      }

#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "SheetCollate")
      )
   {
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

      pszSheetCollate = bundleStringData (nodeItem);
      oss.str ("");
      if (pszSheetCollate)
         oss << "SheetCollate=" << const_cast<char*>(pszSheetCollate);
      stringSheetCollate = oss.str ();

      if (  0 == pszSheetCollate
         || !DeviceSheetCollate::isValid (stringSheetCollate.c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceSheetCollate \"" << pszSheetCollate << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // @TBD
      fFoundItem = true;

      if (!fFoundItem)
      {
         std::cerr << "Error: \"" << pszSheetCollate << "\" is not defined for this device." << std::endl;
         setErrorCondition ();
         goto done;
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszSheetCollate = " << pszSheetCollate << std::endl;
#endif
#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "Sides")
      )
   {
#ifdef INCLUDE_JP_COMMON_SIDE

      pszSide = bundleStringData (nodeItem);
      oss.str ("");
      if (pszSide)
         oss << "Sides=" << const_cast<char*>(pszSide);
      stringSide = oss.str ();

      if (  0 == pszSide
         || !DeviceSide::isValid (stringSide.c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceSide \"" << pszSide << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // @TBD
      fFoundItem = true;

      if (!fFoundItem)
      {
         std::cerr << "Error: \"" << pszSide << "\" is not defined for this device." << std::endl;
         setErrorCondition ();
         goto done;
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszSide = " << pszSide << std::endl;
#endif
#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "Stitching")
      )
   {
#ifdef INCLUDE_JP_COMMON_STITCHING

      std::string *pstringTemp = getXMLJobProperties (nodeItem,
                                                      XMLGetDocNode (nodeItem),
                                                      0);

      if (pstringTemp)
      {
         stringStitching = *pstringTemp;

         delete pstringTemp;
      }

#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
      if (nodeItem == 0)
         goto done;
   }

   if (0 != strcmp (XMLGetName (nodeItem), "InputTray"))
   {
      std::cerr << "Error: Expecting tray tag." << std::endl;
      setErrorCondition ();
      goto done;
   }

   pszTray = bundleStringData (nodeItem);
   oss.str ("");
   if (pszTray)
      oss << "InputTray=" << const_cast<char*>(pszTray);
   stringTray = oss.str ();

   if (  0 == pszTray
      || !DeviceTray::isValid (stringTray.c_str ())
      )
   {
      std::cerr << "Error: Unknown DeviceTray \"" << pszTray << "\"" << std::endl;
      setErrorCondition ();
      goto done;
   }

   fFoundItem = false;
   for ( TrayList::iterator next = pdi_d->listTrays.begin () ;
         next != pdi_d->listTrays.end () ;
         next++ )
   {
      if (0 == strcmp (next->c_str (), pszTray))
      {
         fFoundItem = true;
      }
   }

   if (!fFoundItem)
   {
      std::cerr << "Error: \"" << pszTray << "\" is not defined for this device." << std::endl;
      setErrorCondition ();
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszTray = " << pszTray << std::endl;
#endif

   // Move to the next node
   nodeItem = XMLNextNode (nodeItem);

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "omniTray")
      )
   {
      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   if (  nodeItem
      && 0 == strcmp (XMLGetName (nodeItem), "Trimming")
      )
   {
#ifdef INCLUDE_JP_COMMON_TRIMMING

      pszTrimming = bundleStringData (nodeItem);
      oss.str ("");
      if (pszTrimming)
         oss << "Trimming=" << const_cast<char*>(pszTrimming);
      stringTrimming = oss.str ();

      if (  0 == pszTrimming
         || !DeviceTrimming::isValid (stringTrimming.c_str ())
         )
      {
         std::cerr << "Error: Unknown DeviceTrimming \"" << pszTrimming << "\"" << std::endl;
         setErrorCondition ();
         goto done;
      }

      // @TBD
      fFoundItem = true;

      if (!fFoundItem)
      {
         std::cerr << "Error: \"" << pszTrimming << "\" is not defined for this device." << std::endl;
         setErrorCondition ();
         goto done;
      }

#ifdef DEBUG
      if (fDebugOutput) std::cerr << "pszTrimming = " << pszTrimming << std::endl;
#endif
#endif

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   nodeOther = nodeItem;

   // process optional other(s)
   while (nodeItem)
   {
      if (0 != (strcmp (XMLGetName (nodeItem), "other")))
          goto done;

      // Move to the next node
      nodeItem = XMLNextNode (nodeItem);
   }

   rc = true;

   listCapabilities.sort ();
   listRasterCapabilities.sort ();
   listDeviceOptions.sort ();
   listCapabilities.unique ();
   listRasterCapabilities.unique ();
   listDeviceOptions.unique ();

   openOutputFiles (pstringNameOut, &pofstreamHPP, &pofstreamCPP);

   fFileModified_d = true;

   outputHeader (pofstreamHPP);

   *pofstreamHPP << "#ifndef _" << *pstringNameOut << std::endl;
   *pofstreamHPP << "#define _" << *pstringNameOut << std::endl;
   *pofstreamHPP << std::endl;

#ifdef INCLUDE_JP_UPDF_BOOKLET
   *pofstreamHPP << "#include <" << *pdi_d->pstringBookletClassName << ".hpp>" << std::endl;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   if (pdi_d->pstringCopyClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringCopyClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringFormClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringFormClassName << ".hpp>" << std::endl;
   }
#ifdef INCLUDE_JP_UPDF_JOGGING
   if (pdi_d->pstringJoggingClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringJoggingClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringMediaClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringMediaClassName << ".hpp>" << std::endl;
   }
#ifdef INCLUDE_JP_COMMON_NUP
   if (pdi_d->pstringNUpClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringNUpClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringOrientationClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringOrientationClassName << ".hpp>" << std::endl;
   }
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (pdi_d->pstringOutputBinClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringOutputBinClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringPrintModeClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringPrintModeClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringResolutionClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringResolutionClassName << ".hpp>" << std::endl;
   }
#ifdef INCLUDE_JP_COMMON_SCALING
   if (pdi_d->pstringScalingClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringScalingClassName << ".hpp>" << std::endl;
   }
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (pdi_d->pstringSheetCollateClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringSheetCollateClassName << ".hpp>" << std::endl;
   }
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   if (pdi_d->pstringSideClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringSideClassName << ".hpp>" << std::endl;
   }
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   if (pdi_d->pstringStitchingClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringStitchingClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringTrayClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringTrayClassName << ".hpp>" << std::endl;
   }
#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (pdi_d->pstringTrimmingClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringTrimmingClassName << ".hpp>" << std::endl;
   }
#endif
   if (pdi_d->pstringCommandClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringCommandClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringDataClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringDataClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringStringClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringStringClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringPrintModeClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringPrintModeClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringInstanceClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringInstanceClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringBlitterClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringBlitterClassName << ".hpp>" << std::endl;
   }
   if (pdi_d->pstringGammaClassName)
   {
      *pofstreamHPP << "#include <" << *pdi_d->pstringGammaClassName << ".hpp>" << std::endl;
   }
   *pofstreamHPP << "#include <Device.hpp>" << std::endl;
   *pofstreamHPP << "#include <PrintDevice.hpp>" << std::endl;
   *pofstreamHPP << "#include <JobProperties.hpp>" << std::endl;
   *pofstreamHPP << "#include <OmniProxy.hpp>" << std::endl;
   *pofstreamHPP << std::endl;

   *pofstreamHPP << "class " << *pstringNameOut << " : public PrintDevice" << std::endl;
   *pofstreamHPP << "{" << std::endl;
   *pofstreamHPP << "public:" << std::endl;

   iCount = listRasterCapabilities.size ();
   if (0 < iCount)
   {
      int iValue = 1;

      *pofstreamHPP << "   enum {" << std::endl;

      for ( DriverList::iterator next = listRasterCapabilities.begin () ;
            next != listRasterCapabilities.end () ;
            next++, iCount-- )
      {
         *pofstreamHPP << "      " << *next << " = " << iValue;
         if (1 < iCount)
            *pofstreamHPP << ",";
         *pofstreamHPP << std::endl;

         iValue *= 2;
      }

      *pofstreamHPP << "   };" << std::endl;
      *pofstreamHPP << std::endl;
   }

   *pofstreamHPP << "   " << *pstringNameOut << " ();" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "   " << *pstringNameOut << " (PSZCRO pszJobProperties);" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "   virtual ~" << *pstringNameOut << " ();" << std::endl;
   *pofstreamHPP << std::endl;

   if (0 < listDeviceOptions.size ())
   {
      *pofstreamHPP << "   virtual bool               hasDeviceOption          (PSZCRO pszDeviceOption);" << std::endl;
      *pofstreamHPP << std::endl;
   }
#ifdef INCLUDE_JP_UPDF_BOOKLET
   *pofstreamHPP << "   virtual DeviceBooklet      *getDefaultBooklet        ();" << std::endl;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   *pofstreamHPP << "   virtual DeviceCopies       *getDefaultCopies         ();" << std::endl;
#endif
   *pofstreamHPP << "   virtual PSZCRO              getDefaultDitherID       ();" << std::endl;
   *pofstreamHPP << "   virtual DeviceForm         *getDefaultForm           ();" << std::endl;
#ifdef INCLUDE_JP_UPDF_JOGGING
   *pofstreamHPP << "   virtual DeviceJogging      *getDefaultJogging        ();" << std::endl;
#endif
   *pofstreamHPP << "   virtual DeviceMedia        *getDefaultMedia          ();" << std::endl;
#ifdef INCLUDE_JP_COMMON_NUP
   *pofstreamHPP << "   virtual DeviceNUp          *getDefaultNUp            ();" << std::endl;
#endif
   *pofstreamHPP << "   virtual DeviceOrientation  *getDefaultOrientation    ();" << std::endl;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   *pofstreamHPP << "   virtual DeviceOutputBin    *getDefaultOutputBin      ();" << std::endl;
#endif
   *pofstreamHPP << "   virtual DevicePrintMode    *getDefaultPrintMode      ();" << std::endl;
   *pofstreamHPP << "   virtual DeviceResolution   *getDefaultResolution     ();" << std::endl;
#ifdef INCLUDE_JP_COMMON_SCALING
   *pofstreamHPP << "   virtual DeviceScaling      *getDefaultScaling        ();" << std::endl;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   *pofstreamHPP << "   virtual DeviceSheetCollate *getDefaultSheetCollate   ();" << std::endl;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   *pofstreamHPP << "   virtual DeviceSide         *getDefaultSide           ();" << std::endl;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   *pofstreamHPP << "   virtual DeviceStitching    *getDefaultStitching      ();" << std::endl;
#endif
   *pofstreamHPP << "   virtual DeviceTray         *getDefaultTray           ();" << std::endl;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   *pofstreamHPP << "   virtual DeviceTrimming     *getDefaultTrimming       ();" << std::endl;
#endif
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "   virtual DeviceGamma        *getCurrentGamma          ();" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "   virtual DeviceCommand      *getDefaultCommands       ();" << std::endl;
   *pofstreamHPP << "   virtual DeviceData         *getDefaultData           ();" << std::endl;
   *pofstreamHPP << "   virtual DeviceString       *getDefaultString         ();" << std::endl;
   *pofstreamHPP << std::endl;
   *pofstreamHPP << "private:" << std::endl;
   *pofstreamHPP << "   void commonInit ();" << std::endl;
   *pofstreamHPP << "};" << std::endl;
   *pofstreamHPP << std::endl;

   *pofstreamHPP << "extern \"C\" {" << std::endl;
   *pofstreamHPP << "   PSZCRO       getVersion                  ();" << std::endl;
   *pofstreamHPP << "   Enumeration *getDeviceEnumeration        (PSZCRO  pszLibraryName," << std::endl;
   *pofstreamHPP << "                                             bool    fBuildOnly);" << std::endl;
   *pofstreamHPP << "   Device      *newDeviceW_Advanced         (bool    fAdvanced);" << std::endl;
   *pofstreamHPP << "   Device      *newDeviceW_JopProp_Advanced (PSZCRO  pszJobProperties," << std::endl;
   *pofstreamHPP << "                                             bool    fAdvanced);" << std::endl;
   *pofstreamHPP << "   void         deleteDevice                (Device *pDevice);" << std::endl;
#ifdef HAVE_BACKWARDS_COMPATIBILITY
   *pofstreamHPP << "   void         deleteDevice__FP6Device     (Device *pDevice);" << std::endl;
#endif
   *pofstreamHPP << "};" << std::endl;

   *pofstreamHPP << std::endl;
   *pofstreamHPP << "#endif" << std::endl;

   outputHeader (pofstreamCPP);

   *pofstreamCPP << "#include <" << *pstringNameOut << ".hpp>" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << "PSZCRO" << std::endl;
   *pofstreamCPP << "getVersion ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return VERSION;" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << "Enumeration *" << std::endl;
   *pofstreamCPP << "getDeviceEnumeration (PSZCRO pszLibraryName," << std::endl;
   *pofstreamCPP << "                      bool   fBuildOnly)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return 0;" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << "Device *" << std::endl;
   *pofstreamCPP << "newDeviceW_Advanced (bool fAdvanced)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   DebugOutput::logMessage (LOG_INFO, \"" << *pstringNameOut << ":newDevice: Advanced = %d\", fAdvanced);" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   PrintDevice *pPrintDevice = new " << *pstringNameOut << " ();" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   pPrintDevice->initialize ();" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   if (fAdvanced)" << std::endl;
   *pofstreamCPP << "      return pPrintDevice;" << std::endl;
   *pofstreamCPP << "   else" << std::endl;
   *pofstreamCPP << "      return new OmniProxy (pPrintDevice);" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << "Device *" << std::endl;
   *pofstreamCPP << "newDeviceW_JopProp_Advanced (PSZCRO pszJobProperties, bool fAdvanced)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   if (pszJobProperties)" << std::endl;
   *pofstreamCPP << "   {" << std::endl;
   *pofstreamCPP << "      DebugOutput::logMessage (LOG_INFO, \"" << *pstringNameOut << ":newDevice: JobProperties = \\\"%s\\\", Advanced = %d\", pszJobProperties, fAdvanced);" << std::endl;
   *pofstreamCPP << "   }" << std::endl;
   *pofstreamCPP << "   else" << std::endl;
   *pofstreamCPP << "   {" << std::endl;
   *pofstreamCPP << "      DebugOutput::logMessage (LOG_INFO, \"" << *pstringNameOut << ":newDevice: JobProperties = null, Advanced = %d\", fAdvanced);" << std::endl;
   *pofstreamCPP << "   }" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   PrintDevice *pPrintDevice = new " << *pstringNameOut << " (pszJobProperties);" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   pPrintDevice->initialize ();" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "   if (fAdvanced)" << std::endl;
   *pofstreamCPP << "      return pPrintDevice;" << std::endl;
   *pofstreamCPP << "   else" << std::endl;
   *pofstreamCPP << "      return new OmniProxy (pPrintDevice);" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

#ifdef HAVE_BACKWARDS_COMPATIBILITY
   *pofstreamCPP << "void" << std::endl;
   *pofstreamCPP << "deleteDevice__FP6Device (Device *pDevice)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   deleteDevice (pDevice);" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;
#endif

   *pofstreamCPP << "void" << std::endl;
   *pofstreamCPP << "deleteDevice (Device *pDevice)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   delete pDevice;" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << "void " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "commonInit ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;

   iCount = listCapabilities.size ();
   if (0 < iCount)
   {
      *pofstreamCPP << "   setCapabilities ( ";

      for ( DriverList::iterator next = listCapabilities.begin () ;
            next != listCapabilities.end () ;
            next++, iCount-- )
      {
         *pofstreamCPP << "Capability::" << *next << std::endl;
         if (1 < iCount)
            *pofstreamCPP << "                   | ";
      }

      *pofstreamCPP << "                   );" << std::endl;
   }

   iCount = listRasterCapabilities.size ();
   if (0 < iCount)
   {
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   setRasterCapabilities ( ";

      for ( DriverList::iterator next = listRasterCapabilities.begin () ;
            next != listRasterCapabilities.end () ;
            next++, iCount-- )
      {
         *pofstreamCPP << *next << std::endl;
         if (1 < iCount)
            *pofstreamCPP << "                   | ";
      }

      *pofstreamCPP << "                         );" << std::endl;
   }

   *pofstreamCPP << std::endl;
   if (pdi_d->pstringExeName)
   {
      const char *pszData = "";

      if (pdi_d->pstringData)
         pszData = pdi_d->pstringData->c_str ();
      *pofstreamCPP << "   setDeviceInstance (new PluggableInstance (this," << std::endl;
      *pofstreamCPP << "                                             \"" << *pdi_d->pstringExeName << "\"," << std::endl;
      *pofstreamCPP << "                                             \"" << pszData << "\"));" << std::endl;
      *pofstreamCPP << "   setDeviceBlitter (new PluggableBlitter (this));" << std::endl;
   }
   else if (  pdi_d->pstringInstanceClassName
           && pdi_d->pstringBlitterClassName
           )
   {
      *pofstreamCPP << "   setDeviceInstance (new " << *pdi_d->pstringInstanceClassName << " (this));" << std::endl;
      *pofstreamCPP << "   setDeviceBlitter (new " << *pdi_d->pstringBlitterClassName << " (this));" << std::endl;
   }

   if (  pszMajorPDLLevel
      && pszMinorPDLLevel
      && pszMajorRevisionLevel
      && pszMinorRevisionLevel
      )
   {
      *pofstreamCPP << "   setPDL (new PDL (PDL::"
                    << pszMajorPDLLevel
                    << ",PDL::"
                    << pszMinorPDLLevel
                    << ","
                    << pszMajorRevisionLevel
                    << ","
                    << pszMinorRevisionLevel
                    << "));"
                    << std::endl;
   }

   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;

   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << *pstringNameOut << " ()" << std::endl;
   *pofstreamCPP << "   : PrintDevice (\"" << pszDriverName << "\"," << std::endl;
   *pofstreamCPP << "                  \"" << pszDeviceName << "\"," << std::endl;
   *pofstreamCPP << "                  \"" << *pstringNameOut << "\"," << std::endl;
   *pofstreamCPP << "                  \"lib" << *pstringNameOut << ".so\"," << std::endl;
   *pofstreamCPP << "                  OMNI_CLASS_COMPILED," << std::endl;
   *pofstreamCPP << "                  \"dither=" << pszDither;
   *pofstreamCPP << " " << stringForm;
   *pofstreamCPP << " " << stringMedia;
   *pofstreamCPP << " " << stringOrientation;
   *pofstreamCPP << " " << stringPrintMode;
   *pofstreamCPP << " " << stringResolution;
   *pofstreamCPP << " " << stringTray;
#ifdef INCLUDE_JP_COMMON_COPIES
   *pofstreamCPP << " " << stringCopies;
#endif
#ifdef INCLUDE_JP_COMMON_NUP
   *pofstreamCPP << " " << stringNUp;
#endif
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   *pofstreamCPP << " " << stringOutputBin;
#endif
#ifdef INCLUDE_JP_COMMON_SCALING
   *pofstreamCPP << " " << stringScaling;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   *pofstreamCPP << " " << stringSheetCollate;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   *pofstreamCPP << " " << stringSide;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   *pofstreamCPP << " " << stringStitching;
#endif
#ifdef INCLUDE_JP_COMMON_TRIMMING
   *pofstreamCPP << " " << stringTrimming;
#endif

   // process optional other(s)
   while (nodeOther)
   {
      const char *pszId = bundleStringData (nodeOther);

      *pofstreamCPP << " " << pszId;

      // Move to the next node
      nodeOther = XMLNextNode (nodeOther);

      if (pszId)
      {
         free ((void *)pszId);
      }
   }

   *pofstreamCPP << "\")" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   commonInit ();" << std::endl;
   *pofstreamCPP << "}" << std::endl;
   *pofstreamCPP << std::endl;
   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << *pstringNameOut << " (PSZCRO pszJobProperties)" << std::endl;
   *pofstreamCPP << "   : PrintDevice (\"" << pszDriverName << "\"," << std::endl;
   *pofstreamCPP << "                  \"" << pszDeviceName << "\"," << std::endl;
   *pofstreamCPP << "                  \"" << *pstringNameOut << "\"," << std::endl;
   *pofstreamCPP << "                  \"lib" << *pstringNameOut << ".so\"," << std::endl;
   *pofstreamCPP << "                  OMNI_CLASS_COMPILED," << std::endl;
   *pofstreamCPP << "                  pszJobProperties)" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   commonInit ();" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "~" << *pstringNameOut << " ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   DebugOutput::logMessage (LOG_INFO, \"deleted " << *pstringNameOut << "\");" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   iCount = listDeviceOptions.size ();
   if (0 < iCount)
   {
      bool fFirst = true;

      *pofstreamCPP << std::endl;
      *pofstreamCPP << "bool " << *pstringNameOut << "::" << std::endl;
      *pofstreamCPP << "hasDeviceOption (PSZCRO pszDeviceOption)" << std::endl;
      *pofstreamCPP << "{" << std::endl;

      for ( DriverList::iterator next = listDeviceOptions.begin () ;
            next != listDeviceOptions.end () ;
            next++, iCount-- )
      {
         if (fFirst)
         {
            *pofstreamCPP << "   if (  0 == strcmp (pszDeviceOption, \"" << *next << "\")" << std::endl;
            fFirst = false;
         }
         else
         {
            *pofstreamCPP << "      || 0 == strcmp (pszDeviceOption, \"" << *next << "\")" << std::endl;
         }
      }

      *pofstreamCPP << "      )" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return true;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   else" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      return false;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "}" << std::endl;
   }

#ifdef INCLUDE_JP_UPDF_BOOKLET
   // @TBD
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceBooklet * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultBooklet ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return 0;" << std::endl;
   *pofstreamCPP << "}" << std::endl;
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceCopies * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultCopies ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringCopyClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringCopyClassName << "::createS (this, \"" << stringCopies << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultCopies::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultCopies (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "PSZCRO " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultDitherID ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return \"" << pszDither << "\";" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceForm * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultForm ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return " << *pdi_d->pstringFormClassName << "::createS (this, \"" << stringForm << "\");"  << std::endl;
   *pofstreamCPP << "}" << std::endl;

#ifdef INCLUDE_JP_UPDF_JOGGING
   // @TBD
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceJogging * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultJogging ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringJoggingClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringJoggingClassName << "::createS (this, \"" << stringJogging << "\");"  << std::endl;
   }
   else
   {
      *pofstreamCPP << "   return 0;" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceMedia * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultMedia ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringMediaClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringMediaClassName << "::createS (this, \"" << stringMedia << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultMedia::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultMedia (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;

#ifdef INCLUDE_JP_COMMON_NUP
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceNUp * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultNUp ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringNUpClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringNUpClassName << "::createS (this, \"" << stringNUp << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultNUp::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultNUp (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceOrientation * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultOrientation ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringOrientationClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringOrientationClassName << "::createS (this, \"" << stringOrientation << "\");" << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultOrientation::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultOrientation (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceOutputBin * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultOutputBin ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringOutputBinClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringOutputBinClassName << "::createS (this, \"" << stringOutputBin << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultOutputBin::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultOutputBin (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DevicePrintMode * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultPrintMode ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return " << *pdi_d->pstringPrintModeClassName << "::createS (this, \"" << stringPrintMode << "\");"  << std::endl;
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceResolution * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultResolution ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return " << *pdi_d->pstringResolutionClassName << "::createS (this, \"" << stringResolution << "\");"  << std::endl;
   *pofstreamCPP << "}" << std::endl;

#ifdef INCLUDE_JP_COMMON_SCALING
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceScaling * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultScaling ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringScalingClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringScalingClassName << "::createS (this, \"" << stringScaling << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultScaling::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultScaling (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceSheetCollate * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultSheetCollate ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringSheetCollateClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringSheetCollateClassName << "::createS (this, \"" << stringSheetCollate << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultSheetCollate::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultSheetCollate (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceSide * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultSide ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringSideClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringSideClassName << "::createS (this, \"" << stringSide << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultSide::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultSide (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceStitching * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultStitching ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringStitchingClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringStitchingClassName << "::createS (this, \"" << stringStitching << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultStitching::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultStitching (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceTray * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultTray ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringTrayClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringTrayClassName << "::createS (this, \"" << stringTray << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultTray::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultTray (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;

#ifdef INCLUDE_JP_COMMON_TRIMMING
   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceTrimming * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultTrimming ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringTrimmingClassName)
   {
      *pofstreamCPP << "   return " << *pdi_d->pstringTrimmingClassName << "::createS (this, \"" << stringTrimming << "\");"  << std::endl;
   }
   else
   {
      std::ostringstream oss;

      DefaultTrimming::writeDefaultJP (oss);

      *pofstreamCPP << "   return new DefaultTrimming (this, \"" << oss.str () << "\");" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;
#endif

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceGamma * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getCurrentGamma ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringGammaClassName)
   {
      *pofstreamCPP << "   JobProperties          jobProp (\"\"); // @BUG?" << std::endl;
      *pofstreamCPP << "   JobPropertyEnumerator *pEnum                      = 0;" << std::endl;
      *pofstreamCPP << "   std::string           *pstringRet                 = 0;" << std::endl;
      *pofstreamCPP << "   PSZRO                  pszResolution              = 0;" << std::endl;
      *pofstreamCPP << "   PSZRO                  pszMedia                   = 0;" << std::endl;
      *pofstreamCPP << "   PSZRO                  pszPrintMode               = 0;" << std::endl;
      *pofstreamCPP << "   DeviceGamma           *pRet                       = 0;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pstringRet = getCurrentResolution ()->getJobProperties ();" << std::endl;
      *pofstreamCPP << "   if (pstringRet)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      jobProp.setJobProperties (pstringRet->c_str ());" << std::endl;
      *pofstreamCPP << "      delete pstringRet;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   pstringRet = getCurrentMedia ()->getJobProperties ();" << std::endl;
      *pofstreamCPP << "   if (pstringRet)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      jobProp.setJobProperties (pstringRet->c_str ());" << std::endl;
      *pofstreamCPP << "      delete pstringRet;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   pstringRet = getCurrentPrintMode ()->getJobProperties ();" << std::endl;
      *pofstreamCPP << "   if (pstringRet)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      jobProp.setJobProperties (pstringRet->c_str ());" << std::endl;
      *pofstreamCPP << "      delete pstringRet;" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   pEnum = jobProp.getEnumeration ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   while (pEnum->hasMoreElements ())" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      PSZCRO pszKey   = pEnum->getCurrentKey ();" << std::endl;
      *pofstreamCPP << "      PSZCRO pszValue = pEnum->getCurrentValue ();" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      if (0 == strcmp (pszKey, \"Resolution\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pszResolution = strdup (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 == strcmp (pszKey, \"media\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pszMedia = strdup (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << "      else if (0 == strcmp (pszKey, \"printmode\"))" << std::endl;
      *pofstreamCPP << "      {" << std::endl;
      *pofstreamCPP << "         pszPrintMode = strdup (pszValue);" << std::endl;
      *pofstreamCPP << "      }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "      pEnum->nextElement ();" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   delete pEnum;" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (  pszResolution" << std::endl;
      *pofstreamCPP << "      && pszMedia" << std::endl;
      *pofstreamCPP << "      && pszPrintMode" << std::endl;
      *pofstreamCPP << "      )" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      pRet = " << *pdi_d->pstringGammaClassName << "::getDeviceGamma (pszResolution," << std::endl;
      *pofstreamCPP << "                               pszMedia," << std::endl;
      *pofstreamCPP << "                               pszPrintMode," << std::endl;
      *pofstreamCPP << "                               getCurrentDitherID ());" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   if (pszResolution)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      free ((void *)pszResolution);" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   if (pszMedia)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      free ((void *)pszMedia);" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << "   if (pszPrintMode)" << std::endl;
      *pofstreamCPP << "   {" << std::endl;
      *pofstreamCPP << "      free ((void *)pszPrintMode);" << std::endl;
      *pofstreamCPP << "   }" << std::endl;
      *pofstreamCPP << std::endl;
      *pofstreamCPP << "   return pRet;" << std::endl;
   }
   else
   {
      *pofstreamCPP << "   return 0;" << std::endl;
   }
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceCommand * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultCommands ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   *pofstreamCPP << "   return new " << *pdi_d->pstringCommandClassName << " ();" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceData * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultData ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringDataClassName)
      *pofstreamCPP << "   return new " << *pdi_d->pstringDataClassName << " ();" << std::endl;
   else
      *pofstreamCPP << "   return 0;" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   *pofstreamCPP << std::endl;
   *pofstreamCPP << "DeviceString * " << *pstringNameOut << "::" << std::endl;
   *pofstreamCPP << "getDefaultString ()" << std::endl;
   *pofstreamCPP << "{" << std::endl;
   if (pdi_d->pstringStringClassName)
      *pofstreamCPP << "   return new " << *pdi_d->pstringStringClassName << " ();" << std::endl;
   else
      *pofstreamCPP << "   return 0;" << std::endl;
   *pofstreamCPP << "}" << std::endl;

   if (!fAutoconf_d)
   {
      openMakeFile (pstringNameOut, &pofstreamMAK);

      *pofstreamMAK << "DEVICENAME            = " << *pstringNameOut << std::endl;
      *pofstreamMAK << "DEVCFILES             = " << *pdi_d->pstringPrintModeClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringResolutionClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringOrientationClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringCommandClassName << "~ \\" << std::endl;
      if (pdi_d->pstringGammaClassName)
         *pofstreamMAK << "                        " << *pdi_d->pstringGammaClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringTrayClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringFormClassName << "~ \\" << std::endl;
      if (pdi_d->pstringInstanceClassName)
         *pofstreamMAK << "                        " << *pdi_d->pstringInstanceClassName << "~ \\" << std::endl;
      if (pdi_d->pstringBlitterClassName)
         *pofstreamMAK << "                        " << *pdi_d->pstringBlitterClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pdi_d->pstringMediaClassName << "~ \\" << std::endl;
      if (pdi_d->pstringDataClassName)
         *pofstreamMAK << "                        " << *pdi_d->pstringDataClassName << "~ \\" << std::endl;
      if (pdi_d->pstringStringClassName)
         *pofstreamMAK << "                        " << *pdi_d->pstringStringClassName << "~ \\" << std::endl;
      *pofstreamMAK << "                        " << *pstringNameOut << "~" << std::endl;
      *pofstreamMAK << "OMNI_PATH             = .." << std::endl;
      *pofstreamMAK << std::endl;
      *pofstreamMAK << "all:    lib$(DEVICENAME).so" << std::endl;
      *pofstreamMAK << std::endl;
      *pofstreamMAK << "include ../common.mak" << std::endl;
      *pofstreamMAK << std::endl;
      *pofstreamMAK << *pstringNameOut
                    << ".o:\t"
                    << *pstringNameOut
                    << ".cpp ";
      if (pdi_d->pstringBlitterClassName)
         *pofstreamMAK << *pdi_d->pstringBlitterClassName << ".hpp ";
      if (pdi_d->pstringInstanceClassName)
         *pofstreamMAK << *pdi_d->pstringInstanceClassName << ".hpp ";
      *pofstreamMAK << *pdi_d->pstringFormClassName
                    << ".o "
                    << *pdi_d->pstringResolutionClassName
                    << ".o "
                    << *pdi_d->pstringMediaClassName
                    << ".o "
                    << *pdi_d->pstringTrayClassName
                    << ".o "
                    << *pdi_d->pstringOrientationClassName
                    << ".o "
                    << *pdi_d->pstringCommandClassName
                    << ".o "
                    << "../Device.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringFormClassName
                    << ".o:\t"
                    << *pdi_d->pstringFormClassName
                    << ".cpp "
                    << *pdi_d->pstringFormClassName
                    << ".hpp "
                    << "../DeviceForm.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringResolutionClassName
                    << ".o:\t"
                    << *pdi_d->pstringResolutionClassName
                    << ".cpp "
                    << *pdi_d->pstringResolutionClassName
                    << ".hpp "
                    << "../DeviceResolution.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringMediaClassName
                    << ".o:\t"
                    << *pdi_d->pstringMediaClassName
                    << ".cpp "
                    << *pdi_d->pstringMediaClassName
                    << ".hpp "
                    << "../DeviceMedia.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringTrayClassName
                    << ".o:\t"
                    << *pdi_d->pstringTrayClassName
                    << ".cpp "
                    << *pdi_d->pstringTrayClassName
                    << ".hpp "
                    << "../DeviceTray.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringOrientationClassName
                    << ".o:\t"
                    << *pdi_d->pstringOrientationClassName
                    << ".cpp "
                    << *pdi_d->pstringOrientationClassName
                    << ".hpp "
                    << "../DeviceOrientation.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringCommandClassName
                    << ".o:\t"
                    << *pdi_d->pstringCommandClassName
                    << ".cpp "
                    << *pdi_d->pstringCommandClassName
                    << ".hpp "
                    << "../DeviceCommand.o"
                    << std::endl;
      *pofstreamMAK << *pdi_d->pstringPrintModeClassName
                    << ".o:\t"
                    << *pdi_d->pstringPrintModeClassName
                    << ".cpp "
                    << *pdi_d->pstringPrintModeClassName
                    << ".hpp "
                    << "../DevicePrintMode.o"
                    << std::endl;
//////*pofstreamMAK << *pdi_d->pstringConnectionClassName
//////              << ".o:\t"
//////              << *pdi_d->pstringConnectionClassName
//////              << ".cpp "
//////              << *pdi_d->pstringConnectionClassName
//////              << ".hpp "
//////              << "../DeviceConnection.o"
//////              << std::endl;
      if (pdi_d->pstringGammaClassName != 0)
      {
         *pofstreamMAK << *pdi_d->pstringGammaClassName
                       << ".o:\t"
                       << *pdi_d->pstringGammaClassName
                       << ".cpp "
                       << *pdi_d->pstringGammaClassName
                       << ".hpp "
                       << *pdi_d->pstringResolutionClassName
                       << ".hpp "
                       << *pdi_d->pstringMediaClassName
                       << ".hpp "
                       << *pdi_d->pstringPrintModeClassName
                       << ".hpp "
                       << "../DeviceGammaTable.hpp "
                       << "../DeviceGamma.hpp "
                       << "../DeviceResolution.hpp "
                       << "../DeviceMedia.hpp "
                       << "../DevicePrintMode.hpp "
                       << "../Enumeration.hpp "
                       << "../BinaryData.hpp "
                       << "../DeviceDither.hpp "
                       << "../defines.hpp"
                       << std::endl;
      }
      if (pdi_d->pstringBlitterClassName)
      {
         *pofstreamMAK << *pdi_d->pstringBlitterClassName
                       << ".o:\t"
                       << *pdi_d->pstringBlitterClassName
                       << ".cpp "
                       << *pdi_d->pstringBlitterClassName
                       << ".hpp "
                       << "../DeviceBlitter.o ";
         if (pdi_d->pstringInstanceClassName)
            *pofstreamMAK << *pdi_d->pstringInstanceClassName
                          << ".hpp ";
         *pofstreamMAK << *pdi_d->pstringPrintModeClassName
                       << ".hpp "
                       << *pdi_d->pstringResolutionClassName
                       << ".hpp "
                       << "../JobProperties.hpp"
                       << std::endl;
      }
      if (pdi_d->pstringInstanceClassName)
      {
         *pofstreamMAK << *pdi_d->pstringInstanceClassName
                       << ".o:\t"
                       << *pdi_d->pstringInstanceClassName
                       << ".cpp "
                       << *pdi_d->pstringInstanceClassName
                       << ".hpp "
                       << "../DeviceInstance.o"
                       << std::endl;
      }
      if (pdi_d->pstringDataClassName)
      {
         *pofstreamMAK << *pdi_d->pstringDataClassName
                       << ".o:\t"
                       << *pdi_d->pstringDataClassName
                       << ".cpp "
                       << *pdi_d->pstringDataClassName
                       << ".hpp "
                       << "../DeviceData.o"
                       << std::endl;
      }
      if (pdi_d->pstringStringClassName)
      {
         *pofstreamMAK << *pdi_d->pstringStringClassName
                       << ".o:\t"
                       << *pdi_d->pstringStringClassName
                       << ".cpp "
                       << *pdi_d->pstringStringClassName
                       << ".hpp "
                       << "../DeviceString.o"
                       << std::endl;
      }
   }

done:
   if (!rc)
   {
      std::cerr << "Error: Could not traverse the tree for \"" << *pstrXMLFile_d << "\"" << std::endl;
      setErrorCondition ();

      XMLPrintDocument (XMLGetDocNode (driverNode), 0);
   }

   // Clean up!
   if (pszDeviceName)
   {
      free ((void *)pszDeviceName);
   }
   if (pszDriverName)
   {
      free ((void *)pszDriverName);
   }
   if (pszOrientation)
   {
      free ((void *)pszOrientation);
   }
   if (pszForm)
   {
      free ((void *)pszForm);
   }
   if (pszJogging)
   {
      free ((void *)pszJogging);
   }
   if (pszTray)
   {
      free ((void *)pszTray);
   }
   if (pszMedia)
   {
      free ((void *)pszMedia);
   }
   if (pszResolution)
   {
      free ((void *)pszResolution);
   }
   if (pszDither)
   {
      free ((void *)pszDither);
   }
   if (pszPrintMode)
   {
      free ((void *)pszPrintMode);
   }
   if (pszOutputBin)
   {
      free ((void *)pszOutputBin);
   }
   if (pszSide)
   {
      free ((void *)pszSide);
   }
   if (pszStitchingPosition)
   {
      free ((void *)pszStitchingPosition);
   }
   if (pszStitchingRefEdge)
   {
      free ((void *)pszStitchingRefEdge);
   }
   if (pszStitchingType)
   {
      free ((void *)pszStitchingType);
   }
   if (pszStitchingCount)
   {
      free ((void *)pszStitchingCount);
   }
   if (pszStitchingAngle)
   {
      free ((void *)pszStitchingAngle);
   }
   if (pszSheetCollate)
   {
      free ((void *)pszSheetCollate);
   }
   if (pszNUpX)
   {
      free ((void *)pszNUpX);
   }
   if (pszNUpY)
   {
      free ((void *)pszNUpY);
   }
   if (pszNUpDirection)
   {
      free ((void *)pszNUpDirection);
   }
   if (pszCopies)
   {
      free ((void *)pszCopies);
   }
   if (pszTrimming)
   {
      free ((void *)pszTrimming);
   }
   if (pszScalingPercentage)
   {
      free ((void *)pszScalingPercentage);
   }
   if (pszScalingType)
   {
      free ((void *)pszScalingType);
   }
   if (pszMajorPDLLevel)
   {
      free ((void *)pszMajorPDLLevel);
   }
   if (pszMinorPDLLevel)
   {
      free ((void *)pszMinorPDLLevel);
   }
   if (pszMajorRevisionLevel)
   {
      free ((void *)pszMajorRevisionLevel);
   }
   if (pszMinorRevisionLevel)
   {
      free ((void *)pszMinorRevisionLevel);
   }

   delete pofstreamHPP;
   delete pofstreamCPP;
   delete pofstreamMAK;
   delete pstringNameOut;

   return rc;
}

void OmniDomParser::
processDOMNode (XmlNodePtr node)
{
   PSZCRO pszElementName = XMLGetName (node);

   if (  0 == strcmp ("Uses", pszElementName)
      || 0 == strcmp ("Has",  pszElementName)
      )
   {
      PSZCRO pszFilename = XMLNodeListGetString (XMLGetDocNode (node),
                                                 XMLGetChildrenNode (node),
                                                 1);

      OmniDomParser parser (pszFilename,
                            pdi_d,
                            fAutoconf_d,
                            fAutoconfNoInst_d);

      parser.parse ();

      fFileModified_d = fFileModified_d || parser.fileModified ();

      if (pszFilename)
      {
         free ((void *)pszFilename);
      }
   }
   else if (0 == strcmp ("deviceCopies", pszElementName))
      processDeviceCopies (node);
   else if (0 == strcmp ("deviceForms", pszElementName))
      processDeviceForms (node);
   else if (0 == strcmp ("deviceMedias", pszElementName))
      processDeviceMedias (node);
   else if (0 == strcmp ("deviceNumberUps", pszElementName))
      processDeviceNUps (node);
   else if (0 == strcmp ("deviceOrientations", pszElementName))
      processDeviceOrientations (node);
   else if (0 == strcmp ("deviceOutputBins", pszElementName))
      processDeviceOutputBins (node);
   else if (0 == strcmp ("devicePrintModes", pszElementName))
      processDevicePrintModes (node);
   else if (0 == strcmp ("deviceResolutions", pszElementName))
      processDeviceResolutions (node);
   else if (0 == strcmp ("deviceScalings", pszElementName))
      processDeviceScalings (node);
   else if (0 == strcmp ("deviceSheetCollates", pszElementName))
      processDeviceSheetCollates (node);
   else if (0 == strcmp ("deviceSides", pszElementName))
      processDeviceSides (node);
   else if (0 == strcmp ("deviceStitchings", pszElementName))
      processDeviceStitchings (node);
   else if (0 == strcmp ("deviceTrays", pszElementName))
      processDeviceTrays (node);
   else if (0 == strcmp ("deviceTrimmings", pszElementName))
      processDeviceTrimmings (node);
   else if (0 == strcmp ("deviceConnections", pszElementName))
      processDeviceConnections (node);
   else if (0 == strcmp ("deviceGammaTables", pszElementName))
   {
      if (  pdi_d->pstringResolutionClassName
         && pdi_d->pstringMediaClassName
         && pdi_d->pstringPrintModeClassName
         )
      {
         processDeviceGammas (node);
      }
      else
      {
         std::cout << "Skipping:   \"" << *pstrXMLFile_d << "\"" << std::endl;

         pdi_d->pstrGammaTablesXMLFile_d = new std::string (*pstrXMLFile_d);
      }
   }
   else if (0 == strcmp ("deviceCommands", pszElementName))
      processDeviceCommands (node);
   else if (0 == strcmp ("deviceDatas", pszElementName))
      processDeviceDatas (node);
   else if (0 == strcmp ("deviceStrings", pszElementName))
      processDeviceStrings (node);
   else if (0 == strcmp ("Device", pszElementName))
   {
      XmlNodePtr nodeCurrent = XMLFirstNode (XMLGetChildrenNode (node));

      while (nodeCurrent)
      {
#ifdef DEBUG
/////////if (fDebugOutput)
/////////{
/////////   std::cerr << "processDOMNode: " << std::endl;
/////////   printXMLNode (nodeCurrent);
/////////}
#endif

         processDOMNode (nodeCurrent);

         // Move to the next one
         nodeCurrent = XMLNextNode (nodeCurrent);
      }

      processDriver (node);
   }
}

bool OmniDomParser::
parse ()
{
   PSZCRO pszXMLFile = pstrXMLFile_d->c_str ();

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "DOM Parsing: " << pszXMLFile << std::endl;
#endif

   doc_d = XMLParseFile (pszXMLFile);

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "doc_d = 0x" << std::hex << (int)doc_d << std::dec << std::endl;
#endif

   if (doc_d)
   {
      if (XMLValidateFile (doc_d, this))
      {
#ifdef DEBUG
         if (fDebugOutput) std::cerr << "Document validated." << std::endl;
#endif
      }

      processDOMNode (XMLDocGetRootElement (doc_d));

      if (  pdi_d->pstrGammaTablesXMLFile_d
         && pdi_d->pstringResolutionClassName
         && pdi_d->pstringMediaClassName
         && pdi_d->pstringPrintModeClassName
         )
      {
         std::string stringGammaTablesXMLFile = std::move(*(pdi_d->pstrGammaTablesXMLFile_d));

//          delete pdi_d->pstrGammaTablesXMLFile_d;
//          pdi_d->pstrGammaTablesXMLFile_d = 0;

         OmniDomParser parser (stringGammaTablesXMLFile.c_str (),
                               pdi_d,
                               fAutoconf_d,
                               fAutoconfNoInst_d);

         parser.parse ();
      }
   }
   else
   {
      std::cerr << "Error: File not found \"" << pszXMLFile << "\"" << std::endl;

      fErrorCondition_d = true;
   }

   return !fErrorCondition_d;
}
