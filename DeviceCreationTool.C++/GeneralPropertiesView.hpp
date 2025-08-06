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
#ifndef _COM_IBM_LTC_OMNI_GeneralPropertiesView_Hpp
#define _COM_IBM_LTC_OMNI_GeneralPropertiesView_Hpp

#include <gtk--/entry.h>

#include "GenericPropertyView.hpp"

namespace OmniDeviceCreationTool {

class GeneralPropertiesView : public GenericPropertyView
{
public:

   static GeneralPropertiesView * getInstance();

protected:
   void createWidgets();
   void addWidgets();
   void refreshView();
   void refreshNode();
   void customizeView();

   GeneralPropertiesView();

private:
   Entry *pDeviceNameEntry;
   Entry *pDriverNameEntry;
   Entry *pDeviceOptionsTypeEntry;
   Entry *pDeviceOptionsType2Entry;
   Entry *pPdlLevelEntry;
   Entry *pPdlSubLevelEntry;
   Entry *pPdlMajorEntry;
   Entry *pPdlMinorEntry;
   Entry *pCapabilityTypeEntry;
   Entry *pRasterCapabilityTypeEntry;

   Entry *pBlitterCppEntry;
   Entry *pBlitterHppEntry;
   Entry *pInstanceCppEntry;
   Entry *pInstanceHppEntry;
   Entry *pPluggableInstanceEntry;
   Entry *pPluggableBlitterEntry;

};

}; // end of namespace

#endif
