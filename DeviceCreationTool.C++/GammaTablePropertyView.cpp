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

#include "GammaTablePropertyView.hpp"
#include "GammaTableProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;


GammaTablePropertyView* GammaTablePropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }
   pUniqueInstance = new GammaTablePropertyView();
   GammaTablePropertyView *pUniqueInstance2 = (GammaTablePropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();

   return pUniqueInstance2;
}

GammaTablePropertyView::
GammaTablePropertyView()
{
   pGammaTableResolutionEntry = NULL;
   pGammaTableMediaEntry= NULL;
   pGammaTablePrintModeEntry = NULL;
   pGammaTableDitherCatagoryEntry = NULL;
   pGammaTableCGammaEntry = NULL;
   pGammaTableMGammaEntry = NULL;
   pGammaTableYGammaEntry  = NULL;
   pGammaTableKGammaEntry = NULL;
   pGammaTableCBiasEntry = NULL;
   pGammaTableMBiasEntry = NULL;
   pGammaTableYBiasEntry= NULL;
   pGammaTableKBiasEntry = NULL;
}

void GammaTablePropertyView::
createWidgets()
{

   pGammaTableResolutionEntry = manage(new Entry());
   pGammaTableMediaEntry  = manage(new Entry());
   pGammaTablePrintModeEntry = manage(new Entry());
   pGammaTableDitherCatagoryEntry = manage(new Entry());
   pGammaTableCGammaEntry = manage(new Entry());
   pGammaTableMGammaEntry = manage(new Entry());
   pGammaTableYGammaEntry  = manage(new Entry());
   pGammaTableKGammaEntry = manage(new Entry());
   pGammaTableCBiasEntry = manage(new Entry());
   pGammaTableMBiasEntry = manage(new Entry());
   pGammaTableYBiasEntry = manage(new Entry());
   pGammaTableKBiasEntry = manage(new Entry());

    return;
}

void GammaTablePropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("GammaTable Resolution ", pGammaTableResolutionEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable Media ", pGammaTableMediaEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable Print Mode ", pGammaTablePrintModeEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable Dither Catagory ",
                                                                  pGammaTableDitherCatagoryEntry) ,
                                                                  false,false);
   getVbox()->pack_start(constructPair("GammaTable C Gamma ", pGammaTableCGammaEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable M Gamma", pGammaTableMGammaEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable Y Gamma ", pGammaTableYGammaEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable K Gamma ", pGammaTableKGammaEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable C Bias ", pGammaTableCBiasEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable M Bias ", pGammaTableMBiasEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable Y Bias ", pGammaTableYBiasEntry) ,false,false);
   getVbox()->pack_start(constructPair("GammaTable K Bias ", pGammaTableKBiasEntry) ,false,false);


   return;
}

void GammaTablePropertyView::refreshView()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating needed ...

   GammaTableProperty *pDataNode2 = (GammaTableProperty *)(this->getDataNode());

   // .............

   pGammaTableResolutionEntry->set_text(pDataNode2->getGammaTableResolution());
   pGammaTableMediaEntry->set_text(pDataNode2->getGammaTableMedia()) ;
   pGammaTablePrintModeEntry->set_text(pDataNode2->getGammaTablePrintMode());
   pGammaTableDitherCatagoryEntry->set_text(pDataNode2->getGammaTableDitherCatagory());
   pGammaTableCGammaEntry->set_text(pDataNode2->getGammaTableCGamma());
   pGammaTableMGammaEntry->set_text(pDataNode2->getGammaTableMGamma());
   pGammaTableYGammaEntry->set_text(pDataNode2->getGammaTableYGamma());
   pGammaTableKGammaEntry->set_text(pDataNode2->getGammaTableKGamma());
   pGammaTableCBiasEntry->set_text(pDataNode2->getGammaTableCBias());
   pGammaTableMBiasEntry->set_text(pDataNode2->getGammaTableMBias());
   pGammaTableYBiasEntry->set_text(pDataNode2->getGammaTableYBias());
   pGammaTableKBiasEntry->set_text(pDataNode2->getGammaTableKBias());

   return;
}

void GammaTablePropertyView::refreshNode()
{
   if(this->getDataNode() == NULL)
      return;

   // SAFE Downcating needed ...

   GammaTableProperty *pDataNode2 = (GammaTableProperty *)(this->getDataNode());

   // .............

   pDataNode2->setGammaTableResolution(pGammaTableResolutionEntry->get_text());
   pDataNode2->setGammaTableMedia(pGammaTableMediaEntry->get_text()) ;
   pDataNode2->setGammaTablePrintMode(pGammaTablePrintModeEntry->get_text());
   pDataNode2->setGammaTableDitherCatagory(pGammaTableDitherCatagoryEntry->get_text());
   pDataNode2->setGammaTableCGamma(pGammaTableCGammaEntry->get_text());
   pDataNode2->setGammaTableMGamma(pGammaTableMGammaEntry->get_text());
   pDataNode2->setGammaTableYGamma(pGammaTableYGammaEntry->get_text());
   pDataNode2->setGammaTableKGamma(pGammaTableKGammaEntry->get_text());
   pDataNode2->setGammaTableCBias(pGammaTableCBiasEntry->get_text());
   pDataNode2->setGammaTableMBias(pGammaTableMBiasEntry->get_text());
   pDataNode2->setGammaTableYBias(pGammaTableYBiasEntry->get_text());
   pDataNode2->setGammaTableKBias(pGammaTableKBiasEntry->get_text());


   return;
}
