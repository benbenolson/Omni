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
#include <PDC/pdc_driver.h>

#include "Omni.hpp"

#include <ostream>

class ByteStream
{
   #define SIZEOF_DATA      (64*1024)
public:
   ByteStream ()
   {
      iMembs_d         = 1;
      pbData_d         = (byte *)calloc (iMembs_d, SIZEOF_DATA);
      cbLeft_d         = SIZEOF_DATA;
      iOffsetCurrent_d = 0;
   }
   void
   addData (const char *pszData)
   {
      if (!pszData)
         return;

      if (!growData (strlen (pszData)))
         return;

      appendData ((byte *)pszData, strlen (pszData));
   }
   void
   addData (char chData)
   {
      if (!growData (1))
         return;

      appendData ((byte *)&chData, 1);
   }
   const byte *
   getData ()
   {
      return pbData_d;
   }
   int
   getSize ()
   {
      return iOffsetCurrent_d;
   }
private:
   bool
   growData (int cbRequested)
   {
      if (cbRequested > cbLeft_d)
      {
         byte *pbNewData = 0;

         pbNewData = (byte *)calloc (iMembs_d + 1, SIZEOF_DATA);

         if (pbNewData)
         {
            memcpy (pbNewData, pbData_d, iMembs_d * SIZEOF_DATA);

            pbData_d = pbNewData;
            iMembs_d++;

            return true;
         }
         else
         {
            return false;
         }
      }
      else
      {
         return true;
      }
   }
   void
   appendData (byte *pszData, int cbData)
   {
      memcpy (pbData_d + iOffsetCurrent_d, pszData, cbData);
      iOffsetCurrent_d += cbData;
      cbLeft_d         -= cbData;
   }

   byte *pbData_d;
   int   iMembs_d;
   int   cbLeft_d;
   int   iOffsetCurrent_d;
};

typedef struct _OmniHandle {
   int         cbSize;
   char        achSignature[4];
   Device     *pDevice;
   GModule    *hmodDevice;
   bool        fInPage;
   int         iPageNumber;
} OMNIHANDLE, *POMNIHANDLE;

#define OMNI_SIGNATURE 0x494E4D4F // 'OMNI'

POMNIHANDLE
validateOmniHandle (const void *pvOmniHandle)
{
   POMNIHANDLE pOmniHandle = (POMNIHANDLE)pvOmniHandle;

   if (!pOmniHandle)
   {
      return 0;
   }
   if (sizeof (OMNIHANDLE) != pOmniHandle->cbSize)
   {
      return 0;
   }
   if (OMNI_SIGNATURE != *(int *)pOmniHandle->achSignature)
   {
      return 0;
   }
   if (!dynamic_cast <Device *>(pOmniHandle->pDevice))
   {
      return 0;
   }
   if (!pOmniHandle->hmodDevice)
   {
      return 0;
   }

   return pOmniHandle;
}

void
FreeOmniHandle (const void *pvOmniHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvOmniHandle);

   if (pOmniHandle)
   {
      delete pOmniHandle->pDevice;
      pOmniHandle->pDevice = 0;

      if (pOmniHandle->hmodDevice)
      {
         g_module_close (pOmniHandle->hmodDevice);
         pOmniHandle->hmodDevice = 0;
      }

      free ((void *)pOmniHandle);
   }
}

const char *
pdcInitializeSession (const char *pszClientVersion)
{
   Omni::initialize ();

#ifndef RETAIL
   DebugOutput::setDebugOutput ("all");
#endif

   return "@TBD";
}

void
pdcCloseSession ()
{
   Omni::terminate ();
}

STRINGPAIRARRAY
pdcEnumerateDevices ()
{
   Enumeration     *pEnum     = 0;
   ByteStream      *pBS       = 0;
   STRINGPAIRARRAY  aszReturn = 0;

   pEnum = Omni::listDevices (false);
   if (!pEnum)
   {
      return 0;
   }

   pBS = new ByteStream ();

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         GModule *hmodDevice = 0;
         Device  *pDevice    = Omni::createDevice (pOD, &hmodDevice);

         if (pDevice)
         {
            pBS->addData (pDevice->getDriverName ());
            pBS->addData ('.');
            pBS->addData (pDevice->getDeviceName ());
            pBS->addData ('\0');
            pBS->addData (pDevice->getShortName ());
            pBS->addData ('\0');

            delete pDevice;
         }
         if (hmodDevice)
         {
            g_module_close (hmodDevice);
         }
      }

      delete pOD;
   }

   delete pEnum;

   pBS->addData ('\0');

   aszReturn = (char *)pBS->getData ();

   delete pBS;

   return aszReturn;
}

const char *
pdcGetDeviceHandle (const char *pszDisplayHandle)
{
   return 0;
}

const void *
pdcOpenDevice (const char *pszDeviceHandle)
{
   OmniDevice *pOD = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (\"" << SAFE_PRINT_PSZ (pszDeviceHandle) << "\")" << std::endl;
#endif

   pOD = Omni::findOmniDeviceEntry (pszDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " : pOD = " << pOD << std::endl;
#endif

   if (!pOD)
   {
      return 0;
   }

   PSZCRO             pszLibName        = pOD->getLibraryName ();
   PSZCRO             pszJobProperties  = pOD->getJobProperties ();
   Device            *pDevice           = 0;
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;
   GModule           *hmodDevice        = 0;

   hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

   if (hmodDevice)
   {
      g_module_symbol (hmodDevice,
                       "newDeviceW_JopProp_Advanced",
                       (gpointer *)&pfnNewDeviceWArgs);

      if (pfnNewDeviceWArgs)
      {
         pDevice = pfnNewDeviceWArgs (pszJobProperties, true);

         if (pDevice)
         {
            POMNIHANDLE pOmniHandle = 0;

            pOmniHandle = (POMNIHANDLE)calloc (1, sizeof (OMNIHANDLE));

            if (pOmniHandle)
            {
               pOmniHandle->cbSize = sizeof (OMNIHANDLE);
               *(int *)pOmniHandle->achSignature = OMNI_SIGNATURE;
               pOmniHandle->pDevice     = pDevice;
               pOmniHandle->hmodDevice  = hmodDevice;
               pOmniHandle->fInPage     = false;
               pOmniHandle->iPageNumber = 0;

               delete pOD;

               return pOmniHandle;
            }
         }
      }
   }

   // Clean up
   delete pOD;

   if (pDevice)
   {
      delete pDevice;
   }
   if (hmodDevice)
   {
      g_module_close (hmodDevice);
   }

   return 0;
}

int
pdcCloseDevice (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      FreeOmniHandle (pvDeviceHandle);

      return 1;
   }

   return 0;
}

int
pdcSetTranslatableLanguage (const void *pvDeviceHandle,
                            const char *pszLanguage)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszLanguage) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice     = 0;
      int     iLanguageID = StringResource::LANGUAGE_UNKNOWN;

      pDevice     = pOmniHandle->pDevice;
      iLanguageID = StringResource::nameToID (pszLanguage);

      if (pDevice->setLanguage (iLanguageID))
      {
         return 1;
      }
   }

   return 0;
}

const char *
pdcGetTranslatableLanguage (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device     *pDevice     = 0;
      int         iLanguageID = StringResource::LANGUAGE_UNKNOWN;
      const char *pszLanguage = 0;
      const char *pszReturn   = 0;

      pDevice     = pOmniHandle->pDevice;
      iLanguageID = (int)pDevice->getLanguage ();
      pszLanguage = StringResource::IDToName (iLanguageID);

      pszReturn = (const char *)malloc (strlen (pszLanguage) + 1);
      if (pszReturn)
      {
         strcpy ((char *)pszReturn, pszLanguage);
      }

      return pszReturn;
   }

   return 0;
}

STRINGARRAY
pdcQueryTranslatableLanguages (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device             *pDevice          = pOmniHandle->pDevice;
      Enumeration        *pEnum            = 0;
      bool                fFirstElement    = true;
      ByteStream         *pBS              = 0;
      STRINGARRAY         aszReturn        = 0;

      pBS = new ByteStream ();

      pEnum = pDevice->getLanguages ();

      while (  pEnum
            && pEnum->hasMoreElements ()
            )
      {
         if (!fFirstElement)
         {
            pBS->addData ('\0');
         }

         int iLanguageID = (int)pEnum->nextElement ();

         pBS->addData (StringResource::IDToName (iLanguageID));

         fFirstElement = false;
      }

      delete pEnum;

      pBS->addData ('\0');

      aszReturn = (char *)pBS->getData ();

      delete pBS;

      return aszReturn;
   }

   return 0;
}

const char *
pdcGetPDLInformation (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

   }

   return 0;
}

const char *
pdcGetJobProperties (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device      *pDevice    = 0;
      std::string *pstringJPs = 0;

      pDevice    = pOmniHandle->pDevice;
      pstringJPs = pDevice->getJobProperties ();

      if (pstringJPs)
      {
         const char *pszReturn = 0;

         pszReturn = (const char *)malloc (pstringJPs->length () + 1);
         if (pszReturn)
         {
            strcpy ((char *)pszReturn, pstringJPs->c_str ());
         }

         delete pstringJPs;

         return pszReturn;
      }
   }

   return 0;
}

int
pdcSetJobProperties (const void *pvDeviceHandle,
                     const char *pszJobProperties)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (pDevice->setJobProperties (pszJobProperties))
      {
         return 0;
      }
      else
      {
         return 1;
      }
   }

   return 0;
}

STRINGARRAY
pdcListJobPropertyKeys (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device             *pDevice          = pOmniHandle->pDevice;
      Enumeration        *pEnum            = 0;
      bool                fFirstElement    = true;
      ByteStream         *pBS              = 0;
      STRINGARRAY         aszReturn        = 0;

      pBS = new ByteStream ();

      pEnum = pDevice->listJobPropertyKeys ();

      while (  pEnum
            && pEnum->hasMoreElements ()
            )
      {
         if (!fFirstElement)
         {
            pBS->addData ('\0');
         }

         pBS->addData ((char *)pEnum->nextElement ());

         fFirstElement = false;
      }

      delete pEnum;

      pEnum = pDevice->listDeviceJobPropertyKeys ();

      while (  pEnum
            && pEnum->hasMoreElements ()
            )
      {
         if (!fFirstElement)
         {
            pBS->addData ('\0');
         }

         pBS->addData ((char *)pEnum->nextElement ());

         fFirstElement = false;
      }

      delete pEnum;

      pBS->addData ('\0');

      aszReturn = (char *)pBS->getData ();

      delete pBS;

      return aszReturn;
   }

   return 0;
}

STRINGARRAY
pdcListJobPropertyKeyValues (const void *pvDeviceHandle,
                             const char *pszKey)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device             *pDevice          = pOmniHandle->pDevice;
      Enumeration        *pEnum            = 0;
      bool                fFirstElement    = true;
      ByteStream         *pBS              = 0;
      STRINGARRAY         aszReturn        = 0;

      pBS = new ByteStream ();

      pEnum = pDevice->listKeyValues (pszKey);

      while (  pEnum
            && pEnum->hasMoreElements ()
            )
      {
         if (!fFirstElement)
         {
            pBS->addData ('\0');
         }

         pBS->addData ((char *)pEnum->nextElement ());

         fFirstElement = false;
      }

      delete pEnum;

      pBS->addData ('\0');

      aszReturn = (char *)pBS->getData ();

      delete pBS;

      return aszReturn;
   }

   return 0;
}

const char *
pdcGetJobProperty (const void *pvDeviceHandle,
                   const char *pszKey)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device      *pDevice      = 0;
      std::string *pstringValue = 0;

      pDevice      = pOmniHandle->pDevice;
      pstringValue = pDevice->getJobProperty (pszKey);

      if (pstringValue)
      {
         const char *pszReturn = 0;

         pszReturn = (const char *)malloc (pstringValue->length () + 1);
         if (pszReturn)
         {
            strcpy ((char *)pszReturn, pstringValue->c_str ());
         }

         delete pstringValue;

         return pszReturn;
      }
   }

   return 0;
}

const char *
pdcGetJobPropertyType (const void *pvDeviceHandle,
                       const char *pszKey)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszKey) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device      *pDevice      = 0;
      std::string *pstringValue = 0;

      pDevice      = pOmniHandle->pDevice;
      pstringValue = pDevice->getJobPropertyType (pszKey);

      if (pstringValue)
      {
         const char *pszReturn = 0;

         pszReturn = (const char *)malloc (pstringValue->length () + 1);
         if (pszReturn)
         {
            strcpy ((char *)pszReturn, pstringValue->c_str ());
         }

         delete pstringValue;

         return pszReturn;
      }
   }

   return 0;
}

STRINGARRAY
pdcTranslateJobProperty (const void *pvDeviceHandle,
                         const char *pszKey,
                         const char *pszValue)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszKey) <<  "\", \"" << SAFE_PRINT_PSZ (pszValue) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device      *pDevice      = 0;
      std::string *pstringValue = 0;

      pDevice      = pOmniHandle->pDevice;
      pstringValue = pDevice->translateKeyValue (pszKey, pszValue);

      if (pstringValue)
      {
         STRINGARRAY aszReturn = 0;

         aszReturn = (STRINGARRAY)calloc (1, pstringValue->length () + 2);
         if (aszReturn)
         {
            char *pszEquals = 0;

            strcpy ((char *)aszReturn, pstringValue->c_str ());

            pszEquals = strchr (aszReturn, '=');
            if (pszEquals)
            {
               *pszEquals = '\0';
            }
         }

         delete pstringValue;

         return aszReturn;
      }
   }

   return 0;
}

int
pdcBeginJob (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (  0     != pOmniHandle->iPageNumber
         || false != pOmniHandle->fInPage
         )
      {
         return 0;
      }

      return 1;
   }

   return 0;
}

int
pdcStartPage (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (pOmniHandle->fInPage)
      {
         return 0;
      }

      pOmniHandle->iPageNumber++;

      if (1 == pOmniHandle->iPageNumber)
      {
         if (pDevice->beginJob ())
         {
            pOmniHandle->fInPage = true;

            return 1;
         }
      }
      else
      {
         if (pDevice->newFrame ())
         {
            pOmniHandle->fInPage = true;

            return 1;
         }
      }
   }

   return 0;
}

int
pdcStartPageWithProperties (const void *pvDeviceHandle,
                            const char *pszJobProperties)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ", \"" << SAFE_PRINT_PSZ (pszJobProperties) << "\")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (pOmniHandle->fInPage)
      {
         return 0;
      }

      pOmniHandle->iPageNumber++;

      if (1 == pOmniHandle->iPageNumber)
      {
         if (pDevice->beginJob (pszJobProperties))
         {
            pOmniHandle->fInPage = true;

            return 1;
         }
      }
      else
      {
         if (pDevice->newFrame (pszJobProperties))
         {
            pOmniHandle->fInPage = true;

            return 1;
         }
      }
   }

   return 0;
}

int
pdcEndPage (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (!pOmniHandle->fInPage)
      {
         return 0;
      }

      pOmniHandle->fInPage = false;

      return 1;
   }

   return 0;
}

int
pdcEndJob (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      if (pOmniHandle->fInPage)
      {
         return 0;
      }

      pOmniHandle->iPageNumber = 0;

      if (pDevice->endJob ())
      {
         return 1;
      }
      else
      {
         return 0;
      }
   }

   return 0;
}

int
pdcAbortPage (const void *pvDeviceHandle)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   // We do not handle aborting of pages!
   return 0;
}

int
pdcAbortJob (const void *pvDeviceHandle)
{
   POMNIHANDLE pOmniHandle = validateOmniHandle (pvDeviceHandle);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCInterface ()) DebugOutput::getErrorStream () << "PDCInterface::" << __FUNCTION__ << " (" << std::hex << (int)pvDeviceHandle << std::dec << ")" << std::endl;
#endif

   if (pOmniHandle)
   {
      Device *pDevice = 0;

      pDevice = pOmniHandle->pDevice;

      pOmniHandle->fInPage     = 0;
      pOmniHandle->iPageNumber = 0;

      if (pDevice->abortJob ())
      {
         return 1;
      }
      else
      {
         return 0;
      }
   }

   return 0;
}
