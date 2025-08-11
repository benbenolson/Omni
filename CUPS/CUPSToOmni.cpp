/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2001
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

/*
 * # gdb debugging trick: force cups/omni to pause before printing
 * # (gdb) file CUPSToOmni
 * # (gdb) shell ps -efl | grep CUPSToOmni       to get the <pid>.
 * # (gdb) attach <pid>
 * # (gdb) set fPause = 0
 * # (gdb) c
 * mv /usr/lib/cups/filter/CUPSToOmni /usr/lib/cups/filter/CUPSToOmni.orig
 * echo "#!/bin/bash"                              > /usr/lib/cups/filter/CUPSToOmni
 * echo "export OMNI_CUPS_SERVER_PAUSE=1"         >> /usr/lib/cups/filter/CUPSToOmni
 * echo "/usr/lib/cups/filter/CUPSToOmni.orig $*" >> /usr/lib/cups/filter/CUPSToOmni
 * chmod +x /usr/lib/cups/filter/CUPSToOmni
 *
 * (cd CUPS/; make; /bin/cp .libs/CUPSToOmni /usr/lib/cups/filter/; /bin/cp .libs/CUPSToOmni /usr/bin/)
 *
 * lp.cups -o PageSize=DFO1_144 /home/ghostscript/test/examples/bunny.ps
 *
 * vi /var/log/cups/error_log
 *    search for CUPSToOmni
 * grep "\[Job 3\] CUPSToOmni: " /var/log/cups/error_log
 */

/*
 * @BUG
 * file:     cups-1.1.19/scheduler/ipp.c
 * function: copy_model()
 *
 * CUPS will blindly alter DefaultPageSize, DefaultPageRegion, DefaultPaperDimension,
 * and DefaultImageableArea to "Letter".  This assumes that "Letter" is a valid
 * key for the PPD file!
 */

#include "Device.hpp"
#include "Omni.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <fcntl.h>

#include <glib.h>
#include <gmodule.h>

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>

#define DEBUG_LOGFILE "/tmp/CUPSToOmni.debug.log"

typedef struct _HWMAR {
   float fLeftClip;
   float fBottomClip;
   float fRightClip;
   float fTopClip;
   float fxWidth;
   float fyHeight;
} HWMARGINS;

typedef struct _HWRES {
   float xRes;
   float yRes;
   float fScanDots;  // number of dots in scan line
} HWRESOLUTION;

typedef struct _PRTMODE {
   int iBitCount;
   int iPlanes;
} PRINTMODE;

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/* Function: FindBandSize                                                      */
/*  This function figures out the appropriate band size based on the amount    */
/*  of memory that can be occupied by the band and bitmap information.         */
/*  bitmap, and the specifics of the hardware                                  */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
ULONG
FindBandSize (ULONG ulYHeight,
              ULONG ulXWidth,
              ULONG ulBitsPerPel,
              ULONG ulNumPlanes,
              ULONG ulModulus,
              ULONG ulMemoryLimit)
{
   ULONG ulSizeScanLine;
   ULONG ulMemoryNeeded;
   ULONG ulNumLinesFit;

   if (0 == ulModulus)
   {
      ulModulus = 1;
   }

   // figure out how much memory is needed fore each line
   ulSizeScanLine = ((ulBitsPerPel * ulXWidth + 31) / 32) * ulNumPlanes * 4;

   // Figure out how much memory is needed for the page
   ulMemoryNeeded = ulYHeight * ulSizeScanLine;

   // How many lines can fit in the size given?
   ulNumLinesFit = ulMemoryLimit / ulSizeScanLine;

   if (0 == ulNumLinesFit)
      // Minimum of 1 scan line
      ulNumLinesFit = 1;

   if (ulNumLinesFit <= ulModulus)
      // Not enough lines... Promote it to a modulus.
      ulNumLinesFit = ulModulus;
   else
      // Bump down the number of lines so that it is a modulus.
      ulNumLinesFit -= ulNumLinesFit % ulModulus;

   if ((ulYHeight % ulNumLinesFit) * 100 / ulYHeight <= 15)
   {
      USHORT usBumpUp;

      usBumpUp  = ulYHeight % ulNumLinesFit;
      usBumpUp += ulModulus - 1;
      usBumpUp /= ulModulus;
      usBumpUp *= ulModulus;

      ulNumLinesFit += usBumpUp;
   }

   return ulNumLinesFit;  // return the number of lines we want for the band
}

int
main (int argc, char *argv[])
{
#define MAX_SCAN_LINES 1024
   PSZRO               pszPPDFilename         = 0; /* PPD environment variable   */
   ppd_file_t         *ppd                    = 0; /* PPD file                   */
   int                 fdInput                = 0; /* File descriptor            */
   cups_raster_t      *pRasterStream          = 0; /* Raster stream to read from */
   cups_page_header_t  pageHeader;                 /* Page header from file      */
   cups_page_header_t  pageHeaderCurrent;          /* Page header from file      */
   int                 num_options            = 0; /* Number of print options    */
   ppd_choice_t	    *choice                 = 0; /* PPD option choice          */
   cups_option_t      *options                = 0; /* Print options              */
   const char         *val                    = 0; /* Option value               */
   char               *pszLibraryName         = 0;
   GModule            *hmodDevice             = 0;
   Device             *pDevice                = 0;
   Device             *pDeviceTmp             = 0;
   PFNNEWDEVICEWARGS   pfnNewDeviceWArgs      = 0;
   int                 iPageNumber            = 0;
   unsigned int        uiScanline             = 0;
   unsigned int        iNumScanLines          = MAX_SCAN_LINES;
   PBYTE               pbBits                 = 0;
   PBYTE               pbBitsToSend           = 0;
   PBITMAPINFO2        pbmi2Bitmap            = 0;
   int                 iNumColors             = 0;
   int                 icbBitmapScanLine      = 0;
   int                 iBytesToAllocate       = 0;
   RECTL               rectlPageLocation;
   HWRESOLUTION        hwRes;
   DeviceResolution   *pDevRes                = 0; // Device Resolution stuff
   HardCopyCap        *pHCC                   = 0; // Hard copy information
   unsigned long       ulBytesRead            = 0;
   int                 i,
                       j;
   bool                fTopOfBand             = false;
#ifdef DEBUG
   int                 fDebugOutput           = 0;
   FILE               *fpCUPSDebug            = 0;
#endif
   int                 fDoOutput              = 0;
   int                 rc                     = 0;
   PSZRO               pszJobProperties       = 0;
   JobProperties       jobProp ("");
   ppd_attr_t         *ppdattrOmniLibrary     = 0;
   ppd_attr_t         *ppdattrOmniJobProp     = 0;
   std::ostream&       cerrReal               = std::cerr;

   if (getenv ("OMNI_CUPS_SERVER_PAUSE"))
   {
      // Handy to attach via gdb debugger.  do:
      // (gdb) file omni
      // (gdb) shell ps -efl.  Look for: 100 R lp <pid> 9295 83 85 0 - 1263 - 11:09 ? 00:01:15 bob 44 hamzy tiger.ps 1
      // (gdb) attach <pid>
      // (gdb) set fPause = 0
      // (gdb) c
      bool fPause = true;

      while (fPause)
      {
      }
   }

   cerrReal << "DEBUG: CUPSToOmni: Starting a print job." << std::endl;

#ifdef DEBUG
   fDebugOutput = 1;
#endif

#ifdef DEBUG
   if (fDebugOutput)
   {
      fpCUPSDebug = fopen (DEBUG_LOGFILE, "wb");

      DebugOutput::setErrorStream (fpCUPSDebug);
      DebugOutput::setDebugOutput ("all");
   }
#endif

#ifdef DEBUG
   if (fDebugOutput)
   {
      fprintf (fpCUPSDebug, "CUPSToOmni: Args = %d\n", argc);
      for (i = 0; i < argc; i++)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: Arg %d = \"%s\"\n", i, argv[i]);
      }
   }
#endif
   cerrReal << "DEBUG: CUPSToOmni: Args = "
            << argc;
   for (i = 0; i < argc; i++)
   {
      cerrReal << ", \"" << SAFE_PRINT_PSZ(argv[i]) << "\"";
   }
   cerrReal << std::endl;

   if (  argc < 5
      || argc > 7
      )
   {
      /*
       * We don't have the correct number of arguments; write an error message
       * and return.
       */
#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "ERROR: CUPSToOmni job-id user title copies options [file]\n");
         fprintf (fpCUPSDebug, "       and argc = %d\n", argc);
      }
#endif
      cerrReal << "ERROR: CUPSToOmni: job-id user title copies options [file]" << std::endl;
      cerrReal << "ERROR: CUPSToOmni: and argc = " << argc << std::endl;

      rc = 1;
      goto done;
   }

   Omni::initialize ();

   options     = 0;
   num_options = cupsParseOptions (argv[5], 0, &options);

   if (cupsGetOption ("no-output", num_options, options))
      fDoOutput = 0;
   else
      fDoOutput = 1;

#ifdef DEBUG
   if (cupsGetOption ("no-debug", num_options, options))
      fDebugOutput = 0;
#endif

#ifdef DEBUG
   if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: fDoOutput = %d\n", fDoOutput);
   cerrReal << "DEBUG: CUPSToOmni: fDoOutput = " << fDoOutput << ", fDebugOutput = " << fDebugOutput << std::endl;
#endif

   Omni::addOmniToPATH ();

   /*
    * Get the PPD file and figure out which driver to use...
    */
   if ((pszPPDFilename = getenv ("PPD")) == NULL)
   {
#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "\n");
      }
#endif
      cerrReal << "ERROR: CUPSToOmni: Fatal error: PPD environment variable not set!" << std::endl;

      rc = 1;
      goto done;
   }

   if ((ppd = ppdOpenFile (pszPPDFilename)) == NULL)
   {
#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "ERROR: Fatal error: Unable to load PPD file %s\n", pszPPDFilename);
      }
#endif
      cerrReal << "ERROR: CUPSToOmni: Fatal error: Unable to load PPD file " << SAFE_PRINT_PSZ(pszPPDFilename) << std::endl;

      rc = 1;
      goto done;
   }

#ifdef DEBUG
   if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: successful ppd open\n");
#endif

   ppdattrOmniLibrary = ppdFindAttr (ppd, "OmniLibrary", 0);
   ppdattrOmniJobProp = ppdFindAttr (ppd, "OmniJobProperties", 0);

#ifdef DEBUG
   if (fDebugOutput)
   {
      fprintf (fpCUPSDebug, "OmniJobLibrary    = \"%s\"\n", (ppdattrOmniLibrary && ppdattrOmniLibrary->value ? ppdattrOmniLibrary->value : "(null)"));
      fprintf (fpCUPSDebug, "OmniJobProperties = \"%s\"\n", (ppdattrOmniJobProp && ppdattrOmniJobProp->value ? ppdattrOmniJobProp->value : "(null)"));
   }
#endif

   if (  !ppdattrOmniLibrary
      || !ppdattrOmniLibrary->value
      )
   {
#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "ERROR: Fatal error: No OmniLibrary attribute in PPD file %s\n", pszPPDFilename);
      }
#endif
      cerrReal << "ERROR: CUPSToOmni: Fatal error: No OmniLibrary attribute in PPD file " << SAFE_PRINT_PSZ(pszPPDFilename) << std::endl;

      rc = 1;
      goto done;
   }

   pszLibraryName = ppdattrOmniLibrary->value;

   if (  !ppdattrOmniJobProp
      || !ppdattrOmniJobProp->value
      )
   {
#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "ERROR: Fatal error: No OmniJobProperties attribute in PPD file %s\n", pszPPDFilename);
      }
#endif
      cerrReal << "ERROR: CUPSToOmni: Fatal error: No OmniJobProperties attribute in PPD file " << SAFE_PRINT_PSZ(pszPPDFilename) << std::endl;

      rc = 1;
      goto done;
   }
   else
   {
      PSZCRO pszDequoted = Omni::dequoteString (ppdattrOmniJobProp->value);

      if (pszDequoted)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "OmniJobProperties = \"%s\"\n", pszDequoted);
#endif
         jobProp.setJobProperties (pszDequoted);

         free ((void *)pszDequoted);
      }
   }

   cerrReal << "DEBUG: CUPSToOmni: The library name is \"" << SAFE_PRINT_PSZ(pszLibraryName) << "\"" << std::endl;

   hmodDevice = g_module_open (pszLibraryName, (GModuleFlags)0);

#ifdef DEBUG
   if (fDebugOutput)
   {
      if (hmodDevice)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: successful driver load\n");
      }
      else
      {
         fprintf (fpCUPSDebug, "ERROR: CUPSToOmni: unsuccessful driver load\n");
      }
   }
#endif

   if (!hmodDevice)
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Error: unsuccessful driver load\"%s\"\n", g_module_error ());
#endif

      cerrReal << "ERROR: CUPSToOmni: Fatal error: unable to load \""
               << SAFE_PRINT_PSZ(pszLibraryName)
               << "\", g_module_error returns \""
               << g_module_error ()
               << "\""
               << std::endl;

      rc = 1;
      goto done;
   }

   g_module_symbol (hmodDevice,
                    "newDeviceW_JopProp_Advanced",
                    (gpointer *)&pfnNewDeviceWArgs);

#ifdef DEBUG
   if (fDebugOutput)
   {
      if (pfnNewDeviceWArgs)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: set up driver successful\n");
      }
      else
      {
         fprintf (fpCUPSDebug, "ERROR: CUPSToOmni: set up driver unsuccessful!\n");
      }
   }
#endif

   if (!pfnNewDeviceWArgs)
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Error: missing newDeviceW_JopProp_Advanced from %s\n", pszLibraryName);
#endif

      cerrReal << "ERROR: CUPSToOmni: Fatal error: " << SAFE_PRINT_PSZ(pszLibraryName) << " is not a valid library" << std::endl;

      rc = 1;
      goto done;
   }

   /*
    * Open the page stream...
    */
   if (argc == 7)
   {
      if ((fdInput = open (argv[6], O_RDONLY)) == -1)  //open the raster file
      {
         perror ("ERROR: Fatal error: Unable to open raster file - ");
         sleep (1);

         rc = 1;
         goto done;
      }
   }
   else
   {
      fdInput = 0;
   }

   pRasterStream = cupsRasterOpen (fdInput, CUPS_RASTER_READ);

#ifdef DEBUG
   if (fDebugOutput)
   {
      if (!pRasterStream)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: Warning: cupsRasterOpen Failed.\n");
      }
   }
#endif

   pszJobProperties = jobProp.getJobProperties ();

   if (pszJobProperties)
   {
      // true for non-pdc implementation
      pDeviceTmp = pfnNewDeviceWArgs (pszJobProperties, true);

      free ((void *)pszJobProperties);
   }

#ifdef DEBUG
   fprintf (fpCUPSDebug, "CUPSToOmni: pszJobProperties = %08X, pDeviceTmp = %08x\n", (int)pszJobProperties, (int)pDeviceTmp);
#endif
   cerrReal << "DEBUG: CUPSToOmni: pszJobProperties = 0x" << std::hex << (int)pszJobProperties << ", pDeviceTmp = 0x" << (int)pDeviceTmp << std::dec << std::endl;

   ppdMarkDefaults (ppd);

   val = cupsGetOption ("PageSize", num_options, options);
   if (val == NULL)
   {
      choice = ppdFindMarkedChoice (ppd, "PageSize");
      if (choice != NULL)
      {
         val = choice->choice;

#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD default PageSize = %s\n", val);
#endif
         cerrReal << "DEBUG: CUPSToOmni: Postscript PPD default PageSize = " << SAFE_PRINT_PSZ(val) << std::endl;
      }
      else
      {
#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Error: Could not find marked PageSize\n");
#endif
         cerrReal << "DEBUG: CUPSToOmni: Error: Could not find marked PageSize" << std::endl;
      }
   }
   else
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD command line PageSize = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD command line PageSize = " << SAFE_PRINT_PSZ(val) << std::endl;
   }

   if (  val != NULL
      && pDeviceTmp
      )
   {
      DeviceForm  *pFormNew     = 0;
      DeviceForm  *pFormCurrent = 0;
      std::string *pstringJP    = 0;

      pFormCurrent = pDeviceTmp->getCurrentForm ();
      if (pFormCurrent)
      {
         pFormNew = pFormCurrent->createWithHash (pDeviceTmp, val);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing pFormNew!" << std::endl;
      }

      if (pFormNew)
      {
         pstringJP = pFormNew->getJobProperties (false);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing form's pstringJP!" << std::endl;
      }

      if (pstringJP)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: setting new omni form = %s\n", pstringJP->c_str ());
#endif
         cerrReal << "DEBUG: CUPSToOmni: setting new omni form \"" << SAFE_PRINT_PSZ(pstringJP->c_str ()) << "\"" << std::endl;

         jobProp.setJobProperty (pstringJP->c_str ());
      }

      delete pFormNew;
      delete pstringJP;
   }

   val = cupsGetOption ("MediaType", num_options, options);
   if (val == NULL)
   {
      choice = ppdFindMarkedChoice (ppd, "MediaType");
      if (choice != NULL)
      {
         val = choice->choice;

#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD default MediaType = %s\n", val);
#endif
         cerrReal << "DEBUG: CUPSToOmni: Postscript PPD default MediaType = " << SAFE_PRINT_PSZ(val) << std::endl;
      }
      else
      {
#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Error: Could not find marked MediaType\n");
#endif
         cerrReal << "DEBUG: CUPSToOmni: Error: Could not find marked MediaType" << std::endl;
      }
   }
   else
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD command line MediaType = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD command line MediaType = " << SAFE_PRINT_PSZ(val) << std::endl;
   }

   if (  val != NULL
      && pDeviceTmp
      )
   {
      DeviceMedia *pMediaNew     = 0;
      DeviceMedia *pMediaCurrent = 0;
      std::string *pstringJP     = 0;

      pMediaCurrent = pDeviceTmp->getCurrentMedia ();
      if (pMediaCurrent)
      {
         pMediaNew = pMediaCurrent->createWithHash (pDeviceTmp, val);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing pMediaNew!" << std::endl;
      }

      if (pMediaNew)
      {
         pstringJP = pMediaNew->getJobProperties (false);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing media's pstringJP!" << std::endl;
      }

      if (pstringJP)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: setting new omni media = %s\n", pstringJP->c_str ());
#endif
         cerrReal << "DEBUG: CUPSToOmni: setting new omni media \"" << SAFE_PRINT_PSZ(pstringJP->c_str ()) << "\"" << std::endl;

         jobProp.setJobProperty (pstringJP->c_str ());
      }

      delete pMediaNew;
      delete pstringJP;
   }

   val = cupsGetOption ("InputSlot", num_options, options);
   if (val == NULL)
   {
      choice = ppdFindMarkedChoice (ppd, "InputSlot");
      if (choice != NULL)
      {
         val = choice->choice;

#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD default InputSlot = %s\n", val);
#endif
         cerrReal << "DEBUG: CUPSToOmni: Postscript PPD default InputSlot = " << SAFE_PRINT_PSZ(val) << std::endl;
      }
      else
      {
#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Error: Could not find marked InputSlot\n");
#endif
         cerrReal << "DEBUG: CUPSToOmni: Error: Could not find marked InputSlot" << std::endl;
      }
   }
   else
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD command line InputSlot = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD command line InputSlot = " << SAFE_PRINT_PSZ(val) << std::endl;
   }

   if (  val != NULL
      && pDeviceTmp
      )
   {
      DeviceTray  *pTrayNew     = 0;
      DeviceTray  *pTrayCurrent = 0;
      std::string *pstringJP    = 0;

      pTrayCurrent = pDeviceTmp->getCurrentTray ();
      if (pTrayCurrent)
      {
         pTrayNew = pTrayCurrent->createWithHash (pDeviceTmp, val);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing pTrayNew!" << std::endl;
      }

      if (pTrayNew)
      {
         pstringJP = pTrayNew->getJobProperties (false);
      }
      else
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing tray's pstringJP!" << std::endl;
      }

      if (pstringJP)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: setting new omni tray = %s\n", pstringJP->c_str ());
#endif
         cerrReal << "DEBUG: CUPSToOmni: setting new omni tray \"" << SAFE_PRINT_PSZ(pstringJP->c_str ()) << "\"" << std::endl;

         jobProp.setJobProperty (pstringJP->c_str ());
      }

      delete pTrayNew;
      delete pstringJP;
   }

   val = cupsGetOption ("Dither", num_options, options);
   if (val == NULL)
   {
      choice = ppdFindMarkedChoice (ppd, "Dither");
      if (choice != NULL)
      {
         val = choice->choice;

#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD default Dither = %s\n", val);
#endif
         cerrReal << "DEBUG: CUPSToOmni: Postscript PPD default Dither = " << SAFE_PRINT_PSZ(val) << std::endl;
      }
      else
      {
#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Error: Could not find marked Dither\n");
#endif
         cerrReal << "DEBUG: CUPSToOmni: Error: Could not find marked Dither" << std::endl;
      }
   }
   else
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD command line Dither = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD command line Dither = " << SAFE_PRINT_PSZ(val) << std::endl;
   }

   if (val != NULL)
   {
      PSZRO       pszCUPSDither    = 0;
      std::string stringDitherHash = val;

#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD option Dither = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD option Dither = " << SAFE_PRINT_PSZ(val) << std::endl;

      pszCUPSDither = DeviceDither::getIDFromCreateHash (&stringDitherHash);
      if (!pszCUPSDither)
      {
         cerrReal << "DEBUG: CUPSToOmni: Error: missing pszCUPSDither!" << std::endl;
      }

      if (pszCUPSDither)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: setting new omni dither = \"dither=%s\"\n", pszCUPSDither);
#endif
         cerrReal << "DEBUG: CUPSToOmni: setting new omni dither \"dither=" << SAFE_PRINT_PSZ(pszCUPSDither) << "\"" << std::endl;

         jobProp.setJobProperty ("dither", pszCUPSDither);
      }
   }

   val = cupsGetOption ("Resolution", num_options, options);
   if (val == NULL)
   {
      choice = ppdFindMarkedChoice (ppd, "Resolution");
      if (choice != NULL)
      {
         val = choice->choice;

#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD default Resolution = %s\n", val);
#endif
         cerrReal << "DEBUG: CUPSToOmni: Postscript PPD default Resolution = " << SAFE_PRINT_PSZ(val) << std::endl;
      }
      else
      {
#ifdef DEBUG
         fprintf (fpCUPSDebug, "CUPSToOmni: Error: Could not find marked Resolution\n");
#endif
         cerrReal << "DEBUG: CUPSToOmni: Error: Could not find marked Resolution" << std::endl;
      }
   }
   else
   {
#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD command line Resolution = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD command line Resolution = " << SAFE_PRINT_PSZ(val) << std::endl;
   }

   if (  val != NULL
      && pDeviceTmp
      )
   {
      PSZRO pszCUPSResolution = val;

#ifdef DEBUG
      fprintf (fpCUPSDebug, "CUPSToOmni: Postscript PPD option Resolution = %s\n", val);
#endif
      cerrReal << "DEBUG: CUPSToOmni: Postscript PPD option Resolution = " << SAFE_PRINT_PSZ(val) << ", pDeviceTmp = " << std::hex << pDeviceTmp << std::dec << std::endl;

      if (pDeviceTmp)
      {
         DeviceResolution *pResolutionNew     = 0;
         DeviceResolution *pResolutionCurrent = 0;
         std::string      *pstringJP          = 0;

         pResolutionCurrent = pDeviceTmp->getCurrentResolution ();
         if (pResolutionCurrent)
         {
            pResolutionNew = pResolutionCurrent->createWithHash (pDeviceTmp, pszCUPSResolution);
         }
         else
         {
            cerrReal << "DEBUG: CUPSToOmni: Error: missing pResolutionNew!" << std::endl;
         }

         if (pResolutionNew)
         {
            pstringJP = pResolutionNew->getJobProperties (false);
         }
         else
         {
            cerrReal << "DEBUG: CUPSToOmni: Error: missing resolution's pstringJP!" << std::endl;
         }

         if (pstringJP)
         {
#ifdef DEBUG
            if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: setting new omni resolution = %s\n", pstringJP->c_str ());
#endif
            cerrReal << "DEBUG: CUPSToOmni: setting new omni resolution \"" << SAFE_PRINT_PSZ(pstringJP->c_str ()) << "\"" << std::endl;

            jobProp.setJobProperty (pstringJP->c_str ());
         }

         delete pResolutionNew;
         delete pstringJP;
      }
   }

#ifdef DEBUG
   if (fDebugOutput)
   {
      fprintf (fpCUPSDebug, "CUPSToOmni: processing pages...\n");
   }
#endif

   /*
    * Process pages as needed...
    */
   while (cupsRasterReadHeader (pRasterStream, &pageHeader))
   {
      /***************************************************************************/
      /*                                                                         */
      /* Current Monochrome selection is:                                        */
      /*   -o ColorModel=Gray                                                    */
      /*                                                                         */
      /* We are currently using the FORM_ strings and such to invoke Omni        */
      /*                                                                         */
      /* Formulate a string to pass into the driver based on the following       */
      /* parameter list                                                          */
      /*                                                                         */
      /*   - form:                                                               */
      /*                                                                         */
      /*     form=FORM_LETTER or form=FORM_A4                                    */
      /*                                                                         */
      /*   - resolution:                                                         */
      /*                                                                         */
      /*     resolution=RESOLUTION_360_X_360 or resolution=RESOLUTION_180_X_180  */
      /*                                                                         */
      /*   - media:                                                              */
      /*                                                                         */
      /*     media=MEDIA_PLAIN or media=MEDIA_GLOSSY                             */
      /*                                                                         */
      /*   - dithering:                                                          */
      /*                                                                         */
      /*     dither=DITHER_STUCKI_DIFFUSION or dither=DITHER_STEINBERG_DIFFUSION */
      /*                                                                         */
      /*   - tray:                                                               */
      /*                                                                         */
      /*     tray=TRAY_MANUAL_FEEDER                                             */
      /*                                                                         */
      /*   - printmode:                                                          */
      /*                                                                         */
      /*     printmode=PRINT_MODE_1_ANY (**) or printmode=PRINT_MODE_24_CMYK     */
      /*                                                                         */
      /***************************************************************************/

      // we need to do preliminary driver setup here
      if (iPageNumber == 0)
      {
         cerrReal << "DEBUG: CUPSToOmni: cupsBitsPerPixel = " << pageHeader.cupsBitsPerPixel << std::endl;
         if (pageHeader.cupsBitsPerPixel == 1)
         {
            jobProp.setJobProperty ("printmode", "PRINT_MODE_1_ANY");
         }
         else
         {
            // @TBD
         }

         pszJobProperties = jobProp.getJobProperties ();

         cerrReal << "DEBUG: CUPSToOmni: The job properties are \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\"" << std::endl;

#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: pDevice = 0x%08X, Calling NewDeviceWArgs with \"%s\"\n", (int)pDevice, pszJobProperties);
#endif

         // true for non-pdc implementation
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

#ifdef DEBUG
         if (fDebugOutput)
         {
            if (pDevice)
            {
               fprintf (fpCUPSDebug, "CUPSToOmni: set up of new driver properties successful\n");
            }
         }
#endif
         if (pDevice)
         {
             cerrReal << "DEBUG: CUPSToOmni: set up of new driver properties successful" << std::endl;
         }

         if (!pDevice)
         {
#ifdef DEBUG
            fprintf (fpCUPSDebug, "CUPSToOmni: ERROR: Creating a new device with \"%s\" failed\n", pszJobProperties);
#endif

            cerrReal << "ERROR: CUPSToOmni: Fatal error: Creating a new device with \"" << SAFE_PRINT_PSZ(pszJobProperties) << "\" failed" << std::endl;

            rc = 1;
            goto done;
         }

         if (pszJobProperties)
         {
            free ((void *)pszJobProperties);
            pszJobProperties = 0;
         }

         pageHeaderCurrent.cupsColorSpace        = pageHeader.cupsColorSpace;
         pageHeaderCurrent.cupsWidth             = pageHeader.cupsWidth;
         pageHeaderCurrent.cupsHeight            = pageHeader.cupsHeight;
         pageHeaderCurrent.cupsBitsPerColor      = pageHeader.cupsBitsPerColor;
         pageHeaderCurrent.cupsBytesPerLine      = pageHeader.cupsBytesPerLine;
         pageHeaderCurrent.cupsBitsPerPixel      = pageHeader.cupsBitsPerPixel;
         strcpy (pageHeaderCurrent.MediaClass, pageHeader.MediaClass);
         strcpy (pageHeaderCurrent.MediaType,  pageHeader.MediaType);
         strcpy (pageHeaderCurrent.OutputType, pageHeader.OutputType);
         pageHeaderCurrent.HWResolution[0]       = pageHeader.HWResolution[0];
         pageHeaderCurrent.HWResolution[1]       = pageHeader.HWResolution[1];
         pageHeaderCurrent.PageSize[0]           = pageHeader.PageSize[0];
         pageHeaderCurrent.PageSize[1]           = pageHeader.PageSize[1];
         pageHeaderCurrent.ImagingBoundingBox[0] = pageHeader.ImagingBoundingBox[0];
         pageHeaderCurrent.ImagingBoundingBox[1] = pageHeader.ImagingBoundingBox[1];
         pageHeaderCurrent.ImagingBoundingBox[2] = pageHeader.ImagingBoundingBox[2];
         pageHeaderCurrent.ImagingBoundingBox[3] = pageHeader.ImagingBoundingBox[3];

         delete pDeviceTmp;
      }

#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: HW Res                 = %u x %u\n", pageHeader.HWResolution[0],
                                                                                 pageHeader.HWResolution[1]);
         fprintf (fpCUPSDebug, "CUPSToOmni: page size              = %d x %d\n", (int)pageHeader.PageSize[0],
                                                                                 (int)pageHeader.PageSize[1]);
         fprintf (fpCUPSDebug, "CUPSToOmni: Bounding Box           = %u %u %u %u\n", pageHeader.ImagingBoundingBox[0],
                                                                                     pageHeader.ImagingBoundingBox[1],
                                                                                     pageHeader.ImagingBoundingBox[2],
                                                                                     pageHeader.ImagingBoundingBox[3]);

         fprintf (fpCUPSDebug, "CUPSToOmni: cupsColorSpace         = %d\n", pageHeader.cupsColorSpace);
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsWidth              = %d\n", pageHeader.cupsWidth);
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsHeight             = %d\n", pageHeader.cupsHeight);
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsBitsPerColor       = %d\n", pageHeader.cupsBitsPerColor);
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsBytesPerLine       = %d\n", pageHeader.cupsBytesPerLine);
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsBitsPerPixel       = %d\n", pageHeader.cupsBitsPerPixel);
         fprintf (fpCUPSDebug, "CUPSToOmni: MediaClass             = %s\n", pageHeader.MediaClass);
         fprintf (fpCUPSDebug, "CUPSToOmni: MediaType              = %s\n", pageHeader.MediaType);
         fprintf (fpCUPSDebug, "CUPSToOmni: OutputType             = %s\n", pageHeader.OutputType);
      }
#endif

//////iNumScanLines = omni::min (MAX_SCAN_LINES, (int)(pageHeader.cupsHeight - uiScanline));
      iNumColors    = 1 << pageHeaderCurrent.cupsBitsPerPixel;

#ifdef DEBUG
      if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: Number of colors       = %d\n", iNumColors);
#endif

      if (!pbmi2Bitmap)
      {
         // Allocate a full bitmap info 2 and bitmap header message
         iBytesToAllocate = sizeof (BITMAPINFO2);
         if (256 >= iNumColors)
            iBytesToAllocate += (iNumColors - 1) * sizeof (RGB2);

         pbmi2Bitmap = (PBITMAPINFO2)calloc (1, iBytesToAllocate);
         if (!pbmi2Bitmap)
         {
#ifdef DEBUG
            fprintf (fpCUPSDebug, "CUPSToOmni: Error: failed to allocate bitmap info memory!\n");
#endif

            cerrReal << "ERROR: CUPSToOmni: Fatal error: failed to allocate bitmap info memory!" << std::endl;

            rc = 1;
            goto done;
         }

         // Initialize it
         pbmi2Bitmap->cbFix         = sizeof (BITMAPINFO2)
                                    - sizeof (((PBITMAPINFO2)0)->argbColor);

         pbmi2Bitmap->cx            = pageHeaderCurrent.cupsWidth;
         pbmi2Bitmap->cPlanes       = 1;
         pbmi2Bitmap->cBitCount     = pageHeaderCurrent.cupsBitsPerPixel;
         pbmi2Bitmap->cclrUsed      = 1 << pbmi2Bitmap->cBitCount;
         pbmi2Bitmap->cclrImportant = 0;                            // All are important

         // Read the color table, if there is one
         // @TBD
      }

      icbBitmapScanLine = (((pbmi2Bitmap->cx) * pbmi2Bitmap->cBitCount + 31) >> 5) << 2;

      iPageNumber++;
      uiScanline = 0;

      if (1 == iPageNumber)
      {
         pDevice->beginJob ();

#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: Begin job.\n");
#endif

         cerrReal << "DEBUG: CUPSToOmni: Begin job." << std::endl;
      }

      // get the hardware resolution
      pDevRes = pDevice->getCurrentResolution ();

      hwRes.xRes      = pDevRes->getXRes ();
      hwRes.yRes      = pDevRes->getYRes ();
      hwRes.fScanDots = pDevRes->getScanlineMultiple (); // need head pass information

      // get the hardware page margins information
      // **** might not currently need this info
      pHCC = pDevice->getCurrentForm ()->getHardCopyCap ();

      if (iPageNumber == 1)
      {
         // need to calculate the correct bandsize needed for the device
         iNumScanLines = FindBandSize (pageHeaderCurrent.cupsHeight,
                                       pageHeaderCurrent.cupsWidth,
                                       24,           // always use 24 bits for source bitmap (for now)
                                       1,            // number of planes
                                       (unsigned)hwRes.fScanDots,
                                       4000 * 1024); // eight meg buffer
      }

#ifdef DEBUG
      if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: bandsize               = %d\n", iNumScanLines);
#endif

      if (!pbBits)
      {
         iBytesToAllocate = icbBitmapScanLine * iNumScanLines;

         pbBits = (PBYTE)calloc (1, iBytesToAllocate);
         if (!pbBits)
         {
#ifdef DEBUG
            fprintf (fpCUPSDebug, "CUPSToOmni: Error: failed to allocate bitmap memory!\n");
#endif

            cerrReal << "ERROR: CUPSToOmni: Fatal error: failed to allocate bitmap memory!" << std::endl;

            rc = 1;
            goto done;
         }
      }

#ifdef DEBUG
      if (fDebugOutput)
      {
         fprintf (fpCUPSDebug, "CUPSToOmni: cupsBytesPerLine       = %d\n", pageHeaderCurrent.cupsBytesPerLine);
         fprintf (fpCUPSDebug, "CUPSToOmni: icbBitmapScanLine      = %d\n", icbBitmapScanLine);
      }
#endif

      uiScanline = pageHeaderCurrent.cupsHeight - 1;

      rectlPageLocation.xLeft  = 0;
      rectlPageLocation.xRight = pbmi2Bitmap->cx;

      while (uiScanline >= 0)
      {
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: uiScanline             = %u\n", uiScanline);
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: pbmi2Bitmap->cx        = %d\n", pbmi2Bitmap->cx);
#endif

         pbmi2Bitmap->cy = (iNumScanLines < uiScanline) ? iNumScanLines : uiScanline;

         rectlPageLocation.yTop    = uiScanline;
         rectlPageLocation.yBottom = uiScanline - pbmi2Bitmap->cy;

#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: pbmi2Bitmap->cy        = %d\n", pbmi2Bitmap->cy);
#endif

         ulBytesRead = cupsRasterReadPixels (pRasterStream,
                                             pbBits,
                                             pageHeaderCurrent.cupsBytesPerLine * pbmi2Bitmap->cy);

#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: bytes to read          = %ld\n", (long)(pageHeaderCurrent.cupsBytesPerLine * pbmi2Bitmap->cy));
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: bytes read             = %ld\n", ulBytesRead);
#endif

         // @TBD fix this!  It should not always say error.
         if (ulBytesRead != (ULONG)(pageHeaderCurrent.cupsBytesPerLine * pbmi2Bitmap->cy))
         {
#ifdef DEBUG
            fprintf (fpCUPSDebug, "CUPSToOmni: Error: reading bitmap data\n");
            fprintf (fpCUPSDebug, "CUPSToOmni: read %ld bytes of raster data\n", (long)ulBytesRead);
            fprintf (fpCUPSDebug, "CUPSToOmni: needed %ld bytes of raster data\n", (long)(pageHeaderCurrent.cupsBytesPerLine * pbmi2Bitmap->cy));
#endif

            cerrReal << "ERROR: CUPSToOmni: Error reading bitmap data" << std::endl;
            cerrReal << "ERROR: CUPSToOmni: read " << ulBytesRead <<  " bytes of raster data" << std::endl;
            cerrReal << "ERROR: CUPSToOmni: needed " << (pageHeaderCurrent.cupsBytesPerLine * pbmi2Bitmap->cy) << " bytes of raster data\n" << std::endl;

            rc = 1;
            break;
         }

         // Test to see if CUPS had anymore data for us even though the page routine is called.
#ifdef DEBUG
         if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: ************** Page Number = %d\n", iPageNumber);
#endif

         if (  (iPageNumber != 1)
            && ulBytesRead
            && fTopOfBand
            )
         {
            fTopOfBand = false;

            pDevice->newFrame ();

#ifdef DEBUG
            if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: New frame.\n");
#endif

            cerrReal << "DEBUG: CUPSToOmni: New frame." << std::endl;
         }

#ifdef DEBUG
         fflush (fpCUPSDebug);
#endif

         pbBitsToSend = (PBYTE)malloc (icbBitmapScanLine * pbmi2Bitmap->cy);

         if (!pbBitsToSend)
         {
#ifdef DEBUG
            fprintf (fpCUPSDebug, "CUPSToOmni: Error: failed to allocate flip bitmap memory!\n");
#endif

            cerrReal << "ERROR: CUPSToOmni: failed to allocate flip bitmap memory!" << std::endl;

            rc = 1;
            goto done;
         }

         if (pageHeaderCurrent.cupsBitsPerPixel != 1)
         {
            // flip and rotate BGR to RGB in the output band
            for (i = 0; i < pbmi2Bitmap->cy; i++)
            {
               for (j = 0; j < (int)pageHeaderCurrent.cupsBytesPerLine - 2; j += 3)
               {
                  int p = j + 2;

                  pbBitsToSend[i*icbBitmapScanLine+j] =    // b --> r
                                  pbBits[pageHeaderCurrent.cupsBytesPerLine * (pbmi2Bitmap->cy-1 -i)+p];
                  pbBitsToSend[i*icbBitmapScanLine+p] =    // r --> b
                                  pbBits[pageHeaderCurrent.cupsBytesPerLine * (pbmi2Bitmap->cy-1 -i)+j];
                  pbBitsToSend[i*icbBitmapScanLine+j+1] =  // g --> g
                                  pbBits[pageHeaderCurrent.cupsBytesPerLine * (pbmi2Bitmap->cy-1 -i)+j+1];
               }
            }

#ifdef DEBUG
            if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: middle cy = %u\n", pbmi2Bitmap->cy);
#endif
         }
         else
         {
            for (i = 0; i < pbmi2Bitmap->cy; i++)
            {
               memcpy ((void *)&pbBitsToSend[i*icbBitmapScanLine],
                       (void *)&pbBits[pageHeaderCurrent.cupsBytesPerLine * (pbmi2Bitmap->cy-1 -i)],
                       pageHeaderCurrent.cupsBytesPerLine);
            }
         }

         uiScanline -= pbmi2Bitmap->cy;  // add in number of lines read

         if (fDoOutput)
         {
            bool fRC;

            fRC = pDevice->rasterize (pbBitsToSend,
                                      pbmi2Bitmap,
                                      &rectlPageLocation,
                                      BITBLT_BITMAP);

             cerrReal << "DEBUG: CUPSToOmni: Rasterize ("
                      << rectlPageLocation.xLeft
                      << ", "
                      << rectlPageLocation.yBottom
                      << ") - ("
                      << rectlPageLocation.xRight
                      << ", "
                      << rectlPageLocation.yTop
                      << ") = "
                      << fRC
                      << std::endl;
         }

         if (pbBitsToSend)
         {
            free (pbBitsToSend);
            pbBitsToSend = 0;
         }

#ifdef DEBUG
         if (fDebugOutput)
         {
            fprintf (fpCUPSDebug, "CUPSToOmni: End pbmi2Bitmap->cy    = %d\n", pbmi2Bitmap->cy);
            fprintf (fpCUPSDebug, "CUPSToOmni: End uiScanline         = %u\n", uiScanline);
         }
#endif

         if (!uiScanline)
            break;

      }  // end while to read raster data

      fTopOfBand = true;

#ifdef DEBUG
      if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: Through first loop\n");
#endif
   }  // end read of ppd file

done:

#ifdef DEBUG
   if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: dropping out. pDevice = %08X, iPageNumber = %d\n", (int)pDevice, iPageNumber);
#endif

   if (  pDevice
      && 1 <= iPageNumber
      )
   {
      pDevice->endJob ();

#ifdef DEBUG
      if (fDebugOutput) fprintf (fpCUPSDebug, "CUPSToOmni: End job.\n");
#endif

      cerrReal << "DEBUG: CUPSToOmni: End job." << std::endl;
   }

   /*
    * Close the raster stream...
    */
   if (pRasterStream)
   {
      cupsRasterClose (pRasterStream);
      pRasterStream = 0;
   }

   if (fdInput != 0)
   {
      close (fdInput);
      fdInput = 0;
   }

   if (ppd)
   {
      ppdClose (ppd);
      ppd = 0;
   }

   if (pbmi2Bitmap)
   {
      free (pbmi2Bitmap);
      pbmi2Bitmap = 0;
   }
   if (pbBitsToSend)
   {
      free (pbBitsToSend);
      pbBitsToSend = 0;
   }
   if (pbBits)
   {
      free (pbBits);
      pbBits = 0;
   }
   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
      pszJobProperties = 0;
   }

   delete pDevice; pDevice = 0;

   if (hmodDevice)
   {
      g_module_close (hmodDevice);
      hmodDevice = 0;
   }

#ifdef DEBUG
   if (fDebugOutput)
   {
      fprintf (fpCUPSDebug, "CUPSToOmni: Closing output stream\n");

      if (fpCUPSDebug)
      {
         fclose (fpCUPSDebug);

         chmod (DEBUG_LOGFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
      }
   }
#endif

   if (0 == iPageNumber)
   {
      cerrReal << "ERROR: CUPSToOmni: No pages printed!" << std::endl;
   }
   else
   {
      cerrReal << "DEBUG: CUPSToOmni: Ready to print." << std::endl;
   }

   Omni::terminate ();

   return rc;
}
