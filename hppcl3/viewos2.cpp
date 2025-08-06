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
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_ERRORS
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_DEV
#define INCL_DEVDJP
#define INCL_PM
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// TCP/IP includes
#include <types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

extern "C" {

// Common library
#include <mem.h>
#include <assrt.h>
#include <button.h>
#include <toolbar.h>

}

#define APP_NAME "Viewer App"

/* Globals...
*/
HWND            hwndFrameServer      = (HWND)NULL;
HWND            hwndClientServer     = (HWND)NULL;
CLASSINFO       fci;

/* Function prototypes...
*/
VOID             SetItemChecked         (HWND                hwnd,
                                         SHORT               menu_item,
                                         BOOL                fChecked);
MRESULT EXPENTRY ClientWndProc          (HWND                hwnd,
                                         ULONG               msg,
                                         MPARAM              mp1,
                                         MPARAM              mp2);
MRESULT EXPENTRY BitmapViewerFrameProc  (HWND                hwnd,
                                         ULONG               msg,
                                         MPARAM              mp1,
                                         MPARAM              mp2);
MRESULT EXPENTRY BitmapViewerClientProc (HWND                hwnd,
                                         ULONG               msg,
                                         MPARAM              mp1,
                                         MPARAM              mp2);
VOID             GetWindowLong          (HWND                hwnd,
                                         PLONG               plValue);
VOID             SetWindowLong          (HWND                hwnd,
                                         LONG                lValue);
VOID             SetIconInDialog        (HWND                hwndDlg,
                                         HPOINTER            hptrIcon,
                                         ULONG               ulIconID);
MRESULT EXPENTRY PrintDlgProc           (HWND                hwnd,
                                         USHORT              msg,
                                         MPARAM              mp1,
                                         MPARAM              mp2);
MRESULT EXPENTRY PrintSetupDlgProc      (HWND                hwnd,
                                         USHORT              msg,
                                         MPARAM              mp1,
                                         MPARAM              mp2);
ULONG            NumberOfControls       (HWND                hwnd);
VOID             SetUpScrollBars        (HWND                hwnd,
                                         PCHILDINFO          pSelf);

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID APIENTRY
SafeWinPostMsg (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   INT     iTries = 0;
   APIRET  rc;

   do
   {
      rc = WinPostMsg (hwnd, msg, mp1, mp2);

      if (!rc)
      {
         APIRET rc2;

         DBPRINTF (("WinPostMsg hwnd = %08X msg = %08X mp1 = %08X mp2 = %08X failed! Sleeping 250ms...\n",
                    hwnd, msg, mp1, mp2));

         rc2 = DosSleep (250);
         assertT (rc2);
      }

      iTries++;
   } while (!rc && 10 > iTries);

   if (!rc && 10 == iTries)
   {
      DBPRINTF (("WinPostMsg hwnd = %08X msg = %08X mp1 = %08X mp2 = %08X failed! *** Giving up! ***\n",
                 hwnd, msg, mp1, mp2));
      assertF (rc);
   }
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID
SetItemChecked (HWND hwnd, SHORT menu_item, BOOL fChecked)
{
   HWND    hwndMenu;

   hwndMenu = WinWindowFromID (hwnd, FID_MENU);
   WinSendMsg (hwndMenu,
               MM_SETITEMATTR,
               MPFROM2SHORT (menu_item, TRUE),
               MPFROM2SHORT (MIA_CHECKED,
                             (fChecked ? MIA_CHECKED : FALSE)));
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID
SetItemGray (HWND hwnd, SHORT menu_item, BOOL fGray)
{
   HWND    hwndMenu;

   DBPRINTF (("SetItemGray = hwnd %08X\n", hwnd));
   hwndMenu = WinWindowFromID (hwnd, FID_MENU);
   DBPRINTF (("SetItemGray = hwndMenu %08X\n", hwndMenu));
   WinSendMsg (hwndMenu,
               MM_SETITEMATTR,
               MPFROM2SHORT (menu_item, TRUE),
               MPFROM2SHORT (MIA_DISABLED,
                             (fGray ? MIA_DISABLED : FALSE)));
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID
PopupError (PSZ pszError, ...)
{
   CHAR         achText[500];
   va_list      list;

   va_start (list, pszError);
   vsprintf (achText, pszError, list);
   va_end (list);

   WinMessageBox (HWND_DESKTOP,
                  HWND_DESKTOP,
                  achText,
                  "Error",
                  0,
                  MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
int
main (int argc, char *argv[])
{
   APIRET       rc;
   HAB          hab;
   HMQ          hmq;
   QMSG         qmsg;
   CHAR         szClientClass[50];
   HHEAP        hHeap;
   ULONG        flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU    |
                               FCF_SIZEBORDER    | FCF_MINMAX     |
                               FCF_TASKLIST      | FCF_MENU       ;
   register INT i;

   HookAssert (HOOKTYPE_PMPRINTF);

   // Create a heap instance: 64 K start 4 MB total
   hHeap = CreateHeapInstance (64*1024, PROCESS_MEMORY, 4*16-1);
   assertF (hHeap);

   /* Initialize program for PM...
   */
   hab = WinInitialize (0);
   hmq = WinCreateMsgQueue (hab, 0);

   // Query the frame's class info
   rc = WinQueryClassInfo (hab, WC_FRAME, &fci);
   fci.flClassStyle &= ~CS_PUBLIC;               // Make it private

   /* Register our client function.
   */
   sprintf (szClientClass, "%s Client", APP_NAME);
   rc = WinRegisterClass (hab,
                          szClientClass,
                          (PFNWP)ClientWndProc,
                          CS_SIZEREDRAW
                          | CS_CLIPCHILDREN,
                          0);
   assertF (rc);

   // Tell the system that we do not want to listen to shutdown messages...
   rc = WinCancelShutdown (hmq, TRUE);
   assertF (rc);

   // Create the windows... we'll need hwndFrameServer in a bit
   hwndFrameServer = WinCreateStdWindow (HWND_DESKTOP,
                                         WS_VISIBLE | flFrameFlags,
                                         &flFrameFlags,
                                         szClientClass,
                                         (PSZ)APP_NAME,
                                         WS_VISIBLE,
                                         (HMODULE)NULL,
                                         ID_RESOURCE,
                                         &hwndClientServer);
   assertF (hwndFrameServer);
   assertF (hwndClientServer);

   /* Go into the message loop.
   */
   if (hwndFrameServer && hwndClientServer)
   {
      WinSetWindowPos (hwndFrameServer, NULLHANDLE, 0, 0, 400, 400,
                       SWP_MOVE | SWP_SIZE);
      while (WinGetMsg (hab, &qmsg, (HWND)NULL, 0, 0))
         WinDispatchMsg (hab, &qmsg);
   }

   /* Clean up PM stuff...
   */
   WinDestroyWindow (hwndFrameServer);
   WinDestroyMsgQueue (hmq);
   WinTerminate (hab);

   DeleteHeapInstance (hHeap);

   return 0;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
MRESULT EXPENTRY
ClientWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch (msg)
   {
   case WM_CREATE:
   {
      break;
   }

   case WM_CLOSE:
   {
      /* Since this client window owns all of the pipe thread windows,
      ** it is only fair that this window should have to clean up after
      ** itself...
      */
      return WinDefWindowProc (hwnd, msg, mp1, mp2);
   }

   case WM_COMMAND:
   {
      switch (SHORT1FROMMP (mp1))
      {
      case IDM_EXITMENU:
      {
         WinSendMsg (hwnd, WM_CLOSE, NULL, NULL);
         break;
      }
      }
      break;
   }

   case WM_PAINT:
   {
      HPS   hps;

      hps = WinBeginPaint (hwnd, (HPS)NULL, (PRECTL)NULL);
      GpiErase (hps);
      WinEndPaint (hps);
      break;
   }

   default:
      return WinDefWindowProc (hwnd, msg, mp1, mp2);
   }

   return (MRESULT)FALSE;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID FAR _System
CreateChildWindow (PCHILDINFO pSelf)
{
   CHAR         szTitle[50];
   HAB          habThread;
   HMQ          hmqThread;
   QMSG         qmsgThread;
   HWND         hwndFrame;
   HWND         hwndClient;
   FRAMECDATA   fcdata;
   SWP          swp;
   APIRET       rc;

   ReferenceChild (pSelf);

   sprintf (szTitle, "%s Client", APP_NAME);

   /* Initialize for PM...
   */
   habThread = WinInitialize (0);
   hmqThread = WinCreateMsgQueue (habThread, 0);

   fcdata.cb = sizeof (FRAMECDATA);
   fcdata.flCreateFlags = FCF_TITLEBAR      | FCF_SYSMENU    |
                          FCF_SIZEBORDER    | FCF_MINMAX     |
                          FCF_NOBYTEALIGN   | FCF_VERTSCROLL |
                          FCF_HORZSCROLL    ;

   fcdata.hmodResources = (HMODULE)NULL;
   fcdata.idResources   = ID_RESOURCE;

   // Query the frame's class info
   rc = WinQueryClassInfo (habThread, WC_FRAME, &fci);
   fci.flClassStyle &= ~CS_PUBLIC;               // Make it private

   rc = WinRegisterClass (habThread,
                          BITMAP_VIEWER_FRAME_CLASS,
                          (PFNWP)BitmapViewerFrameProc,
                          fci.flClassStyle,
                          fci.cbWindowData);
   assertF (rc);

   rc = WinRegisterClass (habThread,
                          BITMAP_VIEWER_CLASS,
                          (PFNWP)BitmapViewerClientProc,
                          CS_MOVENOTIFY
                          | CS_SIZEREDRAW
                          | CS_CLIPSIBLINGS,
                          sizeof (ULONG));
   assertF (rc);

   rc = WinRegisterClass (habThread,
                          BMPBUTTONCLASS,
                          (PFNWP)BmpButtonWndProc,
                          CS_SIZEREDRAW
                          | CS_MOVENOTIFY
                          | CS_HITTEST
                          | CS_CLIPSIBLINGS,
                          BMPBUTTONEXTRA);
   assertF (rc);

   /* Find out the size of the client.  We will set the new window to this
   ** size.
   */
   rc = WinQueryWindowPos (hwndClientServer, &swp);
   assertF (rc);
   DBPRINTF (("Client window pos: x = %d, y = %d, cx = %d, cy = %d\n",
               swp.x, swp.y, swp.cx, swp.cy));

   hwndFrame = WinCreateWindow (hwndClientServer,         // Parent
                                BITMAP_VIEWER_FRAME_CLASS,// Class
                                szTitle,                  // Title
                                0,                        // Style
                                0, 0, 0, 0,               // x, y, cx, cy
                                (HWND)NULL,
                                HWND_TOP,
                                100,
                                &fcdata,
                                NULL);
   assertF (hwndFrame);
   pSelf->hwndFrame = hwndFrame;

   hwndClient = WinCreateWindow (hwndFrame,
                                 BITMAP_VIEWER_CLASS,
                                 (PSZ)NULL,
                                 0,
                                 0, 0, 0, 0,
                                 hwndFrame,
                                 HWND_BOTTOM,
                                 FID_CLIENT,
                                 (PVOID)pSelf,
                                 NULL);
   assertF (hwndClient);

   // Notify the server that we are up and running
   rc = DosPostEventSem (pSelf->hevWindowStarted);
   assertT (rc);

   if (hwndFrame && hwndClient)
   {
      // Set our instance data
      rc = WinSetWindowULong (hwndClient, QWL_USER, (ULONG)pSelf);
      assertF (rc);

      WinSetWindowPos (hwndFrame,
                       (HWND)NULL,
                       0         ,
                       swp.cy / 2,
                       swp.cx / 2,
                       swp.cy / 2,
                       SWP_SHOW | SWP_MOVE | SWP_SIZE);

      while (WinGetMsg (habThread, &qmsgThread, (HWND)NULL, 0, 0))
         WinDispatchMsg (habThread, &qmsgThread);
   }

   /* Clean up PM stuff...
   */
   WinDestroyWindow (hwndFrame);
   WinDestroyMsgQueue (hmqThread);
   WinTerminate (habThread);

   rc = DosCloseEventSem (pSelf->hevWindowStarted);
   assertT (rc);

   DereferenceChild (pSelf);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
ULONG
NumberOfControls (HWND hwnd)
{
   HENUM   henum;
   ULONG   ulCount = 0;
   HWND    hwndRet;

   henum = WinBeginEnumWindows (hwnd);
   do
   {
      // Get the next window in the curent level...
      hwndRet = WinGetNextWindow (henum);
      if (hwndRet)
         ulCount++;
   } while (hwndRet);

   WinEndEnumWindows (henum);

   return ulCount;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
MRESULT EXPENTRY
BitmapViewerFrameProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   static WPOINT            wpt;
   APIRET                   rc;

   switch (msg)
   {
   case WM_CREATE:
   {
      HWND  hwndDialog;

      // Call the system's frame class window proc first!
      (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);

      // Find out the size of the dialog border
      WinSendMsg (hwnd, WM_QUERYBORDERSIZE, MPFROMP (&wpt), (MPARAM)NULL);

      // Create the dialog
      // The WS_VISIBLE flag should not be set!  In the frame formatting
      // code, this dialog will be changed to visible after its size has
      // been dynamically changed
      hwndDialog = LoadToolbarDlg (hwnd, hwnd, IDD_TOOLBAR);
      assertF (hwndDialog);
      break;
   }

   case WM_DESTROY:
   {
      HWND         hwndDialog;

      DBPRINTF (("BitmapViewerFrameProc WM_DESTROY\n"));
      /* Grab our extra dialog control
      ** ... and destroy it!
      */
      hwndDialog = WinWindowFromID (hwnd, IDD_TOOLBAR);
      assertF (hwndDialog);

      if (hwndDialog)
         WinSendMsg (hwndDialog, WM_CLOSE, (MPARAM)0, (MPARAM)0);

      // Call the system's frame class window proc
      (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);
      break;
   }

   case WM_QUERYFRAMECTLCOUNT:
   {
      // return the number of existing controls PLUS 1
      return (MRESULT)((LONG)(*fci.pfnWindowProc) (hwnd, msg, mp1, mp2) + 1);
   }

   case WM_FORMATFRAME:
   {
      register INT i;
      PSWP         aswp;
      SWP          swpClient;
      HWND         hwndDialog;
      BOOL         fWindowMinimized;
      LONG         lNumberOfSWPs;
      USHORT       usID;
      SIZEL        sizelFrame;
      INT          iPosSysMenu  = -1;
      INT          iPosTitleBar = -1;
      INT          iPosMinMax   = -1;
      INT          iPosClient   = -1;
      INT          iPosHScroll  = -1;
      INT          iPosVScroll  = -1;
      LONG         lNumButtons;
      LONG         lXMaxButtonSize;
      LONG         lYMaxButtonSize;
      INT          cy;

      // First, call the original frame window
      lNumberOfSWPs = (LONG)(*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);
      aswp = (PSWP)mp1;

      DBPRINTF (("WM_FORMATFRAME\n"));

      // Query the size of the frame (use swpClient temporarily)
      WinQueryWindowPos (hwnd, &swpClient);
      sizelFrame.cx = swpClient.cx;
      sizelFrame.cy = swpClient.cy;

//////DEBUG_PRINT_SWPS (aswp, lNumberOfSWPs);

      // Find out the indicies of everyone
      for (i = 0; i < lNumberOfSWPs; i++)
      {
         // Who is this?
         usID = WinQueryWindowUShort (aswp[i].hwnd, QWS_ID);
         switch (usID)
         {
         case FID_SYSMENU:
            iPosSysMenu = i;
            break;
         case FID_TITLEBAR:
            iPosTitleBar = i;
            break;
         case FID_MINMAX:
            iPosMinMax = i;
            break;
         case FID_HORZSCROLL:
            iPosHScroll = i;
            break;
         case FID_VERTSCROLL:
            iPosVScroll = i;
            break;
         case FID_CLIENT:
            iPosClient = i;
            break;
         }
      }

      // FID_SYSMENU    ..
      iPosSysMenu = iPosSysMenu;      // Shut up compiler

      // FID_MINMAX     ..
      iPosMinMax = iPosMinMax;        // Shut up compiler

      // FID_HORZSCROLL ..
      iPosHScroll = iPosHScroll;      // Shut up compiler

      // FID_VERTSCROLL ..
      iPosVScroll = iPosVScroll;      // Shut up compiler

      // FID_TITLEBAR   ..

      /* Calculate the size and position of the dialog.
      ** The default frame procedure has already filled out
      ** the array of windows.  The last window is FID_CLIENT
      ** (which is at iPosClient position).  So, insert our
      ** window (toolbar) here and put FID_CLIENT after it.
      */
      // Save the client structure
      swpClient = aswp[iPosClient];

      // Get the toolbar dialog hwnd
      hwndDialog = WinWindowFromID (hwnd, IDD_TOOLBAR);
      assertF (hwndDialog);

      fWindowMinimized = aswp[0].fl & SWP_HIDE;
      PRINT_VAR (fWindowMinimized);

      // How many buttons are there?
      lNumButtons = NumberOfControls (hwndDialog);
      PRINT_VAR (lNumButtons);

{
   CHAR   achClassName[50];

PRINT_VARt (aswp[iPosClient+0].hwnd, 08X);

   rc = WinQueryClassName (aswp[iPosClient+0].hwnd,
                           sizeof (achClassName),
                           achClassName);
   assertF (rc);
   PRINT_VARt (achClassName, s);

PRINT_VARt (hwndDialog, 08X);

   rc = WinQueryClassName (hwndDialog,
                           sizeof (achClassName),
                           achClassName);
   assertF (rc);
   PRINT_VARt (achClassName, s);
}

      // Fill in toolbar's info
      aswp[iPosClient+0].fl                  = SWP_MOVE | SWP_SIZE;
      if (fWindowMinimized)
      {
         aswp[iPosClient+0].fl              |= SWP_HIDE;
         aswp[iPosClient+0].x                = 0;
         aswp[iPosClient+0].y                = 0;
         aswp[iPosClient+0].cx               = 0;
         aswp[iPosClient+0].cy               = 0;
      }
      else
      {
         // Find out the maximum button size
         MaxButtonSize (hwndDialog, &lXMaxButtonSize, &lYMaxButtonSize);
         PRINT_VAR (lXMaxButtonSize);
         PRINT_VAR (lYMaxButtonSize);

         // Find out how many buttons can fit in client area
         cy = swpClient.cy / lYMaxButtonSize;
         cy = (lNumButtons + cy - 1)/ cy;

         aswp[iPosClient+0].fl              |= SWP_SHOW;
         aswp[iPosClient+0].x                = wpt.x;
         aswp[iPosClient+0].y                = wpt.y;
         aswp[iPosClient+0].cx               = lXMaxButtonSize*cy;
PRINT_VAR (sizelFrame.cy);
PRINT_VAR (wpt.y);
         aswp[iPosClient+0].cy               = sizelFrame.cy       -
                                               2*wpt.y             -
                                               aswp[iPosTitleBar].cy;

         if (0 <= iPosHScroll)
         {
            // Move dialog above the horizontal scroll bar
            aswp[iPosClient+0].y  += aswp[iPosHScroll].cy;
            aswp[iPosClient+0].cy -= aswp[iPosHScroll].cy;

            aswp[iPosClient+0].y--;  // BUG BUG BUG -- WHY? WHY? WHY?
            aswp[iPosClient+0].cy++; // BUG BUG BUG -- WHY? WHY? WHY?
         }
      }
      aswp[iPosClient+0].hwnd                = hwndDialog;
      aswp[iPosClient+0].hwndInsertBehind    = NULLHANDLE;
      aswp[iPosClient+0].ulReserved1         = 0;
      aswp[iPosClient+0].ulReserved2         = 0;

      // Place FID_CLIENT after our controls
      // First, restore the old client
      aswp[iPosClient+1] = swpClient;
      // Now, change FID_CLIENT
      aswp[iPosClient+1].fl                  = SWP_MOVE | SWP_SIZE;
      if (fWindowMinimized)
      {
         aswp[iPosClient+1].fl              |= SWP_HIDE;
         aswp[iPosClient+1].x                = wpt.x;
         aswp[iPosClient+1].y                = wpt.y;
         aswp[iPosClient+1].cx               = 0;
         aswp[iPosClient+1].cy               = 0;
      }
      else
      {
         aswp[iPosClient+1].fl              |= SWP_SHOW;
         aswp[iPosClient+1].x                = aswp[iPosClient+0].x +
                                               aswp[iPosClient+0].cx;
         aswp[iPosClient+1].cx               = sizelFrame.cx        -
                                               2*wpt.x              -
                                               aswp[iPosClient+0].cx;

         if (0 <= iPosVScroll)
         {
            aswp[iPosClient+1].cx -= aswp[iPosVScroll].cx - 1;
         }
      }

      // Account for the added controls
      lNumberOfSWPs += 1;

//////DEBUG_PRINT_SWPS (aswp, lNumberOfSWPs);

      // Return the number of frame controls that were formatted
      return (MPARAM)lNumberOfSWPs;
   }

   /* Here, we can control how large or small we can get...
   */
   case WM_QUERYTRACKINFO:
   {
      PTRACKINFO   pTrack;
      LONG         lXMaxButtonSize;
      LONG         lYMaxButtonSize;
      SWP          swpTitleBar;
      HWND         hwndDialog;

      // Let the original frame window fill in info
      (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);

      // Get the toolbar dialog hwnd
      hwndDialog = WinWindowFromID (hwnd, IDD_TOOLBAR);
      assertF (hwndDialog);

      // Find out the maximum button size
      MaxButtonSize (hwndDialog, &lXMaxButtonSize, &lYMaxButtonSize);

      // Find out the title bar's size
      WinQueryWindowPos (WinWindowFromID (hwnd, FID_TITLEBAR), &swpTitleBar);

      pTrack = (PTRACKINFO)mp2;
      pTrack->ptlMinTrackSize.x = lXMaxButtonSize + 10 + 2*wpt.x;
      pTrack->ptlMinTrackSize.y = lYMaxButtonSize + swpTitleBar.cy + 2*wpt.y;

      return (MRESULT)TRUE;
   }

   case WM_WINDOWPOSCHANGED:
   {
      PCHILDINFO  pSelf;
      HWND        hwndClient;
      MRESULT     ret;

      // First, call the original frame window
      ret = (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);

      // Next, save the window size of the client
      hwndClient = WinWindowFromID (hwnd, FID_CLIENT);
      if (hwndClient)
      {
         pSelf = (PCHILDINFO)WinQueryWindowULong (hwndClient, QWL_USER);
         assertF (pSelf);

         if (pSelf)
            WinQueryWindowPos (hwndClient, &pSelf->swp);
      }
      return ret;
   }

   case WM_TRACKFRAME:
   {
      USHORT   usFlags = SHORT1FROMMP (mp1);
      MRESULT  ret;

      // First, call the original frame window
      ret = (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);

      if (usFlags & TF_LEFT)
         DBPRINTF ((" TF_LEFT"));
      if (usFlags & TF_TOP)
         DBPRINTF ((" TF_TOP"));
      if (usFlags & TF_RIGHT)
         DBPRINTF ((" TF_RIGHT"));
      if (usFlags & TF_BOTTOM)
         DBPRINTF ((" TF_BOTTOM"));
      DBPRINTF (("\n"));

      return ret;
   }

   default:
      // Call the original frame's window handler!
      return (*fci.pfnWindowProc) (hwnd, msg, mp1, mp2);
   }

   return (MRESULT)FALSE;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
MRESULT EXPENTRY
BitmapViewerClientProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   static HPOINTER    hptrMousePtr       = (HPOINTER)NULL;
   static BOOL        fMouseScrollActive = FALSE;
   static POINTL      ptlMousePos;
   APIRET             rc;
   PCHILDINFO         pSelf        = NULL;

   switch (msg)
   {
   case WM_CREATE:
   {
      HWND   hwndHScroll;
      HWND   hwndVScroll;

      DBPRINTF (("%s: WM_CREATE pSelf = %08X\n", __FUNCTION__, mp1));
      pSelf = (PCHILDINFO)mp1;
      pSelf->hwndClient = hwnd;

      // Create the scroll bars but dont do anything with them yet
      hwndHScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT), FID_HORZSCROLL);
      hwndVScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT), FID_VERTSCROLL);

      WinEnableWindow (hwndHScroll, FALSE);
      WinEnableWindow (hwndVScroll, FALSE);

      // @TBD - need a way to disable windows until bitmap is done loading
#if 0
      WinEnableWindow (WinWindowFromID (WinWindowFromID (hwnd, IDD_TOOLBAR),
                                        IDB_SAVE),
                       FALSE);
      WinEnableWindow (WinWindowFromID (WinWindowFromID (hwnd, IDD_TOOLBAR),
                                        IDB_TRANSFER),
                       FALSE);
#endif

      // Load the special mouse pointer
      hptrMousePtr = WinLoadPointer (HWND_DESKTOP, (HMODULE)0, IDIC_CROSSHR);
      assertF (hptrMousePtr);
      break;
   }

   case WM_DESTROY:
   {
      DBPRINTF (("%s: WM_DESTROY\n", __FUNCTION__));

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      if (hptrMousePtr)
      {
         rc = WinDestroyPointer (hptrMousePtr);
         assertF (rc);

         hptrMousePtr = (HPOINTER)NULL;
      }
      break;
   }

   case WM_BUTTON2DOWN:
   {
      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);
      if (!pSelf)
         break;

      fMouseScrollActive = TRUE;

      // Get the current position of the pointer
      rc = WinQueryPointerPos (HWND_DESKTOP, &ptlMousePos);
      assertF (rc);
      rc = WinMapWindowPoints (HWND_DESKTOP, hwnd, &ptlMousePos, 1);
      assertF (rc);

      /* Capture the mouse pointer so that we will get WM_MOUSEMOVE
      ** messages regardless where the mouse goes...
      */
      rc = WinSetCapture (HWND_DESKTOP, hwnd);
      assertF (rc);
      break;
   }

   case WM_BUTTON2UP:
   {
      fMouseScrollActive = FALSE;

      // Release the mouse
      rc = WinSetCapture (HWND_DESKTOP, (HWND)NULL);
      assertF (rc);
      break;
   }

   case WM_MOUSEMOVE:
   {
      SHORT        x = SHORT1FROMMP (mp1),
                   y = SHORT2FROMMP (mp1);

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      if (fMouseScrollActive)
      {
         LONG  lDeltaX,   lDeltaY;
         LONG  lMaxX,     lMaxY;
         LONG  lMinY;
         LONG  lBitmapCX, lBitmapCY;
         SWP   swp;

         if (!pSelf || !pSelf->pBitmap)
            break;

         lDeltaX = x - ptlMousePos.x;
         lDeltaY = y - ptlMousePos.y;
         DBPRINTF (("moveto (%d, %d) delta (%d, %d)\n", x, y, lDeltaX, lDeltaY));

         lBitmapCX = pSelf->pBitmap->cx ();
         lBitmapCY = pSelf->pBitmap->cy ();

         // Find size of the client
         WinQueryWindowPos (hwnd, &swp);

         lMaxX = lBitmapCX - swp.cx;
         lMaxY = lBitmapCY - swp.cy - 1;
         lMinY = 0;

         if (1 < pSelf->iZoom)
         {
            lMinY -= swp.cy - swp.cy / pSelf->iZoom;
            lMaxX += swp.cx - swp.cx / pSelf->iZoom;
         }

         // min
         if (0 > lDeltaX + pSelf->ptlOrigin.x)
            lDeltaX = 0;
         if (lMinY > lDeltaY + pSelf->ptlOrigin.y)
            lDeltaY = 0;

         // max
         if (lMaxX < lDeltaX + pSelf->ptlOrigin.x)
            lDeltaX = 0;
         if (lMaxY < lDeltaY + pSelf->ptlOrigin.y)
            lDeltaY = 0;

         DBPRINTF (("lMax (%d, %d), lDelta (%d, %d)\n",
                    lMaxX, lMaxY, lDeltaX, lDeltaY));
         pSelf->ptlOrigin.x += lDeltaX;
         pSelf->ptlOrigin.y += lDeltaY;

         ptlMousePos.x = x;
         ptlMousePos.y = y;

         // Force a repaint
         WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);

         WinSendMsg (WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                     FID_HORZSCROLL),
                     SBM_SETPOS,
                     MPFROMSHORT (pSelf->ptlOrigin.x),
                     (MPARAM)0);
         WinSendMsg (WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                     FID_VERTSCROLL),
                     SBM_SETPOS,
                     MPFROMSHORT (lMaxY - pSelf->ptlOrigin.y),
                     (MPARAM)0);

         /* Because we hook out mouse move, we need to set the default
         ** pointer
         */
         WinSetPointer (HWND_DESKTOP,
                        WinQuerySysPointer (HWND_DESKTOP,
                                            SPTR_ARROW,
                                            FALSE));
      }
      else
      {
         /* Because we hook out mouse move, we need to set the default
         ** pointer
         */
         WinSetPointer (HWND_DESKTOP,
                        WinQuerySysPointer (HWND_DESKTOP,
                                            SPTR_ARROW,
                                            FALSE));
      }
      break;
   }

   case WM_PAINT:
   {
      HPS    hps;
      RECTL  rectl;

      hps = WinBeginPaint (hwnd, (HPS)NULL, &rectl);
      assertF (hps);

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);

      // Do we have instance data and a bitmap to display?
      if (pSelf && pSelf->pBitmap)
      {
         POINTL   aptlPoints[4];
         SWP      swp;
         INT      iZoom      = pSelf->iZoom;
         INT      iXOffset   = pSelf->ptlOrigin.x;
         INT      iYOffset   = pSelf->ptlOrigin.y;
         INT      iXTarget;
         INT      iYTarget;
         LONG     lBitmapCX  = pSelf->pBitmap->cx ();
         LONG     lBitmapCY  = pSelf->pBitmap->cy ();
         INT      iSideSrcX,
                  iSideSrcY,
                  iSideTrgX,
                  iSideTrgY;

         // Find out the client size
         WinQueryWindowPos (hwnd, &swp);

         // Grow the bitmap by the zoom factor
         lBitmapCX *= iZoom;
         lBitmapCY *= iZoom;

         // Take the smaller of the window size or bitmap size
         iSideTrgX = omni::min (lBitmapCX, swp.cx);
         iSideTrgY = omni::min (lBitmapCY, swp.cy);

         /* Calculate the source's rectangle based on the zooming factor
         ** bump up the source size to a multiple of the zoom factor.
         */
         iSideSrcX = (iSideTrgX + iZoom - 1) / iZoom;
         iSideSrcY = (iSideTrgY + iZoom - 1) / iZoom;

         /* Recalculate the target's rectangle so there is an even
         ** sized aspect ratio x:x
         */
         iSideTrgX = iSideSrcX * iZoom;
         iSideTrgY = iSideSrcY * iZoom;

         if (1 < iZoom)
            // Offset origin to account for inverse y scrolling
            iYOffset += swp.cy - swp.cy / iZoom;

         // Offset the bitmap if it is smaller than the window
         iXTarget = 0;
         iYTarget = 0;

         if (lBitmapCX < swp.cx || lBitmapCY < swp.cy)
         {
            POINTL   ptl;

            /* Fill in background with some appeasing pattern :)
            */
            rc = GpiSetPattern (hps, PATSYM_DENSE8);
            assertF (rc);

            rc = GpiSetBackMix (hps, BM_OVERPAINT);
            assertF (rc);

            ptl.x = -1;
            ptl.y = -1;
            rc = GpiMove (hps, &ptl);
            assertF (rc);

            ptl.x = swp.cx;
            ptl.y = swp.cy;

            // Fill interior only
            rc = GpiBox (hps, DRO_FILL, &ptl, 0, 0);
            assertF (rc);

            rc = GpiSetPattern (hps, PATSYM_SOLID);
            assertF (rc);

            // @TBD - investigate -- buggy!
            PRINT_VAR (lBitmapCX);
            PRINT_VAR (swp.cx);
            PRINT_VAR (lBitmapCX);
            PRINT_VAR (lBitmapCY);
            PRINT_VAR (swp.cy);
            PRINT_VAR (lBitmapCY);

            // Offset target if necessary
            if (lBitmapCX < swp.cx)
               iXTarget = (swp.cx - lBitmapCX) / 2;
            if (lBitmapCY < swp.cy)
               iYTarget = (swp.cy - lBitmapCY) / 2;
            PRINT_VAR (iXTarget);
            PRINT_VAR (iYTarget);
         }

         // Is this a monochrome bitmap?
         if (1 == pSelf->pBitmap->BitCount ())
         {
            IMAGEBUNDLE   iBundle;

#define RGBTOLONG(r) ((*(PLONG)&(r)) & 0xFFFFFF)

            /* Monochrome bitmaps are special cases (though why I don't
            ** understand).  The color table is not used.  Ones in the image
            ** data are displayed with the foreground color and zeroes are
            ** displayed with the background color.  So, we must change the
            ** image primitive fore/background colors to match the bitmap's
            ** colors.
            */

            // Go into RGB color mode!
            rc = GpiCreateLogColorTable (hps,
                                         LCOL_RESET,
                                         LCOLF_RGB,
                                         0L,
                                         0L,
                                         (PLONG)NULL);
            assertF (rc);

            // Set up the image bundle
            iBundle.lColor     = pSelf->pBitmap->ColorTableIndexLong (1);
            iBundle.lBackColor = pSelf->pBitmap->ColorTableIndexLong (0);
            rc = GpiSetAttrs (hps,
                              PRIM_IMAGE,
                              IBB_COLOR | IBB_BACK_COLOR,    // which to set
                              0,                             // which to default
                              &iBundle);
            assertF (rc);
         }

         // Set of the bitblt source and destination points
         aptlPoints[0].x = iXTarget;                   // LL Target
         aptlPoints[0].y = iYTarget;                   //
         aptlPoints[1].x = iSideTrgX + iXTarget;       // UR Target
         aptlPoints[1].y = iSideTrgY + iYTarget;       //    (non-inclusive)
         aptlPoints[2].x = 0;                          // LL Source
         aptlPoints[2].y = 0;                          //
         aptlPoints[3].x = iSideSrcX;                  // UR Source
         aptlPoints[3].y = iSideSrcY;                  //    (non-inclusive)

         aptlPoints[2].x += iXOffset;
         aptlPoints[2].y += iYOffset;
         aptlPoints[3].x += iXOffset;
         aptlPoints[3].y += iYOffset;

         DBPRINTF (("window size (%d,%d)\n", swp.cx, swp.cy));
         DBPRINTF (("Orgn (%d,%d), Zoom-factor = %d\n",
                    pSelf->ptlOrigin.x, pSelf->ptlOrigin.y,
                    iZoom));
         DBPRINTF (("T:(%d,%d)-(%d,%d) S:(%d,%d)-(%d,%d)\n",
                   aptlPoints[0].x, aptlPoints[0].y,
                   aptlPoints[1].x, aptlPoints[1].y,
                   aptlPoints[2].x, aptlPoints[2].y,
                   aptlPoints[3].x, aptlPoints[3].y));

         // Blt it!
         pSelf->pBitmap->BitBlt (hps,
                                 4,
                                 aptlPoints,
                                 ROP_SRCCOPY,
                                 BBO_IGNORE);
      }
      else
      {
         // Erase the screen
         rc = WinFillRect (hps, &rectl, CLR_BACKGROUND);
         assertF (rc);
      }

      rc = WinEndPaint (hps);
      assertF (rc);
      break;
   }

   case WM_SIZE:
   {
      SWP   swp;

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      if (pSelf && pSelf->pBitmap)
      {
         LONG     lBitmapCY  = pSelf->pBitmap->cy ();

         lBitmapCY *= pSelf->iZoom;

         rc = WinQueryWindowPos (hwnd, &swp);

         pSelf->ptlOrigin.x = 0;
         if (lBitmapCY < swp.cy)
            pSelf->ptlOrigin.y = 0;
         else
            pSelf->ptlOrigin.y = lBitmapCY - swp.cy;

         SetUpScrollBars (hwnd, pSelf);
      }
      break;
   }

   case WM_HSCROLL:
   {
      LONG   lHscrollInc, lHscrollMax, lHscrollPos;
      HWND   hwndHScroll;

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      hwndHScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                     FID_HORZSCROLL);
      lHscrollInc = 0;
      lHscrollMax = pSelf->lHscrollMax;

      // Query our current position
      lHscrollPos = (LONG)WinSendMsg (hwndHScroll,
                                      SBM_QUERYPOS,
                                      (MPARAM)NULL, (MPARAM)NULL);

      switch (SHORT2FROMMP (mp2))
      {
      case SB_LINELEFT:
           lHscrollInc = -1;
           break;
      case SB_LINERIGHT:
           lHscrollInc = 1;
           break;
      case SB_PAGELEFT:
           lHscrollInc = omni::max (-pSelf->swp.cx, -(lHscrollPos - 1));
           break;
      case SB_PAGERIGHT:
           lHscrollInc = omni::min (pSelf->swp.cx, lHscrollMax - lHscrollPos);
           break;
      case SB_SLIDERTRACK:
           lHscrollInc = SHORT1FROMMP (mp2) - lHscrollPos;
           break;
      }

      if (lHscrollInc != 0)
      {
         lHscrollPos += lHscrollInc;
         pSelf->ptlOrigin.x += lHscrollInc;
         lHscrollInc *= pSelf->iZoom;

         // Force a repaint
         WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);

         // Update the slider with the current position
         WinSendMsg (hwndHScroll,
                     SBM_SETPOS,
                     MPFROMSHORT (lHscrollPos), NULL);
      }
      break;
   }

   case WM_VSCROLL:
   {
      LONG   lVscrollInc, lVscrollMax, lVscrollPos;
      HWND   hwndVScroll;

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      hwndVScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                     FID_VERTSCROLL);

      lVscrollInc = 0;
      lVscrollMax = pSelf->lVscrollMax;

      // Query our current position
      lVscrollPos = (LONG)WinSendMsg (hwndVScroll,
                                      SBM_QUERYPOS,
                                      (MPARAM)NULL, (MPARAM)NULL);

      switch (SHORT2FROMMP (mp2))
      {
      case SB_LINEUP:
           lVscrollInc = -1;
           break;
      case SB_LINEDOWN:
           lVscrollInc = 1;
           break;
      case SB_PAGEUP:
           lVscrollInc = omni::max (-pSelf->swp.cy, -(lVscrollPos - 1));
           break;
      case SB_PAGEDOWN:
           lVscrollInc = omni::min (pSelf->swp.cy, lVscrollMax - lVscrollPos);
           break;
      case SB_SLIDERTRACK:
           lVscrollInc = SHORT1FROMMP (mp2) - lVscrollPos;
           break;
      }

      if (lVscrollInc != 0)
      {
         lVscrollPos += lVscrollInc;

         // Flip to show origin at UL corner
         lVscrollInc *= -1;

         pSelf->ptlOrigin.y += lVscrollInc;
         lVscrollInc *= pSelf->iZoom;

         // Force a repaint
         WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);

         // Update slider with new info...
         WinSendMsg (hwndVScroll,
                     SBM_SETPOS,
                     MPFROMSHORT (lVscrollPos), NULL);
      }
      break;
   }

   case WM_COMMAND:
   {
      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      switch (SHORT1FROMMP (mp1))
      {
      case IDB_ZOOMIN:
      {
         pSelf->iZoom++;

         SetUpScrollBars (hwnd, pSelf);

         WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);
         break;
      }

      case IDB_ZOOMOUT:
      {
         LONG  lMinY;
         LONG  lMaxX;
         SWP   swp;

         // Find size of the client
         WinQueryWindowPos (hwnd, &swp);

         if (1 < pSelf->iZoom)
            pSelf->iZoom--;
         else
            break;

         if (!pSelf->pBitmap)
            break;

         // Make sure that the window stays within the bitmap
         lMaxX = pSelf->pBitmap->cx () - swp.cx;
         lMinY = 0;
         if (1 < pSelf->iZoom)
         {
            lMinY -= swp.cy - swp.cy / pSelf->iZoom;
            lMaxX += swp.cx - swp.cx / pSelf->iZoom;
         }

         if (lMaxX > pSelf->ptlOrigin.y)
            pSelf->ptlOrigin.x = lMaxX;
         if (lMinY > pSelf->ptlOrigin.y)
            pSelf->ptlOrigin.y = lMinY;

         SetUpScrollBars (hwnd, pSelf);

         WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);
         break;
      }
      }
      break;
   }

   case WM_TIMER:
   {
      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      // Stop the timer!
      WinStopTimer (WinQueryAnchorBlock (hwnd),
                    hwnd,
                    TID_USERMAX-2);

      SafeWinPostMsg (pSelf->hwndFrame,
                      WM_CLOSE,
                      (MPARAM)NULL,
                      (MPARAM)NULL);
      break;
   }

   case WM_USER_LOAD_DONE:
   {
      RECTL    rcl;

      DBPRINTF (("%s WM_USER_LOAD_DONE\n", __FUNCTION__));

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      // @TBD - enable save bitmap & transfer bitmap buttons

      if (vfFullSizeWhenLoaded && pSelf->pBitmap)
      {
         // Size the frame to contain the entire picture
         rcl.xLeft   = 0;
         rcl.yBottom = 0;
         rcl.xRight  = pSelf->pBitmap->cx ();
         rcl.yTop    = pSelf->pBitmap->cy ();

         rc = WinCalcFrameRect (pSelf->hwndFrame,
                                &rcl,
                                FALSE);

         rc = WinSetWindowPos (pSelf->hwndFrame,
                               (HWND)NULL,
                               0,
                               0,
                               rcl.xRight - rcl.xLeft,
                               rcl.yTop - rcl.yBottom,
                               SWP_MOVE | SWP_SIZE);
      }
      break;
   }

   case WM_USER_INVALIDATE_WINDOW:
   {
      SWP   swp;

      // Grab our instance data
      pSelf = (PCHILDINFO)WinQueryWindowULong (hwnd, QWL_USER);
      assertF (pSelf);

      if (pSelf && pSelf->pBitmap)
      {
         rc = WinQueryWindowPos (hwnd, &swp);

         pSelf->ptlOrigin.x = 0;
         pSelf->ptlOrigin.y = pSelf->pBitmap->cy () - swp.cy;

         SetUpScrollBars (hwnd, pSelf);
      }

      WinInvalidateRect (hwnd, (PRECTL)NULL, TRUE);
      break;
   }

   case WM_USER_SAVE_FILE_DONE:
   {
      DBPRINTF (("%s WM_USER_SAVE_FILE_DONE\n", __FUNCTION__));
      break;
   }

   case WM_USER_TRANSFER_BITMAP_DONE:
   {
      DBPRINTF (("%s WM_USER_TRANSFER_BITMAP_DONE\n", __FUNCTION__));
      break;
   }

   default:
      return WinDefWindowProc (hwnd, msg, mp1, mp2);
   }

   return FALSE;
}

VOID
SetWindowLong (HWND hwnd, LONG lValue)
{
   CHAR    achValue[25];

   sprintf (achValue, "%ld", lValue);
   WinSetWindowText (hwnd, achValue);
}

VOID
GetWindowLong (HWND hwnd, PLONG plValue)
{
   CHAR    achValue[25];

   WinQueryWindowText (hwnd, sizeof (achValue), achValue);
   *plValue = atoi (achValue);
}

VOID
SetIconInDialog (HWND hwndDlg, HPOINTER hptrIcon, ULONG ulIconID)
{
   HWND          hwndIcon;        // handle to Icon control
   HPS           hpsIcon;         // handle to PS for Icon
   RECTL         rclIcon;

   // Retrieve window handle of icon to be (re)set
   hwndIcon = WinWindowFromID (hwndDlg, ulIconID);

   // Query rectangle that contains the icon to be (re)set
   WinQueryWindowRect (hwndIcon, &rclIcon);

   // Retrieve the Presentation Space for static icon window
   hpsIcon  = WinGetPS (hwndIcon);

   /* Erase any previous icon's image from window as our
   ** new icon may not completely cover previous one
   */
   WinFillRect (hpsIcon, &rclIcon, SYSCLR_DIALOGBACKGROUND);

   WinSendDlgItemMsg (hwndDlg,
                      ulIconID,
                      SM_SETHANDLE,
                      MPFROMLONG (hptrIcon),
                      MPVOID);

   // release pres. space for icon since we are finished drawing
   WinReleasePS (hpsIcon);
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/
VOID
SetUpScrollBars (HWND hwnd, PCHILDINFO pSelf)
{
   HWND     hwndHScroll;
   HWND     hwndVScroll;
   SWP      swp;
   LONG     lBitmapCX;
   LONG     lBitmapCY;
   BOOL     fNeedHScroll;
   BOOL     fNeedVScroll;

   if (!pSelf || !pSelf->pBitmap)
      return;

   // Find size of the client
   WinQueryWindowPos (hwnd, &swp);

   // Find out the size of the bitmap
   lBitmapCX = pSelf->pBitmap->cx ();
   lBitmapCY = pSelf->pBitmap->cy ();

   // Shrink window if zoomed in
   if (1 < pSelf->iZoom)
   {
      // Shrink window by zoom factor
      swp.cx /= pSelf->iZoom;
      swp.cy /= pSelf->iZoom;
   }

   // Activate the scroll bars?
   fNeedHScroll = lBitmapCX > swp.cx;
   fNeedVScroll = lBitmapCY > swp.cy;

   hwndHScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                  FID_HORZSCROLL);
   hwndVScroll = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT),
                                  FID_VERTSCROLL);

   // Setup the horizontal scroll bar
   if (fNeedHScroll)
   {
      LONG  lSize;

      lSize = lBitmapCX - swp.cx;

      WinSendMsg (hwndHScroll, SBM_SETSCROLLBAR,
                  (MPARAM)WinSendMsg (hwndHScroll, SBM_QUERYPOS,
                                      (MPARAM)NULL, (MPARAM)NULL),
                  MPFROM2SHORT (0, lSize));

      WinSendMsg (hwndHScroll, SBM_SETTHUMBSIZE,
                  MPFROM2SHORT (swp.cx, lBitmapCX),
                  (MPARAM)NULL);

      pSelf->lHscrollMax = lSize;
   }
   else
      WinEnableWindow (hwndHScroll, FALSE);

   // Set up the vertical scroll bar
   if (fNeedVScroll)
   {
      LONG  lSize;

      lSize = lBitmapCY - swp.cy;

      WinSendMsg (hwndVScroll, SBM_SETSCROLLBAR,
                  (MPARAM)WinSendMsg (hwndVScroll, SBM_QUERYPOS,
                                      (MPARAM)NULL, (MPARAM)NULL),
                  MPFROM2SHORT (0, lSize));

      WinSendMsg (hwndVScroll, SBM_SETTHUMBSIZE,
                  MPFROM2SHORT (swp.cy, lBitmapCY),
                  (MPARAM)NULL);

      pSelf->lVscrollMax = lSize;
   }
   else
      WinEnableWindow (hwndVScroll, FALSE);
}
