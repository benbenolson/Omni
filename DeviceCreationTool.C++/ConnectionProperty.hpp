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
#ifndef _com_ibm_ltc_omni_guitool_ConnectionProperty_hpp
#define _com_ibm_ltc_omni_guitool_ConnectionProperty_hpp

#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class ConnectionProperty : public Node
{
   private:
      string & connectionName;
      string connectionForm;
      string connectionTray;
      string connectionMedia;

   public:
      ConnectionProperty(string newNodeName, Node * pParent);
      ConnectionProperty(ConnectionProperty *);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView * getGUIPanel();
      virtual void          writeXML(ofstream & outFile, string deviceName);

   // Other methods required

      // getters and setters

      string getConnectionName()
      { return connectionName; }

      void setConnectionName(string newConnectionName)
      { connectionName = newConnectionName; }

      string getConnectionForm()
      { return connectionForm; }

      void setConnectionForm(string newConnectionForm)
      { connectionForm = newConnectionForm; }

      string getConnectionTray()
      { return connectionTray; }

      void setConnectionTray(string newConnectionTray)
      { connectionTray = newConnectionTray; }

      string getConnectionMedia()
      { return connectionMedia; }

      void setConnectionMedia(string newConnectionMedia)
      { connectionMedia = newConnectionMedia; }

}; // end class definition

}; // end of namespace

#endif
