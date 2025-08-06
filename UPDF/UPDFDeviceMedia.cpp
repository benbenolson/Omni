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

#include <UPDFDeviceMedia.hpp>

UPDFDeviceMedia::
UPDFDeviceMedia (Device      *pDevice,
                 PSZRO        pszJobProperties,
                 BinaryData  *pbdData,
                 int          iColorAdjustRequired,
                 int          iAbsorption,
                 XmlNodePtr   node)
   : DeviceMedia (pDevice,
                  pszJobProperties,
                  pbdData,
                  iColorAdjustRequired,
                  iAbsorption)
{
   node_d = node;
}

UPDFDeviceMedia::
~UPDFDeviceMedia ()
{
}

static XmlNodePtr
skipInvalidMedias (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceMedia *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  node)
{
   UPDFDevice         *pUPDFDevice          = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszUPDFMediaName     = 0;
   PSZRO               pszOmniMediaName     = 0;
   int                 iColorAdjustRequired = 0;
   int                 iAbsorption          = DeviceMedia::MEDIA_NO_ABSORPTION;
   BinaryData         *pbdData              = 0;
   std::ostringstream  oss;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   pszUPDFMediaName = XMLGetProp (node, "ClassifyingID");

   if (!UPDFDeviceMedia::mapUPDFToOmni (pszUPDFMediaName, &pszOmniMediaName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !mapUPDFToOmni" << std::endl;
#endif

      return 0;
   }

   if (pszUPDFMediaName)
   {
      XMLFree ((void *)pszUPDFMediaName);
   }

   if (pszOmniMediaName)
   {
      oss << "media=" << pszOmniMediaName;

      return new UPDFDeviceMedia (pUPDFDevice,
                                  oss.str ().c_str (),
                                  pbdData,
                                  iColorAdjustRequired,
                                  iAbsorption,
                                  node);
   }
   else
   {
      return 0;
   }
}

DeviceMedia * UPDFDeviceMedia::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice  *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr   nodeMedias       = 0;
   DeviceMedia *pMediaRet        = 0;
   XmlNodePtr   nodeItem         = 0;
   PSZRO        pszMediaName     = 0;
   PSZRO        pszUPDFMediaName = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszMediaName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceMedia::mapOmniToUPDF (pszMediaName, &pszUPDFMediaName))
   {
      goto done;
   }

   if (!pszUPDFMediaName)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !pszUPDFMediaName" << std::endl;
#endif

      goto done;
   }

   nodeMedias = findMedias (pUPDFDevice);

   if (!nodeMedias)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !nodeMedias" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeMedias));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !nodeItem" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !pMediaRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszUPDFMediaName))
         {
            pMediaRet = createFromXMLNode (pDevice, nodeItem);
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pMediaRet)
   {
      pMediaRet = pUPDFDevice->getDefaultMedia ();
   }

   return pMediaRet;
}

DeviceMedia * UPDFDeviceMedia::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceMedia::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice  *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr   nodeMedias       = 0;
   XmlNodePtr   nodeItem         = 0;
   PSZRO        pszMediaName     = 0;
   PSZRO        pszUPDFMediaName = 0;
   bool         fRet             = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, &pszMediaName, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceMedia::mapOmniToUPDF (pszMediaName, &pszUPDFMediaName))
   {
      goto done;
   }

   if (!pszUPDFMediaName)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !pszUPDFMediaName" << std::endl;
#endif

      goto done;
   }

   nodeMedias = findMedias (pUPDFDevice);

   if (!nodeMedias)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !nodeMedias" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeMedias));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": !nodeItem" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszUPDFMediaName))
         {
            fRet = true;
         }

         XMLFree ((void *)pszNodeId);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   return fRet;
}

PSZCRO UPDFDeviceMedia::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceMedia::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet        = 0;
   UPDFDevice                 *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeMedias  = 0;
   XmlNodePtr                  nodeMedia   = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeMedias = findMedias (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": nodeMedias = " << std::hex << (int *)nodeMedias << std::dec << std::endl;
#endif

   if (!nodeMedias)
   {
      goto done;
   }

   nodeMedia = XMLFirstNode (XMLGetChildrenNode (nodeMedias));
   nodeMedia = skipInvalidMedias (nodeMedia);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": nodeMedia = " << std::hex << (int *)nodeMedia << std::dec << std::endl;
#endif

   while (nodeMedia)
   {
      DeviceMedia   *pMedia = 0;
      JobProperties *pJP    = 0;

      pMedia = createFromXMLNode (pDevice_d, nodeMedia);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": pMedia = " << std::hex << (int *)pMedia << std::dec << std::endl;
#endif

      if (pMedia)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pMedia->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pMedia;
      }

      nodeMedia = XMLNextNode (nodeMedia);
      nodeMedia = skipInvalidMedias (nodeMedia);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceMedia ()) DebugOutput::getErrorStream () << "UPDFDeviceMedia::" << __FUNCTION__ << ": nodeMedia = " << std::hex << (int *)nodeMedia << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceMedia::
findMedias (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeMedias = 0;

   if (!pUPDFDevice)
   {
      return nodeMedias;
   }

   if (  ((nodeMedias = FINDUDRENTRY (pUPDFDevice, nodeMedias, "PrintCapabilities",  UPDFDeviceMedia)) != 0)
      && ((nodeMedias = FINDUDRENTRY (pUPDFDevice, nodeMedias, "Features",           UPDFDeviceMedia)) != 0)
      && ((nodeMedias = FINDUDRENTRY (pUPDFDevice, nodeMedias, "MediaType",          UPDFDeviceMedia)) != 0)
      )
      return nodeMedias;

   return nodeMedias;
}

bool UPDFDeviceMedia::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniJobProperties)
{
   typedef struct _MediaMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } MEDIAMAPPING, *PMEDIAMAPPING;
   MEDIAMAPPING ammFromUPDFToOmni[] = {
      { "cardstock",                    "MEDIA_CARDSTOCK"           },
      { "custom-media-type-bond",       "MEDIA_BOND"                },
      { "custom-media-type-color",      "MEDIA_COLOR"               },
      { "custom-media-type-recycled",   "MEDIA_RECYCLED"            },
      { "custom-media-type-rough",      "MEDIA_ROUGH"               },
      { "device-setting",               "MEDIA_USE_PRINTER_SETTING" },
      { "labels",                       "MEDIA_LABELS"              },
      { "stationery",                   "MEDIA_PLAIN"               },
      { "stationery-letterhead",        "MEDIA_LETTERHEAD"          },
      { "stationery-preprinted",        "MEDIA_PREPRINTED"          },
      { "stationery-prepunched",        "MEDIA_PREPUNCHED"          },
      { "transparency",                 "MEDIA_TRANSPARENCY"        }
   };
   int iLow                 = 0;
   int iMid                 = 0;
   int iHigh                = 0;
   int iResult;

   iMid  = (int)dimof (ammFromUPDFToOmni) / 2;
   iHigh = (int)dimof (ammFromUPDFToOmni) - 1;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszUPDFValue, ammFromUPDFToOmni[iMid].pszFrom);

      if (0 == iResult)
      {
         if (ppszOmniJobProperties)
         {
            *ppszOmniJobProperties = ammFromUPDFToOmni[iMid].pszTo;
         }

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

bool UPDFDeviceMedia::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   typedef struct _MediaMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } MEDIAMAPPING, *PMEDIAMAPPING;
   MEDIAMAPPING ammFromOmniToUPDF[] = {
      { "MEDIA_BOND",                "custom-media-type-bond"     },
      { "MEDIA_CARDSTOCK",           "cardstock"                  },
      { "MEDIA_COLOR",               "custom-media-type-color"    },
      { "MEDIA_LABELS",              "labels"                     },
      { "MEDIA_LETTERHEAD",          "stationery-letterhead"      },
      { "MEDIA_PLAIN",               "stationery"                 },
      { "MEDIA_PREPRINTED",          "stationery-preprinted"      },
      { "MEDIA_PREPUNCHED",          "stationery-prepunched"      },
      { "MEDIA_RECYCLED",            "custom-media-type-recycled" },
      { "MEDIA_ROUGH",               "custom-media-type-rough"    },
      { "MEDIA_TRANSPARENCY",        "transparency"               },
      { "MEDIA_USE_PRINTER_SETTING", "device-setting"             }
   };
   int iLow                 = 0;
   int iMid                 = 0;
   int iHigh                = 0;
   int iResult;

   iMid  = (int)dimof (ammFromOmniToUPDF) / 2;
   iHigh = (int)dimof (ammFromOmniToUPDF) - 1;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszOmniValue, ammFromOmniToUPDF[iMid].pszFrom);

      if (0 == iResult)
      {
         if (ppszUPDFValue)
         {
            *ppszUPDFValue = ammFromOmniToUPDF[iMid].pszTo;
         }

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

#ifndef RETAIL

void UPDFDeviceMedia::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceMedia::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceMedia: "
       << DeviceMedia::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceMedia& const_self)
{
   UPDFDeviceMedia&   self = const_cast<UPDFDeviceMedia&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
