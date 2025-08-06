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
#ifndef _com_ibm_ltc_omni_guitool_CompositeNode_hpp
#define _com_ibm_ltc_omni_guitool_CompositeNode_hpp

#include <string>
#include <vector>
#include <gtk--/tree.h>

#include "Node.hpp"

using namespace Gtk;
using namespace std;

namespace OmniDeviceCreationTool {

class CompositeNode : public Node
{
   protected:
      vector<Node *> childVector; // vector to hold all the children

   public:

      CompositeNode(string nodeName);

      void createGUITree(Tree *& guiTree);
      // Tree *& - reference of a pointer

      virtual void addChild(Node * newChild){};

      /*
         This method is used to add a child to a node.
         A child is generally a particular property feature
         such as Resolution of 640x480 which gets added to
         its parent (the parent is usually a HeadNode).

         This method adds the child to the childVector
         and also adds it to the
         correspnding GUI tree.
         The event handlers for treeSelection are also registered
         in this method.
         This function will be overridden by derived classes.
      */


      virtual void deleteChild(Node * deletable){};
      /*
         This method will delete the specified child from the
         childVector.
         It also deletes the corresponding Gtk::TreeItem from
         the GUI Tree.

         This method will be overridden in the HeadNode.
      */

      virtual void deleteAllChildren(){};
      /*
      *  This method deletes all the children
      */

      virtual void writeXML(ofstream & outFile, string dirName){};
      /*
      * This method writes the output XML files
      */

      virtual GenericView* getGUIPanel()
      { GenericView *temp; return temp; } // dummy in this class

      vector<Node *> * getChildVector() // Returns a pointer to the vector
      { return &childVector; }

}; // end of class definition

}; // end of namespace

#endif
