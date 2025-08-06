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

const bool vfDebug = false;

#define SAFE_PRINT_PSZ(pszString) (pszString ? pszString : "(null)")

PDCMethodLink::
PDCMethodLink (DriverMap *pMapDriver)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (0x" << std::hex << pMapDriver << std::dec << ")" << std::endl;
#endif

   hmodDevice_d                    = 0;

   pfnInitializeSession_d          = 0;
   pfnCloseSession_d               = 0;
   pfnEnumerateDevices_d           = 0;
   pfnGetDeviceHandle_d            = 0;
   pfnOpenDevice_d                 = 0;
   pfnCloseDevice_d                = 0;
   pfnSetTranslatableLanguage_d    = 0;
   pfnGetTranslatableLanguage_d    = 0;
   pfnQueryTranslatableLanguages_d = 0;
   pfnGetPDLInformation_d          = 0;
   pfnQueryCurrentJobProperties_d  = 0;
   pfnSetJobProperties_d           = 0;
   pfnListJobPropertyKeys_d        = 0;
   pfnGetJobProperty_d             = 0;
   pfnGetJobPropertyType_d         = 0;
   pfnTranslateJobProperty_d       = 0;
   pfnBeginJob_d                   = 0;
   pfnStartPage_d                  = 0;
   pfnStartPageWithProperties_d    = 0;
   pfnEndPage_d                    = 0;
   pfnEndJob_d                     = 0;
   pfnAbortPage_d                  = 0;
   pfnAbortJob_d                   = 0;

   loadDriver (pMapDriver);
}

PDCMethodLink::
~PDCMethodLink ()
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::~" << __FUNCTION__ << std::endl;
#endif

   if (hmodDevice_d)
   {
      g_module_close (hmodDevice_d);
      hmodDevice_d = 0;
   }
}

bool PDCMethodLink::
loadDriver (DriverMap *pMapDriver)
{
   if (hmodDevice_d)
   {
      return true;
   }

   std::string  stringLibraryKey ("library");
   std::string  stringLibraryValue;
   bool         fSuccess                     = false;

   stringLibraryValue = (*pMapDriver)[stringLibraryKey];

   if (!stringLibraryValue.size ())
      return false;

   hmodDevice_d = g_module_open (stringLibraryValue.c_str (), (GModuleFlags)0);

#ifndef RETAIL
   if (vfDebug)
   {
      std::cerr << "PDCMethodLink::" << __FUNCTION__ << ": hmodDevice_d = " << std::hex << hmodDevice_d << std::dec << std::endl;
      if (!hmodDevice_d)
      {
         std::cerr << "PDCMethodLink::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;
      }
   }
#endif

   if (!hmodDevice_d)
   {
      return false;
   }

   typedef struct _PDCFunctions {
      const char *pszName;
      gpointer   *pptr;
   } PDCFUNCTIONS, *PPDCFUNCTIONS;
   PDCFUNCTIONS aPDCFunctions[] = {
      { "pdcInitializeSession",          (gpointer*)&pfnInitializeSession_d          },
      { "pdcCloseSession",               (gpointer*)&pfnCloseSession_d               },
      { "pdcEnumerateDevices",           (gpointer*)&pfnEnumerateDevices_d           },
      { "pdcGetDeviceHandle",            (gpointer*)&pfnGetDeviceHandle_d            },
      { "pdcOpenDevice",                 (gpointer*)&pfnOpenDevice_d                 },
      { "pdcCloseDevice",                (gpointer*)&pfnCloseDevice_d                },
      { "pdcSetTranslatableLanguage",    (gpointer*)&pfnSetTranslatableLanguage_d    },
      { "pdcGetTranslatableLanguage",    (gpointer*)&pfnGetTranslatableLanguage_d    },
      { "pdcQueryTranslatableLanguages", (gpointer*)&pfnQueryTranslatableLanguages_d },
      { "pdcGetPDLInformation",          (gpointer*)&pfnGetPDLInformation_d          },
      { "pdcQueryCurrentJobProperties",  (gpointer*)&pfnQueryCurrentJobProperties_d  },
      { "pdcSetJobProperties",           (gpointer*)&pfnSetJobProperties_d           },
      { "pdcListJobPropertyKeys",        (gpointer*)&pfnListJobPropertyKeys_d        },
      { "pdcListJobPropertyKeyValues",   (gpointer*)&pfnListJobPropertyKeyValues_d   },
      { "pdcGetJobProperty",             (gpointer*)&pfnGetJobProperty_d             },
      { "pdcGetJobPropertyType",         (gpointer*)&pfnGetJobPropertyType_d         },
      { "pdcTranslateJobProperty",       (gpointer*)&pfnTranslateJobProperty_d       },
      { "pdcBeginJob",                   (gpointer*)&pfnBeginJob_d                   },
      { "pdcStartPage",                  (gpointer*)&pfnStartPage_d                  },
      { "pdcStartPageWithProperties",    (gpointer*)&pfnStartPageWithProperties_d    },
      { "pdcEndPage",                    (gpointer*)&pfnEndPage_d                    },
      { "pdcEndJob",                     (gpointer*)&pfnEndJob_d                     },
      { "pdcAbortPage",                  (gpointer*)&pfnAbortPage_d                  },
      { "pdcAbortJob",                   (gpointer*)&pfnAbortJob_d                   }
   };

   fSuccess = true;

   for (int i = 0; i < (int)dimof (aPDCFunctions); i++)
   {
      gpointer ptr = 0;

      g_module_symbol (hmodDevice_d, aPDCFunctions[i].pszName, &ptr);

#ifndef RETAIL
      if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << ": Loading " << aPDCFunctions[i].pszName << " = " << std::hex << ptr << std::dec << std::endl;
#endif

      if (ptr)
      {
         *aPDCFunctions[i].pptr = ptr;
      }
      else
      {
         fSuccess = false;
      }
   }

   return true;
}

STRINGPAIRARRAY PDCMethodLink::
pdcEnumerateDevices ()
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << ": pfnEnumerateDevices_d = " << std::hex << (int)pfnEnumerateDevices_d << std::dec << std::endl;
#endif

   if (pfnEnumerateDevices_d)
   {
      return pfnEnumerateDevices_d ();
   }

   return 0;
}

const char * PDCMethodLink::
pdcGetDeviceHandle (const char *pszDisplayHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << "(\"" << SAFE_PRINT_PSZ (pszDisplayHandle) << "\"): pfnGetDeviceHandle_d = " << std::hex << (int)pfnGetDeviceHandle_d << std::dec << std::endl;
#endif

   if (pfnGetDeviceHandle_d)
   {
      return pfnGetDeviceHandle_d (pszDisplayHandle);
   }

   return 0;
}

const void * PDCMethodLink::
pdcOpenDevice (const char *pszDisplayHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ (pszDisplayHandle) << "\"): pfnOpenDevice_d = " << std::hex << (int)pfnOpenDevice_d << std::dec << std::endl;
#endif

   if (pfnOpenDevice_d)
   {
      return pfnOpenDevice_d (pszDisplayHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcCloseDevice (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" <<std::hex << (int)pvDeviceHandle << "): pfnCloseDevice_d = " << (int)pfnCloseDevice_d << std::dec << std::endl;
#endif

   if (pfnCloseDevice_d)
   {
      return pfnCloseDevice_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcSetTranslatableLanguage (const void *pvDeviceHandle,
                            const char *pszLanguage)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszLanguage) << "\"): pfnSetTranslatableLanguage_d = " << std::hex << (int)pfnSetTranslatableLanguage_d << std::dec << std::endl;
#endif

   if (pfnSetTranslatableLanguage_d)
   {
      return pfnSetTranslatableLanguage_d (pvDeviceHandle, pszLanguage);
   }

   return 0;
}

const char * PDCMethodLink::
pdcGetTranslatableLanguage (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnGetTranslatableLanguage_d = " << (int)pfnGetTranslatableLanguage_d << std::dec << std::endl;
#endif

   if (pfnGetTranslatableLanguage_d)
   {
      return pfnGetTranslatableLanguage_d (pvDeviceHandle);
   }

   return 0;
}

STRINGARRAY PDCMethodLink::
pdcQueryTranslatableLanguages (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnQueryTranslatableLanguages_d = " << (int)pfnQueryTranslatableLanguages_d << std::dec << std::endl;
#endif

   if (pfnQueryTranslatableLanguages_d)
   {
      return pfnQueryTranslatableLanguages_d (pvDeviceHandle);
   }

   return 0;
}

const char * PDCMethodLink::
pdcGetPDLInformation (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnGetPDLInformation_d = " << (int)pfnGetPDLInformation_d << std::dec << std::endl;
#endif

   if (pfnGetPDLInformation_d)
   {
      return pfnGetPDLInformation_d (pvDeviceHandle);
   }

   return 0;
}

const char * PDCMethodLink::
pdcQueryCurrentJobProperties (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnQueryCurrentJobProperties_d = " << (int)pfnQueryCurrentJobProperties_d << std::dec << std::endl;
#endif

   if (pfnQueryCurrentJobProperties_d)
   {
      return pfnQueryCurrentJobProperties_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcSetJobProperties (const void *pvDeviceHandle,
                     const char *pszJobProperties)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\"): pfnSetJobProperties_d = " << (int)pfnSetJobProperties_d << std::dec << std::endl;
#endif

   if (pfnSetJobProperties_d)
   {
      return pfnSetJobProperties_d (pvDeviceHandle, pszJobProperties);
   }

   return 0;
}

STRINGARRAY PDCMethodLink::
pdcListJobPropertyKeys (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnListJobPropertyKeys_d = " << (int)pfnListJobPropertyKeys_d << std::dec << std::endl;
#endif

   if (pfnListJobPropertyKeys_d)
   {
      return pfnListJobPropertyKeys_d (pvDeviceHandle);
   }

   return 0;
}

STRINGARRAY PDCMethodLink::
pdcListJobPropertyKeyValues (const void *pvDeviceHandle,
                             const char *pszKey)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\"): pfnGetJobProperty_d = " << (int)pfnGetJobProperty_d << std::dec << std::endl;
#endif

   if (pfnListJobPropertyKeyValues_d)
   {
      return pfnListJobPropertyKeyValues_d (pvDeviceHandle, pszKey);
   }

   return 0;
}

const char * PDCMethodLink::
pdcGetJobProperty (const void *pvDeviceHandle,
                   const char *pszKey)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\"): pfnGetJobProperty_d = " << (int)pfnGetJobProperty_d << std::dec << std::endl;
#endif

   if (pfnGetJobProperty_d)
   {
      return pfnGetJobProperty_d (pvDeviceHandle, pszKey);
   }

   return 0;
}

const char * PDCMethodLink::
pdcGetJobPropertyType (const void *pvDeviceHandle,
                       const char *pszKey)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\"): pfnGetJobPropertyType_d = " << (int)pfnGetJobPropertyType_d << std::dec << std::endl;
#endif

   if (pfnGetJobPropertyType_d)
   {
      return pfnGetJobPropertyType_d (pvDeviceHandle, pszKey);
   }

   return 0;
}

STRINGARRAY PDCMethodLink::
pdcTranslateJobProperty (const void *pvDeviceHandle,
                         const char *pszKey,
                         const char *pszValue)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszKey) << ", \"" << SAFE_PRINT_PSZ (pszValue) << "\"): pfnTranslateJobProperty_d = " << (int)pfnTranslateJobProperty_d << std::dec << std::endl;
#endif

   if (pfnTranslateJobProperty_d)
   {
      return pfnTranslateJobProperty_d (pvDeviceHandle, pszKey, pszValue);
   }

   return 0;
}

int PDCMethodLink::
pdcBeginJob (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pvDeviceHandle = " << (int)pvDeviceHandle << std::dec << std::endl;
#endif

   if (pvDeviceHandle)
   {
      return pfnBeginJob_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcStartPage (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnStartPage_d = " << (int)pfnStartPage_d << std::dec << std::endl;
#endif

   if (pfnStartPage_d)
   {
      return pfnStartPage_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcStartPageWithProperties (const void *pvDeviceHandle,
                            const char *pszJobProperties)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\"): pfnStartPageWithProperties_d = " << (int)pfnStartPageWithProperties_d << std::dec << std::endl;
#endif

   if (pfnStartPageWithProperties_d)
   {
      return pfnStartPageWithProperties_d (pvDeviceHandle, pszJobProperties);
   }

   return 0;
}

int PDCMethodLink::
pdcEndPage (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnEndPage_d = " << (int)pfnEndPage_d << std::dec << std::endl;
#endif

   if (pfnEndPage_d)
   {
      return pfnEndPage_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcEndJob (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnEndJob_d = " << (int)pfnEndJob_d << std::dec << std::endl;
#endif

   if (pfnEndJob_d)
   {
      return pfnEndJob_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcAbortPage (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnAbortPage_d = " << (int)pfnAbortPage_d << std::dec << std::endl;
#endif

   if (pfnAbortPage_d)
   {
      return pfnAbortPage_d (pvDeviceHandle);
   }

   return 0;
}

int PDCMethodLink::
pdcAbortJob (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "PDCMethodLink::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << "): pfnAbortJob_d = " << (int)pfnAbortJob_d << std::dec << std::endl;
#endif

   if (pfnAbortJob_d)
   {
      return pfnAbortJob_d (pvDeviceHandle);
   }

   return 0;
}
