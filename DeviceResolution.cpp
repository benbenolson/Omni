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
#include "DeviceResolution.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "Resolution"
};

#define JOBPROP_RESOLUTION     vapszJobPropertyKeys[0]

static int vaResolutions[][2] = {
   {   60,   60 },
   {   60,   72 },
   {   60,  180 },
   {   60,  360 },
   {   72,   72 },
   {   75,   75 },
   {   80,   60 },
   {   80,   72 },
   {   90,  180 },
   {   90,  360 },
   {   90,   60 },
   {   90,   72 },
   {   90,   90 },
   {  100,  100 },
   {  120,   60 },
   {  120,   72 },
   {  120,  120 },
   {  120,  144 },
   {  120,  180 },
   {  120,  360 },
   {  144,   72 },
   {  150,  150 },
   {  180,  180 },
   {  180,  360 },
   {  200,  200 },
   {  240,   60 },
   {  240,   72 },
   {  240,  144 },
   {  240,  240 },
   {  300,  300 },
   {  360,  180 },
   {  360,  360 },
   {  360,  720 },
   {  400,  400 },
   {  600,  300 },
   {  600,  600 },
   {  720,  360 },
   {  720,  720 },
   { 1200,  600 },
   { 1200, 1200 },
   { 1440,  720 },
   { 2880,  720 }
};

/* Function prototypes...
*/

DeviceResolution::
DeviceResolution (Device     *pDevice,
                  PSZRO       pszJobProperties,
                  int         iXInternalRes,
                  int         iYInternalRes,
                  BinaryData *pbdData,
                  int         iCapabilities,
                  int         iDestinationBitsPerPel,
                  int         iScanlineMultiple)
{
   pDevice_d                = pDevice;
   pszResolution_d          = 0;
   iXRes_d                  = 0;
   iYRes_d                  = 0;
   iXInternalRes_d          = iXInternalRes;
   iYInternalRes_d          = iYInternalRes;
   pbdData_d                = pbdData;
   iCapabilities_d          = iCapabilities;
   iDestinationBitsPerPel_d = iDestinationBitsPerPel;
   iScanlineMultiple_d      = iScanlineMultiple;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties,
                     &pszResolution_d,
                     &iXRes_d,
                     &iYRes_d);
   }
}

DeviceResolution::
~DeviceResolution ()
{
   if (pszResolution_d)
   {
      free ((void *)pszResolution_d);
   }

   delete pbdData_d;

   pDevice_d                = 0;
   pszResolution_d          = 0;
   iXRes_d                  = 0;
   iYRes_d                  = 0;
   iXInternalRes_d          = 0;
   iYInternalRes_d          = 0;
   pbdData_d                = 0;
   iCapabilities_d          = 0;
   iDestinationBitsPerPel_d = 0;
   iScanlineMultiple_d      = 0;
}

bool DeviceResolution::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0);
}

bool DeviceResolution::
isEqual (PSZCRO pszJobProperties)
{
   int iXRes = -1;
   int iYRes = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iXRes,
                      &iYRes))
   {
      return    iXRes == iXRes_d
             && iYRes == iYRes_d;
   }

   return false;
}

std::string * DeviceResolution::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DRE1_"
       << iXRes_d
       << "_"
       << iYRes_d;

   return new std::string (oss.str ());
}

DeviceResolution * DeviceResolution::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceResolution *pResolutionRet = 0;
   int               iXRes          = -1;
   int               iYRes          = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DRE1_", 5))
   {
      PSZRO pszScan = pszCreateHash + 5;

      if (0 == sscanf (pszScan, "%d", &iXRes))
      {
         return 0;
      }

      pszScan = strchr (pszScan, '_');
      if (!pszScan)
      {
         return 0;
      }
      pszScan++;

      if (0 == sscanf (pszScan, "%d", &iYRes))
      {
         return 0;
      }

      std::ostringstream oss;

      oss << JOBPROP_RESOLUTION
          << "="
          << iXRes
          << "x"
          << iYRes;

      pResolutionRet = create (pDevice, oss.str ().c_str ());
   }

   return pResolutionRet;
}

bool DeviceResolution::
handlesKey (PSZCRO pszKey)
{
   if (  !pszKey
      || !*pszKey
      )
   {
      return false;
   }

   for (int i = 0; i < (int)dimof (vapszJobPropertyKeys); i++)
   {
      if (0 == strcmp (pszKey, vapszJobPropertyKeys[i]))
      {
         return true;
      }
   }

   return false;
}

std::string * DeviceResolution::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_RESOLUTION))
   {
      if (  iXRes_d
         && iYRes_d
         )
      {
         std::ostringstream oss;

         oss << "string " << iXRes_d << "x" << iYRes_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceResolution::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_RESOLUTION))
   {
      if (  iXRes_d
         && iYRes_d
         )
      {
         std::ostringstream oss;

         oss << iXRes_d << "x" << iYRes_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceResolution::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_RESOLUTION, pszKey))
   {
      std::ostringstream oss;
      PSZRO              pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_RESOLUTION);
      if (pszXLateKey)
      {
         oss << pszXLateKey;

         if (  pszValue
            && *pszValue
            )
         {
            oss << "="
                << pszValue;
         }

         return new std::string (oss.str ());
      }
   }

   return pRet;
}

std::string * DeviceResolution::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_RESOLUTIONS,
                                              pszResolution_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceResolution::
getJobProperties (bool fInDeviceSpecific)
{
   std::ostringstream oss;

   if (  fInDeviceSpecific
      && getDeviceID ()
      )
   {
      PSZCRO pszID = getDeviceID ();

      oss << JOBPROP_RESOLUTION << "=" << pszID;
   }
   else
   {
      if (  iXRes_d
         && iYRes_d
         )
      {
         oss << JOBPROP_RESOLUTION << "=" << iXRes_d << "x" << iYRes_d;
      }
   }

   if (oss.str ()[0])
   {
      return new std::string (oss.str ());
   }
   else
   {
      return 0;
   }
}

int DeviceResolution::
getXRes ()
{
   if (iXInternalRes_d > 0)
   {
      return iXInternalRes_d;
   }
   else
   {
      return iXRes_d;
   }
}

int DeviceResolution::
getYRes ()
{
   if (iYInternalRes_d > 0)
   {
      return iYInternalRes_d;
   }
   else
   {
      return iYRes_d;
   }
}

int DeviceResolution::
getExternalXRes ()
{
   return iXRes_d;
}

int DeviceResolution::
getExternalYRes ()
{
   return iYRes_d;
}

int DeviceResolution::
getInternalXRes ()
{
   return iXInternalRes_d;
}

int DeviceResolution::
getInternalYRes ()
{
   return iYInternalRes_d;
}

BinaryData * DeviceResolution::
getData ()
{
   return pbdData_d;
}

bool DeviceResolution::
hasCapability (int iFlag)
{
   return 0 != (iFlag & iCapabilities_d);
}

int DeviceResolution::
getCapabilities ()
{
   return iCapabilities_d;
}

int DeviceResolution::
getDstBitsPerPel ()
{
   return iDestinationBitsPerPel_d;
}

int DeviceResolution::
getScanlineMultiple ()
{
   return iScanlineMultiple_d;
}

PSZCRO DeviceResolution::
getDeviceID ()
{
   return 0;
}

void DeviceResolution::
setInternalXRes (int iXInternalRes)
{
   iXInternalRes_d = iXInternalRes;
}

void DeviceResolution::
setInternalYRes (int iYInternalRes)
{
   iYInternalRes_d = iYInternalRes;
}

typedef struct _ReservedMap {
   PSZCRO pszName;
   int    iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "NO_CAPABILITIES",             0x00000000 }
};

bool DeviceResolution::
isReservedKeyword (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return true;
   }

   return false;
}

int DeviceResolution::
getReservedValue (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return vaReservedKeywords[i].iValue;
   }

   return 0;
}

class ResolutionEnumerator : public Enumeration
{
public:

   ResolutionEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   ResolutionEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iIndex_d < (int)dimof (vaResolutions);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return NULL;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_RESOLUTION
          << "="
          << vaResolutions[iIndex_d][0]
          << "x"
          << vaResolutions[iIndex_d][1];

      iIndex_d++;

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceResolution::
getAllEnumeration ()
{
   return new ResolutionEnumerator ();
}

#ifndef RETAIL

void DeviceResolution::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceResolution::
toString (std::ostringstream& oss)
{
   oss << "{DeviceResolution: "
       << "iXRes_d = " << iXRes_d
       << ", iYRes_d = " << iYRes_d
       << std::hex << ", iCapabilities_d = 0x" << iCapabilities_d
       << std::dec << ", iDestinationBitsPerPel_d = " << iDestinationBitsPerPel_d
       << ", iScanlineMultiple_d = " << iScanlineMultiple_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceResolution& const_self)
{
   DeviceResolution&  self = const_cast<DeviceResolution&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

char *
validateResolutionString (PSZCRO pszResolution)
{
   PSZRO pszTest   = pszResolution;
   bool  fFirst    = true;
   char *pszReturn = 0;

   if (  !pszResolution
      || !*pszResolution
      )
   {
      return 0;
   }

   do
   {
      if (  'x' == *pszTest
         || 'X' == *pszTest
         )
      {
         if (fFirst)
         {
            return 0;
         }
         else
         {
            pszTest++;
            break;
         }
      }
      else if (  '0' > *pszTest
              || '9' < *pszTest
              )
      {
         return 0;
      }

      pszTest++;

      fFirst = false;

   } while (*pszTest);

   fFirst = true;
   do
   {
      if (!*pszTest)
      {
         if (fFirst)
         {
            return 0;
         }
         else
         {
            break;
         }
      }
      else if (  '0' > *pszTest
              || '9' < *pszTest
              )
      {
         return 0;
      }

      pszTest++;

      fFirst = false;

   } while (*pszTest);

   pszReturn = (char *)malloc (strlen (pszResolution) + 1);

   if (pszReturn)
   {
      strcpy (pszReturn, pszResolution);
   }

   return pszReturn;
}

bool
validateResolutionValues (int iX,
                          int iY)
{
   int iLow    = 0;
   int iMid    = (int)dimof (vaResolutions) / 2;
   int iHigh   = (int)dimof (vaResolutions) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = iX - vaResolutions[iMid][0];

      if (0 == iResult)
      {
         // val1 == val2

         if (iY == vaResolutions[iMid][1])
         {
            return true;
         }

         int iIndex = iMid - 1;

         while (iX == vaResolutions[iIndex][0])
         {
            if (iY == vaResolutions[iIndex][1])
            {
               return true;
            }

            iIndex--;
         }

         iIndex = iMid + 1;

         while (iX == vaResolutions[iIndex][0])
         {
            if (iY == vaResolutions[iIndex][1])
            {
               return true;
            }

            iIndex++;
         }

         return false;
      }
      else if (0 > iResult)
      {
         // val1 < val2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // val1 > val2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

bool DeviceResolution::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszResolution,
               int    *piX,
               int    *piY)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_RESOLUTION))
      {
         char *pszResolutionName = 0;

         pszResolutionName = validateResolutionString (pszValue);

         if (pszResolutionName)
         {
            char *pszTimes = 0;
            int   iX       = 0;
            int   iY       = 0;

            pszTimes = strchr (pszResolutionName, 'x');
            if (pszTimes)
            {
               *pszTimes = '\0';
            }

            if (pszTimes)
            {
               iX = atoi (pszResolutionName);
               iY = atoi (pszTimes + 1);
            }
            else
            {
               iX = atoi (pszResolutionName);
               iY = iX;
            }

            if (  iX
               && iY
               && validateResolutionValues (iX, iY)
               )
            {
               if (piX)
               {
                  *piX = iX;
               }
               if (piY)
               {
                  *piY = iY;
               }
               if (ppszResolution)
               {
                  *ppszResolution = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszResolution)
                  {
                     strcpy ((char *)*ppszResolution, pszValue);
                  }
               }

               fRet = true;
            }

            free ((void *)pszResolutionName);
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}
