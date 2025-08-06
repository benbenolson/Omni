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
#include <fstream>

#include "Controller.hpp"
#include "Node.hpp"
#include "RootNode.hpp"
#include "HeadNode.hpp"
#include "GeneralProperties.hpp"
#include "DefaultJobProperties.hpp"
#include "DeviceToolConstants.hpp"
#include "DeviceToolSAX.hpp"

#include "PopUpDialog.hpp"

using namespace OmniDeviceCreationTool;

// static member initialization
Controller * Controller::pInstance = NULL;


Controller * Controller::
getInstance()
{
   if (Controller::pInstance == NULL)
        Controller::pInstance = new Controller();
   return Controller::pInstance;
}


Controller::
Controller()
{
   pDataTreeRoot = NULL;
   pGUITreeRoot = NULL;
   pSkeleton = NULL;
}

string Controller::
getDeviceName()
{
   vector<Node *>::iterator iter;
   iter = getDataTreeRoot()->getChildVector()->begin();
   // first object in the vector will be General Properties
   GeneralProperties *pGenProp = (GeneralProperties *) (*iter);// down casting
   return pGenProp->getDeviceName();
}

void Controller::
startApplication()
{
   pDataTreeRoot = createDataTree(pDataTreeRoot);
   pDataTreeRoot->createGUITree(pGUITreeRoot);

   pSkeleton = new Skeleton();
   pSkeleton->buildSkeleton();
   pSkeleton->addTree(pGUITreeRoot);

   vector<Node *>::iterator iter;
   iter = pDataTreeRoot->getChildVector()->begin();
   (*iter)->display();
   /*
      The above two lines display the GeneralProperites Panel.
   */

   Gtk::Kit::run();
}

RootNode * Controller::
createDataTree(RootNode * pRoot)
{

   pRoot = new RootNode(DEVICE);

   //the prototype of ctor is GeneralProperties(string nodeName, Node * pParent)
   //the prototype of ctor is HeadNode(string nodeName, Node * pParent)

   pRoot -> addChild(new GeneralProperties(GENERALPROPERTIES, pRoot));
   pRoot -> addChild(new HeadNode(COMMANDS, pRoot));
   pRoot -> addChild(new HeadNode(RESOLUTIONS,pRoot));
   pRoot -> addChild(new HeadNode(PRINTMODES,pRoot));
   pRoot -> addChild(new HeadNode(TRAYS,pRoot));
   pRoot -> addChild(new HeadNode(FORMS,pRoot));
   pRoot -> addChild(new HeadNode(MEDIAS,pRoot));
   pRoot -> addChild(new HeadNode(CONNECTIONS,pRoot));
   pRoot -> addChild(new HeadNode(GAMMAS,pRoot));
   pRoot -> addChild(new HeadNode(DATA,pRoot));
   pRoot -> addChild(new DefaultJobProperties(DEFAULTJP,pRoot));
   pRoot -> addChild(new HeadNode(ORIENTATIONS,pRoot));

   // All the heads are added - no leaf nodes are created.
   // The pRoot pointer is returned back

   return pRoot;
}


void Controller::
replacePanel(GenericView * pNewPanel)
{
   getSkeleton()->removeView();
   getSkeleton()->addView(pNewPanel);
}

void Controller::
writeOutput(string directory)
{
   ofstream temp;
   getDataTreeRoot()->writeXML(temp,directory);
   // temp is a dummy parameter - not used in the invoked function

}

void Controller::
freeDataTree(RootNode * pRootNode)
{
   vector<Node *>::iterator iter;
   for ( iter = pRootNode->getChildVector()->begin();
      iter != pRootNode->getChildVector()->end();
      iter++
        )
   {
      string name = (*iter)->getNodeName() ;
      if ( name == GENERALPROPERTIES || name == DEFAULTJP )
      {
         continue;
      }
      else
      {
         (( HeadNode *) (*iter ))->deleteAllChildren();
      }
   }

   pRootNode->getChildVector()->clear();
   delete pRootNode;
}


void Controller::
openFile(string fileName)
{

   RootNode * pNewDataTreeRoot;
   Tree * pNewGuiTreeRoot;

   pNewDataTreeRoot = NULL;
   pNewGuiTreeRoot = NULL;

   pNewDataTreeRoot = createDataTree(pNewDataTreeRoot);
   pNewDataTreeRoot->createGUITree(pNewGuiTreeRoot);

   int retVal = 0;      // retVal of 0 indicates successful parsing
   DeviceToolSAX * pSax = DeviceToolSAX::getInstance();

   retVal = pSax->parseFile(fileName, pNewDataTreeRoot);

   if (retVal == 0)
   {
      // free existing data tree and gui tree

      freeDataTree(pDataTreeRoot);
      pDataTreeRoot = NULL;
      pSkeleton->removeTree();
      pGUITreeRoot = NULL;

      pDataTreeRoot = pNewDataTreeRoot;
      pGUITreeRoot = pNewGuiTreeRoot ;

      pSkeleton->addTree( pGUITreeRoot );

      // update XML home dir
      string::size_type posLastSlash = fileName.find_last_of('/');
      if(posLastSlash != string::npos)
         setHomeDir(fileName.substr(0,posLastSlash+1));

      vector<Node *>::iterator iter;
      iter = pDataTreeRoot->getChildVector()->begin();
      (*iter)->display();

   }
   else
   {
      cerr << "An error occured during parsing" << endl ;
      freeDataTree (pNewDataTreeRoot);
      pNewGuiTreeRoot = NULL;
   }

}

void Controller::
fileNewClicked()
{
   bool fRetVal = false;
   fRetVal = PopUpDialog::pop_dialog("The present data will be lost !", "Want to continue");
   if (!fRetVal)
      return;

   // reset XML home dir
   string temp;
   setHomeDir(temp);

   // delete existing tree
   freeDataTree(pDataTreeRoot);
   pDataTreeRoot = NULL;

   pSkeleton->removeTree();
   pGUITreeRoot = NULL;

   pDataTreeRoot = createDataTree(pDataTreeRoot);
   pDataTreeRoot->createGUITree(pGUITreeRoot);

   pSkeleton->addTree(pGUITreeRoot);

   vector<Node *>::iterator iter;
   iter = pDataTreeRoot->getChildVector()->begin();
   (*iter)->display();

}
