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
#include "PluggableInstance.hpp"
#include "Omni.hpp"
#include "JobProperties.hpp"
#include "Enumeration.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>

static PSZCRO vpszPluggableServerToClient = "OMNI_BLITTER_S2C";
static PSZCRO vpszPluggableClientToServer = "OMNI_BLITTER_C2S";

static char *setupBlitterName (PSZCRO pszName,
                               PSZCRO pszFormat,
                               PSZCRO pszShortName);

void
deletePluggableInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

static char
encodeCharacter (int iCh)
{
   if (26 > iCh)
      return 'a' + iCh;
   else if (52 > iCh)
      return 'A' + (iCh - 26);
   else if (62 > iCh)
      return '0' + (iCh - 52);
   else if (62 == iCh)
      return '+';
   else if (63 == iCh)
      return '-';

   return '?';
}

static char *
setupBlitterName (PSZCRO pszName,
                  PSZCRO pszFormat,
                  PSZCRO pszShortName)
{
   std::ostringstream  oss;
   struct timeval      tv;
   long int            lTime;
   std::string         stringResult;
   char               *pszRet       = 0;
   int                 rc;

   oss << pszName
       << pszFormat
       << pszShortName
       << "_";

   rc = gettimeofday (&tv, 0);
   if (!rc)
   {
      lTime = tv.tv_usec;
      for (int i = 0; i < 6; i++)
      {
         oss << encodeCharacter (lTime & 0x3F);

         lTime >>= 6;
      }

      lTime = tv.tv_sec;
      for (int i = 0; i < 6; i++)
      {
         oss << encodeCharacter (lTime & 0x3F);

         lTime >>= 6;
      }

      oss << std::ends;
   }

   stringResult = oss.str ();

   pszRet = (char *)malloc (strlen (stringResult.c_str ()) + 1);

   if (!pszRet)
      return 0;

   strcpy (pszRet, stringResult.c_str ());

   if (-1 == putenv (pszRet))
      return 0;

   return pszRet;
}

PluggableInstance::
PluggableInstance (PrintDevice *pDevice,
                   PSZCRO       pszExeName,
                   PSZCRO       pszData)
   : DeviceInstance (pDevice)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": exename = " << pszExeName << std::endl;
#endif

   fHasError_d             = false;
   pszExeName_d            = 0;
   pszData_d               = 0;

   fdS2C_d                 = -1;
   fdC2S_d                 = -1;
   fRemoveS2C_d            = false;
   fRemoveC2S_d            = false;
   pszS2C_d                = 0;
   pszC2S_d                = 0;

   idBuffer1_d             = -1;
   cbBuffer1_d             = 0;
   pbBuffer1_d             = 0;
   idBuffer2_d             = -1;
   cbBuffer2_d             = 0;
   pbBuffer2_d             = 0;

   pCmd_d                  = 0;

   fdOutputStream_d        = STDOUT_FILENO; // @TBD
   fdErrorStream_d         = fileno (DebugOutput::getErrorStreamFILE ());

   pjpJobProperties_d      = new JobProperties ("");

   if (pszExeName)
   {
      pszExeName_d = (char *)malloc (strlen (pszExeName) + 1);
      if (pszExeName_d)
      {
         strcpy (pszExeName_d, pszExeName);
      }
   }
}

PluggableInstance::
~PluggableInstance ()
{
   stopPDCSession (false);

   if (pszExeName_d)
   {
      free (pszExeName_d);
      pszExeName_d = 0;
   }

   delete pjpJobProperties_d; pjpJobProperties_d = 0;
}

void PluggableInstance::
initializeInstance (PSZCRO pszJobProperties)
{
}

bool PluggableInstance::
hasError ()
{
   return fHasError_d;
}

void PluggableInstance::
startPDCSession ()
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": pCmd_d = " << pCmd_d << ", fHasError_d = " << fHasError_d << ", pszExeName_d = " << pszExeName_d << std::endl;
#endif

   if (pCmd_d)
   {
      // Already initialized
      return;
   }

   if (  fHasError_d
      || !pszExeName_d
      )
   {
      // Something failed somewhere
      return;
   }

   char        *pszS2C       = 0;
   char        *pszC2S       = 0;
   int          fdStdOut     = fdOutputStream_d;
   int          fdStdErr     = fdErrorStream_d;
   bool         fSucceeded   = false;
   int          pid;

   fHasError_d = true;

   // Assign fifo names for environment space
   pszS2C_d = setupBlitterName (vpszPluggableServerToClient,
                                "=/tmp/PDC_B_s2c_",
                                pDevice_d->getShortName ());
   pszC2S_d = setupBlitterName (vpszPluggableClientToServer,
                                "=/tmp/PDC_B_c2s_",
                                pDevice_d->getShortName ());
   pszS2C   = getenv (vpszPluggableServerToClient);
   pszC2S   = getenv (vpszPluggableClientToServer);
   pCmd_d   = new PrinterCommand ("PluggableInstance");

   if (  !pszS2C_d
      || !pszC2S_d
      || !pCmd_d
      )
   {
      if (!pszS2C_d)
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": pszS2C_d is NULL!" << std::endl;
      if (!pszC2S_d)
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": pszC2S_d is NULL!" << std::endl;
      if (!pCmd_d)
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": pCmd_d is NULL!" << std::endl;

      goto BUGOUT;
   }

   /* Create actual fifos in filesystem. */
   if ((mkfifo (pszS2C, 0666)) < 0)
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": mkfifo (" << pszS2C << ") failed." << std::endl;

      goto BUGOUT;
   }
   fRemoveS2C_d = true;

   if ((mkfifo (pszC2S, 0666)) < 0)
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": mkfifo (" << pszC2S << ") failed." << std::endl;

      goto BUGOUT;
   }
   fRemoveC2S_d = true;

   /* Spawn the server. */
   if ((pid = fork ()) < 0)
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": fork of " << pszExeName_d << " failed." << std::endl;

      goto BUGOUT;
   }
   else if (pid > 0)
   {
      /* parent */

      /* Open fifos. */
      if ((fdS2C_d = open (pszS2C, O_RDONLY)) < 0)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": open (" << pszS2C << ") failed." << std::endl;

         goto BUGOUT;
      }
      if ((fdC2S_d = open (pszC2S, O_WRONLY)) < 0)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": open (" << pszC2S << ") failed." << std::endl;

         goto BUGOUT;
      }
   }
   else
   {
      /* child */
      char         *argv[5];
      int           i       = 0;

      /* Redirect print_stream to stdout. */
      if (fdStdOut != STDOUT_FILENO)
      {
         dup2 (fdStdOut, STDOUT_FILENO);
      }
      if (fdStdErr != STDERR_FILENO)
      {
         dup2 (fdStdErr, STDERR_FILENO);
      }

//////argv[i++] = "sh";
//////argv[i++] = "-c";
      argv[i++] = (char *)pszExeName_d;
      argv[i++] = NULL;

      if (execvp (argv[0], argv) < 0)
      {
         DebugOutput::getErrorStream () << "Error: execlp of " << pszExeName_d << " failed!" << std::endl;

         // Send a negative-reply to the waiting parent process.
         int            fdS2C                           = open (pszS2C, O_WRONLY);
         int            fdC2S                           = open (pszC2S, O_RDONLY);
         PrinterCommand cmd ("PluggableInstance/error");

         if (0 < fdS2C)
         {
            cmd.setCommand (PDCCMD_NACK);
            cmd.sendCommand (fdS2C);

            close (fdS2C);
         }
         else
         {
            DebugOutput::getErrorStream () << "Error: Could not send NACK to parent." << std::endl;
         }

         if (0 < fdC2S)
         {
            cmd.readCommand (fdC2S);

            close (fdC2S);
         }
         else
         {
            DebugOutput::getErrorStream () << "Error: Could not read parent's response to NACK." << std::endl;
         }

         exit (EXIT_FAILURE);
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": execlp succeded." << std::endl;
#endif

      exit (0);
   }

   // Initialize the session
   if (  !pCmd_d->setCommand (PDCCMD_INITIALIZE_SESSION, PDC_VERSION)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Cannot initialize the session!" << std::endl;

      goto BUGOUT;
   }

   fHasError_d = false;

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Setting the device name to \"" << pDevice_d->getDeviceName () << "\"" << std::endl;
#endif

   // Ask the server if the device name is valid
   if (  !pCmd_d->setCommand (PDCCMD_IS_VALID_DEVICE_NAME, pDevice_d->getDeviceName ())
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Pluggable device name is not valid!" << std::endl;

      goto BUGOUT;
   }

   // Tell the server what the device name will be
   if (  !pCmd_d->setCommand (PDCCMD_SET_DEVICE_NAME, pDevice_d->getDeviceName ())
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Failed to set Pluggable device name!" << std::endl;

      goto BUGOUT;
   }

   fSucceeded = true;

BUGOUT:
   if (!fSucceeded)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": failed!" << std::endl;
#endif

      // @TBD throw error?

      stopPDCSession (true);

      return;
   }
}

void PluggableInstance::
stopPDCSession (bool fError)
{
   PSZRO pszS2C = 0;
   PSZRO pszC2S = 0;
   int   rc     = 0;

   // End the connection
   if (pCmd_d)
   {
      if (  !fHasError_d
         && pCmd_d->setCommand (PDCCMD_CLOSE_SESSION)
         )
      {
         pCmd_d->sendCommand (fdC2S_d);
      }

      delete pCmd_d;

      pCmd_d = 0;
   }

   if (pszS2C_d)
   {
      pszS2C = strchr (pszS2C_d, '=');
      if (pszS2C)
         pszS2C++;
   }
   if (pszC2S_d)
   {
      pszC2S = strchr (pszC2S_d, '=');
      if (pszC2S)
         pszC2S++;
   }

   // Clean up
   if (-1 != fdS2C_d)
   {
      rc = close (fdS2C_d);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": remove (" << pszS2C << ") = " << errno << std::endl;
      }
      fdS2C_d = -1;
   }

   if (-1 != fdC2S_d)
   {
      rc = close (fdC2S_d);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": remove (" << pszC2S << ") = " << errno << std::endl;
      }
      fdC2S_d = -1;
   }

   if (  fRemoveS2C_d
      && pszS2C
      )
   {
      rc = remove (pszS2C);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": remove (" << pszS2C << ") = " << errno << std::endl;
      }
      fRemoveS2C_d = false;
   }

   if (  fRemoveC2S_d
      && pszC2S
      )
   {
      rc = remove (pszC2S);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": remove (" << pszC2S << ") = " << errno << std::endl;
      }
      fRemoveC2S_d = false;
   }

   if (pszS2C_d)
   {
      free (pszS2C_d);
      pszS2C_d = 0;
   }

   if (pszC2S_d)
   {
      free (pszC2S_d);
      pszC2S_d = 0;
   }

   if (pbBuffer1_d)
   {
      shmdt (pbBuffer1_d);
      shmctl (idBuffer1_d, IPC_RMID, 0);
      pbBuffer1_d = 0;
   }

   if (pbBuffer2_d)
   {
      shmdt (pbBuffer2_d);
      shmctl (idBuffer2_d, IPC_RMID, 0);
      pbBuffer2_d = 0;
   }
   if (pszData_d)
   {
      free (pszData_d);
      pszData_d = 0;
   }

   fHasError_d = fError;

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Stopped the session.  fHasError_d = " << fHasError_d << std::endl;
#endif
}

std::string * PluggableInstance::
getJobProperties (bool fInDeviceSpecific)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTIES, fInDeviceSpecific)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      return new std::string (pCmd_d->getCommandString (false));
   }

   return 0;
}

bool PluggableInstance::
setJobProperties (PSZCRO pszJobProperties)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   pjpJobProperties_d->setJobProperties (pszJobProperties);

   pushDeviceObjects ();

   if (  pCmd_d->setCommand (PDCCMD_SET_JOB_PROPERTIES, pszJobProperties)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      return true;
   }

   return false;
}

class JPEnumerator : public EnumEnumerator
{
public:
   JPEnumerator (bool fInDeviceSpecific) // @TBD
   {
   }

   virtual
   ~JPEnumerator ()
   {
   }
};

Enumeration * PluggableInstance::
getGroupEnumeration (bool fInDeviceSpecific)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return new NullEnumerator ();
      }
   }

   JPEnumerator *pRet = new JPEnumerator (fInDeviceSpecific);

   if (  pCmd_d->setCommand (PDCCMD_ENUM_INSTANCE_PROPS, fInDeviceSpecific)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      PSZ pszResponse = pCmd_d->getCommandString (false);
      int cbResponse  = pCmd_d->getCommandLength ();
      PSZ pszArray    = 0;
      int cbArray     = 0;

      if (cbResponse)
      {
/////////std::cerr << "[";

         while (*pszResponse)
         {
////////////std::cerr << "[";

            pszArray = pszResponse;
            cbArray  = 0;

            while (*pszResponse)
            {
///////////////std::cerr << pszResponse;

               cbArray     += strlen (pszResponse) + 1;
               pszResponse += strlen (pszResponse) + 1;

///////////////if (*pszResponse)
///////////////{
///////////////   std::cerr << ",";
///////////////}
            }

            cbArray++;

////////////std::cerr << "]";

            pszResponse++;

            pRet->addElement (new StringArrayJPEnumeration (pszArray, cbArray));

////////////if (*pszResponse)
////////////{
////////////   std::cerr << ",";
////////////}
         }

/////////std::cerr << "]";
      }
   }

   return pRet;
}

std::string * PluggableInstance::
getJobPropertyType (PSZCRO pszKey)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTY_TYPE, pszKey)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      return new std::string (pCmd_d->getCommandString (false));
   }

   return 0;
}

std::string * PluggableInstance::
getJobProperty (PSZCRO pszKey)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTY, pszKey)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      return new std::string (pCmd_d->getCommandString (false));
   }

   return 0;
}

std::string * PluggableInstance::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   if (!pCmd_d->setCommand (PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE, pszKey))
   {
      return DeviceInstance::translateKeyValue (pszKey, pszValue);
   }

   if (  pszValue
      && (  !pCmd_d->appendCommand ("=")
         || !pCmd_d->appendCommand (pszValue)
         )
      )
   {
      return DeviceInstance::translateKeyValue (pszKey, pszValue);
   }

   if (  pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      return new std::string (pCmd_d->getCommandString (false));
   }

   return DeviceInstance::translateKeyValue (pszKey, pszValue);
}

bool PluggableInstance::
setOutputStream (FILE *pFile)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Setting the output stream to " << pFile << std::endl;
#endif

   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }
   else
   {
      stopPDCSession (false);
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }

      PSZCRO pszJobProperties = pjpJobProperties_d->getJobProperties (false);

      if (pszJobProperties)
      {
         setJobProperties (pszJobProperties);

         free ((void *)pszJobProperties);
      }
   }

   char achIntegerString[11];  // 4294967295 + '\0' // @TBD
   int  iFileNo              = fileno (pFile);

   sprintf (achIntegerString, "%d", iFileNo);

   fdOutputStream_d = iFileNo;

   if (  !pCmd_d->setCommand (PDCCMD_SET_OUTPUT_STREAM, achIntegerString)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool PluggableInstance::
setErrorStream (FILE *pFile)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Setting the error stream to " << pFile << std::endl;
#endif

   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   char achIntegerString[11];  // 4294967295 + '\0' // @TBD
   int  iFileNo              = fileno (pFile);

   sprintf (achIntegerString, "%d", iFileNo);

   fdErrorStream_d = iFileNo;

   if (  !pCmd_d->setCommand (PDCCMD_SET_ERROR_STREAM, achIntegerString)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool PluggableInstance::
setLanguage (int iLanguageID)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": Setting the language to " << iLanguageID << std::endl;
#endif

   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   PSZCRO pszLanguageID = StringResource::IDToName (iLanguageID);

   if (  !pCmd_d->setCommand (PDCMD_SET_TRANSLATABLE_LANGUAGE, pszLanguageID)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

void PluggableInstance::
pushDeviceObjects ()
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   DeviceBooklet      *pBooklet       = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   DeviceCopies       *pCopies        = 0;
#endif
   DeviceForm         *pForm          = 0;
   HardCopyCap        *pHCC           = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   DeviceJogging      *pJogging       = 0;
#endif
   DeviceMedia        *pMedia         = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   DeviceNUp          *pNUp           = 0;
#endif
   DeviceOrientation  *pOrientation   = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   DeviceOutputBin    *pOutputBin     = 0;
#endif
   DevicePrintMode    *pPrintMode     = 0;
   DeviceResolution   *pResolution    = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   DeviceScaling      *pScaling       = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   DeviceSheetCollate *pSheetCollate  = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   DeviceSide         *pSide          = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   DeviceStitching    *pStitching     = 0;
#endif
   DeviceTray         *pTray          = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   DeviceTrimming     *pTrimming      = 0;
#endif
   DeviceGamma        *pGamma         = 0;
   std::string        *pstringJP      = 0;
   PSZRO               pszJPQuoted    = 0;
   std::ostringstream  oss;

#ifdef INCLUDE_JP_UPDF_BOOKLET

   pBooklet = pDevice_d->getCurrentBooklet ();

   pstringJP = pBooklet->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted; // @TBD

      if (pBooklet->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pBooklet->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_BOOKLET, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

#ifdef INCLUDE_JP_COMMON_COPIES

   pCopies = pDevice_d->getCurrentCopies ();

   pstringJP = pCopies->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << pCopies->getMinimum ()
          << " "
          << pCopies->getMaximum ()
          << " "
          << (pCopies->needsSimulation () ? 1 : 0);

      if (pCopies->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pCopies->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_COPIES, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_DITHER_ID, pDevice_d->getCurrentDitherID ())
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
      return;

   pForm = pDevice_d->getCurrentForm ();
   pHCC  = pForm->getHardCopyCap ();

   pstringJP = pForm->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
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

      if (pForm->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pForm->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_FORM, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#ifdef INCLUDE_JP_UPDF_JOGGING

   pJogging = pDevice_d->getCurrentJogging ();

   pstringJP = pJogging->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted; // @TBD

      if (pJogging->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pJogging->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_JOGGING, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   pMedia = pDevice_d->getCurrentMedia ();

   pstringJP = pMedia->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << pMedia->getColorAdjustRequired ()
          << " "
          << pMedia->getAbsorption ();

      if (pMedia->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pMedia->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_MEDIA, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#ifdef INCLUDE_JP_COMMON_NUP

   pNUp = pDevice_d->getCurrentNUp ();

   pstringJP = pNUp->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << (pNUp->needsSimulation () ? 1 : 0);

      if (pNUp->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pNUp->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_NUP, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   pOrientation = pDevice_d->getCurrentOrientation ();

   pstringJP = pOrientation->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << (pOrientation->needsSimulation () ? 1 : 0);

      if (pOrientation->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pOrientation->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_ORIENTATION, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

   pOutputBin = pDevice_d->getCurrentOutputBin ();

   pstringJP = pOutputBin->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted;

      if (pOutputBin->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pOutputBin->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_OUTPUT_BIN, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   pPrintMode = pDevice_d->getCurrentPrintMode ();

   pstringJP = pPrintMode->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << pPrintMode->getPhysicalCount ()
          << " "
          << pPrintMode->getLogicalCount ()
          << " "
          << pPrintMode->getNumPlanes ();

      if (pPrintMode->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pPrintMode->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_PRINT_MODE, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

   // Push the resolution before pushing the form!
   pResolution = pDevice_d->getCurrentResolution ();

   pstringJP = pResolution->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
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

      if (pResolution->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pResolution->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_RESOLUTION, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#ifdef INCLUDE_JP_COMMON_SCALING

   pScaling = pDevice_d->getCurrentScaling ();

   pstringJP = pScaling->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << pScaling->getMinimumPercentage ()
          << " "
          << pScaling->getMaximumPercentage ();

      if (pScaling->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pScaling->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_SCALING, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

   pSheetCollate = pDevice_d->getCurrentSheetCollate ();

   pstringJP = pSheetCollate->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted;

      if (pSheetCollate->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pSheetCollate->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_SHEET_COLLATE, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

   pSide = pDevice_d->getCurrentSide ();

   pstringJP = pSide->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << (pSide->needsSimulation () ? 1 : 0);

      if (pSide->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pSide->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_SIDE, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

   pStitching = pDevice_d->getCurrentStitching ();

   pstringJP = pStitching->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted;

      if (pStitching->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pStitching->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_STITCHING, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   pTray = pDevice_d->getCurrentTray ();

   pstringJP = pTray->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted
          << " "
          << pTray->getType ();

      if (pTray->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pTray->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_TRAY, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#ifdef INCLUDE_JP_COMMON_TRIMMING

   pTrimming = pDevice_d->getCurrentTrimming ();

   pstringJP = pTrimming->getJobProperties ();
   if (!pstringJP)
      return;

   pszJPQuoted = Omni::quoteString (pstringJP->c_str ());

   if (pszJPQuoted)
   {
      oss.str ("");
      oss << pszJPQuoted;

      if (pTrimming->getDeviceID ())
      {
         PSZCRO pszIDQuoted = Omni::quoteString (pTrimming->getDeviceID ());

         if (pszIDQuoted)
         {
            oss << " "
                << pszIDQuoted;

            free ((void *)pszIDQuoted);
         }
      }

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_TRIMMING, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;

      free ((void *)pszJPQuoted);
   }

   delete pstringJP;

#endif

   pGamma = pDevice_d->getCurrentGamma ();

   if (pGamma)
   {
      oss.str ("");
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

      if (  !pCmd_d->setCommand (PDCCMD_PUSH_CURRENT_GAMMA, oss.str ().c_str ())
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return;
   }
}

bool PluggableInstance::
commonBeginJob ()
{
   if (fHasError_d)
   {
      return false;
   }

   // Start the job on the device
   if (  pCmd_d->setCommand (PDCCMD_BEGIN_JOB)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool PluggableInstance::
beginJob ()
{
   if (fHasError_d)
   {
      return false;
   }

   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   return commonBeginJob ();
}

bool PluggableInstance::
beginJob (bool fJobPropertiesChanged)
{
   bool    fRetVal      = false;

   if (fHasError_d)
   {
      return false;
   }

   if (!pCmd_d)
   {
      startPDCSession ();

      if (!pCmd_d)
      {
         return false;
      }
   }

   fRetVal = commonBeginJob ();

   return fRetVal;
}

bool PluggableInstance::
commonNewFrame ()
{
   // Send a new page to the device
   if (  pCmd_d->setCommand (PDCCMD_END_PAGE)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // Send a new page to the device
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool PluggableInstance::
newFrame ()
{
   if (  fHasError_d
      || !pCmd_d
      )
   {
      return false;
   }

   return commonNewFrame ();
}

bool PluggableInstance::
newFrame (bool fJobPropertiesChanged)
{
   bool fRetVal = false;

   if (  fHasError_d
      || !pCmd_d
      )
   {
      return false;
   }

   fRetVal = commonNewFrame ();

   return fRetVal;
}

bool PluggableInstance::
endJob ()
{
   if (  fHasError_d
      || !pCmd_d
      )
   {
      return false;
   }

   // End the job on the device
   if (  pCmd_d->setCommand (PDCCMD_END_PAGE)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // End the job on the device
      if (  pCmd_d->setCommand (PDCCMD_END_JOB)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool PluggableInstance::
abortJob ()
{
   if (  fHasError_d
      || !pCmd_d
      )
   {
      return false;
   }

   // Abort the job on the device
   if (  pCmd_d->setCommand (PDCCMD_ABORT_JOB)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return true;

   return false;
}

bool PluggableInstance::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
   if (  fHasError_d
      || !pCmd_d
      )
   {
      return false;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ())
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__
           <<  ": pbmi->cbFix = " << pbmi->cbFix
           << ", cbBuffer1_d = " << cbBuffer1_d;
#endif

   if (pbmi->cbFix > cbBuffer1_d)
   {
      // Release the previous buffer
      if (pbBuffer1_d)
      {
         if (  !pCmd_d->setCommand (PDCCMD_DETACH_BUFFER1, idBuffer1_d)
            || !pCmd_d->sendCommand (fdC2S_d)
            || !pCmd_d->readCommand (fdS2C_d)
            || PDCCMD_ACK != pCmd_d->getCommandType ()
            )
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": could not PDCCMD_DETACH_BUFFER1" << std::endl;
#endif

            return false;
         }

         shmdt (pbBuffer1_d);
         shmctl (idBuffer1_d, IPC_RMID, 0);

         idBuffer1_d = -1;
         cbBuffer1_d = 0;
         pbBuffer1_d = 0;
      }

      idBuffer1_d = shmget (IPC_PRIVATE, pbmi->cbFix, 0666);

      if (0 < idBuffer1_d)
      {
         cbBuffer1_d = pbmi->cbFix;
         pbBuffer1_d = (byte *)shmat (idBuffer1_d, 0, 0);

         if (-1 == (int)pbBuffer1_d)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": 1st shmat (" << idBuffer1_d << ") failed." << std::endl;
#endif

            return false;
         }

         if (  !pCmd_d->setCommand (PDCCMD_ATTACH_BUFFER1, idBuffer1_d)
            || !pCmd_d->sendCommand (fdC2S_d)
            || !pCmd_d->readCommand (fdS2C_d)
            || PDCCMD_ACK != pCmd_d->getCommandType ()
            )
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": could not PDCCMD_ATTACH_BUFFER1" << std::endl;
#endif

            return false;
         }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputPluggableInstance ())
         DebugOutput::getErrorStream () << ", now cbBuffer1_d = " << cbBuffer1_d;
#endif
   }

   int cbBuffer2;

   cbBuffer2 = pbmi->cy * (((pbmi->cx * pbmi->cBitCount + 31) >> 5) << 2);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ())
      DebugOutput::getErrorStream () << ", cbBuffer2_d = " << cbBuffer2_d;
#endif

   if (cbBuffer2 > cbBuffer2_d)
   {
      // Release the previous buffer
      if (pbBuffer2_d)
      {
         if (  !pCmd_d->setCommand (PDCCMD_DETACH_BUFFER2, idBuffer2_d)
            || !pCmd_d->sendCommand (fdC2S_d)
            || !pCmd_d->readCommand (fdS2C_d)
            || PDCCMD_ACK != pCmd_d->getCommandType ()
            )
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": could not PDCCMD_DETACH_BUFFER2" << std::endl;
#endif

            return false;
         }

         shmdt (pbBuffer2_d);
         shmctl (idBuffer2_d, IPC_RMID, 0);

         idBuffer2_d = -1;
         cbBuffer2_d = 0;
         pbBuffer2_d = 0;
      }

      idBuffer2_d = shmget (IPC_PRIVATE, cbBuffer2, 0666);

      if (0 < idBuffer2_d)
      {
         cbBuffer2_d = cbBuffer2;
         pbBuffer2_d = (byte *)shmat (idBuffer2_d, 0, 0);

         if (-1 == (int)pbBuffer2_d)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": 2nd shmat (" << idBuffer2_d << ") failed." << std::endl;
#endif

            return false;
         }

         if (  !pCmd_d->setCommand (PDCCMD_ATTACH_BUFFER2, idBuffer2_d)
            || !pCmd_d->sendCommand (fdC2S_d)
            || !pCmd_d->readCommand (fdS2C_d)
            || PDCCMD_ACK != pCmd_d->getCommandType ()
            )
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputPluggableInstance ())
               DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": could not PDCCMD_ATTACH_BUFFER2" << std::endl;
#endif

            return false;
         }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputPluggableInstance ())
         DebugOutput::getErrorStream () << ", now cbBuffer2_d = " << cbBuffer2_d;
#endif
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ()) DebugOutput::getErrorStream () << std::endl;
#endif

   memcpy (pbBuffer1_d, pbmi,   pbmi->cbFix);
   memcpy (pbBuffer2_d, pbBits, cbBuffer2);

   char achBuffer[64]; // @TBD

   sprintf (achBuffer,
            "%d %d %d %d %d",
            eType,
            prectlPageLocation->xLeft,
            prectlPageLocation->yBottom,
            prectlPageLocation->xRight,
            prectlPageLocation->yTop);

   if (  pCmd_d->setCommand (PDCCMD_RASTERIZE, achBuffer)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return true;

#ifndef RETAIL
   if (DebugOutput::shouldOutputPluggableInstance ())
      DebugOutput::getErrorStream () << "PluggableInstance::" << __FUNCTION__ << ": could not PDCCMD_RASTERIZE" << std::endl;
#endif

   return false;
}

#ifndef RETAIL

void PluggableInstance::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string PluggableInstance::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{PluggableInstance: "
       << DeviceInstance::toString (oss2) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const PluggableInstance& const_self)
{
   PluggableInstance& self = const_cast<PluggableInstance&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
