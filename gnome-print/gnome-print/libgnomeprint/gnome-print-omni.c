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
#include <dlfcn.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <memory.h>

#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gnome-print-omni.h>
#include <libgnomeprint/gnome-printer-private.h>

const static int fDebugOutput = 0;

// @TBD - why global?
///static GnomePrintRGBPClass *omni_parent_class;

// @TBD - @HACK
// no way to get to our class data from a GnomePrintContext
int (* showpage)         (GnomePrintContext *pc);

static void     gnome_print_omni_init          (GnomePrintOmni *pOmni);
static void     gnome_print_omni_destroy       (GtkObject *object);
static void     gnome_print_omni_class_init    (GnomePrintOmniClass *klass);
static int      gnome_print_omni_print_band    (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *rect);
static gint     gnome_print_omni_showpage      (GnomePrintContext *pc);
static gint     gnome_print_omni_close         (GnomePrintContext *pc);
static void     gnome_print_omni_write_file    (void *pMagicCookie, unsigned char *puchData, int iSize);

GnomePrintContext *
gnome_print_omni_new (GnomePrinter *printer, const char *paper_size, int dpi) // @HACK no nonsquare resolution allowed!
{
   GnomePrintOmni   *pOmni;
   gdouble           value;
   const char       *pszPaperName;

   if (fDebugOutput) printf (__FUNCTION__ ": printer = 0x%08x, paper_size = %s, dpi = %d\n", (int)printer, paper_size, dpi);

   g_return_val_if_fail (printer != NULL, NULL);
   g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
   g_return_val_if_fail (paper_size != NULL, NULL);
   g_return_val_if_fail (dpi >= 0, NULL);

   pOmni = gtk_type_new (gnome_print_omni_get_type ());

   pOmni->paper_info = gnome_paper_with_name (paper_size);
   if (pOmni->paper_info == NULL)
      g_return_val_if_fail (FALSE, NULL);

   if (fDebugOutput) printf (__FUNCTION__ ": paper width        %f\n", gnome_paper_pswidth (pOmni->paper_info));
///if (fDebugOutput) printf (__FUNCTION__ ": paper left margin  %f\n", gnome_paper_lmargin (pOmni->paper_info));
///if (fDebugOutput) printf (__FUNCTION__ ": paper right margin %f\n", gnome_paper_rmargin (pOmni->paper_info));

   value =  gnome_paper_pswidth (pOmni->paper_info);
   // @HACK - the ps size should not include margins!
///value -= gnome_paper_lmargin (pOmni->paper_info);
///value -= gnome_paper_rmargin (pOmni->paper_info);

   value = gnome_paper_convert (value, gnome_unit_with_name ("Inch"));

   if (fDebugOutput) printf (__FUNCTION__ ": value = %f\n", value);

   pOmni->iPaperWidth  = value * dpi;

   if (fDebugOutput) printf ("pOmni->iPaperWidth = %d\n", pOmni->iPaperWidth);

   if (fDebugOutput) printf (__FUNCTION__ ": paper height        %f\n", gnome_paper_psheight (pOmni->paper_info));
///if (fDebugOutput) printf (__FUNCTION__ ": paper top margin    %f\n", gnome_paper_tmargin  (pOmni->paper_info));
///if (fDebugOutput) printf (__FUNCTION__ ": paper bottom margin %f\n", gnome_paper_bmargin  (pOmni->paper_info));

   value =  gnome_paper_psheight (pOmni->paper_info);
   // @HACK - the ps size should not include margins!
///value -= gnome_paper_tmargin  (pOmni->paper_info);
///value -= gnome_paper_bmargin  (pOmni->paper_info);

   value = gnome_paper_convert (value, gnome_unit_with_name ("Inch"));

   if (fDebugOutput) printf (__FUNCTION__ ": value = %f\n", value);

   pOmni->iPaperHeight = value * dpi;

   if (fDebugOutput) printf (__FUNCTION__ ": pOmni->iPaperHeight = %d\n", pOmni->iPaperHeight);

   pszPaperName = gnome_paper_name (pOmni->paper_info);

   // @TBD - need to merge with gnome-print
   if (0 == g_strcasecmp (pszPaperName, "US-Letter"))
      pOmni->pszPaperProps = "FORM_LETTER";
   else if (0 == g_strcasecmp (pszPaperName, "US-Legal"))
      pOmni->pszPaperProps = "FORM_LEGAL";
   else if (0 == g_strcasecmp (pszPaperName, "A3"))
      pOmni->pszPaperProps = "FORM_A3";
   else if (0 == g_strcasecmp (pszPaperName, "A4"))
      pOmni->pszPaperProps = "FORM_A4";
   else if (0 == g_strcasecmp (pszPaperName, "A5"))
      pOmni->pszPaperProps = "FORM_A5";
   else if (0 == g_strcasecmp (pszPaperName, "B4"))
      pOmni->pszPaperProps = "FORM_B4";
   else if (0 == g_strcasecmp (pszPaperName, "B5"))
      pOmni->pszPaperProps = "FORM_B5";
   else if (0 == g_strcasecmp (pszPaperName, "B5-Japan"))
      pOmni->pszPaperProps = "FORM_JIS_B5";
   else if (0 == g_strcasecmp (pszPaperName, "Half-Letter"))
      pOmni->pszPaperProps = "FORM_HALF_LETTER";
   else if (0 == g_strcasecmp (pszPaperName, "Executive"))
      pOmni->pszPaperProps = "FORM_EXECUTIVE";
   else if (0 == g_strcasecmp (pszPaperName, "Tabloid/Ledger"))
      pOmni->pszPaperProps = "FORM_LEDGER";
   else if (0 == g_strcasecmp (pszPaperName, "Monarch"))
      pOmni->pszPaperProps = "@TBD"; // @TBD
   else if (0 == g_strcasecmp (pszPaperName, "SuperB"))
      pOmni->pszPaperProps = "FORM_SUPER_B";
   else if (0 == g_strcasecmp (pszPaperName, "Envelope-Commercial"))
      pOmni->pszPaperProps = "@TBD"; // @TBD
   else if (0 == g_strcasecmp (pszPaperName, "Envelope-Monarch"))
      pOmni->pszPaperProps = "FORM_MONARCH_ENVELOPE";
   else if (0 == g_strcasecmp (pszPaperName, "Envelope-DL"))
      pOmni->pszPaperProps = "FORM_DL_ENVELOPE";
   else if (0 == g_strcasecmp (pszPaperName, "Envelope-C5"))
      pOmni->pszPaperProps = "FORM_C5_ENVELOPE";
   else if (0 == g_strcasecmp (pszPaperName, "EuroPostcard"))
      pOmni->pszPaperProps = "@TBD"; // @TBD

   if (!gnome_print_omni_construct (pOmni, printer, pOmni->paper_info, dpi))
   {
      gtk_object_unref (GTK_OBJECT (pOmni));

      // @TBD gnome should handle this
      return NULL;
   }

   return GNOME_PRINT_CONTEXT (pOmni);
}

/**
 * gnome_print_omni_get_type:
 *
 * GTK type identification routine for #GnomePrintOmni
 *
 * Returns: The Gtk type for the #GnomePrintOmni object
 */
GtkType
gnome_print_omni_get_type (void)
{
   static GtkType type = 0;

   if (fDebugOutput) printf (__FUNCTION__ "\n");

   if (!type)
   {
      GtkTypeInfo info = {
         "GnomePrintOmni",
         sizeof (GnomePrintOmni),
         sizeof (GnomePrintOmniClass),
         (GtkClassInitFunc)gnome_print_omni_class_init,
         (GtkObjectInitFunc)gnome_print_omni_init,
         /* reserved_1 */ NULL,
         /* reserved_2 */ NULL,
         (GtkClassInitFunc) NULL,
      };

      type = gtk_type_unique (gnome_print_rgbp_get_type (), &info);
   }

   return type;
}

GnomePrintOmni *
gnome_print_omni_construct (GnomePrintOmni *pOmni, GnomePrinter *printer, const GnomePaper *paper_info, int dpi)
{
   GnomePrintContext *pc = GNOME_PRINT_CONTEXT (pOmni);

   if (fDebugOutput) printf (__FUNCTION__ ": pOmni = 0x%08x, printer = 0x%08x, paper_info = 0x%08x, dpi = %d\n", (int)pOmni, (int)printer, (int)paper_info, dpi);

   g_return_val_if_fail (printer != NULL, NULL);
   g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
   g_return_val_if_fail (pOmni != NULL, NULL);
   g_return_val_if_fail (GNOME_IS_PRINT_OMNI (pOmni), NULL);
   g_return_val_if_fail (paper_info != NULL, NULL);
   g_return_val_if_fail (dpi >= 0, NULL);

   if (gnome_print_rgbp_construct (GNOME_PRINT_RGBP (pOmni), paper_info, dpi))
   {
      char  achJobProperties[1024];                            // @TBD
      char *pszDeviceName          = "libHP_DeskJet_1120C.so"; // @HACK

      gnome_print_context_open_file (pc, printer->filename);

      pOmni->vhOmni   = dlopen ("libomni.so", RTLD_NOW | RTLD_GLOBAL);
      pOmni->vhDevice = 0;
      pOmni->pDevice  = 0;

      if (fDebugOutput) printf (__FUNCTION__ ": dlopen (libomni.so) = 0x%08x\n", (int)pOmni->vhOmni);

      if (!pOmni->vhOmni)
      {
         if (fDebugOutput) printf (__FUNCTION__ ": dlerror returns %s\n", dlerror ());

         return NULL;
      }

      pOmni->vhDevice = dlopen (pszDeviceName, RTLD_NOW | RTLD_GLOBAL);

      if (fDebugOutput) printf (__FUNCTION__ ": dlopen (libHP_DeskJet_1120C.so) = 0x%08x\n", (int)pOmni->vhOmni);

      if (!pOmni->vhDevice)
      {
         if (fDebugOutput) printf (__FUNCTION__ ": dlerror returns %s\n", dlerror ());

         return NULL;
      }

      pOmni->pfnNewDeviceWArgs = (PFNNEWDEVICEWARGS)dlsym (pOmni->vhDevice, "newDevice__FPcb");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (newDevice__FPcb) = 0x%08x\n", (int)pOmni->pfnNewDeviceWArgs);

      pOmni->pfnDeleteDevice = (PFNDELETEDEVICE)dlsym (pOmni->vhDevice, "deleteDevice__FP6Device");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (deleteDevice__FP6Device) = 0x%08x\n", (int)pOmni->pfnDeleteDevice);

      pOmni->pfnSetOutputFunction = (PFNSETOUTPUTFUNCTION)dlsym (pOmni->vhOmni, "SetOutputFunctionNN__FPvPFPvPUci_vT0");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (SetOutputFunction__FPvPFPvPUci_vT0) = 0x%08x\n", (int)pOmni->pfnSetOutputFunction);

      pOmni->pfnBeginJob = (PFNBEGINJOB)dlsym (pOmni->vhOmni, "BeginJob__FPv");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (BeginJob__FPv) = 0x%08x\n", (int)pOmni->pfnBeginJob);

      pOmni->pfnNewPage = (PFNNEWPAGE)dlsym (pOmni->vhOmni, "NewPage__FPv");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (NewPage__FPv) = 0x%08x\n", (int)pOmni->pfnNewPage);

      pOmni->pfnEndJob = (PFNENDJOB)dlsym (pOmni->vhOmni, "EndJob__FPv");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (EndJob__FPv) = 0x%08x\n", (int)pOmni->pfnEndJob);

      pOmni->pfnRasterize = (PFNRASTERIZE)dlsym (pOmni->vhOmni, "Rasterize");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (Rasterize) = 0x%08x\n", (int)pOmni->pfnRasterize);

      pOmni->pfnFindResolutionName = (PFNFINDRESOLUTIONNAME)dlsym (pOmni->vhOmni, "FindResolutionName__FPci");

      if (fDebugOutput) printf (__FUNCTION__ ": dlsym (FindResolutionName) = 0x%08x\n", (int)pOmni->pfnFindResolutionName);

      if (  !pOmni->pfnNewDeviceWArgs
         || !pOmni->pfnDeleteDevice
         || !pOmni->pfnSetOutputFunction
         || !pOmni->pfnBeginJob
         || !pOmni->pfnNewPage
         || !pOmni->pfnEndJob
         || !pOmni->pfnRasterize
         || !pOmni->pfnFindResolutionName
         )
      {
         if (fDebugOutput) printf (__FUNCTION__ ": dlerror returns %s\n", dlerror ());

         return NULL;
      }

      pOmni->iPageNumber      = 0;
      pOmni->icbBitmapBuffer  = 0;
      pOmni->puchBitmapBuffer = 0;

      sprintf (achJobProperties, "orientation=PORTRAIT form=%s resolution=%s",
               pOmni->pszPaperProps,
               pOmni->pfnFindResolutionName (pszDeviceName, dpi));

      if (fDebugOutput) printf (__FUNCTION__ ": job properties are %s\n", achJobProperties);

      pOmni->pDevice = pOmni->pfnNewDeviceWArgs (achJobProperties, 0);

      if (fDebugOutput) printf (__FUNCTION__ ": pDevice = 0x%08x\n", (int)pOmni->pDevice);

      pOmni->pfnSetOutputFunction (pOmni->pDevice, gnome_print_omni_write_file, pc);

      return pOmni;
   }
   else
   {
      return NULL;
   }
}

static void
gnome_print_omni_init (GnomePrintOmni *pOmni)
{
   GnomePrintContext *pc = GNOME_PRINT_CONTEXT (pOmni);

   if (fDebugOutput) printf (__FUNCTION__ ": pOmni = 0x%08x\n", (int)pOmni);
   if (fDebugOutput) printf (__FUNCTION__ ": pc = 0x%08x\n", (int)pc);
}

static void
gnome_print_omni_destroy (GtkObject *object)
{
   GnomePrintOmni *pOmni;

   if (fDebugOutput) printf (__FUNCTION__ ": object = 0x%08x\n", (int)object);

   g_return_if_fail (object != NULL);
   g_return_if_fail (GNOME_IS_PRINT_OMNI (object));

   pOmni = GNOME_PRINT_OMNI (object);

   if (  pOmni->pDevice
      && pOmni->pfnDeleteDevice
      )
   {
      if (fDebugOutput) printf (__FUNCTION__ ": deleting 0x%08x\n", (int)pOmni->pDevice);

      pOmni->pfnDeleteDevice (pOmni->pDevice);
   }

   if (pOmni->puchBitmapBuffer)
   {
      free (pOmni->puchBitmapBuffer);
      pOmni->puchBitmapBuffer = 0;
      pOmni->icbBitmapBuffer  = 0;
   }

   if (pOmni->vhDevice)
   {
      int rc = dlclose (pOmni->vhDevice);

      if (fDebugOutput) printf (__FUNCTION__ ": dlclose (0x%08x) = %d\n", (int)pOmni->vhDevice, rc);
   }

   if (pOmni->vhOmni)
   {
      int rc = dlclose (pOmni->vhOmni);

      if (fDebugOutput) printf (__FUNCTION__ ": dlclose (0x%08x) = %d\n", (int)pOmni->vhOmni, rc);
   }
}

static void
gnome_print_omni_class_init (GnomePrintOmniClass *klass)
{
   GtkObjectClass         *object_class;
   GnomePrintContextClass *pc_class;
   GnomePrintRGBPClass    *rgbp_class;

   if (fDebugOutput) printf (__FUNCTION__ ": klass = 0x%08x\n", (int)klass);

   // Access our information
   object_class = (GtkObjectClass *)klass;
   pc_class     = (GnomePrintContextClass *)klass;
   rgbp_class   = (GnomePrintRGBPClass *)klass;

   showpage     = pc_class->showpage;

///omni_parent_class = gtk_type_class (gnome_print_context_get_type ());

   object_class->destroy  = gnome_print_omni_destroy;

   pc_class->showpage     = gnome_print_omni_showpage;
   pc_class->close        = gnome_print_omni_close;

   rgbp_class->print_band = gnome_print_omni_print_band;
}

static int
gnome_print_omni_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *pRect)
{
	GnomePrintContext *pc                 = 0;
	GnomePrintOmni    *pOmni              = 0;
   ArtIRect           rect;
   int                icbBitmapScanline  = 0;
   int                icbRGBScanline     = 0;
   int                iBytesToAlloc      = 0;
   RECTL              rectlPageLocation;
   BITMAPINFO2        bmi2;
   register int       yBitmap, yRGB, x;

   if (fDebugOutput) printf (__FUNCTION__ ": rgbp = 0x%08x, rgb_buffer = 0x%08x, pRect = 0x%08x\n", (int)rgbp, (int)rgb_buffer, (int)pRect);
   if (fDebugOutput) printf (__FUNCTION__ ": rect = { (%d, %d) - (%d, %d)\n", pRect->x0, pRect->y0, pRect->x1, pRect->y1);

   // Access our information
   pc    = GNOME_PRINT_CONTEXT (rgbp);
   pOmni = GNOME_PRINT_OMNI (pc);

   // Make a local copy
   rect.x0 = pRect->x0;
   rect.y0 = pRect->y0;
   rect.x1 = pRect->x1;
   rect.y1 = pRect->y1;

   // Set up the bitblt location structure
   rectlPageLocation.xLeft   = pRect->x0;
   rectlPageLocation.yBottom = pRect->y0;
   rectlPageLocation.xRight  = pRect->x1;
   rectlPageLocation.yTop    = pRect->y1;

   // Inverse and swap @HACK
   rectlPageLocation.yBottom = pOmni->iPaperHeight - pRect->y1;
   rectlPageLocation.yTop    = pOmni->iPaperHeight - pRect->y0;

   if (fDebugOutput) printf (__FUNCTION__ ": rectlPageLocation.xLeft   = %d\n", rectlPageLocation.xLeft  );
   if (fDebugOutput) printf (__FUNCTION__ ": rectlPageLocation.yBottom = %d\n", rectlPageLocation.yBottom);
   if (fDebugOutput) printf (__FUNCTION__ ": rectlPageLocation.xRight  = %d\n", rectlPageLocation.xRight );
   if (fDebugOutput) printf (__FUNCTION__ ": rectlPageLocation.yTop    = %d\n", rectlPageLocation.yTop   );

   // translate to 0
   rect.x1 -= rect.x0;
   rect.x0  = 0;
   rect.y1 -= rect.y0;
   rect.y0  = 0;

   // Setup the bitmap info structure
   bmi2.cbFix         = sizeof (BITMAPINFO2)
                      - sizeof (((PBITMAPINFO2)0)->argbColor)
                      ;
   bmi2.cx            = rect.x1;
   bmi2.cy            = rect.y1;
   bmi2.cPlanes       = 1;
   bmi2.cBitCount     = 24;
   bmi2.cclrUsed      = 1 << bmi2.cBitCount;
   bmi2.cclrImportant = 0;

   if (fDebugOutput) printf (__FUNCTION__ ": rect.x0 = %d\n", rect.x0);
   if (fDebugOutput) printf (__FUNCTION__ ": rect.y0 = %d\n", rect.y0);
   if (fDebugOutput) printf (__FUNCTION__ ": rect.x1 = %d\n", rect.x1);
   if (fDebugOutput) printf (__FUNCTION__ ": rect.y1 = %d\n", rect.y1);

   icbBitmapScanline = ((rect.x1 * 24 + 31) >> 5) << 2;
   icbRGBScanline    = rect.x1 * 3;
   iBytesToAlloc     = icbBitmapScanline * rect.y1;

   if (fDebugOutput) printf (__FUNCTION__ ": icbBitmapScanline = %d\n", icbBitmapScanline);
   if (fDebugOutput) printf (__FUNCTION__ ": icbRGBScanline = %d\n", icbRGBScanline);
   if (fDebugOutput) printf (__FUNCTION__ ": iBytesToAlloc = %d\n", iBytesToAlloc);

   // Allocate enough space
   if (iBytesToAlloc > pOmni->icbBitmapBuffer)
   {
      if (pOmni->puchBitmapBuffer)
      {
         free (pOmni->puchBitmapBuffer);
         pOmni->puchBitmapBuffer = 0;
         pOmni->icbBitmapBuffer  = 0;
      }

      pOmni->puchBitmapBuffer = malloc (iBytesToAlloc);
      if (pOmni->puchBitmapBuffer)
      {
         pOmni->icbBitmapBuffer = iBytesToAlloc;
      }
   }

   // Copy the scanlines
   for (yBitmap = rect.y0, yRGB = rect.y1 - 1;
        yBitmap < rect.y1;
        yBitmap++, yRGB--)
   {
      int iTo = yBitmap * icbBitmapScanline;

      for (x = rect.x0; x < rect.x1; x++)
      {
         int iFrom = (x * 3) + 2 + yRGB * icbRGBScanline;

         pOmni->puchBitmapBuffer[iTo++] = rgb_buffer[iFrom--];
         pOmni->puchBitmapBuffer[iTo++] = rgb_buffer[iFrom--];
         pOmni->puchBitmapBuffer[iTo++] = rgb_buffer[iFrom];
      }
   }

   // Bitblt it to omni
   pOmni->pfnRasterize (pOmni->pDevice,
                        pOmni->puchBitmapBuffer,
                        &bmi2,
                        NULL,
                        &rectlPageLocation,
                        BITBLT_BITMAP);

   return 0;
}

static gint
gnome_print_omni_showpage (GnomePrintContext *pc)
{
	GnomePrintOmni *pOmni = 0;

   if (fDebugOutput) printf (__FUNCTION__ ": pc = 0x%08x\n", (int)pc);

   // Access our information
   pOmni = GNOME_PRINT_OMNI (pc);

   pOmni->iPageNumber++;

   if (1 == pOmni->iPageNumber)
   {
      // Notify omni of the start of the job
      pOmni->pfnBeginJob (pOmni->pDevice);
   }
   else
   {
      // Notify omni of the start of another page
      pOmni->pfnNewPage (pOmni->pDevice);
   }

   // brute force callback to RGBP driver
   return showpage (pc);
}

static gint
gnome_print_omni_close (GnomePrintContext *pc)
{
	GnomePrintOmni *pOmni = 0;

   if (fDebugOutput) printf (__FUNCTION__ ": pc = 0x%08x\n", (int)pc);

   // Access our information
   pOmni = GNOME_PRINT_OMNI (pc);

   // Notify omni of the end of the job
   pOmni->pfnEndJob (pOmni->pDevice);

   return 0;
}

static void
gnome_print_omni_write_file (void *pMagicCookie, unsigned char *puchData, int iSize)
{
	GnomePrintContext *pc = 0;

///if (fDebugOutput) printf (__FUNCTION__ ": pMagicCookie = 0x%08x, puchData = 0x%08x, iSize = %d\n", (int)pMagicCookie, (int)puchData, iSize);

   // Access our information
   pc = (GnomePrintContext *)pMagicCookie;

   gnome_print_context_write_file (pc, puchData, iSize);
}
