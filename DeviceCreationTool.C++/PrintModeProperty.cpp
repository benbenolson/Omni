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


#include "PrintModeProperty.hpp"
#include "PrintModePropertyView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* printModeName is an alias to nodeName -inherited from Node */
PrintModeProperty::
PrintModeProperty(string newName,  Node * pNewParent)
           : Node(newName), printModeName(nodeName)
{
   setParentNode(pNewParent);
}

PrintModeProperty::
PrintModeProperty(PrintModeProperty *pP)
           : printModeName(nodeName)
{
   setParentNode(pP->getParentNode());
   setPrintModeName(pP->getPrintModeName());
   setPrintModePhysicalCount(pP->getPrintModePhysicalCount());
   setPrintModeLogicalCount(pP->getPrintModeLogicalCount());
   setPrintModePlanes(pP->getPrintModePlanes());
}


void PrintModeProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (!outFile.is_open())
   {
      cerr << "Cannot write to File - Stream not open: PrintMode Property" << endl;
      return;
   }

   outFile << TAB1 << "<devicePrintMode>" << endl ;

   outFile << TAB2 << "<name>" + getPrintModeName() + "</name>" << endl ;
   outFile << TAB2 << "<printModePhysicalCount>" + getPrintModePhysicalCount() ;
         outFile << "</printModePhysicalCount>" << endl ;
   outFile << TAB2 << "<printModeLogicalCount>" + getPrintModeLogicalCount() ;
         outFile << "</printModeLogicalCount>" << endl ;
   outFile << TAB2 << "<printModePlanes>"
	   + getPrintModePlanes() + "</printModePlanes>" << endl ;

   outFile << TAB1 << "</devicePrintMode>" << endl ;

   return;
}

inline
GenericView * PrintModeProperty::
getGUIPanel()
{
   return PrintModePropertyView::getInstance();
}
