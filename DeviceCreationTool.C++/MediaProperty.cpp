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


#include "MediaProperty.hpp"
#include "MediaPropertyView.hpp"
#include "DeviceToolConstants.hpp"

#include "Node.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

/* mediaName is an alias to nodename -inherited from Node */

MediaProperty::
MediaProperty(string newName, Node * pNewParent)
        : Node(newName), mediaName(nodeName)
{
   setParentNode(pNewParent);
}

MediaProperty::
MediaProperty(MediaProperty *pM)
        : mediaName(nodeName)
{
   setParentNode(pM->getParentNode());
   setMediaName(pM->getMediaName());
   setCommand(pM->getCommand());
   setMediaColorAdjustRequired(pM->getMediaColorAdjustRequired());
   setMediaAbsorption(pM->getMediaAbsorption());
}


void MediaProperty::
writeXML(ofstream & outFile, string deviceName)
{
   if (! outFile.is_open() )
   {
      cerr << "Cannot write to File - Stream not open: Media Property" << endl;
      return;
   }

   outFile << TAB1 << "<deviceMedia>" << endl ;

   outFile << TAB2 << "<name>" + getMediaName() + "</name>" << endl ;
   outFile <<TAB2 << "<command>" + getCommand() + "</command>" << endl ;
   outFile << TAB2 << "<mediaColorAdjustRequired>"
	   + getMediaColorAdjustRequired() ;
         outFile << "</mediaColorAdjustRequired>" << endl ;
   outFile << TAB2 << "<mediaAbsorption>"
	   + getMediaAbsorption() + "</mediaAbsorption>" << endl ;

   outFile << TAB1 << "</deviceMedia>" << endl ;


   return;
}

inline
GenericView * MediaProperty::
getGUIPanel()
{
   return MediaPropertyView::getInstance();
}
