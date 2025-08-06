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
#ifndef _com_ibm_ltc_omni_guitool_DataProperty_hpp
#define _com_ibm_ltc_omni_guitool_DataProperty_hpp

#include <string>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class DataProperty : public Node
{
   private:
      string deviceData;
      string & dataName;
      string dataType;

   public:
      DataProperty(string newNodeName, Node * pParent);
      DataProperty(DataProperty *);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView *  getGUIPanel();
      virtual void           writeXML(ofstream & outFile, string deviceName);

      // Other methods required
      // getters and setters

      string getDeviceData()
      { return deviceData; }

      void setDeviceData(string newData)
      { deviceData = newData; }

      string getDataName()
      { return dataName; }

      void setDataName(string newDataName)
      { dataName = newDataName; }

      string getDataType()
      { return dataType; }

      void setDataType(string newDataType)
      { dataType = newDataType; }

}; // end class definition

}; // end of namespace

#endif
