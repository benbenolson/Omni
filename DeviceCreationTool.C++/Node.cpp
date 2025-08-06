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


#include "Node.hpp"
#include "Controller.hpp"
#include "GenericView.hpp"

using namespace OmniDeviceCreationTool;

Node::
Node()
{
   Node("untitled");
}

Node::
Node(string newNodeName)
{
   nodeName = newNodeName;
   pParentNode = NULL;
   pTreeItem = NULL;
   fSaved = false;
}


void Node::
display()
{
   GenericView * pGuiPanel;
   pGuiPanel = this->getGUIPanel();
   pGuiPanel->populateView(this);

   Controller * pCon;
   pCon = Controller::getInstance();
   pCon->replacePanel(pGuiPanel);

   return;
}


void Node::
treeSelectionHandler()
{
   this->display();
   return;
}


void Node::
updateDisplayName()
{
   getTreeItem()->remove();
   getTreeItem()->add_label(getNodeName());
}
