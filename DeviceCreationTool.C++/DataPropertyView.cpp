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

#include "DataPropertyView.hpp"
#include "DataProperty.hpp"


using namespace OmniDeviceCreationTool;
using namespace Gtk;

DataPropertyView* DataPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new DataPropertyView(); 
   DataPropertyView *pUniqueInstance2 = (DataPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();
   
   return pUniqueInstance2; 
}

DataPropertyView::
DataPropertyView()
{
   pDataNameEntry = NULL;
   pDataTypeEntry = NULL;
   pDeviceDataEntry = NULL;
}

void DataPropertyView::
createWidgets()
{
   pDataNameEntry = manage(new Entry());
   pDataTypeEntry = manage(new Entry()); 
   pDeviceDataEntry = manage(new Entry());
   return;  
}

void DataPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Data Name ", pDataNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Data Type", pDataTypeEntry) ,false,false);
   getVbox()->pack_start(constructPair("Device Data ", pDeviceDataEntry) ,false,false);

   return;
}

void DataPropertyView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating  ...
   
   DataProperty *pDataNode2 = (DataProperty *)(this->getDataNode());
   
   // .............  
   pDataNameEntry->set_text(pDataNode2->getDataName());
   pDataTypeEntry->set_text(pDataNode2->getDataType());
   pDeviceDataEntry->set_text(pDataNode2->getDeviceData());
   
   return;
}

void DataPropertyView::
refreshNode()
{
   if(this->getDataNode() == NULL)
      return;
      
   // SAFE Downcating ...
   
   DataProperty *pDataNode2 = (DataProperty *)(this->getDataNode());
   
   // .............  
   pDataNode2->setDataName(pDataNameEntry->get_text());
   pDataNode2->setDataType(pDataTypeEntry->get_text());
   pDataNode2->setDeviceData(pDeviceDataEntry->get_text());   

   return;
}
