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

#include <iostream>

#define SAFE_PRINT_PSZ(pszString) (pszString ? pszString : "(null)")

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << std::endl
             << "\t--driver\t\tThe driver name"
             << std::endl
             << "\t--device\t\tThe device name"
             << std::endl
             << "\t--help\t\tPrints this out"
             << std::endl;
}

void
handleDeviceJobProperties (DEVICEHANDLE handleDevice)
{
   STRINGARRAY  aszKeys    = 0;
   const char  *pszKey     = 0;
   STRINGARRAY  aszValues  = 0;
   const char  *pszValue   = 0;
   const char  *pszDefault = 0;
   STRINGARRAY  aszXLates  = 0;
   const char  *pszXLate   = 0;
   bool         fFirst     = false;
   int          iRC        = 0;

   pszKey = pdcQueryCurrentJobProperties (handleDevice);

   if (pszKey)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcQueryCurrentJobProperties" << std::endl;

   std::cout << "The current job properties are: \"" << SAFE_PRINT_PSZ (pszKey) << "\"" << std::endl;

   if (pszKey)
   {
      iRC = pdcSetJobProperties (handleDevice, pszKey);

      if (iRC)
      {
         std::cout << "PASS: ";
      }
      else
      {
         std::cout << "FAIL: ";
      }
      std::cout << "pdcSetJobProperties" << std::endl;

      pdcFree ((void *)pszKey);
      pszKey = 0;
   }

   aszKeys = pdcListJobPropertyKeys (handleDevice);
   pszKey  = aszKeys;

   if (aszKeys)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcListJobPropertyKeys" << std::endl;

   while (  pszKey
         && *pszKey
         )
   {
      aszValues  = pdcListJobPropertyKeyValues (handleDevice, pszKey);
      pszValue   = aszValues;
      fFirst     = true;
      pszDefault = pdcGetJobProperty (handleDevice, pszKey);

      if (!pszDefault)
      {
         std::cout << "FAIL: pdcGetJobProperty" << std::endl;
      }

      while (  pszValue
            && *pszValue
            )
      {
         aszXLates = pdcTranslateJobProperty (handleDevice,
                                              pszKey,
                                              pszValue);
         pszXLate  = aszXLates;

         if (fFirst)
         {
            std::cout << SAFE_PRINT_PSZ (pszKey)
                      << " ["
                      << SAFE_PRINT_PSZ (pszXLate)
                      << "]"
                      << std::endl;

            fFirst = false;
         }

         if (!aszXLates)
         {
            std::cout << "FAIL: pdcTranslateJobProperty" << std::endl;
         }
         else
         {
            pszXLate += strlen (pszXLate) + 1;
         }

         std::cout << "\t"
                   << SAFE_PRINT_PSZ (pszValue)
                   << " ["
                   << SAFE_PRINT_PSZ (pszXLate)
                   << "]";

         if (  pszDefault
            && 0 == strcmp (pszDefault, pszValue)
            )
         {
            std::cout << " (*)";
         }

         std::cout << std::endl;

         if (aszXLates)
         {
            pdcFree ((void *)aszXLates);
            aszXLates = 0;
         }

         pszValue += strlen (pszValue) + 1;
      }

      if (aszValues)
      {
         pdcFree ((void *)aszValues);
         aszValues = 0;
      }

      if (pszDefault)
      {
         pdcFree ((void *)pszDefault);
         pszDefault = 0;
      }

      pszKey += strlen (pszKey) + 1;
   }

   if (aszKeys)
   {
      pdcFree ((void *)aszKeys);
      aszKeys = 0;
   }
}

void
executeDevice (DRIVERHANDLE  handleDriver,
               const char   *pszDeviceHandle)
{
   DEVICEHANDLE  handleDevice   = 0;
   STRINGARRAY   aszLanguages   = 0;
   const char   *pszLanguage    = 0;
   bool          fFirst         = false;
   int           iRC            = 0;

   handleDevice = pdcOpenDevice (handleDriver, pszDeviceHandle);

   if (handleDevice)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcOpenDevice" << std::endl;

   if (!handleDevice)
   {
      std::cerr << "Error: Cannot open a device!" << std::endl;

      return;
   }

   aszLanguages = pdcQueryTranslatableLanguages (handleDevice);
   pszLanguage  = aszLanguages;
   fFirst       = true;

   if (aszLanguages)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcQueryTranslatableLanguages" << std::endl;

   std::cout << "Translatable strings are: ";

   while (  pszLanguage
         && *pszLanguage
         )
   {
      std::cout << SAFE_PRINT_PSZ (pszLanguage);

      if (fFirst)
      {
         fFirst = false;
      }
      else
      {
         std::cout << " ";
      }

      pszLanguage += strlen (pszLanguage) + 1;
   }

   std::cout << std::endl;

   if (aszLanguages)
   {
      pdcFree ((void *)aszLanguages);
      aszLanguages = 0;
   }

   handleDeviceJobProperties (handleDevice);

   iRC = pdcCloseDevice (handleDevice);

   if (iRC)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcCloseDevice" << std::endl;
}

void
executeDriver (PDCHANDLE   handlePDC,
               const char *pszDriver)
{
   DRIVERHANDLE    handleDriver = 0;
   STRINGPAIRARRAY aszDevices   = 0;

   handleDriver = pdcOpenDriver (handlePDC, pszDriver);

   if (handleDriver)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcOpenDriver" << std::endl;

   if (!handleDriver)
   {
      std::cerr << "Error: Cannot open a driver!" << std::endl;

      return;
   }

   aszDevices = pdcEnumerateDevices (handleDriver);

   if (aszDevices)
   {
      std::cout << "PASS: ";
   }
   else
   {
      std::cout << "FAIL: ";
   }
   std::cout << "pdcEnumerateDevices" << std::endl;

   if (aszDevices)
   {
      const char *pszDevice = aszDevices;

      while (  pszDevice
            && *pszDevice
            )
      {
         const char *pszDeviceHandle = 0;

         pszDeviceHandle = pszDevice + strlen (pszDevice) + 1;

         std::cout << "For \"" << pszDevice << "\"" << std::endl;

         executeDevice (handleDriver, pszDeviceHandle);

         pszDevice = pszDeviceHandle + strlen (pszDeviceHandle) + 1;
      }
   }

   // Clean up!
   if (aszDevices)
   {
      pdcFree ((void *)aszDevices);
   }
   if (handleDriver)
   {
      int iRC = 0;

      iRC = pdcCloseDriver (handleDriver);

      if (iRC)
      {
         std::cout << "PASS: ";
      }
      else
      {
         std::cout << "FAIL: ";
      }
      std::cout << "pdcCloseDriver" << std::endl;
   }
}

int
main (int argc, char *argv[])
{
   PDCHANDLE    handlePDC   = 0;
   STRINGARRAY  aszDrivers  = 0;
   const char  *pszDriver   = 0;
   const char  *pszDevice   = 0;

   handlePDC = pdcOpenLibrary ();

   if (!handlePDC)
   {
      std::cerr << "Error: Cannot open the PDC library!" << std::endl;

      return 1;
   }

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "--driver"))
      {
         pszDriver = argv[++i];
      }
      else if (0 == strcasecmp (argv[i], "--device"))
      {
         pszDevice = argv[++i];
      }
      else if (  0 == strcasecmp (argv[i], "--help")
              || 0 == strcasecmp (argv[i], "-?")
              )
      {
         printUsage (argv[0]);

         return 1;
      }
   }

   if (  pszDriver
      && pszDevice
      )
   {
      DRIVERHANDLE handleDriver = 0;

      handleDriver = pdcOpenDriver (handlePDC, pszDriver);

      if (handleDriver)
      {
         std::cout << "PASS: ";
      }
      else
      {
         std::cout << "FAIL: ";
      }
      std::cout << "pdcOpenDriver" << std::endl;

      if (handleDriver)
      {
         executeDevice (handleDriver, pszDevice);

         int iRC = 0;

         iRC = pdcCloseDriver (handleDriver);

         if (iRC)
         {
            std::cout << "PASS: ";
         }
         else
         {
            std::cout << "FAIL: ";
         }
         std::cout << "pdcCloseDriver" << std::endl;
      }
   }
   else if (  !pszDriver
           && !pszDevice
           )
   {
      aszDrivers = pdcEnumerateDrivers (handlePDC);

      if (aszDrivers)
      {
         std::cout << "PASS: ";
      }
      else
      {
         std::cout << "FAIL: ";
      }
      std::cout << "pdcEnumerateDrivers" << std::endl;

      pszDriver = aszDrivers;

      while (  pszDriver
            && *pszDriver
            )
      {
         executeDriver (handlePDC, pszDriver);

         pszDriver += strlen (pszDriver) + 1;
      }
   }
   else if (!pszDevice)
   {
      executeDriver (handlePDC, pszDriver);
   }
   else
   {
      // Error!
   }

   // Clean up!
   if (aszDrivers)
   {
      pdcFree ((void *)aszDrivers);
   }

   if (handlePDC)
   {
      int iRC = 0;

      iRC = pdcCloseLibrary (handlePDC);

      if (iRC)
      {
         std::cout << "PASS: ";
      }
      else
      {
         std::cout << "FAIL: ";
      }
      std::cout << "pdcCloseLibrary" << std::endl;
   }

   return 0;
}
