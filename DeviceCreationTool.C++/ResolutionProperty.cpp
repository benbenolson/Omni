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


#include "ResolutionProperty.hpp"
#include "ResolutionPropertyView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* resolutionName is an alias to nodeName -inherited fromNode */

ResolutionProperty::
ResolutionProperty(string newName, Node * pNewParent)
         : Node(newName), resolutionName(nodeName)
{
   setParentNode(pNewParent);
}

ResolutionProperty::
ResolutionProperty(ResolutionProperty *pR)
         : resolutionName(nodeName)
{
   setParentNode(pR->getParentNode());
   setResolutionName(pR->getResolutionName());
   setXRes(pR->getXRes());
   setYRes(pR->getYRes());
   setXInternalRes(pR->getXInternalRes());
   setYInternalRes(pR->getYInternalRes());
   setCommand(pR->getCommand());
   setResolutionCapability(pR->getResolutionCapability());
   setResolutionDestinationBitsPerPel(pR->getResolutionDestinationBitsPerPel());
   setResolutionScanLineMultiple(pR->getResolutionScanLineMultiple());
}


void ResolutionProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (! outFile.is_open() )
   {
      cerr << "Cannot write to File - Stream not open: Resolution Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceResolution>" << endl ;

   outFile << TAB2 << "<name>" + getResolutionName() + "</name>" << endl ;
   outFile << TAB2 << "<xRes>" + getXRes() + "</xRes>" << endl;
   outFile << TAB2 << "<yRes>" + getYRes() + "</yRes>" << endl;
   outFile << TAB2 << "<xInternalRes>"
	                 + getXInternalRes() + "</xInternalRes>" << endl;
   outFile << TAB2 << "<yInternalRes>"
	                 + getYInternalRes() + "</yInternalRes>" << endl;
   outFile << TAB2 << "<command>" + getCommand() + "</command>" << endl ;
   outFile << TAB2 << "<resolutionCapability>" + getResolutionCapability() ;
         outFile << "</resolutionCapability>" << endl ;
   outFile << TAB2 << "<resolutionDestinationBitsPerPel>"
	                 + getResolutionDestinationBitsPerPel() ;
         outFile << "</resolutionDestinationBitsPerPel>" << endl ;
   outFile << TAB2 << "<resolutionScanlineMultiple>"
	                 + getResolutionScanLineMultiple() ;
         outFile << "</resolutionScanlineMultiple>" << endl ;

   outFile << TAB1 << "</deviceResolution>" << endl ;

   return;
}

inline
GenericView * ResolutionProperty::
getGUIPanel()
{
   return ResolutionPropertyView::getInstance();
}
