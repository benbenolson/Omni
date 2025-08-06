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


#include "OrientationProperty.hpp"
#include "OrientationPropertyView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

// The constructor
/*  orientationName is an alias to nodeName -inherited from Node  */

OrientationProperty::
OrientationProperty(string newName, Node * pNewParent)
           : Node(newName), orientationName(nodeName)
{
   setParentNode(pNewParent);
}


OrientationProperty::
OrientationProperty(OrientationProperty * pO)
           : orientationName(nodeName)
{
   setParentNode(pO->getParentNode());
   setOrientationName(pO->getOrientationName());
}


void OrientationProperty::
writeXML(ofstream & outFile,
               string deviceName)
{
   if (!outFile.is_open())
   {
      cerr<<"Cannot write to File - Stream not open:Orientation Property" << endl;
      return;
   }
   outFile << TAB1 << "<deviceOrientation>" << endl;

   outFile << TAB2 << "<name>" + getOrientationName() + "</name>" << endl;

   outFile << TAB1 << "</deviceOrientation>" << endl;

   return;
}

inline GenericView * OrientationProperty::
getGUIPanel()
{
   return  OrientationPropertyView::getInstance();
}
