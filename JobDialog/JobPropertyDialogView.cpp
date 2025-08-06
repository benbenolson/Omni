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
#include "JobPropertyDialogView.hpp"
#include "JobPropertyDialogController.hpp"

using namespace Gtk;
using namespace JobDialog;

JobPropertyDialogView::
JobPropertyDialogView (JobPropertyDialogController *pCtrl,
                       int                          cPropertyCount)
{
   pController_d         = pCtrl;
   iTotalPropertyCount_d = cPropertyCount;
   iAddedPropertyCount   = 0;

   pVbox_d               = manage (new VBox ());
   pScrWindow_d          = manage (new ScrolledWindow ());
   pTable_d              = manage (new Table (iTotalPropertyCount_d, 2));
   pFrame_d              = manage (new Frame ());
   pHbox_d               = manage (new HButtonBox ());

   pSaveButton_d         = manage (new Button ("Save"));
   pSaveNprintButton_d   = manage (new Button ("Save&Print"));
   pPrintButton_d        = manage (new Button ("Print"));
   pCancelButton_d       = manage (new Button ("Cancel"));

   createSkeleton ();
   setupConnections ();
}

bool JobPropertyDialogView::
addWidget (Widget *pWidget,
           string  displayName)
{
   if (iAddedPropertyCount >= iTotalPropertyCount_d)
      return false;

   Label *pDisplayNameLabel = manage (new Label (displayName));

   /***************************************************************/
   /* void Table::attach  (Widget& child,                         */
   /*                      guint left_attach,                     */
   /*                      guint right_attach,                    */
   /*                      guint top_attach,                      */
   /*                      guint bottom_attach,                   */
   /*                      gint xoptions=(GTK_FILL|GTK_EXPAND),   */
   /*                      gint yoptions=(GTK_FILL|GTK_EXPAND),   */
   /*                      guint xpadding=0, guint ypadding=0);   */
   /***************************************************************/
   pTable_d->attach (*pDisplayNameLabel,
                     0,
                     1,
                     iAddedPropertyCount,
                     iAddedPropertyCount + 1,
                     0,
                     0,
                     0,
                     0);

   pTable_d->attach (*pWidget,
                     1,
                     2,
                     iAddedPropertyCount,
                     iAddedPropertyCount + 1,
                     0,
                     0,
                     0,
                     0);

   iAddedPropertyCount++;

   return true;
}

void JobPropertyDialogView::
showDialog ()
{
   show_all ();
}

gint JobPropertyDialogView::
delete_event_impl (GdkEventAny* p0)
{
   // Just ignore explicit window closing.
   return true;
}

void JobPropertyDialogView::
createSkeleton ()
{
   add (*pVbox_d);

   pVbox_d->pack_start (*pScrWindow_d);
   pVbox_d->pack_start (*pFrame_d, false, false);

   pScrWindow_d->set_policy (GTK_POLICY_AUTOMATIC,
                             GTK_POLICY_AUTOMATIC);

   pScrWindow_d->add_with_viewport (*pTable_d);

   pTable_d->set_row_spacings (15);
   pTable_d->set_col_spacings (15);
   pTable_d->set_border_width (20);

   pFrame_d->add (*pHbox_d);

   pHbox_d->set_border_width (10);
   pHbox_d->set_layout (GTK_BUTTONBOX_END);
   pHbox_d->set_spacing (20);
   pHbox_d->set_child_size (60,25);

///pHbox_d->pack_start (*pSaveButton_d, false);
///pHbox_d->pack_start (*pSaveNprintButton_d, false);
   pHbox_d->pack_start (*pPrintButton_d, false);
   pHbox_d->pack_start (*pCancelButton_d, false);

   set_usize (450,400);
   set_modal (true);
}

void JobPropertyDialogView::
setupConnections()
{
   using SigC::slot;

   pSaveNprintButton_d->clicked.connect (slot (pController_d, &JobPropertyDialogController::onSaveNprintButtonClicked));
   pSaveButton_d->clicked.connect (slot (pController_d, &JobPropertyDialogController::onSaveButtonClicked));
   pPrintButton_d->clicked.connect (slot (pController_d, &JobPropertyDialogController::onPrintButtonClicked));
   pCancelButton_d->clicked.connect (slot (pController_d, &JobPropertyDialogController::onCancelButtonClicked));
}
