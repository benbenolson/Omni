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

#include "OrientationPropertyView.hpp"
#include "OrientationProperty.hpp"

using namespace OmniDeviceCreationTool;
using namespace Gtk;


OrientationPropertyView* OrientationPropertyView::
getInstance()
{
   if(pUniqueInstance != NULL)
   {
      if (pPreviousInstance != NULL)
         delete pPreviousInstance;
      pPreviousInstance = pUniqueInstance;
   }

   pUniqueInstance = new OrientationPropertyView();
   OrientationPropertyView *pUniqueInstance2 = (OrientationPropertyView *)pUniqueInstance;
   pUniqueInstance2->initializeView();
   return pUniqueInstance2;
}

// Constructor
OrientationPropertyView::
OrientationPropertyView()
{
   pOrientationNameEntry = NULL;
}

void OrientationPropertyView::
createWidgets()
{
   if(NULL == pOrientationNameEntry)
      pOrientationNameEntry = manage(new Entry());

    return;
}

void OrientationPropertyView::
addWidgets()
{
   getVbox()->pack_start(constructPair("Orientation Name ", pOrientationNameEntry),
                                 false,
                                 false);

   return;
}

void OrientationPropertyView::
refreshView()
{
   if (this->getDataNode() == NULL)
      return;

   // SAFE Downcating  ...

   OrientationProperty *pDataNode2 = (OrientationProperty *)(this->getDataNode());

   pOrientationNameEntry->set_text(pDataNode2->getOrientationName());
   return;
}

void OrientationPropertyView::
refreshNode()
{
   if (this->getDataNode() == NULL)
      return;

   // SAFE Downcating ...

   OrientationProperty *pDataNode2 = (OrientationProperty *)(this->getDataNode());

   pDataNode2->setOrientationName(pOrientationNameEntry->get_text());

   return;
}
