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
#include "pdc.h"
#include "pdc_internal.hpp"
#include "pdc_method_link.hpp"

#include <iostream>
#include <fstream>

#include <glib.h>
#include <gmodule.h>

// (export LD_LIBRARY_PATH=`pwd`/.libs; valgrind -v --leak-check=yes --show-reachable=yes .libs/tester)

const bool vfDebug = true;

#define SAFE_PRINT_PSZ(pszString) (pszString ? pszString : "(null)")

char *
trim (char *pszString)
{
   while (  ' '  == *pszString
         || '\t' == *pszString
         || '\r' == *pszString
         || '\n' == *pszString
         )
   {
      pszString++;
   }

   if (*pszString)
   {
      char *pszEnd = pszString + strlen (pszString) - 1;

      while (  (  ' '  == *pszEnd
               || '\t' == *pszEnd
               || '\r' == *pszEnd
               || '\n' == *pszEnd
               )
            && pszEnd > pszString
            )
      {
         pszEnd--;
      }

      pszEnd++;
      *pszEnd = '\0';
   }

   return pszString;
}

DriversMap *
readDrivers ()
{
   char           achLine[512];           // @TBD
   std::ifstream  ifIn ("/etc/pdc");
   DriversMap    *pMapDrivers        = 0;
   DriverMap     *pMapDriver         = 0;

///while (std::getline (ifIn, stringLine))
   while (0 < ifIn.getline (achLine, sizeof (achLine)))
   {
      if (!pMapDrivers)
      {
         pMapDrivers = new DriversMap ();

#ifndef RETAIL
         if (vfDebug) std::cerr << __FUNCTION__ << ": New DriversMap @ " << std::hex << (int)pMapDrivers << std::dec << std::endl;
#endif
      }

      char *pszLine   = 0;
      char *pszEquals = 0;

      pszLine = trim (achLine);

#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": Read: \"" << pszLine << "\"" << std::endl;
#endif

      pszEquals = strchr (pszLine, '=');

      if (!pszEquals)
      {
         continue;
      }

      *pszEquals = '\0';

      std::string stringKey   = pszLine;
      std::string stringValue = pszEquals + 1;

      if (0 == strcasecmp (pszLine, "driver"))
      {
         pMapDriver = (*pMapDrivers)[stringValue];

#ifndef RETAIL
         if (vfDebug) std::cerr << __FUNCTION__ << ": pMapDrivers[" << stringValue << "] = " << std::hex << pMapDriver << std::dec << std::endl;
#endif

         if (!pMapDriver)
         {
            pMapDriver = new DriverMap ();

#ifndef RETAIL
            if (vfDebug) std::cerr << __FUNCTION__ << ": New MapDriver = " << std::hex << (int)pMapDriver << std::dec << std::endl;
#endif

            (*pMapDrivers)[stringValue] = pMapDriver;
         }
      }

      if (pMapDriver)
      {
#ifndef RETAIL
         if (vfDebug) std::cerr << __FUNCTION__ << ": Adding: " << stringKey << "=" << stringValue << std::endl;
#endif

         (*pMapDriver)[stringKey] = stringValue;
      }
      else
      {
#ifndef RETAIL
         if (vfDebug) std::cerr << __FUNCTION__ << ": Error: " << stringKey << "=" << stringValue << " is not associated witha driver." << std::endl;
#endif
      }
   }

   return pMapDrivers;
}

void
freeDrivers (DriversMap *pMapDrivers)
{
   if (pMapDrivers)
   {
      for ( DriversMap::iterator nextDrivers = pMapDrivers->begin ();
            nextDrivers != pMapDrivers->end ();
            nextDrivers++ )
      {
         DriverMap *pMapDriver = nextDrivers->second;

#ifndef RETAIL
         if (vfDebug) std::cerr << __FUNCTION__ << ": Freeing Driver Map: " << nextDrivers->first << " = " << std::hex << pMapDriver << std::dec << std::endl;
#endif

         delete pMapDriver;
      }

#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": Freeing Drivers Map: " << std::hex << (int)pMapDrivers << std::dec << std::endl;
#endif

      delete pMapDrivers;
   }
}

PPDCDATA
validatePDCHandle (PDCHANDLE handlePDC,
                   bool      fStrict)
{
   PPDCDATA pPDCData = (PPDCDATA)handlePDC;

   if (!pPDCData)
   {
      return 0;
   }
   if (sizeof (PDCDATA) != pPDCData->cbSize)
   {
      return 0;
   }
   if (PDC_SIGNATURE != *(int *)pPDCData->achSignature)
   {
      return 0;
   }
   if (  fStrict
      && !pPDCData->pMapDrivers
      )
   {
      return 0;
   }

   return pPDCData;
}

bool
freePDCHandle (PDCHANDLE handlePDC)
{
   PPDCDATA pPDCData = validatePDCHandle (handlePDC, false);

   if (pPDCData)
   {
      freeDrivers (pPDCData->pMapDrivers);

      memset (pPDCData, 0, sizeof (PDCHANDLE));

      free ((void *)pPDCData);

      return true;
   }

   return false;
}

PDRIVERDATA
validateDriverHandle (DRIVERHANDLE handleDriver)
{
   PDRIVERDATA pDriverData = (PDRIVERDATA)handleDriver;

   if (!pDriverData)
   {
      return 0;
   }
   if (sizeof (DRIVERDATA) != pDriverData->cbSize)
   {
      return 0;
   }
   if (DRIVER_SIGNATURE != *(int *)pDriverData->achSignature)
   {
      return 0;
   }
   if (!dynamic_cast <PDCMethod *>(pDriverData->pPDCMethod))
   {
      return 0;
   }

   return pDriverData;
}

bool
freeDriverHandle (DRIVERHANDLE handleDriver)
{
   PDRIVERDATA pDriverData = validateDriverHandle (handleDriver);

   if (pDriverData)
   {
      if (pDriverData->pPDCMethod)
      {
         delete pDriverData->pPDCMethod;
      }

      memset (pDriverData, 0, sizeof (DRIVERHANDLE));

      free ((void *)pDriverData);

      return true;
   }

   return false;
}

PDEVICEDATA
validateDeviceHandle (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = (PDEVICEDATA)handleDevice;

   if (!pDeviceData)
   {
      return 0;
   }
   if (sizeof (DEVICEDATA) != pDeviceData->cbSize)
   {
      return 0;
   }
   if (DEVICE_SIGNATURE != *(int *)pDeviceData->achSignature)
   {
      return 0;
   }
   if (!dynamic_cast <PDCMethod *>(pDeviceData->pPDCMethod))
   {
      return 0;
   }
   if (!pDeviceData->pvHandle)
   {
      return 0;
   }

   return pDeviceData;
}

bool
freeDeviceHandle (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = validateDeviceHandle (handleDevice);

   if (pDeviceData)
   {
      memset (pDeviceData, 0, sizeof (DEVICEHANDLE));

      free ((void *)pDeviceData);

      return true;
   }

   return false;
}

PDCMethod *
loadLinkedDriver (PPDCDATA    pPDCData,
                  DriverMap  *pMapDriver)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << pPDCData << ", " << pMapDriver << std::dec << ")" << std::endl;
#endif

   return new PDCMethodLink (pMapDriver);
}

/* 8<---8<---8<---8<---8<---8<---8<---8<---8<---8<---8<---8<---8<---8<---8<--- */

PDCHANDLE
pdcOpenLibrary ()
{
   PPDCDATA pPDCData = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " ()" << std::endl;
#endif

   pPDCData = (PPDCDATA)calloc (1, sizeof (PDCDATA));

   if (pPDCData)
   {
      pPDCData->cbSize = sizeof (PDCDATA);
      *(int *)pPDCData->achSignature = PDC_SIGNATURE;
      pPDCData->pMapDrivers = readDrivers ();
   }

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << ": returning " << std::hex << pPDCData << std::dec << std::endl;
#endif

   return (PDCHANDLE)pPDCData;
}

int
pdcCloseLibrary (PDCHANDLE handlePDC)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handlePDC << std::dec << ")" << std::endl;
#endif

   if (freePDCHandle (handlePDC))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

STRINGARRAY
pdcEnumerateDrivers (PDCHANDLE handlePDC)
{
   PPDCDATA    pPDCData    = 0;
   DriversMap *pMapDrivers = 0;
   int         cbAlloc     = 0;
   char       *pszAlloc    = 0;
   const char *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handlePDC << std::dec << ")" << std::endl;
#endif

   pPDCData = validatePDCHandle (handlePDC, true);

   if (!pPDCData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validatePDCHandle fails!" << std::endl;
#endif

      return 0;
   }

   pMapDrivers = pPDCData->pMapDrivers;

   for ( DriversMap::iterator next = pMapDrivers->begin ();
         next != pMapDrivers->end ();
         next++ )
   {
      if (!next->second)
      {
         continue;
      }

      std::string stringValue;

      stringValue = (*next->second)[std::string ("driver")];

#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": stringValue = " << stringValue << std::endl;
#endif

      cbAlloc += stringValue.length () + 1;
   }

   if (cbAlloc)
   {
      cbAlloc++;  // Null terminated
   }

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << ": cbAlloc = " << cbAlloc << std::endl;
#endif

   if (cbAlloc)
   {
      pszAlloc = (char *)calloc (1, cbAlloc);

      if (pszAlloc)
      {
         pszReturn = pszAlloc;

         for ( DriversMap::iterator next = pMapDrivers->begin ();
               next != pMapDrivers->end ();
               next++ )
         {
            if (!next->second)
            {
               continue;
            }

            std::string stringValue;

            stringValue = (*next->second)[std::string ("driver")];

            if (stringValue.length ())
            {
               strcpy (pszAlloc, stringValue.c_str ());

               pszAlloc += strlen (stringValue.c_str ()) + 1;
            }
         }

         *pszAlloc++ = '\0';
      }
   }

   return (STRINGARRAY)pszReturn;
}

DRIVERHANDLE
pdcOpenDriver (PDCHANDLE   handlePDC,
               const char *pszDriver)
{
   PPDCDATA      pPDCData     = 0;
   DriverMap    *pMapDriver   = 0;
   PDCMethod    *pPDCMethod   = 0;
   DRIVERHANDLE  handleDriver = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handlePDC << std::dec << ", \"" << SAFE_PRINT_PSZ (pszDriver) << "\")" << std::endl;
#endif

   pPDCData = validatePDCHandle (handlePDC, true);

   if (!pPDCData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validatePDCHandle fails!" << std::endl;
#endif

      return 0;
   }

   pMapDriver = (*pPDCData->pMapDrivers)[std::string (pszDriver)];

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << ": pMapDriver = " << std::hex << pMapDriver << std::dec << std::endl;
#endif

   if (!pMapDriver)
   {
      return handleDriver;
   }

   std::string stringMethodKey ("method");
   std::string stringMethodValue;

   stringMethodValue = (*pMapDriver)[stringMethodKey];

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << ": stringMethodValue = " << stringMethodValue << std::endl;
#endif

   if (!stringMethodValue.size ())
   {
      return handleDriver;
   }

   if (stringMethodValue == "link")
   {
      pPDCMethod = loadLinkedDriver (pPDCData, pMapDriver);
   }

   if (pPDCMethod)
   {
      PDRIVERDATA pDriverData = 0;

      pDriverData = (PDRIVERDATA)calloc (1, sizeof (DRIVERDATA));

      if (pDriverData)
      {
         pDriverData->cbSize = sizeof (DRIVERDATA);
         *(int *)pDriverData->achSignature = DRIVER_SIGNATURE;
         pDriverData->pPDCMethod = pPDCMethod;

         handleDriver = (DRIVERHANDLE)pDriverData;
      }
   }

   return handleDriver;
}

int
pdcCloseDriver (DRIVERHANDLE handleDriver)
{
   PDRIVERDATA pDriverData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handleDriver << std::dec << ")" << std::endl;
#endif

   pDriverData = validateDriverHandle (handleDriver);

   if (!pDriverData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDriverHandle fails!" << std::endl;
#endif

      return iRC;
   }

   freeDriverHandle (handleDriver);

   iRC = 1;

   return iRC;
}

STRINGPAIRARRAY
pdcEnumerateDevices (DRIVERHANDLE handleDriver)
{
   PDRIVERDATA     pDriverData = 0;
   STRINGPAIRARRAY aszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handleDriver << std::dec << ")" << std::endl;
#endif

   pDriverData = validateDriverHandle (handleDriver);

   if (!pDriverData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDriverHandle fails!" << std::endl;
#endif

      return aszReturn;
   }

   aszReturn = pDriverData->pPDCMethod->pdcEnumerateDevices ();

   return aszReturn;
}

const char *
pdcGetDeviceHandle (DRIVERHANDLE  handleDriver,
                    const char   *pszDisplayHandle)
{
   PDRIVERDATA  pDriverData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handleDriver << std::dec << ", \"" << SAFE_PRINT_PSZ (pszDisplayHandle) << "\")" << std::endl;
#endif

   pDriverData = validateDriverHandle (handleDriver);

   if (!pDriverData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDriverHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDriverData->pPDCMethod;

   pszReturn = pPDCMethod->pdcGetDeviceHandle (pszDisplayHandle);

   return pszReturn;
}

DEVICEHANDLE
pdcOpenDevice (DRIVERHANDLE  handleDriver,
               const char   *pszDeviceHandle)
{
   PDRIVERDATA  pDriverData  = 0;
   DEVICEHANDLE handleReturn = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handleDriver << std::dec << ", \"" << SAFE_PRINT_PSZ (pszDeviceHandle) << "\")" << std::endl;
#endif

   pDriverData = validateDriverHandle (handleDriver);

   if (!pDriverData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDriverHandle fails!" << std::endl;
#endif

      return handleReturn;
   }

   PDCMethod  *pPDCMethod = pDriverData->pPDCMethod;
   const void *pvHandle = 0;

   pvHandle = pPDCMethod->pdcOpenDevice (pszDeviceHandle);

   if (pvHandle)
   {
      PDEVICEDATA pDeviceData = 0;

      pDeviceData = (PDEVICEDATA)calloc (1, sizeof (DEVICEDATA));

      if (pDeviceData)
      {
         pDeviceData->cbSize = sizeof (DEVICEDATA);
         *(int *)pDeviceData->achSignature = DEVICE_SIGNATURE;
         pDeviceData->pPDCMethod = pDriverData->pPDCMethod;
         pDeviceData->pvHandle   = pvHandle;

         handleReturn = (DEVICEHANDLE)pDeviceData;
      }
   }
   else
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": pPDCMethod->pdcOpenDevice fails!" << std::endl;
#endif
   }

   return handleReturn;
}

int
pdcCloseDevice (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << handleDevice << std::dec << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcCloseDevice (pDeviceData->pvHandle);

   freeDeviceHandle (handleDevice);

   return iRC;
}

int
pdcSetTranslatableLanguage (DEVICEHANDLE  handleDevice,
                            const char   *pszLanguage)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << std::dec << ", \"" << SAFE_PRINT_PSZ (pszLanguage) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcSetTranslatableLanguage (pDeviceData->pvHandle,
                                                 pszLanguage);

   return iRC;
}

const char *
pdcGetTranslatableLanguage (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcGetTranslatableLanguage (pDeviceData->pvHandle);

   return pszReturn;
}

STRINGARRAY
pdcQueryTranslatableLanguages (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   STRINGARRAY aszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return aszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   aszReturn = pPDCMethod->pdcQueryTranslatableLanguages (pDeviceData->pvHandle);

   return aszReturn;
}

const char *
pdcGetPDLInformation (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcGetPDLInformation (pDeviceData->pvHandle);

   return pszReturn;
}

const char *
pdcQueryCurrentJobProperties (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcQueryCurrentJobProperties (pDeviceData->pvHandle);

   return pszReturn;
}

int
pdcSetJobProperties (DEVICEHANDLE handleDevice,
                     const char  *pszJobProperties)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcSetJobProperties (pDeviceData->pvHandle,
                                          pszJobProperties);

   return iRC;
}

const char *
pdcListJobPropertyKeys (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcListJobPropertyKeys (pDeviceData->pvHandle);

   return pszReturn;
}

const char *
pdcListJobPropertyKeyValues (DEVICEHANDLE handleDevice,
                             const char  *pszKey)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcListJobPropertyKeyValues (pDeviceData->pvHandle,
                                                        pszKey);

   return pszReturn;
}

const char *
pdcGetJobProperty (DEVICEHANDLE handleDevice,
                   const char  *pszKey)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcGetJobProperty (pDeviceData->pvHandle,
                                              pszKey);

   return pszReturn;
}

const char *
pdcGetJobPropertyType (DEVICEHANDLE handleDevice,
                       const char  *pszKey)
{
   PDEVICEDATA  pDeviceData = 0;
   const char  *pszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return pszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   pszReturn = pPDCMethod->pdcGetJobPropertyType (pDeviceData->pvHandle,
                                                  pszKey);

   return pszReturn;
}

STRINGARRAY
pdcTranslateJobProperty (DEVICEHANDLE  handleDevice,
                         const char   *pszKey,
                         const char   *pszValue)
{
   PDEVICEDATA  pDeviceData = 0;
   STRINGARRAY  aszReturn   = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszKey) << ", \"" << SAFE_PRINT_PSZ (pszValue) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return aszReturn;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   aszReturn = pPDCMethod->pdcTranslateJobProperty (pDeviceData->pvHandle,
                                                    pszKey,
                                                    pszValue);

   return aszReturn;
}

int
pdcBeginJob (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcBeginJob (pDeviceData->pvHandle);

   return iRC;
}

int
pdcStartPage (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcStartPage (pDeviceData->pvHandle);

   return iRC;
}

int
pdcStartPageWithProperties (DEVICEHANDLE handleDevice,
                            const char  *pszJobProperties)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcStartPageWithProperties (pDeviceData->pvHandle,
                                                 pszJobProperties);

   return iRC;
}

int
pdcEndPage (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcEndPage (pDeviceData->pvHandle);

   return iRC;
}

int
pdcEndJob (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcEndJob (pDeviceData->pvHandle);

   return iRC;
}

int
pdcAbortPage (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcAbortPage (pDeviceData->pvHandle);

   return iRC;
}

int
pdcAbortJob (DEVICEHANDLE handleDevice)
{
   PDEVICEDATA pDeviceData = 0;
   int         iRC         = 0;

#ifndef RETAIL
   if (vfDebug) std::cerr << __FUNCTION__ << " (" << std::hex << (int)handleDevice << ")" << std::endl;
#endif

   pDeviceData = validateDeviceHandle (handleDevice);

   if (!pDeviceData)
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << __FUNCTION__ << ": validateDeviceHandle fails!" << std::endl;
#endif

      return iRC;
   }

   PDCMethod *pPDCMethod = pDeviceData->pPDCMethod;

   iRC = pPDCMethod->pdcAbortJob (pDeviceData->pvHandle);

   return iRC;
}

void
pdcFree (void *pvData)
{
   if (pvData)
   {
      free (pvData);
   }
}
