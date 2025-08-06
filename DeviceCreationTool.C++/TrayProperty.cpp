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


#include "TrayProperty.hpp"
#include "TrayPropertyView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* trayName is an alias to nodeName -inherited from Node */

TrayProperty::
TrayProperty(string newName, Node * pNewParent)
       : Node(newName), trayName(nodeName)
{
   setParentNode(pNewParent);
}


TrayProperty::
TrayProperty(TrayProperty * pT)
       : trayName(nodeName)
{
   setParentNode(pT->getParentNode());
   setTrayName(pT->getTrayName());
   setTrayType(pT->getTrayType());
   setCommand(pT->getCommand());
}


void TrayProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (! outFile.is_open() )
   {
      cerr << "Cannot write to File - Stream not open: Tray Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceTray>" << endl ;

   outFile<< TAB2 << "<name>" + getTrayName() + "</name>" << endl ;
   outFile<< TAB2 << "<trayType>" + getTrayType() + "</trayType>" << endl ;
   outFile<< TAB2 << "<command>" + getCommand() + "</command>" << endl ;

   outFile << TAB1 << "</deviceTray>" << endl ;

   return;
}

inline
GenericView * TrayProperty::
getGUIPanel()
{
   return TrayPropertyView::getInstance();
}
