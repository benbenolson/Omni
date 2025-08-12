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
#include "Device.hpp"
#include "PrintDevice.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <glib.h>
#include <gmodule.h>

// Typedefs to dither prototypes...
typedef DeviceDither *     (*PFNCREATEDITHERINSTANCE) (PSZCRO            pszDitherType,
                                                       Device           *pDevice,
                                                       PSZCRO            pszOptions);
typedef std::string *      (*PFNGETCREATEHASH)        (PSZRO             pszJobProperties);
typedef PSZCRO             (*PFNGETIDFROMCREATEHASH)  (std::string      *pstringHash);

typedef int                (*PFNDITHERNAMEVALID)      (PSZCRO            pszId);
typedef bool               (*PFNDITHERCATAGORYVALID)  (PSZCRO            pszId);
typedef PSZCRO             (*PFNGETDITHERCATAGORY)    (PSZCRO            pszId);
// @TBD - determine language
typedef PSZCRO             (*PFNGETDITHERNAME)        (PSZCRO            pszId);

typedef Enumeration *      (*PFNGETALLENUMERATION)    ();

// Function prototypes...
bool   ditherLibraryValid    (PSZCRO            pszName);
char  *queryLibrary          (char             *pszFillIn,
                              PSZCRO            pszID);
PSZ    convert               (char             *pszFillIn,
                              PSZCRO            pszPFN,
                              PSZ               pszLibraryName);
char  *truncate              (char             *pszLibraryName);

std::string * DeviceDither::
getDitherValue (PSZRO pszJobProperties)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   std::string           *pstringRet                 = 0;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, "dither"))
      {
         delete pstringRet;

         pstringRet = new std::string (pszValue);
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return pstringRet;
}

std::string * DeviceDither::
getDitherJobProperties (PSZRO pszDitherID)
{
   std::ostringstream oss;

   if (  pszDitherID
      && *pszDitherID
      )
   {
      oss << "dither=" << pszDitherID;
   }

   return new std::string (oss.str ());
}

DeviceDither::
DeviceDither (Device *pDevice)
{
   pDevice_d = pDevice;
}

DeviceDither::
~DeviceDither ()
{
   pDevice_d = 0;
}

char *
truncate (char *pszLibraryName)
{
   if (  0 == strncmp ("lib", pszLibraryName, 3)                           // has the head part
      && 0 == strcmp (".so", pszLibraryName + strlen (pszLibraryName) - 3) // has the tail part
      )
   {
      pszLibraryName += 3;                                // remove the head part
      pszLibraryName[strlen (pszLibraryName) - 3] = '\0'; // remove the tail part
   }

   return pszLibraryName;
}

PSZCRO DeviceDither::
getName (Device      *pDevice,
         PSZCRO       pszJobProperties)
{
   char         achLibraryName[512]; // @TBD
   char         achFN[512];          // @TBD
   std::string *pstringId            = 0;
   PSZRO        pszRet               = 0;

   pstringId = getDitherValue (pszJobProperties);

   if (!pstringId)
   {
      return pszRet;
   }

   pszRet = StringResource::getString (pDevice->getLanguageResource (),
                                       StringResource::STRINGGROUP_DITHERS,
                                       pstringId->c_str ());

   if (!pszRet)
   {
      PSZ pszLibraryName = queryLibrary (achLibraryName, pstringId->c_str ());

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getName: queryLibrary (" << *pstringId << ") = " << pszLibraryName << std::endl;
#endif

      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "DeviceDither::getDitherName: This program needs glib's module routines!" << std::endl;

         return 0;
      }

      if (pszLibraryName)
      {
         PFNGETDITHERNAME  pfnGetDitherName = 0;
         GModule          *hModLibrary      = 0;

         hModLibrary = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherName: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary << std::endl;
#endif

         if (hModLibrary)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherName: g_module_symbol (" << convert (achFN, "getDitherName", pszLibraryName) << ")";
#endif

            g_module_symbol (hModLibrary,
                             convert (achFN,
                                      "getDitherName",
                                      pszLibraryName),
                             (gpointer *)&pfnGetDitherName);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnGetDitherName << std::endl;
#endif

            if (pfnGetDitherName)
            {
               pszRet = pfnGetDitherName (pstringId->c_str ());

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherName: it returns " << pszRet << std::endl;
#endif
            }

            g_module_close (hModLibrary);
         }
      }
   }

   delete pstringId;

   return pszRet;
}

PSZCRO DeviceDither::
getName ()
{
   return StringResource::getString (pDevice_d->getLanguageResource (),
                                     StringResource::STRINGGROUP_DITHERS,
                                     getID ());
}

DeviceDither * DeviceDither::
createDitherInstance (PSZCRO  pszDitherType,
                      Device *pDevice,
                      PSZCRO  pszOptions)
{
   char         achLibraryName[512]; // @TBD
   char         achFN[512];          // @TBD
   PSZ          pszLibraryName      = 0;
   PrintDevice *pPrintDevice        = dynamic_cast <PrintDevice *>(pDevice);

   if (!GplDitherInstance::ditherNameValid (pszDitherType))
      pszLibraryName = queryLibrary (achLibraryName, pszDitherType);

   if (  pPrintDevice
      && pszLibraryName
      )
   {
      PFNCREATEDITHERINSTANCE pfnCreateDitherInstance = 0;

      pPrintDevice->loadLibrary (pszLibraryName);

      pfnCreateDitherInstance = (PFNCREATEDITHERINSTANCE)pPrintDevice->dlsym (pszLibraryName,
                                                                              convert (achFN,
                                                                                       "createDitherInstance",
                                                                                       pszLibraryName));

      if (pfnCreateDitherInstance)
      {
         return pfnCreateDitherInstance (pszDitherType,
                                         pDevice,
                                         pszOptions);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::createDitherInstance: Failed to find createDitherInstance in " << pszLibraryName << std::endl;
#endif
         return 0;
      }
   }

   return GplDitherInstance::createDitherInstance (pszDitherType,
                                                   pDevice,
                                                   pszOptions);
}

std::string * DeviceDither::
getCreateHash (PSZRO pszJobProperties)
{
   char         achLibraryName[512]; // @TBD
   char         achFN[512];          // @TBD
   std::string *pstringRet          = 0;

   pstringRet = GplDitherInstance::getCreateHash (pszJobProperties);

   if (!pstringRet)
   {
      std::string *pstringValue = getDitherValue (pszJobProperties);

      if (!pstringValue)
      {
         return pstringRet;
      }

      PSZRO pszId          = pstringValue->c_str ();
      PSZ   pszLibraryName = queryLibrary (achLibraryName, pszId);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: queryLibrary (" << pszId << ") = " << pszLibraryName << std::endl;
#endif

      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: This program needs glib's module routines!" << std::endl;

         return pstringRet;
      }

      if (pszLibraryName)
      {
         PFNGETCREATEHASH  pfnGetCreateHash = 0;
         GModule          *hModLibrary      = 0;

         hModLibrary = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary << std::endl;
#endif

         if (hModLibrary)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: g_module_symbol (" << convert (achFN, "getCreateHash", pszLibraryName) << ")";
#endif

            g_module_symbol (hModLibrary,
                             convert (achFN,
                                      "getCreateHash",
                                      pszLibraryName),
                             (gpointer *)&pfnGetCreateHash);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnGetCreateHash << std::endl;
#endif

            if (pfnGetCreateHash)
            {
               pstringRet = pfnGetCreateHash (pszJobProperties);

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getCreateHash: it returns " << pstringRet << std::endl;
#endif
            }

            g_module_close (hModLibrary);
         }
      }

      delete pstringValue;
   }

   return pstringRet;
}

PSZCRO DeviceDither::
getIDFromCreateHash (std::string *pstringHash)
{
///char         achLibraryName[512]; // @TBD
///char         achFN[512];          // @TBD
   PSZRO        pszRet              = 0;

   pszRet = GplDitherInstance::getIDFromCreateHash (pstringHash);

   if (!pszRet)
   {
      // @TBD - enumerate all dither libraries and successively call until success!
   }

   return pszRet;
}

// This is here for consistency's sake... So that the code looks similiar
// to objects which use numeric IDs instead of string IDs.
PSZCRO DeviceDither::
IDToName (PSZCRO pszId)
{
   return pszId;
}

bool DeviceDither::
ditherNameValid (PSZCRO pszId)
{
   char achLibraryName[512]; // @TBD
   char achFN[512];          // @TBD
   bool fRet;

   fRet = GplDitherInstance::ditherNameValid (pszId);

   if (!fRet)
   {
      PSZ pszLibraryName = queryLibrary (achLibraryName, pszId);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherNameValid: queryLibrary (" << pszId << ") = " << pszLibraryName << std::endl;
#endif

      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "DeviceDither::ditherNameValid: This program needs glib's module routines!" << std::endl;

         return false;
      }

      if (pszLibraryName)
      {
         PFNDITHERNAMEVALID  pfnDitherNameValid = 0;
         GModule            *hModLibrary        = 0;

         hModLibrary = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherNameValid: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary << std::endl;
#endif

         if (hModLibrary)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherNameValid: g_module_symbol (" << convert (achFN, "ditherNameValid", pszLibraryName) << ")";
#endif

            g_module_symbol (hModLibrary,
                             convert (achFN,
                                      "ditherNameValid",
                                      pszLibraryName),
                             (gpointer *)&pfnDitherNameValid);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnDitherNameValid << std::endl;
#endif

            if (pfnDitherNameValid)
            {
               fRet = pfnDitherNameValid (pszId);

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherNameValid: it returns " << fRet << std::endl;
#endif
            }

            g_module_close (hModLibrary);
         }
      }
   }

   return fRet;
}

PSZCRO DeviceDither::
getDitherCatagory (PSZCRO pszId)
{
   char   achLibraryName[512]; // @TBD
   char   achFN[512];          // @TBD
   PSZRO  pszRet               = 0;

   pszRet = GplDitherInstance::getDitherCatagory (pszId);

   if (!pszRet)
   {
      PSZ pszLibraryName = queryLibrary (achLibraryName, pszId);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: queryLibrary (" << pszId << ") = " << pszLibraryName << std::endl;
#endif

      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: This program needs glib's module routines!" << std::endl;

         return pszRet;
      }

      if (pszLibraryName)
      {
         PFNGETDITHERCATAGORY  pfnGetDitherCatagory = 0;
         GModule              *hModLibrary          = 0;

         hModLibrary = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary << std::endl;
#endif

         if (hModLibrary)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: g_module_symbol (" << convert (achFN, "getDitherCatagory", pszLibraryName) << ")";
#endif

            g_module_symbol (hModLibrary,
                             convert (achFN,
                                      "getDitherCatagory",
                                      pszLibraryName),
                             (gpointer *)&pfnGetDitherCatagory);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnGetDitherCatagory << std::endl;
#endif

            if (pfnGetDitherCatagory)
            {
               pszRet = pfnGetDitherCatagory (pszId);

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getDitherCatagory: it returns " << pszRet << std::endl;
#endif
            }

            g_module_close (hModLibrary);
         }
      }
   }

   return pszRet;
}

bool DeviceDither::
ditherCatagoryValid (PSZCRO pszId)
{
   char   achLibraryName[512]; // @TBD
   char   achFN[512];          // @TBD
   bool   fRet;

   fRet = GplDitherInstance::ditherCatagoryValid (pszId);

   if (!fRet)
   {
      PSZ pszLibraryName = queryLibrary (achLibraryName, pszId);

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherCatagoryValid: queryLibrary (" << pszId << ") = " << pszLibraryName << std::endl;
#endif

      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "DeviceDither::ditherCatagoryValid: This program needs glib's module routines!" << std::endl;

         return false;
      }

      if (pszLibraryName)
      {
         PFNDITHERCATAGORYVALID  pfnDitherCatagoryValid = 0;
         GModule                *hModLibrary            = 0;

         hModLibrary = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherCatagoryValid: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hModLibrary << std::endl;
#endif

         if (hModLibrary)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherCatagoryValid: g_module_symbol (" << convert (achFN, "ditherCatagoryValid", pszLibraryName) << ")";
#endif

            g_module_symbol (hModLibrary,
                             convert (achFN,
                                      "ditherCatagoryValid",
                                      pszLibraryName),
                             (gpointer *)&pfnDitherCatagoryValid);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnDitherCatagoryValid << std::endl;
#endif

            if (pfnDitherCatagoryValid)
            {
               fRet = pfnDitherCatagoryValid (pszId);

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::ditherCatagoryValid: it returns " << fRet << std::endl;
#endif
            }

            g_module_close (hModLibrary);
         }
      }
   }

   return fRet;
}

class DitherEnumerator : public Enumeration
{
private:
   typedef struct _NameEntry {
      struct _NameEntry *pNext;
      char               achName[1];
   } __attribute__ ((aligned (1))) __attribute__ ((packed)) NAMEENTRY, *PNAMEENTRY;

   #define NAMEENTRY_HEADER (sizeof (((PNAMEENTRY)0)->pNext))
   #define NAMEENTRY_BLOCK  4096

public:
   DitherEnumerator ()
   {
      pCurrentEnum_d = GplDitherInstance::getAllEnumeration ();
      pNames_d       = (PNAMEENTRY)calloc (1, NAMEENTRY_BLOCK); // fix from manolo@NCC-1701.B.shuttle.de
      pCurrentName_d = pNames_d;
      hLibrary_d     = 0;

      if (pNames_d)
      {
         int            cbNames            = NAMEENTRY_BLOCK;
         int            cbAllocated        = NAMEENTRY_BLOCK;
         char           achLine[512]; // @TBD
         std::ifstream  ifIn ("/etc/omni");
         char          *pszLine;
         PNAMEENTRY     pCurrent           = pNames_d;

         while (ifIn.getline (achLine, sizeof (achLine)))
         {
            pszLine = achLine;

            // Skip whitespace
            while (isspace (*pszLine))
               pszLine++;

            if ('#' == *pszLine)
            {
               // Found a comment
               continue;
            }
            else if (0 != strncmp (pszLine, "dither ", 7))
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither: Found non-dither " << pszLine << std::endl;
#endif
               continue;
            }

            pszLine += 7;

            // Skip the LHS
            while (*pszLine && !isspace (*pszLine))
               pszLine++;

            while (!isspace (*pszLine))
               continue;

            // Skip whitespace
            while (isspace (*pszLine))
               pszLine++;

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: Testing: " << pszLine << std::endl;
#endif

            if (!ditherLibraryValid (pszLine))
               continue;

            if (elementExists (pNames_d, pszLine))
               continue;

            int iLength = 6                // lib.so
                        + strlen (pszLine)
                        + 1;               // end-of-string

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: Adding: " << pszLine << std::endl;
#endif

            // Is there room of an element plus an empty end-of-list one?
            if (cbNames > iLength + (int)sizeof (NAMEENTRY))
            {
               pCurrent->pNext = (PNAMEENTRY)(pCurrent->achName + iLength);
               sprintf (pCurrent->achName, "lib%s.so", pszLine);
               cbNames -= iLength + NAMEENTRY_HEADER;

               pCurrent = pCurrent->pNext;

               // Terminate the list
               pCurrent->pNext = 0;
               pCurrent->achName[0] = '\0';
            }
            else
            {
               PNAMEENTRY pTemp;
               PNAMEENTRY pFrom;
               PNAMEENTRY pTo;

               cbAllocated += NAMEENTRY_BLOCK;

               // Allocate a new block
               pTemp = (PNAMEENTRY)calloc (1, cbAllocated);
               if (pTemp)
               {
                  cbNames = cbAllocated;

                  // Copy the old list to the new
                  pFrom = pNames_d;
                  pTo   = pTemp;
                  while (pFrom->achName[0])
                  {
                     iLength = strlen (pFrom->achName) + 1;
                     pTo->pNext = (PNAMEENTRY)(pTo->achName + iLength);
                     strcpy (pTo->achName, pFrom->achName);
                     cbNames -= iLength + NAMEENTRY_HEADER;

                     // Move to the next one
                     pFrom = pFrom->pNext;
                     pTo   = pTo->pNext;
                  }

                  // Add the new element
                  iLength = 6 + strlen (pszLine) + 1;
                  pTo->pNext = (PNAMEENTRY)(pTo->achName + iLength);
                  sprintf (pCurrent->achName, "lib%s.so", pszLine);
                  cbNames -= iLength + NAMEENTRY_HEADER;

                  // Terminate the list
                  pTo->pNext = 0;
                  pTo->achName[0] = 0;

                  // Update the root pointer
                  free (pNames_d);
                  pNames_d = pTemp;
               }
               else
               {
                  // Error!
                  free (pNames_d);
                  pNames_d = 0;
                  break;
               }
            }
         }
      }
   }

   virtual
   ~DitherEnumerator ()
   {
      // Clean up
      if (pCurrentEnum_d)
      {
         delete pCurrentEnum_d;
         pCurrentEnum_d = 0;
      }

      if (pNames_d)
      {
         free (pNames_d);
         pNames_d = 0;
      }

      if (hLibrary_d)
      {
         g_module_close (hLibrary_d);
         hLibrary_d = 0;
      }
   }

   virtual bool
   hasMoreElements ()
   {
      if (!pCurrentEnum_d)
      {
         // Nothing to enumerate
         return false;
      }

      if (pCurrentEnum_d->hasMoreElements ())
      {
         // Current enumeration has more elements
         return true;
      }

      // Move to the next one
      // First, clean up
      delete pCurrentEnum_d;
      pCurrentEnum_d = 0;

      if (hLibrary_d)
      {
         g_module_close (hLibrary_d);
         hLibrary_d = 0;
      }

      // Next, loop through our list of libraries
      while (  pCurrentName_d
            && pCurrentName_d->achName[0]
            )
      {
         char                 achFN[512];          // @TBD
         PFNGETALLENUMERATION pfnGetAllEnumeration;
         PSZ                  pszLibraryName;

         pszLibraryName = pCurrentName_d->achName;

         // Load it
         hLibrary_d = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: g_module_open (" << pszLibraryName << ") = 0x" << std::hex << (int)hLibrary_d << std::endl;
#endif

         if (hLibrary_d)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: g_module_symbol (" << convert (achFN, "getAllEnumeration", pszLibraryName) << ")";
#endif

            // Get the enumeration function
            g_module_symbol (hLibrary_d,
                             convert (achFN,
                                      "getAllEnumeration",
                                      pszLibraryName),
                             (gpointer *)&pfnGetAllEnumeration);

#ifndef RETAIL
            if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << " = 0x" << std::hex << (int)pfnGetAllEnumeration << std::endl;
#endif

            if (pfnGetAllEnumeration)
            {
               // Call it
               pCurrentEnum_d = pfnGetAllEnumeration ();

#ifndef RETAIL
               if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: pCurrentEnum_d = 0x" << std::hex << (int)pCurrentEnum_d << std::endl;
#endif
            }
         }

         // Move to the next name in the list
         pCurrentName_d = pCurrentName_d->pNext;

         if (pCurrentEnum_d)
         {
            // Success
            return true;
         }
      }

      return false;
   }

   virtual void *
   nextElement ()
   {
      void *pvRet = 0;

      if (pCurrentEnum_d)
      {
         pvRet = pCurrentEnum_d->nextElement ();
      }

      return pvRet;
   }

private:
   bool
   elementExists (PNAMEENTRY pRoot, PSZCRO pszElement)
   {
      PNAMEENTRY pItem = pRoot;

      if (!pItem)
      {
         return false;
      }

      while (pItem->achName[0])
      {
         if (0 == strncmp (pItem->achName + 3,   // Skip the "lib" head
                           pszElement,
                           strlen (pszElement))) // Only test for the name (which doesnt have the ".so" tail)
            return true;

         pItem = pItem->pNext;
      }

      return false;
   }

   Enumeration *pCurrentEnum_d;
   PNAMEENTRY   pNames_d;
   PNAMEENTRY   pCurrentName_d;
   GModule     *hLibrary_d;
};

Enumeration * DeviceDither::
getAllEnumeration ()
{
   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "DeviceDither::getAllEnumeration: This program needs glib's module routines!" << std::endl;

      return 0;
   }

   return new DitherEnumerator ();
}

static PSZCRO apszRoutines[] = {
   "createDitherInstance",
   "ditherCatagoryValid",
   "ditherNameValid",
   "getCreateHash",
   "getDitherCatagory",
   "getDitherName",
   "getAllEnumeration"
};

bool
ditherLibraryValid (PSZCRO pszName)
{
   char         achFileName[512]; // @TBD
   GModule     *hDevice;
   bool         fValid           = true;

   if (  !pszName
      || !*pszName
      )
   {
      return false;
   }

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "ditherLibraryValid: This program needs glib's module routines!" << std::endl;

      return false;
   }

   sprintf (achFileName, "lib%s.so", pszName);

   hDevice = g_module_open (achFileName, (GModuleFlags)0);

   if (hDevice)
   {
      gpointer ptr;

      for (int i = 0; i < (int)dimof (apszRoutines); i++)
      {
         if (!g_module_symbol (hDevice, apszRoutines[i], &ptr))
         {
            fValid = false;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ())
         {
            DebugOutput::getErrorStream () << "DeviceDither::ditherLibraryValid: g_module_symbol (" << achFileName << ") returns 0x" << std::hex << (int)ptr << std::endl;
            if (!fValid)
            {
               DebugOutput::getErrorStream () << "DeviceDither::ditherLibraryValid: Error this function must exist!" << std::endl;
            }
         }
#endif
      }

      g_module_close (hDevice);
   }
   else
   {
      DebugOutput::getErrorStream () << "DeviceDither::ditherLibraryValid: g_module_error returns " << g_module_error () << std::endl;

      fValid = false;
   }

   return fValid;
}

char *
queryLibrary (char *pszFillIn, PSZCRO pszID)
{
   char            achLine[512]; // @TBD
   std::ifstream   ifIn ("/etc/omni");
   char           *pszLine;
   char           *pszLHS;
   char           *pszRHS;
   char           *pszSplit;

   *pszFillIn = '\0';

   while (ifIn.getline (achLine, sizeof (achLine)))
   {
      pszLine = achLine;

      // Skip whitespace
      while (isspace (*pszLine))
         pszLine++;

      if ('#' == *pszLine)
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::queryLibrary: Found comment " << pszLine << std::endl;
#endif
         continue;
      }
      else if (0 != strncmp (pszLine, "dither ", 7))
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::queryLibrary: Found non-dither " << pszLine << std::endl;
#endif
         continue;
      }

      pszLine = pszLine + 7;
      pszLHS  = pszLine;

      while (*pszLine && !isspace (*pszLine))
         pszLine++;

      while (!isspace (*pszLine))
         continue;

      pszSplit = pszLine;

      // Skip whitespace
      while (isspace (*pszLine))
         pszLine++;

      pszRHS = pszLine;
      *pszSplit = '\0';

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::queryLibrary: pszLHS = " << pszLHS << std::endl;
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::queryLibrary: pszRHS = " << pszRHS << std::endl;
#endif

      // found a match?
#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceDither ()) DebugOutput::getErrorStream () << "DeviceDither::queryLibrary: Testing \"" << pszID << "\" with \"" << pszLHS << "\"" << std::endl;
#endif
      if (0 != strcmp (pszID, pszLHS))
         continue;

      if (ditherLibraryValid (pszRHS))
      {
         sprintf (pszFillIn, "lib%s.so", pszRHS);

         return pszFillIn;
      }
   }

   return 0;
}

PSZ
convert (char  *pszFillIn,
         PSZCRO pszPFN,
         PSZ    pszLibraryName)
{
   strcpy (pszFillIn, pszPFN);

   // Loop through the routines that we know about
   for (int i = 0; i < (int)dimof (apszRoutines); i++)
   {
      // Match?
      if (0 == strncmp (pszPFN, apszRoutines[i], strlen (pszPFN)))
      {
         char achLibraryName[512]; // @TBD

         strcpy (achLibraryName, pszLibraryName);

         pszLibraryName = truncate (achLibraryName);

         // fill out the entry
         sprintf (pszFillIn, apszRoutines[i], strlen (pszLibraryName), pszLibraryName);

         break;
      }
   }

   return pszFillIn;
}
