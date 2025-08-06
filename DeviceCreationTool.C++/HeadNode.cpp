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

#include "HeadNode.hpp"
#include "DeviceToolConstants.hpp"
#include "OmniDCTUtils.hpp"
#include "Node.hpp"
#include "RootNode.hpp"
#include "CompositeNode.hpp"
#include "HeadNodeView.hpp"
#include "Controller.hpp"
#include "DeviceToolSAX.hpp"

#include "ResolutionProperty.hpp"
#include "CommandProperty.hpp"
#include "PrintModeProperty.hpp"
#include "TrayProperty.hpp"
#include "FormProperty.hpp"
#include "MediaProperty.hpp"
#include "ConnectionProperty.hpp"
#include "GammaTableProperty.hpp"
#include "DataProperty.hpp"
#include "OrientationProperty.hpp"


using namespace OmniDeviceCreationTool;
using namespace Gtk;
using namespace std;


HeadNode::
HeadNode(string newName, Node * pNewParent)
    : CompositeNode(newName)
{
   setParentNode(pNewParent);
   setTreeItem(NULL);
   setToggleFlag(1);
   setSaved(true);
   iChildCount = 0;      // used only in Gammas HeadNode
}


void HeadNode::
writeXML(ofstream & outFile, string deviceName)
{
   if (getToggleFlag() == 2)     // using an existing file
      return;

   if (childVector.empty())
	return;   // no children have been added - nothing to write to output


   XMLValues xmlVal = getXMLValues(deviceName);


   // convert string to char *
   outFile.open(xmlVal.fileName.c_str());

   if (!outFile.is_open())
   {
      cerr << "Error while creating file" << endl;
      return;
   }


   // writing into the file begins here

   outFile << XMLHEADER << endl;
   outFile << xmlVal.docType << endl << endl;

   outFile << COPYRIGHT << endl << endl;

   outFile << xmlVal.startTag << endl;

   vector<Node *>::iterator iter;
   for(iter=childVector.begin(); iter != childVector.end(); iter++)
   {
      (*iter)->writeXML(outFile, deviceName);
   }

   outFile << xmlVal.endTag << endl << endl;

   outFile.close();
   return;
}


inline
GenericView * HeadNode::
getGUIPanel()
{
   return HeadNodeView::getInstance();
}

/*
* The method getNewChildNode checks the
* name of the HeadNode and according to
* the name returns a proper childPropertyNode
*/

Node * HeadNode::
getNewChildNode()
{

   // The constructor for individual properties is of the form
   // xxxProperty(string nodeName, Node * pParentNode)
   // all the  constants have been #defined in DeviceTollConstants.hpp

   Node * pChildNode;
   string name = this->getNodeName();

   if (name == RESOLUTIONS)
   {
      pChildNode = new ResolutionProperty("untitled resolution", this);
   }
   else if (name == COMMANDS)
   {
      pChildNode = new CommandProperty("untitled command", this);
   }
   else if (name == PRINTMODES)
   {
      pChildNode = new PrintModeProperty("untitled print mode", this);
   }
   else if (name == TRAYS)
   {
      pChildNode = new TrayProperty("untitled tray", this);
   }
   else if (name == FORMS)
   {
      pChildNode = new FormProperty("untitled form", this);
   }
   else if (name == MEDIAS)
   {
      pChildNode = new MediaProperty("untitled media", this);
   }
   else if (name == CONNECTIONS)
   {
      pChildNode = new ConnectionProperty("untitled connection", this);
   }
   else if (name == GAMMAS)
   {
      iChildCount ++;
      string gammaName = "Gamma " + OmniDCTUtils::i2str(iChildCount);
      pChildNode = new GammaTableProperty(gammaName, this);
   }
   else if (name == DATA)
   {
      pChildNode = new DataProperty("untitled data", this);
   }
   else if (name == ORIENTATIONS)
   {
      pChildNode = new OrientationProperty("untitled orientation", this);
   }

   return pChildNode;
}

Node * HeadNode::
getNewChildNode(Node *pOldNode)
{
   Node * pChildNode;
   string name = this->getNodeName();

   if (name == RESOLUTIONS)
   {
      pChildNode = new ResolutionProperty((ResolutionProperty *)pOldNode);
   }
   else if (name == COMMANDS)
   {
      pChildNode = new CommandProperty((CommandProperty *)pOldNode);
   }
   else if (name == PRINTMODES)
   {
      pChildNode = new PrintModeProperty((PrintModeProperty *)pOldNode);
   }
   else if (name == TRAYS)
   {
      pChildNode = new TrayProperty((TrayProperty *)pOldNode);
   }
   else if (name == FORMS)
   {
      pChildNode = new FormProperty((FormProperty *)pOldNode);
   }
   else if (name == MEDIAS)
   {
      pChildNode = new MediaProperty((MediaProperty *)pOldNode);
   }
   else if (name == CONNECTIONS)
   {
      pChildNode = new ConnectionProperty((ConnectionProperty *)pOldNode);
   }
   else if (name == GAMMAS)
   {
      /*  uncomment this code to obtain new numbers in the gamma name
      * iChildCount ++;
      * string gammaName = "Gamma " + OmniDCTUtils::i2str(iChildCount);
      */
      pChildNode = new GammaTableProperty((GammaTableProperty *)pOldNode);
   }
   else if (name == DATA)
   {
      pChildNode = new DataProperty((DataProperty *)pOldNode);
   }
   else if (name == ORIENTATIONS)
   {
      pChildNode = new OrientationProperty((OrientationProperty *)pOldNode);
   }

   return pChildNode;
}



void HeadNode::
loadButtonClickHandler(string fileName)
{
   // down-casting
   RootNode *pRoot = (RootNode *) this->getParentNode();
   DeviceToolSAX::getInstance()->loadFile(fileName, pRoot, this->getNodeName());
}


void HeadNode::
addButtonClickHandler()
{
   Node * pChild = getNewChildNode();
   this->addChild(pChild);
   this->getTreeItem()->expand();
   Controller * pCon = Controller::getInstance();
   pCon->getGUITreeRoot()->unselect_child(*(this->getTreeItem()));
   pCon->getGUITreeRoot()->select_child(*(pChild->getTreeItem()));
}


void HeadNode::
addChild(Node * pNewChildNode)
{
   /*
      this method wil be invoked by addButtonClickHandler()
      the input parameter pNewChildNode will have its nodeName
      as "untitled" and pParentNode* pointing to parent
   */

   this->childVector.push_back(pNewChildNode);

   TreeItem * pTreeItem = this->getTreeItem();
   Tree * pTree = pTreeItem->get_subtree();
   if (pTree == NULL)
   {
      /*
         This is true when the first child is added.
         Add a tree to the headNode's tree item.
         The child itself is added as a treeitem to the just added tree
      */
      pTree = manage (new Gtk::Tree());
      pTree->set_selection_mode(GTK_SELECTION_SINGLE);
      pTree->set_view_mode(GTK_TREE_VIEW_ITEM);
      pTreeItem->set_subtree(*(pTree));
   }

   TreeItem * pChildTreeItem;
   pChildTreeItem = manage (new Gtk::TreeItem(pNewChildNode->getNodeName()));

   // asociation between data and ui tree
   pNewChildNode->setTreeItem(pChildTreeItem);

   // registering the event handler
   pChildTreeItem->select.connect((slot((pNewChildNode), &Node::treeSelectionHandler)));

   pTree->append(*(pChildTreeItem));
   pChildTreeItem->show();

}

void HeadNode::
cloneChild(Node * pOldNode)
{
   /*
      this method wil be invoked by GenericPropertyView::onCloneButtonClicked()
   */
   // obtain a new copy of the oldNode
   Node *pNewChildNode = getNewChildNode(pOldNode);

   // obtain position of oldNode in the vector
   vector<Node *>::iterator iter;
   for (iter=childVector.begin(); iter!=childVector.end(); iter++)
   {
      if ((*iter) == pOldNode) break;
   }

   // insert before oldNode
   childVector.insert(iter, pNewChildNode);

   TreeItem * pTreeItem = this->getTreeItem();
   Tree * pTree = pTreeItem->get_subtree();
   if (pTree == NULL)
   {
      /*
         This is true when the first child is added.
         Add a tree to the headNode's tree item.
         The child itself is added as a treeitem to the just added tree
      */
      pTree = manage (new Gtk::Tree());
      pTree->set_selection_mode(GTK_SELECTION_SINGLE);
      pTree->set_view_mode(GTK_TREE_VIEW_ITEM);
      pTreeItem->set_subtree(*(pTree));
   }

   TreeItem * pChildTreeItem;
   pChildTreeItem = manage (new Gtk::TreeItem(pNewChildNode->getNodeName()));

   // asociation between data and ui tree
   pNewChildNode->setTreeItem(pChildTreeItem);

   // registering the event handler
   pChildTreeItem->select.connect((slot((pNewChildNode), &Node::treeSelectionHandler)));

   // obtain position of oldTreeItem
   int iPos = pTree->child_position(*(pOldNode->getTreeItem()));

   // insert before old position
   pTree->insert(*(pChildTreeItem), iPos);
   pChildTreeItem->show();

}


void HeadNode::
deleteAllChildren()
{
   while (!childVector.empty())
   {
      vector<Node *>::iterator iter;
      iter = childVector.begin();
      deleteChild(*iter);
   }
   iChildCount = 0;
}


void HeadNode::
deleteChild(Node * pDeletable)
{

   /*
      This method deletes the specified child
      and it also removes the corresponding
      treeitem form the gui tree
   */

   // remove the treeitem from the tree

   Tree * pTree = this->getTreeItem()->get_subtree();

   pTree->remove_item(*(pDeletable->getTreeItem()));

   // remove the node from the vector

   vector<Node *>::iterator iter;
   for (iter = childVector.begin(); iter != childVector.end(); iter++)
   {
      if (*iter == pDeletable)
      {
         childVector.erase(iter);
         delete pDeletable;
         break;
      }
   }

}



HeadNode::XMLValues HeadNode::
getXMLValues(string deviceName)
{
   XMLValues val;

   string name = this->getNodeName();

   string fileName;
   string tag;
   string docType;

   if (name == RESOLUTIONS)
   {
      fileName = deviceName + RESOLUTIONS_FILENAME;
      tag = "deviceResolutions";
      docType = RESOLUTIONS_DOCTYPE;

   }
   else if (name == COMMANDS)
   {
      fileName = deviceName + COMMANDS_FILENAME ;
      tag = "deviceCommands" ;
      docType = COMMANDS_DOCTYPE ;
   }
   else if (name == PRINTMODES)
   {
      fileName = deviceName + PRINTMODES_FILENAME ;
      tag = "devicePrintModes" ;
      docType = PRINTMODES_DOCTYPE ;
   }
   else if (name == TRAYS)
   {
      fileName = deviceName + TRAYS_FILENAME ;
      tag = "deviceTrays" ;
      docType = TRAYS_DOCTYPE ;
   }
   else if (name == FORMS)
   {
      fileName = deviceName + FORMS_FILENAME ;
      tag = "deviceForms" ;
      docType = FORMS_DOCTYPE ;
   }
   else if (name == MEDIAS)
   {
      fileName = deviceName + MEDIAS_FILENAME ;
      tag = "deviceMedias" ;
      docType = MEDIAS_DOCTYPE ;
   }
   else if (name == CONNECTIONS)
   {
      fileName = deviceName + CONNECTIONS_FILENAME ;
      tag = "deviceConnections" ;
      docType = CONNECTIONS_DOCTYPE ;
   }
   else if (name == GAMMAS)
   {
      fileName = deviceName + GAMMAS_FILENAME ;
      tag = "deviceGammaTables" ;
      docType = GAMMAS_DOCTYPE ;
   }
   else if (name == DATA)
   {
      fileName = deviceName + DATAS_FILENAME ;
      tag = "deviceDatas" ;
      docType = DATAS_DOCTYPE ;
   }

  else if (name == ORIENTATIONS)
   {
      // search for first blank in deviceName to obtain the vendor name
      int firstBlank = deviceName.find_first_of(' ');
      string vendorName = deviceName.substr(0,firstBlank);
      fileName = vendorName + ORIENTATIONS_FILENAME ;
      tag = "deviceOrientations" ;
      docType = ORIENTATIONS_DOCTYPE ;
   }
   val.fileName = fileName ;
   val.startTag = "<" + tag + ">" ;
   val.endTag = "</" + tag + ">" ;
   val.docType = docType ;

   return val;

}
