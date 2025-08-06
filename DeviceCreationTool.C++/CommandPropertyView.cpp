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

#include "CommandPropertyView.hpp"
#include "CommandProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;


CommandPropertyView* CommandPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new CommandPropertyView();
   CommandPropertyView *pUniqueInstance2 = (CommandPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();
   return pUniqueInstance2;
}

// Constructor
CommandPropertyView::
CommandPropertyView()
{
   pCommandNameEntry = NULL;
   pCommandEntry = NULL;
}

void CommandPropertyView::
createWidgets()
{
   if(NULL == pCommandNameEntry)
      pCommandNameEntry = manage(new Entry());

   if(NULL ==pCommandEntry)
      pCommandEntry = manage(new Entry());

    return;
}

void CommandPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Command Name ", pCommandNameEntry),
                                 false,
                                 false);
   getVbox()->pack_start(constructPair("Command", pCommandEntry),
                                 false,
                                 false);

   return;
}

void CommandPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   CommandProperty *pDataNode2 = (CommandProperty *)(this->getDataNode());

   pCommandNameEntry->set_text(pDataNode2->getCommandName());
   pCommandEntry->set_text(pDataNode2->getCommand());
   return;
}

void CommandPropertyView::
refreshNode()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating ...

   CommandProperty *pDataNode2 = (CommandProperty *)(this->getDataNode());

   pDataNode2->setCommandName(pCommandNameEntry->get_text());
   pDataNode2->setCommand(pCommandEntry->get_text());

   return;
}
