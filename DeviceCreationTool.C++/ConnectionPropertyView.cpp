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

#include "ConnectionPropertyView.hpp"
#include "ConnectionProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;

ConnectionPropertyView* ConnectionPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new ConnectionPropertyView();
   // downcasting
   ConnectionPropertyView *pUniqueInstance2 = (ConnectionPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

ConnectionPropertyView::
ConnectionPropertyView()
{
   pConnectionNameEntry = NULL;
   pConnectionFormEntry = NULL;
   pConnectionTrayEntry = NULL;
   pConnectionMediaEntry = NULL;
}

void ConnectionPropertyView::
createWidgets()
{
   pConnectionNameEntry = manage(new Entry());
   pConnectionFormEntry = manage(new Entry());
   pConnectionTrayEntry = manage(new Entry());
   pConnectionMediaEntry = manage(new Entry());

    return;
}

void ConnectionPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Command Name ", pConnectionNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Connection Form", pConnectionFormEntry) ,false,false);
   getVbox()->pack_start(constructPair("Connection Tray ", pConnectionTrayEntry ) ,false,false);
   getVbox()->pack_start(constructPair("Connection Media ", pConnectionMediaEntry) ,false,false);

   return;
}

void ConnectionPropertyView::refreshView()
{
   if(this->getDataNode() ==  NULL )
      return;

   // SAFE Downcating needed ...

   ConnectionProperty *pDataNode2 = (ConnectionProperty *)(this->getDataNode());

   // .............

   pConnectionNameEntry->set_text(pDataNode2->getConnectionName());
   pConnectionFormEntry->set_text(pDataNode2->getConnectionForm());
   pConnectionTrayEntry->set_text(pDataNode2->getConnectionTray());
   pConnectionMediaEntry->set_text(pDataNode2->getConnectionMedia());

   //..........

   return;
}

void ConnectionPropertyView::refreshNode()
{
   if(this->getDataNode() == NULL )
      return;

   // SAFE Downcating needed ...

   ConnectionProperty *pDataNode2 = (ConnectionProperty *)(this->getDataNode());

   // .............

   pDataNode2->setConnectionName(pConnectionNameEntry->get_text());
   pDataNode2->setConnectionForm(pConnectionFormEntry->get_text());
   pDataNode2->setConnectionTray(pConnectionTrayEntry->get_text());
   pDataNode2->setConnectionMedia(pConnectionMediaEntry->get_text());

   //............

   return;
}
