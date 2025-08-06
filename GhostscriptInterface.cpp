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
#include "GhostscriptInterface.hpp"
#include "Omni.hpp"

#include <cstdio>
#include <sstream>

/*--------------------------------------------------------------------------------*/
/*                          GhostscriptInferfaceInit                              */
/*--------------------------------------------------------------------------------*/
void
GhostscriptInferfaceInit (void *pvOmni)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;

   if (!isOmni (pCoreOmni->cSignature))
      return;

   Omni::initialize ();

   strcpy (pCoreOmni->cVersion, GIVersion);
}

/*--------------------------------------------------------------------------------*/
/*                          GhostscriptInferfaceTerm                              */
/*--------------------------------------------------------------------------------*/
void
GhostscriptInferfaceTerm (void *pvOmni)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;

   if (!isOmni (pCoreOmni->cSignature))
      return;

   if (pCoreOmni->pfpErr)
   {
      fclose (pCoreOmni->pfpErr);

      pCoreOmni->pfpErr = 0;
   }

   Omni::terminate ();
}

/*------------------------------------------------------------------------*/
/*  GetResolutionInfo                                                     */
/*------------------------------------------------------------------------*/
bool
GetResolutionInfo (void *pvOmni, HWRESOLUTION *hwRes)
{
   bool              bRet      = false;
   DeviceResolution *pDevRes   = 0;
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pDevRes = pDevice->getCurrentResolution ();  // Device Resolution
   }
   else
   {
      // Device Resolution
      pDevRes = pCoreOmni->pDevice->getCurrentResolution ();

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pDev = 0x" << std::hex << (int)pvOmni << ", hwRes = 0x" << (int)hwRes << std::dec << std::endl;
#endif
   }

   if (  pCoreOmni
      && pDevRes
      )
   {
      hwRes->xRes = pDevRes->getXRes ();
      hwRes->yRes = pDevRes->getYRes ();
      hwRes->fScanDots = pDevRes->getScanlineMultiple ();
      bRet = true;
   }

   return bRet;
}

/*------------------------------------------------------------------------*/
/*   GetMarginInfo                                                        */
/*------------------------------------------------------------------------*/
bool
GetMarginInfo (void *pvOmni, HWMARGINS *hwMargins)
{
   bool              bRet      = false;
   HardCopyCap      *pHCC      = 0;                         // Hard copy information
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << ", hwMargins = 0x" << (int)hwMargins << std::dec << std::endl;
#endif

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pHCC = pDevice->getCurrentForm ()->getHardCopyCap ();
   }
   else
   {
      pHCC = pCoreOmni->pDevice->getCurrentForm ()->getHardCopyCap ();
   }

   if (pHCC)
   {
      hwMargins->fxWidth     = pHCC->getXPels ();
      hwMargins->fyHeight    = pHCC->getYPels ();

      hwMargins->fLeftClip   = 0; // changed since Ghostscript needs to render full size of
      hwMargins->fTopClip    = 0; // the page printable area.  If we add in the margin, we
      hwMargins->fRightClip  = 0; // actually force GS to remove the margin from the printable
      hwMargins->fBottomClip = 0; // area.

//    hwMargins->fLeftClip   = pHCC->getLeftClip ();
//    hwMargins->fTopClip    = pHCC->getTopClip ();
//    hwMargins->fRightClip  = pHCC->getRightClip ();
//    hwMargins->fBottomClip = pHCC->getBottomClip ();
      bRet = true;
   }

   return bRet;
}

/*--------------------------------------------------------------------------------*/
/*                          GetPrintModeInfo                                      */
/*--------------------------------------------------------------------------------*/
bool
GetPrintModeInfo (void *pvOmni, PRINTMODE *pPrtMode)
{
   bool              bRet        = false;
   DevicePrintMode  *pDevPrtMode = 0;
   core_omni_device *pCoreOmni   = (core_omni_device *)pvOmni;
   Device           *pDevice     = 0;

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pDevPrtMode = pDevice->getCurrentPrintMode ();
   }
   else
   {
      pDevPrtMode = pCoreOmni->pDevice->getCurrentPrintMode ();
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << ", pPrtMode = 0x" << (int)pPrtMode << std::dec << std::endl;
#endif

   if (  pCoreOmni
      && pPrtMode
      && pDevPrtMode
      )
   {
      pPrtMode->iBitCount = pDevPrtMode->getLogicalCount ();
      pPrtMode->iPlanes   = pDevPrtMode->getNumPlanes ();
      bRet = true;
   }

   return bRet;
}

/*--------------------------------------------------------------------------------*/
/*                          SetOutputFunction                                     */
/*--------------------------------------------------------------------------------*/
void
SetOutputFunction (void *pvOmni, PFNOUTPUTFUNCTION pfn, void *pMC)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;

   if (!pCoreOmni->bPDCDevice)
      pCoreOmni->pDevice->setOutputFunction (pfn, pMC);
}

/*--------------------------------------------------------------------------------*/
/*                          BeginJob                                              */
/*--------------------------------------------------------------------------------*/
void *
BeginJob (void *pvOmni, FILE *pfpOutputStream)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << ", pfpOut = 0x" << (int)pCoreOmni->pfpOut << std::dec << std::endl;
#endif

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pDevice->setOutputStream (pfpOutputStream);
      pDevice->beginJob ();
   }
   else
   {
      if (pCoreOmni->bPDCDevice)
      {
#ifdef PDC_INTERFACE_ENABLED
         int fdOut = STDOUT_FILENO;
         int fdErr = STDERR_FILENO;

         // delete the previous device so that we can
         // get the correct file handle to the spawned
         // process - the first device was for queries
         delete pCoreOmni->pDevice;

         if (pCoreOmni->pfpOut)
         {
            fdOut = fileno (pCoreOmni->pfpOut);
         }
         if (pCoreOmni->pfpErr)
         {
            fdErr = fileno (pCoreOmni->pfpErr);
         }

         pCoreOmni->pDevice = new OmniPDCProxy (pCoreOmni->cServer,       // client exe to spawn
                                                pCoreOmni->cDeviceName,   // device name
                                                pCoreOmni->pszJobOptions, // job properties
                                                true,                     // is renderer
                                                fdOut,                    // stdout
                                                fdErr);                   // stderr

         pCoreOmni->pDevice->beginJob ();
#endif
      }
      else
      {
         pCoreOmni->pDevice->setOutputStream (pCoreOmni->pfpOut);
         pCoreOmni->pDevice->beginJob ();
      }
   }

   return 0;
}

/*--------------------------------------------------------------------------------*/
/*                          BeginJob                                              */
/*--------------------------------------------------------------------------------*/
void
BeginJob (void *pvOmni)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << std::dec << std::endl;
#endif

   pCoreOmni->pDevice->beginJob ();
}

/*--------------------------------------------------------------------------------*/
/*                          Rasterize                                             */
/*--------------------------------------------------------------------------------*/
void
Rasterize (void             *pvOmni,
           PBYTE             pbBits,
           PBITMAPINFO2      pbmi,
           PSIZEL            psizelPage,
           PRECTL            prectlPageLocation,
           BITBLT_TYPE       eType)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

   if (!isOmni(pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pDevice->rasterize (pbBits,
                          pbmi,
                          prectlPageLocation,
                          eType);
   }
   else
   {
      pCoreOmni->pDevice->rasterize (pbBits,
                                     pbmi,
                                     prectlPageLocation,
                                     eType);
   }
}

/*--------------------------------------------------------------------------------*/
/*                          NewPage                                               */
/*--------------------------------------------------------------------------------*/
void
NewFrame (void *pvOmni)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << std::dec << std::endl;
#endif

   if (!isOmni(pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
      pDevice = (Device *)pvOmni;
      pDevice->newFrame ();
   }
   else
   {
      pCoreOmni->pDevice->newFrame ();
   }
}

/*--------------------------------------------------------------------------------*/
/*                          EndJob                                                */
/*--------------------------------------------------------------------------------*/
void
EndJob (void *pvOmni)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;
   Device           *pDevice   = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvOmni = 0x" << std::hex << (int)pvOmni << std::dec << std::endl;
#endif

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure just a pDevice
   {
       pDevice = (Device *)pvOmni;
       pDevice->endJob ();
       delete pDevice;
   }
   else
   {
      if (pCoreOmni->pDevice)
      {
         pCoreOmni->pDevice->endJob ();

         delete pCoreOmni->pDevice;

         pCoreOmni->pDevice = 0;
      }
   }
}

/*--------------------------------------------------------------------------------*/
/*                            createDevice                                        */
/* NOTE: Deprecated!                                                              */
/*--------------------------------------------------------------------------------*/
void *
createDevice (char  *pszDeviceName,
              void  *pOutputObject,
              void **pvhDevice,
              char  *pszCerr,
              char  *pszJobProperties,
              int    iUseClient,
              FILE  *pFileIn)
{
#ifdef PDC_INTERFACE_ENABLED
   OmniPDCProxy *pPDCDevice = 0;
#endif
   Device       *pDevice    = 0;

   if (iUseClient)
   {
#ifdef PDC_INTERFACE_ENABLED
      // initial device is created for setting up caller with correct option and printable area
      pPDCDevice = (OmniPDCProxy *)new OmniPDCProxy (0,
                                                     pszDeviceName,
                                                     pszJobProperties,
                                                     true,
                                                     /*fileno (pFileIn)*/0,
                                                     0);

      return (void *)pPDCDevice;
#endif
   }
   else
   {
      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "This program needs glib's module routines!" << std::endl;
         return 0;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << std::endl;
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszDeviceName    = " << pszDeviceName << std::endl;
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pOutputObject    = 0x" << std::hex << (int)pOutputObject << std::endl;
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvhDevice        = 0x" << (int)pvhDevice << std::endl;
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszCerr          = " << pszCerr << std::endl;
      if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszJobProperties = " << pszJobProperties << std::dec << std::endl;
#endif

#define LIBDEVICENAME3(s) STRINGIZE(s)
#define LIBDEVICENAME2(x) LIBDEVICENAME3(lib##x##.so)
#define LIBDEVICENAME1(x) LIBDEVICENAME2(x)
#define LIBDEVICENAME     LIBDEVICENAME1(DEVICENAME)

typedef Device * (*PFNNEWDEVICEWARGS) (char *pszJobProperties, bool fAdvanced);
typedef Device * (*PFNNEWDEVICE)      (bool fAdvanced);

      char               cDeviceName[64];
      PFNNEWDEVICE       pfnNewDevice      = 0;
      PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

      pDevice  = 0;

#if 0 // @GCC
      // Added for debugging convienence
      if (  pszCerr
         && pszCerr[0]
         )
      {
         ofstream *fcerr = new ofstream (pszCerr);
         cerr = *fcerr;
         pOutputObject = (void *)fcerr;
      }
#endif

      if (0 != strncmp (pszDeviceName, "lib", 3))
         sprintf (cDeviceName, "lib%s.so", pszDeviceName);
      else
         strcpy (cDeviceName, pszDeviceName);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": Trying to load " << cDeviceName << std::endl;
#endif

      char *pszDeviceLib = 0;

      *pvhDevice = 0;

      for (int i = 0; vapszLibraryPaths[i] && !*pvhDevice; i++)
      {
         pszDeviceLib = (char *)malloc ( strlen (cDeviceName)
                                       + strlen (vapszLibraryPaths[i])
                                       + 1);
         if (pszDeviceLib)
         {
            sprintf (pszDeviceLib, "%s%s", vapszLibraryPaths[i], cDeviceName);

            *pvhDevice = g_module_open (pszDeviceLib, (GModuleFlags)0);
         }
         else
         {
            return 0;
         }

         free (pszDeviceLib);
      }

      if (!*pvhDevice)
      {
         DebugOutput::getErrorStream () << std::endl << "<<<<<<<<<<<<<<<<<<<<<< ERROR >>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
         DebugOutput::getErrorStream () << std::endl << std::endl;
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": cDeviceName = " << cDeviceName << std::endl;
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszDeviceName = " << pszDeviceName << std::endl;
         DebugOutput::getErrorStream () << std::endl;
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": LD_LIBRARY_PATH =  " << getenv ("LD_LIBRARY_PATH") << std::endl;
         DebugOutput::getErrorStream () << std::endl;
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": Omni device library not found in the following paths:" << std::endl;
         for (int i = 0; vapszLibraryPaths[i] && !*pvhDevice; i++)
         {
            DebugOutput::getErrorStream () << "\t" << vapszLibraryPaths[i] << "." << std::endl;
         }
         DebugOutput::getErrorStream () << "\t$LD_LIBRARY_PATH" << std::endl;

         return 0;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": *pvhDevice = " << std::hex << *pvhDevice << std::dec << std::endl;
#endif

      // nm libHP_Deskjet_1120Cxi.so

      // returns 00011e20 T newDeviceW_Advanced for
      // extern Device *newDevice (bool fAdvanced);
      g_module_symbol ((GModule *)*pvhDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pfnNewDevice = 0x" << std::hex << (int)pfnNewDevice << std::endl;
#endif

      if (!pfnNewDevice)
      {
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": g_module_error returns " << std::dec << g_module_error () << std::endl;

         return 0;
      }

      // returns 00011d30 T newDeviceW_JobProp_Advanced for
      // extern Device *newDevice (char *pszJobProperties, bool fAdvanced);
      g_module_symbol ((GModule *)*pvhDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pfnNewDeviceWArgs = 0x" << std::hex << (int)pfnNewDeviceWArgs << std::endl;
#endif

      if (!pfnNewDeviceWArgs)
      {
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": g_module_error returns " << std::dec << g_module_error () << std::endl;

         return 0;
      }

      if (pszJobProperties && pszJobProperties[0])
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);
      else
         pDevice = pfnNewDevice (true);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pDevice = " << *pDevice << std::endl;
#endif

      return (void *)pDevice;
   }

   return 0;
}

/*--------------------------------------------------------------------------------*/
/*                            CreateDevice                                        */
/*--------------------------------------------------------------------------------*/
void *
CreateDevice (void  *pvOmni,
              void **pvhDevice,
              int    iUsePDC)
{
   core_omni_device *pCoreOmni = (core_omni_device *)pvOmni;

   if (!isOmni (pCoreOmni->cSignature))  // not an OMNI structure
      return 0;

   if (!pvhDevice)
   {
      return 0;
   }

   *pvhDevice = 0;

   char    *pszDeviceName    = pCoreOmni->cDeviceName;
   char    *pszJobProperties = pCoreOmni->pszJobOptions;
   char    *pszCerr          = pCoreOmni->cDebugFile;
   char    *pszPDCServerName = pCoreOmni->cServer;
   Device  *pDevice          = 0;
   FILE    *pfpErr           = stderr;

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << std::endl;
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszDeviceName    = " << pszDeviceName << std::endl;
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pvhDevice        = 0x" << std::hex << (int)pvhDevice << std::dec << std::endl;
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszCerr          = " << pszCerr << std::endl;
   if (DebugOutput::shouldOutputOmniInterface ()) DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pszJobProperties = " << pszJobProperties << std::dec << std::endl;
#endif

   // Added for debugging convienence
   if (  pszCerr
      && pszCerr[0]
      )
   {
      pCoreOmni->pfpErr = fopen (pszCerr, "w");

      pfpErr = pCoreOmni->pfpErr;
   }

   // NOTE: Yes.  pDevice == 0 at this point
   Omni::setErrorStream (pDevice, pfpErr);
   DebugOutput::applyAllDebugOutput (pszJobProperties);

   if (iUsePDC)
   {
#ifdef PDC_INTERFACE_ENABLED
      // initial device is created for setting up caller with correct option and printable area
      pDevice = new OmniPDCProxy (pszPDCServerName,      // client exe to spawn
                                  pszDeviceName,         // device name
                                  pszJobProperties,      // job properties
                                  true,                  // is renderer
                                  0,                     // stdout
                                  fileno (pfpErr));      // stderr
#endif
   }
   else
   {
      if (!g_module_supported ())
      {
         DebugOutput::getErrorStream () << "This program needs glib's module routines!" << std::endl;

         return 0;
      }

#define LIBDEVICENAME3(s) STRINGIZE(s)
#define LIBDEVICENAME2(x) LIBDEVICENAME3(lib##x##.so)
#define LIBDEVICENAME1(x) LIBDEVICENAME2(x)
#define LIBDEVICENAME     LIBDEVICENAME1(DEVICENAME)

typedef Device * (*PFNNEWDEVICEWARGS) (char *pszJobProperties, bool fAdvanced);
typedef Device * (*PFNNEWDEVICE)      (bool fAdvanced);

      char               cDeviceName[64];
      PFNNEWDEVICE       pfnNewDevice      = 0;
      PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

      if (0 != strncmp (pszDeviceName, "lib", 3))
         sprintf (cDeviceName, "lib%s.so", pszDeviceName);
      else
         strcpy (cDeviceName, pszDeviceName);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": Trying to load " << cDeviceName << std::endl;
#endif

      Omni::openAndTestDeviceLibrary (cDeviceName, (GModule **)pvhDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": *pvhDevice = " << std::hex << *pvhDevice << std::dec << std::endl;
#endif

      if (!*pvhDevice)
      {
         return 0;
      }

      // nm libHP_Deskjet_1120Cxi.so

      // returns 00011e20 T newDeviceW_Advanced for
      // extern Device *newDevice (bool fAdvanced);
      g_module_symbol ((GModule *)*pvhDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pfnNewDevice = 0x" << std::hex << (int)pfnNewDevice << std::endl;
#endif

      if (!pfnNewDevice)
      {
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": g_module_error returns " << std::dec << g_module_error () << std::endl;

         g_module_close ((GModule *)*pvhDevice);
         *pvhDevice = 0;

         return 0;
      }

      // returns 00011d30 T newDeviceW_JobProp_Advanced for
      // extern Device *newDevice (char *pszJobProperties, bool fAdvanced);
      g_module_symbol ((GModule *)*pvhDevice, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniInterface ())
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pfnNewDeviceWArgs = 0x" << std::hex << (int)pfnNewDeviceWArgs << std::endl;
#endif

      if (!pfnNewDeviceWArgs)
      {
         DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": g_module_error returns " << std::dec << g_module_error () << std::endl;

         g_module_close ((GModule *)*pvhDevice);
         *pvhDevice = 0;

         return 0;
      }

      if (  pszJobProperties
         && pszJobProperties[0]
         )
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);
      else
         pDevice = pfnNewDevice (true);
   }

#ifndef RETAIL
   if (pfpErr)
   {
      Omni::setErrorStream (pDevice, pfpErr);
   }

   if (DebugOutput::shouldOutputOmniInterface ())
      DebugOutput::getErrorStream () << "GhostscriptInterface::" << __FUNCTION__ << ": pDevice = " << *pDevice << std::endl;
#endif

   if (pDevice->hasError ())
   {
      delete pDevice;

      return 0;
   }

   strcpy (pCoreOmni->cOmniVersion, pDevice->getVersion ());

   return (void *)pDevice;
}

/*--------------------------------------------------------------------------------*/
/*                            DeleteDevice                                        */
/*--------------------------------------------------------------------------------*/
void
DeleteDevice (void  *pvOmni)
{
   Device *pDevice = (Device *)pvOmni;

   delete pDevice;
}

/*--------------------------------------------------------------------------------*/
/*                          isOmni                                                */
/*--------------------------------------------------------------------------------*/
bool
isOmni (char *pszSignature)
{
   bool fRet = false;

   if (!memcmp (pszSignature, Signature, 4))  // check for signature
      fRet = true;

   return fRet;
}
