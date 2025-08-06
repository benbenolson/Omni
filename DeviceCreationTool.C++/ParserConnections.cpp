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

#include "ParserConnections.hpp"
#include "ConnectionProperty.hpp"

using namespace std;
using namespace OmniDeviceCreationTool;

void ParserConnections::
start_element(const string &n, const XMLPropertyHash &p)
{
   if (n == "deviceConnection")
   {
      pPropertyNode = (ConnectionProperty *) getConnectionsHead()->getNewChildNode();
      pPropertyNode->setSaved(true);
   }
}
// end of start_element

void ParserConnections::
end_element(const string &n)
{
   if (n == "deviceConnection")
   {
      getConnectionsHead()->addChild(pPropertyNode);
   }
   else if (n == "name" )
   {
       pPropertyNode->setConnectionName(charsRead);
   }
   else if (n == "connectionForm" )
   {
      pPropertyNode->setConnectionForm(charsRead);
   }
   else if (n == "connectionTray" )
   {
      pPropertyNode->setConnectionTray(charsRead);
   }
   else if (n == "connectionMedia" )
   {
      pPropertyNode->setConnectionMedia(charsRead);
   }

   clear_charsRead();

}
