/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#ifndef _COM_IBM_LINUX_OMNI_JOB_ComboBoxHandler_hpp_
#define _COM_IBM_LINUX_OMNI_JOB_ComboBoxHandler_hpp_	

#include "WidgetInterface.hpp"

#include <gtk--/combo.h>

using namespace Gtk;

namespace JobDialog
{

class ComboBoxHandler : public WidgetInterface
{
public:
   static ComboBoxHandler  *instance          ();

   // for WidgetInterface
   bool                     populateValues    (Widget         *pWidget,
                                               DriverProperty *pDriverProperty);
   string                   getSelectedValue  (Widget         *pWidget);

protected:
                            ComboBoxHandler   ();
   Combo*                   safeDowncast      (Widget         *pWidget);
		
private:	
   static ComboBoxHandler *pUniqueInstance;
};

};  // end of namespace JobDialog
#endif
