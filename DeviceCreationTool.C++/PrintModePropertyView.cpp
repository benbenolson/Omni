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

#include "PrintModePropertyView.hpp"
#include "PrintModeProperty.hpp"


using namespace OmniDeviceCreationTool;
using namespace Gtk;

PrintModePropertyView* PrintModePropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new PrintModePropertyView();
   PrintModePropertyView *pUniqueInstance2 = (PrintModePropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

PrintModePropertyView::
PrintModePropertyView()
{
   pPrintModeNameEntry = NULL;
   pPrintModePhysicalCountEntry = NULL;
   pPrintModeLogicalCountEntry = NULL;
   pPrintModePlanesEntry = NULL;

}

void PrintModePropertyView::
createWidgets()
{
   pPrintModeNameEntry = manage(new Entry());
   pPrintModePhysicalCountEntry = manage(new Entry());
   pPrintModeLogicalCountEntry = manage(new Entry());
   pPrintModePlanesEntry = manage(new Entry());

   return;
}

void PrintModePropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Print Mode Name ", pPrintModeNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Print Mode Physical Count ", pPrintModePhysicalCountEntry) ,false,false);
   getVbox()->pack_start(constructPair("Print Mode Logical Count ", pPrintModeLogicalCountEntry) ,false,false);
   getVbox()->pack_start(constructPair("Print Mode Planes ", pPrintModePlanesEntry) ,false,false);

   return;
}

void PrintModePropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   PrintModeProperty *pDataNode2 = (PrintModeProperty *)(this->getDataNode());

   // .............

   pPrintModeNameEntry->set_text(pDataNode2->getPrintModeName());
   pPrintModePhysicalCountEntry->set_text(pDataNode2->getPrintModePhysicalCount());
   pPrintModeLogicalCountEntry->set_text(pDataNode2->getPrintModeLogicalCount());
   pPrintModePlanesEntry->set_text(pDataNode2->getPrintModePlanes());

   //..........

   return;
}

void PrintModePropertyView::
refreshNode()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   PrintModeProperty *pDataNode2 = (PrintModeProperty *)(this->getDataNode());

   // .............

   pDataNode2->setPrintModeName(pPrintModeNameEntry->get_text());
   pDataNode2->setPrintModePhysicalCount(pPrintModePhysicalCountEntry->get_text());
   pDataNode2->setPrintModeLogicalCount(pPrintModeLogicalCountEntry->get_text());
   pDataNode2->setPrintModePlanes(pPrintModePlanesEntry->get_text());
   //............

   return;
}
