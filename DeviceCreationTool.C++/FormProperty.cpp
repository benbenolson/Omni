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

#include "FormProperty.hpp"
#include "FormPropertyView.hpp"
#include "DeviceToolConstants.hpp"
#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* formName is an alias to nodename */
FormProperty::
FormProperty(string newName, Node * pNewParent)
          : Node(newName), formName(nodeName)
{
   setParentNode(pNewParent);
}

// copy ctor
FormProperty::
FormProperty(FormProperty *pFP)
          : formName(nodeName)
{
   setParentNode(pFP->getParentNode());
   setFormName(pFP->getFormName());
   setFormCapability(pFP->getFormCapability());
   setCommand(pFP->getCommand());
   setHardCopyCapLeft(pFP->getHardCopyCapLeft());
   setHardCopyCapTop(pFP->getHardCopyCapTop());
   setHardCopyCapRight(pFP->getHardCopyCapRight());
   setHardCopyCapBottom(pFP->getHardCopyCapBottom());
}


void FormProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (! outFile.is_open() )
   {
      cerr << "Cannot write to File - Stream not open: Form Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceForm>" << endl ;

   outFile << TAB2 << "<name>" + getFormName() + "</name>" << endl ;
   outFile << TAB2 << "<formCapabilities>"
	   + getFormCapability() + "</formCapabilities>" << endl ;
   outFile << TAB2 << "<command>" + getCommand() + "</command>" << endl ;

   outFile << TAB2 << "<hardCopyCap>"  << endl ;

   outFile << TAB3 << "<hardCopyCapLeft>"
	   + getHardCopyCapLeft() + "</hardCopyCapLeft>"  << endl ;
   outFile << TAB3 << "<hardCopyCapTop>"
	   + getHardCopyCapTop() + "</hardCopyCapTop>"  << endl ;
   outFile << TAB3 << "<hardCopyCapRight>"
	   + getHardCopyCapRight() + "</hardCopyCapRight>"  << endl ;
   outFile << TAB3 << "<hardCopyCapBottom>"
	   + getHardCopyCapBottom() + "</hardCopyCapBottom>"  << endl ;

   outFile << TAB2 << "</hardCopyCap>"  << endl ;

   outFile << TAB1 << "</deviceForm>" << endl ;

   return;
}

inline
GenericView * FormProperty::
getGUIPanel()
{
   return FormPropertyView::getInstance();
}
