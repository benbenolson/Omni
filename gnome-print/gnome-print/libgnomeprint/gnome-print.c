/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-print.c: Main GnomePrint API
 *
 * Authors:
 *   Raph Levien (raph@acm.org)
 *   Miguel de Icaza (miguel@kernel.org)
 *   Lauris Kaplinski <lauris@ariman.ee>
 *   Chema Celorio (chema@celorio.com)
 */

/*
 * GnomePrintContext now has graphicContext member. It is updated by every
 * method, except clip and eoclip - I yet have to figure out, whether maintaining
 * explicit SVP-s here is worth the effort.
 * GC is updated AFTER corresponding class method is invoked - we expect, that if
 * context implements class method, it manages it's own data.
 * So you do not have to implement most class methods any more - simply wait for
 * interesting ones (fill, stroke), and update current state from GC.
 * GC is also used to check errors, so lineto without preceding moveto is now
 * handled by GnomePrintContext itself. -Lauris-
 */

/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <string.h>
#include <locale.h>

#include <gtk/gtkobject.h>

#include <stdio.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>

#include <gtk/gtkobject.h>
#include "libgnome/gnome-defs.h"
#include <libgnome/gnome-util.h>

#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-private.h>

/* Gnome Print Drivers */
#include <libgnomeprint/gnome-print-ps.h>
#include <libgnomeprint/gnome-print-ps2.h>
#include <libgnomeprint/gnome-print-pdf.h>
#include <libgnomeprint/gnome-print-pclr.h>
#include <libgnomeprint/gnome-print-pclv.h>
#include <libgnomeprint/gnome-print-omni.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-print-frgba.h>

#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>

/* fixme: */
#ifdef USE_GNOME_FONT
#include <libgnomefont/gf-ps-unicode.h>
#define gp_unicode_from_ps gf_unicode_from_ps
#else
#include <libgnomeprint/gp-ps-unicode.h>
#endif

#ifdef ENABLE_LIBGPA
#include <libgnomeprint/gnome-print-file.h>
#endif

static void gnome_print_context_class_init (GnomePrintContextClass *klass);

static void gnome_print_context_init (GnomePrintContext *pc);

static void gnome_print_context_finalize (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

GtkType
gnome_print_context_get_type (void)
{
	static GtkType pc_type = 0;
	
	if (!pc_type)
	{
		GtkTypeInfo pc_info =
		{
			"GnomePrintContext",
			sizeof (GnomePrintContext),
			sizeof (GnomePrintContextClass),
			(GtkClassInitFunc) gnome_print_context_class_init,
			(GtkObjectInitFunc) gnome_print_context_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		pc_type = gtk_type_unique (gtk_object_get_type (), &pc_info);
	}
	
	return pc_type;
}

static void
gnome_print_context_class_init (GnomePrintContextClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) class;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->finalize = gnome_print_context_finalize;
}

static void
gnome_print_context_init (GnomePrintContext *pc)
{
	pc->gc = gp_gc_new ();

	pc->f = NULL;
}


GnomePrintContext *
gnome_print_context_new_with_paper_size (GnomePrinter *printer, const char *paper_size)
{
	const gchar *driver;
	
	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	g_return_val_if_fail (paper_size != NULL, NULL);

#ifdef ENABLE_LIBGPA
	/* If print to file pop up a dialog */
	if (printer->print_to_file)
		if (gnome_print_file_dialog (printer) != 0)
			return NULL;

	if (printer->filename == NULL)
		printer->filename = gnome_printer_dup_command (printer);

	/* Get the driver */
	driver = gnome_printer_const_get (printer, "Driver");
	if (!driver || *driver == 0) {
		g_warning ("Could not get the Driver from the Printer.\n");
		return NULL;
	}

#else	
	driver = printer->driver;
#endif	

        /* This is just TEMPORARY !, we will prolly use dlopen */
	if (strcmp (driver, "gnome-print-ps") == 0) {
		GnomePrintPs *ps;
		GnomePrintContext *frgba;
		ps = gnome_print_ps_new (printer);
		if (!GNOME_IS_PRINT_PS (ps))
			return NULL;
		frgba = gnome_print_frgba_new ((GnomePrintContext *) ps);
		gtk_object_unref (GTK_OBJECT (ps));
		return frgba ? frgba : NULL;
	} else 	if (strcmp (driver, "gnome-print-ps-rgb") == 0) {
		GnomePrintPs *ps = gnome_print_ps_new (printer);
		return ps ? GNOME_PRINT_CONTEXT (ps) : NULL;
	} else if (strcmp (driver, "gnome-print-ps2") == 0) {
		GnomePrintPs2 *ps2;
		GnomePrintContext *frgba;
		ps2 = gnome_print_ps2_new (printer);
		if (!GNOME_IS_PRINT_PS2 (ps2))
			return NULL;
		frgba = gnome_print_frgba_new ((GnomePrintContext *) ps2);
		gtk_object_unref (GTK_OBJECT (ps2));
		return frgba ? frgba : NULL;
	} else  if (strcmp (driver, "gnome-print-pdf") == 0) {
		GnomePrintPdf *pdf = gnome_print_pdf_new_with_paper (printer, paper_size);
		return pdf ? GNOME_PRINT_CONTEXT (pdf) : NULL;
	} else 	if (strcmp (driver, "gnome-print-pclr") == 0) {
		return gnome_print_pclr_new (printer, paper_size, 300);
	} else 	if (strcmp (driver, "gnome-print-pclv") == 0) {
		return gnome_print_pclv_new (printer, paper_size, 300);
	} else 	if (strcmp (driver, "gnome-print-omni") == 0) {
		return gnome_print_omni_new (printer, paper_size, 300);
	} else {
		g_warning ("Could not determine the driver to call ..\n");
	}

#if 0
	if (strncmp (printer->driver, "gnome-print-uni", strlen ("gnome-print-uni")) == 0)
		return gnome_print_pixbuf_new (printer, paper_size, 75);
#endif

	return NULL;
}

/*
 * This dispatch is by cases rather than being object oriented. It may
 * be desirable to make it more OO. Basically, the way that would be
 * done would be to have a gnome_printer_make_new_context () method
 * on GnomePrinter objects.
 *
 * However, this will do for now, and saves making extra classes.
*/
GnomePrintContext *
gnome_print_context_new (GnomePrinter *printer)
{
	g_return_val_if_fail (printer != NULL, NULL);

	return gnome_print_context_new_with_paper_size (printer, "US-Letter");
}

static void
gnome_print_context_finalize (GtkObject *object)
{
	GnomePrintContext *pc;

	pc = GNOME_PRINT_CONTEXT (object);

	gp_gc_unref (pc->gc);

	(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/* methods */

#define PRINT_CLASS(pc) GNOME_PRINT_CONTEXT_CLASS(GTK_OBJECT(pc)->klass)

int
gnome_print_newpath (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->newpath)
		ret =  PRINT_CLASS(pc)->newpath (pc);

	gp_gc_newpath (pc->gc);

	return ret;
}

int
gnome_print_moveto (GnomePrintContext *pc, double x, double y)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->moveto)
		ret = PRINT_CLASS(pc)->moveto (pc, x, y);

	gp_gc_moveto (pc->gc, x, y);

	return ret;
}

int
gnome_print_lineto (GnomePrintContext *pc, double x, double y)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);

	if (PRINT_CLASS (pc)->lineto)
		ret = PRINT_CLASS(pc)->lineto (pc, x, y);

	gp_gc_lineto (pc->gc, x, y);

	return ret;
}

int
gnome_print_curveto (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);

	if (PRINT_CLASS (pc)->curveto)
		ret = PRINT_CLASS(pc)->curveto (pc, x1, y1, x2, y2, x3, y3);

	gp_gc_curveto (pc->gc, x1, y1, x2, y2, x3, y3);

	return ret;
}

int
gnome_print_closepath (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	if (PRINT_CLASS (pc)->closepath)
		ret = PRINT_CLASS(pc)->closepath (pc);

	gp_gc_closepath (pc->gc);

	return ret;
}

int
gnome_print_setrgbcolor (GnomePrintContext *pc, double r, double g, double b)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->setrgbcolor)
		ret = PRINT_CLASS(pc)->setrgbcolor (pc, r, g, b);

	gp_gc_set_rgbcolor (pc->gc, r, g, b);

	return ret;
}

int
gnome_print_fill (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);

	if (PRINT_CLASS (pc)->fill)
		ret = PRINT_CLASS(pc)->fill (pc, ART_WIND_RULE_NONZERO);

	gp_gc_newpath (pc->gc);

	return ret;
}

int
gnome_print_eofill (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);

	if (PRINT_CLASS (pc)->fill)
		ret = PRINT_CLASS(pc)->fill (pc, ART_WIND_RULE_ODDEVEN);

	gp_gc_newpath (pc->gc);

	return ret;
}

int
gnome_print_setlinewidth (GnomePrintContext *pc, double width)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (width >= 0, GNOME_PRINT_ERROR_BADVALUE);

	if (PRINT_CLASS (pc)->setlinewidth)
		ret = PRINT_CLASS(pc)->setlinewidth (pc, width);

	gp_gc_set_linewidth (pc->gc, width);

	return ret;
}

int
gnome_print_setmiterlimit (GnomePrintContext *pc, double limit)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (limit >= 1.0, GNOME_PRINT_ERROR_BADVALUE);

	gp_gc_set_miterlimit (pc->gc, limit);

	if (PRINT_CLASS (pc)->setmiterlimit)
		ret = PRINT_CLASS(pc)->setmiterlimit (pc, limit);

	gp_gc_set_miterlimit (pc->gc, limit);

	return ret;
}

int
gnome_print_setlinejoin (GnomePrintContext *pc, int jointype)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail ((jointype >= 0) && (jointype < 3), GNOME_PRINT_ERROR_BADVALUE);

	if (PRINT_CLASS (pc)->setlinejoin)
		ret = PRINT_CLASS(pc)->setlinejoin (pc, jointype);

	gp_gc_set_linejoin (pc->gc, jointype);

	return ret;
}

int
gnome_print_setlinecap (GnomePrintContext *pc, int captype)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail ((captype >= 0) && (captype < 3), GNOME_PRINT_ERROR_BADVALUE);

	gp_gc_set_linecap (pc->gc, captype);

	if (PRINT_CLASS (pc)->setlinecap)
		ret = PRINT_CLASS(pc)->setlinecap (pc, captype);

	return ret;
}

int
gnome_print_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (n_values != 0) g_return_val_if_fail (values != NULL, GNOME_PRINT_ERROR_BADVALUE);

	if (PRINT_CLASS (pc)->setdash)
		ret = PRINT_CLASS(pc)->setdash (pc, n_values, values, offset);

	gp_gc_set_dash (pc->gc, n_values, values, offset);

	return ret;
}

int
gnome_print_strokepath (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	if (PRINT_CLASS (pc)->strokepath)
		ret = PRINT_CLASS(pc)->strokepath (pc);

	gp_gc_strokepath (pc->gc);

	return ret;
}

int
gnome_print_stroke (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->stroke)
		ret = PRINT_CLASS(pc)->stroke (pc);

	gp_gc_newpath (pc->gc);

	return ret;
}

int
gnome_print_setfont (GnomePrintContext *pc, GnomeFont *font)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (font != NULL, GNOME_PRINT_ERROR_BADVALUE);
	g_return_val_if_fail (GNOME_IS_FONT (font), GNOME_PRINT_ERROR_BADVALUE);

	if (PRINT_CLASS (pc)->setfont)
		ret = PRINT_CLASS(pc)->setfont (pc, font);

	gp_gc_set_font (pc->gc, font);

	return ret;
}

int
gnome_print_show (GnomePrintContext *pc, char const *text)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (text != NULL, GNOME_PRINT_ERROR_BADVALUE);

	if (PRINT_CLASS (pc)->show_sized)
		return (* PRINT_CLASS (pc)->show_sized) (pc, text, strlen (text));

	return GNOME_PRINT_OK;
}

int
gnome_print_show_sized (GnomePrintContext *pc, char const *text, int bytes)
{
	const char *invalid;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (text != NULL, -1);

	if (bytes < 1) return GNOME_PRINT_OK;

	if (!g_utf8_validate (text, bytes, &invalid))
		g_warning ("Could not UTF8 Validate *%s*", text);
	
	g_return_val_if_fail (g_utf8_validate (text, bytes, &invalid), GNOME_PRINT_ERROR_TEXTCORRUPT);

	if (PRINT_CLASS (pc)->show_sized)
		return (* PRINT_CLASS (pc)->show_sized) (pc, text, bytes);

	return GNOME_PRINT_OK;
}

/* WARNING: show_ucs4 is DEPRECATED */
int
gnome_print_show_ucs4 (GnomePrintContext *pc, guint32 *buf, gint length)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (buf != NULL, -1);

	if (length < 1) return GNOME_PRINT_OK;

	{
		static gint warned = FALSE;
		if (!warned)
			g_warning ("gnome_print_show_ucs4 is deprecated");
		warned = TRUE;
	}

	if (PRINT_CLASS (pc)->show_sized) {
		guchar *utf, *p;
		gint i;
		gint retval;

		utf = g_malloc (length * 2);
		for (p = utf, i = 0; i < length; i++) {
			p += g_unichar_to_utf8 (GINT_FROM_BE (buf[i]), p);
		}
		retval = (* PRINT_CLASS (pc)->show_sized) (pc, utf, p - utf);
		g_free (utf);
		return retval;
	} else {
		return GNOME_PRINT_OK;
	}
}

int
gnome_print_concat (GnomePrintContext *pc, const double matrix[6])
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (matrix != NULL, -1);

	if (PRINT_CLASS (pc)->concat)
		ret = PRINT_CLASS(pc)->concat (pc, matrix);

	gp_gc_concat (pc->gc, matrix);

	return ret;
}

int
gnome_print_gsave (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->gsave)
		ret = PRINT_CLASS(pc)->gsave (pc);

	gp_gc_gsave (pc->gc);

	return ret;
}

int
gnome_print_grestore (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->grestore)
		ret = PRINT_CLASS(pc)->grestore (pc);

	gp_gc_grestore (pc->gc);

	return ret;
}

int
gnome_print_clip (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);

	if (PRINT_CLASS (pc)->clip)
		ret = PRINT_CLASS(pc)->clip (pc, ART_WIND_RULE_NONZERO);

	return ret;
}

int
gnome_print_eoclip (GnomePrintContext *pc)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	gp_gc_close_all (pc->gc);

	if (PRINT_CLASS (pc)->clip)
		ret = PRINT_CLASS(pc)->clip (pc, ART_WIND_RULE_ODDEVEN);

	return ret;
}

int
gnome_print_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (data != NULL, -1);

	if (PRINT_CLASS (pc)->grayimage)
		return PRINT_CLASS(pc)->grayimage (pc, data, width, height, rowstride);

	return GNOME_PRINT_OK;
}

int
gnome_print_rgbimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (data != NULL, -1);

	if (PRINT_CLASS (pc)->rgbimage)
		return PRINT_CLASS(pc)->rgbimage (pc, data, width, height, rowstride);

	return GNOME_PRINT_OK;
}

int
gnome_print_rgbaimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	guchar * b, * d;
	const guchar * s;
	gint x, y, alpha, tmp, ret;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (data != NULL, -1);

	if (PRINT_CLASS (pc)->rgbaimage != NULL) {

		/* We have REAL rgba method */

		return PRINT_CLASS (pc)->rgbaimage (pc, data, width, height, rowstride);
	}

	if (PRINT_CLASS (pc)->rgbimage == NULL) return 0;

	/* Print RGB - better than nothing */

	b = g_new (gchar, width * height * 3);
	g_return_val_if_fail (b != NULL, -1);

	for (y = 0; y < height; y++) {
		s = data + y * rowstride;
		d = b + y * width * 3;
		for (x = 0; x < width; x++) {
			alpha = s[3];
			tmp = (s[0] - 0xff) * alpha;
			d[0] = 0xff + ((tmp + 0x80) >> 8);
			tmp = (s[1] - 0xff) * alpha;
			d[1] = 0xff + ((tmp + 0x80) >> 8);
			tmp = (s[2] - 0xff) * alpha;
			d[2] = 0xff + ((tmp + 0x80) >> 8);
			s += 4;
			d += 3;
		}
	}

	ret = PRINT_CLASS (pc)->rgbimage (pc, b, width, height, width * 3);

	g_free (b);

	return ret;
}

int
gnome_print_pixbuf (GnomePrintContext *pc, GdkPixbuf *pixbuf)
{
       if (gdk_pixbuf_get_has_alpha (pixbuf))
               return gnome_print_rgbaimage  (pc,
					      gdk_pixbuf_get_pixels    (pixbuf),
					      gdk_pixbuf_get_width     (pixbuf),
					      gdk_pixbuf_get_height    (pixbuf),
					      gdk_pixbuf_get_rowstride (pixbuf));
       else
               return gnome_print_rgbimage  (pc,
					     gdk_pixbuf_get_pixels    (pixbuf),
					     gdk_pixbuf_get_width     (pixbuf),
					     gdk_pixbuf_get_height    (pixbuf),
					     gdk_pixbuf_get_rowstride (pixbuf));
}

int
gnome_print_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (line != NULL, -1);

	if (PRINT_CLASS (pc)->textline)
		return PRINT_CLASS(pc)->textline (pc, line);

	return GNOME_PRINT_OK;
}

int
gnome_print_showpage (GnomePrintContext *pc)
{
	const GnomeFont * font;
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->showpage)
		ret = PRINT_CLASS(pc)->showpage (pc);

	/* fixme: do we need to clear the state? */
	font = gp_gc_get_font (pc->gc);
	gnome_font_ref (font);
	gp_gc_reset (pc->gc);
	gp_gc_set_font (pc->gc, (GnomeFont *) font);
	gnome_font_unref (font);

	return ret;
}

int
gnome_print_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->beginpage)
		return PRINT_CLASS(pc)->beginpage (pc, name_of_this_page);

	return GNOME_PRINT_OK;
}

int
gnome_print_setopacity (GnomePrintContext *pc, double opacity)
{
	gint ret = GNOME_PRINT_OK;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->setopacity)
		ret = PRINT_CLASS(pc)->setopacity (pc, opacity);

	gp_gc_set_opacity (pc->gc, opacity);

	return ret;
}

int
gnome_print_context_close (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (PRINT_CLASS (pc)->close)
		return PRINT_CLASS(pc)->close (pc);

	return GNOME_PRINT_OK;
}



/*
 * These functions are wrapper functions around capabilities that may
 * or may not be deconvolved in back ends.
 */

int
gnome_print_scale (GnomePrintContext *pc, double sx, double sy)
{
	double dst[6];

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	art_affine_scale (dst, sx, sy);
	return gnome_print_concat (pc, dst);
}

int
gnome_print_rotate (GnomePrintContext *pc, double theta)
{
	double dst[6];

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	art_affine_rotate (dst, theta);
	return gnome_print_concat (pc, dst);
}

int
gnome_print_translate (GnomePrintContext *pc, double x, double y)
{
	double dst[6];

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	art_affine_translate (dst, x, y);
	return gnome_print_concat (pc, dst);
}

/*
 * These functions provide a common interface for writing bytes to the
 *
 * printer.
 */

/* Return 0 on success */
int
gnome_print_context_open_file (GnomePrintContext *pc, const char *filename)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (filename != NULL, -1);

	if (filename[0] == '|')
	{
		pc->f = popen (filename + 1, "w");
		pc->is_pipe = TRUE;
	}
	else
	{
		/* This is not very intelligent, but it worked for me */
		if ((filename[0] == '~') && (filename[1] == '/'))
		{
			filename = g_concat_dir_and_file (g_get_home_dir (), &filename[2]);
		}
		
		pc->f = fopen (filename, "w");
		pc->is_pipe = FALSE;
	}
	return pc->f != NULL;
}

/* Return number of bytes written */
int
gnome_print_context_write_file (GnomePrintContext *pc, const char *buf, size_t size)
{
	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), 0);
	g_return_val_if_fail (pc->f != NULL, 0);

	return fwrite (buf, sizeof(char), size, pc->f);
}

/* Return number of bytes written, or -1 if error */
int
gnome_print_context_fprintf (GnomePrintContext *pc, const char *fmt, ...)
{
	va_list ap;
	int n_bytes;
	char *oldlocale;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (pc->f != NULL, -1);

	oldlocale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");

	va_start (ap, fmt);
	n_bytes = vfprintf (pc->f, fmt, ap);
	va_end (ap);

	setlocale (LC_NUMERIC, oldlocale);
	g_free (oldlocale);

	return n_bytes;
}

/* Return 0 on success */
int
gnome_print_context_close_file (GnomePrintContext *pc)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	if (pc->f != NULL)
	{
		if (pc->is_pipe) {
			return pclose (pc->f);
		}
		else
		{
			return fclose (pc->f);
		}
		pc->f = NULL;
	} else {
		g_warning ("Could not close file. File unknown\n");
		return 0;
	}
}

void
gnome_print_bpath (GnomePrintContext * gpc, ArtBpath * bpath, gboolean append)
{
	ArtBpath * p;
	gboolean closed;

	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (bpath != NULL);

	if (bpath->code == ART_END) return;

	g_return_if_fail ((bpath->code == ART_MOVETO) || (bpath->code == ART_MOVETO_OPEN));

	closed = (bpath->code == ART_MOVETO);

	if (!append)
		gnome_print_newpath (gpc);

	gnome_print_moveto (gpc,
			    bpath->x3,
			    bpath->y3);

	for (p = bpath + 1; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			if (closed)
				gnome_print_closepath (gpc);

			closed = (p->code == ART_MOVETO);

			gnome_print_moveto (gpc,
					    p->x3,
					    p->y3);
			break;
		case ART_LINETO:
			gnome_print_lineto (gpc,
					    p->x3,
					    p->y3);
			break;
		case ART_CURVETO:
			gnome_print_curveto (gpc,
					     p->x1,
					     p->y1,
					     p->x2,
					     p->y2,
					     p->x3,
					     p->y3);
			break;
		default:
			g_log (G_LOG_DOMAIN,
			       G_LOG_LEVEL_CRITICAL,
			       "Invalid Bpath element");
			return;
			break;
		}
	}

	if (closed)
		gnome_print_closepath (gpc);

}



void
gnome_print_vpath (GnomePrintContext * gpc, ArtVpath * vpath, gboolean append)
{
	ArtVpath * p;
	gboolean closed;

	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_CONTEXT (gpc));
	g_return_if_fail (vpath != NULL);

	if (vpath->code == ART_END) return;

	g_return_if_fail ((vpath->code == ART_MOVETO) || (vpath->code == ART_MOVETO_OPEN));

	closed = (vpath->code == ART_MOVETO);

	if (!append)
		gnome_print_newpath (gpc);

	gnome_print_moveto (gpc,
			    vpath->x,
			    vpath->y);

	for (p = vpath + 1; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
			if (closed)
				gnome_print_closepath (gpc);

			closed = (p->code == ART_MOVETO);

			gnome_print_moveto (gpc,
					    p->x,
					    p->y);
			break;
		case ART_LINETO:
			gnome_print_lineto (gpc,
					    p->x,
					    p->y);
			break;
		default:
			g_log (G_LOG_DOMAIN,
			       G_LOG_LEVEL_CRITICAL,
			       "Invalid Vpath element");
			return;
			break;
		}
	}

	if (closed)
		gnome_print_closepath (gpc);

}

/*
 * WARNING: EXPERIMENTAL
 */

int
gnome_print_glyphlist (GnomePrintContext * pc, GnomeGlyphList * glyphlist)
{
	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (glyphlist != NULL, -1);
	g_return_val_if_fail (GNOME_IS_GLYPHLIST (glyphlist), -1);

	if (((GnomePrintContextClass *) ((GtkObject *) pc)->klass)->glyphlist) {
		return (* ((GnomePrintContextClass *) ((GtkObject *) pc)->klass)->glyphlist) (pc, glyphlist);
	} else {
		GnomePosGlyphList * pgl;
		gdouble affine[6];
		GSList * l;
		art_affine_identity (affine);
		pgl = gnome_pgl_from_gl (glyphlist, affine, 0);
		for (l = pgl->strings; l != NULL; l = l->next) {
			GnomePosString *string;
			GnomeFont *font;
			GnomeFontFace *face;
			gint i;
			string = (GnomePosString *) l->data;
			font = (GnomeFont *) gnome_rfont_get_font (string->rfont);
			face = (GnomeFontFace *) gnome_font_get_face (font);
			gnome_print_setfont (pc, font);
			for (i = 0; i < string->length; i++) {
				gdouble r, g, b, a;
				const gchar *psname;
				gchar utf[6];
				gint len;
				/* fixme: We need currentpoint here :( */
				gnome_print_moveto (pc, string->glyphs[i].x, string->glyphs[i].y);
				r = ((string->glyphs[i].color >> 24) & 0xff) / 255.0;
				g = ((string->glyphs[i].color >> 16) & 0xff) / 255.0;
				b = ((string->glyphs[i].color >>  8) & 0xff) / 255.0;
				a = ((string->glyphs[i].color >>  0) & 0xff) / 255.0;
				gnome_print_setrgbcolor (pc, r, g, b);
				gnome_print_setopacity (pc, a);
				psname = gnome_font_face_get_glyph_ps_name (face, string->glyphs[i].glyph);
				len = g_unichar_to_utf8 (gp_unicode_from_ps (psname), utf);
				gnome_print_show_sized (pc, utf, len);
			}
		}
		gnome_pgl_destroy (pgl);
	}

	return 0;
}
