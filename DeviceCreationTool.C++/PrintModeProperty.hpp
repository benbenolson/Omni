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
#ifndef _com_ibm_ltc_omni_guitool_PrintModeProperty_hpp
#define _com_ibm_ltc_omni_guitool_PrintModeProperty_hpp

#include <string>
#include <fstream>

#include "Node.hpp"
#include "GenericView.hpp"

namespace OmniDeviceCreationTool {

class PrintModeProperty : public Node
{
   private:
      string & printModeName;
      string printModePhysicalCount;
      string printModeLogicalCount;
      string printModePlanes;

   public:
      PrintModeProperty(string newNodeName, Node * pParent);
      PrintModeProperty(PrintModeProperty *);

   /* Methods inherited and for which different implementation is required */

      virtual GenericView *  getGUIPanel();
      virtual void           writeXML(ofstream & outFile, string deviceName);

   // Other methods required
   // getters and setters

      string getPrintModeName()
      { return printModeName; }

      void setPrintModeName(string newPrintModeName)
      { printModeName = newPrintModeName; }

      string getPrintModePhysicalCount()
      { return printModePhysicalCount; }

      void setPrintModePhysicalCount(string newPrintModePhysicalCount)
      { printModePhysicalCount = newPrintModePhysicalCount; }

      string getPrintModeLogicalCount()
      { return printModeLogicalCount; }

      void setPrintModeLogicalCount(string newPrintModeLogicalCount)
      { printModeLogicalCount = newPrintModeLogicalCount; }

      string getPrintModePlanes()
      { return printModePlanes; }

      void setPrintModePlanes(string newPrintModePlanes)
      { printModePlanes = newPrintModePlanes; }

}; // end class definition

}; // end of namespace

#endif
