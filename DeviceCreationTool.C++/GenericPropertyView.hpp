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
#ifndef _COM_IBM_LTC_OMNI_GenericPropertyView_Hpp
#define _COM_IBM_LTC_OMNI_GenericPropertyView_Hpp

#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/frame.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/button.h>

#include "Node.hpp"
#include "GenericView.hpp"

using namespace Gtk;

namespace OmniDeviceCreationTool {

class GenericPropertyView : public GenericView
{
public:
   void buildView();
   virtual void populateView(Node *);

protected:
   GenericPropertyView();

   virtual void createWidgets(){};
   virtual void addWidgets(){};
   virtual void customizeView(){ };
   // Customize Gen & Default Job properties
   //  where no DELETE button should be given.

   virtual void refreshView(){};
   virtual void refreshNode(){};

   void initializeView();
   HBox& constructPair(string s, Widget* pW);
   void setupConnections();

   void onCancelButtonClicked();
   void onOKButtonClicked();
   void onDeleteButtonClicked();
   void onCloneButtonClicked();

protected:

   HButtonBox *pHbox;
   VBox *pVbox;
   Frame *pFrame;
   ScrolledWindow  *pScrWindow;
   Button *pCancelButton;
   Button *pOkButton;
   Button *pDeleteButton;
   Button *pCloneButton;

   VBox *getVbox()
   { return pVbox; }

   HButtonBox *getHbox()
   { return pHbox; }
};

}; // end of namespace

#endif
