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
#include "OmniPDCProxy.hpp"
#include "JobProperties.hpp"
#include "Omni.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
        int val;                    /* value for SETVAL */
        struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

static PSZCRO vpszServerToClient     = "PDC_SRV_TO_CLIENT";
static PSZCRO vpszClientToServer     = "PDC_CLIENT_TO_SRV";
static PSZCRO vpszExecProgram        = "PDC_EXEC_PROGRAM";
static PSZCRO vpszExecProgramDefault = "OmniPDCServer";

char *
setupName (PSZCRO pszName,
           PSZCRO pszFormat)
{
#define ENVIRONMENT_SIZE 64
   char   *pszRet = 0;
   size_t  rc;

   pszRet = (char *)malloc (ENVIRONMENT_SIZE);

   if (!pszRet)
      return 0;

   rc = sprintf (pszRet, pszFormat, pszName, getpid ());
   if (rc > ENVIRONMENT_SIZE)
   {
      exit (EXIT_FAILURE);
   }

   if (-1 == putenv (pszRet))
      return 0;

   return pszRet;
}

OmniPDCProxy::
OmniPDCProxy (PSZRO  pszClientExe,
              PSZCRO pszDeviceName,
              PSZCRO pszJobProperties,
              bool   fAdvanced,
              int    fdStdOut,
              int    fdStdErr)
{
   char        *pszS2C            = 0;
   char        *pszC2S            = 0;
   bool         fSucceeded        = false;
   int          pid;
   int          idSem             = -1;

   pszClientExe_d           = 0;

   if (  !pszClientExe
      || !*pszClientExe
      )
   {
      pszClientExe = getenv (vpszExecProgram);

      if (!pszClientExe)
      {
         pszClientExe = vpszExecProgramDefault;
      }
   }

   pszClientExe_d = (PSZ)malloc (strlen (pszClientExe) + 1);
   if (pszClientExe_d)
   {
      strcpy (pszClientExe_d, pszClientExe);
   }

   fHasError_d              = false;

   fAdvanced_d              = fAdvanced;

   // Assign fifo names for environment space
   pszS2C_d                 = setupName (vpszServerToClient, "%s=/tmp/PDC_s2c_%d");
   pszC2S_d                 = setupName (vpszClientToServer, "%s=/tmp/PDC_c2s_%d");
   pszS2C                   = getenv (vpszServerToClient);
   pszC2S                   = getenv (vpszClientToServer);

   fdS2C_d                  = -1;
   fdC2S_d                  = -1;

   idBuffer1_d              = -1;
   cbBuffer1_d              = 0;
   pbBuffer1_d              = 0;
   idBuffer2_d              = -1;
   cbBuffer2_d              = 0;
   pbBuffer2_d              = 0;

   pCmd_d                   = 0;

   pszVersion_d             = 0;
   pszDriverName_d          = 0;
   pszDeviceName_d          = 0;
   pszShortName_d           = 0;
   pszLibraryName_d         = 0;
   eOmniClass_d             = OMNI_CLASS_UNKNOWN;

   iLanguageID_d            = StringResource::LANGUAGE_DEFAULT;
   pLanguage_d              = StringResource::create (iLanguageID_d, 0);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   pBooklet_d               = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   pCopies_d                = 0;
#endif
   pszDitherID_d            = 0;
   pForm_d                  = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   pJogging_d               = 0;
#endif
   pMedia_d                 = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   pNUp_d                   = 0;
#endif
   pOrientation_d           = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   pOutputBin_d             = 0;
#endif
   pPrintMode_d             = 0;
   pResolution_d            = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   pScaling_d               = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   pSheetCollate_d          = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   pSide_d                  = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   pStitching_d             = 0;
#endif
   pTray_d                  = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   pTrimming_d              = 0;
#endif
   pGamma_d                 = 0;

   fQueriedPDLInfo_d        = false;
   iPDLLevel_d              = 0;
   iPDLSubLevel_d           = 0;
   iPDLMajorRevisionLevel_d = 0;
   iPDLMinorRevisionLevel_d = 0;

   if (  !pszS2C_d
      || !pszC2S_d
      )
   {
      goto BUGOUT;
   }

   DebugOutput::applyAllDebugOutput (pszJobProperties);

   /* Create actual fifos in filesystem. */
   if ((mkfifo (pszS2C, 0666)) < 0)
   {
      goto BUGOUT;
   }

   if ((mkfifo (pszC2S, 0666)) < 0)
   {
      goto BUGOUT;
   }

   idSem = semget (IPC_PRIVATE, 1, SHM_R | SHM_W);

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": idSem = " << idSem << std::endl;
#endif

   if (-1 != idSem)
   {
      union semun sunion;

      sunion.val = 0;

#ifndef RETAIL
      int rc =
#endif
               semctl (idSem, 0, SETVAL, sunion);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": semctl returns " << rc << std::endl;
#endif
   }

   /* Spawn the server. */
   if ((pid = fork ()) < 0)
   {
      std::cerr << "Error: Fork PDC failed" << std::endl;

      goto BUGOUT;
   }
   else if (pid > 0)
   {
      /* parent */
      int           rc;
      int           status = 0;
      struct sembuf sb;

      sb.sem_num = 0;
      sb.sem_op = -1;
      sb.sem_flg = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Parent requesting sem." << std::endl;
#endif

      rc = semop (idSem, &sb, 1);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Parent semop " << rc << std::endl;
#endif

      rc = waitpid (pid, &status, WNOHANG);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ())
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": waitpid returns " << rc << ", status = " << status << std::endl;
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": waitpid pid = " << pid << std::endl;
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": waitpid WIFEXITED (status) = " << WIFEXITED (status) << std::endl;
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": waitpid WEXITSTATUS (status) = " << WEXITSTATUS (status) << std::endl;
      }
#endif

      if (  rc == pid
         && 0 != WIFEXITED (status)
         && EXIT_FAILURE == WEXITSTATUS (status) // child failed
         )
      {
         std::cerr << "Error: Cannot run client" << std::endl;

         goto BUGOUT;
      }

      /* Open fifos. */
      if ((fdS2C_d = open (pszS2C, O_RDONLY)) < 0)
      {
         goto BUGOUT;
      }
      if ((fdC2S_d = open (pszC2S, O_WRONLY)) < 0)
      {
         goto BUGOUT;
      }
   }
   else
   {
      /* child */
      struct sembuf sb;
      int           rc;

      sb.sem_num = 0;
      sb.sem_op  = 1;
      sb.sem_flg = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Child setting sem." << std::endl;
#endif

      rc = semop (idSem, &sb, 1);

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Child semop " << rc << std::endl;
#endif

      /* Redirect print_stream to stdout. */
      if (fdStdOut != STDOUT_FILENO)
      {
         dup2 (fdStdOut, STDOUT_FILENO);
      }
      if (fdStdErr != STDERR_FILENO)
      {
         dup2 (fdStdErr, STDERR_FILENO);
      }

      if (execlp (pszClientExe_d, pszClientExe_d, (char *)0, (char *)0) < 0)
      {
         std::cerr << "Error: Cannot run " << pszClientExe_d << std::endl;

	      goto BUGOUT2;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": execlp succeded." << std::endl;
#endif

      exit (0);

BUGOUT2:
      exit (EXIT_FAILURE);
   }

   pCmd_d = new PrinterCommand ("OmniPDCProxy");

   if (!pCmd_d)
   {
      goto BUGOUT;
   }

   // Initialize the session
   if (  !pCmd_d->setCommand (PDCCMD_INITIALIZE_SESSION, PDC_VERSION)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Failed to initialize the session!" << std::endl;

      goto BUGOUT;
   }

   // Tell the server what the device name will be
   if (  !pCmd_d->setCommand (PDCCMD_SET_DEVICE_NAME, pszDeviceName)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Failed to set OmniPDCProxy device name!" << std::endl;

      goto BUGOUT;
   }

   if (fAdvanced_d)
   {
      // Tell the server to used advanced bitblt mode.  This is optional
      pCmd_d->setCommand (PDCCMD_MODE_IS_RENDERER, true);
      pCmd_d->sendCommand (fdC2S_d);
      pCmd_d->readCommand (fdS2C_d);
   }

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      // If there are job properties, pass them in
      if (  !pCmd_d->setCommand (PDCCMD_SET_JOB_PROPERTIES, pszJobProperties)
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Failed to set OmniPDCProxy job properties!" << std::endl;

         goto BUGOUT;
      }
   }

   // Tell the server to create a device instance
   if (  !pCmd_d->setCommand (PDCCMD_NEW_DEVICE)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": Failed to create a new device instance!" << std::endl;

      goto BUGOUT;
   }

   fSucceeded = true;

BUGOUT:
   if (-1 != idSem)
   {
      semctl (idSem, 0, IPC_RMID, 0);
   }

   if (!fSucceeded)
   {
#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": failed!" << std::endl;
#endif

      // @TBD throw error?
      fHasError_d = true;

      cleanupInstance ();
      return;
   }
}

void OmniPDCProxy::
cleanupInstance ()
{
   PSZRO pszS2C = 0;
   PSZRO pszC2S = 0;
   int   rc;

   // End the connection
   if (pCmd_d)
   {
      if (pCmd_d->setCommand (PDCCMD_CLOSE_SESSION))
      {
         pCmd_d->sendCommand (fdC2S_d);
      }

      delete pCmd_d;
      pCmd_d = 0;
   }

   if (pszClientExe_d)
   {
      free (pszClientExe_d);
      pszClientExe_d = 0;
   }

   pszS2C = getenv (vpszServerToClient);
   pszC2S = getenv (vpszClientToServer);

   // Clean up
   if (-1 != fdS2C_d)
   {
      rc = close (fdS2C_d);
      fdS2C_d = -1;
   }

   if (-1 != fdC2S_d)
   {
      rc = close (fdC2S_d);
      fdC2S_d = -1;
   }

   if (pszS2C)
   {
      rc = remove (pszS2C);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": remove (" << pszS2C << ") = " << errno << std::endl;
      }
   }
   if (pszC2S)
   {
      rc = remove (pszC2S);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": remove (" << pszC2S << ") = " << errno << std::endl;
      }
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

   if (pszVersion_d)
   {
      free (pszVersion_d);
      pszVersion_d = 0;
   }
   if (pszDriverName_d)
   {
      free (pszDriverName_d);
      pszDriverName_d = 0;
   }
   if (pszDeviceName_d)
   {
      free (pszDeviceName_d);
      pszDeviceName_d = 0;
   }
   if (pszShortName_d)
   {
      free (pszShortName_d);
      pszShortName_d = 0;
   }
   if (pszLibraryName_d)
   {
      free (pszLibraryName_d);
      pszLibraryName_d = 0;
   }

   iLanguageID_d = 0;
   delete pLanguage_d; pLanguage_d = 0;

   if (pszDitherID_d)
   {
      free (pszDitherID_d);
      pszDitherID_d = 0;
   }
#ifdef INCLUDE_JP_UPDF_BOOKLET
   delete pBooklet_d; pBooklet_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   delete pCopies_d; pCopies_d = 0;
#endif
   delete pForm_d; pForm_d = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   delete pJogging_d; pJogging_d = 0;
#endif
   delete pMedia_d; pMedia_d = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   delete pNUp_d; pNUp_d = 0;
#endif
   delete pOrientation_d; pOrientation_d = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   delete pOutputBin_d; pOutputBin_d = 0;
#endif
   delete pPrintMode_d; pPrintMode_d = 0;
   delete pResolution_d; pResolution_d = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   delete pScaling_d; pScaling_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   delete pSheetCollate_d; pSheetCollate_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   delete pSide_d; pSide_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   delete pStitching_d; pStitching_d = 0;
#endif
   delete pTray_d; pTray_d = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   delete pTrimming_d; pTrimming_d = 0;
#endif
}

OmniPDCProxy::
~OmniPDCProxy ()
{
   cleanupInstance ();

   fHasError_d = false;
}

bool OmniPDCProxy::
hasError ()
{
   return fHasError_d;
}

void OmniPDCProxy::
initialize ()
{
}

PSZCRO OmniPDCProxy::
getVersion ()
{
   if (!pszVersion_d)
   {
      // Query the version
      if (  pCmd_d->setCommand (PDCCMD_GET_VERSION)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszVersion_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszVersion_d)
            {
               strcpy (pszVersion_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_VERSION failed!" << std::endl;
      }
   }

   return pszVersion_d;
}

PSZCRO OmniPDCProxy::
getDriverName ()
{
   if (!pszDriverName_d)
   {
      // Query the driver name
      if (  pCmd_d->setCommand (PDCCMD_GET_DRIVER_NAME)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszDriverName_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszDriverName_d)
            {
               strcpy (pszDriverName_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_DRIVER_NAME failed!" << std::endl;
      }
   }

   return pszDriverName_d;
}

PSZCRO OmniPDCProxy::
getDeviceName ()
{
   if (!pszDeviceName_d)
   {
      // Query the device name
      if (  pCmd_d->setCommand (PDCCMD_GET_DEVICE_NAME)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszDeviceName_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszDeviceName_d)
            {
               strcpy (pszDeviceName_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_DEVICE_NAME failed!" << std::endl;
      }
   }

   return pszDeviceName_d;
}

PSZCRO OmniPDCProxy::
getShortName ()
{
   if (!pszShortName_d)
   {
      // Query the device short name
      if (  pCmd_d->setCommand (PDCCMD_GET_SHORT_NAME)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszShortName_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszShortName_d)
            {
               strcpy (pszShortName_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_SHORT_NAME failed!" << std::endl;
      }
   }

   return pszShortName_d;
}

PSZCRO OmniPDCProxy::
getLibraryName ()
{
   if (!pszLibraryName_d)
   {
      // Query the library name
      if (  pCmd_d->setCommand (PDCCMD_GET_LIBRARY_NAME)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszLibraryName_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszLibraryName_d)
            {
               strcpy (pszLibraryName_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_LIBRARY_NAME failed!" << std::endl;
      }
   }

   return pszLibraryName_d;
}

EOMNICLASS OmniPDCProxy::
getOmniClass ()
{
   if (eOmniClass_d == OMNI_CLASS_UNKNOWN)
   {
      // Query the omni class
      if (  pCmd_d->setCommand (PDCCMD_GET_OMNI_CLASS)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
          pCmd_d->getCommandInt ((int&)eOmniClass_d);
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_GET_OMNI_CLASS failed!" << std::endl;
      }
   }

   return eOmniClass_d;
}

class CmdArrayEnumeration : public Enumeration
{
public:
            CmdArrayEnumeration (bool            fValid,
                                 Device         *pDevice,
                                 PrinterCommand *pCmd);
   virtual ~CmdArrayEnumeration ();
   bool     hasMoreElements     ();
   void    *nextElement         ();

private:
   bool              fValid_d;
   Device           *pDevice_d;
   PSZ               pszAllocated_d;
   PSZ               pszCurrent_d;
   int               cbLength_d;
};

CmdArrayEnumeration::
CmdArrayEnumeration (bool            fValid,
                     Device         *pDevice,
                     PrinterCommand *pCmd)
{
   fValid_d       = fValid;
   pDevice_d      = pDevice;
   pszAllocated_d = 0;
   pszCurrent_d   = 0;
   cbLength_d     = 0;

   if (  pCmd
      && 0 < pCmd->getCommandLength ()
      && pCmd->getCommandString (false)
      && *pCmd->getCommandString (false)
      )
   {
      cbLength_d     = pCmd->getCommandLength ();
      pszAllocated_d = (PSZ)malloc (cbLength_d);
      pszCurrent_d   = pszAllocated_d;

      if (pszAllocated_d)
      {
         memcpy (pszAllocated_d, pCmd->getCommandString (false), cbLength_d);
      }
   }
}

CmdArrayEnumeration::
~CmdArrayEnumeration ()
{
   if (pszAllocated_d)
   {
      free ((void *)pszAllocated_d);
   }
}

bool CmdArrayEnumeration::
hasMoreElements ()
{
   if (  !fValid_d
      || 0 == cbLength_d
      )
   {
      return false;
   }

   return true;
}

void * CmdArrayEnumeration::
nextElement ()
{
   void *pvRet = 0;

   if (cbLength_d)
   {
      pvRet = new JobProperties (pszCurrent_d);

      int iLength = strlen (pszCurrent_d) + 1;

      cbLength_d   -= iLength;
      pszCurrent_d += iLength;

      if (1 == cbLength_d)
      {
         cbLength_d   = 0;
         pszCurrent_d = 0;
      }
   }

   return pvRet;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET

class OmniPDCProxyBooklet : public DeviceBooklet
{
public:
   OmniPDCProxyBooklet (Device         *pDevice,
                        PSZCRO          pszJobProperties,
                        BinaryData     *pbdData,
                        PrinterCommand *pCmd,
                        int             fdC2S,
                        int             fdS2C)
      : DeviceBooklet (pDevice,
                       pszJobProperties,
                       pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_BOOKLET_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_BOOKLETS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceBooklet *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the booklet of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_BOOKLET, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyBooklet:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceBooklet *pBooklet;

         pBooklet = new OmniPDCProxyBooklet (pDevice,
                                             pszJP,
                                             0,
                                             pCmd,
                                             fdC2S,
                                             fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pBooklet)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyBooklet:" << __FUNCTION__ << ": pBooklet = " << *pBooklet << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyBooklet:" << __FUNCTION__ << ": pBooklet = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pBooklet;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_BOOKLET failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceBooklet *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyBooklet:"
          << DeviceBooklet::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&              os,
               const OmniPDCProxyBooklet& const_self)
   {
      OmniPDCProxyBooklet& self = const_cast<OmniPDCProxyBooklet&>(const_self);
      std::ostringstream   oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceBooklet * OmniPDCProxy::
getCurrentBooklet ()
{
   if (!pBooklet_d)
   {
      // Query the booklet of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_BOOKLET)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pBooklet_d = OmniPDCProxyBooklet::createS (this,
                                                       pszJobProperties,
                                                       pCmd_d,
                                                       fdC2S_d,
                                                       fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pBooklet_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_COPIES

class OmniPDCProxyCopies : public DeviceCopies
{
public:
   OmniPDCProxyCopies (Device         *pDevice,
                       PSZCRO          pszJobProperties,
                       BinaryData     *pbdData,
                       int             iMinimum,
                       int             iMaximum,
                       bool            fSimulationRequired,
                       PrinterCommand *pCmd,
                       int             fdC2S,
                       int             fdS2C)
      : DeviceCopies (pDevice,
                      pszJobProperties,
                      pbdData,
                      iMinimum,
                      iMaximum,
                      fSimulationRequired)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_COPIES_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the copies of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_COPIES, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceCopies *
   createS (Device         *pDevice,
            PSZCRO          pszJobProperties,
            PrinterCommand *pCmd,
            int             fdC2S,
            int             fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the copies of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_COPIES, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace            = 0;
         PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
         PSZRO   pszJP               = 0;
         int     iMinimum            = 0;
         int     iMaximum            = 0;
         int     iSimulationRequired = 0;
         bool    fSimulationRequired = false;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d %d",
                 &iMinimum,
                 &iMaximum,
                 &iSimulationRequired);

         if (iSimulationRequired)
         {
            fSimulationRequired = true;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": pszJP               = " << pszJP << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": iMinimum            = " << iMinimum << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": iMaximum            = " << iMaximum << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
         }
#endif

         DeviceCopies *pCopies;

         pCopies = new OmniPDCProxyCopies (pDevice,
                                           pszJP,
                                           0,
                                           iMinimum,
                                           iMaximum,
                                           fSimulationRequired,
                                           pCmd,
                                           fdC2S,
                                           fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pCopies)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": pCopies = " << *pCopies << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": pCopies = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pCopies;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_COPIES failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceCopies *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyCopies:"
          << DeviceCopies::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&             os,
               const OmniPDCProxyCopies& const_self)
   {
      OmniPDCProxyCopies& self = const_cast<OmniPDCProxyCopies&>(const_self);
      std::ostringstream  oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceCopies * OmniPDCProxy::
getCurrentCopies ()
{
   if (!pCopies_d)
   {
      // Query the copies of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_COPIES)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pCopies_d = OmniPDCProxyCopies::createS (this,
                                                     pszJobProperties,
                                                     pCmd_d,
                                                     fdC2S_d,
                                                     fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pCopies_d;
}

#endif

PSZCRO OmniPDCProxy::
getCurrentDitherID ()
{
   if (!pszDitherID_d)
   {
      // Query the dither of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_DITHER_ID)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszResponse = pCmd_d->getCommandString (false);

         if (  pszResponse
            && *pszResponse
            )
         {
            pszDitherID_d = (char *)malloc (strlen (pszResponse) + 1);

            if (pszDitherID_d)
            {
               strcpy (pszDitherID_d, pszResponse);
            }
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_DITHER_ID failed!" << std::endl;
      }
   }

   return pszDitherID_d;
}

class OmniPDCFormEnumeration : public Enumeration
{
public:
            OmniPDCFormEnumeration (bool              fValid,
                                    Device           *pDevice,
                                    DeviceResolution *pResolution,
                                    PrinterCommand   *pCmd);
   virtual ~OmniPDCFormEnumeration ();
   bool     hasMoreElements        ();
   void    *nextElement            ();

private:
   bool              fValid_d;
   Device           *pDevice_d;
   DeviceResolution *pResolution_d;
   PSZ               pszAllocated_d;
   PSZ               pszCurrent_d;
   int               cbLength_d;
};

OmniPDCFormEnumeration::
OmniPDCFormEnumeration (bool              fValid,
                        Device           *pDevice,
                        DeviceResolution *pResolution,
                        PrinterCommand   *pCmd)
{
   fValid_d       = fValid;
   pDevice_d      = pDevice;
   pResolution_d  = pResolution;
   pszAllocated_d = 0;
   pszCurrent_d   = 0;
   cbLength_d     = 0;

   if (  pCmd
      && 0 < pCmd->getCommandLength ()
      && pCmd->getCommandString (false)
      && *pCmd->getCommandString (false)
      )
   {
      cbLength_d     = pCmd->getCommandLength ();
      pszAllocated_d = (PSZ)malloc (cbLength_d);
      pszCurrent_d   = pszAllocated_d;

      if (pszAllocated_d)
      {
         memcpy (pszAllocated_d, pCmd->getCommandString (false), cbLength_d);

/////////std::cerr << "cbLength_d = " << cbLength_d << ", {";
/////////
/////////for (int i = 0; i < cbLength_d; i++)
/////////{
/////////   if (isprint (pszAllocated_d[i]))
/////////   {
/////////      std::cerr << '\'' << pszAllocated_d[i] << '\'';
/////////   }
/////////   else
/////////   {
/////////      std::cerr << "0x" << std::hex;
/////////      std::cerr.width (2);
/////////      std::cerr.fill ('0');
/////////      std::cerr << (int)pszAllocated_d[i]
/////////                << std::dec;
/////////   }
/////////
/////////   if (i < cbLength_d - 1)
/////////      std::cerr << ",";
/////////}
/////////
/////////std::cerr << "}" << std::endl;
      }
   }
}

OmniPDCFormEnumeration::
~OmniPDCFormEnumeration ()
{
   if (pszAllocated_d)
   {
      free ((void *)pszAllocated_d);
   }
}

bool OmniPDCFormEnumeration::
hasMoreElements ()
{
   if (  !fValid_d
      || 0 == cbLength_d
      )
   {
      return false;
   }

   return true;
}

void * OmniPDCFormEnumeration::
nextElement ()
{
   void *pvRet = 0;

   if (cbLength_d)
   {
      pvRet = new JobProperties (pszCurrent_d);

      int iLength = strlen (pszCurrent_d) + 1;

      cbLength_d   -= iLength;
      pszCurrent_d += iLength;

      if (1 == cbLength_d)
      {
         cbLength_d   = 0;
         pszCurrent_d = 0;
      }
   }

   return pvRet;
}

class OmniPDCProxyForm : public DeviceForm
{
public:
   OmniPDCProxyForm (Device           *pDevice,
                     PSZCRO            pszJobProperties,
                     int               iCapabilities,
                     BinaryData       *pbdData,
                     HardCopyCap      *hcInfo,
                     DeviceResolution *pResolution,
                     PrinterCommand   *pCmd,
                     int               fdC2S,
                     int               fdS2C)
      : DeviceForm (pDevice,
                    pszJobProperties,
                    iCapabilities,
                    pbdData,
                    hcInfo)
   {
      pDevice_d     = pDevice;
      pResolution_d = pResolution;
      pCmd_d        = pCmd;
      fdC2S_d       = fdC2S;
      fdS2C_d       = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_FORM_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the forms of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_FORMS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new OmniPDCFormEnumeration (fSuccess, pDevice_d, pResolution_d, pCmd_d);
   }

   static DeviceForm *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            DeviceResolution *pResolution,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the form of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_FORM, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace      = 0;
         PSZCRO  pszJPQuoted   = pCmd->getCommandString (false);
         PSZRO   pszJP         = 0;
         int     iCapabilities = 0;
         int     iLeftClip     = 0;
         int     iTopClip      = 0;
         int     iRightClip    = 0;
         int     iBottomClip   = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d %d %d %d",
                 &iCapabilities,
                 &iLeftClip,
                 &iTopClip,
                 &iRightClip,
                 &iBottomClip);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": pszJP            = " << pszJP            << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": iCapabilities    = " << iCapabilities    << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": iLeftClip        = " << iLeftClip        << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": iTopClip         = " << iTopClip         << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": iRightClip       = " << iRightClip       << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": iBottomClip      = " << iBottomClip      << std::endl;
         }
#endif

         DeviceForm *pForm;

         pForm = new OmniPDCProxyForm (pDevice,
                                       pszJP,
                                       iCapabilities,
                                       0,
                                       new HardCopyCap (iLeftClip,
                                                        iTopClip,
                                                        iRightClip,
                                                        iBottomClip),
                                       pResolution,
                                       pCmd,
                                       fdC2S,
                                       fdS2C);

         if (pForm)
         {
            pForm->associateWith (pResolution);
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pForm)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": pForm = " << *pForm << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyForm:" << __FUNCTION__ << ": pForm = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pForm;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_FORM failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceForm *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pResolution_d,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyForm:"
          << DeviceForm::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&           os,
               const OmniPDCProxyForm& const_self)
   {
      OmniPDCProxyForm&  self = const_cast<OmniPDCProxyForm&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   DeviceResolution *pResolution_d;
   PrinterCommand   *pCmd_d;
   int               fdC2S_d;
   int               fdS2C_d;
};

DeviceForm * OmniPDCProxy::
getCurrentForm ()
{
   if (!pForm_d)
   {
      // Query the form of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_FORM)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pForm_d = OmniPDCProxyForm::createS (this,
                                                 pszJobProperties,
                                                 getCurrentResolution (),
                                                 pCmd_d,
                                                 fdC2S_d,
                                                 fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pForm_d;
}

#ifdef INCLUDE_JP_UPDF_JOGGING

class OmniPDCProxyJogging : public DeviceJogging
{
public:
   OmniPDCProxyJogging (Device         *pDevice,
                        PSZCRO          pszJobProperties,
                        BinaryData     *pbdData,
                        PrinterCommand *pCmd,
                        int             fdC2S,
                        int             fdS2C)
      : DeviceJogging (pDevice,
                       pszJobProperties,
                       pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_JOGGING_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_JOGGINGS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceJogging *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the jogging of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_JOGGING, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyJogging:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceJogging *pJogging;

         pJogging = new OmniPDCProxyJogging (pDevice,
                                             pszJP,
                                             0,
                                             pCmd,
                                             fdC2S,
                                             fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pJogging)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyJogging:" << __FUNCTION__ << ": pJogging = " << *pJogging << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyJogging:" << __FUNCTION__ << ": pJogging = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pJogging;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_JOGGING failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceJogging *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyJogging:"
          << DeviceJogging::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&              os,
               const OmniPDCProxyJogging& const_self)
   {
      OmniPDCProxyJogging& self = const_cast<OmniPDCProxyJogging&>(const_self);
      std::ostringstream   oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceJogging * OmniPDCProxy::
getCurrentJogging ()
{
   if (!pJogging_d)
   {
      // Query the jogging of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_JOGGING)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pJogging_d = OmniPDCProxyJogging::createS (this,
                                                       pszJobProperties,
                                                       pCmd_d,
                                                       fdC2S_d,
                                                       fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pJogging_d;
}

#endif

class OmniPDCProxyMedia : public DeviceMedia
{
public:
   OmniPDCProxyMedia (Device         *pDevice,
                      PSZCRO          pszJobProperties,
                      BinaryData     *pbdData,
                      int             iColorAdjustRequired,
                      int             iAbsorption,
                      PrinterCommand *pCmd,
                      int             fdC2S,
                      int             fdS2C)
      : DeviceMedia (pDevice,
                     pszJobProperties,
                     pbdData,
                     iColorAdjustRequired,
                     iAbsorption)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_MEDIA_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the medias of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_MEDIAS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceMedia *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the media of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_MEDIA, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace             = 0;
         PSZCRO  pszJPQuoted          = pCmd->getCommandString (false);
         PSZRO   pszJP                = 0;
         int     iColorAdjustRequired = 0;
         int     iAbsorption          = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d",
                 &iColorAdjustRequired,
                 &iAbsorption);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyMedia:" << __FUNCTION__ << ": pszJP                = " << pszJP                << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyMedia:" << __FUNCTION__ << ": iColorAdjustRequired = " << iColorAdjustRequired << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyMedia:" << __FUNCTION__ << ": iAbsorption          = " << iAbsorption          << std::endl;
         }
#endif

         DeviceMedia *pMedia;

         pMedia = new OmniPDCProxyMedia (pDevice,
                                         pszJP,
                                         0,
                                         iColorAdjustRequired,
                                         iAbsorption,
                                         pCmd,
                                         fdC2S,
                                         fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pMedia)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyMedia:" << __FUNCTION__ << ": pMedia = " << *pMedia << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyMedia:" << __FUNCTION__ << ": pMedia = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pMedia;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_MEDIA failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceMedia *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyMedia:"
          << DeviceMedia::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&            os,
               const OmniPDCProxyMedia& const_self)
   {
      OmniPDCProxyMedia& self = const_cast<OmniPDCProxyMedia&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceMedia * OmniPDCProxy::
getCurrentMedia ()
{
   if (!pMedia_d)
   {
      // Query the media of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_MEDIA)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pMedia_d = OmniPDCProxyMedia::createS (this,
                                                   pszJobProperties,
                                                   pCmd_d,
                                                   fdC2S_d,
                                                   fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pMedia_d;
}

#ifdef INCLUDE_JP_COMMON_NUP

class OmniPDCProxyNUp : public DeviceNUp
{
public:
   OmniPDCProxyNUp (Device         *pDevice,
                    PSZCRO          pszJobProperties,
                    BinaryData     *pbdData,
                    bool            fSimulationRequired,
                    PrinterCommand *pCmd,
                    int             fdC2S,
                    int             fdS2C)
      : DeviceNUp (pDevice,
                   pszJobProperties,
                   pbdData,
                   fSimulationRequired)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_NUP_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the medias of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_NUPS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceNUp *
   createS (Device         *pDevice,
            PSZCRO          pszJobProperties,
            PrinterCommand *pCmd,
            int             fdC2S,
            int             fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the nup of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_NUP, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace            = 0;
         PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
         PSZRO   pszJP               = 0;
         int     iSimulationRequired = 0;
         bool    fSimulationRequired = false;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d",
                 &iSimulationRequired);

         if (iSimulationRequired)
         {
            fSimulationRequired = true;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyNUp:" << __FUNCTION__ << ": pszJP               = " << pszJP               << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyNUp:" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
         }
#endif

         DeviceNUp *pNUp;

         pNUp = new OmniPDCProxyNUp (pDevice,
                                     pszJP,
                                     0,
                                     fSimulationRequired,
                                     pCmd,
                                     fdC2S,
                                     fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pNUp)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyNUp:" << __FUNCTION__ << ": pNUp = " << *pNUp << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyNUp:" << __FUNCTION__ << ": pNUp = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pNUp;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_MEDIA failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceNUp *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyNUp:"
          << DeviceNUp::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&          os,
               const OmniPDCProxyNUp& const_self)
   {
      OmniPDCProxyNUp&   self = const_cast<OmniPDCProxyNUp&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceNUp * OmniPDCProxy::
getCurrentNUp ()
{
   if (!pNUp_d)
   {
      // Query the nup of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_NUP)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pNUp_d = OmniPDCProxyNUp::createS (this,
                                               pszJobProperties,
                                               pCmd_d,
                                               fdC2S_d,
                                               fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pNUp_d;
}

#endif

class OmniPDCProxyOrientation : public DeviceOrientation
{
public:
   OmniPDCProxyOrientation (Device           *pDevice,
                            PSZCRO            pszJobProperties,
                            bool              fSimulationRequired,
                            PrinterCommand   *pCmd,
                            int               fdC2S,
                            int               fdS2C)
      : DeviceOrientation (pDevice,
                           pszJobProperties,
                           fSimulationRequired)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_ORIENTATION_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the orientations of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_ORIENTATIONS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceOrientation *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the orientation of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_ORIENTATION, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace            = 0;
         PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
         PSZRO   pszJP               = 0;
         int     iSimulationRequired = 0;
         bool    fSimulationRequired = false;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d",
                 &iSimulationRequired);

         if (iSimulationRequired)
         {
            fSimulationRequired = true;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": pszJP               = " << pszJP << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyCopies:" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
         }
#endif

         DeviceOrientation *pOrientation;

         pOrientation = new OmniPDCProxyOrientation (pDevice,
                                                     pszJP,
                                                     fSimulationRequired,
                                                     pCmd,
                                                     fdC2S,
                                                     fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pOrientation)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pOrientation = " << *pOrientation << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pOrientation = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pOrientation;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_ORIENTATION failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceOrientation *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyOrientation:"
          << DeviceOrientation::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                  os,
               const OmniPDCProxyOrientation& const_self)
   {
      OmniPDCProxyOrientation& self = const_cast<OmniPDCProxyOrientation&>(const_self);
      std::ostringstream       oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand   *pCmd_d;
   int               fdC2S_d;
   int               fdS2C_d;
};

DeviceOrientation * OmniPDCProxy::
getCurrentOrientation ()
{
   if (!pOrientation_d)
   {
      // Query the orientation of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_ORIENTATION)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pOrientation_d = OmniPDCProxyOrientation::createS (this,
                                                               pszJobProperties,
                                                               pCmd_d,
                                                               fdC2S_d,
                                                               fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pOrientation_d;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

class OmniPDCProxyOutputBin : public DeviceOutputBin
{
public:
   OmniPDCProxyOutputBin (Device         *pDevice,
                          PSZCRO          pszJobProperties,
                          BinaryData     *pbdData,
                          PrinterCommand *pCmd,
                          int             fdC2S,
                          int             fdS2C)
      : DeviceOutputBin (pDevice,
                         pszJobProperties,
                         pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_OUTPUT_BIN_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_OUTPUT_BINS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceOutputBin *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the outputbin of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_OUTPUT_BIN, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyOutputBin:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceOutputBin *pOutputBin;

         pOutputBin = new OmniPDCProxyOutputBin (pDevice,
                                                 pszJP,
                                                 0,
                                                 pCmd,
                                                 fdC2S,
                                                 fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pOutputBin)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyOutputBin:" << __FUNCTION__ << ": pOutputBin = " << *pOutputBin << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyOutputBin:" << __FUNCTION__ << ": pOutputBin = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pOutputBin;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_OUTPUT_BIN failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceOutputBin *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyOutputBin:"
          << DeviceOutputBin::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                os,
               const OmniPDCProxyOutputBin& const_self)
   {
      OmniPDCProxyOutputBin& self = const_cast<OmniPDCProxyOutputBin&>(const_self);
      std::ostringstream     oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceOutputBin * OmniPDCProxy::
getCurrentOutputBin ()
{
   if (!pOutputBin_d)
   {
      // Query the outputbin of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_OUTPUT_BIN)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pOutputBin_d = OmniPDCProxyOutputBin::createS (this,
                                                           pszJobProperties,
                                                           pCmd_d,
                                                           fdC2S_d,
                                                           fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pOutputBin_d;
}

#endif

class OmniPDCProxyPrintMode : public DevicePrintMode
{
public:
   OmniPDCProxyPrintMode (Device         *pDevice,
                          PSZCRO          pszJobProperties,
                          int             iPhysicalCount,
                          int             iLogicalCount,
                          int             iPlanes,
                          PrinterCommand *pCmd,
                          int             fdC2S,
                          int             fdS2C)
      : DevicePrintMode (pDevice,
                         pszJobProperties,
                         iPhysicalCount,
                         iLogicalCount,
                         iPlanes)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_PRINT_MODE_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the print modes of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_PRINT_MODES, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DevicePrintMode *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the print mode of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_PRINT_MODE, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace       = 0;
         PSZCRO  pszJPQuoted    = pCmd->getCommandString (false);
         PSZRO   pszJP          = 0;
         int     iPhysicalCount = 0;
         int     iLogicalCount  = 0;
         int     iPlanes        = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d %d",
                 &iPhysicalCount,
                 &iLogicalCount,
                 &iPlanes);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": pszJP                  = " << pszJP          << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": iPhysicalCount         = " << iPhysicalCount << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": iLogicalCount          = " << iLogicalCount  << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": iPlanes                = " << iPlanes        << std::endl;
         }
#endif

         DevicePrintMode *pPrintMode;

         pPrintMode = new OmniPDCProxyPrintMode (pDevice,
                                                 pszJP,
                                                 iPhysicalCount,
                                                 iLogicalCount,
                                                 iPlanes,
                                                 pCmd,
                                                 fdC2S,
                                                 fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pPrintMode)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": pPrintMode = " << *pPrintMode << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyPrintMode:" << __FUNCTION__ << ": pPrintMode = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pPrintMode;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_PRINT_MODE failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DevicePrintMode *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyPrintMode:"
          << DevicePrintMode::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                os,
               const OmniPDCProxyPrintMode& const_self)
   {
      OmniPDCProxyPrintMode& self = const_cast<OmniPDCProxyPrintMode&>(const_self);
      std::ostringstream     oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DevicePrintMode * OmniPDCProxy::
getCurrentPrintMode ()
{
   if (!pPrintMode_d)
   {
      // Query the printmode of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_PRINT_MODE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pPrintMode_d = OmniPDCProxyPrintMode::createS (this,
                                                           pszJobProperties,
                                                           pCmd_d,
                                                           fdC2S_d,
                                                           fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pPrintMode_d;
}

class OmniPDCProxyResolution : public DeviceResolution
{
public:
   OmniPDCProxyResolution (Device         *pDevice,
                           PSZCRO          pszJobProperties,
                           int             iXInternalRes,
                           int             iYInternalRes,
                           BinaryData     *pbdData,
                           int             iCapabilities,
                           int             iDestinationBitsPerPel,
                           int             iScanlineMultiple,
                           PrinterCommand *pCmd,
                           int             fdC2S,
                           int             fdS2C)
      : DeviceResolution (pDevice,
                          pszJobProperties,
                          iXInternalRes,
                          iYInternalRes,
                          pbdData,
                          iCapabilities,
                          iDestinationBitsPerPel,
                          iScanlineMultiple)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_RESOLUTION_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the resolutions of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_RESOLUTIONS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceResolution *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the resolution of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_RESOLUTION, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace               = 0;
         PSZCRO  pszJPQuoted            = pCmd->getCommandString (false);
         PSZRO   pszJP                  = 0;
         int     iXRes                  = 0;
         int     iYRes                  = 0;
         int     iXInternalRes          = 0;
         int     iYInternalRes          = 0;
         int     iCapabilities          = 0;
         int     iDestinationBitsPerPel = 0;
         int     iScanlineMultiple      = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d %d %d %d %d %d",
                 &iXRes,
                 &iYRes,
                 &iXInternalRes,
                 &iYInternalRes,
                 &iCapabilities,
                 &iDestinationBitsPerPel,
                 &iScanlineMultiple);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": pszJP                  = " << pszJP                  << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": iXInternalRes          = " << iXInternalRes          << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": iYInternalRes          = " << iYInternalRes          << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": iCapabilities          = " << iCapabilities          << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": iDestinationBitsPerPel = " << iDestinationBitsPerPel << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": iScanlineMultiple      = " << iScanlineMultiple      << std::endl;
         }
#endif

         DeviceResolution *pResolution;

         pResolution = new OmniPDCProxyResolution (pDevice,
                                                   pszJP,
                                                   iXInternalRes,
                                                   iYInternalRes,
                                                   0,
                                                   iCapabilities,
                                                   iDestinationBitsPerPel,
                                                   iScanlineMultiple,
                                                   pCmd,
                                                   fdC2S,
                                                   fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pResolution)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": pResolution = " << *pResolution << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyResolution:" << __FUNCTION__ << ": pResolution = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pResolution;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_RESOLUTION failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceResolution *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyResolution:"
          << DeviceResolution::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                 os,
               const OmniPDCProxyResolution& const_self)
   {
      OmniPDCProxyResolution& self = const_cast<OmniPDCProxyResolution&>(const_self);
      std::ostringstream      oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceResolution * OmniPDCProxy::
getCurrentResolution ()
{
   if (!pResolution_d)
   {
      // Query the resolution of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_RESOLUTION)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pResolution_d = OmniPDCProxyResolution::createS (this,
                                                             pszJobProperties,
                                                             pCmd_d,
                                                             fdC2S_d,
                                                             fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pResolution_d;
}

#ifdef INCLUDE_JP_COMMON_SCALING

class OmniPDCProxyScaling : public DeviceScaling
{
public:
   OmniPDCProxyScaling (Device         *pDevice,
                        PSZCRO          pszJobProperties,
                        BinaryData     *pbdData,
                        int             iMinimumScale,
                        int             iMaximumScale,
                        PrinterCommand *pCmd,
                        int             fdC2S,
                        int             fdS2C)
      : DeviceScaling (pDevice,
                       pszJobProperties,
                       pbdData,
                       iMinimumScale,
                       iMaximumScale)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_SCALING_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the scalings of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_SCALINGS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceScaling *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the scaling of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_SCALING, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace      = 0;
         PSZCRO  pszJPQuoted   = pCmd->getCommandString (false);
         PSZRO   pszJP         = 0;
         int     iMinimumScale = 0;
         int     iMaximumScale = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d %d",
                 &iMinimumScale,
                 &iMaximumScale);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyScaling:" << __FUNCTION__ << ": pszJP            = " << pszJP << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyScaling:" << __FUNCTION__ << ": iMinimumScale    = " << iMinimumScale << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyScaling:" << __FUNCTION__ << ": iMaximumScale    = " << iMaximumScale << std::endl;
         }
#endif

         DeviceScaling *pScaling;

         pScaling = new OmniPDCProxyScaling (pDevice,
                                             pszJP,
                                             0,
                                             iMinimumScale,
                                             iMaximumScale,
                                             pCmd,
                                             fdC2S,
                                             fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pScaling)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyScaling:" << __FUNCTION__ << ": pScaling = " << *pScaling << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyScaling:" << __FUNCTION__ << ": pScaling = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pScaling;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_SCALING failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceScaling *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyScaling:"
          << DeviceScaling::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&              os,
               const OmniPDCProxyScaling& const_self)
   {
      OmniPDCProxyScaling& self = const_cast<OmniPDCProxyScaling&>(const_self);
      std::ostringstream   oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceScaling * OmniPDCProxy::
getCurrentScaling ()
{
   if (!pScaling_d)
   {
      // Query the scaling of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_SCALING)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pScaling_d = OmniPDCProxyScaling::createS (this,
                                                       pszJobProperties,
                                                       pCmd_d,
                                                       fdC2S_d,
                                                       fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pScaling_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

class OmniPDCProxySheetCollate : public DeviceSheetCollate
{
public:
   OmniPDCProxySheetCollate (Device         *pDevice,
                             PSZCRO          pszJobProperties,
                             BinaryData     *pbdData,
                             PrinterCommand *pCmd,
                             int             fdC2S,
                             int             fdS2C)
      : DeviceSheetCollate (pDevice,
                            pszJobProperties,
                            pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_SHEET_COLLATE_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_SHEET_COLLATES, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceSheetCollate *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the sheetcollate of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_SHEET_COLLATE, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxySheetCollate:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceSheetCollate *pSheetCollate;

         pSheetCollate = new OmniPDCProxySheetCollate (pDevice,
                                                       pszJP,
                                                       0,
                                                       pCmd,
                                                       fdC2S,
                                                       fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pSheetCollate)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxySheetCollate:" << __FUNCTION__ << ": pSheetCollate = " << *pSheetCollate << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxySheetCollate:" << __FUNCTION__ << ": pSheetCollate = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pSheetCollate;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_SHEET_COLLATE failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceSheetCollate *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxySheetCollate:"
          << DeviceSheetCollate::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                   os,
               const OmniPDCProxySheetCollate& const_self)
   {
      OmniPDCProxySheetCollate& self = const_cast<OmniPDCProxySheetCollate&>(const_self);
      std::ostringstream        oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceSheetCollate * OmniPDCProxy::
getCurrentSheetCollate ()
{
   if (!pSheetCollate_d)
   {
      // Query the sheetcollate of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_SHEET_COLLATE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pSheetCollate_d = OmniPDCProxySheetCollate::createS (this,
                                                                 pszJobProperties,
                                                                 pCmd_d,
                                                                 fdC2S_d,
                                                                 fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pSheetCollate_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

class OmniPDCProxySide : public DeviceSide
{
public:
   OmniPDCProxySide (Device         *pDevice,
                     PSZCRO          pszJobProperties,
                     BinaryData     *pbdData,
                     bool            fSimulationRequired,
                     PrinterCommand *pCmd,
                     int             fdC2S,
                     int             fdS2C)
      : DeviceSide (pDevice,
                    pszJobProperties,
                    pbdData,
                    fSimulationRequired)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_SIDE_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_SIDES, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceSide *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the side of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_SIDE, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace            = 0;
         PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
         PSZRO   pszJP               = 0;
         int     iSimulationRequired = 0;
         bool    fSimulationRequired = false;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d",
                 &iSimulationRequired);

         if (iSimulationRequired)
         {
            fSimulationRequired = true;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxySide:" << __FUNCTION__ << ": pszJP               = " << pszJP << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxySide:" << __FUNCTION__ << ": fSimulationRequired = " << fSimulationRequired << std::endl;
         }
#endif

         DeviceSide *pSide;

         pSide = new OmniPDCProxySide (pDevice,
                                       pszJP,
                                       0,
                                       fSimulationRequired,
                                       pCmd,
                                       fdC2S,
                                       fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pSide)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxySide:" << __FUNCTION__ << ": pSide = " << *pSide << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxySide:" << __FUNCTION__ << ": pSide = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pSide;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_SIDE failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceSide *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxySide:"
          << DeviceSide::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&           os,
               const OmniPDCProxySide& const_self)
   {
      OmniPDCProxySide&  self = const_cast<OmniPDCProxySide&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceSide * OmniPDCProxy::
getCurrentSide ()
{
   if (!pSide_d)
   {
      // Query the side of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_SIDE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pSide_d = OmniPDCProxySide::createS (this,
                                                 pszJobProperties,
                                                 pCmd_d,
                                                 fdC2S_d,
                                                 fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pSide_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

class OmniPDCProxyStitching : public DeviceStitching
{
public:
   OmniPDCProxyStitching (Device         *pDevice,
                          PSZCRO          pszJobProperties,
                          BinaryData     *pbdData,
                          PrinterCommand *pCmd,
                          int             fdC2S,
                          int             fdS2C)
      : DeviceStitching (pDevice,
                         pszJobProperties,
                         pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_STITCHING_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_STITCHINGS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceStitching *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the stitching of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_STITCHING, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyStitching:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceStitching *pStitching;

         pStitching = new OmniPDCProxyStitching (pDevice,
                                                 pszJP,
                                                 0,
                                                 pCmd,
                                                 fdC2S,
                                                 fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pStitching)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyStitching:" << __FUNCTION__ << ": pStitching = " << *pStitching << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyStitching:" << __FUNCTION__ << ": pStitching = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pStitching;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_STITCHING failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceStitching *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyStitching:"
          << DeviceStitching::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&                os,
               const OmniPDCProxyStitching& const_self)
   {
      OmniPDCProxyStitching&  self = const_cast<OmniPDCProxyStitching&>(const_self);
      std::ostringstream      oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceStitching * OmniPDCProxy::
getCurrentStitching ()
{
   if (!pStitching_d)
   {
      // Query the stitching of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_STITCHING)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pStitching_d = OmniPDCProxyStitching::createS (this,
                                                           pszJobProperties,
                                                           pCmd_d,
                                                           fdC2S_d,
                                                           fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pStitching_d;
}

#endif

class OmniPDCProxyTray : public DeviceTray
{
public:
   OmniPDCProxyTray (Device         *pDevice,
                     PSZCRO          pszJobProperties,
                     int             iType,
                     BinaryData     *pbdData,
                     PrinterCommand *pCmd,
                     int             fdC2S,
                     int             fdS2C)
      : DeviceTray (pDevice,
                    pszJobProperties,
                    iType,
                    pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_TRAY_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_TRAYS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceTray *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the tray of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_TRAY, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         char   *pszSpace    = 0;
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;
         int     iType       = 0;

         pszSpace = const_cast<char*>(strchr (pszJPQuoted, ' '));

         if (!pszSpace)
            return 0;

         *pszSpace = '\0';

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            *pszSpace = ' ';
            return 0;
         }

         sscanf (pszSpace + 1,
                 "%d",
                 &iType);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyTray:" << __FUNCTION__ << ": pszJP            = " << pszJP << std::endl;
            DebugOutput::getErrorStream () << "OmniPDCProxyTray:" << __FUNCTION__ << ": iType            = " << iType << std::endl;
         }
#endif

         DeviceTray *pTray;

         pTray = new OmniPDCProxyTray (pDevice,
                                       pszJP,
                                       iType,
                                       0,
                                       pCmd,
                                       fdC2S,
                                       fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pTray)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyTray:" << __FUNCTION__ << ": pTray = " << *pTray << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyTray:" << __FUNCTION__ << ": pTray = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pTray;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_TRAY failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceTray *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyTray:"
          << DeviceTray::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&           os,
               const OmniPDCProxyTray& const_self)
   {
      OmniPDCProxyTray&  self = const_cast<OmniPDCProxyTray&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceTray * OmniPDCProxy::
getCurrentTray ()
{
   if (!pTray_d)
   {
      // Query the tray of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_TRAY)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZCRO pszNetworkJobProperties = pCmd_d->getCommandString (true);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;
            char  *pszSpace         = 0;

            pszSpace = const_cast<char*>(strchr (pszNetworkJobProperties, ' '));

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pTray_d = OmniPDCProxyTray::createS (this,
                                                 pszJobProperties,
                                                 pCmd_d,
                                                 fdC2S_d,
                                                 fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pTray_d;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING

class OmniPDCProxyTrimming : public DeviceTrimming
{
public:
   OmniPDCProxyTrimming (Device         *pDevice,
                         PSZCRO          pszJobProperties,
                         BinaryData     *pbdData,
                         PrinterCommand *pCmd,
                         int             fdC2S,
                         int             fdS2C)
      : DeviceTrimming (pDevice,
                        pszJobProperties,
                        pbdData)
   {
      pDevice_d = pDevice;
      pCmd_d    = pCmd;
      fdC2S_d   = fdC2S;
      fdS2C_d   = fdS2C;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      PSZRO pszNetworkJobProperties = 0;
      bool  fSuccess                = false;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      if (  pCmd_d->setCommand (PDCCMD_IS_TRIMMING_SUPPORTED, pszNetworkJobProperties)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         fSuccess = true;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return fSuccess;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      bool fSuccess = false;

      // Enumerate the trays of that device
      if (  pCmd_d->setCommand (PDCCMD_ENUM_TRIMMINGS, fInDeviceSpecific)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         fSuccess = true;

      return new CmdArrayEnumeration (fSuccess, pDevice_d, pCmd_d);
   }

   static DeviceTrimming *
   createS (Device           *pDevice,
            PSZCRO            pszJobProperties,
            PrinterCommand   *pCmd,
            int               fdC2S,
            int               fdS2C)
   {
      PSZRO pszNetworkJobProperties = 0;

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxyOrientation:" << __FUNCTION__ << ": pszJobProperties = " << SAFE_PRINT_PSZ (pszJobProperties) << std::endl;
#endif

      if (  pszJobProperties
         && *pszJobProperties
         )
      {
         pszNetworkJobProperties = Omni::quoteString (pszJobProperties);
      }

      // Query the trimming of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_TRIMMING, pszNetworkJobProperties)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         && PDCCMD_ACK == pCmd->getCommandType ()
         )
      {
         PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
         PSZRO   pszJP       = 0;

         pszJP = Omni::dequoteString (pszJPQuoted);

         if (!pszJP)
         {
            return 0;
         }

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            DebugOutput::getErrorStream () << "OmniPDCProxyTrimming:" << __FUNCTION__ << ": pszJP = " << pszJP << std::endl;
         }
#endif

         DeviceTrimming *pTrimming;

         pTrimming = new OmniPDCProxyTrimming (pDevice,
                                               pszJP,
                                               0,
                                               pCmd,
                                               fdC2S,
                                               fdS2C);

#ifndef RETAIL
         if (DebugOutput::shouldOutputOmniPDCProxy ())
         {
            if (pTrimming)
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyTrimming:" << __FUNCTION__ << ": pTrimming = " << *pTrimming << std::endl;
            }
            else
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyTrimming:" << __FUNCTION__ << ": pTrimming = NULL " << std::endl;
            }
         }
#endif

         free ((void *)pszJP);

         if (pszNetworkJobProperties)
         {
            free ((void *)pszNetworkJobProperties);
         }

         return pTrimming;
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_TRIMMING failed!" << std::endl;
      }

      if (pszNetworkJobProperties)
      {
         free ((void *)pszNetworkJobProperties);
      }

      return 0;
   }

   DeviceTrimming *
   create (Device *pDevice,
           PSZCRO  pszJobProperties)
   {
      return createS (pDevice,
                      pszJobProperties,
                      pCmd_d,
                      fdC2S_d,
                      fdS2C_d);
   }

#ifndef RETAIL

   void
   outputSelf ()
   {
      DebugOutput::getErrorStream () << *this << std::endl;
   }

#endif

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{OmniPDCProxyTrimming:"
          << DeviceTrimming::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&               os,
               const OmniPDCProxyTrimming& const_self)
   {
      OmniPDCProxyTrimming&  self = const_cast<OmniPDCProxyTrimming&>(const_self);
      std::ostringstream     oss;

      os << self.toString (oss);

      return os;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceTrimming * OmniPDCProxy::
getCurrentTrimming ()
{
   if (!pTrimming_d)
   {
      // Query the trimming of that device
      if (  pCmd_d->setCommand (PDCCMD_QUERY_CURRENT_TRIMMING)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
      {
         PSZRO pszNetworkJobProperties = pCmd_d->getCommandString (false);

         if (pszNetworkJobProperties)
         {
            PSZRO  pszJobProperties = 0;

            pszJobProperties = Omni::dequoteString (pszNetworkJobProperties);

            pTrimming_d = OmniPDCProxyTrimming::createS (this,
                                                         pszJobProperties,
                                                         pCmd_d,
                                                         fdC2S_d,
                                                         fdS2C_d);

            free ((void *)pszJobProperties);
         }
      }
   }

   return pTrimming_d;
}

#endif

class OmniPDCProxyGamma : public DeviceGamma
{
public:
   OmniPDCProxyGamma (int             iCGamma,
                      int             iMGamma,
                      int             iYGamma,
                      int             iKGamma,
                      int             iCBias,
                      int             iMBias,
                      int             iYBias,
                      int             iKBias,
                      PrinterCommand *pCmd,
                      int             fdC2S,
                      int             fdS2C)
      : DeviceGamma (iCGamma,
                     iMGamma,
                     iYGamma,
                     iKGamma,
                     iCBias,
                     iMBias,
                     iYBias,
                     iKBias)
   {
      pCmd_d  = pCmd;
      fdC2S_d = fdC2S;
      fdS2C_d = fdS2C;
   }

   static DeviceGamma *
   createS (PrinterCommand *pCmd,
            int             fdC2S,
            int             fdS2C)
   {
      // Query the gamma of that device
      if (  pCmd->setCommand (PDCCMD_QUERY_CURRENT_GAMMA)
         && pCmd->sendCommand (fdC2S)
         && pCmd->readCommand (fdS2C)
         )
      {
         if (PDCCMD_ACK == pCmd->getCommandType ())
         {
            PSZCRO pszResponse = pCmd->getCommandString (false);
            int    iCGamma     = 0;
            int    iMGamma     = 0;
            int    iYGamma     = 0;
            int    iKGamma     = 0;
            int    iCBias      = 0;
            int    iMBias      = 0;
            int    iYBias      = 0;
            int    iKBias      = 0;

            sscanf (pszResponse,
                    "%d %d %d %d %d %d %d %d",
                    &iCGamma,
                    &iMGamma,
                    &iYGamma,
                    &iKGamma,
                    &iCBias,
                    &iMBias,
                    &iYBias,
                    &iKBias);

#ifndef RETAIL
            if (DebugOutput::shouldOutputOmniPDCProxy ())
            {
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iCGamma = " << iCGamma << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iMGamma = " << iMGamma << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iYGamma = " << iYGamma << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iKGamma = " << iKGamma << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iCBias  = " << iCBias  << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iMBias  = " << iMBias  << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iYBias  = " << iYBias  << std::endl;
               DebugOutput::getErrorStream () << "OmniPDCProxyGamma:" << __FUNCTION__ << ": iKBias  = " << iKBias  << std::endl;
            }
#endif

            return new OmniPDCProxyGamma (iCGamma,
                                          iMGamma,
                                          iYGamma,
                                          iKGamma,
                                          iCBias,
                                          iMBias,
                                          iYBias,
                                          iKBias,
                                          pCmd,
                                          fdC2S,
                                          fdS2C);
         }
         else
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": creating NULL gamma..." << std::endl;
#endif

            return 0;
         }
      }
      else
      {
         DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": PDCCMD_QUERY_CURRENT_GAMMA failed!" << std::endl;
      }

      return 0;
   }

private:
   PrinterCommand *pCmd_d;
   int             fdC2S_d;
   int             fdS2C_d;
};

DeviceGamma * OmniPDCProxy::
getCurrentGamma ()
{
   if (pGamma_d)
      return pGamma_d;

   pGamma_d = OmniPDCProxyGamma::createS (pCmd_d,
                                          fdC2S_d,
                                          fdS2C_d);

   return pGamma_d;
}

DeviceInstance * OmniPDCProxy::
getInstance ()
{
   return 0;
}

bool OmniPDCProxy::
queryPDLInfo ()
{
   if (fQueriedPDLInfo_d)
   {
      return fQueriedPDLInfo_d;
   }

   if (  !pCmd_d->setCommand (PDCCMD_GET_PDL_INFO)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      || !pCmd_d->getCommandString (false)
      )
      return false;

   fQueriedPDLInfo_d = 4 == sscanf (pCmd_d->getCommandString (false),
                                    "%d %d %d %d",
                                    &iPDLLevel_d,
                                    &iPDLSubLevel_d,
                                    &iPDLMajorRevisionLevel_d,
                                    &iPDLMinorRevisionLevel_d);

   return true;
}

int OmniPDCProxy::
getPDLLevel ()
{
   if (!queryPDLInfo ())
   {
      return 0;
   }

   return iPDLLevel_d;
}

int OmniPDCProxy::
getPDLSubLevel ()
{
   if (!queryPDLInfo ())
   {
      return 0;
   }

   return iPDLSubLevel_d;
}

int OmniPDCProxy::
getPDLMajorRevisionLevel ()
{
   if (!queryPDLInfo ())
   {
      return 0;
   }

   return iPDLMajorRevisionLevel_d;
}

int OmniPDCProxy::
getPDLMinorRevisionLevel ()
{
   if (!queryPDLInfo ())
   {
      return 0;
   }

   return iPDLMinorRevisionLevel_d;
}

bool OmniPDCProxy::
hasCapability (long lMask)
{
   bool fRet = false;

   if (  pCmd_d->setCommand (PDCCMD_HAS_CAPABILITY, lMask)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      && pCmd_d->getCommandBool (fRet)
      )
      return fRet;

   return false;
}

bool OmniPDCProxy::
hasRasterCapability (long lMask)
{
   bool fRet = false;

   if (  pCmd_d->setCommand (PDCCMD_HAS_RASTER_CAPABILITY, lMask)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      && pCmd_d->getCommandBool (fRet)
      )
      return fRet;

   return false;
}

bool OmniPDCProxy::
hasDeviceOption (PSZCRO pszDeviceOption)
{
   bool fRet = false;

   if (  pCmd_d->setCommand (PDCCMD_HAS_DEVICE_OPTION, pszDeviceOption)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      && pCmd_d->getCommandBool (fRet)
      )
      return fRet;

   return false;
}

int OmniPDCProxy::
getScanlineMultiple ()
{
   if (!pResolution_d)
   {
      pResolution_d = getCurrentResolution ();
   }

   if (pResolution_d)
      return pResolution_d->getScanlineMultiple ();
   else
      return 1;
}

bool OmniPDCProxy::
beginJob ()
{
   // Start the job on the device
   if (  pCmd_d->setCommand (PDCCMD_BEGIN_JOB)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // Start the page on the device
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool OmniPDCProxy::
beginJob (PSZCRO pszJobProperties)
{
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      // If there are job properties, pass them in
      if (  !pCmd_d->setCommand (PDCCMD_SET_JOB_PROPERTIES, pszJobProperties)
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return false;
   }

   // Start the job on the device
   if (  pCmd_d->setCommand (PDCCMD_BEGIN_JOB)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // Start the page on the device
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool OmniPDCProxy::
newFrame ()
{
   // Send a new page to the device
   if (  pCmd_d->setCommand (PDCCMD_END_PAGE)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // Start the page on the device
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool OmniPDCProxy::
newFrame (PSZCRO pszJobProperties)
{
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      // If there are job properties, pass them in
      if (  !pCmd_d->setCommand (PDCCMD_SET_JOB_PROPERTIES, pszJobProperties)
         || !pCmd_d->sendCommand (fdC2S_d)
         || !pCmd_d->readCommand (fdS2C_d)
         || PDCCMD_ACK != pCmd_d->getCommandType ()
         )
         return false;
   }

   // Send a new page to the device
   if (  pCmd_d->setCommand (PDCCMD_END_PAGE)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
   {
      // Start the page on the device
      if (  pCmd_d->setCommand (PDCCMD_START_PAGE)
         && pCmd_d->sendCommand (fdC2S_d)
         && pCmd_d->readCommand (fdS2C_d)
         && PDCCMD_ACK == pCmd_d->getCommandType ()
         )
         return true;
   }

   return false;
}

bool OmniPDCProxy::
endJob ()
{
   // End the page on the device
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

bool OmniPDCProxy::
abortJob ()
{
   // Abort the job on the device
   if (  pCmd_d->setCommand (PDCCMD_ABORT_JOB)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return true;

   return false;
}

bool OmniPDCProxy::
rasterize (PBYTE        pbBits,
           PBITMAPINFO2 pbmi,
           PRECTL       prectlPageLocation,
           BITBLT_TYPE  eType)
{
#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniPDCProxy ())
      DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__
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
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": could not PDCCMD_DETACH_BUFFER1" << std::endl;
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

         if (-1 == (uintptr_t)pbBuffer1_d)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": 1st shmat (" << idBuffer1_d << ") failed." << std::endl;
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
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": could not PDCCMD_ATTACH_BUFFER1" << std::endl;
#endif

            return false;
         }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ())
         DebugOutput::getErrorStream () << ", now cbBuffer1_d = " << cbBuffer1_d;
#endif
   }

   int cbBuffer2;

   cbBuffer2 = pbmi->cy * (((pbmi->cx * pbmi->cBitCount + 31) >> 5) << 2);

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniPDCProxy ())
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
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": could not PDCCMD_DETACH_BUFFER2" << std::endl;
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

         if (-1 == (uintptr_t)pbBuffer2_d)
         {
#ifndef RETAIL
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": 2nd shmat (" << idBuffer2_d << ") failed." << std::endl;
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
            if (DebugOutput::shouldOutputOmniPDCProxy ())
               DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": could not PDCCMD_ATTACH_BUFFER2" << std::endl;
#endif

            return false;
         }
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputOmniPDCProxy ())
         DebugOutput::getErrorStream () << ", now cbBuffer2_d = " << cbBuffer2_d;
#endif
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputOmniPDCProxy ()) DebugOutput::getErrorStream () << std::endl;
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
   if (DebugOutput::shouldOutputOmniPDCProxy ())
      DebugOutput::getErrorStream () << "OmniPDCProxy::" << __FUNCTION__ << ": could not PDCCMD_RASTERIZE" << std::endl;
#endif

   return false;
}

void OmniPDCProxy::
setOutputStream (FILE *pFile)
{
   // @TBD
}

void OmniPDCProxy::
setOutputFunction (PFNOUTPUTFUNCTION  pfnOutputFunction,
                   void              *pMagicCookie)
{
   // @TBD
}

void OmniPDCProxy::
setErrorStream (FILE *pFile)
{
   // @TBD
}

bool OmniPDCProxy::
setLanguage (int iLanguageID)
{
   if (iLanguageID != iLanguageID_d)
   {
      StringResource *pNewLanguage = StringResource::create (iLanguageID, 0);

      if (pNewLanguage)
      {
         delete pLanguage_d;

         iLanguageID_d = iLanguageID;
         pLanguage_d   = pNewLanguage;

         return true;
      }
      else
      {
         return false;
      }
   }

   return true;
}

int OmniPDCProxy::
getLanguage ()
{
   return iLanguageID_d;
}

Enumeration * OmniPDCProxy::
getLanguages ()
{
   // @TBD
   return 0;
}

StringResource * OmniPDCProxy::
getLanguageResource ()
{
   return pLanguage_d;
}

std::string * OmniPDCProxy::
getJobProperties (bool fInDeviceSpecific)
{
   // Query the current job properties
   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTIES, fInDeviceSpecific)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return new std::string (pCmd_d->getCommandString (false));

   return 0;
}

bool OmniPDCProxy::
setJobProperties (PSZCRO pszJobProperties)
{
   // Set the current job properties
   if (  !pCmd_d->setCommand (PDCCMD_SET_JOB_PROPERTIES, pszJobProperties)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
      return false;

   return true;
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

Enumeration * OmniPDCProxy::
listJobProperties (bool fInDeviceSpecific) // @TBD
{
   JPEnumerator *pRet = new JPEnumerator (fInDeviceSpecific);

#ifdef INCLUDE_JP_UPDF_BOOKLET
   if (getCurrentBooklet ())
   {
      pRet->addElement (getCurrentBooklet ()->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   if (getCurrentCopies ())
   {
      pRet->addElement (getCurrentCopies ()->getEnumeration (fInDeviceSpecific));
   }
#endif
   pRet->addElement (getDitherEnumeration (fInDeviceSpecific));
   if (getCurrentForm ())
   {
      pRet->addElement (getCurrentForm ()->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_UPDF_JOGGING
   if (getCurrentJogging ())
   {
      pRet->addElement (getCurrentJogging ()->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (getCurrentMedia ())
   {
      pRet->addElement (getCurrentMedia ()->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_NUP
   if (getCurrentNUp ())
   {
      pRet->addElement (getCurrentNUp ()->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (getCurrentOrientation ())
   {
      pRet->addElement (getCurrentOrientation ()->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   if (getCurrentOutputBin ())
   {
      pRet->addElement (getCurrentOutputBin ()->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (getCurrentPrintMode ())
   {
      pRet->addElement (getCurrentPrintMode ()->getEnumeration (fInDeviceSpecific));
   }
   if (getCurrentResolution ())
   {
      pRet->addElement (getCurrentResolution ()->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_SCALING
   if (getCurrentScaling ())
   {
      pRet->addElement (getCurrentScaling ()->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   if (getCurrentSheetCollate ())
   {
      pRet->addElement (getCurrentSheetCollate ()->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   if (getCurrentSide ())
   {
      pRet->addElement (getCurrentSide ()->getEnumeration (fInDeviceSpecific));
   }
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   if (getCurrentStitching ())
   {
      pRet->addElement (getCurrentStitching ()->getEnumeration (fInDeviceSpecific));
   }
#endif
   if (getCurrentTray ())
   {
      pRet->addElement (getCurrentTray ()->getEnumeration (fInDeviceSpecific));
   }
#ifdef INCLUDE_JP_COMMON_TRIMMING
   if (getCurrentTrimming ())
   {
      pRet->addElement (getCurrentTrimming ()->getEnumeration (fInDeviceSpecific));
   }
#endif

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

std::string * OmniPDCProxy::
getJobPropertyType (PSZCRO pszKey)
{
   // Query the current job properties
   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTY_TYPE, pszKey)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return new std::string (pCmd_d->getCommandString (false));

   return 0;
}

std::string * OmniPDCProxy::
getJobProperty (PSZCRO pszKey)
{
   // Query the current job properties
   if (  pCmd_d->setCommand (PDCCMD_GET_JOB_PROPERTY, pszKey)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return new std::string (pCmd_d->getCommandString (false));

   return 0;
}

std::string * OmniPDCProxy::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string cmd = pszKey;

   cmd += "=";
   cmd += pszValue;

   if (  pCmd_d->setCommand (PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE, &cmd)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      return new std::string (pCmd_d->getCommandString (false));

   return 0;
}

Enumeration * OmniPDCProxy::
getDitherEnumeration (bool fInDeviceSpecific)
{
   bool fSuccess = false;

   // Enumerate the dither ids of that device
   if (  pCmd_d->setCommand (PDCCMD_ENUM_DITHER_IDS, fInDeviceSpecific)
      && pCmd_d->sendCommand (fdC2S_d)
      && pCmd_d->readCommand (fdS2C_d)
      && PDCCMD_ACK == pCmd_d->getCommandType ()
      )
      fSuccess = true;

   return new CmdArrayEnumeration (fSuccess, 0, pCmd_d);
}

/*8<--------8<--------8<--------8<--------8<--------8<--------8<--------*/

#ifndef RETAIL

void OmniPDCProxy::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string OmniPDCProxy::
toString (std::ostringstream& oss)
{
   oss << "{OmniPDCProxy: "
          // @TBD
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const OmniPDCProxy& const_self)
{
   OmniPDCProxy&      self = const_cast<OmniPDCProxy&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
