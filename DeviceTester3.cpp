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
#include "Device.hpp"
#include "Omni.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

static char *vpszServerToClient     = "PDC_SRV_TO_CLIENT";
static char *vpszClientToServer     = "PDC_CLIENT_TO_SRV";
static char *vpszExecProgram        = "PDC_EXEC_PROGRAM";
static char *vpszExecProgramDefault = "OmniPDCServer";

char *
setupName (char *pszName, char *pszFormat)
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

int
main (int argc, char *argv[])
{
   char                *pszJobProperties  = 0;
   int                  fdStdOut          = STDOUT_FILENO;
   int                  fdStdErr          = STDERR_FILENO;
   char                *pszDeviceName     = 0;
   bool                 fHasError_d;
   bool                 fAdvanced_d;
   int                  fdS2C_d;
   int                  fdC2S_d;
   char                *pszS2C_d;
   char                *pszC2S_d;
   int                  idBuffer1_d;
   int                  cbBuffer1_d;
   byte                *pbBuffer1_d;
   int                  idBuffer2_d;
   int                  cbBuffer2_d;
   byte                *pbBuffer2_d;
   PrinterCommand      *pCmd_d;
   char                *pszVersion_d;
   char                *pszDriverName_d;
   char                *pszShortName_d;
   char                *pszS2C            = 0;
   char                *pszC2S            = 0;
   bool                 fSucceeded        = false;
   int                  pid;
   int                  rc;

   if (2 > argc)
   {
      std::cerr << "Usage: " << argv[0] << " PrinterName" << std::endl;

      return 1;
   }

   Omni::initialize ();

   pszDeviceName = argv[1];

   // Assign fifo names for environment space
   pszS2C_d                 = setupName (vpszServerToClient, "%s=/tmp/PDC_s2c_%d");
   pszC2S_d                 = setupName (vpszClientToServer, "%s=/tmp/PDC_c2s_%d");
   pszS2C                   = getenv (vpszServerToClient);
   pszC2S                   = getenv (vpszClientToServer);

   fHasError_d              = false;

   fAdvanced_d              = true;

   fdS2C_d                  = -1;
   fdC2S_d                  = -1;

   idBuffer1_d              = -1;
   cbBuffer1_d              = 0;
   pbBuffer1_d              = 0;
   idBuffer2_d              = -1;
   cbBuffer2_d              = 0;
   pbBuffer2_d              = 0;

   pCmd_d                   = new PrinterCommand ("DeviceTester7");

   pszVersion_d             = 0;
   pszDriverName_d          = 0;
   pszShortName_d           = 0;

   if (  !pszS2C_d
      || !pszC2S_d
      || !pCmd_d
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

   /* Spawn the server. */
   if ((pid = fork ()) < 0)
   {
      goto BUGOUT;
   }
   else if (pid > 0)
   {
      /* parent */
      int rc;
      int status = 0;

      rc = waitpid (pid, &status, WNOHANG);

      if (  rc == pid
         && 0 != WIFEXITED (status)
         && EXIT_FAILURE == WEXITSTATUS (status)
         )
      {
         DebugOutput::getErrorStream () << __FUNCTION__ << ": client not running." << std::endl;

         goto BUGOUT;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ())
      {
         DebugOutput::getErrorStream () << __FUNCTION__ << ": waitpid returns " << rc << ", status = " << status << std::endl;
         DebugOutput::getErrorStream () << __FUNCTION__ << ": waitpid: " << WIFEXITED (status) << std::endl;
         DebugOutput::getErrorStream () << __FUNCTION__ << ": waitpid: " << WEXITSTATUS (status) << std::endl;
      }
#endif

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
      char *pszExecProgram = 0;

      /* Redirect print_stream to stdout. */
      if (fdStdOut != STDOUT_FILENO)
      {
         dup2 (fdStdOut, STDOUT_FILENO);
      }
      if (fdStdErr != STDERR_FILENO)
      {
         dup2 (fdStdErr, STDERR_FILENO);
      }

      pszExecProgram = getenv (vpszExecProgram);
      if (!pszExecProgram)
      {
         pszExecProgram = vpszExecProgramDefault;
      }

      if (execlp (pszExecProgram, pszExecProgram, (char *)0, (char *)0) < 0)
      {
         DebugOutput::getErrorStream () << __FUNCTION__ << ": execlp failed." << std::endl;

	      goto BUGOUT2;
      }

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceTester ()) DebugOutput::getErrorStream () << __FUNCTION__ << ": execlp succeded." << std::endl;
#endif

      exit (0);

BUGOUT2:
      exit (EXIT_FAILURE);
   }

   // Initialize the session
   if (  !pCmd_d->setCommand (PDCCMD_INITIALIZE_SESSION,
                              PDC_VERSION)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to initialize the session!" << std::endl;

      goto BUGOUT;
   }

   // Tell the server what the device name will be
   if (  !pCmd_d->setCommand (PDCCMD_SET_DEVICE_NAME, pszDeviceName)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to set main device name!" << std::endl;

      goto BUGOUT;
   }

   if (fAdvanced_d)
   {
      // Tell the server to used advanced bitblt mode.  This is optional
      pCmd_d->setCommand (PDCCMD_MODE_IS_RENDERER, 1);
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
         DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to set main job properties!" << std::endl;

         goto BUGOUT;
      }
   }

   // Tell the server to create a device instance
   if (  !pCmd_d->setCommand (PDCCMD_NEW_DEVICE, (char *)0)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to create a new device instance!" << std::endl;

      goto BUGOUT;
   }

   if (  !pCmd_d->setCommand (PDCCMD_ENUM_LONG_DEVICES, (char *)0)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to PDCCMD_ENUM_LONG_DEVICES!" << std::endl;

      goto BUGOUT;
   }
   else
   {
      std::cout << *pCmd_d << std::endl;
   }

   if (  !pCmd_d->setCommand (PDCCMD_ENUM_SHORT_DEVICES, (char *)0)
      || !pCmd_d->sendCommand (fdC2S_d)
      || !pCmd_d->readCommand (fdS2C_d)
      || PDCCMD_ACK != pCmd_d->getCommandType ()
      )
   {
      DebugOutput::getErrorStream () << __FUNCTION__ << ": Failed to PDCCMD_ENUM_SHORT_DEVICES!" << std::endl;

      goto BUGOUT;
   }
   else
   {
      std::cout << *pCmd_d << std::endl;
   }

   fSucceeded = true;

BUGOUT:

   // End the connection
   if (pCmd_d)
   {
      if (pCmd_d->setCommand (PDCCMD_CLOSE_SESSION, (char *)0))
      {
         pCmd_d->sendCommand (fdC2S_d);
      }

      delete pCmd_d;
      pCmd_d = 0;
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
         DebugOutput::getErrorStream () << __FUNCTION__ << ": remove (" << pszS2C << ") = " << errno << std::endl;
      }
   }
   if (pszC2S)
   {
      rc = remove (pszC2S);
      if (-1 == rc)
      {
         DebugOutput::getErrorStream () << __FUNCTION__ << ": remove (" << pszC2S << ") = " << errno << std::endl;
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
   if (pszShortName_d)
   {
      free (pszShortName_d);
      pszShortName_d = 0;
   }

   Omni::terminate ();

   return fSucceeded ? 0 : 1;
}
