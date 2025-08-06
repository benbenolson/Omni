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

#include "FileSelector.hpp"
#include "Skeleton.hpp"
#include "Controller.hpp"
#include "Version.hpp"

#include "PopUpDialog.hpp"

using namespace Gtk;
using namespace OmniDeviceCreationTool;

Skeleton::
Skeleton()
{
   pVBox = manage(new VBox());
   pMenubar = manage(new MenuBar());
   pFileMenu = manage(new Menu());
   pHelpMenu =  manage(new Menu());
   pHBox = manage(new HBox());
   pHPaned = manage(new HPaned());
   pLeftPanel = manage(new ScrolledWindow());
   pRightPanel = manage(new ScrolledWindow());
   pGenericView = NULL;
   pTree = NULL;
}

void Skeleton::
setupMenu()
{
    using namespace Menu_Helpers;

    MenuList& deviceMenuList     = pFileMenu->items();

   deviceMenuList.push_back(MenuElem("_New", CTL|'n', slot(this, &Skeleton::onFileNewSelected)));
   deviceMenuList.push_back(MenuElem("_Save", "<control>s", slot(this, &Skeleton::onFileSaveSelected)));
   deviceMenuList.push_back(MenuElem("_Open Device", "<control>o", slot(this, &Skeleton::onFileOpenSelected)));
   deviceMenuList.push_back(SeparatorElem());
   deviceMenuList.push_back(MenuElem("_Quit", "<control>q", slot(this, &Skeleton::onFileQuitSelected)));

   pHelpMenu->items().push_back(MenuElem("_About", "<control>b", slot(this, &Skeleton::onHelpAboutSelected)));

   MenuList& menubarList = pMenubar->items();
   menubarList.push_front(MenuElem("_Help","<control>h",*pHelpMenu));
//   menubarList.front()->right_justify();
   pMenubar->items().push_front(MenuElem("_File","<control>d",*pFileMenu));

   return;
}

void Skeleton::
onFileNewSelected()
{
   Controller::getInstance()->fileNewClicked();
}

void Skeleton::
onFileSaveSelected()
{
   string deviceName =Controller::getInstance()->getDeviceName();
   if (deviceName.empty())
   {
      PopUpDialog::pop_dialog_OK("Device Name not given", "Cannot continue save operation");
      return;
   }
   string dirName;      // ="dummy" ;
   FileSelector fs("Choose a DIRECTORY only");
   fs.showDialog();
   if(fs.isOKClicked() )
   {
      dirName  = fs.getSelectedFileName();

      int last = dirName.find_last_of('/');
      dirName = dirName.substr(0,last+1);

      Controller::getInstance()->writeOutput(dirName);
   }
   return;
}


void Skeleton::
onFileOpenSelected()
{

   string fileName;     // ="dummy" ;
   FileSelector fs("Choose a DEVICE");
   fs.showDialog();
   if(fs.isOKClicked() )
   {
      fileName= fs.getSelectedFileName();
      if ( ! fileName.empty() )
         Controller::getInstance()->openFile(fileName);
   }

   return;
}


void Skeleton::
onFileQuitSelected()
{
   Gtk::Main::quit();
}

void Skeleton::
onHelpAboutSelected()
{
   //cout<<"About Menu "<<endl;
   string temp;
   temp = temp + TOOL_NAME +"\n" + VERSION;
   PopUpDialog::pop_dialog_OK(temp, COMPANY);
}

void Skeleton::
buildSkeleton()
{
   set_usize(700,500);

   setupMenu();

   add(*pVBox);
   pVBox->pack_start(*pMenubar,false,false,0);
   pVBox->pack_start(*pHBox);

   pHBox->pack_start(*pHPaned);

   pHPaned->add1(*pLeftPanel);
   pHPaned->add2(*pRightPanel);

   pLeftPanel->set_policy(GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
   pRightPanel->set_policy(GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

   pLeftPanel->set_usize(200,500);
}

void Skeleton::
removeTree()
{
   if(pTree == NULL)
       return;

   pLeftPanel->remove();
   pTree = NULL;
   return;
}

void Skeleton::
addTree(Tree *rootTree)
{
   if(pTree != NULL)
      pLeftPanel->remove();
   pTree =   rootTree;
   pLeftPanel->add_with_viewport(*pTree);

   show_all();

   return;
}

void Skeleton::
addView(GenericView *gv)
{
   if(pGenericView != NULL)
      pRightPanel->remove();
   pGenericView = gv;
   pRightPanel->add_with_viewport(*pGenericView);

   show_all();
   return;
}

void Skeleton::
removeView()
{
   if(pGenericView == NULL)
      return;
   pRightPanel->remove();
   pGenericView = NULL;

   return;
}
