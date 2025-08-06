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
#ifndef _com_ibm_ltc_omni_guitool_CommandProperty_hpp
#define _com_ibm_ltc_omni_guitool_CommandProperty_hpp

#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class CommandProperty : public Node
{
   private:
      string command;
      string & commandName;

   public:
      CommandProperty(string newNodeName, Node * pParentNode);
      CommandProperty(CommandProperty *);
      /*
      * Methods inherited and for which different
      * implementation is required
      */

      virtual GenericView *  getGUIPanel();
      virtual void           writeXML(ofstream & outFile, string deviceName);

      /* Other methods required  */

      // Getters and Setters
      string getCommand()
      { return command; }

      void setCommand(string newCommand)
      { command = newCommand; }

      string getCommandName()
      { return commandName; }

      void setCommandName(string newCommandName)
      { commandName = newCommandName; }

}; // end class definition

}; // end of namespace

#endif
