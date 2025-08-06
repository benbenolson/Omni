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
#ifndef _gnome_print_omni
#define _gnome_print_omni

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_OMNI            (gnome_print_omni_get_type ())
#define GNOME_PRINT_OMNI(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_OMNI, GnomePrintOmni))
#define GNOME_PRINT_OMNI_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_OMNI, GnomePrintOmniClass))
#define GNOME_IS_PRINT_OMNI(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_OMNI))
#define GNOME_IS_PRINT_OMNI_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_OMNI))

typedef struct _GnomePrintOmni       GnomePrintOmni;
typedef struct _GnomePrintOmniClass  GnomePrintOmniClass;

typedef unsigned char byte, BYTE, *PBYTE;
typedef struct _Rectl {
   int xLeft;
   int yBottom;
   int xRight;
   int yTop;
} RECTL, *PRECTL;
typedef struct _Sizel {
   int cx;
   int cy;
} SIZEL, *PSIZEL;
typedef struct _RGB2        /* rgb2 */
{
   BYTE bBlue;              /* Blue component of the color definition */
   BYTE bGreen;             /* Green component of the color definition*/
   BYTE bRed;               /* Red component of the color definition  */
   BYTE fcOptions;          /* Reserved, must be zero                 */
} RGB2, *PRGB2;
typedef struct _BitmapInfo {
   int  cbFix;
   int  cx;
   int  cy;
   int  cPlanes;
   int  cBitCount;
   int  ulCompresstion;
   int  cclrUsed;
   int  cclrImportant;
   RGB2 argbColor[1];
} BITMAPINFO2, *PBITMAPINFO2;
typedef enum {
   BITBLT_BITMAP,
   BITBLT_AREA,
   BITBLT_TEXT
} BITBLT_TYPE;

typedef void * (*PFNNEWDEVICEWARGS)     (char              *pszJobProperties,
                                         int                fAdvanced);
typedef void   (*PFNDELETEDEVICE)       (void              *pDevice);

typedef void   (*PFNOUTPUTFUNCTION)     (void              *pMagicCookie,
                                         unsigned char     *puchData,
                                         int                iSize);
typedef void   (*PFNSETOUTPUTFUNCTION)  (void              *pDev,
                                         PFNOUTPUTFUNCTION  pfn,
                                         void              *pMC);
typedef void   (*PFNBEGINJOB)           (void              *pDev);
typedef void   (*PFNNEWPAGE)            (void              *pDev);
typedef void   (*PFNENDJOB)             (void              *pDev);
typedef void   (*PFNRASTERIZE)          (void              *pDev,
                                         PBYTE              pbBits,
                                         PBITMAPINFO2       pbmi,
                                         PSIZEL             psizelPage,
                                         PRECTL             prectlPageLocation,
                                         BITBLT_TYPE        eType);

typedef char * (*PFNFINDRESOLUTIONNAME) (char              *pszDeviceName,
                                         int                iDpi);

struct _GnomePrintOmni {
	GnomePrintRGBP        rgbp;

   void                 *vhOmni;
   void                 *vhDevice;
   PFNNEWDEVICEWARGS     pfnNewDeviceWArgs;
   PFNDELETEDEVICE       pfnDeleteDevice;
   void                 *pDevice;
   PFNSETOUTPUTFUNCTION  pfnSetOutputFunction;
   PFNBEGINJOB           pfnBeginJob;
   PFNNEWPAGE            pfnNewPage;
   PFNENDJOB             pfnEndJob;
   PFNRASTERIZE          pfnRasterize;
   PFNFINDRESOLUTIONNAME pfnFindResolutionName;

   int                   iPageNumber;

   int                   icbBitmapBuffer;
   unsigned char        *puchBitmapBuffer;

   char                 *pszPaperProps;
   const GnomePaper     *paper_info;
   int                   iPaperWidth;
   int                   iPaperHeight;
};

struct _GnomePrintOmniClass {
	GnomePrintRGBPClass  parent_class;
};

GtkType 	          gnome_print_omni_get_type  (void);
GnomePrintContext *gnome_print_omni_new       (GnomePrinter *printer, const char *paper_size, int dpi);
GnomePrintOmni    *gnome_print_omni_construct (GnomePrintOmni *pOmni, GnomePrinter *printer, const GnomePaper *paper_info, int dpi);

END_GNOME_DECLS

#endif
