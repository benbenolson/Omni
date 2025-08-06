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

#include "MediaPropertyView.hpp"
#include "MediaProperty.hpp"


using namespace OmniDeviceCreationTool;
using namespace Gtk;


MediaPropertyView* MediaPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new MediaPropertyView();
   MediaPropertyView *pUniqueInstance2 = (MediaPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();
   
   return pUniqueInstance2; 
}

MediaPropertyView::
MediaPropertyView()
{
   pMediaNameEntry = NULL;
   pCommandEntry = NULL;
   pMediaColorAdjustRequiredEntry = NULL;
   pMediaAbsorptionEntry = NULL;
}

void MediaPropertyView::
createWidgets()
{
   pMediaNameEntry = manage(new Entry() );
   pCommandEntry = manage(new Entry() );
   pMediaColorAdjustRequiredEntry = manage(new Entry() );
   pMediaAbsorptionEntry = manage(new Entry() );

    return; 
}

void MediaPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Media Name ", pMediaNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Command", pCommandEntry) ,false,false);
   getVbox()->pack_start(constructPair("Media Color Adjust Required ", 
                                          pMediaColorAdjustRequiredEntry) ,false,false);
   getVbox()->pack_start(constructPair("Media Absorption ", pMediaAbsorptionEntry) ,false,false);
   
   return;
}

void MediaPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   MediaProperty *pDataNode2 = (MediaProperty *)(this->getDataNode());
   
   // .............  
   
   pMediaNameEntry->set_text(pDataNode2->getMediaName());
   pCommandEntry->set_text(pDataNode2->getCommand());
   pMediaColorAdjustRequiredEntry->set_text(pDataNode2->getMediaColorAdjustRequired());
   pMediaAbsorptionEntry->set_text(pDataNode2->getMediaAbsorption());
   
   //..........
   
   return;
}

void MediaPropertyView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   MediaProperty *pDataNode2 = (MediaProperty *)(this->getDataNode());
   
   // .............  
   
   pDataNode2->setMediaName(pMediaNameEntry->get_text());
   pDataNode2->setCommand(pCommandEntry->get_text());
   pDataNode2->setMediaColorAdjustRequired(pMediaColorAdjustRequiredEntry->get_text());
   pDataNode2->setMediaAbsorption(pMediaAbsorptionEntry->get_text());
   
   //............
   
   return;
}
