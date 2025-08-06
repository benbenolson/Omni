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
#include "OmniGSInterface.hpp"
#include "DriverInfo.hpp"
#include "JobPropertyDialogController.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <gtk--/main.h>

int
GetOmniJobProperties (const char  *pszDriverName,
                      const char **ppszJobProperties)
{
   int        iArgc                   = 1;
   char      *apszArgs[]              = {
      "JobProperty-Dialog",
      "Deceive-gtk-by-faking"
   };
   char     **apszArgv                = (char **)apszArgs;
   Gtk::Kit   kit (&iArgc, &apszArgv);
   bool       fDriverFound            = false;
   DriverInfo di;
   string     driverName ("");

   if (0 != strncmp (pszDriverName, "lib", 3))
      driverName += "lib";
   driverName += pszDriverName;
   if (0 != strncmp (pszDriverName + strlen (pszDriverName) - 3, ".so", 3))
      driverName += ".so";

   if (1 == di.openDevice ((char *)driverName.c_str (),
                           (ppszJobProperties ? *ppszJobProperties : 0)))
   {
      di.generateDriverInfo ();
      di.closeDevice ();

      fDriverFound = true;
   }
   else
   {
      return 0;
   }

   if (!fDriverFound)
   {
      std::cerr << std::endl << "The driver < " << driverName << " > was not found" << std::endl;
      std::cerr << "Choose a valid driver name." << std::endl << std::endl;
      std::cerr << "Also make sure that the LD_LIBRARY_PATH is correctly set" << std::endl;
      std::cerr << " to point to the directory where lib" << driverName << ".so exists" << std::endl;

      return 0;
   }

   JobDialog::JobPropertyDialogController controller (di.getDriver ());
   std::vector<string>                    jobprops   = controller.getSelectedJobProperties ();
   int                                    cbAlloc    = 0;

   for (int ijob = 0; ijob < (int)jobprops.size (); ijob++)
   {
      if (cbAlloc)
         // Add a space separator
         cbAlloc++;
      cbAlloc += strlen (jobprops[ijob].c_str ());
   }

   if (!cbAlloc)
      return 0;

   *ppszJobProperties = (char *)calloc (1, cbAlloc + 1);

   if (!*ppszJobProperties)
      return 0;

   for (int ijob = 0; ijob < (int)jobprops.size (); ijob++)
   {
      if (**ppszJobProperties)
         strcat ((char *)*ppszJobProperties, " ");
      strcat ((char *)*ppszJobProperties, jobprops[ijob].c_str ());
   }

   return 1;
}

int
FreeOmniJobProperties (char **ppszJobProperties)
{
   if (*ppszJobProperties)
   {
      free (*ppszJobProperties);
      *ppszJobProperties = 0;
   }

   return 1;
}
