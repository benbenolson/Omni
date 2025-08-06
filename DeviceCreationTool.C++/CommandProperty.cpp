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

#include "CommandProperty.hpp"
#include "CommandPropertyView.hpp"
#include "DeviceToolConstants.hpp"
#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

// The constructor
/*  commandName is an alias to nodeName -inherited from Node  */

CommandProperty::
CommandProperty(string newName, Node * pNewParent)
         : Node(newName), commandName(nodeName)
{
   setParentNode(pNewParent);
}

// copy constructor
CommandProperty::
CommandProperty(CommandProperty *pCom)
        : commandName(nodeName)
{
   setCommandName(pCom->getCommandName());
   setCommand(pCom->getCommand());
   setParentNode(pCom->getParentNode());
}

void CommandProperty::
writeXML(ofstream & outFile,
               string deviceName)
{
   if (!outFile.is_open())
   {
      cerr<<"Cannot write to File - Stream not open:Command Property" << endl;
      return;
   }
   outFile << TAB1 << "<command name=\"" + getCommandName() + "\">";
   outFile << getCommand() ;
   outFile << "</command>" << endl ;

   return;
}

inline GenericView * CommandProperty::
getGUIPanel()
{
   return  CommandPropertyView::getInstance();
}
