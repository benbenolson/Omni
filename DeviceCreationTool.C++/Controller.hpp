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
#ifndef _com_ibm_ltc_omni_guitool_Controller_hpp
#define _com_ibm_ltc_omni_guitool_Controller_hpp

#include <string>
#include <gtk--/tree.h>

#include "RootNode.hpp"
#include "Skeleton.hpp"

using namespace Gtk;
using namespace std;

namespace OmniDeviceCreationTool {

class Controller
{

   private:
      static Controller * pInstance;
      // variable to hold the single instance of the Controller

      Skeleton * pSkeleton;
      //varriable to hold the GUI main window

      RootNode * pDataTreeRoot;	
      // variable which holds the root of the dataTree

      Tree * pGUITreeRoot;
      // variable which holds the root of the guiTree

      string homeDir;
      // vaiable which holds the home directory of XML files

      Controller();
      RootNode * createDataTree(RootNode * pRoot);
      void freeDataTree(RootNode * pRootNode);

   public:
      static Controller * getInstance();
      void replacePanel(GenericView * pNewPanel);
      void startApplication();
      void writeOutput(string directory);
      void openFile(string fileName);
      void fileNewClicked();
      string getDeviceName();
		
      // get and set methods
	
      Skeleton * getSkeleton()
      { return pSkeleton; }

      void setSkeleton(Skeleton * pNewSkeleton)
      { pSkeleton = pNewSkeleton; }

      RootNode * getDataTreeRoot()
      { return pDataTreeRoot; }

      void setDataTreeRoot(RootNode * pNewRoot)
      { pDataTreeRoot = pNewRoot; }

      Tree * getGUITreeRoot()
      { return pGUITreeRoot; }

      void setGUITreeRoot(Tree * pNewRoot)
      { pGUITreeRoot = pNewRoot; }

      string getHomeDir()
      { return homeDir; }

      void setHomeDir(string newDir)
      { homeDir = newDir; }
		
};	// end of class definition

};	// end of namespace

#endif
