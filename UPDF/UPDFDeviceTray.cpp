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

#include <UPDFDeviceTray.hpp>

UPDFDeviceTray::
UPDFDeviceTray (Device      *pDevice,
                PSZRO        pszJobProperties,
                int          iType,
                BinaryData  *pbdData,
                XmlNodePtr   node)
   : DeviceTray (pDevice,
                 pszJobProperties,
                 iType,
                 pbdData)
{
   node_d = node;
}

UPDFDeviceTray::
~UPDFDeviceTray ()
{
}

static XmlNodePtr
skipInvalidTrays (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceTray *
createFromXMLNode (Device      *pDevice,
                   XmlNodePtr   node)
{
   UPDFDevice         *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);
   PSZRO               pszTrayName = 0;
   int                 iType       = 0; // @TBD
   BinaryData         *pbdData     = 0; // @TBD
   std::ostringstream  oss;
   DeviceTray         *pTrayRet    = 0;

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   pszTrayName = XMLGetProp (node, "ClassifyingID");

   if (pszTrayName)
   {
      PSZRO pszOmniJP = 0;

      if (UPDFDeviceTray::mapUPDFToOmni (pszTrayName,
                                         &pszOmniJP))
      {
         oss << "InputTray=" << pszOmniJP;

         pTrayRet = new UPDFDeviceTray (pUPDFDevice,
                                        oss.str ().c_str (),
                                        iType,
                                        pbdData,
                                        node);
      }
   }

   if (pszTrayName)
   {
      XMLFree ((void *)pszTrayName);
   }

   return pTrayRet;
}

DeviceTray * UPDFDeviceTray::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr  nodeTrays   = 0;
   XmlNodePtr  nodeItem    = 0;
   XmlNodePtr  nodeFound   = 0;
   PSZRO       pszTrayName = 0;
   PSZRO       pszOmniName = 0;
   DeviceTray *pTrayRet    = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceTray::mapOmniToUPDF (pszOmniName, &pszTrayName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeTrays = findTrays (pUPDFDevice);

   if (!nodeTrays)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !nodeTrays" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeTrays));

   while (  nodeItem != 0
         && !pTrayRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszTrayName))
         {
            nodeFound = nodeItem;
         }

         XMLFree ((void *)pszNodeId);
      }

      if (nodeFound)
      {
         pTrayRet = createFromXMLNode (pDevice, nodeItem);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

done:
   if (!pTrayRet)
   {
      pTrayRet = pUPDFDevice->getDefaultTray ();
   }

   return pTrayRet;
}

DeviceTray * UPDFDeviceTray::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceTray::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr  nodeTrays   = 0;
   XmlNodePtr  nodeItem    = 0;
   XmlNodePtr  nodeFound   = 0;
   PSZRO       pszTrayName = 0;
   PSZRO       pszOmniName = 0;
   bool        fRet        = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties,
                       &pszOmniName,
                       0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   if (!UPDFDeviceTray::mapOmniToUPDF (pszOmniName, &pszTrayName))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !UPDF Name" << std::endl;
#endif

      goto done;
   }

   nodeTrays = findTrays (pUPDFDevice);

   if (!nodeTrays)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": !nodeTrays" << std::endl;
#endif

      return 0;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeTrays));

   while (  nodeItem != 0
         && !fRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         if (0 == strcmp (pszNodeId, pszTrayName))
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

PSZCRO UPDFDeviceTray::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceTray::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet        = 0;
   UPDFDevice                 *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeTrays   = 0;
   XmlNodePtr                  nodeTray    = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeTrays = findTrays (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": nodeTrays = " << std::hex << (int *)nodeTrays << std::dec << std::endl;
#endif

   if (!nodeTrays)
   {
      goto done;
   }

   nodeTray = XMLFirstNode (XMLGetChildrenNode (nodeTrays));
   nodeTray = skipInvalidTrays (nodeTray);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": nodeTray = " << std::hex << (int *)nodeTray << std::dec << std::endl;
#endif

   while (nodeTray)
   {
      DeviceTray    *pTray = 0;
      JobProperties *pJP   = 0;

      pTray = createFromXMLNode (pDevice_d, nodeTray);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": pTray = " << std::hex << (int *)pTray << std::dec << std::endl;
#endif

      if (pTray)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pTray->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pTray;
      }

      nodeTray = XMLNextNode (nodeTray);
      nodeTray = skipInvalidTrays (nodeTray);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceTray ()) DebugOutput::getErrorStream () << "UPDFDeviceTray::" << __FUNCTION__ << ": nodeTray = " << std::hex << (int *)nodeTray << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceTray::
findTrays (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeTrays = 0;

   if (!pUPDFDevice)
   {
      return nodeTrays;
   }

   if (  ((nodeTrays = FINDUDRENTRY (pUPDFDevice, nodeTrays, "PrintCapabilities",   UPDFDeviceTray)) != 0)
      && ((nodeTrays = FINDUDRENTRY (pUPDFDevice, nodeTrays, "Features",            UPDFDeviceTray)) != 0)
      && ((nodeTrays = FINDUDRENTRY (pUPDFDevice, nodeTrays, "MediaInputTrayCheck", UPDFDeviceTray)) != 0)
      )
      return nodeTrays;

   return nodeTrays;
}

bool UPDFDeviceTray::
mapUPDFToOmni (PSZCRO  pszUPDFValue,
               PSZRO  *ppszOmniValue)
{
   typedef struct _TrayMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromUPDFToOmni[] = {
      { "any-large-format",  "AnyLargeFormat",  },
      { "any-small-format",  "AnySmallFormat"   },
      { "bottom",            "Bottom"           },
      { "bypass-tray",       "BypassTray"       },
      { "bypass-tray-1",     "BypassTray-1"     },
      { "bypass-tray-2",     "BypassTray-2"     },
      { "bypass-tray-3",     "BypassTray-3"     },
      { "bypass-tray-4",     "BypassTray-4"     },
      { "bypass-tray-5",     "BypassTray-5"     },
      { "bypass-tray-6",     "BypassTray-6"     },
      { "bypass-tray-7",     "BypassTray-7"     },
      { "bypass-tray-8",     "BypassTray-8"     },
      { "bypass-tray-9",     "BypassTray-9"     },
      { "continuous",        "Continuous"       },
      { "device-setting",    "AutoSelect"       },
      { "disc",              "Disc"             },
      { "disc-1",            "Disc-1"           },
      { "disc-2",            "Disc-2"           },
      { "disc-3",            "Disc-3"           },
      { "disc-4",            "Disc-4"           },
      { "disc-5",            "Disc-5"           },
      { "disc-6",            "Disc-6"           },
      { "disc-7",            "Disc-7"           },
      { "disc-8",            "Disc-8"           },
      { "disc-9",            "Disc-9"           },
      { "envelope",          "Envelope"         },
      { "envelope-1",        "Envelope-1"       },
      { "envelope-2",        "Envelope-2"       },
      { "envelope-3",        "Envelope-3"       },
      { "envelope-4",        "Envelope-4"       },
      { "envelope-5",        "Envelope-5"       },
      { "envelope-6",        "Envelope-6"       },
      { "envelope-7",        "Envelope-7"       },
      { "envelope-8",        "Envelope-8"       },
      { "envelope-9",        "Envelope-9"       },
      { "front",             "Front"            },
      { "insert-tray",       "InsertTray"       },
      { "insert-tray-1",     "InsertTray-1"     },
      { "insert-tray-2",     "InsertTray-2"     },
      { "insert-tray-3",     "InsertTray-3"     },
      { "insert-tray-4",     "InsertTray-4"     },
      { "insert-tray-5",     "InsertTray-5"     },
      { "insert-tray-6",     "InsertTray-6"     },
      { "insert-tray-7",     "InsertTray-7"     },
      { "insert-tray-8",     "InsertTray-8"     },
      { "insert-tray-9",     "InsertTray-9"     },
      { "large-capacity",    "LargeCapacity"    },
      { "large-capacity-1",  "LargeCapacity-1"  },
      { "large-capacity-2",  "LargeCapacity-2"  },
      { "large-capacity-3",  "LargeCapacity-3"  },
      { "large-capacity-4",  "LargeCapacity-4"  },
      { "large-capacity-5",  "LargeCapacity-5"  },
      { "large-capacity-6",  "LargeCapacity-6"  },
      { "large-capacity-7",  "LargeCapacity-7"  },
      { "large-capacity-8",  "LargeCapacity-8"  },
      { "large-capacity-9",  "LargeCapacity-9"  },
      { "left",              "Left"             },
      { "middle",            "Middle"           },
      { "panel-select",      "PanelSelect"      },
      { "rear",              "Rear"             },
      { "right",             "Right"            },
      { "roll",              "Roll"             },
      { "roll-1",            "Roll-1"           },
      { "roll-2",            "Roll-2"           },
      { "roll-3",            "Roll-3"           },
      { "roll-4",            "Roll-4"           },
      { "roll-5",            "Roll-5"           },
      { "roll-6",            "Roll-6"           },
      { "roll-7",            "Roll-7"           },
      { "roll-8",            "Roll-8"           },
      { "roll-9",            "Roll-9"           },
      { "side",              "Side"             },
      { "top",               "Top"              },
      { "tray",              "Tray"             },
      { "tray-1",            "Tray-1"           },
      { "tray-2",            "Tray-2"           },
      { "tray-3",            "Tray-3"           },
      { "tray-4",            "Tray-4"           },
      { "tray-5",            "Tray-5"           },
      { "tray-6",            "Tray-6"           },
      { "tray-7",            "Tray-7"           },
      { "tray-8",            "Tray-8"           },
      { "tray-9",            "Tray-9"           }
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

bool UPDFDeviceTray::
mapOmniToUPDF (PSZCRO  pszOmniValue,
               PSZRO  *ppszUPDFValue)
{
   typedef struct _TrayMapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } SIDEMAPPING, *PSIDEMAPPING;
   SIDEMAPPING ammFromOmniToUPDF[] = {
      { "AnyLargeFormat",  "any-large-format"  },
      { "AnySmallFormat",  "any-small-format"  },
      { "AutoSelect",      "device-setting"    },
      { "Bottom",          "bottom"            },
      { "BypassTray",      "bypass-tray"       },
      { "BypassTray-1",    "bypass-tray-1"     },
      { "BypassTray-2",    "bypass-tray-2"     },
      { "BypassTray-3",    "bypass-tray-3"     },
      { "BypassTray-4",    "bypass-tray-4"     },
      { "BypassTray-5",    "bypass-tray-5"     },
      { "BypassTray-6",    "bypass-tray-6"     },
      { "BypassTray-7",    "bypass-tray-7"     },
      { "BypassTray-8",    "bypass-tray-8"     },
      { "BypassTray-9",    "bypass-tray-9"     },
      { "Continuous",      "continuous"        },
      { "Disc",            "disc"              },
      { "Disc-1",          "disc-1"            },
      { "Disc-2",          "disc-2"            },
      { "Disc-3",          "disc-3"            },
      { "Disc-4",          "disc-4"            },
      { "Disc-5",          "disc-5"            },
      { "Disc-6",          "disc-6"            },
      { "Disc-7",          "disc-7"            },
      { "Disc-8",          "disc-8"            },
      { "Disc-9",          "disc-9"            },
      { "Envelope",        "envelope"          },
      { "Envelope-1",      "envelope-1"        },
      { "Envelope-2",      "envelope-2"        },
      { "Envelope-3",      "envelope-3"        },
      { "Envelope-4",      "envelope-4"        },
      { "Envelope-5",      "envelope-5"        },
      { "Envelope-6",      "envelope-6"        },
      { "Envelope-7",      "envelope-7"        },
      { "Envelope-8",      "envelope-8"        },
      { "Envelope-9",      "envelope-9"        },
      { "Front",           "front"             },
      { "InsertTray",      "insert-tray"       },
      { "InsertTray-1",    "insert-tray-1"     },
      { "InsertTray-2",    "insert-tray-2"     },
      { "InsertTray-3",    "insert-tray-3"     },
      { "InsertTray-4",    "insert-tray-4"     },
      { "InsertTray-5",    "insert-tray-5"     },
      { "InsertTray-6",    "insert-tray-6"     },
      { "InsertTray-7",    "insert-tray-7"     },
      { "InsertTray-8",    "insert-tray-8"     },
      { "InsertTray-9",    "insert-tray-9"     },
      { "LargeCapacity",   "large-capacity"    },
      { "LargeCapacity-1", "large-capacity-1"  },
      { "LargeCapacity-2", "large-capacity-2"  },
      { "LargeCapacity-3", "large-capacity-3"  },
      { "LargeCapacity-4", "large-capacity-4"  },
      { "LargeCapacity-5", "large-capacity-5"  },
      { "LargeCapacity-6", "large-capacity-6"  },
      { "LargeCapacity-7", "large-capacity-7"  },
      { "LargeCapacity-8", "large-capacity-8"  },
      { "LargeCapacity-9", "large-capacity-9"  },
      { "Left",            "left"              },
      { "Middle",          "middle"            },
      { "PanelSelect",     "panel-select"      },
      { "Rear",            "rear"              },
      { "Right",           "right"             },
      { "Roll",            "roll"              },
      { "Roll-1",          "roll-1"            },
      { "Roll-2",          "roll-2"            },
      { "Roll-3",          "roll-3"            },
      { "Roll-4",          "roll-4"            },
      { "Roll-5",          "roll-5"            },
      { "Roll-6",          "roll-6"            },
      { "Roll-7",          "roll-7"            },
      { "Roll-8",          "roll-8"            },
      { "Roll-9",          "roll-9"            },
      { "Side",            "side"              },
      { "Top",             "top"               },
      { "Tray",            "tray"              },
      { "Tray-1",          "tray-1"            },
      { "Tray-2",          "tray-2"            },
      { "Tray-3",          "tray-3"            },
      { "Tray-4",          "tray-4"            },
      { "Tray-5",          "tray-5"            },
      { "Tray-6",          "tray-6"            },
      { "Tray-7",          "tray-7"            },
      { "Tray-8",          "tray-8"            },
      { "Tray-9",          "tray-9"            }
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

void UPDFDeviceTray::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceTray::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceTray: "
       << DeviceTray::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceTray& const_self)
{
   UPDFDeviceTray&    self = const_cast<UPDFDeviceTray&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
