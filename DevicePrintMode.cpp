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
#include "DevicePrintMode.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "printmode"
};

#define JOBPROP_PRINTMODE     vapszJobPropertyKeys[0]

typedef struct _ModeMapping {
   PSZCRO pszName;
   int    iColorTech;
} MODEMAPPING, *PMODEMAPPING;

static MODEMAPPING vaMappings[] = {
   { "PRINT_MODE_1_ANY",      DevicePrintMode::COLOR_TECH_K       },
   { "PRINT_MODE_24_CMY",     DevicePrintMode::COLOR_TECH_CMY     },
   { "PRINT_MODE_24_CMYK",    DevicePrintMode::COLOR_TECH_CMYK    },
   { "PRINT_MODE_24_CcMmYK",  DevicePrintMode::COLOR_TECH_CcMmYK  },
   { "PRINT_MODE_24_CcMmYyK", DevicePrintMode::COLOR_TECH_CcMmYyK },
   { "PRINT_MODE_24_K",       DevicePrintMode::COLOR_TECH_K       },
   { "PRINT_MODE_24_RGB",     DevicePrintMode::COLOR_TECH_RGB     },
   { "PRINT_MODE_8_CMY",      DevicePrintMode::COLOR_TECH_CMY     },
   { "PRINT_MODE_8_CMYK",     DevicePrintMode::COLOR_TECH_CMYK    },
   { "PRINT_MODE_8_CcMmYK",   DevicePrintMode::COLOR_TECH_CcMmYK  },
   { "PRINT_MODE_8_CcMmYyK",  DevicePrintMode::COLOR_TECH_CcMmYyK },
   { "PRINT_MODE_8_K",        DevicePrintMode::COLOR_TECH_K       },
   { "PRINT_MODE_8_RGB",      DevicePrintMode::COLOR_TECH_RGB     }
};

DevicePrintMode::
DevicePrintMode (Device *pDevice,
                 PSZRO   pszJobProperties,
                 int     iPhysicalCount,
                 int     iLogicalCount,
                 int     iPlanes)
{
   pDevice_d        = pDevice;
   pszPrintMode_d   = 0;
   indexPrintMode_d = -1;
   iPhysicalCount_d = iPhysicalCount;
   iLogicalCount_d  = iLogicalCount;
   iPlanes_d        = iPlanes;
   iColorTech_d     = -1;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties,
                     &pszPrintMode_d,
                     &indexPrintMode_d,
                     &iColorTech_d);
   }
}

DevicePrintMode::
~DevicePrintMode ()
{
   if (pszPrintMode_d)
   {
      free ((void *)pszPrintMode_d);
   }

   pDevice_d        = 0;
   pszPrintMode_d   = 0;
   indexPrintMode_d = -1;
   iPhysicalCount_d = 0;
   iLogicalCount_d  = 0;
   iPlanes_d        = 0;
}

bool DevicePrintMode::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0);
}

bool DevicePrintMode::
isEqual (PSZCRO pszJobProperties)
{
   int indexPrintMode = -1;
   int iColorTech     = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexPrintMode,
                      &iColorTech))
   {
      return    indexPrintMode == indexPrintMode_d
             && iColorTech     == iColorTech_d;
   }

   return false;
}

std::string * DevicePrintMode::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DPM1_"
       << indexPrintMode_d;

   return new std::string (oss.str ());
}

DevicePrintMode * DevicePrintMode::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DevicePrintMode *pPrintModeRet  = 0;
   int              indexPrintMode = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DPM1_", 5))
   {
      if (  1              == sscanf (pszCreateHash, "DPM1_%d", &indexPrintMode)
         && 0              <= indexPrintMode
         && indexPrintMode <  (int)dimof (vaMappings)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_PRINTMODE
             << "="
             << vaMappings[indexPrintMode].pszName;

         pPrintModeRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pPrintModeRet;
}

bool DevicePrintMode::
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

std::string * DevicePrintMode::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_PRINTMODE))
   {
      if (pszPrintMode_d)
      {
         std::ostringstream oss;

         oss << "string " << pszPrintMode_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DevicePrintMode::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_PRINTMODE))
   {
      if (pszPrintMode_d)
      {
         return new std::string (pszPrintMode_d);
      }
   }

   return 0;
}

std::string * DevicePrintMode::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_PRINTMODE, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_PRINTMODE);
      if (pszXLateKey)
      {
         pRet = new std::string (pszXLateKey);
      }

      if (  pszValue
         && *pszValue
         && pRet
         )
      {
         PSZRO pszXLateValue = 0;

         pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                                    StringResource::STRINGGROUP_PRINTMODES,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }

   return pRet;
}

std::string * DevicePrintMode::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_PRINTMODES,
                                              pszPrintMode_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DevicePrintMode::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszPrintModeID = 0;

   if (fInDeviceSpecific)
   {
      pszPrintModeID = getDeviceID ();
   }

   if (!pszPrintModeID)
   {
      pszPrintModeID = pszPrintMode_d;
   }

   if (pszPrintModeID)
   {
      std::ostringstream oss;

      oss << JOBPROP_PRINTMODE << "=" << pszPrintModeID;

      return new std::string (oss.str ());
   }

   return 0;
}

int DevicePrintMode::
getColorTech ()
{
   return iColorTech_d;
}

int DevicePrintMode::
getPhysicalCount ()
{
   return iPhysicalCount_d;
}

int DevicePrintMode::
getLogicalCount ()
{
   return iLogicalCount_d;
}

int DevicePrintMode::
getNumPlanes ()
{
   return iPlanes_d;
}

PSZCRO DevicePrintMode::
getDeviceID ()
{
   return 0;
}

class PrintModeEnumerator : public Enumeration
{
public:

   PrintModeEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   PrintModeEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iIndex_d < (int)dimof (vaMappings);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return false;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_PRINTMODE << "=" << vaMappings[iIndex_d++].pszName;

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DevicePrintMode::
getAllEnumeration ()
{
   return new PrintModeEnumerator ();
}

#ifndef RETAIL

void DevicePrintMode::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DevicePrintMode::
toString (std::ostringstream& oss)
{
   oss << "{DevicePrintMode: "
       << "pszPrintMode_d = " << SAFE_PRINT_PSZ (pszPrintMode_d)
       << ", iPhysicalCount_d = " << iPhysicalCount_d
       << ", iLogicalCount_d = " << iLogicalCount_d
       << ", iPlanes_d = " << iPlanes_d
       << ", iColorTech_d = " << iColorTech_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DevicePrintMode& const_self)
{
   DevicePrintMode&   self = const_cast<DevicePrintMode&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DevicePrintMode::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszPrintMode,
               int    *pindexPrintMode,
               int    *piColorTech)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_PRINTMODE))
      {
         int iLow  = 0;
         int iMid  = (int)dimof (vaMappings) / 2;
         int iHigh = (int)dimof (vaMappings) - 1;
         int iResult;

         while (iLow <= iHigh)
         {
            iResult = strcmp (pszValue, vaMappings[iMid].pszName);

            if (0 == iResult)
            {
               fRet = true;

               if (pindexPrintMode)
               {
                  *pindexPrintMode = iMid;
               }
               if (ppszPrintMode)
               {
                  *ppszPrintMode = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszPrintMode)
                  {
                     strcpy ((char *)*ppszPrintMode, pszValue);
                  }
               }

               if (piColorTech)
               {
                  *piColorTech = vaMappings[iMid].iColorTech;
               }
               break;
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
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}
