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

#include <string>
#include <vector>
#include <gtk--/tree.h>
#include <gtk--/treeitem.h>

#include "CompositeNode.hpp"

using namespace Gtk;
using namespace std;
using namespace OmniDeviceCreationTool;

CompositeNode::
CompositeNode(string name)
	: Node(name)
{
   setSaved(true);
}


void CompositeNode::
createGUITree(Tree *& pGuiTree) // Tree *& - reference to a pointer
{

   if (pGuiTree == NULL)
   {
      // for the dataRoot node only
      pGuiTree = manage (new Gtk::Tree());
      pGuiTree->set_selection_mode(GTK_SELECTION_SINGLE);
      pGuiTree->set_view_mode(GTK_TREE_VIEW_ITEM);
      vector<Node *>::iterator iter;
      for(iter=childVector.begin(); iter != childVector.end(); iter++)
      {
         // down-cast
	 CompositeNode * pCompNode = (CompositeNode *) (*iter);
         pCompNode->createGUITree(pGuiTree);
      }

   }
   else
   {
      // for other nodes
      TreeItem * pTreeItem = manage(new Gtk::TreeItem(this->getNodeName()));

      // associate gui-TreeItem with data Node
      this->setTreeItem(pTreeItem);

      // registering the event handler
      pTreeItem->select.connect((slot((this), &Node::treeSelectionHandler)));

      // attach the treeItem to the tree
      pGuiTree->append(*pTreeItem);

      if (this->childVector.empty() == false)
      {
         Tree * pNewTree = manage(new Gtk::Tree());
         pNewTree->set_selection_mode(GTK_SELECTION_SINGLE);
         pGuiTree->set_view_mode(GTK_TREE_VIEW_ITEM);
         pTreeItem->set_subtree(*pNewTree);
         vector<Node *>::iterator iter2;
         for(iter2=childVector.begin(); iter2 != childVector.end(); iter2++)
         {
            CompositeNode * pCompNode2 = (CompositeNode *)  (*iter2);
            pCompNode2->createGUITree(pNewTree);
         }
      }
   }

   return;
}
