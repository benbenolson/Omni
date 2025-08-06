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
#ifndef _COM_IBM_LINUX_OMNI_JOB_WidgetInterface
#define _COM_IBM_LINUX_OMNI_JOB_WidgetInterface

#include "DriverProperty.hpp"
#include <gtk--/widget.h>

using namespace Gtk;

namespace JobDialog
{

class WidgetInterface
{
public:
   virtual bool    populateValues    (Widget         *pWidget,
                                      DriverProperty *pDriverProperty) = 0;
   virtual string  getSelectedValue  (Widget         *pWidget) = 0;
};

};  //end of namespace
#endif

