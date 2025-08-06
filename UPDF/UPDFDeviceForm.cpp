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

#include <UPDFDeviceForm.hpp>
#include <UPDFDeviceInstance.hpp>

UPDFDeviceForm::
UPDFDeviceForm (Device      *pDevice,
                PSZRO        pszJobProperties,
                int          iCapabilities,
                BinaryData  *pbdData,
                HardCopyCap *hcInfo,
                XmlNodePtr   node)
   : DeviceForm (pDevice,
                 pszJobProperties,
                 iCapabilities,
                 pbdData,
                 hcInfo)
{
   node_d = node;
}

UPDFDeviceForm::
~UPDFDeviceForm ()
{
}

#if 0
static int
convertUPDFMarginToOmniMargin (PSZCRO pszMarginInfo)
{
   float flMargin = 0.0;
   int   iReturn  = -1;

   if (1 == sscanf (pszMarginInfo, "%f", &flMargin))
   {
      PSZCRO pszEnd = pszMarginInfo + strlen (pszMarginInfo);

      if (0 == strcmp (pszEnd - 2, "in"))
      {
         iReturn = (int)(flMargin * 25400.0);
      }
      else if (0 == strcmp (pszEnd - 2, "mm"))
      {
         iReturn = (int)(flMargin * 1000.0);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << " Unknown margin encoding!" << std::endl;
#endif
      }
   }

   return iReturn;
}
#endif

static XmlNodePtr
skipInvalidForms (XmlNodePtr node)
{
   if (!node)
   {
      return 0;
   }

   return node;
}

static DeviceForm *
createFromXMLNode (Device     *pDevice,
                   XmlNodePtr  node)
{
   UPDFDevice *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      return 0;
   }

   int                 iCapabilities      = 0;
   BinaryData         *pbdData            = 0;
   int                 iTopMargin         = 0;
   int                 iBottomMargin      = 0;
   int                 iLeftMargin        = 0;
   int                 iRightMargin       = 0;
   XmlNodePtr          nodeMargins        = 0;
   PSZRO               pszFormName        = 0;
   DeviceInstance     *pInstance          = pUPDFDevice->getDeviceInstance ();
   UPDFDeviceInstance *pUPDFInstance      = UPDFDeviceInstance::isAUPDFDeviceInstance (pInstance);
   std::ostringstream  oss;

   pszFormName = XMLGetProp (node, "ClassifyingID");

   nodeMargins = pUPDFInstance->getXMLObjectNode ("MediaHardwareMargins");

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": nodeMargins = " << nodeMargins << std::endl;
#endif

   if (nodeMargins)
   {
#if 0
      PSZCRO pszTopMargin    = XMLGetProp (nodeMargins, "Top");
      PSZCRO pszBottomMargin = XMLGetProp (nodeMargins, "Bottom");
      PSZCRO pszLeftMargin   = XMLGetProp (nodeMargins, "Left");
      PSZCRO pszRightMargin  = XMLGetProp (nodeMargins, "Right");

      iTopMargin    = convertUPDFMarginToOmniMargin (pszTopMargin);
      iBottomMargin = convertUPDFMarginToOmniMargin (pszBottomMargin);
      iLeftMargin   = convertUPDFMarginToOmniMargin (pszLeftMargin);
      iRightMargin  = convertUPDFMarginToOmniMargin (pszRightMargin);

      if (pszTopMargin)
      {
         XMLFree ((void *)pszTopMargin);
      }
      if (pszBottomMargin)
      {
         XMLFree ((void *)pszBottomMargin);
      }
      if (pszLeftMargin)
      {
         XMLFree ((void *)pszLeftMargin);
      }
      if (pszRightMargin)
      {
         XMLFree ((void *)pszRightMargin);
      }
#else
      PSZCRO pszMargins = XMLGetProp (nodeMargins, "ClassifyingID");
      float  flLeft     = 0.0;
      float  flTop      = 0.0;
      float  flRight    = 0.0;
      float  flBottom   = 0.0;
      int    iRC        = -1;

      if (pszMargins)
      {
         iRC = sscanf (pszMargins,
                       "margins_left-%f_top-%f_right-%f_bottom-%f",
                       &flLeft,
                       &flTop,
                       &flRight,
                       &flBottom);

#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceForm ())
            DebugOutput::getErrorStream () << "UPDFDeviceForm::"
                                           << __FUNCTION__
                                           << ": iRC = "
                                           << iRC
                                           << ", flLeft = "
                                           << flLeft
                                           << ", flTop = "
                                           << flTop
                                           << ", flRight = "
                                           << flRight
                                           << ", flBottom = "
                                           << flBottom
                                           << std::endl;
#endif

         if (4 == iRC)
         {
            if (0 == strcasecmp (pszMargins + strlen (pszMargins) - 2, "in"))
            {
               iTopMargin    = (int)(flTop    * 25400.0);
               iBottomMargin = (int)(flBottom * 25400.0);
               iLeftMargin   = (int)(flLeft   * 25400.0);
               iRightMargin  = (int)(flRight  * 25400.0);

#ifndef RETAIL
               if (DebugOutput::shouldOutputUPDFDeviceForm ())
                  DebugOutput::getErrorStream () << "UPDFDeviceForm::"
                                                 << __FUNCTION__
                                                 << ": INCHES iTopMargin = "
                                                 << iTopMargin
                                                 << ", iBottomMargin = "
                                                 << iBottomMargin
                                                 << ", iLeftMargin = "
                                                 << iLeftMargin
                                                 << ", iRightMargin = "
                                                 << iRightMargin
                                                 << std::endl;
#endif
            }
         }
      }

      if (pszMargins)
      {
         XMLFree ((void *)pszMargins);
      }
#endif
   }

   oss << "Form=" << SAFE_PRINT_PSZ (pszFormName);

   if (pszFormName)
   {
      XMLFree ((void *)pszFormName);
   }

   return new UPDFDeviceForm (pDevice,
                              oss.str ().c_str (),
                              iCapabilities,
                              pbdData,
                              new HardCopyCap (iLeftMargin,
                                               iTopMargin,
                                               iRightMargin,
                                               iBottomMargin),
                              node);
}

DeviceForm * UPDFDeviceForm::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   UPDFDevice        *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice);
   XmlNodePtr         nodeForms   = 0;
   DeviceForm        *pFormRet    = 0;
   XmlNodePtr         nodeItem    = 0;
   XmlNodePtr         nodeFound   = 0;
   int                idxFormName = -1;
   std::ostringstream oss;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << " (" << pDevice << ", \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, 0, &idxFormName, 0, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   nodeForms = findForms (pUPDFDevice);

   if (!nodeForms)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !nodeForms" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeForms));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !nodeItem" << std::endl;
#endif

      goto done;
   }

   while (  nodeItem
         && !pFormRet
         )
   {
      PSZCRO pszNodeId = XMLGetProp (nodeItem, "ClassifyingID");

      if (pszNodeId)
      {
         int idxNodeFormName = -1;

         oss.str ("");
         oss << "Form=" << pszNodeId;

         if (getComponents (oss.str ().c_str (), 0, &idxNodeFormName, 0, 0))
         {
            if (idxNodeFormName == idxFormName)
            {
               nodeFound = nodeItem;
            }
         }

         XMLFree ((void *)pszNodeId);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": ClassifyingID not found!" << std::endl;
#endif
      }

      if (nodeFound)
      {
         pFormRet = createFromXMLNode (pDevice, nodeFound);
      }

      nodeItem = XMLNextNode (nodeItem);
   }

#if 0
#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": nodeFound = " << nodeFound << std::endl;
#endif

   UPDFDeviceInstance *pInstance    = dynamic_cast <UPDFDeviceInstance *>(pDevice->getInstance ());
   UPDFObjectStore    *pObjectStore = 0;

   if (!pInstance)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !pInstance" << std::endl;
#endif

      return 0;
   }

   pObjectStore = pInstance->getObjectStore ();

   if (!pObjectStore)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !pObjectStore" << std::endl;
#endif

      return 0;
   }

   if (!pObjectStore->applyRequiredJobProperties (pszJobProperties,
                                                  "MediaSize",
                                                  NULL))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !applyRequiredJobProperties" << std::endl;
#endif

      delete pObjectStore;

      return 0;
   }

   delete pObjectStore;
#endif

done:
   if (!pFormRet)
   {
      pFormRet = pUPDFDevice->getDefaultForm ();
   }

   return pFormRet;
}

DeviceForm * UPDFDeviceForm::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool UPDFDeviceForm::
isSupported (PSZCRO pszJobProperties)
{
   UPDFDevice        *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr         nodeForms   = 0;
   XmlNodePtr         nodeItem    = 0;
   XmlNodePtr         nodeFound   = 0;
   int                idxFormName = -1;
   std::ostringstream oss;
   bool               fRet        = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ(pszJobProperties) << "\")" << std::endl;
#endif

   if (!pUPDFDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !pUPDFDevice" << std::endl;
#endif

      goto done;
   }

   if (!getComponents (pszJobProperties, 0, &idxFormName, 0, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !getComponents (\"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

      goto done;
   }

   nodeForms = findForms (pUPDFDevice);

   if (!nodeForms)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !nodeForms" << std::endl;
#endif

      goto done;
   }

   nodeItem = XMLFirstNode (XMLGetChildrenNode (nodeForms));

   if (!nodeItem)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": !nodeItem" << std::endl;
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
         int idxNodeFormName = -1;

         oss.str ("");
         oss << "Form=" << pszNodeId;

         if (getComponents (oss.str ().c_str (), 0, &idxNodeFormName, 0, 0))
         {
            if (idxNodeFormName == idxFormName)
            {
               nodeFound = nodeItem;
            }
         }

         XMLFree ((void *)pszNodeId);
      }
      else
      {
#ifndef RETAIL
         if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": ClassifyingID not found!" << std::endl;
#endif
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

PSZCRO UPDFDeviceForm::
getDeviceID ()
{
   return 0;
}

Enumeration * UPDFDeviceForm::
getEnumeration (bool fInDeviceSpecific)
{
   MultiJobPropertyEnumerator *pRet        = 0;
   UPDFDevice                 *pUPDFDevice = UPDFDevice::isAUPDFDevice (pDevice_d);
   XmlNodePtr                  nodeForms   = 0;
   XmlNodePtr                  nodeForm    = 0;

   pRet = new MultiJobPropertyEnumerator ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": pUPDFDevice = " << std::hex << (int *)pUPDFDevice << std::dec << std::endl;
#endif

   if (!pUPDFDevice)
   {
      goto done;
   }

   nodeForms = findForms (pUPDFDevice);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": nodeForms = " << std::hex << (int *)nodeForms << std::dec << std::endl;
#endif

   if (!nodeForms)
   {
      goto done;
   }

   nodeForm = XMLFirstNode (XMLGetChildrenNode (nodeForms));
   nodeForm = skipInvalidForms (nodeForm);

#ifndef RETAIL
   if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": nodeForm = " << std::hex << (int *)nodeForm << std::dec << std::endl;
#endif

   while (nodeForm)
   {
      DeviceForm    *pForm = 0;
      JobProperties *pJP   = 0;

      pForm = createFromXMLNode (pDevice_d, nodeForm);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": pForm = " << std::hex << (int *)pForm << std::dec << std::endl;
#endif

      if (pForm)
      {
         std::string *pstringJPs = 0;

         pstringJPs = pForm->getJobProperties (fInDeviceSpecific);

         if (pstringJPs)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": *pstringJPs = " << *pstringJPs << std::endl;
#endif

            pJP = new JobProperties (pstringJPs->c_str ());

            pRet->addElement (pJP);

            delete pstringJPs;
         }

         delete pForm;
      }

      nodeForm = XMLNextNode (nodeForm);
      nodeForm = skipInvalidForms (nodeForm);

#ifndef RETAIL
      if (DebugOutput::shouldOutputUPDFDeviceForm ()) DebugOutput::getErrorStream () << "UPDFDeviceForm::" << __FUNCTION__ << ": nodeForm = " << std::hex << (int *)nodeForm << std::dec << std::endl;
#endif
   }

done:
   return pRet;
}

XmlNodePtr UPDFDeviceForm::
findForms (UPDFDevice *pUPDFDevice)
{
   XmlNodePtr nodeForms = 0;

   if (!pUPDFDevice)
   {
      return nodeForms;
   }

   if (  ((nodeForms = FINDUDRENTRY (pUPDFDevice, nodeForms, "PrintCapabilities",  UPDFDeviceForm)) != 0)
      && ((nodeForms = FINDUDRENTRY (pUPDFDevice, nodeForms, "Features",           UPDFDeviceForm)) != 0)
      && ((nodeForms = FINDUDRENTRY (pUPDFDevice, nodeForms, "MediaSize",          UPDFDeviceForm)) != 0)
      )
      return nodeForms;

   return nodeForms;
}

#ifndef RETAIL

void UPDFDeviceForm::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string UPDFDeviceForm::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{UPDFDeviceForm: "
       << DeviceForm::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const UPDFDeviceForm& const_self)
{
   UPDFDeviceForm&    self = const_cast<UPDFDeviceForm&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
