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


#include <iostream.h>
#include <string>
#include <xml++.h>

#include "ParserDevice.hpp"
#include "HeadNode.hpp"
#include "DeviceToolConstants.hpp"
#include "DeviceToolSAX.hpp"

using namespace OmniDeviceCreationTool;


void ParserDevice::
start_element(const string &n, const XMLPropertyHash &p)
{
   XMLPropertyHash propHash = p;
   XMLProperty *prop;

   if (n == "Device")
   {
      prop = propHash["name"];
      attribDeviceName = prop->value() ;
      getGeneralProperties()->setDeviceName(prop->value());
   }
   else if (n == "Capability")
   {
      prop = propHash["type"];

      string val = getGeneralProperties()->getCapabilityType();
      if (val.empty())
      {
            getGeneralProperties()->setCapabilityType(prop->value());
      }
      else
      {
            val = val + "," + prop->value();
            getGeneralProperties()->setCapabilityType(val);
      }


   }
   else if (n == "RasterCapabilities")
   {
      prop = propHash["type"];

      string val = getGeneralProperties()->getRasterCapabilityType();
      if (val.empty())
      {
            getGeneralProperties()->setRasterCapabilityType(prop->value());
      }
      else
      {
            val = val + "," + prop->value();
            getGeneralProperties()->setRasterCapabilityType(val);
      }

   }
   else if (n == "DeviceOptions")
   {
      prop = propHash["type"];

      string val = getGeneralProperties()->getDeviceOptionsType();
      if (val.empty())
      {
            getGeneralProperties()->setDeviceOptionsType(prop->value());
      }
      else
      {
            val = val + "," + prop->value();
            getGeneralProperties()->setDeviceOptionsType(val);
      }

   }
   // new
   else if (n == "DeviceOptions2")
   {
      prop = propHash["type"];

      string val = getGeneralProperties()->getDeviceOptionsType2();
      if (val.empty())
      {
            getGeneralProperties()->setDeviceOptionsType2(prop->value());
      }
      else
      {
            val = val + "," + prop->value();
            getGeneralProperties()->setDeviceOptionsType2(val);
      }

   }
   else if (n == "PDL")
   {
      prop = propHash["level"];
      getGeneralProperties()->setPDLLevel(prop->value());

      prop = propHash["sublevel"];
      getGeneralProperties()->setPDLSubLevel(prop->value());

      prop = propHash["major"];
      getGeneralProperties()->setPDLMajor(prop->value());

      prop = propHash["minor"];
      getGeneralProperties()->setPDLMinor(prop->value());
   }

   // new
   else if (n == "PluggableInstance")
   {
      prop = propHash["exename"];
      getGeneralProperties()->setPluggableInstance(prop->value());
   }
   else if (n == "PluggableBlitter")
   {
      cerr << "Attribute name not known-PluggableBlitter" << endl;
      /*
      * the following code needs to be changed according to the
      * arrtibute name of PluggableBlitter
      */

      /*
      *prop = propHash["exename"];
      *getGeneralProperties()->setPluggableBlitter(prop->value());
      */
   }

}
// end of start_element



void ParserDevice::
end_element(const string &n)
{
   if (n == "DriverName")
   {
      getGeneralProperties()->setDriverName( charsRead ) ;
   }
   else if (n == "Uses")
   {
      handleUsesHas( charsRead );
   }
   else if (n == "Has")
   {
      handleUsesHas( charsRead );
   }
   else if (n == "Instance")
   {
      int val = charsRead.find(".cpp");
      if (val == string::npos)       // search unsuccessful
      {
            getGeneralProperties()->setInstanceHPP(charsRead);
      }
      else
      {
            getGeneralProperties()->setInstanceCPP(charsRead);
      }
   }
   else if (n =="Blitter")
   {
      int val = charsRead.find(".cpp");
      if (val == string::npos)       // search unsuccessful
      {
         getGeneralProperties()->setBlitterHPP(charsRead);
      }
      else
      {
         getGeneralProperties()->setBlitterCPP(charsRead);
      }
   }

   // default job properties

   else if (n == "orientation")
   {
      getDefaultJobProperties()->setOrientation(charsRead);
   }
   else if (n == "form")
   {
      getDefaultJobProperties()->setForm(charsRead);
   }
   else if (n == "tray")
   {
      getDefaultJobProperties()->setTray(charsRead);
   }
   else if (n == "media")
   {
      getDefaultJobProperties()->setMedia(charsRead);
   }
   else if (n == "resolution")
   {
      getDefaultJobProperties()->setResolution( charsRead ) ;
   }
   else if (n == "dither")
   {
      getDefaultJobProperties()->setDither( charsRead ) ;
   }
   else if (n == "printmode")
   {
      getDefaultJobProperties()->setPrintMode( charsRead ) ;
   }
// new
    else if (n == "other")
   {
      string val = getDefaultJobProperties()->getOther();
      if (val.empty())
      {
            getDefaultJobProperties()->setOther(charsRead);
      }
      else
      {
            val = val + "," + charsRead;
            getDefaultJobProperties()->setOther(val);
      }
  }

   clear_charsRead();
}
 // end of end_element


void ParserDevice::
handleUsesHas(string charsRead)
{
   int lastBlank = charsRead.find_last_of(' '); // search for last blank space - the blank just before aaa.xml
   if (lastBlank==string::npos)
      cerr << "Invalid file name -" << charsRead << endl;
   string val = charsRead.substr(0,lastBlank);
   bool fParsingRequired = false;

   if (val == attribDeviceName)
   {
         fParsingRequired = true;
   }
   else
   {
         fParsingRequired = false;
   }

   // obtain vendorName
   int firstBlank = attribDeviceName.find_first_of(' ');
   string vendorName = attribDeviceName.substr(0,firstBlank);

   if (val == vendorName)
   {
       // This will be true for Orientations
       fParsingRequired = true;
   }

   int lastDot = charsRead.find_last_of('.');   // search for last . (dot) - the dot before .xml
   string property = charsRead.substr(lastBlank+1, lastDot-(lastBlank+1));

   HeadNode * pHeadNode;

   if (property == RESOLUTIONS)
      pHeadNode = getResolutionsHead();
   else if (property == COMMANDS)
      pHeadNode = getCommandsHead();
   else if (property == PRINTMODES)
      pHeadNode = getPrintModesHead();
   else if (property == TRAYS)
      pHeadNode = getTraysHead();
   else if (property == FORMS)
      pHeadNode = getFormsHead();
   else if (property == MEDIAS)
      pHeadNode = getMediasHead();
   else if (property == CONNECTIONS)
      pHeadNode = getConnectionsHead();
   else if (property == GAMMAS)
      pHeadNode = getGammasHead();
   else if (property == DATA)
      pHeadNode = getDatasHead();
   else if (property == ORIENTATIONS)
      pHeadNode = getOrientationsHead();
   else
   {
      cerr << "Invalid file name -" << charsRead << endl;
      return;
   }


   if (! fParsingRequired)
   {
         pHeadNode->setToggleFlag(2);    // use existing file
         pHeadNode->setSelectedFileName(charsRead);
   }
   else
   {
      // store the file name for further processing
         DeviceToolSAX::getInstance()->addFile(charsRead);
         pHeadNode->setToggleFlag(1);
   }
}
