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


#include <iostream.h>
#include <fstream.h>
#include <string>
#include <vector>

#include "RootNode.hpp"
#include "Node.hpp"

#include "Controller.hpp"
#include "DeviceToolConstants.hpp"
#include "GeneralProperties.hpp"
#include "HeadNode.hpp"
#include "DefaultJobProperties.hpp"


using namespace OmniDeviceCreationTool;


RootNode::
RootNode(string newNodeName)
   : CompositeNode(newNodeName)
{
   setParentNode(NULL);
   setSaved(true);
}

void RootNode::
addChild(Node * pNewChild)
{
   childVector.push_back(pNewChild);
   return;
}


void RootNode::
writeXML(ofstream & temp, string directoryName)
{

   string deviceName;

   // obtain device name from GeneralProperties
   vector<Node *>::iterator iter;
   GeneralProperties * pGeneralProp;
   for ( iter=childVector.begin(); iter != childVector.end(); iter++)
   {
      if ( (*iter)->getNodeName() == GENERALPROPERTIES )
      {
         // downcasting Node * to GeneralProperties *
         pGeneralProp = (GeneralProperties *)(*iter);
         deviceName = pGeneralProp->getDeviceName();
         break;
      }
   }

   if (deviceName.empty())
   {
      cerr << "Error: Device Name not specified, cannot save!" << endl;
      return;
   }

   string fileName = directoryName + deviceName + EXT;
   // EXT obtained from DeviceToolConstants.hpp

   ofstream outFile;
   outFile.open(fileName.c_str());

   if (!outFile.is_open())
   {
      cerr << "Error while creating File" << endl;
      return;
   }

   outFile << XMLHEADER << endl;
   outFile << DEVICE_DOCTYPE << endl;
   outFile << COPYRIGHT << endl;


   outFile << "<Device name =\"" + deviceName + "\">" << endl ;

   pGeneralProp->writeXML(outFile,deviceName);

   HeadNode * pHead;

   // uses and has
   for ( iter=childVector.begin(); iter != childVector.end(); iter++)
   {
      string nodeName = (*iter)->getNodeName();
      if (nodeName == GENERALPROPERTIES || nodeName == DEFAULTJP)
         continue;

      pHead = (HeadNode *)  (*iter);  // down casting
      if (pHead->getToggleFlag() == 2)      //use existing
      {
         outFile << TAB1 << "<Has>" + pHead->getSelectedFileName()
		 + "</Has>" << endl;
      }
      else if (pHead->getChildVector()->empty() == false)  // child added
      {
         outFile << TAB1 << "<Has>"
		 + nameToFileName(deviceName, nodeName)
		 + "</Has>" << endl;
      }

   }

   pGeneralProp->writeXML(outFile,deviceName);
   // This call will write Instance and Blitter

   // obtain DefaultJobProperties
   DefaultJobProperties * pDJP;
   for ( iter=childVector.begin(); iter != childVector.end(); iter++)
   {
      string nodeName = (*iter)->getNodeName();
      if ( nodeName == DEFAULTJP )
         pDJP = (DefaultJobProperties *)(*iter) ;   // down casting
   }

   pDJP->writeXML(outFile, deviceName);

   outFile << "</Device>" << endl << endl ;


   outFile.close();

   // other head nodes

   for (iter = childVector.begin(); iter != childVector.end(); iter++)
   {
      string nodeName = (*iter)->getNodeName();
      if (nodeName == GENERALPROPERTIES || nodeName == DEFAULTJP)
         continue;
      (*iter)->writeXML(outFile, directoryName + deviceName);
   }

   return;
}


string RootNode::nameToFileName(string deviceName, string nodeName)
{
   if (nodeName == RESOLUTIONS)
   {
      return deviceName + RESOLUTIONS_FILENAME;
   }
   else if (nodeName == COMMANDS)
   {
      return deviceName + COMMANDS_FILENAME ;
   }
   else if (nodeName == PRINTMODES)
   {
      return  deviceName + PRINTMODES_FILENAME ;
   }
   else if (nodeName == TRAYS)
   {
      return deviceName + TRAYS_FILENAME ;
   }
   else if (nodeName == FORMS)
   {
      return deviceName + FORMS_FILENAME ;
   }
   else if (nodeName == MEDIAS)
   {
      return deviceName + MEDIAS_FILENAME ;
   }
   else if (nodeName == CONNECTIONS)
   {
      return deviceName + CONNECTIONS_FILENAME ;
   }
   else if (nodeName == GAMMAS)
   {
      return deviceName + GAMMAS_FILENAME ;
   }
   else if (nodeName == DATA)
   {
      return deviceName + DATAS_FILENAME ;
   }
   else if (nodeName == ORIENTATIONS)
   {
       // obtain vendor Name;
      int firstBlank = deviceName.find_first_of(' ');
      // search for the first blank character
      string vendorName = deviceName.substr(0,firstBlank);
      return vendorName + ORIENTATIONS_FILENAME ;
   }
   else
   {
      cerr << "Unknown Attribute " << nodeName << endl;
   }
}
