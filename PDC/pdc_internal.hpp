/*
** Copyright (c) 2003 International Business Machines
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
*/
#ifndef _PDC_INTERNAL_H
#define _PDC_INTERNAL_H 1

#include "pdc_method.hpp"

#if !defined (dimof)
#define dimof(a) (sizeof (a)/sizeof (a[0]))
#endif

typedef struct _DeviceHandle {
   int         cbSize;
   char        achSignature[4];
   PDCMethod  *pPDCMethod;
   const void *pvHandle;
} DEVICEDATA, *PDEVICEDATA;

#define DEVICE_SIGNATURE 0x56454450 // 'PDEV'

typedef struct _DriverHandle {
   int         cbSize;
   char        achSignature[4];
   PDCMethod  *pPDCMethod;
} DRIVERDATA, *PDRIVERDATA;

#define DRIVER_SIGNATURE 0x56524450 // 'PDRV'

typedef struct _PDCHandle {
   int         cbSize;
   char        achSignature[4];
   DriversMap *pMapDrivers;
} PDCDATA, *PPDCDATA;

#define PDC_SIGNATURE    0x48434450 // 'PDCH'

char         *trim                               (char         *pszString);
DriversMap   *readDrivers                        ();
void          freeDrivers                        (DriversMap   *pmapDrivers);
PPDCDATA      validatePDCHandle                  (PDCHANDLE     handlePDC,
                                                  bool          fStrict);
bool          freePDCHandle                      (PDCHANDLE     handlePDC);
PDRIVERDATA   validateDriverHandle               (DRIVERHANDLE  handleDriver);
bool          freeDriverHandle                   (DRIVERHANDLE  handleDriver);
PDEVICEDATA   validateDeviceHandle               (DEVICEHANDLE  handleDevice);
bool          freeDeviceHandle                   (DEVICEHANDLE  handleDevice);
PDCMethod    *loadLinkedDriver                   (PPDCDATA      pPDCHandle,
                                                  DriverMap    *pMapDriver);

#endif /* _PDC_INTERNAL_H */
