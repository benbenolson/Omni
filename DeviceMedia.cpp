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
#include "DeviceMedia.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>

static PSZCRO vapszJobPropertyKeys[] = {
   "media"
};

#define JOBPROP_MEDIA        vapszJobPropertyKeys[0]

static PSZCRO vapszNames[] = {
   "MEDIA_ARCHIVAL_MATTE_PAPER",
   "MEDIA_AUTO",
   "MEDIA_BACKPRINT",
   "MEDIA_BOND",
   "MEDIA_BRIGHT_WHITE_INKJET_PAPER",
   "MEDIA_CARDBOARD",
   "MEDIA_CARDSTOCK",
   "MEDIA_CD_MASTER",
   "MEDIA_CLOTH",
   "MEDIA_COATED",
   "MEDIA_COLOR",
   "MEDIA_COLORLIFE_PHOTO_PAPER",
   "MEDIA_CUSTOM_1",
   "MEDIA_CUSTOM_2",
   "MEDIA_CUSTOM_3",
   "MEDIA_CUSTOM_4",
   "MEDIA_CUSTOM_5",
   "MEDIA_DOUBLE_SIDED_MATTE_PAPER",
   "MEDIA_ENVELOPE",
   "MEDIA_GLOSSY",
   "MEDIA_HEAVYWEIGH_MATTE_PAPER",
   "MEDIA_HIGH_GLOSS_FILM",
   "MEDIA_HIGH_RESOLUTION",
   "MEDIA_HP_PHOTOGRAPHIC_PAPER",
   "MEDIA_HP_PREMIUM_PAPER",
   "MEDIA_IRON_ON",
   "MEDIA_LABECA",
   "MEDIA_LABELS",
   "MEDIA_LETTERHEAD",
   "MEDIA_OTHER",
   "MEDIA_PHOTOGRAPHIC_INKJET_PAPER",
   "MEDIA_PHOTOGRAPHIC_LABEL",
   "MEDIA_PHOTOGRAPHIC_PAPER",
   "MEDIA_PLAIN",
   "MEDIA_PLAIN_ENHANCED",
   "MEDIA_POSTCARD",
   "MEDIA_PREM_SEMIGLOSS_PHOTO_PAPER",
   "MEDIA_PREPRINTED",
   "MEDIA_PREPUNCHED",
   "MEDIA_RECYCLED",
   "MEDIA_ROUGH",
   "MEDIA_SPECIAL",
   "MEDIA_SPECIAL_360",
   "MEDIA_SPECIAL_720",
   "MEDIA_SPECIAL_BLUE",
   "MEDIA_SPECIAL_GREEN",
   "MEDIA_SPECIAL_GREY",
   "MEDIA_SPECIAL_IVORY",
   "MEDIA_SPECIAL_LETTERHEAD",
   "MEDIA_SPECIAL_ORANGE",
   "MEDIA_SPECIAL_PINK",
   "MEDIA_SPECIAL_PURPLE",
   "MEDIA_SPECIAL_RED",
   "MEDIA_SPECIAL_USER_COLOR",
   "MEDIA_SPECIAL_YELLOW",
   "MEDIA_TABSTOCK",
   "MEDIA_TABSTOCK_2",
   "MEDIA_TABSTOCK_3",
   "MEDIA_TABSTOCK_4",
   "MEDIA_TABSTOCK_5",
   "MEDIA_TABSTOCK_6",
   "MEDIA_TABSTOCK_7",
   "MEDIA_TABSTOCK_8",
   "MEDIA_TABSTOCK_9",
   "MEDIA_THERMAL",
   "MEDIA_THICK",
   "MEDIA_THICK_1",
   "MEDIA_THICK_2",
   "MEDIA_THICK_3",
   "MEDIA_THICK_BLUE",
   "MEDIA_THICK_GREEN",
   "MEDIA_THICK_GREY",
   "MEDIA_THICK_IVORY",
   "MEDIA_THICK_LETTERHEAD",
   "MEDIA_THICK_ORANGE",
   "MEDIA_THICK_PINK",
   "MEDIA_THICK_PURPLE",
   "MEDIA_THICK_RED",
   "MEDIA_THICK_USER_COLOR",
   "MEDIA_THICK_YELLOW",
   "MEDIA_TRANSLUCENT",
   "MEDIA_TRANSPARENCY",
   "MEDIA_USE_PRINTER_SETTING"
};

DeviceMedia::
DeviceMedia (Device     *pDevice,
             PSZRO       pszJobProperties,
             BinaryData *pbdData,
             int         iColorAdjustRequired,
             int         iAbsorption)
{
   pDevice_d              = pDevice;
   pszMedia_d             = 0;
   indexMedia_d           = -1;
   pbdData_d              = pbdData;
   iColorAdjustRequired_d = iColorAdjustRequired;
   iAbsorption_d          = iAbsorption;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      getComponents (pszJobProperties, &pszMedia_d, &indexMedia_d);
   }
}

DeviceMedia::
~DeviceMedia ()
{
   if (pszMedia_d)
   {
      free ((void *)pszMedia_d);
   }

   delete pbdData_d;

   pDevice_d              = 0;
   pszMedia_d             = 0;
   indexMedia_d           = -1;
   pbdData_d              = 0;
   iColorAdjustRequired_d = 0;
   iAbsorption_d          = 0;
}

bool DeviceMedia::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0);
}

bool DeviceMedia::
isEqual (PSZCRO pszJobProperties)
{
   int indexMedia = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexMedia))
   {
      return indexMedia == indexMedia_d;
   }

   return false;
}

std::string * DeviceMedia::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DME1_"
       << indexMedia_d;

   return new std::string (oss.str ());
}

DeviceMedia * DeviceMedia::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceMedia *pMediaRet  = 0;
   int          indexMedia = -1;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DME1_", 5))
   {
      if (  1          == sscanf (pszCreateHash, "DME1_%d", &indexMedia)
         && 0          <= indexMedia
         && indexMedia <  (int)dimof (vapszNames)
         )
      {
         std::ostringstream oss;

         oss << JOBPROP_MEDIA
             << "="
             << vapszNames[indexMedia];

         pMediaRet = create (pDevice, oss.str ().c_str ());
      }
   }

   return pMediaRet;
}

bool DeviceMedia::
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

std::string * DeviceMedia::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_MEDIA))
   {
      if (pszMedia_d)
      {
         std::ostringstream oss;

         oss << "string " << pszMedia_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceMedia::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_MEDIA))
   {
      if (pszMedia_d)
      {
         return new std::string (pszMedia_d);
      }
   }

   return 0;
}

std::string * DeviceMedia::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_MEDIA, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_MEDIA);
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
                                                    StringResource::STRINGGROUP_MEDIAS,
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

std::string * DeviceMedia::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_MEDIAS,
                                              pszMedia_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceMedia::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszMediaID = 0;

   if (fInDeviceSpecific)
   {
      pszMediaID = getDeviceID ();
   }

   if (!pszMediaID)
   {
      pszMediaID = pszMedia_d;
   }

   if (pszMediaID)
   {
      std::ostringstream oss;

      oss << JOBPROP_MEDIA << "=" << pszMediaID;

      return new std::string (oss.str ());
   }

   return 0;
}

BinaryData * DeviceMedia::
getData ()
{
   return pbdData_d;
}

int DeviceMedia::
getColorAdjustRequired ()
{
   return iColorAdjustRequired_d;
}

int DeviceMedia::
getAbsorption ()
{
   return iAbsorption_d;
}

PSZCRO DeviceMedia::
getDeviceID ()
{
   return 0;
}

std::string * DeviceMedia::
getMedia ()
{
   if (pszMedia_d)
   {
      return new std::string (pszMedia_d);
   }

   return 0;
}

typedef struct _ReservedMap {
   PSZCRO pszName;
   int    iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "MEDIA_NO_ABSORPTION",    0 },
   { "MEDIA_LIGHT_ABSORPTION", 1 },
   { "MEDIA_HEAVY_ABSORPTION", 2 }
};

bool DeviceMedia::
isReservedKeyword (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return true;
   }

   return false;
}

int DeviceMedia::
getReservedValue (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return vaReservedKeywords[i].iValue;
   }

   return 0;
}

class MediaEnumerator : public Enumeration
{
public:

   MediaEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   MediaEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iIndex_d < (int)dimof (vapszNames);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return 0;
      }

      std::ostringstream  oss;
      void               *pvRet = 0;

      oss << JOBPROP_MEDIA << "=" << vapszNames[iIndex_d++];

      pvRet = (void *)new JobProperties (oss.str ());

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceMedia::
getAllEnumeration ()
{
   return new MediaEnumerator ();
}

#ifndef RETAIL

void DeviceMedia::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceMedia::
toString (std::ostringstream& oss)
{
   oss << "{DeviceMedia: "
       << "pszMedia_d = " << SAFE_PRINT_PSZ (pszMedia_d)
       << ", iColorAdjustRequired_d = " << iColorAdjustRequired_d
       << ", iAbsorption_d = " << iAbsorption_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceMedia& const_self)
{
   DeviceMedia&       self = const_cast<DeviceMedia&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool DeviceMedia::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszMedia,
               int    *pindexMedia)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_MEDIA))
      {
         int iLow  = 0;
         int iMid  = (int)dimof (vapszNames) / 2;
         int iHigh = (int)dimof (vapszNames) - 1;
         int iResult;

         while (iLow <= iHigh)
         {
            iResult = strcmp (pszValue, vapszNames[iMid]);

            if (0 == iResult)
            {
               fRet = true;

               if (ppszMedia)
               {
                  *ppszMedia = (PSZRO)malloc (strlen (pszValue) + 1);
                  if (*ppszMedia)
                  {
                     strcpy ((char *)*ppszMedia, pszValue);
                  }
               }
               if (pindexMedia)
               {
                  *pindexMedia = iMid;
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

DefaultMedia::
DefaultMedia (Device *pDevice,
              PSZRO   pszJobProperties)
   : DeviceMedia (pDevice,
                  pszJobProperties,
                  0,
                  DEFAULT_COLOR_ADJUST_REQUIRED,
                  DEFAULT_ABSORPTION)
{
}

DeviceMedia * DefaultMedia::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   int iMedia = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iMedia))
   {
      if (DEFAULT_MEDIA_INDEX == iMedia)
      {
         std::ostringstream oss;

         writeDefaultJP (oss);

         return new DefaultMedia (pDevice, oss.str ().c_str ());
      }
   }

   return 0;
}

DeviceMedia * DefaultMedia::
create (Device *pDevice, PSZCRO pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool DefaultMedia::
isSupported (PSZCRO pszJobProperties)
{
   int iMedia = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &iMedia))
   {
      return DEFAULT_MEDIA_INDEX == iMedia;
   }

   return false;
}

Enumeration * DefaultMedia::
getEnumeration (bool fInDeviceSpecific)
{
   class DefaultMediaValueEnumerator : public Enumeration
   {
   public:
      DefaultMediaValueEnumerator ()
      {
         fReturnedValue_d = false;
      }

      ~DefaultMediaValueEnumerator ()
      {
         fReturnedValue_d = true;
      }

      virtual bool hasMoreElements ()
      {
         return !fReturnedValue_d;
      }

      virtual void *nextElement ()
      {
         void *pvRet = 0;

         if (!fReturnedValue_d)
         {
            std::ostringstream oss;

            fReturnedValue_d = true;

            writeDefaultJP (oss);

            stringReturn_d = oss.str ();

            pvRet = (void *)new JobProperties (stringReturn_d);
         }

         return pvRet;
      }

   private:
      bool        fReturnedValue_d;
      std::string stringReturn_d;
   };

   return new DefaultMediaValueEnumerator ();
}

void DefaultMedia::
writeDefaultJP (std::ostringstream& oss)
{
   oss << JOBPROP_MEDIA << "=" << vapszNames[DEFAULT_MEDIA_INDEX];
}
