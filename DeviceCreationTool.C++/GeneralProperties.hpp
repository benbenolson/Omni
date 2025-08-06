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
#ifndef _com_ibm_ltc_omni_guitool_GeneralProperties_hpp
#define _com_ibm_ltc_omni_guitool_GeneralProperties_hpp

#include <fstream>
#include <string>
#include <vector>

#include "Node.hpp"
#include "GenericView.hpp"
#include "OmniDCTUtils.hpp"

namespace OmniDeviceCreationTool {

class GeneralProperties : public Node
{
   private:
      string deviceName;
      string driverName;
      string capabilityType;
      string rasterCapabilityType;
      string deviceOptionsType;
      string deviceOptionsType2;
      string pdlLevel;
      string pdlSubLevel;
      string pdlMajor;
      string pdlMinor;
      string instanceCPP;
      string instanceHPP;
      string blitterCPP;
      string blitterHPP;
      string pluggableInstance;
      string pluggableBlitter;

   /*
      NOTE : capabilityType, rasterCapabilityType,  deviceOptionsType
      and deviceOptionsType2 - These three strings will
      contain a comma separated list of values.
      The individual values are extracted when output XML
      file is being written (in function writeXML)
   */


      int iFirstCall;
      /*
         The variable firstCall is used in writeXML to
         differentiate b/w first call and subsequent call

         instance and blitter are written in the second call,
         all other properties are written in the first call
      */

   public:
      GeneralProperties(string newNodeName, Node * pParent);

      /* Methods inherited and for which different implementation is required */

      virtual GenericView * getGUIPanel();
      virtual void writeXML(ofstream & outFile, string deviceName);

   /* Other methods required */
   /* Getter and setter methods */

      string getDeviceName()
      { return deviceName; }

      void setDeviceName(string newName)
      { deviceName = OmniDCTUtils::leftTrim(newName); }

      string getDriverName()
      { return driverName; }

      void setDriverName(string newDriverName)
      { driverName = newDriverName; }

      string getCapabilityType()
      { return capabilityType; }

      void setCapabilityType(string newCapabilityType)
      { capabilityType = newCapabilityType; }

      string getRasterCapabilityType()
      { return rasterCapabilityType; }

      void setRasterCapabilityType(string newRasterCapabilityType)
      { rasterCapabilityType = newRasterCapabilityType; }

      string getDeviceOptionsType()
      { return deviceOptionsType; }

      void setDeviceOptionsType(string newDeviceOptionsType)
      { deviceOptionsType = newDeviceOptionsType; }

      string getDeviceOptionsType2()
      { return deviceOptionsType2; }

      void setDeviceOptionsType2(string newDeviceOptionsType2)
      { deviceOptionsType2 = newDeviceOptionsType2; }

      string getPDLLevel()
      { return pdlLevel; }

      void setPDLLevel(string newPDLLevel)
      { pdlLevel = newPDLLevel; }

      string getPDLSubLevel()
      { return pdlSubLevel; }

      void setPDLSubLevel(string newPDLSubLevel)
      { pdlSubLevel = newPDLSubLevel; }

      string getPDLMajor()
      { return pdlMajor; }

      void setPDLMajor(string newPDLMajor)
      { pdlMajor = newPDLMajor; }

      string getPDLMinor()
      { return pdlMinor; }

      void setPDLMinor(string newPDLMinor)
      { pdlMinor = newPDLMinor; }

      string getInstanceCPP()
      { return instanceCPP; }

      void setInstanceCPP(string newInstanceCPP)
      { instanceCPP = newInstanceCPP; }

      string getInstanceHPP()
      { return instanceHPP; }

      void setInstanceHPP(string newInstanceHPP)
      { instanceHPP = newInstanceHPP; }

      string getBlitterCPP()
      { return blitterCPP; }

      void setBlitterCPP(string newBlitterCPP)
      { blitterCPP = newBlitterCPP; }

      string getBlitterHPP()
      { return blitterHPP; }

      void setBlitterHPP(string newBlitterHPP)
      { blitterHPP = newBlitterHPP; }

      string getPluggableInstance()
      { return pluggableInstance; }

      void setPluggableInstance(string newPluggableInstance)
      { pluggableInstance = newPluggableInstance; }

      string getPluggableBlitter()
      { return pluggableBlitter; }

      void setPluggableBlitter(string newPluggableBlitter)
      { pluggableBlitter = newPluggableBlitter; }

}; // end class definition

}; // end of namespace

#endif
