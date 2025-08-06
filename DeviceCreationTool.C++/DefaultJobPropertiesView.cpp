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

#include "DefaultJobPropertiesView.hpp"
#include "DefaultJobProperties.hpp"

using namespace OmniDeviceCreationTool;

DefaultJobPropertiesView * DefaultJobPropertiesView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new DefaultJobPropertiesView();
   DefaultJobPropertiesView *pUniqueInstance2 = (DefaultJobPropertiesView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

DefaultJobPropertiesView::
DefaultJobPropertiesView()
{
   pOrientationEntry = NULL;
   pFormEntry = NULL;
   pTrayEntry = NULL;
   pMediaEntry = NULL;
   pResolutionEntry = NULL;
   pDitherEntry = NULL;
   pPrintModeEntry = NULL;
   pOtherEntry = NULL;
}


void DefaultJobPropertiesView::
customizeView()
{
   // Remove the delete button and clone button
   getHbox()->remove(*pDeleteButton);
   getHbox()->remove(*pCloneButton);
}


void DefaultJobPropertiesView::
createWidgets()
{
   pOrientationEntry = manage(new Entry());
   pFormEntry = manage(new Entry());
   pTrayEntry = manage(new Entry());
   pMediaEntry = manage(new Entry());
   pResolutionEntry = manage(new Entry());
   pDitherEntry = manage(new Entry());
   pPrintModeEntry = manage(new Entry());
   pOtherEntry = manage(new Entry());
}



void DefaultJobPropertiesView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Orientation", pOrientationEntry) ,false,false);
   getVbox()->pack_start(constructPair("Form", pFormEntry) ,false,false);
   getVbox()->pack_start(constructPair("Tray", pTrayEntry) ,false,false);
   getVbox()->pack_start(constructPair("Media", pMediaEntry) ,false,false);
   getVbox()->pack_start(constructPair("Resolution", pResolutionEntry) ,false,false);
   getVbox()->pack_start(constructPair("Dither", pDitherEntry) ,false,false);
   getVbox()->pack_start(constructPair("Print Mode", pPrintModeEntry) ,false,false);
   getVbox()->pack_start(constructPair("Other", pOtherEntry) ,false,false);
   return ;
}


void DefaultJobPropertiesView::
refreshView()
{
   if(this->getDataNode() == NULL )
      return;

   // SAFE Downcating  ...

   DefaultJobProperties * pDataNode2 = (DefaultJobProperties*)(this->getDataNode());

   pOrientationEntry->set_text(pDataNode2->getOrientation());
   pFormEntry->set_text(pDataNode2->getForm());
   pTrayEntry->set_text(pDataNode2->getTray());
   pMediaEntry->set_text(pDataNode2->getMedia());
   pResolutionEntry->set_text(pDataNode2->getResolution());
   pDitherEntry->set_text(pDataNode2->getDither());
   pPrintModeEntry->set_text(pDataNode2->getPrintMode());
   // new
   pOtherEntry->set_text(pDataNode2->getOther());
   return;
}

void DefaultJobPropertiesView::
refreshNode()
{
   if (this->getDataNode() == NULL)
      return;

   // SAFE Downcating ...

   DefaultJobProperties * pDataNode2 = (DefaultJobProperties*)(this->getDataNode());

   pDataNode2->setOrientation(pOrientationEntry->get_text());
   pDataNode2->setForm(pFormEntry->get_text());
   pDataNode2->setTray(pTrayEntry->get_text());
   pDataNode2->setMedia(pMediaEntry->get_text());
   pDataNode2->setResolution(pResolutionEntry->get_text());
   pDataNode2->setDither(pDitherEntry->get_text());
   pDataNode2->setPrintMode(pPrintModeEntry->get_text());
   pDataNode2->setOther(pOtherEntry->get_text());
   return;
}
