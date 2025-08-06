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

#include "TrayPropertyView.hpp"
#include "TrayProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;

TrayPropertyView* TrayPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new TrayPropertyView();
   TrayPropertyView *pUniqueInstance2 = (TrayPropertyView *)pUniqueInstance; 
   pUniqueInstance2->initializeView();
   
   return pUniqueInstance2; 
}

TrayPropertyView::
TrayPropertyView()
{
   pTrayNameEntry = NULL ;
   pTrayTypeEntry = NULL ;
   pCommandEntry = NULL ;
}

void TrayPropertyView::
createWidgets()
{
   pTrayNameEntry = manage(new Entry());
   pTrayTypeEntry = manage(new Entry());
   pCommandEntry = manage(new Entry());
   
    return; 
}

void TrayPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Tray Name ", pTrayNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Tray Type ", pTrayTypeEntry) ,false,false);
   getVbox()->pack_start(constructPair("Command ", pCommandEntry) ,false,false);

   return;
}

void TrayPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   TrayProperty *pDataNode2 = (TrayProperty *)(this->getDataNode());
   
   // .............  
   
   pTrayNameEntry->set_text(pDataNode2->getTrayName());
   pTrayTypeEntry->set_text(pDataNode2->getTrayType());
   pCommandEntry->set_text(pDataNode2->getCommand());
   
   //..........
   
   return;
}

void TrayPropertyView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   TrayProperty *pDataNode2 = (TrayProperty *)(this->getDataNode());
   
   // .............  
   
   pDataNode2->setTrayName(pTrayNameEntry->get_text());
   pDataNode2->setTrayType(pTrayTypeEntry->get_text());
   pDataNode2->setCommand(pCommandEntry->get_text());

   //............
   
   return;
}
