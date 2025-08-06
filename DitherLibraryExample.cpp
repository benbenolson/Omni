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

/*
 * make DitherLibraryExample.o
 * cc -shared -Wl,-soname,libDitherLibraryExample.so -o libDitherLibraryExample.so DitherLibraryExample.o
 *
 * in /etc/omni
 * should be a line:
 * dither DITHER_EXAMPLE DitherLibraryExample
 * dither DITHER_CATAGORY_EXAMPLE DitherLibraryExample
 */
#include "Device.hpp"
#include "JobProperties.hpp"

class DitherLibraryExample : public DeviceDither
{
public:
   enum {
      DITHER_EXAMPLE
   };
   enum {
      DITHER_CATAGORY_EXAMPLE
   };

                               DitherLibraryExample           (Device           *pDevice,
                                                               int               iDitherType);
   virtual                    ~DitherLibraryExample           ();

   static DeviceDither        *createDitherInstance           (PSZCRO            pszDitherType,
                                                               Device           *pDevice,
                                                               PSZCRO            pszOptions);

   static std::string         *getCreateHash                  (PSZRO             pszJobProperties);
   static PSZCRO               getIDFromCreateHash            (std::string      *pstringHash);

   void                        ditherRGBtoCMYK                (PBITMAPINFO2      pbmi2,
                                                               PBYTE             pbStart);

   bool                        ditherAllPlanesBlank           ();
   bool                        ditherCPlaneBlank              ();
   bool                        ditherMPlaneBlank              ();
   bool                        ditherYPlaneBlank              ();
   bool                        ditherKPlaneBlank              ();
   bool                        ditherLCPlaneBlank             ();
   bool                        ditherLMPlaneBlank             ();

   void                        newFrame                       ();

   BinaryData                 *getCPlane                      ();
   BinaryData                 *getMPlane                      ();
   BinaryData                 *getYPlane                      ();
   BinaryData                 *getKPlane                      ();
   BinaryData                 *getLCPlane                     ();
   BinaryData                 *getLMPlane                     ();

   PSZCRO                      getID                          ();

   static bool                 ditherNameValid                (PSZCRO            pszId);
   static bool                 ditherCatagoryValid            (PSZCRO            pszId);
   static PSZCRO               getDitherCatagory              (PSZCRO            pszId);
   static PSZCRO               getDitherName                  (PSZCRO            pszId);

   static Enumeration         *getAllEnumeration              ();

private:
   static int                  nameToID                       (PSZCRO            pszId);

   int iDitherType_d;
};

#define STRING_OF(x) (apszDitherNames[x - DITHER_EXAMPLE])

static PSZCRO            apszDitherNames[] = {
   "DITHER_EXAMPLE"
};

int DitherLibraryExample::
nameToID (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DITHER_EXAMPLE
   };

   int iLow  = 0;
   int iMid  = (int)dimof (asiEntries) / 2;
   int iHigh = (int)dimof (asiEntries) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, STRING_OF (asiEntries[iMid]));

      if (0 == iResult)
      {
         return asiEntries[iMid];
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return DITHER_UNLISTED;
}

DeviceDither * DitherLibraryExample::
createDitherInstance (PSZCRO  pszDitherType,
                      Device *pDevice,
                      PSZCRO  pszOptions)
{
   DebugOutput::getErrorStream () << "DitherLibraryExample called with " << pszDitherType << ", " << pszOptions << std::endl;

   if (!ditherNameValid (pszDitherType))
   {
      // Error!
      return 0;
   }

   // Parse the options that were passed into pszOptions

   return new DitherLibraryExample (pDevice, nameToID (pszDitherType));
}

std::string * DitherLibraryExample::
getCreateHash (PSZRO pszJobProperties)
{
   std::string *pstringValue = 0;
   std::string *pstringRet   = 0;

   pstringValue = DeviceDither::getDitherValue (pszJobProperties);

   if (pstringValue)
   {
      int id = nameToID (pstringValue->c_str ());

      if (DITHER_UNLISTED != id)
      {
         std::ostringstream oss;

         oss << "DLE1_"
             << id;

         pstringRet = new std::string (oss.str ());
      }

      delete pstringValue;
   }

   return pstringRet;
}

PSZCRO DitherLibraryExample::
getIDFromCreateHash (std::string *pstringHash)
{
   PSZRO pszCreateHash = 0;
   int   indexDither   = -1;

   if (!pstringHash)
   {
      return 0;
   }

   pszCreateHash = pstringHash->c_str ();

   if (0 == strncmp (pszCreateHash, "DLE1_", 5))
   {
      if (  1           == sscanf (pszCreateHash, "DLE1_%d", &indexDither)
         && 0           <= indexDither
         && indexDither <  (int)dimof (apszDitherNames)
         )
      {
         return apszDitherNames[indexDither];
      }
   }

   return 0;
}

DitherLibraryExample::
DitherLibraryExample (Device *pDevice,
                      int     iDitherType)
   : DeviceDither (pDevice)
{
   iDitherType_d = iDitherType;
}

DitherLibraryExample::
~DitherLibraryExample ()
{
}

PSZCRO DitherLibraryExample::
getID ()
{
   return apszDitherNames[iDitherType_d];
}

void DitherLibraryExample::
ditherRGBtoCMYK (PBITMAPINFO2 pbmi2,
                 PBYTE        pbStart)
{
}

bool DitherLibraryExample::
ditherAllPlanesBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherCPlaneBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherMPlaneBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherYPlaneBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherKPlaneBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherLCPlaneBlank ()
{
   return 0;
}

bool DitherLibraryExample::
ditherLMPlaneBlank ()
{
   return 0;
}

void DitherLibraryExample::
newFrame ()
{
}

BinaryData * DitherLibraryExample::
getCPlane ()
{
   return 0;
}

BinaryData * DitherLibraryExample::
getMPlane ()
{
   return 0;
}

BinaryData * DitherLibraryExample::
getYPlane ()
{
   return 0;
}

BinaryData * DitherLibraryExample::
getKPlane ()
{
   return 0;
}

BinaryData * DitherLibraryExample::
getLCPlane ()
{
   return 0;
}

BinaryData * DitherLibraryExample::
getLMPlane ()
{
   return 0;
}

bool DitherLibraryExample::
ditherNameValid (PSZCRO pszId)
{
   static short int asiEntries[] = {
      DITHER_EXAMPLE
   };

   int iLow  = 0;
   int iMid  = (int)dimof (asiEntries) / 2;
   int iHigh = (int)dimof (asiEntries) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, STRING_OF (asiEntries[iMid]));

      if (0 == iResult)
      {
         return true;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

PSZCRO DitherLibraryExample::
getDitherCatagory (PSZCRO pszId)
{
   if (0 == strcmp ("DITHER_EXAMPLE", pszId))
      return "DITHER_CATAGORY_EXAMPLE";

   return 0;
}

PSZCRO DitherLibraryExample::
getDitherName (PSZCRO pszId)
{
   // @TBD - determine language
   if (0 == strcmp ("DITHER_EXAMPLE", pszId))
      return "An example dithering routine";

   return 0;
}

bool DitherLibraryExample::
ditherCatagoryValid (PSZCRO pszId)
{
   if (0 == strcmp ("DITHER_CATAGORY_EXAMPLE", pszId))
      return true;

   return false;
}

class DitherLibraryExampleEnumerator : public Enumeration
{
public:
   DitherLibraryExampleEnumerator (int cDithers, PSZCRO aDithers[])
   {
      iDither_d  = 0;
      cDithers_d = cDithers;
      aDithers_d = aDithers;
   }

   virtual bool hasMoreElements ()
   {
      if (iDither_d < cDithers_d)
         return true;
      else
         return false;
   }

   virtual void *nextElement ()
   {
      if (iDither_d > cDithers_d - 1)
         return 0;

      std::ostringstream oss;

      oss << JOBPROP_DITHER
          << "="
          << aDithers_d[iDither_d++];

      return (void *)new JobProperties (oss.str ().c_str ());
   }

private:
   int     iDither_d;
   int     cDithers_d;
   PSZCRO *aDithers_d;
};

Enumeration * DitherLibraryExample::
getAllEnumeration ()
{
   return new DitherLibraryExampleEnumerator (dimof (apszDitherNames), apszDitherNames);
}

extern "C"
{

DeviceDither        *createDitherInstance           (PSZCRO  pszDitherType,
                                                     Device *pDevice,
                                                     PSZCRO  pszOptions);
std::string         *getCreateHash                  (PSZRO   pszJobProperties);
bool                 ditherNameValid                (PSZCRO  pszId);
bool                 ditherCatagoryValid            (PSZCRO  pszId);
PSZCRO               getDitherCatagory              (PSZCRO  pszId);
PSZCRO               getDitherName                  (PSZCRO  pszId);
Enumeration         *getAllEnumeration              ();

DeviceDither *
createDitherInstance (PSZCRO  pszDitherType,
                      Device *pDevice,
                      PSZCRO  pszOptions)
{
   return DitherLibraryExample::createDitherInstance (pszDitherType, pDevice, pszOptions);
}

std::string *
getCreateHash (PSZRO pszJobProperties)
{
   return DitherLibraryExample::getCreateHash (pszJobProperties);
}

bool
ditherNameValid (PSZCRO pszId)
{
   return DitherLibraryExample::ditherNameValid (pszId);
}

bool
ditherCatagoryValid (PSZCRO pszId)
{
   return DitherLibraryExample::ditherCatagoryValid (pszId);
}

PSZCRO
getDitherCatagory (PSZCRO pszId)
{
   return DitherLibraryExample::getDitherCatagory (pszId);
}

PSZCRO
getDitherName (PSZCRO pszId)
{
   // @TBD - determine language
   return DitherLibraryExample::getDitherName (pszId);
}

Enumeration *
getAllEnumeration ()
{
   return DitherLibraryExample::getAllEnumeration ();
}

}
