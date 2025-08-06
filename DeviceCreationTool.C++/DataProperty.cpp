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

#include <iostream>

#include "DataProperty.hpp"
#include "DataPropertyView.hpp"
#include "DeviceToolConstants.hpp"
#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* dataName is an alias to nodeName */

DataProperty::
DataProperty(string newName, Node * pNewParent)
       	: Node(newName), dataName(nodeName)
{
   setParentNode(pNewParent);
}

// copy ctor
DataProperty::
DataProperty(DataProperty * pProp)
          :  dataName(nodeName)
{
   setParentNode(pProp->getParentNode());
   setDeviceData(pProp->getDeviceData());
   setDataName(pProp->getDataName());
   setDataType(pProp->getDataType());
}


void DataProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (! outFile.is_open() )
   {
      cerr << "Cannot write to File - Stream not open: Data Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceData name=\""
	   + getDataName() + "\" type=\"" + getDataType() + "\">" ;
   outFile << getDeviceData() ;
   outFile << "</deviceData>" << endl ;

   return;
}

inline
GenericView * DataProperty::
getGUIPanel()
{
   return DataPropertyView::getInstance();
}
