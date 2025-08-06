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
#ifndef _com_ibm_ltc_omni_guitool_Node_hpp
#define _com_ibm_ltc_omni_guitool_Node_hpp

#include <fstream>
#include <string>
#include <gtk--/tree.h>
#include <gtk--/treeitem.h>
#include <sigc++/object_slot.h>

#include "GenericView.hpp"

using namespace Gtk;
using namespace std;
using namespace SigC;

namespace OmniDeviceCreationTool {

class Node : public SigC::Object
{

   protected:
      string nodeName;  // holds the node name
      Node * pParentNode;   // pointer to the parent Node
      TreeItem * pTreeItem;
      // pointer to the Gtk::TreeItem that represents this node in the tree

      bool fSaved;
      // fSaved=true when data in the node is saved, false otherwise

   public:
      Node();
      Node(string newNodeName);

      void display();
      /*
         Each node knows how to display the information in it.
         The node also knows the GUI form (or Panel) to be used
         to display its information.
         The particular GUI form is obtained by calling the
         method   getGUIPanel().
         The display function is implemented in Node class and
         the same implementaion is inherited by all the
         derived classes.
      */

      virtual GenericView* getGUIPanel()
      {GenericView *temp; return temp;}  // dummy in Node class
      /*
         This method will get the particular GUI form (or Panel)
         that will be used to dispay the contents of the node.
         The contents of the node are also "added/edited/deleted"
         from this form/panel.
         This method will be invoked by the display() method.

         This method will be overridden by derived classes and
         will return the form/panel relevant to them.
      */


      void treeSelectionHandler();
      /*
         This method serves as event handler for
         treeSelectionEvents.
         The event will be thrown by the
         Gtk::TreeItem (select event)
         when it is selected using a mouse.
         The registration of event handler will happen in
	 CompositeNode::createGUITree() and HeadNode::addChild() methods.

         The method is implemented in Node class and the same
         implementaion is inherited by all the derived classes.
      */

      virtual void writeXML(ofstream & outFile, string dirName){};
      /*
      *   This method will be invoked to write the output XML file
      */

      void updateDisplayName();
      /*
      This method will update the name displayed in the GUI tree.
      */

      /* get and set methods */

      string getNodeName()
      { return nodeName; }

      void setNodeName(string newNodeName)
      { nodeName = newNodeName; }

      Node * getParentNode()
      { return pParentNode; }

      void setParentNode(Node * pNewParentNode)
      { pParentNode = pNewParentNode; }

      TreeItem * getTreeItem()
      { return pTreeItem; }

      void setTreeItem(TreeItem * pNewTreeItem)
      { pTreeItem = pNewTreeItem; }

      bool isSaved()
      { return fSaved; }

      void setSaved(bool fNewVal)
      { fSaved = fNewVal; }


}; // end of class definition

}; // end of namespace

#endif
