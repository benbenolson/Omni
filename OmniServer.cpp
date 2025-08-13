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
 *
 * Idea borrowed from http://hpinkjet.sourceforge.net
 */
#include "Device.hpp"
#include "OmniServer.hpp"
#include "PrinterCommand.hpp"
#include "Omni.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <sstream>
#include <cstdint>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/shm.h>

#include <glib.h>
#include <gmodule.h>

/* Function prototypes...
*/

class DeviceReference
{
public:
   DeviceReference (char *pszDeviceName)
   {
      pszLibDeviceName_d  = 0;
      hmodDevice_d        = 0;
      pfnNewDevice_d      = 0;
      pfnNewDeviceWArgs_d = 0;
      pszJobProperties_d  = 0;
      pDevice_d           = 0;

      int cbLibDeviceName = strlen (pszDeviceName) + 1;

      // Check if it starts with lib
      if (0 != strncasecmp (pszDeviceName, "lib", 3))
         cbLibDeviceName += 6;

      pszLibDeviceName_d = (char *)malloc (cbLibDeviceName);

      if (pszLibDeviceName_d)
      {
         if (0 != strncasecmp (pszDeviceName, "lib", 3))
         {
            sprintf (pszLibDeviceName_d, "lib%s.so", pszDeviceName);
         }
         else
         {
            strcpy (pszLibDeviceName_d, pszDeviceName);
         }
      }
   }

   ~DeviceReference ()
   {
      delete pDevice_d;
      pDevice_d = 0;

      if (hmodDevice_d)
      {
         g_module_close (hmodDevice_d);
         hmodDevice_d = 0;
      }

      if (pszLibDeviceName_d)
      {
         free (pszLibDeviceName_d);
         pszLibDeviceName_d = 0;
      }

      if (pszJobProperties_d)
      {
         free (pszJobProperties_d);
         pszJobProperties_d = 0;
      }
   }

   bool
   loadDevice ()
   {
      if (!pszLibDeviceName_d)
         return false;

      if (hmodDevice_d)
         return true;

      Omni::openAndTestDeviceLibrary (pszLibDeviceName_d, &hmodDevice_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": hmodDevice = 0x" << std::hex << (int)hmodDevice_d << std::dec << std::endl;
#endif

      if (!hmodDevice_d)
         return false;

      // Get the constructor
      g_module_symbol (hmodDevice_d,
                       "newDeviceW_Advanced",
                       (gpointer *)&pfnNewDevice_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": pfnNewDevice = 0x" << std::hex << (int)pfnNewDevice_d << std::dec << std::endl;
#endif

      g_module_symbol (hmodDevice_d,
                       "newDeviceW_JopProp_Advanced",
                       (gpointer *)&pfnNewDeviceWArgs_d);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": pfnNewDeviceWArgs = 0x" << std::hex << (int)pfnNewDeviceWArgs_d << std::dec << std::endl;
#endif

      if (  !pfnNewDevice_d
         || !pfnNewDeviceWArgs_d
         )
      {
         g_module_close (hmodDevice_d);
         hmodDevice_d = 0;

         return false;
      }
      else
      {
         return true;
      }
   }

   void
   instantiateDevice (char *pszJobProperties, bool fAdvanced)
   {
      delete pDevice_d;
      pDevice_d = 0;

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pDevice_d = pfnNewDeviceWArgs_d (pszJobProperties, fAdvanced);
      }
      else
      {
         pDevice_d = pfnNewDevice_d (fAdvanced);
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": pDevice = " << *pDevice_d << std::endl;
#endif
   }

   Device *
   getDevice ()
   {
      return pDevice_d;
   }

private:
   char              *pszLibDeviceName_d;
   GModule           *hmodDevice_d;
   PFNNEWDEVICE       pfnNewDevice_d;
   PFNNEWDEVICEWARGS  pfnNewDeviceWArgs_d;
   char              *pszJobProperties_d;
   Device            *pDevice_d;
};

int
main (int argc, char *argv[])
{
   char              *pszS2C;
   char              *pszC2S;
   int                fdS2C                 = -1;            /* server to client */
   int                fdC2S                 = -1;            /* client to server */
   bool               fRun                  = true;
   PDC_CMD            eCommand              = PDCCMD_NACK;
   PrinterCommand    *pCmd                  = 0;
   DeviceReference   *pDeviceRef            = 0;
   bool               fAdvanced             = false;
   char              *pszJobProperties      = 0;
   size_t             cbJobProperties       = 0;
   bool               fNewJobPropertiesSent = false;
   byte              *pbBuffer1             = 0;
   byte              *pbBuffer2             = 0;
   FILE              *pfpOut                = 0;
   FILE              *pfpErr                = 0;
   int                iCurrentLanguage      = StringResource::LANGUAGE_DEFAULT;
   int                iPageNumber           = 0;

   if (getenv ("OMNI_PDC_SERVER_PAUSE"))
   {
      // Handy to attach via gdb debugger.  do:
      // (gdb) file .libs/OmniPDCServer
      // (gdb) shell ps -efl | grep OmniPDCServer          to get the <pid>.
      // (gdb) attach <pid>
      // (gdb) set fPause = 0
      // (gdb) c
      bool fPause = true;

      while (fPause)
      {
      }
   }

   /* Get local copy. */
   pszS2C = getenv ("PDC_SRV_TO_CLIENT");
   pszC2S = getenv ("PDC_CLIENT_TO_SRV");

   if (  !pszS2C
      || !pszC2S
      )
   {
      std::cerr << argv[0] << " must have PDC_SRV_TO_CLIENT and PDC_CLIENT_TO_SRV set in the environment!" << std::endl;

      return 1;
   }

   Omni::initialize ();

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": PDC_SRV_TO_CLIENT=" << pszS2C << std::endl;
   if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": PDC_CLIENT_TO_SRV=" << pszC2S << std::endl;
#endif

   /* Open fifos. */
   if ((fdS2C = open (pszS2C, O_WRONLY)) < 0)
   {
      goto BUGOUT;
   }

   if ((fdC2S = open (pszC2S, O_RDONLY)) < 0)
   {
      goto BUGOUT;
   }

   pCmd = new PrinterCommand ("OmniServer");

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniServer ()) DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": server: ready." << std::endl;
#endif

   while (fRun)
   {
      static char *pszErrorNoDevice      = "Device not created";
      static char *pszErrorUnexpectedANU = "Unexpected ack/nack/unsupported command";
      static char *pszErrorMissingCmd    = "Missing command parameter";

      if (!pCmd->readCommand (fdC2S))
         goto BUGOUT;

      eCommand = PDCCMD_NACK;

      switch (pCmd->getCommandType ())
      {
      case PDCCMD_ACK:
      case PDCCMD_NACK:
      case PDCCMD_UNSUPPORTED:
      {
         if (  !pCmd->setCommand (PDCCMD_NACK, pszErrorUnexpectedANU)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_INITIALIZE_SESSION:
      {
         if (  !pCmd->setCommand (PDCCMD_ACK, PDC_VERSION)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_CLOSE_SESSION:
      {
         fRun = false;
         break;
      }

      case PDCMD_SET_TRANSLATABLE_LANGUAGE:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            int iLangIn = StringResource::nameToID (pCmd->getCommandString (false));

            if (StringResource::LANGUAGE_UNKNOWN != iLangIn)
            {
               Enumeration *pEnum = StringResource::getLanguages ();

               while (pEnum->hasMoreElements ())
               {
                  intptr_t iLangSupported = (intptr_t)pEnum->nextElement ();

                  if (iLangIn == iLangSupported)
                  {
                     iCurrentLanguage = iLangIn;
                     eCommand         = PDCCMD_ACK;
                  }
               }

               delete pEnum;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCMD_GET_TRANSLATABLE_LANGUAGE:
      {

         if (  !pCmd->setCommand (PDCCMD_ACK,
                                  StringResource::IDToName (iCurrentLanguage))
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCMD_QUERY_TRANSLATABLE_LANGUAGES:
      {
         Enumeration *pEnum        = StringResource::getLanguages ();
         int          cbLanguages  = 0;
         char        *pszLanguages = 0;

         while (pEnum->hasMoreElements ())
         {
            pEnum->nextElement ();

            if (cbLanguages)
               cbLanguages++;
            cbLanguages += 2;
         }

         delete pEnum;

         pszLanguages = (char *)calloc (1, cbLanguages);

         pEnum = StringResource::getLanguages ();

         while (pEnum->hasMoreElements ())
         {
            intptr_t iLangSupported = (intptr_t)pEnum->nextElement ();

            if (*pszLanguages)
               strcat (pszLanguages, " ");
            strcat (pszLanguages, StringResource::IDToName ((int)iLangSupported));
         }

         delete pEnum;

         if (  !pCmd->setCommand (PDCCMD_ACK, pszLanguages)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            free ((void *)pszLanguages);
            goto BUGOUT;
         }

         free ((void *)pszLanguages);
         break;
      }

      case PDCCMD_IS_CMD_SUPPORTED:
      {
                  int iCmd = -1;
 
         pCmd->getCommandInt (iCmd);
 
         switch ((unsigned int)iCmd)
         {
         case PDCCMD_ACK:
         case PDCCMD_NACK:
         case PDCCMD_UNSUPPORTED:

         case PDCCMD_INITIALIZE_SESSION:
         case PDCCMD_CLOSE_SESSION:
         case PDCCMD_IS_CMD_SUPPORTED:
         case PDCMD_SET_TRANSLATABLE_LANGUAGE:
         case PDCMD_GET_TRANSLATABLE_LANGUAGE:
         case PDCMD_QUERY_TRANSLATABLE_LANGUAGES:

         case PDCCMD_ENUM_SHORT_DEVICES:
         case PDCCMD_ENUM_LONG_DEVICES:
         case PDCCMD_SET_DEVICE_NAME:
         case PDCCMD_IS_VALID_DEVICE_NAME:
         case PDCCMD_GET_PDL_INFO:

         case PDCCMD_GET_JOB_PROPERTIES:
         case PDCCMD_SET_JOB_PROPERTIES:
         case PDCCMD_GET_JOB_PROPERTY:
         case PDCCMD_GET_JOB_PROPERTY_TYPE:
         case PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE:

         case PDCCMD_NEW_DEVICE:
         case PDCCMD_SET_OUTPUT_STREAM:
         case PDCCMD_BEGIN_JOB:
         case PDCCMD_START_PAGE:
         case PDCCMD_END_PAGE:
         case PDCCMD_END_JOB:
         case PDCCMD_ABORT_JOB:

         case PDCCMD_MODE_IS_RENDERER:
         case PDCCMD_ATTACH_BUFFER1:
         case PDCCMD_ATTACH_BUFFER2:
         case PDCCMD_DETACH_BUFFER1:
         case PDCCMD_DETACH_BUFFER2:
         case PDCCMD_RASTERIZE:

         case PDCMD_IS_COLOR_PRINTER:
         case PDCMD_HAS_HARDWARE_COPY:

         case PDCCMD_GET_VERSION:
         case PDCCMD_GET_DRIVER_NAME:
         case PDCCMD_GET_DEVICE_NAME:
         case PDCCMD_GET_SHORT_NAME:
         case PDCCMD_GET_LIBRARY_NAME:
         case PDCCMD_GET_OMNI_CLASS:

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_QUERY_CURRENT_BOOKLET:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_QUERY_CURRENT_COPIES:
#endif
         case PDCCMD_QUERY_CURRENT_DITHER_ID:
         case PDCCMD_QUERY_CURRENT_FORM:
#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_QUERY_CURRENT_JOGGING:
#endif
         case PDCCMD_QUERY_CURRENT_MEDIA:
#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_QUERY_CURRENT_NUP:
#endif
         case PDCCMD_QUERY_CURRENT_ORIENTATION:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_QUERY_CURRENT_OUTPUT_BIN:
#endif
         case PDCCMD_QUERY_CURRENT_PRINT_MODE:
         case PDCCMD_QUERY_CURRENT_RESOLUTION:
#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_QUERY_CURRENT_SCALING:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_QUERY_CURRENT_SHEET_COLLATE:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_QUERY_CURRENT_SIDE:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_QUERY_CURRENT_STITCHING:
#endif
         case PDCCMD_QUERY_CURRENT_TRAY:
#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_QUERY_CURRENT_TRIMMING:
#endif
         case PDCCMD_QUERY_CURRENT_GAMMA:

         case PDCCMD_HAS_CAPABILITY:
         case PDCCMD_HAS_RASTER_CAPABILITY:
         case PDCCMD_HAS_DEVICE_OPTION:

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_IS_BOOKLET_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_IS_COPIES_SUPPORTED:
#endif
         case PDCCMD_IS_DITHER_ID_SUPPORTED:
         case PDCCMD_IS_FORM_SUPPORTED:
#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_IS_JOGGING_SUPPORTED:
#endif
         case PDCCMD_IS_MEDIA_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_IS_NUP_SUPPORTED:
#endif
         case PDCCMD_IS_ORIENTATION_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_IS_OUTPUT_BIN_SUPPORTED:
#endif
         case PDCCMD_IS_PRINT_MODE_SUPPORTED:
         case PDCCMD_IS_RESOLUTION_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_IS_SCALING_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_IS_SHEET_COLLATE_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_IS_SIDE_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_IS_STITCHING_SUPPORTED:
#endif
         case PDCCMD_IS_TRAY_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_IS_TRIMMING_SUPPORTED:
#endif

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_ENUM_BOOKLETS:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_ENUM_COPIES:
#endif
         case PDCCMD_ENUM_DITHER_IDS:
         case PDCCMD_ENUM_FORMS:
#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_ENUM_JOGGINGS:
#endif
         case PDCCMD_ENUM_MEDIAS:
#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_ENUM_NUPS:
#endif
         case PDCCMD_ENUM_ORIENTATIONS:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_ENUM_OUTPUT_BINS:
#endif
         case PDCCMD_ENUM_PRINT_MODES:
         case PDCCMD_ENUM_RESOLUTIONS:
#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_ENUM_SCALINGS:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_ENUM_SHEET_COLLATES:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_ENUM_SIDES:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_ENUM_STITCHINGS:
#endif
         case PDCCMD_ENUM_TRAYS:
#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_ENUM_TRIMMINGS:
#endif
         case PDCCMD_ENUM_INSTANCE_PROPS:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_WARNING:
         case PDCCMD_SET_PRINTER_PROPERTIES:
         case PDCCMD_QUERY_CURRENT_PRINTER_PROPERTIES:
         case PDCCMD_LIST_PRINTER_PROPERTY_KEYS:
         case PDCCMD_LIST_DEVICE_PRINTER_PROPERTY_KEYS:
         case PDCCMD_LIST_PRINTER_PROPERTY_KEY_VALUES:
         case PDCCMD_GET_PRINTER_PROPERTY:
         case PDCCMD_GET_PRINTER_PROPERTY_TYPE:
         case PDCMD_XLATE_PRINTER_PROPERTY_KEY_VALUE:
         case PDCMD_QUERY_INPUT_FORMATS:
         case PDCMD_SET_INPUT_FORMAT:
#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_PUSH_CURRENT_BOOKLET:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_PUSH_CURRENT_COPIES:
#endif
         case PDCCMD_PUSH_CURRENT_DITHER_ID:
         case PDCCMD_PUSH_CURRENT_FORM:
#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_PUSH_CURRENT_JOGGING:
#endif
         case PDCCMD_PUSH_CURRENT_MEDIA:
#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_PUSH_CURRENT_NUP:
#endif
         case PDCCMD_PUSH_CURRENT_ORIENTATION:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_PUSH_CURRENT_OUTPUT_BIN:
#endif
         case PDCCMD_PUSH_CURRENT_PRINT_MODE:
         case PDCCMD_PUSH_CURRENT_RESOLUTION:
#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_PUSH_CURRENT_SCALING:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_PUSH_CURRENT_SHEET_COLLATE:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_PUSH_CURRENT_SIDE:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_PUSH_CURRENT_STITCHING:
#endif
         case PDCCMD_PUSH_CURRENT_TRAY:
#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_PUSH_CURRENT_TRIMMING:
#endif
         case PDCCMD_PUSH_CURRENT_GAMMA:
         default:
         {
            if (  !pCmd->setCommand (PDCCMD_NACK, pszErrorMissingCmd)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         /* unreachable with explicit default earlier
         default:
         {
            if (  !pCmd->setCommand (PDCCMD_UNSUPPORTED)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
         */
         }
         break;
      }

      case PDCCMD_SET_DEVICE_NAME:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            delete pDeviceRef;

            pDeviceRef = new DeviceReference (pCmd->getCommandString (false));

            if (pDeviceRef->loadDevice ())
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_IS_VALID_DEVICE_NAME:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            DeviceReference *pDeviceRefTmp;

            pDeviceRefTmp = new DeviceReference (pCmd->getCommandString (false));

            if (pDeviceRefTmp->loadDevice ())
            {
               eCommand = PDCCMD_ACK;
            }

            delete pDeviceRefTmp;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_MODE_IS_RENDERER:
      {
         fAdvanced = false;

         pCmd->getCommandBool (fAdvanced);

         if (  !pCmd->setCommand (PDCCMD_ACK)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_SET_JOB_PROPERTIES:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            if (cbJobProperties < strlen (pCmd->getCommandString (false)) + 1)
            {
               cbJobProperties  = strlen (pCmd->getCommandString (false)) + 1;
               pszJobProperties = (char *)realloc (pszJobProperties, cbJobProperties);
            }

            if (pszJobProperties)
            {
               strcpy (pszJobProperties, pCmd->getCommandString (false));
               eCommand              = PDCCMD_ACK;
               fNewJobPropertiesSent = true;

               DebugOutput::applyAllDebugOutput (pszJobProperties);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_NEW_DEVICE:
      {
         pDeviceRef->instantiateDevice (pszJobProperties, fAdvanced);

         Device *pDevice = pDeviceRef->getDevice ();

         if (pDevice)
         {
            eCommand              = PDCCMD_ACK;
            fNewJobPropertiesSent = false;

            if (StringResource::LANGUAGE_DEFAULT != iCurrentLanguage)
            {
               pDevice->setLanguage (iCurrentLanguage);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_GET_VERSION:
      case PDCCMD_GET_DRIVER_NAME:
      case PDCCMD_GET_DEVICE_NAME:
      case PDCCMD_GET_SHORT_NAME:
      case PDCCMD_GET_LIBRARY_NAME:
      case PDCCMD_GET_OMNI_CLASS:
#ifdef INCLUDE_JP_UPDF_BOOKLET
      case PDCCMD_QUERY_CURRENT_BOOKLET:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
      case PDCCMD_QUERY_CURRENT_COPIES:
#endif
      case PDCCMD_QUERY_CURRENT_DITHER_ID:
      case PDCCMD_QUERY_CURRENT_FORM:
#ifdef INCLUDE_JP_UPDF_JOGGING
      case PDCCMD_QUERY_CURRENT_JOGGING:
#endif
      case PDCCMD_QUERY_CURRENT_MEDIA:
#ifdef INCLUDE_JP_COMMON_NUP
      case PDCCMD_QUERY_CURRENT_NUP:
#endif
      case PDCCMD_QUERY_CURRENT_ORIENTATION:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      case PDCCMD_QUERY_CURRENT_OUTPUT_BIN:
#endif
      case PDCCMD_QUERY_CURRENT_PRINT_MODE:
      case PDCCMD_QUERY_CURRENT_RESOLUTION:
#ifdef INCLUDE_JP_COMMON_SCALING
      case PDCCMD_QUERY_CURRENT_SCALING:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      case PDCCMD_QUERY_CURRENT_SHEET_COLLATE:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
      case PDCCMD_QUERY_CURRENT_SIDE:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
      case PDCCMD_QUERY_CURRENT_STITCHING:
#endif
      case PDCCMD_QUERY_CURRENT_TRAY:
#ifdef INCLUDE_JP_COMMON_TRIMMING
      case PDCCMD_QUERY_CURRENT_TRIMMING:
#endif
      case PDCCMD_QUERY_CURRENT_GAMMA:
      case PDCCMD_GET_PDL_INFO:
      case PDCCMD_HAS_CAPABILITY:
      case PDCCMD_HAS_RASTER_CAPABILITY:
      case PDCCMD_HAS_DEVICE_OPTION:
#ifdef INCLUDE_JP_UPDF_BOOKLET
      case PDCCMD_ENUM_BOOKLETS:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
      case PDCCMD_ENUM_COPIES:
#endif
      case PDCCMD_ENUM_DITHER_IDS:
      case PDCCMD_ENUM_FORMS:
#ifdef INCLUDE_JP_UPDF_JOGGING
      case PDCCMD_ENUM_JOGGINGS:
#endif
      case PDCCMD_ENUM_MEDIAS:
#ifdef INCLUDE_JP_COMMON_NUP
      case PDCCMD_ENUM_NUPS:
#endif
      case PDCCMD_ENUM_ORIENTATIONS:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      case PDCCMD_ENUM_OUTPUT_BINS:
#endif
      case PDCCMD_ENUM_PRINT_MODES:
      case PDCCMD_ENUM_RESOLUTIONS:
#ifdef INCLUDE_JP_COMMON_SCALING
      case PDCCMD_ENUM_SCALINGS:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      case PDCCMD_ENUM_SHEET_COLLATES:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
      case PDCCMD_ENUM_SIDES:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
      case PDCCMD_ENUM_STITCHINGS:
#endif
      case PDCCMD_ENUM_TRAYS:
#ifdef INCLUDE_JP_COMMON_TRIMMING
      case PDCCMD_ENUM_TRIMMINGS:
#endif
      case PDCCMD_ENUM_INSTANCE_PROPS:
#ifdef INCLUDE_JP_UPDF_BOOKLET
      case PDCCMD_IS_BOOKLET_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
      case PDCCMD_IS_COPIES_SUPPORTED:
#endif
      case PDCCMD_IS_DITHER_ID_SUPPORTED:
      case PDCCMD_IS_FORM_SUPPORTED:
#ifdef INCLUDE_JP_UPDF_JOGGING
      case PDCCMD_IS_JOGGING_SUPPORTED:
#endif
      case PDCCMD_IS_MEDIA_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_NUP
      case PDCCMD_IS_NUP_SUPPORTED:
#endif
      case PDCCMD_IS_ORIENTATION_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      case PDCCMD_IS_OUTPUT_BIN_SUPPORTED:
#endif
      case PDCCMD_IS_PRINT_MODE_SUPPORTED:
      case PDCCMD_IS_RESOLUTION_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_SCALING
      case PDCCMD_IS_SCALING_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      case PDCCMD_IS_SHEET_COLLATE_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
      case PDCCMD_IS_SIDE_SUPPORTED:
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
      case PDCCMD_IS_STITCHING_SUPPORTED:
#endif
      case PDCCMD_IS_TRAY_SUPPORTED:
#ifdef INCLUDE_JP_COMMON_TRIMMING
      case PDCCMD_IS_TRIMMING_SUPPORTED:
#endif
      case PDCCMD_BEGIN_JOB:
      case PDCCMD_START_PAGE:
      case PDCCMD_END_PAGE:
      case PDCCMD_END_JOB:
      case PDCCMD_ABORT_JOB:
      case PDCCMD_RASTERIZE:
      case PDCCMD_GET_JOB_PROPERTIES:
      case PDCCMD_GET_JOB_PROPERTY:
      case PDCCMD_GET_JOB_PROPERTY_TYPE:
      case PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE:
      case PDCCMD_SET_OUTPUT_STREAM:
      case PDCCMD_SET_ERROR_STREAM:
      case PDCMD_IS_COLOR_PRINTER:
      case PDCMD_HAS_HARDWARE_COPY:
      {
         Device *pDevice = pDeviceRef->getDevice ();

         if (!pDevice)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK, pszErrorNoDevice)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         switch (pCmd->getCommandType ())
         {
         case PDCCMD_GET_VERSION:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getVersion ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_DRIVER_NAME:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getDriverName ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_DEVICE_NAME:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getDeviceName ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_SHORT_NAME:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getShortName ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_LIBRARY_NAME:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getLibraryName ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_OMNI_CLASS:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getOmniClass ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_QUERY_CURRENT_BOOKLET:
         {
            DeviceBooklet      *pBooklet     = 0;
            DeviceBooklet      *pBookletTemp = 0;
            std::ostringstream  oss;

            pBooklet = pDevice->getCurrentBooklet ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pBookletTemp = pBooklet->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pBookletTemp)
               {
                  pBooklet = pBookletTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named booklet" << std::endl;
               }
            }

            if (pBooklet)
            {
               std::string *pstringJP   = pBooklet->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted; // @TBD

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pBookletTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_QUERY_CURRENT_COPIES:
         {
            DeviceCopies       *pCopies     = 0;
            DeviceCopies       *pCopiesTemp = 0;
            std::ostringstream  oss;

            pCopies = pDevice->getCurrentCopies ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pCopiesTemp = pCopies->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pCopiesTemp)
               {
                  pCopies = pCopiesTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named copies" << std::endl;
               }
            }

            if (pCopies)
            {
               std::string *pstringJP   = pCopies->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pCopies->getMinimum ()
                      << " "
                      << pCopies->getMaximum ()
                      << " "
                      << (pCopies->needsSimulation () ? 1 : 0);

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pCopiesTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_DITHER_ID:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->getCurrentDitherID ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_QUERY_CURRENT_FORM:
         {
            DeviceForm         *pForm     = 0;
            DeviceForm         *pFormTemp = 0;
            HardCopyCap        *pHCC      = 0;
            std::ostringstream  oss;

            pForm = pDevice->getCurrentForm ();
            if (pForm)
               pHCC = pForm->getHardCopyCap ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pFormTemp = pForm->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pFormTemp)
               {
                  pForm = pFormTemp;
                  pHCC  = pForm->getHardCopyCap ();
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named form" << std::endl;
               }
            }

            if (pHCC)
            {
               std::string *pstringJP   = pForm->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pForm->getCapabilities ()
                      << " "
                      << pHCC->getLeftClip ()
                      << " "
                      << pHCC->getTopClip ()
                      << " "
                      << pHCC->getRightClip ()
                      << " "
                      << pHCC->getBottomClip ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pFormTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_QUERY_CURRENT_JOGGING:
         {
            DeviceJogging      *pJogging     = 0;
            DeviceJogging      *pJoggingTemp = 0;
            std::ostringstream  oss;

            pJogging = pDevice->getCurrentJogging ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pJoggingTemp = pJogging->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pJoggingTemp)
               {
                  pJogging = pJoggingTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named jogging" << std::endl;
               }
            }

            if (pJogging)
            {
               std::string *pstringJP   = pJogging->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted; // @TBD

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pJoggingTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_MEDIA:
         {
            DeviceMedia        *pMedia     = 0;
            DeviceMedia        *pMediaTemp = 0;
            std::ostringstream  oss;

            pMedia = pDevice->getCurrentMedia ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pMediaTemp = pMedia->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pMediaTemp)
               {
                  pMedia = pMediaTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named media" << std::endl;
               }
            }
            if (pMedia)
            {
               std::string *pstringJP   = pMedia->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pMedia->getColorAdjustRequired ()
                      << " "
                      << pMedia->getAbsorption ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pMediaTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_QUERY_CURRENT_NUP:
         {
            DeviceNUp          *pNUp     = 0;
            DeviceNUp          *pNUpTemp = 0;
            std::ostringstream  oss;

            pNUp = pDevice->getCurrentNUp ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pNUpTemp = pNUp->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pNUpTemp)
               {
                  pNUp = pNUpTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named nup" << std::endl;
               }
            }

            if (pNUp)
            {
               std::string *pstringJP   = pNUp->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << (pNUp->needsSimulation () ? 1 : 0);

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pNUpTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_ORIENTATION:
         {
            DeviceOrientation  *pOrientation     = 0;
            DeviceOrientation  *pOrientationTemp = 0;
            std::ostringstream  oss;

            pOrientation = pDevice->getCurrentOrientation ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pOrientationTemp = pOrientation->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pOrientationTemp)
               {
                  pOrientation = pOrientationTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named orientation" << std::endl;
               }
            }

            if (pOrientation)
            {
               std::string *pstringJP   = pOrientation->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << (pOrientation->needsSimulation () ? 1 : 0);

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pOrientationTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_QUERY_CURRENT_OUTPUT_BIN:
         {
            DeviceOutputBin    *pOutputBin     = 0;
            DeviceOutputBin    *pOutputBinTemp = 0;
            std::ostringstream  oss;

            pOutputBin = pDevice->getCurrentOutputBin ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pOutputBinTemp = pOutputBin->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pOutputBinTemp)
               {
                  pOutputBin = pOutputBinTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named outputbin" << std::endl;
               }
            }

            if (pOutputBin)
            {
               std::string *pstringJP   = pOutputBin->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted;

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pOutputBinTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_PRINT_MODE:
         {
            DevicePrintMode    *pPrintMode     = 0;
            DevicePrintMode    *pPrintModeTemp = 0;
            std::ostringstream  oss;

            pPrintMode = pDevice->getCurrentPrintMode ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pPrintModeTemp = pPrintMode->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pPrintModeTemp)
               {
                  pPrintMode = pPrintModeTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named print mode" << std::endl;
               }
            }

            if (pPrintMode)
            {
               std::string *pstringJP   = pPrintMode->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pPrintMode->getPhysicalCount ()
                      << " "
                      << pPrintMode->getLogicalCount ()
                      << " "
                      << pPrintMode->getNumPlanes ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pPrintModeTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_QUERY_CURRENT_RESOLUTION:
         {
            DeviceResolution   *pResolution     = 0;
            DeviceResolution   *pResolutionTemp = 0;
            std::ostringstream  oss;

            pResolution = pDevice->getCurrentResolution ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pResolutionTemp = pResolution->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pResolutionTemp)
               {
                  pResolution = pResolutionTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named resolution" << std::endl;
               }
            }
            if (pResolution)
            {
               std::string *pstringJP   = pResolution->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pResolution->getExternalXRes ()
                      << " "
                      << pResolution->getExternalYRes ()
                      << " "
                      << pResolution->getInternalXRes ()
                      << " "
                      << pResolution->getInternalYRes ()
                      << " "
                      << pResolution->getCapabilities ()
                      << " "
                      << pResolution->getDstBitsPerPel ()
                      << " "
                      << pResolution->getScanlineMultiple ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pResolutionTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_QUERY_CURRENT_SCALING:
         {
            DeviceScaling      *pScaling     = 0;
            DeviceScaling      *pScalingTemp = 0;
            std::ostringstream  oss;

            pScaling = pDevice->getCurrentScaling ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pScalingTemp = pScaling->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pScalingTemp)
               {
                  pScaling = pScalingTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named scaling" << std::endl;
               }
            }

            if (pScaling)
            {
               std::string *pstringJP   = pScaling->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pScaling->getMinimumPercentage ()
                      << " "
                      << pScaling->getMaximumPercentage ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pScalingTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_QUERY_CURRENT_SHEET_COLLATE:
         {
            DeviceSheetCollate *pSheetCollate     = 0;
            DeviceSheetCollate *pSheetCollateTemp = 0;
            std::ostringstream  oss;

            pSheetCollate = pDevice->getCurrentSheetCollate ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pSheetCollateTemp = pSheetCollate->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pSheetCollateTemp)
               {
                  pSheetCollate = pSheetCollateTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named sheetcollate" << std::endl;
               }
            }

            if (pSheetCollate)
            {
               std::string *pstringJP   = pSheetCollate->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted;

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pSheetCollateTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_QUERY_CURRENT_SIDE:
         {
            DeviceSide         *pSide     = 0;
            DeviceSide         *pSideTemp = 0;
            std::ostringstream  oss;

            pSide = pDevice->getCurrentSide ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pSideTemp = pSide->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pSideTemp)
               {
                  pSide = pSideTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named side" << std::endl;
               }
            }

            if (pSide)
            {
               std::string *pstringJP   = pSide->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << (pSide->needsSimulation () ? 1 : 0);

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pSideTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_QUERY_CURRENT_STITCHING:
         {
            DeviceStitching    *pStitching     = 0;
            DeviceStitching    *pStitchingTemp = 0;
            std::ostringstream  oss;

            pStitching = pDevice->getCurrentStitching ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pStitchingTemp = pStitching->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pStitchingTemp)
               {
                  pStitching = pStitchingTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named stitching" << std::endl;
               }
            }

            if (pStitching)
            {
               std::string *pstringJP   = pStitching->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted;

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pStitchingTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_TRAY:
         {
            DeviceTray         *pTray     = 0;
            DeviceTray         *pTrayTemp = 0;
            std::ostringstream  oss;

            pTray = pDevice->getCurrentTray ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pTrayTemp = pTray->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pTrayTemp)
               {
                  pTray = pTrayTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named tray" << std::endl;
               }
            }
            if (pTray)
            {
               std::string *pstringJP   = pTray->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted
                      << " "
                      << pTray->getType ();

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pTrayTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_QUERY_CURRENT_TRIMMING:
         {
            DeviceTrimming     *pTrimming     = 0;
            DeviceTrimming     *pTrimmingTemp = 0;
            std::ostringstream  oss;

            pTrimming = pDevice->getCurrentTrimming ();

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               PSZRO pszDequoted = Omni::dequoteString (pCmd->getCommandString (false));

               if (pszDequoted)
               {
                  pTrimmingTemp = pTrimming->create (0, pszDequoted);

                  free ((void *)pszDequoted);
               }

               if (pTrimmingTemp)
               {
                  pTrimming = pTrimmingTemp;
               }
               else
               {
                  DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": Error: Could not create " << SAFE_PRINT_PSZ(pszDequoted) << " named trimming" << std::endl;
               }
            }

            if (pTrimming)
            {
               std::string *pstringJP   = pTrimming->getJobProperties ();
               PSZRO        pszJPQuoted = 0;

               if (pstringJP)
               {
                  pszJPQuoted = Omni::quoteString (pstringJP->c_str ());
               }

               if (  pstringJP
                  && pszJPQuoted
                  )
               {
                  oss << pszJPQuoted;

                  eCommand = PDCCMD_ACK;
               }

               delete pstringJP;
               if (pszJPQuoted)
               {
                  free ((void *)pszJPQuoted);
               }
            }

            delete pTrimmingTemp;

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_QUERY_CURRENT_GAMMA:
         {
            DeviceGamma        *pGamma = 0;
            std::ostringstream  oss;

            pGamma = pDevice->getCurrentGamma ();

            if (pGamma)
               eCommand = PDCCMD_ACK;

            if (PDCCMD_ACK == eCommand)
               oss << pGamma->getCGamma ()
                   << " "
                   << pGamma->getMGamma ()
                   << " "
                   << pGamma->getYGamma ()
                   << " "
                   << pGamma->getKGamma ()
                   << " "
                   << pGamma->getCBias ()
                   << " "
                   << pGamma->getMBias ()
                   << " "
                   << pGamma->getYBias ()
                   << " "
                   << pGamma->getKBias ();

            if (  !pCmd->setCommand (eCommand, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_PDL_INFO:
         {
            std::ostringstream  oss;

            oss << pDevice->getPDLLevel ()
                << " "
                << pDevice->getPDLSubLevel ()
                << " "
                << pDevice->getPDLMajorRevisionLevel ()
                << " "
                << pDevice->getPDLMinorRevisionLevel ();

            if (  !pCmd->setCommand (PDCCMD_ACK, oss.str ())
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_HAS_CAPABILITY:
         {
            long int lValue = 0;

            if (  !pCmd->getCommandLong (lValue)
               || !pCmd->setCommand (PDCCMD_ACK, pDevice->hasCapability (lValue))
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_HAS_RASTER_CAPABILITY:
         {
            long int lValue = 0;

            if (  !pCmd->getCommandLong (lValue)
               || !pCmd->setCommand (PDCCMD_ACK, pDevice->hasRasterCapability (lValue))
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_HAS_DEVICE_OPTION:
         {
            if (  !pCmd->setCommand (PDCCMD_ACK, pDevice->hasDeviceOption (pCmd->getCommandString (false)))
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_ENUM_BOOKLETS:
         {
            bool           fInDeviceSpecific = false;
            DeviceBooklet *pBooklet          = pDevice->getCurrentBooklet ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pBooklet->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pBooklet   = pDevice->getCurrentBooklet ();
                  pEnum      = pBooklet->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_ENUM_COPIES:
            {
               bool           fInDeviceSpecific = false;
               DeviceCopies  *pCopies           = pDevice->getCurrentCopies ();
               Enumeration   *pEnum             = 0;
               JobProperties *pJP               = 0;
               PSZRO          pszJP             = 0;
               PSZ           *ppszResponse      = 0;
               PSZ            pszCurrent        = 0;
               int            cbResponse        = 0;

               if (pCmd->getCommandBool (fInDeviceSpecific))
               {
                  pEnum = pCopies->getEnumeration (fInDeviceSpecific);
               }

               while (  pEnum
                     && pEnum->hasMoreElements ()
                     )
               {
                  pJP = (JobProperties *)pEnum->nextElement ();
                  if (pJP)
                  {
                     pszJP = pJP->getJobProperties ();
                     if (pszJP)
                     {
                        cbResponse += strlen (pszJP) + 1;

                        free ((void *)pszJP);
                     }
                  }

                  delete pJP;
               }

               delete pEnum;

               if (0 < cbResponse)
               {
                  ppszResponse = (PSZ *)malloc (cbResponse + 1);

                  if (ppszResponse)
                  {
                     pszCurrent = (PSZ)ppszResponse;
                     pCopies    = pDevice->getCurrentCopies ();
                     pEnum      = pCopies->getEnumeration (fInDeviceSpecific);

                     while (pEnum->hasMoreElements ())
                     {
                        pJP = (JobProperties *)pEnum->nextElement ();
                        if (pJP)
                        {
                           pszJP = pJP->getJobProperties ();
                           if (pszJP)
                           {
                              strcpy (pszCurrent, pszJP);
                              pszCurrent += strlen (pszJP) + 1;

                              free ((void *)pszJP);
                           }
                        }

                        delete pJP;
                     }

                     *pszCurrent = '\0';

                     delete pEnum;

                     eCommand = PDCCMD_ACK;
                  }
               }

               if (  !pCmd->setCommand (eCommand, ppszResponse)
                  || !pCmd->sendCommand (fdS2C)
                  )
               {
                  if (ppszResponse)
                  {
                     free ((void *)ppszResponse);
                  }

                  goto BUGOUT;
               }

               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }
               break;
            }
#endif

         case PDCCMD_ENUM_DITHER_IDS:
         {
            bool           fInDeviceSpecific = false;
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = DeviceDither::getAllEnumeration ();
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pEnum      = DeviceDither::getAllEnumeration ();

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

         case PDCCMD_ENUM_FORMS:
         {
            bool           fInDeviceSpecific = false;
            DeviceForm    *pForm             = pDevice->getCurrentForm ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pForm->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            while (pEnum->hasMoreElements ())
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pForm      = pDevice->getCurrentForm ();
                  pEnum      = pForm->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_ENUM_JOGGINGS:
         {
            bool           fInDeviceSpecific = false;
            DeviceJogging *pJogging          = pDevice->getCurrentJogging ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pJogging->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pJogging   = pDevice->getCurrentJogging ();
                  pEnum      = pJogging->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

         case PDCCMD_ENUM_MEDIAS:
         {
            bool           fInDeviceSpecific = false;
            DeviceMedia   *pMedia            = pDevice->getCurrentMedia ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pMedia->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pMedia     = pDevice->getCurrentMedia ();
                  pEnum      = pMedia->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_ENUM_NUPS:
         {
            bool           fInDeviceSpecific = false;
            DeviceNUp     *pNUp              = pDevice->getCurrentNUp ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pNUp->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pNUp       = pDevice->getCurrentNUp ();
                  pEnum      = pNUp->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

         case PDCCMD_ENUM_ORIENTATIONS:
         {
            bool               fInDeviceSpecific = false;
            DeviceOrientation *pOrientation      = pDevice->getCurrentOrientation ();
            Enumeration       *pEnum             = 0;
            JobProperties     *pJP               = 0;
            PSZRO              pszJP             = 0;
            PSZ               *ppszResponse      = 0;
            PSZ                pszCurrent        = 0;
            int                cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pOrientation->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent   = (PSZ)ppszResponse;
                  pOrientation = pDevice->getCurrentOrientation ();
                  pEnum        = pOrientation->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_ENUM_OUTPUT_BINS:
         {
            bool             fInDeviceSpecific = false;
            DeviceOutputBin *pDeviceOutputBin  = pDevice->getCurrentOutputBin ();
            Enumeration     *pEnum             = 0;
            JobProperties   *pJP               = 0;
            PSZRO            pszJP             = 0;
            PSZ             *ppszResponse      = 0;
            PSZ              pszCurrent        = 0;
            int              cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pDeviceOutputBin->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent       = (PSZ)ppszResponse;
                  pDeviceOutputBin = pDevice->getCurrentOutputBin ();
                  pEnum            = pDeviceOutputBin->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

         case PDCCMD_ENUM_PRINT_MODES:
         {
            bool             fInDeviceSpecific = false;
            DevicePrintMode *pPrintMode        = pDevice->getCurrentPrintMode ();
            Enumeration     *pEnum             = 0;
            JobProperties   *pJP               = 0;
            PSZRO            pszJP             = 0;
            PSZ             *ppszResponse      = 0;
            PSZ              pszCurrent        = 0;
            int              cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pPrintMode->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pPrintMode = pDevice->getCurrentPrintMode ();
                  pEnum      = pPrintMode->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

         case PDCCMD_ENUM_RESOLUTIONS:
         {
            bool              fInDeviceSpecific = false;
            DeviceResolution *pResolution       = pDevice->getCurrentResolution ();
            Enumeration      *pEnum             = 0;
            JobProperties    *pJP               = 0;
            PSZRO             pszJP             = 0;
            PSZ              *ppszResponse      = 0;
            PSZ               pszCurrent        = 0;
            int               cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pResolution->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent  = (PSZ)ppszResponse;
                  pResolution = pDevice->getCurrentResolution ();
                  pEnum       = pResolution->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_ENUM_SCALINGS:
         {
            bool           fInDeviceSpecific = false;
            DeviceScaling *pScaling          = pDevice->getCurrentScaling ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pScaling->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pScaling   = pDevice->getCurrentScaling ();
                  pEnum      = pScaling->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_ENUM_SHEET_COLLATES:
         {
            bool                fInDeviceSpecific = false;
            DeviceSheetCollate *pSheetCollate     = pDevice->getCurrentSheetCollate ();
            Enumeration        *pEnum             = 0;
            JobProperties      *pJP               = 0;
            PSZRO               pszJP             = 0;
            PSZ                *ppszResponse      = 0;
            PSZ                 pszCurrent        = 0;
            int                 cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pSheetCollate->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent    = (PSZ)ppszResponse;
                  pSheetCollate = pDevice->getCurrentSheetCollate ();
                  pEnum         = pSheetCollate->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_ENUM_SIDES:
         {
            bool           fInDeviceSpecific = false;
            DeviceSide    *pSide             = pDevice->getCurrentSide ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pSide->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pSide      = pDevice->getCurrentSide ();
                  pEnum      = pSide->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_ENUM_STITCHINGS:
         {
            bool             fInDeviceSpecific = false;
            DeviceStitching *pStitching        = pDevice->getCurrentStitching ();
            Enumeration     *pEnum             = 0;
            JobProperties   *pJP               = 0;
            PSZRO            pszJP             = 0;
            PSZ             *ppszResponse      = 0;
            PSZ              pszCurrent        = 0;
            int              cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pStitching->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pStitching = pDevice->getCurrentStitching ();
                  pEnum      = pStitching->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

         case PDCCMD_ENUM_TRAYS:
         {
            bool           fInDeviceSpecific = false;
            DeviceTray    *pTray             = pDevice->getCurrentTray ();
            Enumeration   *pEnum             = 0;
            JobProperties *pJP               = 0;
            PSZRO          pszJP             = 0;
            PSZ           *ppszResponse      = 0;
            PSZ            pszCurrent        = 0;
            int            cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pTray->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pTray      = pDevice->getCurrentTray ();
                  pEnum      = pTray->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_ENUM_TRIMMINGS:
         {
            bool            fInDeviceSpecific = false;
            DeviceTrimming *pTrimming         = pDevice->getCurrentTrimming ();
            Enumeration    *pEnum             = 0;
            JobProperties  *pJP               = 0;
            PSZRO           pszJP             = 0;
            PSZ            *ppszResponse      = 0;
            PSZ             pszCurrent        = 0;
            int             cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnum = pTrimming->getEnumeration (fInDeviceSpecific);
            }

            while (  pEnum
                  && pEnum->hasMoreElements ()
                  )
            {
               pJP = (JobProperties *)pEnum->nextElement ();
               if (pJP)
               {
                  pszJP = pJP->getJobProperties ();
                  if (pszJP)
                  {
                     cbResponse += strlen (pszJP) + 1;

                     free ((void *)pszJP);
                  }
               }

               delete pJP;
            }

            delete pEnum;

            if (0 < cbResponse)
            {
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent = (PSZ)ppszResponse;
                  pTrimming  = pDevice->getCurrentTrimming ();
                  pEnum      = pTrimming->getEnumeration (fInDeviceSpecific);

                  while (pEnum->hasMoreElements ())
                  {
                     pJP = (JobProperties *)pEnum->nextElement ();
                     if (pJP)
                     {
                        pszJP = pJP->getJobProperties ();
                        if (pszJP)
                        {
                           strcpy (pszCurrent, pszJP);
                           pszCurrent += strlen (pszJP) + 1;

                           free ((void *)pszJP);
                        }
                     }

                     delete pJP;
                  }

                  *pszCurrent = '\0';

                  delete pEnum;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, ppszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }
#endif

         case PDCCMD_ENUM_INSTANCE_PROPS:
         {
            bool            fInDeviceSpecific = false;
            DeviceInstance *pInstance         = pDevice->getInstance ();
            Enumeration    *pEnumGroups       = 0;
            JobProperties  *pJP               = 0;
            PSZRO           pszJP             = 0;
            PSZ            *ppszResponse      = 0;
            PSZ             pszCurrent        = 0;
            int             cbResponse        = 0;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnumGroups = pInstance->getGroupEnumeration (fInDeviceSpecific);
            }

            while (  pEnumGroups
                  && pEnumGroups->hasMoreElements ()
                  )
            {
               Enumeration *pEnum = (Enumeration *)pEnumGroups->nextElement ();

               while (pEnum->hasMoreElements ())
               {
                  pJP = (JobProperties *)pEnum->nextElement ();
                  if (pJP)
                  {
                     pszJP = pJP->getJobProperties ();
                     if (pszJP)
                     {
                        cbResponse += strlen (pszJP) + 1;

                        free ((void *)pszJP);
                     }
                  }

                  delete pJP;
               }

               delete pEnum;

               cbResponse++;
            }

            delete pEnumGroups;

            if (0 < cbResponse)
            {
               cbResponse++;
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent  = (PSZ)ppszResponse;
                  pEnumGroups = pInstance->getGroupEnumeration (fInDeviceSpecific);

                  while (pEnumGroups->hasMoreElements ())
                  {
                     Enumeration *pEnum = (Enumeration *)pEnumGroups->nextElement ();

                     while (pEnum->hasMoreElements ())
                     {
                        pJP = (JobProperties *)pEnum->nextElement ();
                        if (pJP)
                        {
                           pszJP = pJP->getJobProperties ();
                           if (pszJP)
                           {
                              strcpy (pszCurrent, pszJP);
                              pszCurrent += strlen (pszJP) + 1;

                              free ((void *)pszJP);
                           }
                        }

                        delete pJP;
                     }

                     delete pEnum;

                     *pszCurrent++ = '\0';
                  }

                  *pszCurrent++ = '\0';

                  delete pEnumGroups;

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, (PBYTE)ppszResponse, cbResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               if (ppszResponse)
               {
                  free ((void *)ppszResponse);
               }

               goto BUGOUT;
            }

            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_BOOKLET
         case PDCCMD_IS_BOOKLET_SUPPORTED:
         {
            DeviceBooklet *pBooklet = pDevice->getCurrentBooklet ();

            if (  pBooklet
               && pBooklet->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
         case PDCCMD_IS_COPIES_SUPPORTED:
         {
            DeviceCopies *pCopies = pDevice->getCurrentCopies ();

            if (  pCopies
               && pCopies->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_IS_DITHER_ID_SUPPORTED:
         {
            if (DeviceDither::ditherNameValid (pCmd->getCommandString (false)))
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_IS_FORM_SUPPORTED:
         {
            DeviceForm *pForm = pDevice->getCurrentForm ();

            if (  pForm
               && pForm->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_UPDF_JOGGING
         case PDCCMD_IS_JOGGING_SUPPORTED:
         {
            DeviceJogging *pJogging = pDevice->getCurrentJogging ();

            if (  pJogging
               && pJogging->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_IS_MEDIA_SUPPORTED:
         {
            DeviceMedia *pMedia = pDevice->getCurrentMedia ();

            if (  pMedia
               && pMedia->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_NUP
         case PDCCMD_IS_NUP_SUPPORTED:
         {
            DeviceNUp *pNUp = pDevice->getCurrentNUp ();

            if (  pNUp
               && pNUp->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_IS_ORIENTATION_SUPPORTED:
         {
            DeviceOrientation *pOrientation = pDevice->getCurrentOrientation ();

            if (  pOrientation
               && pOrientation->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
         case PDCCMD_IS_OUTPUT_BIN_SUPPORTED:
         {
            DeviceOutputBin *pOutputBin = pDevice->getCurrentOutputBin ();

            if (  pOutputBin
               && pOutputBin->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_IS_PRINT_MODE_SUPPORTED:
         {
            DevicePrintMode *pPrintMode = pDevice->getCurrentPrintMode ();

            if (  pPrintMode
               && pPrintMode->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_IS_RESOLUTION_SUPPORTED:
         {
            DeviceResolution *pResolution = pDevice->getCurrentResolution ();

            if (  pResolution
               && pResolution->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_SCALING
         case PDCCMD_IS_SCALING_SUPPORTED:
         {
            DeviceScaling *pScaling = pDevice->getCurrentScaling ();

            if (  pScaling
               && pScaling->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
         case PDCCMD_IS_SHEET_COLLATE_SUPPORTED:
         {
            DeviceSheetCollate *pSheetCollate = pDevice->getCurrentSheetCollate ();

            if (  pSheetCollate
               && pSheetCollate->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
         case PDCCMD_IS_SIDE_SUPPORTED:
         {
            DeviceSide *pSide = pDevice->getCurrentSide ();

            if (  pSide
               && pSide->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
         case PDCCMD_IS_STITCHING_SUPPORTED:
         {
            DeviceStitching *pStitching = pDevice->getCurrentStitching ();

            if (  pStitching
               && pStitching->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_IS_TRAY_SUPPORTED:
         {
            DeviceTray *pTray = pDevice->getCurrentTray ();

            if (  pTray
               && pTray->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

#ifdef INCLUDE_JP_COMMON_TRIMMING
         case PDCCMD_IS_TRIMMING_SUPPORTED:
         {
            DeviceTrimming *pTrimming = pDevice->getCurrentTrimming ();

            if (  pTrimming
               && pTrimming->isSupported (pCmd->getCommandString (false))
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
#endif

         case PDCCMD_BEGIN_JOB:
         {
            if (pDevice->beginJob ())
            {
               iPageNumber = 1;
               eCommand    = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_START_PAGE:
         {
            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_END_PAGE:
         {
            if (1 == iPageNumber)
            {
               eCommand = PDCCMD_ACK;
            }
            else
            {
               if (fNewJobPropertiesSent)
               {
                  if (pDevice->newFrame (pszJobProperties))
                     eCommand = PDCCMD_ACK;
               }
               else
               {
                  if (pDevice->newFrame ())
                     eCommand = PDCCMD_ACK;
               }
            }

            if (PDCCMD_ACK == eCommand)
            {
               iPageNumber++;
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_END_JOB:
         {
            if (pDevice->endJob ())
               eCommand = PDCCMD_ACK;

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_ABORT_JOB:
         {
            if (pDevice->abortJob ())
               eCommand = PDCCMD_ACK;

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_RASTERIZE:
         {
            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               int          iType;
               BITBLT_TYPE  eType;
               RECTL        rectlPageLocation;

               sscanf (pCmd->getCommandString (false),
                       "%d %d %d %d %d",
                       &iType,
                       &rectlPageLocation.xLeft,
                       &rectlPageLocation.yBottom,
                       &rectlPageLocation.xRight,
                       &rectlPageLocation.yTop);

               eType = (BITBLT_TYPE)iType;

               if (pDevice->rasterize (pbBuffer2,
                                       (PBITMAPINFO2)pbBuffer1,
                                       &rectlPageLocation,
                                       eType))
               {
                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_GET_JOB_PROPERTIES:
         {
            std::string *pStr   = 0;

            pStr = pDevice->getJobProperties ();

            if (pStr)
            {
               eCommand = PDCCMD_ACK;
            }

            if (  !pCmd->setCommand (eCommand, pStr)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               delete pStr;

               goto BUGOUT;
            }

            delete pStr;
            break;
         }

         case PDCCMD_GET_JOB_PROPERTY:
         {
            std::string *pStr = 0;

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               pStr = pDevice->getJobProperty (pCmd->getCommandString (false));

               if (pStr)
               {
                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, pStr)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               delete pStr;

               goto BUGOUT;
            }

            delete pStr;
            break;
         }

         case PDCCMD_GET_JOB_PROPERTY_TYPE:
         {
            std::string *pStr   = 0;

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               pStr = pDevice->getJobPropertyType (pCmd->getCommandString (false));

               if (pStr)
               {
                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand, pStr)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               delete pStr;

               goto BUGOUT;
            }

            delete pStr;
            break;
         }

         case PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE:
         {
            std::string *pStr   = 0;

            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               char *pszKey    = pCmd->getCommandString (false);
               char *pszValue  = 0;
               char *pszEquals = strchr (pszKey, '=');

               if (pszEquals)
               {
                  *pszEquals = '\0';
                  pszValue = pszEquals + 1;

                  pStr = pDevice->translateKeyValue (pszKey,
                                                     pszValue);

                  if (pStr)
                  {
                     eCommand = PDCCMD_ACK;
                  }
               }
            }

            if (  !pCmd->setCommand (eCommand, pStr)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               delete pStr;

               goto BUGOUT;
            }

            delete pStr;
            break;
         }

         case PDCCMD_SET_OUTPUT_STREAM:
         {
            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               int fdHandle = STDOUT_FILENO;

               sscanf (pCmd->getCommandString (false), "%d", &fdHandle);

               if (pfpOut)
               {
                  fclose (pfpOut);
                  pfpOut = 0;
               }

               pfpOut = fdopen (fdHandle, "wb");

               if (pfpOut)
               {
                  Omni::setOutputStream (pDevice, pfpOut);

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCCMD_SET_ERROR_STREAM:
         {
            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               int fdHandle = STDERR_FILENO;

               sscanf (pCmd->getCommandString (false), "%d", &fdHandle);

               if (pfpErr)
               {
                  fclose (pfpErr);
                  pfpErr = 0;
               }

               pfpErr = fdopen (fdHandle, "wb");

               if (pfpErr)
               {
                  Omni::setErrorStream (pDevice, pfpErr);

                  eCommand = PDCCMD_ACK;
               }
            }

            if (  !pCmd->setCommand (eCommand)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCMD_IS_COLOR_PRINTER:
         {
            char *pszResponse = 0;

            eCommand = PDCCMD_ACK;

            if (pDevice->hasCapability (Capability::COLOR))
            {
               pszResponse = "1";
            }
            else
            {
               pszResponse = "0";
            }

            if (  !pCmd->setCommand (eCommand, pszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         case PDCMD_HAS_HARDWARE_COPY:
         {
            char *pszResponse = 0;

            eCommand = PDCCMD_ACK;

            if (pDevice->hasCapability (Capability::HARDWARE_COPIES))
            {
               pszResponse = "1";
            }
            else
            {
               pszResponse = "0";
            }

            if (  !pCmd->setCommand (eCommand, pszResponse)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }
         }
         break;
      }

      case PDCCMD_ATTACH_BUFFER1:
      {
         if (pbBuffer1)
         {
            shmdt (pbBuffer1);
         }

         int iValue = 0;

         if (pCmd->getCommandInt (iValue))
         {
            pbBuffer1 = (byte *)shmat (iValue, 0, 0);
         }
         else
         {
            pbBuffer1 = 0;
         }

                   if (pbBuffer1 != 0)
         {
            eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ATTACH_BUFFER2:
      {
         if (pbBuffer2)
         {
            shmdt (pbBuffer2);
         }

         int iValue = 0;

         if (pCmd->getCommandInt (iValue))
         {
            pbBuffer2 = (byte *)shmat (iValue, 0, 0);
         }
         else
         {
            pbBuffer2 = 0;
         }

                   if (pbBuffer2 != 0)
          {
             eCommand = PDCCMD_ACK;
          }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_DETACH_BUFFER1:
      {
         if (pbBuffer1)
         {
            int rc = shmdt (pbBuffer1);

            if (0 == rc)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_DETACH_BUFFER2:
      {
         if (pbBuffer2)
         {
            int rc = shmdt (pbBuffer2);

            if (0 == rc)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ENUM_SHORT_DEVICES:
      {
         Enumeration *pEnum                           = Omni::listDevices ();
         int          cbSize                          = 0;
         char        *pszMatch                        = 0;
         bool         fMustMatchPDLLevel              = false;
         int          iPDLLevel                       = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLSubLevel           = false;
         int          iPDLSubLevel                    = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLMajorRevisionLevel = false;
         int          iPDLMajorRevisionLevel          = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLMinorRevisionLevel = false;
         int          iPDLMinorRevisionLevel          = PDL::PDL_DONTCARE;

         if (!pEnum)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         pszMatch = pCmd->getCommandString (false);
         if (  pszMatch
            && *pszMatch
            )
         {
            if (1 == sscanf (pszMatch, "%d", &iPDLLevel))
            {
               fMustMatchPDLLevel = true;
               pszMatch = strchr (pszMatch, ' ');

               if (pszMatch)
               {
                  if (1 == sscanf (pszMatch + 1, "%d", &iPDLSubLevel))
                  {
                     fMustMatchPDLSubLevel = true;
                     pszMatch = strchr (pszMatch + 1, ' ');

                     if (pszMatch)
                     {
                        if (1 == sscanf (pszMatch + 1,
                                         "%d",
                                         &iPDLMajorRevisionLevel))
                        {
                           fMustMatchPDLMajorRevisionLevel = true;
                           pszMatch = strchr (pszMatch + 1, ' ');

                           if (pszMatch)
                           {
                              if (1 == sscanf (pszMatch + 1,
                                               "%d",
                                               &iPDLMinorRevisionLevel))
                              {
                                 fMustMatchPDLMinorRevisionLevel = true;
                                 pszMatch = strchr (pszMatch + 1, ' ');
                              }
                           }
                        }
                     }
                  }
               }
            }
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniServer ())
         {
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLLevel              = " << fMustMatchPDLLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLLevel                       = " << iPDLLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLSubLevel           = " << fMustMatchPDLSubLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLSubLevel                    = " << iPDLSubLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLMajorRevisionLevel = " << fMustMatchPDLMajorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLMajorRevisionLevel          = " << iPDLMajorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLMinorRevisionLevel = " << fMustMatchPDLMinorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLMinorRevisionLevel          = " << iPDLMinorRevisionLevel << std::endl;
         }
#endif

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO            pszLibName   = pOD->getLibraryName ();
               Device           *pDevice      = 0;
               PFNNEWDEVICE      pfnNewDevice = 0;
               GModule          *hmodDevice   = 0;

               hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

               if (hmodDevice)
               {
                  g_module_symbol (hmodDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

                  if (pfnNewDevice)
                  {
                     pDevice = pfnNewDevice (true);

                     if (  (  (  fMustMatchPDLLevel
                              && iPDLLevel == pDevice->getPDLLevel ()
                              )
                           || !fMustMatchPDLLevel
                           )
                        && (  (  fMustMatchPDLSubLevel
                              && iPDLSubLevel == pDevice->getPDLSubLevel ()
                              )
                           || !fMustMatchPDLSubLevel
                           )
                        && (  (  fMustMatchPDLMajorRevisionLevel
                              && iPDLMajorRevisionLevel == pDevice->getPDLMajorRevisionLevel ()
                              )
                           || !fMustMatchPDLMajorRevisionLevel
                           )
                        && (  (  fMustMatchPDLMinorRevisionLevel
                              && iPDLMinorRevisionLevel == pDevice->getPDLMinorRevisionLevel ()
                              )
                           || !fMustMatchPDLMinorRevisionLevel
                           )
                        )
                     {
                        cbSize += strlen (pDevice->getShortName ())
                                + 1;                                 // end-of-string
                     }

                     delete pDevice;
                  }

                  g_module_close (hmodDevice);
               }

               delete pOD;
            }
         }

         if (0 < cbSize)
            // add end-of-array
            cbSize++;

         delete pEnum;

         if (0 == cbSize)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         char *pszResponse = 0;
         char *pszAppend   = 0;

         pszResponse = (char *)calloc (1, cbSize);

         if (!pszResponse)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         pszAppend = pszResponse;
         pEnum     = Omni::listDevices ();

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO            pszLibName   = pOD->getLibraryName ();
               Device           *pDevice      = 0;
               PFNNEWDEVICE      pfnNewDevice = 0;
               GModule          *hmodDevice   = 0;

               hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

               if (hmodDevice)
               {
                  g_module_symbol (hmodDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

                  if (pfnNewDevice)
                  {
                     pDevice = pfnNewDevice (true);

                     if (  (  (  fMustMatchPDLLevel
                              && iPDLLevel == pDevice->getPDLLevel ()
                              )
                           || !fMustMatchPDLLevel
                           )
                        && (  (  fMustMatchPDLSubLevel
                              && iPDLSubLevel == pDevice->getPDLSubLevel ()
                              )
                           || !fMustMatchPDLSubLevel
                           )
                        && (  (  fMustMatchPDLMajorRevisionLevel
                              && iPDLMajorRevisionLevel == pDevice->getPDLMajorRevisionLevel ()
                              )
                           || !fMustMatchPDLMajorRevisionLevel
                           )
                        && (  (  fMustMatchPDLMinorRevisionLevel
                              && iPDLMinorRevisionLevel == pDevice->getPDLMinorRevisionLevel ()
                              )
                           || !fMustMatchPDLMinorRevisionLevel
                           )
                        )
                     {
                        strcat (pszAppend, pDevice->getShortName ());
                        pszAppend += strlen (pszAppend) + 1;
                     }

                     delete pDevice;
                  }

                  g_module_close (hmodDevice);
               }

               delete pOD;
            }
         }

         *pszAppend++ = '\0'; // end-of-array

         delete pEnum;

         if (  !pCmd->setCommand (PDCCMD_ACK, (char **)pszResponse)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            free ((void *)pszResponse);

            goto BUGOUT;
         }

         free ((void *)pszResponse);

         break;
      }

      case PDCCMD_ENUM_LONG_DEVICES:
      {
         Enumeration *pEnum                           = Omni::listDevices ();
         int          cbSize                          = 0;
         char        *pszMatch                        = 0;
         bool         fMustMatchPDLLevel              = false;
         int          iPDLLevel                       = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLSubLevel           = false;
         int          iPDLSubLevel                    = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLMajorRevisionLevel = false;
         int          iPDLMajorRevisionLevel          = PDL::PDL_DONTCARE;
         bool         fMustMatchPDLMinorRevisionLevel = false;
         int          iPDLMinorRevisionLevel          = PDL::PDL_DONTCARE;

         if (!pEnum)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         pszMatch = pCmd->getCommandString (false);
         if (  pszMatch
            && *pszMatch
            )
         {
            if (1 == sscanf (pszMatch, "%d", &iPDLLevel))
            {
               fMustMatchPDLLevel = true;
               pszMatch = strchr (pszMatch, ' ');

               if (pszMatch)
               {
                  if (1 == sscanf (pszMatch + 1, "%d", &iPDLSubLevel))
                  {
                     fMustMatchPDLSubLevel = true;
                     pszMatch = strchr (pszMatch + 1, ' ');

                     if (pszMatch)
                     {
                        if (1 == sscanf (pszMatch + 1,
                                         "%d",
                                         &iPDLMajorRevisionLevel))
                        {
                           fMustMatchPDLMajorRevisionLevel = true;
                           pszMatch = strchr (pszMatch + 1, ' ');

                           if (pszMatch)
                           {
                              if (1 == sscanf (pszMatch + 1,
                                               "%d",
                                               &iPDLMinorRevisionLevel))
                              {
                                 fMustMatchPDLMinorRevisionLevel = true;
                                 pszMatch = strchr (pszMatch + 1, ' ');
                              }
                           }
                        }
                     }
                  }
               }
            }
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniServer ())
         {
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLLevel              = " << fMustMatchPDLLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLLevel                       = " << iPDLLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLSubLevel           = " << fMustMatchPDLSubLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLSubLevel                    = " << iPDLSubLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLMajorRevisionLevel = " << fMustMatchPDLMajorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLMajorRevisionLevel          = " << iPDLMajorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": fMustMatchPDLMinorRevisionLevel = " << fMustMatchPDLMinorRevisionLevel << std::endl;
            DebugOutput::getErrorStream () << "OmniServer::" << __FUNCTION__ << ": iPDLMinorRevisionLevel          = " << iPDLMinorRevisionLevel << std::endl;
         }
#endif

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO            pszLibName   = pOD->getLibraryName ();
               Device           *pDevice      = 0;
               PFNNEWDEVICE      pfnNewDevice = 0;
               GModule          *hmodDevice   = 0;

               hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

               if (hmodDevice)
               {
                  g_module_symbol (hmodDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

                  if (pfnNewDevice)
                  {
                     pDevice = pfnNewDevice (true);

                     if (  (  (  fMustMatchPDLLevel
                              && iPDLLevel == pDevice->getPDLLevel ()
                              )
                           || !fMustMatchPDLLevel
                           )
                        && (  (  fMustMatchPDLSubLevel
                              && iPDLSubLevel == pDevice->getPDLSubLevel ()
                              )
                           || !fMustMatchPDLSubLevel
                           )
                        && (  (  fMustMatchPDLMajorRevisionLevel
                              && iPDLMajorRevisionLevel == pDevice->getPDLMajorRevisionLevel ()
                              )
                           || !fMustMatchPDLMajorRevisionLevel
                           )
                        && (  (  fMustMatchPDLMinorRevisionLevel
                              && iPDLMinorRevisionLevel == pDevice->getPDLMinorRevisionLevel ()
                              )
                           || !fMustMatchPDLMinorRevisionLevel
                           )
                        )
                     {
                        cbSize += strlen (pDevice->getDriverName ())
                                + 1                                  // '.'
                                + strlen (pDevice->getDeviceName ())
                                + 1;                                 // end-of-string
                     }

                     delete pDevice;
                  }

                  g_module_close (hmodDevice);
               }

               delete pOD;
            }
         }

         if (0 < cbSize)
            // add end-of-array
            cbSize++;

         delete pEnum;

         if (0 == cbSize)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         char *pszResponse = 0;
         char *pszAppend   = 0;

         pszResponse = (char *)calloc (1, cbSize);

         if (!pszResponse)
         {
            if (  !pCmd->setCommand (PDCCMD_NACK)
               || !pCmd->sendCommand (fdS2C)
               )
            {
               goto BUGOUT;
            }
            break;
         }

         pszAppend = pszResponse;
         pEnum     = Omni::listDevices ();

         while (pEnum->hasMoreElements ())
         {
            OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

            if (pOD)
            {
               PSZCRO        pszLibName   = pOD->getLibraryName ();
               Device       *pDevice      = 0;
               PFNNEWDEVICE  pfnNewDevice = 0;
               GModule      *hmodDevice   = 0;

               hmodDevice = g_module_open (pszLibName, (GModuleFlags)0);

               if (hmodDevice)
               {
                  g_module_symbol (hmodDevice, "newDeviceW_Advanced", (gpointer *)&pfnNewDevice);

                  if (pfnNewDevice)
                  {
                     pDevice = pfnNewDevice (true);

                     if (  (  (  fMustMatchPDLLevel
                              && iPDLLevel == pDevice->getPDLLevel ()
                              )
                           || !fMustMatchPDLLevel
                           )
                        && (  (  fMustMatchPDLSubLevel
                              && iPDLSubLevel == pDevice->getPDLSubLevel ()
                              )
                           || !fMustMatchPDLSubLevel
                           )
                        && (  (  fMustMatchPDLMajorRevisionLevel
                              && iPDLMajorRevisionLevel == pDevice->getPDLMajorRevisionLevel ()
                              )
                           || !fMustMatchPDLMajorRevisionLevel
                           )
                        && (  (  fMustMatchPDLMinorRevisionLevel
                              && iPDLMinorRevisionLevel == pDevice->getPDLMinorRevisionLevel ()
                              )
                           || !fMustMatchPDLMinorRevisionLevel
                           )
                        )
                     {
                        sprintf (pszAppend,
                                 "%s.%s",
                                 pDevice->getDriverName (),
                                 pDevice->getDeviceName ());
                        pszAppend += strlen (pszAppend) + 1;
                     }

                     delete pDevice;
                  }

                  g_module_close (hmodDevice);
               }

               delete pOD;
            }
         }

         *pszAppend++ = '\0'; // end-of-array

         delete pEnum;

         if (  !pCmd->setCommand (PDCCMD_ACK, (char **)pszResponse)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            free ((void *)pszResponse);

            goto BUGOUT;
         }

         free ((void *)pszResponse);

         break;
      }

      default:
      {
         if (  !pCmd->setCommand (PDCCMD_UNSUPPORTED, "Unknown command")
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
      }
   }

BUGOUT:

   // Clean up
   close (fdS2C);
   close (fdC2S);

   if (pszJobProperties)
   {
      free (pszJobProperties);
   }

   if (pbBuffer1)
   {
      shmdt (pbBuffer1);
   }
   if (pbBuffer2)
   {
      shmdt (pbBuffer2);
   }

   delete pDeviceRef;
   delete pCmd;

   if (pfpOut)
   {
      fclose (pfpOut);
      pfpOut = 0;
   }

   if (pfpErr)
   {
      fclose (pfpErr);
      pfpErr = 0;
   }

   Omni::terminate ();

   return 0;
}
