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

#include "ResolutionPropertyView.hpp"
#include "ResolutionProperty.hpp"


using namespace OmniDeviceCreationTool;
using namespace Gtk;


ResolutionPropertyView* ResolutionPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new ResolutionPropertyView();
   ResolutionPropertyView *pUniqueInstance2 = (ResolutionPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

ResolutionPropertyView::
ResolutionPropertyView()
{
   pResolutionNameEntry = NULL;
   pXResEntry = NULL;
   pYResEntry = NULL;
   pXInternalResEntry = NULL;
   pYInternalResEntry = NULL;
   pCommandEntry = NULL ;
   pResolutionCapabilityEntry = NULL ;
   pResolutionDestinationBitsPerPelEntry = NULL;
   pResolutionScanLineMultipleEntry = NULL;

}

void ResolutionPropertyView::
createWidgets()
{
   pResolutionNameEntry = manage(new Entry());
   pXResEntry = manage(new Entry());
   pYResEntry = manage(new Entry());
   pXInternalResEntry = manage(new Entry());
   pYInternalResEntry = manage(new Entry());
   pCommandEntry = manage(new Entry());
   pResolutionCapabilityEntry  = manage(new Entry());
   pResolutionDestinationBitsPerPelEntry  = manage(new Entry());
   pResolutionScanLineMultipleEntry  = manage(new Entry());

    return;
}

void ResolutionPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Resolution Name", pResolutionNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("X Resolution", pXResEntry ) ,false,false);
   getVbox()->pack_start(constructPair("Y Resolution", pYResEntry) ,false,false);
   getVbox()->pack_start(constructPair("X Internal Resolution", pXInternalResEntry ) ,false,false);
   getVbox()->pack_start(constructPair("Y Internal Resolution", pYInternalResEntry) ,false,false);
   getVbox()->pack_start(constructPair("Command", pCommandEntry) ,false,false);
   getVbox()->pack_start(constructPair("ResolutionCapability", pResolutionCapabilityEntry ) ,false,false);
   getVbox()->pack_start(constructPair("ResolutionDestinationBitsPerPel",
                                                                  pResolutionDestinationBitsPerPelEntry) ,false,false);
   getVbox()->pack_start(constructPair("Resolution Scan Line Multiple",
                                                                  pResolutionScanLineMultipleEntry ) ,false,false);

   return;
}

void ResolutionPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;
   // SAFE Downcating ...
   ResolutionProperty *pDataNode2 = (ResolutionProperty *)(this->getDataNode());

   pResolutionNameEntry->set_text(pDataNode2->getResolutionName());
   pXResEntry->set_text(pDataNode2->getXRes());
   pYResEntry->set_text(pDataNode2->getYRes());
   pXInternalResEntry->set_text(pDataNode2->getXInternalRes());
   pYInternalResEntry->set_text(pDataNode2->getYInternalRes());
   pCommandEntry->set_text(pDataNode2->getCommand());
   pResolutionCapabilityEntry->set_text(pDataNode2->getResolutionCapability());
   pResolutionDestinationBitsPerPelEntry->set_text(pDataNode2->getResolutionDestinationBitsPerPel());
   pResolutionScanLineMultipleEntry->set_text(pDataNode2->getResolutionScanLineMultiple());

   return;
}

void ResolutionPropertyView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;
   // SAFE Downcating  ...
   ResolutionProperty *pDataNode2 = (ResolutionProperty *)(this->getDataNode());

   pDataNode2->setResolutionName(pResolutionNameEntry->get_text());
   pDataNode2->setXRes(pXResEntry->get_text());
   pDataNode2->setYRes(pYResEntry->get_text());
   pDataNode2->setXInternalRes(pXInternalResEntry->get_text());
   pDataNode2->setYInternalRes(pYInternalResEntry->get_text());
   pDataNode2->setCommand(pCommandEntry->get_text());
   pDataNode2->setResolutionCapability(pResolutionCapabilityEntry->get_text());
   pDataNode2->setResolutionDestinationBitsPerPel(pResolutionDestinationBitsPerPelEntry->get_text());
   pDataNode2->setResolutionScanLineMultiple(pResolutionScanLineMultipleEntry->get_text());

   return;
}
