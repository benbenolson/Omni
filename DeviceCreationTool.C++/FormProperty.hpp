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
#ifndef _com_ibm_ltc_omni_guitool_FormProperty_hpp
#define _com_ibm_ltc_omni_guitool_FormProperty_hpp

#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class FormProperty : public Node
{
   private:
      string & formName;
      string formCapability;
      string command;
      string hardCopyCapLeft;
      string hardCopyCapTop;
      string hardCopyCapRight;
      string hardCopyCapBottom;

   public:
      FormProperty(string newNodeName, Node * pParent );
      FormProperty(FormProperty *);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView *   getGUIPanel();
      virtual void            writeXML(ofstream & outFile, string deviceName);

   // Other methods required
   // getters and setters

      string getFormName()
      { return formName; }

      void setFormName(string newFormName)
      { formName = newFormName; }

      string getFormCapability()
      { return formCapability; }

      void setFormCapability(string newFormCapability)
      { formCapability = newFormCapability; }

      string getCommand()
      { return command; }

      void setCommand(string newCommand)
      { command = newCommand; }

      string getHardCopyCapLeft()
      { return hardCopyCapLeft; }

      void setHardCopyCapLeft(string newHardCopyCapLeft)
      { hardCopyCapLeft = newHardCopyCapLeft; }

      string getHardCopyCapTop()
      { return hardCopyCapTop; }

      void setHardCopyCapTop(string newHardCopyCapTop)
      { hardCopyCapTop = newHardCopyCapTop; }

      string getHardCopyCapRight()
      { return hardCopyCapRight; }

      void setHardCopyCapRight(string newHardCopyCapRight)
      { hardCopyCapRight = newHardCopyCapRight; }

      string getHardCopyCapBottom()
      { return hardCopyCapBottom; }

      void setHardCopyCapBottom(string newHardCopyCapBottom)
      { hardCopyCapBottom = newHardCopyCapBottom; }

}; // end class definition

}; // end of namespace

#endif
