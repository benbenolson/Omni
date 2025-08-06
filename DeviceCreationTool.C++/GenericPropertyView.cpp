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

#include <gtk--/entry.h>
#include <gtk--/label.h>
#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/frame.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/button.h>
#include <gtk--/main.h>

#include "GenericPropertyView.hpp"
#include "HeadNode.hpp"
#include "GenericView.hpp"
#include "Controller.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;


GenericPropertyView::GenericPropertyView()
       	: GenericView()
{
   setDataNode(NULL);
   pHbox = manage(new HButtonBox());
   pVbox = manage(new VBox());
   pFrame =manage(new Frame());
   pScrWindow=manage(new  ScrolledWindow());
   pCancelButton=manage(new  Button("Cancel"));
   pOkButton=manage(new  Button("OK"));
   pDeleteButton = manage(new Button("Delete"));
   pCloneButton = manage(new Button("Clone"));
}

void GenericPropertyView::
populateView(Node *pNode)
{
   setDataNode(pNode);
   refreshView();
   return;
}

void GenericPropertyView::
initializeView()
{
   buildView();
   setupConnections();
   customizeView();
   createWidgets();
   addWidgets();
}

void GenericPropertyView::
buildView()
{
   pack_start(*pScrWindow);
   pack_end(*pFrame,false,false);

   pScrWindow->add_with_viewport(*pVbox);
   pFrame->add(*pHbox);

   pScrWindow->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   pScrWindow->set_border_width(20);
   pHbox->set_border_width(10);
   pHbox->set_layout(GTK_BUTTONBOX_END);
   pHbox->set_spacing(20);
   pHbox->set_child_size(60,25);

   pHbox->pack_start(*pOkButton, false);
   pHbox->pack_start(*pCancelButton, false);
   pHbox->pack_start(*pCloneButton, false);
   pHbox->pack_start(*pDeleteButton, false);
}

HBox& GenericPropertyView::
constructPair(string s, Widget* pWidget)
{
   HBox *pPairBox = manage(new HBox());
   Label *pLabel = manage(new Label(s,0));

   pPairBox->pack_start(*pLabel);
   pPairBox->pack_start(*pWidget, false, false, 20);

   pPairBox->set_border_width(5);
   pWidget->set_usize(200,25);

   return *pPairBox;
}

void GenericPropertyView::
setupConnections()
{
   pCancelButton->clicked.connect(slot(this, &GenericPropertyView::onCancelButtonClicked));
   pOkButton->clicked.connect(slot(this, &GenericPropertyView::onOKButtonClicked));
   pDeleteButton->clicked.connect(slot(this, &GenericPropertyView::onDeleteButtonClicked));
   pCloneButton->clicked.connect(slot(this, &GenericPropertyView::onCloneButtonClicked));

}

void GenericPropertyView::
onCancelButtonClicked()
{
   refreshView();
   return;
}

void GenericPropertyView::
onOKButtonClicked()
{
   // Validate here
   refreshNode();
   getDataNode()->setSaved(true);
   getDataNode()->updateDisplayName();
   return;
}

void GenericPropertyView::
onDeleteButtonClicked()
{
   // down cast Node* to HeadNode*
   HeadNode* pParent = (HeadNode *) getDataNode()->getParentNode();
   pParent->deleteChild(getDataNode());

   Controller::getInstance()->getGUITreeRoot()->select_child(*(pParent->getTreeItem()));
   return;
}

void GenericPropertyView::
onCloneButtonClicked()
{
   // down cast Node* to HeadNode*
   HeadNode* pParent = (HeadNode *) getDataNode()->getParentNode();
   pParent->cloneChild(getDataNode());
   return;
}
