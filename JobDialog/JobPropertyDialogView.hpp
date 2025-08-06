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
#ifndef _COM_IBM_LINUX_OMNI_JOB_JobPropertyDialogView
#define _COM_IBM_LINUX_OMNI_JOB_JobPropertyDialogView

#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/table.h>
#include <gtk--/frame.h>
#include <gtk--/buttonbox.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/main.h>

using namespace Gtk;

namespace JobDialog
{

class JobPropertyDialogController;        // Forward declaration

class JobPropertyDialogView : public Window
{
public:
          JobPropertyDialogView (JobPropertyDialogController *pCtrl,
                                 int                          cPropertyCount);

   bool   addWidget             (Widget                      *pWidget,
                                 string                       displayName);
   void   showDialog            ();
   gint   delete_event_impl     (GdkEventAny                 *p0);
protected:
   void   createSkeleton        ();
   void   setupConnections      ();

protected:
   JobPropertyDialogController *pController_d;
   unsigned int                 iTotalPropertyCount_d;
   unsigned int                 iAddedPropertyCount;

   VBox                        *pVbox_d;
   ScrolledWindow              *pScrWindow_d;
   Table                       *pTable_d;
   Frame                       *pFrame_d;
   HButtonBox                  *pHbox_d;

   Button                      *pPrintButton_d;
   Button                      *pCancelButton_d;
   Button                      *pSaveButton_d;
   Button                      *pSaveNprintButton_d;

/***************************************/
/* +VBox                               */
/*  |--+ScrollWindow                   */
/*  |  +Table                          */
/*  |     +(Label &TextBox )*          */
/*  |--+Frame                          */
/*      +HButtonBox                    */
/*        +(Button)*                   */
/*                                     */
/***************************************/

};

};  //end of namespace.

#endif
