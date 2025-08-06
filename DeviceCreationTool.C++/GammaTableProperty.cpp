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

#include <iostream>

#include "GammaTableProperty.hpp"
#include "GammaTablePropertyView.hpp"
#include "DeviceToolConstants.hpp"
#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;


GammaTableProperty::
GammaTableProperty(string newName, Node * pNewParent)
            : Node(newName)
{
   setParentNode(pNewParent);
}

GammaTableProperty::
GammaTableProperty(GammaTableProperty *pG)
{
   setParentNode(pG->getParentNode());
   setNodeName(pG->getNodeName());
   setGammaTableResolution(pG->getGammaTableResolution());
   setGammaTableMedia(pG->getGammaTableMedia());
   setGammaTablePrintMode(pG->getGammaTablePrintMode());
   setGammaTableDitherCatagory(pG->getGammaTableDitherCatagory());
   setGammaTableCGamma(pG->getGammaTableCGamma());
   setGammaTableMGamma(pG->getGammaTableMGamma());
   setGammaTableYGamma(pG->getGammaTableYGamma());
   setGammaTableKGamma(pG->getGammaTableKGamma());
   setGammaTableCBias(pG->getGammaTableCBias());
   setGammaTableMBias(pG->getGammaTableMBias());
   setGammaTableYBias(pG->getGammaTableYBias());
   setGammaTableKBias(pG->getGammaTableKBias());
}


void GammaTableProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (!outFile.is_open())
   {
      cerr << "Cannot write to File - Stream not open: GammaTable Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceGammaTable>" << endl ;

   outFile << TAB2 << "<gammaTableResolution>" + getGammaTableResolution() ;
         outFile << "</gammaTableResolution>" << endl ;
   outFile << TAB2 << "<gammaTableMedia>" + getGammaTableMedia() ;
         outFile << "</gammaTableMedia>" << endl ;
   outFile << TAB2 << "<gammaTablePrintMode>" + getGammaTablePrintMode() ;
         outFile << "</gammaTablePrintMode>" << endl ;
   outFile << TAB2 << "<gammaTableDitherCatagory>"
	   + getGammaTableDitherCatagory() ;
         outFile << "</gammaTableDitherCatagory>" << endl ;

   outFile << TAB2 << "<gammaTableCGamma>" + getGammaTableCGamma() ;
         outFile << "</gammaTableCGamma>" << endl ;
   outFile << TAB2 << "<gammaTableMGamma>" + getGammaTableMGamma() ;
         outFile << "</gammaTableMGamma>" << endl ;
   outFile <<TAB2 << "<gammaTableYGamma>" + getGammaTableYGamma() ;
         outFile << "</gammaTableYGamma>" << endl ;
   outFile << TAB2 << "<gammaTableKGamma>" + getGammaTableKGamma() ;
         outFile << "</gammaTableKGamma>" << endl ;

   outFile << TAB2 << "<gammaTableCBias>" + getGammaTableCBias() ;
         outFile << "</gammaTableCBias>" << endl ;
   outFile << TAB2 << "<gammaTableMBias>" + getGammaTableMBias() ;
         outFile << "</gammaTableMBias>" << endl ;
   outFile << TAB2 << "<gammaTableYBias>" + getGammaTableYBias() ;
         outFile << "</gammaTableYBias>" << endl ;
   outFile << TAB2 << "<gammaTableKBias>" + getGammaTableKBias() ;
         outFile << "</gammaTableKBias>" << endl ;

   outFile << TAB1 << "</deviceGammaTable>" << endl ;

   return;
}

inline
GenericView * GammaTableProperty::
getGUIPanel()
{
   return GammaTablePropertyView::getInstance();
}
