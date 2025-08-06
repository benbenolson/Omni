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
#include <XMLDeviceForm.hpp>

#include <JobProperties.hpp>

XMLDeviceForm::
XMLDeviceForm (Device      *pDevice,
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
   node_d        = node;
   pszDeviceID_d = 0;
}

XMLDeviceForm::
~XMLDeviceForm ()
{
   if (pszDeviceID_d)
   {
      free ((void *)pszDeviceID_d);
      pszDeviceID_d = 0;
   }
}

DeviceForm * XMLDeviceForm::
createS (Device *pDevice,
         PSZCRO  pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return 0;
   }

   XmlDocPtr   docDeviceForms  = pXMLDevice->getDocForms ();
   XmlNodePtr  rootDeviceForms = XMLDocGetRootElement (docDeviceForms);
   XmlNodePtr  elmDeviceForm   = 0;
   DeviceForm *pFormRet        = 0;

   if (!rootDeviceForms)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !rootDeviceForms " << std::endl;
#endif

      return 0;
   }

   elmDeviceForm = XMLFirstNode (rootDeviceForms);
   if (!elmDeviceForm)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !elmDeviceForm " << std::endl;
#endif

      return 0;
   }

   PSZRO pszFormName = 0;

   if (!getComponents (pszJobProperties, &pszFormName, 0, 0, 0))
   {
      return pXMLDevice->getDefaultForm ();
   }

   elmDeviceForm = XMLFirstNode (XMLGetChildrenNode (elmDeviceForm));

   while (  elmDeviceForm
         && !pFormRet
         )
   {
      PSZCRO pszElmFormName = getXMLContentString (elmDeviceForm,
                                                   docDeviceForms,
                                                   "name");

      if (  pszFormName
         && pszElmFormName
         )
      {
         if (0 == strcmp (pszFormName, pszElmFormName))
         {
            PSZRO        pszCapabilities = 0;
            int          iCapabilities   = 0;
            PSZRO        pszCommand      = 0;
            BinaryData  *pbdData         = 0;
            int          iLeft           = 0;
            int          iTop            = 0;
            int          iRight          = 0;
            int          iBottom         = 0;
            HardCopyCap *hcInfo          = 0;
            XmlNodePtr   elmHCC          = 0;

            try
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": Creating \"" << pszFormName << "\"" << std::endl;
#endif

               // Read in the capabilities
               pszCapabilities = getXMLContentString (elmDeviceForm, docDeviceForms, "formCapabilities");

               if (pszCapabilities)
               {
                  iCapabilities = DeviceForm::getReservedValue (pszCapabilities);

                  XMLFree ((void *)pszCapabilities);
               }

               // Read in the command
               pszCommand = getXMLContentString (elmDeviceForm, docDeviceForms, "command");

               if (pszCommand)
               {
                  byte *pbData = 0;
                  int   cbData = 0;

                  if (XMLDevice::parseBinaryData (pszCommand,
                                                  &pbData,
                                                  &cbData))
                  {
                     pbdData = new BinaryDataDelete (pbData, cbData);
                  }

                  XMLFree ((void *)pszCommand);
               }

               // Read in the HardCopyCap
               elmHCC  = XMLFindEntry (elmDeviceForm, "hardCopyCap", false);
               iLeft   = getXMLContentInt (elmHCC, docDeviceForms, "hardCopyCapLeft");
               iTop    = getXMLContentInt (elmHCC, docDeviceForms, "hardCopyCapTop");
               iRight  = getXMLContentInt (elmHCC, docDeviceForms, "hardCopyCapRight");
               iBottom = getXMLContentInt (elmHCC, docDeviceForms, "hardCopyCapBottom");
               hcInfo  = new HardCopyCap (iLeft, iTop, iRight, iBottom);

#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceForm ())
               {
                  DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": iCapabilities = 0x" << std::hex << iCapabilities << std::dec << std::endl;
                  if (pbdData)
                     DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": pbdData = " << *pbdData << std::endl;
                  else
                     DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": pbdData is null!" << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": iLeft   = " << iLeft << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": iTop    = " << iTop << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": iRight  = " << iRight << std::endl;
                  DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": iBottom = " << iBottom << std::endl;
               }
#endif

               // Create the object
               pFormRet = new XMLDeviceForm (pDevice,
                                             pszJobProperties,
                                             iCapabilities,
                                             pbdData,
                                             hcInfo,
                                             elmDeviceForm);

               if (hcInfo)
               {
                  hcInfo->setOwner (pFormRet);
               }
            }
            catch (std::string *pstringError)
            {
#ifndef RETAIL
               if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": Error: " << *pstringError << std::endl;
#endif

               delete pstringError;
            }
         }
      }

      if (pszElmFormName)
      {
         XMLFree ((void *)pszElmFormName);
      }

      elmDeviceForm = XMLNextNode (elmDeviceForm);
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": returning " << pFormRet << std::endl;
#endif

   if (pszFormName)
   {
      free ((void *)pszFormName);
   }

   return pFormRet;
}

DeviceForm * XMLDeviceForm::
create (Device *pDevice,
        PSZCRO  pszJobProperties)
{
   return createS (pDevice, pszJobProperties);
}

bool XMLDeviceForm::
isSupported (PSZCRO pszJobProperties)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !pXMLDevice " << std::endl;
#endif

      return false;
   }

   XmlDocPtr   docDeviceForms  = pXMLDevice->getDocForms ();
   XmlNodePtr  rootDeviceForms = XMLDocGetRootElement (docDeviceForms);
   XmlNodePtr  elmDeviceForm   = 0;
   bool        fFound          = false;

   if (!rootDeviceForms)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !rootDeviceForms " << std::endl;
#endif

      return false;
   }

   elmDeviceForm = XMLFirstNode (rootDeviceForms);
   if (!elmDeviceForm)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !elmDeviceForm " << std::endl;
#endif

      return false;
   }

   PSZRO pszFormName = 0;

   if (!getComponents (pszJobProperties, &pszFormName, 0, 0, 0))
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": !getComponents " << std::endl;
#endif

      return false;
   }

   elmDeviceForm = XMLFirstNode (XMLGetChildrenNode (elmDeviceForm));

   while (  elmDeviceForm
         && !fFound
         )
   {
      PSZCRO pszElmFormName = getXMLContentString (elmDeviceForm,
                                                   docDeviceForms,
                                                   "name");

      if (  pszFormName
         && pszElmFormName
         )
      {
         if (0 == strcmp (pszFormName, pszElmFormName))
         {
            fFound = true;
         }
      }

      if (pszElmFormName)
      {
         XMLFree ((void *)pszElmFormName);
      }

      elmDeviceForm = XMLNextNode (elmDeviceForm);
   }

   if (pszFormName)
   {
      free ((void *)pszFormName);
   }

   return fFound;
}

PSZCRO XMLDeviceForm::
getDeviceID ()
{
   if (!pszDeviceID_d)
   {
      if (node_d)
      {
         pszDeviceID_d = getXMLContentString (node_d, XMLGetDocNode (node_d), "deviceID");
      }
   }

#ifdef DEBUG
   if (DebugOutput::shouldOutputXMLDeviceForm ()) DebugOutput::getErrorStream () << "XMLDeviceForm::" << __FUNCTION__ << ": returning " << (pszDeviceID_d ? pszDeviceID_d : "NULL") << std::endl;
#endif

   return pszDeviceID_d;
}

class XMLForm_Enumerator : public Enumeration
{
public:
   XMLForm_Enumerator (Device     *pDevice,
                       XmlNodePtr  nodeItem,
                       bool        fInDeviceSpecific)
   {
      XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice);

      pXMLDevice_d        = pXMLDevice;
      docDeviceForms_d    = 0;
      nodeItem_d          = nodeItem;
      fInDeviceSpecific_d = fInDeviceSpecific;

      if (pXMLDevice)
      {
         docDeviceForms_d = pXMLDevice->getDocForms ();
      }
      else
      {
         nodeItem_d = 0;
      }
   }

   virtual ~
   XMLForm_Enumerator ()
   {
      pXMLDevice_d        = 0;
      docDeviceForms_d    = 0;
      nodeItem_d          = 0;
      fInDeviceSpecific_d = false;
   }

   virtual bool
   hasMoreElements ()
   {
      return nodeItem_d ? true : false;
   }

   virtual void *
   nextElement ()
   {
      void *pvRet = 0;

      if (nodeItem_d)
      {
         PSZRO pszFormName = 0;

         if (fInDeviceSpecific_d)
         {
            pszFormName = getXMLContentString (nodeItem_d,
                                               docDeviceForms_d,
                                               "deviceID");
         }

         if (!pszFormName)
         {
            pszFormName = getXMLContentString (nodeItem_d,
                                               docDeviceForms_d,
                                               "name");
         }

         if (pszFormName)
         {
            std::ostringstream oss;

            oss << "Form=" << pszFormName;

            pvRet = (void *)new JobProperties (oss.str ().c_str ());

            XMLFree ((void *)pszFormName);
         }

         nodeItem_d = XMLNextNode (nodeItem_d);
      }

      return pvRet;
   }

private:
   XMLDevice  *pXMLDevice_d;
   XmlDocPtr   docDeviceForms_d;
   XmlNodePtr  nodeItem_d;
   bool        fInDeviceSpecific_d;
};

Enumeration * XMLDeviceForm::
getEnumeration (bool fInDeviceSpecific)
{
   XMLDevice *pXMLDevice = XMLDevice::isAXMLDevice (pDevice_d);

   if (!pXMLDevice)
      return new XMLForm_Enumerator (pDevice_d, 0, false);

   XmlDocPtr   docDeviceForms  = pXMLDevice->getDocForms ();
   XmlNodePtr  rootDeviceForms = XMLDocGetRootElement (docDeviceForms);
   XmlNodePtr  elmDeviceForm   = 0;

   if (!rootDeviceForms)
      return new XMLForm_Enumerator (pDevice_d, 0, false);

   elmDeviceForm = XMLFirstNode (rootDeviceForms);
   if (!elmDeviceForm)
      return new XMLForm_Enumerator (pDevice_d, 0, false);

   elmDeviceForm = XMLFirstNode (XMLGetChildrenNode (elmDeviceForm));

   return new XMLForm_Enumerator (pDevice_d, elmDeviceForm, fInDeviceSpecific);
}

#ifndef RETAIL

void XMLDeviceForm::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string XMLDeviceForm::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{XMLDeviceForm: "
       << DeviceForm::toString (oss2)
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const XMLDeviceForm& const_self)
{
   XMLDeviceForm&     self = const_cast<XMLDeviceForm&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
