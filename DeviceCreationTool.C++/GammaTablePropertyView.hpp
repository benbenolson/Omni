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
#ifndef _COM_IBM_LTC_OMNI_GammaTablePropertyView_Hpp
#define _COM_IBM_LTC_OMNI_GammaTablePropertyView_Hpp

#include <gtk--/entry.h>

#include "GenericPropertyView.hpp"

using namespace Gtk;

namespace OmniDeviceCreationTool {

class GammaTablePropertyView : public GenericPropertyView
{
public:
   static GammaTablePropertyView* getInstance();

protected:
   GammaTablePropertyView();

   void createWidgets();
   void addWidgets();
   void refreshView();
   void refreshNode();

private:

   // Entry *pGammaTableNameEntry;
   Entry *pGammaTableResolutionEntry;
   Entry *pGammaTableMediaEntry;
   Entry *pGammaTablePrintModeEntry;
   Entry *pGammaTableDitherCatagoryEntry;
   Entry *pGammaTableCGammaEntry;
   Entry *pGammaTableMGammaEntry;
   Entry *pGammaTableYGammaEntry;
   Entry *pGammaTableKGammaEntry;
   Entry *pGammaTableCBiasEntry;
   Entry *pGammaTableMBiasEntry;
   Entry *pGammaTableYBiasEntry;
   Entry *pGammaTableKBiasEntry;
};

}; // end of namespace

#endif
