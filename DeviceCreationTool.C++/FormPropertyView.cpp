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

#include "FormPropertyView.hpp"
#include "FormProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;

FormPropertyView* FormPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new FormPropertyView();
   FormPropertyView *pUniqueInstance2 = (FormPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();
   
   return pUniqueInstance2; 
}

FormPropertyView::
FormPropertyView()
{
   pFormNameEntry = NULL;
   pFormCapabilityEntry = NULL;
   pCommandEntry = NULL;
   pHardCopyCapLeftEntry = NULL;
   pHardCopyCapTopEntry = NULL;
   pHardCopyCapRightEntry = NULL;
   pHardCopyCapBottomEntry = NULL;
}

void FormPropertyView::
createWidgets()
{
   pFormNameEntry = manage(new Entry());
   pFormCapabilityEntry = manage(new Entry());
   pCommandEntry = manage(new Entry());
   pHardCopyCapLeftEntry = manage(new Entry());
   pHardCopyCapTopEntry = manage(new Entry());
   pHardCopyCapRightEntry = manage(new Entry());
   pHardCopyCapBottomEntry = manage(new Entry());

    return; 
}

void FormPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Form Name", pFormNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Form Capability", pFormCapabilityEntry) ,false,false);
   getVbox()->pack_start(constructPair("Command", pCommandEntry) ,false,false);
   getVbox()->pack_start(constructPair("Hard Copy Capability - Left", pHardCopyCapLeftEntry) ,false,false);
   getVbox()->pack_start(constructPair("Hard Copy Capability - Top", pHardCopyCapTopEntry) ,false,false);
   getVbox()->pack_start(constructPair("Hard Copy Capability - Right", pHardCopyCapRightEntry) ,false,false);
   getVbox()->pack_start(constructPair("Hard Copy Capability - Bottom", pHardCopyCapBottomEntry) ,false,false);
         
   return;
}

void FormPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   FormProperty *pDataNode2 = (FormProperty *)(this->getDataNode());
 
   pFormNameEntry->set_text(pDataNode2->getFormName());
   pFormCapabilityEntry->set_text(pDataNode2->getFormCapability());
   pCommandEntry->set_text(pDataNode2->getCommand());
   pHardCopyCapLeftEntry->set_text(pDataNode2->getHardCopyCapLeft());
   pHardCopyCapTopEntry->set_text(pDataNode2->getHardCopyCapTop());
   pHardCopyCapRightEntry->set_text(pDataNode2->getHardCopyCapRight());
   pHardCopyCapBottomEntry->set_text(pDataNode2->getHardCopyCapBottom());
 
   return;
}

void FormPropertyView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   FormProperty *pDataNode2 = (FormProperty *)(this->getDataNode());

   pDataNode2->setFormName(pFormNameEntry->get_text());
   pDataNode2->setFormCapability(pFormCapabilityEntry->get_text()); 
   pDataNode2->setCommand(pCommandEntry->get_text());
   pDataNode2->setHardCopyCapLeft(pHardCopyCapLeftEntry->get_text());
   pDataNode2->setHardCopyCapTop(pHardCopyCapTopEntry->get_text());
   pDataNode2->setHardCopyCapRight(pHardCopyCapRightEntry->get_text());
   pDataNode2->setHardCopyCapBottom(pHardCopyCapBottomEntry->get_text());

   return;
}
