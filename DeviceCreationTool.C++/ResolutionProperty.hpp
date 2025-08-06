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
#ifndef _com_ibm_ltc_omni_guitool_ResolutionProperty_hpp
#define _com_ibm_ltc_omni_guitool_ResolutionProperty_hpp

#include <fstream>
#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class ResolutionProperty : public Node
{
   private:
      string & resolutionName;
      string xRes;
      string yRes;
      string command;
      string resolutionCapability;
      string resolutionDestinationBitsPerPel;
      string resolutionScanLineMultiple;
      // new
      string xInternalRes;
      string yInternalRes;

   public:
      ResolutionProperty(string newNodeName, Node * parent);
      ResolutionProperty(ResolutionProperty *);

   /* Methods inherited and for which different implementation is required */

      virtual GenericView *   getGUIPanel();
      virtual void            writeXML(ofstream & outFile, string deviceName);

   // Other methods required
   // getters and setters

      string getResolutionName()
      { return resolutionName; }

      void setResolutionName(string newResolutionName)
      { resolutionName = newResolutionName; }

      string getXRes()
      { return xRes; }

      void setXRes(string newXRes)
      { xRes = newXRes; }

      string getYRes()
      { return yRes; }

      void setYRes(string newYRes)
      { yRes = newYRes; }

      string getCommand()
      { return command; }

      void setCommand(string newCommand)
      { command = newCommand; }

      string getResolutionCapability()
      { return resolutionCapability; }

      void setResolutionCapability(string newResolutionCapability)
      { resolutionCapability = newResolutionCapability; }

      string getResolutionDestinationBitsPerPel()
      { return resolutionDestinationBitsPerPel; }

      void setResolutionDestinationBitsPerPel(string newResolutionDestinationBitsPerPel)
      { resolutionDestinationBitsPerPel = newResolutionDestinationBitsPerPel; }

      string getResolutionScanLineMultiple()
      { return resolutionScanLineMultiple; }

      void setResolutionScanLineMultiple(string newResolutionScanLineMultiple)
      { resolutionScanLineMultiple = newResolutionScanLineMultiple; }

      string getXInternalRes()
      { return xInternalRes; }

      void setXInternalRes(string newXInternalRes)
      { xInternalRes = newXInternalRes; }

      string getYInternalRes()
      { return yInternalRes; }

      void setYInternalRes(string newYInternalRes)
      { yInternalRes = newYInternalRes; }

}; // end class definition

}; // end of namespace

#endif
