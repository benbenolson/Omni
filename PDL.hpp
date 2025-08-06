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
#ifndef _PDL
#define _PDL

#include <iostream>
#include <sstream>
#include <string>

class PDL
{
public:
   enum {
      PDL_DONTCARE     = 0   /* Blank                                                      */
   };

   enum {
      PDL_other        =  1, /* Not on this list                                           */
      PDL_PCL          =  3, /* PCL.  Starting with PCL version 5, HP-GL/2 is included as  */
      PDL_HPGL         =  4, /* Hewlett-Packard Graphics Language.  HP-GL is a registered  */
      PDL_PJL          =  5, /* Peripheral Job Language.  Appears in the data stream       */
      PDL_PS           =  6, /* PostScript Language (tm) Postscript - a trademark of Adobe */
      PDL_IPDS         =  7, /* Intelligent Printer Data Stream Bi-directional print data  */
      PDL_PPDS         =  8, /* IBM Personal Printer Data Stream.  Originally called IBM   */
      PDL_EscapeP      =  9, /* Epson Corp.                                                */
      PDL_Epson        = 10, /* Epson Corp.                                                */
      PDL_DDIF         = 11, /* Digital Document Interchange Format Digital Equipment      */
      PDL_Interpress   = 12, /* Xerox Corp.                                                */
      PDL_ISO6429      = 13, /* ISO 6429.  Control functions for Coded Character Sets      */
      PDL_LineData     = 14, /* line-data : Lines of data as separate ASCII or EBCDIC      */
      PDL_MODCA        = 15, /* Mixed Object Document Content Architecture Definitions     */
      PDL_REGIS        = 16, /* Remote Graphics Instruction Set, Digital Equipment         */
      PDL_SCS          = 17, /* SNA Character String Bi-directional print data stream for  */
      PDL_SPDL         = 18, /* ISO 10180 Standard Page Description Language ISO           */
      PDL_TEK4014      = 19, /* Tektronix Corp.                                            */
      PDL_PDS          = 20,
      PDL_IGP          = 21, /* Printronix Corp.                                           */
      PDL_CodeV        = 22, /* Magnum Code-V, Image and printer control language used     */
      PDL_DSCDSE       = 23, /* DSC-DSE : Data Stream Compatible and Emulation Bi-         */
      PDL_WPS          = 24, /* Windows Printing System, Resource based command/data       */
      PDL_LN03         = 25, /* Early DEC-PPL3, Digital Equipment Corp.                    */
      PDL_CCITT        = 26,
      PDL_QUIC         = 27, /* QUIC (Quality Information Code), Page Description          */
      PDL_CPAP         = 28, /* Common Printer Access Protocol Digital Equipment Corp      */
      PDL_DecPPL       = 29, /* Digital ANSI-Compliant Printing Protocol (DEC-PPL)         */
      PDL_SimpleText   = 30, /* simple-text : character coded data, including NUL,         */
      PDL_NPAP         = 31, /* Network Printer Alliance Protocol (NPAP).  This protocol   */
      PDL_DOC          = 32, /* Document Option Commands, Appears in the data stream       */
      PDL_imPress      = 33, /* imPRESS, Page description language originally              */
      PDL_Pinwriter    = 34, /* 24 wire dot matrix printer for USA, Europe, and            */
      PDL_NPDL         = 35, /* Page printer for Japanese market.  NEC                     */
      PDL_NEC201PL     = 36, /* Serial printer language used in the Japanese market.       */
      PDL_Automatic    = 37, /* Automatic PDL sensing.  Automatic sensing of the           */
      PDL_Pages        = 38, /* Page printer Advanced Graphic Escape Set IBM Japan         */
      PDL_LIPS         = 39, /* LBP Image Processing System                                */
      PDL_TIFF         = 40, /* Tagged Image File Format (Aldus)                           */
      PDL_Diagnostic   = 41, /* A hex dump of the input to the interpreter                 */
      PDL_PSPrinter    = 42, /* The PostScript Language used for control (with any         */
      PDL_CaPSL        = 43, /* Canon Print Systems Language                               */
      PDL_EXCL         = 44, /* Extended Command Language Talaris Systems Inc              */
      PDL_LCDS         = 45, /* Line Conditioned Data Stream Xerox Corporatio              */
      PDL_XES          = 46, /* Xerox Escape Sequences Xerox Corporation                   */
      PDL_PCLXL        = 47, /* Printer Control Language.  Extended language features      */
      PDL_ART          = 48, /* Advanced Rendering Tools (ART).  Page Description          */
      PDL_TIPSI        = 49, /* Transport Independent Printer System Interface (ref.       */
      PDL_Prescribe    = 50, /* Page description and printer control language.  It         */
      PDL_LinePrinter  = 51, /* A simple-text character stream which supports the          */
      PDL_IDP          = 52, /* Imaging Device Protocol Apple Computer.                    */
      PDL_XJCL         = 53, /* Xerox Corp.                                                */
      /* ADD here and give proposal to IPP (KC) */
      PDL_ALPS         = 54, /* ALPS Corp.                                                 */
      PDL_Olivetti     = 55, /* Olivetti Corp.                                             */
      PDL_Deskjet      = 56, /* Hewlett-Packard Deskjet subset (IPCL)                      */
      PDL_Paintjet     = 57, /* Hewlett-Packard Paintjet subset                            */
      PDL_Seiko        = 58, /* Seiko Corp.                                                */
      PDL_PassThru     = 59, /* Does not alter data stream                                 */
      PDL_5577         = 60, /* 5577 Line and Serial Impact Dot Printer IBM Japan          */
      PDL_RPDL         = 61, /*                                                            */
   };

   /************************** LEVELS FOR PDL_PS **************************/
   enum {
      LEVEL_PS_LEVEL1         = 1,    /* Postscript Level  1*/
      LEVEL_PS_LEVEL2         = 2     /* Postscript Level  2*/
   };

   /************************* LEVELS for PDL_PCL **************************/
   enum {
      LEVEL_PCL2              = 1,
      LEVEL_PCL3              = 2,
      LEVEL_PCL3C             = 3,
      LEVEL_PCL4              = 4,
      LEVEL_PCL5              = 5,
      LEVEL_PCL5C             = 6,
      LEVEL_PCL6              = 7,
      LEVEL_PCL5E             = 8
   };

   /************************* LEVELS FOR PDL_HPGL *************************/
   enum {
      LEVEL_HPGL1             = 1,
      LEVEL_HPGL2             = 2,
      LEVEL_HPGL2_RTL         = 3,    /* HPGL2 with RTL language used for raster transfer */
      LEVEL_HPGL2_PCLRTL      = 4,    /* HPGL2 with PCL used for raster transfer */
      LEVEL_HPGL2_MC          = 5     /* HPGL2 with MC command support */
   };

   /************************ LEVELS FOR PDL_EPSON *************************/
   enum {
      LEVEL_ESC               = 1,
      LEVEL_ESCP              = 2,
      LEVEL_ESCP_2            = 3,
      LEVEL_ESCP_2J           = 4     /* Japan version */
   };

   /********************** LEVELS FOR PDL_SimpleText **********************/
   enum {
      LEVEL_ASCII_TEXT        = 1,
      LEVEL_ASCII_PROPRINTER  = 2,
      LEVEL_ASCII_QUITWRITER  = 3,
      LEVEL_ASCII_JISASCII    = 4
   };

   /*********************** LEVELS FOR PDL_Deskjet ************************/
   enum {
      LEVEL_DESKJET           = 1,
      LEVEL_DESKJETJ          = 2     /* Japan version */
   };

                        PDL                     (int                  iPDLLevel,
                                                 int                  iPDLSubLevel,
                                                 int                  iMajorRevisionLevel,
                                                 int                  iMinorRevisionLevel);

   int                  getPDLLevel              ();
   int                  getPDLSubLevel           ();
   int                  getPDLMajorRevisionLevel ();
   int                  getPDLMinorRevisionLevel ();

   static bool          isReservedKeyword        (const char         *pszId);
   static int           getReservedValue         (const char         *pszId);

#ifndef RETAIL
   void                 outputSelf               ();
#endif
   virtual std::string  toString                 (std::ostringstream& oss);
   friend std::ostream& operator<<               (std::ostream&       os,
                                                  const PDL&          self);

private:
   int iPDLLevel_d;
   int iPDLSubLevel_d;
   int iMajorRevisionLevel_d;
   int iMinorRevisionLevel_d;
};

#endif
