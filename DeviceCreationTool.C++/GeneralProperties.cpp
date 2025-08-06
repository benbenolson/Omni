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


#include "GeneralProperties.hpp"
#include "GeneralPropertiesView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"
#include "OmniDCTUtils.hpp"

using namespace OmniDeviceCreationTool;

GeneralProperties::
GeneralProperties(string newName, Node * pNewParent)
	: Node(newName)
{
   setParentNode(pNewParent);
   iFirstCall=0;
   setSaved(true);
}


void GeneralProperties::
writeXML(ofstream & outFile, string deviceName)
{
   // Device name will be handled by Node

   if (iFirstCall == 0)
   {
      iFirstCall = 1;

      outFile<< TAB1 << "<DriverName>"
	      + getDriverName() + "</DriverName>" << endl;

      vector<string> str;

      str = OmniDCTUtils::str2vec(getCapabilityType());
      for(string::size_type i=0; i< str.size(); i++)
      {
         outFile << TAB1 << "<Capability type=\""
		 + OmniDCTUtils::leftTrim(str[i]) + "\" />" << endl;
      }
      str.clear();

      str = OmniDCTUtils::str2vec(getRasterCapabilityType());
      for(string::size_type i=0; i< str.size(); i++)
      {
         outFile << TAB1 << "<RasterCapabilities type=\""
		 + OmniDCTUtils::leftTrim(str[i]) + "\" />" << endl;
      }
      str.clear();

      str = OmniDCTUtils::str2vec(getDeviceOptionsType());
      for(string::size_type i=0; i< str.size(); i++)
      {
         outFile << TAB1 << "<DeviceOptions type=\""
		 + OmniDCTUtils::leftTrim(str[i]) + "\" />" << endl;
      }
      str.clear();

      str = OmniDCTUtils::str2vec(getDeviceOptionsType2());
      for(string::size_type i=0; i< str.size(); i++)
      {
         outFile << TAB1 << "<DeviceOptions2 type=\""
		 + OmniDCTUtils::leftTrim(str[i]) + "\" />" << endl;
      }
      str.clear();

      outFile << TAB1 << "<PDL level=\"" + getPDLLevel() + "\"" +
                        " sublevel=\"" + getPDLSubLevel() + "\"" +
                        " major=\"" + getPDLMajor() + "\"" +
                        " minor=\"" + getPDLMinor() + "\" />"  << endl;
   }
   else if (iFirstCall == 1)
   {
      iFirstCall = 0;

      string pluggableInst = OmniDCTUtils::leftTrim(getPluggableInstance());
      if (pluggableInst.empty())
      {
         outFile<< TAB1 << "<Instance>"
		 + getInstanceHPP() + "</Instance>" << endl;
         outFile<< TAB1 << "<Instance>"
		 + getInstanceCPP() + "</Instance>" << endl;
         outFile<< TAB1 << "<Blitter>"
		 + getBlitterHPP() + "</Blitter>" << endl;
         outFile<< TAB1 << "<Blitter>"
		 + getBlitterCPP() + "</Blitter>" << endl;
      }
      else
      {
         outFile << TAB1 << "<PluggableInstance exename=\""
		 + getPluggableInstance() + "\" />" << endl;

         outFile << TAB1 << "<PluggableBlitter/>" << endl;
         // The above line will have to be changed to
         // accomodate the attribute of PluggableBlitter

      }
   }
   return;
}

inline
GenericView * GeneralProperties::
getGUIPanel()
{
   return GeneralPropertiesView::getInstance();
}
