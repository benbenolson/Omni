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

#include "ConnectionProperty.hpp"
#include "ConnectionPropertyView.hpp"
#include "DeviceToolConstants.hpp"
#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* connectionName is an alias to nodeName - inherited from Node */
ConnectionProperty::
ConnectionProperty(string newName, Node * pNewParent)
               : Node(newName), connectionName(nodeName)
{
   setParentNode(pNewParent);
}

// copy constructor
ConnectionProperty::
ConnectionProperty(ConnectionProperty *pCon)
           : connectionName(nodeName)
{
   setParentNode(pCon->getParentNode());
   setConnectionName(pCon->getConnectionName());
   setConnectionForm(pCon->getConnectionForm());
   setConnectionTray(pCon->getConnectionTray());
   setConnectionMedia(pCon->getConnectionMedia());
}


void ConnectionProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (!outFile.is_open())
   {
      cerr << "Cannot write to File - Stream not open: Connection Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceConnection>" << endl ;
   outFile<< TAB2 << "<name>" + getConnectionName() + "</name>" << endl ;
   outFile<< TAB2 << "<connectionForm>"
	   + getConnectionForm() + "</connectionForm>" << endl ;
   outFile<< TAB2 << "<connectionTray>"
	   + getConnectionTray() + "</connectionTray>" << endl ;
   outFile<< TAB2 << "<connectionMedia>"
	   + getConnectionMedia() + "</connectionMedia>" << endl ;
   outFile << TAB1 << "</deviceConnection>" << endl ;

   return;
}

inline
GenericView * ConnectionProperty::
getGUIPanel()
{
   return ConnectionPropertyView::getInstance();
}
