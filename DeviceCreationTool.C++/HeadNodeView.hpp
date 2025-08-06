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
#ifndef _COM_IBM_LTC_OMNI_HeadNodeView_Hpp_
#define _COM_IBM_LTC_OMNI_HeadNodeView_Hpp_

#include <gtk--.h>

#include "GenericView.hpp"
#include "Node.hpp"

#define TOP_FRAME_LABEL_TEXT "Click on the Create New button if you want to create a\n" \
                             "new file. You can either add new values or load values\n" \
                             "from existing file. The values loaded from a file will be\n" \
                             "displayed and they can be modified"

#define ADD_NEW_BUTTON_LABEL_TEXT "Click this button to add new values"

#define LOAD_FILE_BUTTON_LABEL_TEXT " "

#define BOTTOM_FRAME_LABEL_TEXT "Click on Use Existing button if you want to specify a\n" \
                                "pointer to an existing file. No values will be displayed.\n" \
                                "The pointer (File Name) will be stored in the Base file"

#define SELECT_EXISTING_BUTTON_LABEL_TEXT " "


using namespace Gtk;

namespace OmniDeviceCreationTool {

class HeadNodeView : public GenericView
{

public:
   static HeadNodeView* getInstance();

   void onReuseRadioToggled();
   void onCreateRadioToggled();
   void onAddNewButtonClicked();
   void onLoadFileButtonClicked();
   void onSelectExistingButtonClicked();
   virtual void populateView(Node *);

protected:
   void selectFile();
   HeadNodeView();


   Frame *pTopFrame;
   VBox *pTopFrameVBox;
   Frame *pBottomFrame;
   VBox *pBottomFrameVBox;
   Label  *pTopFrameLabel;
   Label  *pBottomFrameLabel;

   RadioButton *pReuseRadioButton;
   RadioButton *pCreateRadioButton;

   Button *pSelectExistingButton;
   Button *pAddNewButton;
   Button *pLoadFileButton;

   Label *pSelectExistingButtonLabel;
   Label *pLoadFileButtonLabel;
   Label *pAddNewButtonLabel;

   HBox *pAddNewGroupHBox;
   HBox *pLoadFileGroupHBox;
   HBox *pSelectExistingGroupHBox;


};

}; // end of namespace

#endif
