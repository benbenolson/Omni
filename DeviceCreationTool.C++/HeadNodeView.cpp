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

#include <gtk--.h>
#include <iostream>

#include "HeadNodeView.hpp"
#include "GenericView.hpp"
#include "FileSelector.hpp"
#include "HeadNode.hpp"
//#include "Controller.hpp"

#include "PopUpDialog.hpp"

using namespace Gtk;

using namespace OmniDeviceCreationTool;


HeadNodeView* HeadNodeView::
getInstance()
{

   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new HeadNodeView();

   return (HeadNodeView *)pUniqueInstance;
}

HeadNodeView::
HeadNodeView()
{
   setDataNode(NULL);

   set_border_width(20);

   pTopFrame = manage(new Frame("New File Created"));

   pTopFrameVBox = manage(new VBox());
   pTopFrameVBox->set_spacing(15);
   pTopFrameVBox->set_border_width(15);

   pTopFrameLabel = manage(new Label(TOP_FRAME_LABEL_TEXT, 0, 0.5));
   pCreateRadioButton = manage(new RadioButton ("Create New"));

   pAddNewGroupHBox = manage(new HBox());

   pAddNewGroupHBox->set_spacing(10);
   pAddNewGroupHBox->set_border_width(10);

   pAddNewButton = manage(new Button("Add New"));
   pAddNewButton->set_usize(100,25);
   pAddNewButtonLabel = manage(new Label(ADD_NEW_BUTTON_LABEL_TEXT));
   pAddNewButton->clicked.connect(slot(this, &HeadNodeView::onAddNewButtonClicked));

   pLoadFileGroupHBox = manage(new HBox());

   pLoadFileGroupHBox->set_spacing(10);
   pLoadFileGroupHBox->set_border_width(10);

   pLoadFileButton = manage(new Button("Load File"));
   pLoadFileButton->set_usize(100,25);
   pLoadFileButtonLabel = manage(new Label(LOAD_FILE_BUTTON_LABEL_TEXT));
   pLoadFileButton->clicked.connect(slot(this, &HeadNodeView::onLoadFileButtonClicked));

   pBottomFrame = manage(new Frame("File Not created"));
   pBottomFrameVBox = manage(new VBox());
   pBottomFrameVBox->set_spacing(15);
   pBottomFrameVBox->set_border_width(15);

   pBottomFrameLabel = manage(new Label(BOTTOM_FRAME_LABEL_TEXT, 0, 0.5));
   pReuseRadioButton = manage(new RadioButton("Use Existing File"));
   pReuseRadioButton->set_group(pCreateRadioButton->group());

   pSelectExistingGroupHBox = manage(new HBox());
   pSelectExistingGroupHBox->set_spacing(10);
   pSelectExistingButton = manage(new Button("Select Existing File"));
   pSelectExistingButton->set_usize(120,25);
   pSelectExistingButtonLabel = manage(new Label(SELECT_EXISTING_BUTTON_LABEL_TEXT));
   pSelectExistingButton->clicked.connect(slot(this, &HeadNodeView::onSelectExistingButtonClicked));

   pack_start(*pTopFrame, false, false);
   pTopFrame->add(*pTopFrameVBox);
   pTopFrameVBox->pack_start(*pTopFrameLabel, false, false);
   pTopFrameVBox->pack_start(*pCreateRadioButton, false, false);
   pTopFrameVBox->pack_start(*pAddNewGroupHBox, false, false);

   pAddNewGroupHBox->pack_start(*pAddNewButton, false, false);
   pAddNewGroupHBox->pack_start(*pAddNewButtonLabel, false, false);

   pTopFrameVBox->pack_start(*pLoadFileGroupHBox, false, false);
   pLoadFileGroupHBox->pack_start(*pLoadFileButton , false, false);
   pLoadFileGroupHBox->pack_start(*pLoadFileButtonLabel , false, false);

   pack_start(*pBottomFrame, false, false);
   pBottomFrame->add(*pBottomFrameVBox);
   pBottomFrameVBox->pack_start(*pBottomFrameLabel, false, false);
   pBottomFrameVBox->pack_start(*pReuseRadioButton, false, false);
   pBottomFrameVBox->pack_start(*pSelectExistingGroupHBox, false, false);

   pSelectExistingGroupHBox->pack_start(*pSelectExistingButton, false, false);
   pSelectExistingGroupHBox->pack_start(*pSelectExistingButtonLabel, false, false);

   pReuseRadioButton->toggled.connect(slot(this, &HeadNodeView::onReuseRadioToggled));
   pCreateRadioButton->toggled.connect(slot(this, &HeadNodeView::onCreateRadioToggled));

}

void HeadNodeView::
onAddNewButtonClicked()
{
   // downcasting
   HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());
   pDataNode2->addButtonClickHandler();
}

void HeadNodeView::
onLoadFileButtonClicked()
{

   string homeDir = Controller::getInstance()->getHomeDir();
   FileSelector fs("XML File Selection", homeDir);

   fs.show();
   Gtk::Main::run();

   if(fs.isOKClicked())
   {
      string selectedFile2 = fs.getSelectedFileName();
      if (selectedFile2.empty())
         return;
      if (selectedFile2 == homeDir)
         return;

       string temp;
       int iLastSlash = selectedFile2.find_last_of('/');

       if (iLastSlash == string::npos)
          temp = selectedFile2;
       else
          temp = selectedFile2.substr(iLastSlash+1);

      pLoadFileButtonLabel->set_text("Loaded From : " + temp);
      pLoadFileButtonLabel->show();
      // downcasting
      HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());
      pDataNode2->setLoadFileName(temp);
      pDataNode2->loadButtonClickHandler(selectedFile2);
   }
}


void HeadNodeView::
onSelectExistingButtonClicked()
{
   // issue a warning and confirm to delete any added children
   selectFile();
   // downcasting
   HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());
   pSelectExistingButtonLabel->set_text("Selected File : " + pDataNode2->getSelectedFileName());
   pSelectExistingButtonLabel->show();
}

void HeadNodeView::
selectFile()
{
   string homeDir = Controller::getInstance()->getHomeDir();
   FileSelector fs("XML File Selection", homeDir);

   fs.show();
   Gtk::Main::run();

   if(fs.isOKClicked())
   {
      HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());
      string selectedFile = fs.getSelectedFileName() ;

      int iLastSlash = selectedFile.find_last_of('/');
      selectedFile = selectedFile.substr(iLastSlash+1);
      pDataNode2->setSelectedFileName(selectedFile);
   }
}


void HeadNodeView::
onCreateRadioToggled()
{

// clear selected files or added children....

   bool val = pCreateRadioButton->get_active();
   pAddNewButton->set_sensitive(val);
   pLoadFileButton->set_sensitive(val);
   if (val==true)
   {
      HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());
      pDataNode2->setToggleFlag(1);
 /*
  *    string empty;
  *   pSelectExistingButtonLabel->set_text(empty);
  *   pDataNode2->setSelectedFileName(empty);
  */
   }
   return;
}

void HeadNodeView::
onReuseRadioToggled()
{
   bool val = pReuseRadioButton->get_active();
   pSelectExistingButton->set_sensitive(val);
   if (val==true)
   {
/*
*      bool ret = false;
*      ret = PopUpDialog::pop_dialog("Any data added will be lost!", "Want to continue?");
*      if (!ret)
*      {
*         pCreateRadioButton->set_active(true);
*         return;
*      }
*/
      //........
      HeadNode *pDataNode2= (HeadNode *)(this->getDataNode());
      pDataNode2->setToggleFlag(2);
//      pDataNode2->deleteAllChildren();
   }
   return;
}

void HeadNodeView::
populateView(Node* pNode)
{
   setDataNode(pNode);

   if(this->getDataNode() == NULL)
      return;


   // SAFE Downcating ...

   HeadNode *pDataNode2 = (HeadNode *)(this->getDataNode());

   // .............


   switch(pDataNode2->getToggleFlag())
   {
   case 1:
      pCreateRadioButton->set_active(true);
      pSelectExistingButton->set_sensitive(false);
      pAddNewButton->set_sensitive(true);
      pLoadFileButton->set_sensitive(true);
      break;
   case 2:
      pReuseRadioButton->set_active(true);
      pSelectExistingButton->set_sensitive(true);
      pAddNewButton->set_sensitive(false);
      pLoadFileButton->set_sensitive(false);
      break;
   default:
      cerr << "Invalid value for toggle flag" << endl;
      break;
   }

   pLoadFileButtonLabel->set_text("Loaded From : " + pDataNode2->getLoadFileName());
   pSelectExistingButtonLabel->set_text("Selected File : " + pDataNode2->getSelectedFileName());

}
