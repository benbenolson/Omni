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


#include "DefaultJobProperties.hpp"
#include "DefaultJobPropertiesView.hpp"
#include "DeviceToolConstants.hpp"
#include "OmniDCTUtils.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

DefaultJobProperties::
DefaultJobProperties(string newName, Node * pNewParent)
	: Node(newName)
{
   setParentNode(pNewParent);
   setSaved(true);
}


void DefaultJobProperties::
writeXML(ofstream & outFile, string deviceName)
{

   outFile << TAB1 << "<DefaultJobProperties>" << endl ;

   outFile << TAB2 << "<orientation>"
	   + getOrientation() + "</orientation>" << endl ;
   outFile << TAB2 << "<form>" + getForm() + "</form>" << endl ;
   outFile << TAB2 << "<tray>" + getTray() + "</tray>" << endl ;
   outFile << TAB2 << "<media>" + getMedia() + "</media>" << endl ;
   outFile << TAB2 << "<resolution>"
	   + getResolution() + "</resolution>" << endl ;
   outFile << TAB2 << "<dither>" + getDither() + "</dither>" << endl ;
   outFile << TAB2 << "<printmode>" + getPrintMode() + "</printmode>" << endl ;

   vector<string> str;

   str = OmniDCTUtils::str2vec(getOther());

   for (string::size_type i = 0; i < str.size (); i++)
   {
      outFile << TAB2 << "<other>"
	      + OmniDCTUtils::leftTrim(str[i]) + "</other>" << endl;
   }
   str.clear();

   outFile << TAB2 << "</DefaultJobProperties>" << endl ;

   return;

}

inline
GenericView * DefaultJobProperties::
getGUIPanel()
{
   return DefaultJobPropertiesView::getInstance();
}
