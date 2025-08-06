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
#ifndef _com_ibm_ltc_omni_guitool_HeadNode_hpp
#define _com_ibm_ltc_omni_guitool_HeadNode_hpp

#include <fstream>
#include <string>
#include <vector>
#include <gtk--/tree.h>
#include <gtk--/treeitem.h>
#include <sigc++/object_slot.h>

#include "GenericView.hpp"
#include "Node.hpp"
#include "CompositeNode.hpp"

using namespace Gtk;
using namespace std;

namespace OmniDeviceCreationTool {

class HeadNode : public CompositeNode
{
   private:

      int iToggleFlag;
      /*
      * The above variable is used as a flag to indicate the user's choice
      * 1 == create new
      * 2 == use existing
      * by default the value will be 1
      */

      string selectedFileName;
      // will hold the name of the file if iToggleFlag == 2

      string loadFileName;
      	
      struct XMLValues
      {
         string fileName;
         string docType;
         string startTag;
         string endTag;
      };

      int iChildCount;   // used only in Gammas HeadNode


      XMLValues getXMLValues(string deviceName);
      /*
      * This method returns the certain XML values like
      * XML Header, Start tag, End tag etc which are different
      * for different HeadNodes. The values are determined
      * based of the name of the HeadNode.
      * For Eg. If the name of HeadNode is "Resolutions" then
      * values pertaining to Resolutions are returned
      */

   public:

      HeadNode(string newNodeName, Node * pNewParent);


      /* Methods inherited and for which different implementation is required */

      virtual void            addChild(Node * pNewChild);
      virtual void            deleteChild(Node * pDeletable);
      virtual void            deleteAllChildren();
      virtual GenericView *   getGUIPanel();
      virtual void            writeXML(ofstream & outFile, string deviceName);

      // Other methods required

      Node * getNewChildNode(Node *);
      Node * getNewChildNode();
      /*
         This method creates and returns a new instance of the
	 particular  property object held by the head node.
	 For Eg. If the HeadNode holds ResolutionProperty Objects then,
	 a call to this method returns a ResolutionProperty object.
	
	 This method uses the #defined values from the
         DeviceToolConstants.hpp
      */

      void cloneChild(Node *);
      /*
      * This method clones a given child and adds the new
      * copy to the vector and to the GUI tree
      */

      void addButtonClickHandler();
      /*
         This method gets executed when "Add child" button is clicked.
         The GUI form (HeadNodeView) will invoke this method.
         The actual event handler is present in the GUI form.
      */

      void loadButtonClickHandler(string fileName);
      /*
         This method gets executed when "Load" button is clicked.
         The GUI form (HeadNodeView) will invoke this method.
         The actual event handler is present in the GUI form.
      */


      // gett and set methods

      int getToggleFlag()
      { return iToggleFlag; }

      void setToggleFlag(int iNewVal)
      { iToggleFlag = iNewVal; }

      string getSelectedFileName()
      { return selectedFileName; }

      void setSelectedFileName(string newFileName)
      { selectedFileName = newFileName; }

      string getLoadFileName()
      { return loadFileName; }

      void setLoadFileName(string newFileName)
      { loadFileName = newFileName; }

}; // end of class definition

}; // end of namespace

#endif
