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
#ifndef _com_ibm_ltc_omni_guitool_RootNode_hpp
#define _com_ibm_ltc_omni_guitool_RootNode_hpp

#include <fstream>
#include <string>

#include "Node.hpp"
#include "CompositeNode.hpp"

using namespace Gtk;
using namespace std;

namespace OmniDeviceCreationTool {

class RootNode : public CompositeNode
{

   private:

      string nameToFileName(string deviceName, string nodeName);
      /*
      * This method returns the fileName in the form of a string
      * given the nodeName
      * Eg. Given nodeName as Resolutions, the method returns the
      * fillName as xxx Resolutions.xml
      */

   public:

      RootNode(string nodeName);
      virtual void writeXML(ofstream & outFile, string dirName);
      virtual void addChild(Node * pNewChild);
      virtual void deleteChild(Node * pDeletable){}; // dummy in RootNode class
      virtual void deleteAllChildren() {} ;

}; // end of class definition

}; // end of namespace

#endif
