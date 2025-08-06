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

#include <UPDFDeviceOutputBin.hpp>

UPDFDeviceOutputBin::
UPDFDeviceOutputBin (Device     *pDevice,
                     PSZRO       pszJobProperties,
                     BinaryData *pbdData,
                     XmlNodePtr  node)
   : DeviceOutputBin (pDevice,
                      pszJobProperties,
                      pbdData)
{
   node_d = node;
}

UPDFDeviceOutputBin::
~UPDFDeviceOutputBin ()
{
}

static XmlNodePtr
skipInvalidOutputBins (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceOutputBin *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  node)
{
   UPDFDevice         *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszOutputBinName = 0;
   BinaryData         *pbdData          = 0; // @TBD
   std::ostringstream  oss;
   DeviceOutputBin    *pOutputBinRet    = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   pszOutputBinName = XMLGetProp (node, "ClassifyingID");

   if (pszOutputBinName)
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDeviceOutputBin::mapUPDFToOmni (pszOutputBinName,
                                              &pszOmniJP))
      {
         oss << "OutputBin=" << pszOmniJP;

         pOutputBinRet = new UPDFDeviceOutputBin (pUPDFDevice,
                                                  oss.str ().c_str (),
                                                  pbdData,
                                                  node);
      }
   }

   if (pszOutputBinName)
   {
      XMLFree ((void *)pszOutputBinName);
   }

   return pOutputBinRet;
}

DeviceOutputBin * UPDFDeviceOutputBin::
createS (Device  *pDevice,
         PSZCRO   pszJobProperties)
{
   UPDFDevice      *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr       nodeOutputBins   = 0;
   XmlNodePtr       nodeItem         = 0;
   XmlNodePtr       nodeFound        = 0;
   PSZRO            pszOutputBinName = 0;
   PSZRO            pszOmniName      = 0;
   DeviceOutputBin *pOutputBinRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceOutputBin::mapOmniToUPDF (pszOmniName, &pszOutputBinName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeOutputBins = findOutputBins (pUPDFDevice);

   if (!nodeOutputBins)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !nodeOutputBins" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeOutputBins));

   while (  nodeItem != 0
         && !pOutputBinRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszOutputBinName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pOutputBinRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pOutputBinRet)
   {
      pOutputBinRet = pUPDFDevice->getDefaultOutputBin ();
   }

   return pOutputBinRet;
}

DeviceOutputBin * UPDFDeviceOutputBin::
create (Device  *pDevice,
        PSZCRO   pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceOutputBin::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice      *pUPDFDevice      = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr       nodeOutputBins   = 0;
   XmlNodePtr       nodeItem         = 0;
   XmlNodePtr       nodeFound        = 0;
   PSZRO            pszOutputBinName = 0;
   PSZRO            pszOmniName      = 0;
   bool             fRet             = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceOutputBin::mapOmniToUPDF (pszOmniName, &pszOutputBinName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeOutputBins = findOutputBins (pUPDFDevice);

   if (!nodeOutputBins)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": !nodeOutputBins" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeOutputBins));

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszOutputBinName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         fRet = true;
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   return fRet;
}

PSZCRO UPDFDeviceOutputBin::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceOutputBin::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet           = 0;
   UPDFDevice                 *pUPDFDevice    = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeOutputBins = 0;
   XmlNodePtr                  nodeOutputBin  = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeOutputBins = findOutputBins (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": nodeOutputBins = " << std::hex << (int *)nodeOutputBins << std::dec << std::endl;
#endif

   if (!nodeOutputBins)
   {
      goto done;
   }

   nodeOutputBin = XMLFirstNode (XMLGetChildrenNode (nodeOutputBins));
   nodeOutputBin = skipInvalidOutputBins (nodeOutputBin);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": nodeOutputBin = " << std::hex << (int *)nodeOutputBin << std::dec << std::endl;
#endif

   while (nodeOutputBin)
   {
      DeviceOutputBin *pOutputBin = 0;
      JobProperties   *pJP        = 0;

      pOutputBin = createFromXMLNode (pDevice_d, nodeOutputBin);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": pOutputBin = " << std::hex << (int *)pOutputBin << std::dec << std::endl;
#endif

      if (pOutputBin)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pOutputBin->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pOutputBin;
      }

      nodeOutputBin = XMLNextNode (nodeOutputBin);
      nodeOutputBin = skipInvalidOutputBins (nodeOutputBin);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceOutputBin ()) DebugOutput::getErrorStream () << "UPDFDeviceOutputBin::" << __FUNCTION__ << ": nodeOutputBin = " << std::hex << (int *)nodeOutputBin << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceOutputBin::
findOutputBins (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeOutputBins = 0;

   if (!pUPDFDevice)
   {
      return nodeOutputBins;
   }

   if (  ((nodeOutputBins = FINDUDRENTRY (pUPDFDevice, nodeOutputBins, "PrintCapabilities", UPDFDeviceOutputBin)) != 0)
      && ((nodeOutputBins = FINDUDRENTRY (pUPDFDevice, nodeOutputBins, "Features",          UPDFDeviceOutputBin)) != 0)
      && ((nodeOutputBins = FINDUDRENTRY (pUPDFDevice, nodeOutputBins, "OutputBin",         UPDFDeviceOutputBin)) != 0)
      )
      return nodeOutputBins;

   return nodeOutputBins;
}

bool UPDFDeviceOutputBin::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniValue)
{
   typedef struct _OutputBinMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromUPDFToOmni[] = {
      { "Booklet",         "Booklet"           },
      { "Bottom",          "Bottom"            },
      { "Center",          "Center"            },
      { "Face-down",       "FaceDown"          },
      { "Face-up",         "FaceUp"            },
      { "Fit-media",       "FitMedia"          },
      { "Large-capacity",  "LargeCapacity"     },
      { "Left",            "Left"              },
      { "Mailbox-1",       "Mailbox-1"         },
      { "Mailbox-2",       "Mailbox-2"         },
      { "Mailbox-3",       "Mailbox-3"         },
      { "Mailbox-4",       "Mailbox-4"         },
      { "Mailbox-5",       "Mailbox-5"         },
      { "Mailbox-6",       "Mailbox-6"         },
      { "Mailbox-7",       "Mailbox-7"         },
      { "Mailbox-8",       "Mailbox-8"         },
      { "Mailbox-9",       "Mailbox-9"         },
      { "Rear",            "Rear"              },
      { "Right",           "Right"             },
      { "Side",            "Side"              },
      { "Stacker-1",       "Stacker-1"         },
      { "Stacker-2",       "Stacker-2"         },
      { "Stacker-3",       "Stacker-3"         },
      { "Stacker-4",       "Stacker-4"         },
      { "Stacker-5",       "Stacker-5"         },
      { "Stacker-6",       "Stacker-6"         },
      { "Stacker-7",       "Stacker-7"         },
      { "Stacker-8",       "Stacker-8"         },
      { "Stacker-9",       "Stacker-9"         },
      { "Top",             "Top"               },
      { "Tray-1",          "Tray-1"            },
      { "Tray-2",          "Tray-2"            },
      { "Tray-3",          "Tray-3"            },
      { "Tray-4",          "Tray-4"            },
      { "Tray-5",          "Tray-5"            },
      { "Tray-6",          "Tray-6"            },
      { "Tray-7",          "Tray-7"            },
      { "Tray-8",          "Tray-8"            },
      { "Tray-9",          "Tray-9"            }
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
         if (ppszOmniValue)
         {
            *ppszOmniValue = ammFromUPDFToOmni[iMid].pszTo;
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

bool UPDFDeviceOutputBin::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   typedef struct _OutputBinMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromOmniToUPDF[] = {
      { "Booklet",         "Booklet"           },
      { "Bottom",          "Bottom"            },
      { "Center",          "Center"            },
      { "FaceDown",        "Face-down"         },
      { "FaceUp",          "Face-up"           },
      { "FitMedia",        "Fit-media"         },
      { "LargeCapacity",   "Large-capacity"    },
      { "Left",            "Left"              },
      { "Mailbox-1",       "Mailbox-1"         },
      { "Mailbox-2",       "Mailbox-2"         },
      { "Mailbox-3",       "Mailbox-3"         },
      { "Mailbox-4",       "Mailbox-4"         },
      { "Mailbox-5",       "Mailbox-5"         },
      { "Mailbox-6",       "Mailbox-6"         },
      { "Mailbox-7",       "Mailbox-7"         },
      { "Mailbox-8",       "Mailbox-8"         },
      { "Mailbox-9",       "Mailbox-9"         },
      { "Rear",            "Rear"              },
      { "Right",           "Right"             },
      { "Side",            "Side"              },
      { "Stacker-1",       "Stacker-1"         },
      { "Stacker-2",       "Stacker-2"         },
      { "Stacker-3",       "Stacker-3"         },
      { "Stacker-4",       "Stacker-4"         },
      { "Stacker-5",       "Stacker-5"         },
      { "Stacker-6",       "Stacker-6"         },
      { "Stacker-7",       "Stacker-7"         },
      { "Stacker-8",       "Stacker-8"         },
      { "Stacker-9",       "Stacker-9"         },
      { "Top",             "Top"               },
      { "Tray-1",          "Tray-1"            },
      { "Tray-2",          "Tray-2"            },
      { "Tray-3",          "Tray-3"            },
      { "Tray-4",          "Tray-4"            },
      { "Tray-5",          "Tray-5"            },
      { "Tray-6",          "Tray-6"            },
      { "Tray-7",          "Tray-7"            },
      { "Tray-8",          "Tray-8"            },
      { "Tray-9",          "Tray-9"            }
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

void UPDFDeviceOutputBin::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceOutputBin::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceOutputBin: "
       << DeviceOutputBin::toString (oss2)
       << "}";

   return oss.str ();
}

std::ostream&
operator<< (std::ostream&              os,
            const UPDFDeviceOutputBin& const_self)
{
   UPDFDeviceOutputBin& self = const_cast<UPDFDeviceOutputBin&>(const_self);
   std::ostringstream   oss;

   os << self.toString (oss);

   return os;
}
