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
#ifndef _OmniInterface
#define _OmniInterface

#include "OmniPDCProxy.hpp"
#include "defines.hpp"
#include "Device.hpp"

#include <cstdio>

/****************************************************************************/
/* WARNING: The following is duplicated in Ghostscript's gomni.c            */
/****************************************************************************/
#define Signature  "OMNI"
#define MAX_LENGTH 65
#define GIVersion  "0.1.0"

typedef struct omni_dev_struct {
    char          cSignature[4];              // sig = "OMNI"
    int           cbSize;                     // length of the structure
    char          cVersion[10];               // GS interface version
    char          cOmniVersion[10];           // Omni driver version
    bool          bPDCDevice;                 // Is a PDC Device?
    Device       *pDevice;                    // Device handle
    FILE         *pfpOut;                     // output file handle
    FILE         *pfpErr;                     // error file handle
    char         *pszJobOptions;              // Job Options
    char          cDebugFile[MAX_LENGTH];     // debug file name
    char          cDeviceName[MAX_LENGTH];    // device name
    char          cServer[64];                // PDC client exe name
    void         *pvReserved;                 // reserved
} core_omni, core_omni_device;

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

/****************************************************************************/
/* END WARNING                                                              */
/****************************************************************************/

extern "C" {

void          GhostscriptInferfaceInit (void         *pOmni);
void          GhostscriptInferfaceTerm (void         *pOmni);
bool          GetResolutionInfo        (void         *pDev,
                                        HWRESOLUTION *hwRes);
bool          GetMarginInfo            (void         *pDev,
                                        HWMARGINS    *hwMargins);
bool          GetPrintModeInfo         (void         *pDev,
                                        PRINTMODE    *pPrtMode);
void         *BeginJob                 (void         *pDev,
                                        FILE         *pfpOut);
void          NewFrame                 (void         *pDev);
void          EndJob                   (void         *pDev);
void          Rasterize                (void         *pDevice,
                                        PBYTE         pbBits,
                                        PBITMAPINFO2  pbmi,
                                        PSIZEL        psizelPage,
                                        PRECTL        prectlPageLocation,
                                        BITBLT_TYPE   eType);
void         *CreateDevice             (void         *pOmni,
                                        void        **vhDevice,
                                        int           iUsePDC);
void         *createDevice             (char         *pszDeviceName,
                                        void         *pOutputObject,
                                        void        **pvhDevice,
                                        char         *pszCerr,
                                        char         *pszJobProperties,
                                        int           iUseClient,
                                        FILE         *pFileIn);
void          DeleteDevice             (void         *pOmni);
bool          isOmni                   (char         *pszSignature);

}

#endif
