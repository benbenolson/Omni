/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#include "DriverInfo.hpp"
#include "Driver.hpp"

#include <Device.hpp>
#include <Omni.hpp>
#include <JobProperties.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <glib.h>
#include <gmodule.h>

DriverInfo::
DriverInfo ()
{
   pDevice_d    = 0;
#ifndef PDC_INTERFACE_ENABLED
   hmodDevice_d = 0;
#endif
}

DriverInfo::
~DriverInfo ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": enter" << std::endl;
#endif

   closeDevice ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ <<  ": exit" << std::endl;
#endif
}

bool DriverInfo::
translateKeyValue (PSZCRO       pszKey,
                   PSZCRO       pszValue,
                   std::string& stringInternal,
                   std::string& stringExternal)
{
   if (!pszKey)
   {
      return false;
   }

   std::string *pstringXLate = pDevice_d->translateKeyValue (pszKey, pszValue);
   bool         fSuccess     = false;

   if (pstringXLate)
   {
      std::string::size_type pos = pstringXLate->find ("=");

      fSuccess = true;

      if (std::string::npos != pos)
      {
         stringInternal = pszValue;
         stringExternal = pstringXLate->substr (pos + 1);

         if (0 == stringExternal.length ())
         {
            stringExternal = stringInternal;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputDriverInfo ())
            DebugOutput::getErrorStream () << "DriverInfo::"
                                           << __FUNCTION__
                                           << ": \""
                                           << *pstringXLate
                                           << "\" -> \""
                                           << stringInternal
                                           << "\" & \""
                                           << stringExternal
                                           << "\""
                                           << std::endl;
#endif
      }
      else
      {
         stringInternal = pszKey;
         stringExternal = *pstringXLate;
      }

      delete pstringXLate;
   }

   if (!fSuccess)
   {
      stringInternal = pszKey;

      if (pszValue)
      {
         stringExternal = pszValue;
      }
      else
      {
         stringExternal = pszKey;
      }
   }

   return fSuccess;
}

void DriverInfo::
generateDriverInfo ()
{
   std::vector<std::string> listExternal (25);
   std::vector<std::string> listInternal (25);

   Enumeration *pEnumJPGroups = 0;

   pEnumJPGroups = pDevice_d->listJobProperties ();

   while (pEnumJPGroups->hasMoreElements ())
   {
      Enumeration *pEnumJPGroup       = 0;
      bool         fFirstEntry        = true;
      std::string  stringKeyInternal;
      std::string  stringKeyExternal;

      listExternal.clear ();
      listInternal.clear ();

      pEnumJPGroup = (Enumeration *)pEnumJPGroups->nextElement ();

      while (pEnumJPGroup->hasMoreElements ())
      {
         JobProperties         *pJPs     = 0;
         JobPropertyEnumerator *pEnumJPs = 0;

         pJPs     = (JobProperties *)pEnumJPGroup->nextElement ();
         pEnumJPs = pJPs->getEnumeration ();

         while (pEnumJPs->hasMoreElements ())
         {
            PSZRO       pszKey              = pEnumJPs->getCurrentKey   ();
            PSZRO       pszValue            = pEnumJPs->getCurrentValue ();
            std::string stringValueInternal;
            std::string stringValueExternal;

            if (1 == pJPs->getNumProperties ())
            {
               if (fFirstEntry)
               {
                  std::string  stringDefaultValueInternal;
                  std::string  stringDefaultValueExternal;
                  std::string *pstringDefault             = 0;

                  fFirstEntry        = false;
                  stringKeyInternal  = pszKey;

                  translateKeyValue (pszKey,
                                     0,
                                     stringKeyInternal,
                                     stringKeyExternal);

                  pstringDefault = pDevice_d->getJobProperty (pszKey);

                  if (  pstringDefault
                     && translateKeyValue (pszKey,
                                           pstringDefault->c_str (),
                                           stringDefaultValueInternal,
                                           stringDefaultValueExternal)
                     )
                  {
                     listInternal.push_back (stringDefaultValueInternal);
                     listExternal.push_back (stringDefaultValueExternal);
                  }

                  delete pstringDefault;
               }

               if (translateKeyValue (pszKey,
                                      pszValue,
                                      stringValueInternal,
                                      stringValueExternal))
               {
                  listInternal.push_back (stringValueInternal);
                  listExternal.push_back (stringValueExternal);
               }
            }

            pEnumJPs->nextElement ();
         }

         delete pEnumJPs;
         delete pJPs;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputDriverInfo ())
      {
         DebugOutput::getErrorStream () << "DriverInfo::"
                                        << __FUNCTION__
                                        << ": new DriverProperty ("
                                        << stringKeyInternal
                                        << ", "
                                        << stringKeyExternal
                                        << ", ...)"
                                        << std::endl;
         DebugOutput::getErrorStream () << "listExternal:" << std::endl;
         for ( std::vector<std::string>::iterator itr = listExternal.begin ();
               itr != listExternal.end ();
               itr++ )
         {
            DebugOutput::getErrorStream () << *itr << std::endl;
         }
         DebugOutput::getErrorStream () << "listInternal:" << std::endl;
         for ( std::vector<std::string>::iterator itr = listInternal.begin ();
               itr != listInternal.end ();
               itr++ )
         {
            DebugOutput::getErrorStream () << *itr << std::endl;
         }
      }
#endif

      if (!fFirstEntry)
      {
         dps_d.push_back (DriverProperty (stringKeyInternal.c_str (),
                                          stringKeyExternal.c_str (),
                                          "ComboBox",
                                          listExternal,
                                          listInternal));
      }

      delete pEnumJPGroup;
   }

   delete pEnumJPGroups;

   driver_d = Driver ("First Driver", dps_d);
}

std::vector<std::string> DriverInfo::
getDeviceList ()
{
   std::vector<std::string>  deviceList;
   Enumeration              *pEnum       = Omni::listDevices ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": Enumerating the devices. Please wait ... (It may take few seconds)" << std::endl;
#endif

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         PSZCRO pszLibName = pOD->getLibraryName ();

         deviceList.push_back (pszLibName);

         delete pOD;
      }
      else
      {
         std::cerr << "Error: An empty omni device was returned." << std::endl;
      }
   }

   delete pEnum;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << std::endl << "************************************************" << std::endl;
#endif

   for (int iz = 0; iz < (int)deviceList.size (); iz++)
   {
      int point = deviceList[iz].find_last_of ('/');

      deviceList[iz] = deviceList[iz].substr (point + 4);
      deviceList[iz] = deviceList[iz].erase (deviceList[iz].size() - 3);
   }

   return deviceList;
}

int DriverInfo::
openDevice (const char *pszFullDeviceName,
            const char *pszJobProperties)
{
#ifdef PDC_INTERFACE_ENABLED

   pDevice_d = new OmniPDCProxy (0,
                                 pszFullDeviceName,
                                 pszJobProperties,
                                 true);

#else

   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs = 0;

   Omni::openAndTestDeviceLibrary (pszFullDeviceName, &hmodDevice_d);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ <<  ": hmodDevice_d = " << hmodDevice_d << std::endl;
#endif

   if (!hmodDevice_d)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": g_module_error returns " << g_module_error () << std::endl;
#endif

      return 2;
   }

   g_module_symbol (hmodDevice_d, "newDeviceW_JopProp_Advanced", (gpointer *)&pfnNewDeviceWArgs);

   if (!pfnNewDeviceWArgs)
      return 3;

   pDevice_d = pfnNewDeviceWArgs (pszJobProperties, true);

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": pDevice_d = " << pDevice_d << std::endl;
#endif

#endif

   if (  !pDevice_d
      || pDevice_d->hasError ()
      )
   {
      return 0;
   }
   else
   {
      return 1;
   }
}

int DriverInfo::
closeDevice ()
{
   int rc = 0;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": pDevice_d = " << pDevice_d << std::endl;
#ifndef PDC_INTERFACE_ENABLED
   if (DebugOutput::shouldOutputDriverInfo ()) DebugOutput::getErrorStream () << "DriverInfo::" << __FUNCTION__ << ": hmodDevice_d = " << hmodDevice_d << std::endl;
#endif
#endif

   delete pDevice_d;
   pDevice_d = 0;

#ifndef PDC_INTERFACE_ENABLED
   if (hmodDevice_d)
   {
      rc = g_module_close (hmodDevice_d);
      hmodDevice_d = 0;
   }
#endif

   return rc;
}

Driver DriverInfo::
getDriver ()
{
   return driver_d;
}
