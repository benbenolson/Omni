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
#include <gtk--/box.h>
#include <gtk--/buttonbox.h>
#include <gtk--/frame.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/main.h>

#include "GeneralPropertiesView.hpp"
#include "GeneralProperties.hpp"


using namespace OmniDeviceCreationTool;


GeneralPropertiesView* GeneralPropertiesView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new GeneralPropertiesView();
   GeneralPropertiesView *pUniqueInstance2 = (GeneralPropertiesView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

GeneralPropertiesView::
GeneralPropertiesView()
{
   pDeviceNameEntry=NULL;
   pDriverNameEntry=NULL;
   pDeviceOptionsTypeEntry=NULL;
   pDeviceOptionsType2Entry=NULL;
   pPdlLevelEntry=NULL;
   pPdlSubLevelEntry=NULL;
   pPdlMajorEntry=NULL;
   pPdlMinorEntry=NULL;
   pBlitterCppEntry=NULL;
   pBlitterHppEntry=NULL;
   pInstanceCppEntry=NULL;
   pInstanceHppEntry=NULL;
   pCapabilityTypeEntry=NULL;
   pRasterCapabilityTypeEntry=NULL;
   pPluggableInstanceEntry = NULL;
   pPluggableBlitterEntry = NULL;
}


void GeneralPropertiesView::
customizeView()
{
  // Remove the delete button and clone button
   getHbox()->remove(*pDeleteButton);
   getHbox()->remove(*pCloneButton);
}

void GeneralPropertiesView::
createWidgets()
{
      pDeviceNameEntry = manage(new Entry());
      pDriverNameEntry = manage( new Entry());
      pDeviceOptionsTypeEntry =  manage( new Entry());
      // new
      pDeviceOptionsType2Entry =  manage(new Entry());
      pPdlLevelEntry =  manage(new Entry());
      pPdlSubLevelEntry = manage(new Entry());
      pPdlMajorEntry = manage(new Entry());
      pPdlMinorEntry = manage(new Entry());
      pCapabilityTypeEntry = manage(new Entry());
      pRasterCapabilityTypeEntry = manage(new Entry());

      pBlitterCppEntry = manage(new Entry());
      pBlitterHppEntry = manage(new Entry());
      pInstanceCppEntry = manage(new Entry());
      pInstanceHppEntry = manage(new Entry());

      pPluggableInstanceEntry = manage(new Entry());
      pPluggableBlitterEntry = manage(new Entry());

}

void GeneralPropertiesView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Device Name ", pDeviceNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Driver Name ", pDriverNameEntry) ,false,false);
   getVbox()->pack_start(constructPair("Capability Type", pCapabilityTypeEntry) ,false,false);
   getVbox()->pack_start(constructPair("Raster Capability Type", pRasterCapabilityTypeEntry) ,false,false);
   getVbox()->pack_start(constructPair("Device Options Type ", pDeviceOptionsTypeEntry) ,false,false);
   // new
   getVbox()->pack_start(constructPair("Device Options Type 2", pDeviceOptionsType2Entry) ,false,false);
   getVbox()->pack_start(constructPair("PDL Level", pPdlLevelEntry) ,false,false);
   getVbox()->pack_start(constructPair("PDL SubLevel ", pPdlSubLevelEntry) ,false,false);
   getVbox()->pack_start(constructPair("PDL Major ", pPdlMajorEntry) ,false,false);
   getVbox()->pack_start(constructPair("PDL Minor ", pPdlMinorEntry) ,false,false);
   getVbox()->pack_start(constructPair("Blitter [cpp] ", pBlitterCppEntry) ,false,false);
   getVbox()->pack_start(constructPair("Blitter [hpp] ", pBlitterHppEntry) ,false,false);
   getVbox()->pack_start(constructPair("Instance [cpp] ", pInstanceCppEntry) , false,false);
   getVbox()->pack_start(constructPair("Instance [hpp] ", pInstanceHppEntry) ,false,false);
   // new
   getVbox()->pack_start(constructPair("Pluggable Instance", pPluggableInstanceEntry) , false,false);
   getVbox()->pack_start(constructPair("Pluggable Blitter", pPluggableBlitterEntry) ,false,false);

   return ;
}


void GeneralPropertiesView::
refreshView()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   GeneralProperties * pDataNode2 = (GeneralProperties*)(this->getDataNode());

   // .............
   pDeviceNameEntry->set_text(pDataNode2->getDeviceName());
   pDriverNameEntry->set_text(pDataNode2->getDriverName());
   pDeviceOptionsTypeEntry->set_text(pDataNode2->getDeviceOptionsType());
   // new
   pDeviceOptionsType2Entry->set_text(pDataNode2->getDeviceOptionsType2());
   pPdlLevelEntry->set_text(pDataNode2->getPDLLevel());
   pPdlSubLevelEntry->set_text(pDataNode2->getPDLSubLevel());
   pPdlMajorEntry->set_text(pDataNode2->getPDLMajor());
   pPdlMinorEntry->set_text(pDataNode2->getPDLMinor());
   pCapabilityTypeEntry->set_text(pDataNode2->getCapabilityType());
   pRasterCapabilityTypeEntry->set_text(pDataNode2->getRasterCapabilityType());

   pBlitterCppEntry->set_text(pDataNode2->getBlitterCPP());
   pBlitterHppEntry->set_text(pDataNode2->getBlitterHPP());
   pInstanceCppEntry->set_text(pDataNode2->getInstanceCPP());
   pInstanceHppEntry->set_text(pDataNode2->getInstanceHPP());

   // new
   pPluggableInstanceEntry->set_text(pDataNode2->getPluggableInstance());
   pPluggableBlitterEntry->set_text(pDataNode2->getPluggableBlitter());
   return;
}

void GeneralPropertiesView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   GeneralProperties * pDataNode2 = (GeneralProperties*)(this->getDataNode());

   // .............

   pDataNode2->setDeviceName(pDeviceNameEntry->get_text());
   pDataNode2->setDriverName(pDriverNameEntry->get_text());
   pDataNode2->setDeviceOptionsType(pDeviceOptionsTypeEntry->get_text());
   // new
   pDataNode2->setDeviceOptionsType(pDeviceOptionsType2Entry->get_text());

   pDataNode2->setPDLLevel(pPdlLevelEntry->get_text());
   pDataNode2->setPDLSubLevel(pPdlSubLevelEntry->get_text());
   pDataNode2->setPDLMajor(pPdlMajorEntry->get_text());
   pDataNode2->setPDLMinor(pPdlMinorEntry->get_text());
   pDataNode2->setCapabilityType(pCapabilityTypeEntry->get_text());
   pDataNode2->setRasterCapabilityType(pRasterCapabilityTypeEntry->get_text());

   pDataNode2->setBlitterCPP(pBlitterCppEntry->get_text());
   pDataNode2->setBlitterHPP(pBlitterHppEntry->get_text());
   pDataNode2->setInstanceCPP(pInstanceCppEntry->get_text());
   pDataNode2->setInstanceHPP(pInstanceHppEntry->get_text());

   // new
   pDataNode2->setPluggableInstance(pPluggableInstanceEntry->get_text());
   pDataNode2->setPluggableBlitter(pPluggableBlitterEntry->get_text());

   return;
}
