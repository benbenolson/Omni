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

#include "ParserCommands.hpp"
#include "CommandProperty.hpp"

using namespace std;
using namespace OmniDeviceCreationTool;

void ParserCommands::
start_element(const string &n, const XMLPropertyHash &p)
{
   XMLPropertyHash propHash = p;
   XMLProperty *prop;

   if (n == "command")
   {
      prop = propHash["name"];
      pPropertyNode = (CommandProperty *) getCommandsHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
      pPropertyNode->setCommandName(prop->value());
   }

}

void ParserCommands::
end_element(const string &n)
{
   if (n == "command" )
   {
      pPropertyNode->setCommand(charsRead);
      getCommandsHead()->addChild(pPropertyNode) ;
   }
   clear_charsRead();

}
