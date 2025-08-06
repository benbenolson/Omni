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
#ifndef _com_ibm_ltc_omni_guitool_MediaProperty_hpp
#define _com_ibm_ltc_omni_guitool_MediaProperty_hpp

#include <fstream>
#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class MediaProperty : public Node
{
   private:
      string & mediaName;
      string command;
      string mediaColorAdjustRequired;
      string mediaAbsorption;

   public:
      MediaProperty(string newNodeName, Node * pParent);
      MediaProperty(MediaProperty *);

   /* Methods inherited and for which different implementation is required */

      virtual GenericView *   getGUIPanel();
      virtual void            writeXML(ofstream & outFile, string deviceName);

   // Other methods required
   // getters and setters

      string getMediaName()
      { return mediaName; }

      void setMediaName(string newMediaName)
      { mediaName = newMediaName; }

      string getCommand()
      { return command; }

      void setCommand(string newCommand)
      { command = newCommand; }

      string getMediaColorAdjustRequired()
      { return mediaColorAdjustRequired; }

      void setMediaColorAdjustRequired(string newMediaColorAdjustRequired)
      { mediaColorAdjustRequired = newMediaColorAdjustRequired; }

      string getMediaAbsorption()
      { return mediaAbsorption; }

      void setMediaAbsorption(string newMediaAbsorption)
      { mediaAbsorption = newMediaAbsorption; }

}; // end class definition

}; // end of namespace

#endif
