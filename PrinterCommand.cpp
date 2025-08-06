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
#include "PrinterCommand.hpp"
#include "DebugOutput.hpp"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

PrinterCommand::
PrinterCommand (PSZCRO pszProgram)
{
   pCmd_d             = 0;
   iCurrentLength_d   = 0;
   iAllocatedLength_d = 0;
   pszProgram_d       = 0;

   if (  pszProgram
      && *pszProgram
      )
   {
      pszProgram_d = (char *)malloc (strlen (pszProgram) + 1);
      if (pszProgram_d)
      {
         strcpy (pszProgram_d, pszProgram);
      }
   }
}

PrinterCommand::
~PrinterCommand ()
{
   if (pCmd_d)
   {
      free (pCmd_d);
      pCmd_d = 0;
   }
   if (pszProgram_d)
   {
      free (pszProgram_d);
      pszProgram_d = 0;
   }
}

bool PrinterCommand::
setCommand (PDC_CMD eCommand)
{
   size_t iNewCmdLength = sizeof (PDC_PACKET);

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand          = eCommand;
   pCmd_d->cbLength          = iCurrentLength_d;
   pCmd_d->eFormat           = PDCFMT_NULL;
   pCmd_d->achCommandLine[0] = '\0';

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD  eCommand,
            char    *pszLine)
{
   size_t  iNewCmdLength = sizeof (PDC_PACKET);
   PDC_FMT eFormat       = PDCFMT_NULL;

   if (  pszLine
      && *pszLine
      )
   {
      iNewCmdLength += strlen (pszLine);
      eFormat = PDCFMT_STRING;
   }

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand          = eCommand;
   pCmd_d->cbLength          = iCurrentLength_d;
   pCmd_d->eFormat           = eFormat;
   pCmd_d->achCommandLine[0] = '\0';

   if (  pszLine
      && *pszLine
      )
   {
      strcpy (pCmd_d->achCommandLine, pszLine);
   }

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD eCommand,
            PSZCRO  pszLine)
{
   return setCommand (eCommand, (char *)pszLine);
}

bool PrinterCommand::
setCommand (PDC_CMD      eCommand,
            std::string *pstring)
{
   PSZRO pszString = 0;

   if (pstring)
   {
      pszString = pstring->c_str ();
   }

   return setCommand (eCommand, (char *)pszString);
}

bool PrinterCommand::
setCommand (PDC_CMD     eCommand,
            std::string string)
{
   char *pszString = (char *)string.c_str ();

   return setCommand (eCommand, pszString);
}

bool PrinterCommand::
setCommand (PDC_CMD   eCommand,
            char    **ppszLine)
{
   size_t  iNewCmdLength = sizeof (PDC_PACKET);
   PDC_FMT eFormat       = PDCFMT_NULL;
   size_t  icbLine       = 0;

   if (  ppszLine
      && *ppszLine
      )
   {
      char   *pszLine   = (char *)ppszLine;
      size_t  icbLength = 0;

      while (*pszLine)
      {
         icbLength  = strlen (pszLine) + 1;
         icbLine   += icbLength;
         pszLine   += icbLength;
      }

      iNewCmdLength += icbLine;

      eFormat = PDCFMT_STRING_ARRAY;
   }

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand          = eCommand;
   pCmd_d->cbLength          = iCurrentLength_d;
   pCmd_d->eFormat           = eFormat;
   pCmd_d->achCommandLine[0] = '\0';

   if (icbLine)
   {
      memcpy (pCmd_d->achCommandLine, ppszLine, icbLine + 1);
   }

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD eCommand,
            bool    fValue)
{
   size_t iNewCmdLength = sizeof (PDC_PACKET) - 1 + sizeof (bool);

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand = eCommand;
   pCmd_d->cbLength = iCurrentLength_d;
   pCmd_d->eFormat  = PDCFMT_BOOLEAN;

   unsigned char *pbBuffer = (unsigned char *)pCmd_d->achCommandLine;

   for (size_t i = 0; i < sizeof (bool); i++)
   {
      *pbBuffer = fValue & 0xFF;

      fValue >>= 8;
      pbBuffer++;
   }

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD eCommand,
            int     iValue)
{
   size_t iNewCmdLength = sizeof (PDC_PACKET) - 1 + sizeof (int);

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand = eCommand;
   pCmd_d->cbLength = iCurrentLength_d;
   pCmd_d->eFormat  = PDCFMT_INTEGER;

   unsigned char *pbBuffer = (unsigned char *)pCmd_d->achCommandLine;

   for (size_t i = 0; i < sizeof (int); i++)
   {
      *pbBuffer = iValue & 0xFF;

      iValue >>= 8;
      pbBuffer++;
   }

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD  eCommand,
            long int iValue)
{
   size_t iNewCmdLength = sizeof (PDC_PACKET) - 1 + sizeof (long int);

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand = eCommand;
   pCmd_d->cbLength = iCurrentLength_d;
   pCmd_d->eFormat  = PDCFMT_LONG;

   unsigned char *pbBuffer = (unsigned char *)pCmd_d->achCommandLine;

   for (size_t i = 0; i < sizeof (long int); i++)
   {
      *pbBuffer = iValue & 0xFF;

      iValue >>= 8;
      pbBuffer++;
   }

   return true;
}

bool PrinterCommand::
setCommand (PDC_CMD eCommand,
            PBYTE   pbData,
            int     cbData)
{
   size_t  iNewCmdLength = sizeof (PDC_PACKET);
   PDC_FMT eFormat       = PDCFMT_NULL;

   if (  pbData
      && cbData
      )
   {
      iNewCmdLength += cbData;
      eFormat = PDCFMT_BINARY;
   }

   if (!resizeCommand (iNewCmdLength))
      return false;

   pCmd_d->eCommand          = eCommand;
   pCmd_d->cbLength          = iCurrentLength_d;
   pCmd_d->eFormat           = eFormat;
   pCmd_d->achCommandLine[0] = '\0';

   if (  pbData
      && cbData
      )
   {
      memcpy (pCmd_d->achCommandLine, pbData, cbData);
   }

   return true;
}

bool PrinterCommand::
appendCommand (PSZCRO pszLine)
{
   if (pCmd_d->eFormat != PDCFMT_STRING)
   {
      // It must already be a string
      return false;
   }

   if (  !pszLine
      || !*pszLine
      )
   {
      // No string to append
      return false;
   }

   size_t iNewCmdLength = iCurrentLength_d + strlen (pszLine);
   size_t iAppendPos    = ( iCurrentLength_d
                          - sizeof (PDC_PACKET)
                          );

   if (!resizeCommand (iNewCmdLength))
   {
      // Out of memory
      return false;
   }

   strcpy (pCmd_d->achCommandLine + iAppendPos, pszLine);

   pCmd_d->cbLength = iNewCmdLength;
   iCurrentLength_d = iNewCmdLength;

   return true;
}

PPDC_PACKET PrinterCommand::
getCommand ()
{
   return pCmd_d;
}

char * PrinterCommand::
getCommandString (bool fNewCopy)
{
   if (pCmd_d->cbLength == sizeof (PDC_PACKET))
   {
      return 0;
   }

   if (fNewCopy)
   {
      char *pszNewCopy = 0;

      pszNewCopy = (char *)malloc (strlen (pCmd_d->achCommandLine) + 1);

      if (pszNewCopy)
      {
         strcpy (pszNewCopy, pCmd_d->achCommandLine);
      }

      return pszNewCopy;
   }
   else
   {
      return pCmd_d->achCommandLine;
   }
}

bool PrinterCommand::
getCommandBool (bool& fValue)
{
   if (pCmd_d->cbLength == sizeof (PDC_PACKET) - 1 + sizeof (bool))
   {
      fValue = *(bool*)pCmd_d->achCommandLine;

      return true;
   }

   return false;
}

bool PrinterCommand::
getCommandInt (int& iValue)
{
   if (pCmd_d->cbLength == sizeof (PDC_PACKET) - 1 + sizeof (int))
   {
      iValue = *(int*)pCmd_d->achCommandLine;

      return true;
   }

   return false;
}

bool PrinterCommand::
getCommandLong (long int& lValue)
{
   if (pCmd_d->cbLength == sizeof (PDC_PACKET) - 1 + sizeof (long int))
   {
      lValue = *(long int*)pCmd_d->achCommandLine;

      return true;
   }

   return false;
}

PBYTE PrinterCommand::
getCommandData ()
{
   return (PBYTE)pCmd_d->achCommandLine;
}

PDC_CMD PrinterCommand::
getCommandType ()
{
   return pCmd_d->eCommand;
}

size_t PrinterCommand::
getCommandLength ()
{
   return pCmd_d->cbLength - sizeof (PDC_PACKET) + 1;
}

bool PrinterCommand::
readCommand (int fd)
{
   typedef unsigned char byte;

   byte    *pbBuffer;
   ssize_t  cbBytesToRead;
   ssize_t  rc;

   if (!pCmd_d)
   {
      // Strange, can't use alloc...
      pCmd_d = (PPDC_PACKET)realloc (pCmd_d, sizeof (PDC_PACKET));

      if (!pCmd_d)
      {
#ifndef RETAIL
         DebugOutput::getErrorStream () << "PrinterCommand::"
              << __FUNCTION__
              << ": called by "
              << pszProgram_d
              << ": realloc failed! @ line "
              << __LINE__
              << std::endl;
#endif

         return false;
      }
   }

   cbBytesToRead = sizeof (PDC_PACKET);
   pbBuffer      = (byte *)pCmd_d;

   while (0 < cbBytesToRead)
   {
      rc = read (fd, pbBuffer, cbBytesToRead);

      if (   0 == rc
         || -1 == rc
         )
         break;

      cbBytesToRead -= rc;
      pbBuffer      += rc;
   }

   if (0 < cbBytesToRead)
   {
#ifndef RETAIL
      DebugOutput::getErrorStream () << "PrinterCommand::"
           << __FUNCTION__
           << ": called by "
           << pszProgram_d
           << ": read "
           << sizeof (PDC_PACKET) - cbBytesToRead
           << " bytes, expecting "
           << sizeof (PDC_PACKET)
           << " @ line "
           << __LINE__
           << std::endl;
#endif

      return false;
   }

   if (pCmd_d->cbLength > sizeof (PDC_PACKET))
   {
      if (pCmd_d->cbLength > iAllocatedLength_d)
      {
         pCmd_d = (PPDC_PACKET)realloc (pCmd_d, pCmd_d->cbLength);

         if (!pCmd_d)
         {
#ifndef RETAIL
            DebugOutput::getErrorStream () << "PrinterCommand::"
                 << __FUNCTION__
                 << ": called by "
                 << pszProgram_d
                 << ": realloc failed! @ line "
                 << __LINE__
                 << std::endl;
#endif

            return false;
         }

         iAllocatedLength_d = pCmd_d->cbLength;
      }

      iCurrentLength_d = pCmd_d->cbLength;

      size_t iBytesLeft = pCmd_d->cbLength - sizeof (PDC_PACKET);

      cbBytesToRead = iBytesLeft;
      pbBuffer      = (byte *)(pCmd_d->achCommandLine + 1);

      while (0 < cbBytesToRead)
      {
         rc = read (fd, pbBuffer, cbBytesToRead);

         if (   0 == rc
            || -1 == rc
            )
            break;

         cbBytesToRead -= rc;
         pbBuffer      += rc;
      }

      if (0 < cbBytesToRead)
      {
#ifndef RETAIL
         DebugOutput::getErrorStream () << "PrinterCommand::"
              << __FUNCTION__
              << ": called by "
              << pszProgram_d
              << ": read "
              << iBytesLeft - cbBytesToRead
              << " bytes, expecting "
              << iBytesLeft
              << " @ line "
              << __LINE__
              << std::endl;
#endif

         return false;
      }
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPrinterCommand ())
      DebugOutput::getErrorStream () << "PrinterCommand::"
                                     << __FUNCTION__
                                     << ": "
                                     << pszProgram_d
                                     << " read "
                                     << *this
                                     << std::endl;
#endif

   return true;
}

bool PrinterCommand::
sendCommand (int fd)
{
   size_t rc;

   rc = write (fd, pCmd_d, iCurrentLength_d);

   if (rc != iCurrentLength_d)
   {
#ifndef RETAIL
      DebugOutput::getErrorStream () << "PrinterCommand::"
           << __FUNCTION__
           << ": called by "
           << pszProgram_d
           << ": wrote "
           << rc
           << " bytes, expecting "
           << iCurrentLength_d
           << " @ line "
           << __LINE__
           << std::endl;
#endif

      return false;
   }
   else
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputPrinterCommand ())
         DebugOutput::getErrorStream () << "PrinterCommand::"
                                        << __FUNCTION__
                                        << ": "
                                        << pszProgram_d
                                        << " wrote "
                                        << *this
                                        << std::endl;
#endif

      return true;
   }
}

PSZCRO PrinterCommand::
commandToString (PDC_CMD eCommand)
{
   switch (eCommand)
   {
   // 2.0 Return codes
   case PDCCMD_ACK:                                     return "<Acknowledge>";
   case PDCCMD_NACK:                                    return "<Not Acknowledge>";
   case PDCCMD_UNSUPPORTED:                             return "<Unsupported>";
   case PDCCMD_WARNING:                                 return "<Warning>";

   // 2.1 Session management
   case PDCCMD_INITIALIZE_SESSION:                      return "<Initialize Session>";
   case PDCCMD_CLOSE_SESSION:                           return "<Close Session>";
   case PDCMD_SET_TRANSLATABLE_LANGUAGE:                return "<Set Translatable Language>";
   case PDCMD_GET_TRANSLATABLE_LANGUAGE:                return "<Get Translatable Language>";
   case PDCMD_QUERY_TRANSLATABLE_LANGUAGES:             return "<Query Translatable Languages>";

   // 2.2 Device management
   case PDCCMD_ENUM_SHORT_DEVICES:                      return "<Enumerate Short Devices>";
   case PDCCMD_ENUM_LONG_DEVICES:                       return "<Enumerate Long Devices>";
   case PDCCMD_SET_DEVICE_NAME:                         return "<Set Device Name>";
   case PDCCMD_IS_VALID_DEVICE_NAME:                    return "<Is Valid Device Name>";
   case PDCCMD_GET_PDL_INFO:                            return "<Get PDL Info>";

   // 2.3 Job Properties
   case PDCCMD_GET_JOB_PROPERTIES:                      return "<Get Job Properties>";
   case PDCCMD_SET_JOB_PROPERTIES:                      return "<Set Job Properties>";
   case PDCCMD_GET_JOB_PROPERTY:                        return "<Get Job Property>";
   case PDCCMD_GET_JOB_PROPERTY_TYPE:                   return "<Get Job Property Type>";
   case PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE:             return "<Translate Job Property Key/Value>";

   // 2.4 Printer Properties
   case PDCCMD_SET_PRINTER_PROPERTIES:                  return "<Set Printer Properties>";
   case PDCCMD_QUERY_CURRENT_PRINTER_PROPERTIES:        return "<Query Current Printer Property>";
   case PDCCMD_LIST_PRINTER_PROPERTY_KEYS:              return "<List Printer Property Keys>";
   case PDCCMD_LIST_DEVICE_PRINTER_PROPERTY_KEYS:       return "<List Device Printer Property Keys>";
   case PDCCMD_LIST_PRINTER_PROPERTY_KEY_VALUES:        return "<List Device Printer Property Key Values>";
   case PDCCMD_GET_PRINTER_PROPERTY:                    return "<Get Printer Property>";
   case PDCCMD_GET_PRINTER_PROPERTY_TYPE:               return "<Get Printer Property Type>";
   case PDCMD_XLATE_PRINTER_PROPERTY_KEY_VALUE:         return "<Translate Printer Property Key/Value>";

   // 2.5 Job Control
   case PDCCMD_NEW_DEVICE:                              return "<New Device>";
   case PDCCMD_SET_OUTPUT_STREAM:                       return "<Set Output Stream>";
   case PDCCMD_SET_ERROR_STREAM:                        return "<Set Error Stream>";
   case PDCCMD_BEGIN_JOB:                               return "<Begin Job>";
   case PDCCMD_START_PAGE:                              return "<Start Page>";
   case PDCCMD_END_PAGE:                                return "<End Page>";
   case PDCCMD_END_JOB:                                 return "<End Job>";
   case PDCCMD_ABORT_PAGE:                              return "<Abort Page>";
   case PDCCMD_ABORT_JOB:                               return "<Abort Job>";

   // 2.6 Job Data
   case PDCCMD_MODE_IS_RENDERER:                        return "<Mode Is Renderer>";
   case PDCCMD_ATTACH_BUFFER1:                          return "<Attach Buffer #1>";
   case PDCCMD_ATTACH_BUFFER2:                          return "<Attach Buffer #2>";
   case PDCCMD_DETACH_BUFFER1:                          return "<Detach Buffer #1>";
   case PDCCMD_DETACH_BUFFER2:                          return "<Detach Buffer #2>";
   case PDCCMD_RASTERIZE:                               return "<Rasterize>";
   case PDCMD_QUERY_INPUT_FORMATS:                      return "<Query Input Formats>";
   case PDCMD_SET_INPUT_FORMAT:                         return "<Set Input Format>";

   // 2.7 Job Information (Capabilities)
   case PDCMD_IS_COLOR_PRINTER:                         return "<Is Color Printer>";
   case PDCMD_HAS_HARDWARE_COPY:                        return "<Has Hardware Copy>";

   // 2.8 Private PDC to pdc support
   case PDCCMD_PUSH_CURRENT_GAMMA:                      return "<Push Current Gamma>";
   case PDCCMD_PUSH_CURRENT_BOOKLET:                    return "<Push Current Booklet>";
   case PDCCMD_PUSH_CURRENT_COPIES:                     return "<Push Current Copies>";
   case PDCCMD_PUSH_CURRENT_DITHER_ID:                  return "<Push Current Dither ID>";
   case PDCCMD_PUSH_CURRENT_FORM:                       return "<Push Current Form>";
   case PDCCMD_PUSH_CURRENT_JOGGING:                    return "<Push Current Jogging>";
   case PDCCMD_PUSH_CURRENT_MEDIA:                      return "<Push Current Media>";
   case PDCCMD_PUSH_CURRENT_NUP:                        return "<Push Current NUp>";
   case PDCCMD_PUSH_CURRENT_ORIENTATION:                return "<Push Current Orientation>";
   case PDCCMD_PUSH_CURRENT_OUTPUT_BIN:                 return "<Push Current Output Bin>";
   case PDCCMD_PUSH_CURRENT_PRINT_MODE:                 return "<Push Current Print Mode>";
   case PDCCMD_PUSH_CURRENT_RESOLUTION:                 return "<Push Current Resolution>";
   case PDCCMD_PUSH_CURRENT_SCALING:                    return "<Push Current Scaling>";
   case PDCCMD_PUSH_CURRENT_SHEET_COLLATE:              return "<Push Current Sheet Collate>";
   case PDCCMD_PUSH_CURRENT_SIDE:                       return "<Push Current Side>";
   case PDCCMD_PUSH_CURRENT_STITCHING:                  return "<Push Current Stitching>";
   case PDCCMD_PUSH_CURRENT_TRAY:                       return "<Push Current Tray>";
   case PDCCMD_PUSH_CURRENT_TRIMMING:                   return "<Push Current Trimming>";

   // 2.9 Omni Application Support (Presentation Space Capabilities)
   case PDCCMD_GET_VERSION:                             return "<Get Version>";
   case PDCCMD_GET_DRIVER_NAME:                         return "<Get Driver Name>";
   case PDCCMD_GET_DEVICE_NAME:                         return "<Get Device Name>";
   case PDCCMD_GET_SHORT_NAME:                          return "<Get Device Short Name>";
   case PDCCMD_GET_LIBRARY_NAME:                        return "<Get Library Name>";
   case PDCCMD_GET_OMNI_CLASS:                          return "<Get Omni Class>";

   case PDCCMD_QUERY_CURRENT_GAMMA:                     return "<Query Current Gamma>";
   case PDCCMD_QUERY_CURRENT_BOOKLET:                   return "<Query Current Booklet>";
   case PDCCMD_QUERY_CURRENT_COPIES:                    return "<Query Current Copies>";
   case PDCCMD_QUERY_CURRENT_DITHER_ID:                 return "<Query Current Dither ID>";
   case PDCCMD_QUERY_CURRENT_FORM:                      return "<Query Current Form>";
   case PDCCMD_QUERY_CURRENT_JOGGING:                   return "<Query Current Jogging>";
   case PDCCMD_QUERY_CURRENT_MEDIA:                     return "<Query Current Media>";
   case PDCCMD_QUERY_CURRENT_NUP:                       return "<Query Current NUp>";
   case PDCCMD_QUERY_CURRENT_ORIENTATION:               return "<Query Current Orientation>";
   case PDCCMD_QUERY_CURRENT_OUTPUT_BIN:                return "<Query Current Output Bin>";
   case PDCCMD_QUERY_CURRENT_PRINT_MODE:                return "<Query Current Print Mode>";
   case PDCCMD_QUERY_CURRENT_RESOLUTION:                return "<Query Current Resolution>";
   case PDCCMD_QUERY_CURRENT_SCALING:                   return "<Query Current Scaling>";
   case PDCCMD_QUERY_CURRENT_SHEET_COLLATE:             return "<Query Current Sheet Collate>";
   case PDCCMD_QUERY_CURRENT_SIDE:                      return "<Query Current Side>";
   case PDCCMD_QUERY_CURRENT_STITCHING:                 return "<Query Current Stitching>";
   case PDCCMD_QUERY_CURRENT_TRAY:                      return "<Query Current Tray>";
   case PDCCMD_QUERY_CURRENT_TRIMMING:                  return "<Query Current Trimming>";

   case PDCCMD_HAS_CAPABILITY:                          return "<Has Capability>";
   case PDCCMD_HAS_RASTER_CAPABILITY:                   return "<Has Raster Capability>";
   case PDCCMD_HAS_DEVICE_OPTION:                       return "<Has Device Option>";

   case PDCCMD_IS_BOOKLET_SUPPORTED:                    return "<Is Booklet Supported>";
   case PDCCMD_IS_COPIES_SUPPORTED:                     return "<Is Copies Supported>";
   case PDCCMD_IS_DITHER_ID_SUPPORTED:                  return "<Is Dither ID Supported>";
   case PDCCMD_IS_FORM_SUPPORTED:                       return "<Is Form Supported>";
   case PDCCMD_IS_JOGGING_SUPPORTED:                    return "<Is Jogging Supported>";
   case PDCCMD_IS_MEDIA_SUPPORTED:                      return "<Is Media Supported>";
   case PDCCMD_IS_NUP_SUPPORTED:                        return "<Is NUp Supported>";
   case PDCCMD_IS_ORIENTATION_SUPPORTED:                return "<Is Orientation Supported>";
   case PDCCMD_IS_OUTPUT_BIN_SUPPORTED:                 return "<Is Output Bin Supported>";
   case PDCCMD_IS_PRINT_MODE_SUPPORTED:                 return "<Is Print Mode Supported>";
   case PDCCMD_IS_RESOLUTION_SUPPORTED:                 return "<Is Resolution Supported>";
   case PDCCMD_IS_SCALING_SUPPORTED:                    return "<Is Scaling Supported>";
   case PDCCMD_IS_SHEET_COLLATE_SUPPORTED:              return "<Is Sheet Collate Supported>";
   case PDCCMD_IS_SIDE_SUPPORTED:                       return "<Is Sides Supported>";
   case PDCCMD_IS_STITCHING_SUPPORTED:                  return "<Is Stitching Supported>";
   case PDCCMD_IS_TRAY_SUPPORTED:                       return "<Is Tray Supported>";
   case PDCCMD_IS_TRIMMING_SUPPORTED:                   return "<Is Trimming Supported>";

   case PDCCMD_ENUM_BOOKLETS:                           return "<Enumerate Booklets>";
   case PDCCMD_ENUM_COPIES:                             return "<Enumerate Copies>";
   case PDCCMD_ENUM_DITHER_IDS:                         return "<Enumerate Dither IDs>";
   case PDCCMD_ENUM_FORMS:                              return "<Enumerate Forms>";
   case PDCCMD_ENUM_JOGGINGS:                           return "<Enumerate Joggings>";
   case PDCCMD_ENUM_MEDIAS:                             return "<Enumerate Medias>";
   case PDCCMD_ENUM_NUPS:                               return "<Enumerate NUps>";
   case PDCCMD_ENUM_ORIENTATIONS:                       return "<Enumerate Orientations>";
   case PDCCMD_ENUM_OUTPUT_BINS:                        return "<Enumerate Output Bins>";
   case PDCCMD_ENUM_PRINT_MODES:                        return "<Enumerate Print Modes>";
   case PDCCMD_ENUM_RESOLUTIONS:                        return "<Enumerate Resolutions>";
   case PDCCMD_ENUM_SCALINGS:                           return "<Enumerate Scalings>";
   case PDCCMD_ENUM_SHEET_COLLATES:                     return "<Enumerate Sheet Collates>";
   case PDCCMD_ENUM_SIDES:                              return "<Enumerate Sides>";
   case PDCCMD_ENUM_STITCHINGS:                         return "<Enumerate Stitchings>";
   case PDCCMD_ENUM_TRAYS:                              return "<Enumerate Trays>";
   case PDCCMD_ENUM_TRIMMINGS:                          return "<Enumerate Trimmings>";
   case PDCCMD_ENUM_INSTANCE_PROPS:                     return "<Enumerate Instance Job Properties>";

   default:
   {
      static char achUnknown[26];

      sprintf (achUnknown, "%d(%08X)", eCommand, eCommand);
      return achUnknown;
   }
   }
}

#ifndef RETAIL

void PrinterCommand::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string PrinterCommand::
toString (std::ostringstream& oss)
{
   oss << "{PrinterCommand: ";

///oss << "pszProgram_d = "
///    << pszProgram_d
///    << ", ";

   oss << commandToString (pCmd_d->eCommand)
       << ", "
       << pCmd_d->cbLength;

   if (sizeof (PDC_PACKET) < pCmd_d->cbLength)
   {
      oss << " (+" << pCmd_d->cbLength - sizeof (PDC_PACKET) << ")";
   }

   oss << ", ";

   switch (pCmd_d->eFormat)
   {
   case PDCFMT_NULL:         oss << "NULL";         break;
   case PDCFMT_STRING:       oss << "STRING";       break;
   case PDCFMT_BINARY:       oss << "BINARY";       break;
   case PDCFMT_BOOLEAN:      oss << "BOOLEAN";      break;
   case PDCFMT_INTEGER:      oss << "INTEGER";      break;
   case PDCFMT_LONG:         oss << "LONG";         break;
   case PDCFMT_STRING_ARRAY: oss << "STRING ARRAY"; break;
   case PDCFMT_XML_DATA:     oss << "XML DATA";     break;
   }

   if (PDCFMT_NULL == pCmd_d->eFormat)
   {
   }
   else if (PDCFMT_STRING == pCmd_d->eFormat)
   {
      oss << ", \"" << pCmd_d->achCommandLine << "\"";
   }
   else if (PDCFMT_BINARY == pCmd_d->eFormat)
   {
      size_t iBytesLeft = pCmd_d->cbLength - sizeof (PDC_PACKET);

      oss << ", {" << std::hex;

      for (size_t i = 0; i <= iBytesLeft; i++)
      {
         oss << "0x" << (int)pCmd_d->achCommandLine[i];

         if (i != iBytesLeft)
            oss << ",";
      }

      oss << std::dec << "}";
   }
   else if (PDCFMT_BOOLEAN == pCmd_d->eFormat)
   {
      bool fValue = *(bool *)pCmd_d->achCommandLine;

      oss << ", " << (fValue ? "true" : "false");
   }
   else if (PDCFMT_INTEGER == pCmd_d->eFormat)
   {
      oss << ", " << *(int *)pCmd_d->achCommandLine;
   }
   else if (PDCFMT_LONG == pCmd_d->eFormat)
   {
      oss << ", " << *(long int *)pCmd_d->achCommandLine;
   }
   else if (PDCFMT_STRING_ARRAY == pCmd_d->eFormat)
   {
//////int cbLeft = pCmd_d->cbLength - sizeof (*pCmd_d);

      oss << ", [";

      char *pszString = pCmd_d->achCommandLine;

      while (*pszString)
      {
         oss << "\"" << pszString << "\"";

/////////cbLeft -= strlen (pszString) + 1;

         pszString += strlen (pszString) + 1;

         if (*pszString)
         {
            oss << ", ";
         }
      }

//////oss << ", cbLeft = " << cbLeft << std::endl;

      oss << "]";
   }
   else if (PDCFMT_XML_DATA == pCmd_d->eFormat)
   {
      oss << ", \"" << pCmd_d->achCommandLine << "\"";
   }
   else
   {
      oss << ", Error: unhandled case for format " << (long int)pCmd_d->eFormat;
   }

   oss << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const PrinterCommand& const_self)
{
   PrinterCommand&    self = const_cast<PrinterCommand&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

bool PrinterCommand::
resizeCommand (size_t iNewCmdLength)
{
   if (iNewCmdLength > iAllocatedLength_d)
   {
      pCmd_d             = (PPDC_PACKET)realloc (pCmd_d, iNewCmdLength);
      iAllocatedLength_d = iNewCmdLength;
   }

   iCurrentLength_d = iNewCmdLength;

   if (!pCmd_d)
   {
      iCurrentLength_d = 0;

#ifndef RETAIL
      DebugOutput::getErrorStream () << "PrinterCommand::"
           << __FUNCTION__
           << ": called by "
           << pszProgram_d
           << ": Command allocation failed" << std::endl;
#endif

      return false;
   }

   return true;
}
