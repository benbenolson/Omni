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
#include "JobProperties.hpp"

#if 1

int
main (int argc, char *argv[])
{
   Enumeration *pEnum      = 0;
   bool         fBuildOnly = true;

   setenv ("OMNI_DEBUG_OPTIONS", "omni", 1);

   Omni::initialize ();

   if (2 == argc)
   {
      if (0 == strcmp (argv[1], "true"))
      {
         fBuildOnly = true;
      }
      else if (0 == strcmp (argv[1], "false"))
      {
         fBuildOnly = false;
      }
   }

   pEnum = Omni::listXMLDevices (fBuildOnly);

   while (pEnum->hasMoreElements ())
   {
      char *pszFileName = (char *)pEnum->nextElement ();

      std::cout << (pszFileName ? pszFileName : "NULL") << std::endl;
   }

   delete pEnum;

   Omni::terminate ();

   return 0;
}

#endif

#if 0

int
main (int argc, char *argv[])
{
   Omni::initialize ();

   Enumeration *pEnum = Omni::listDevices (true);

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD     = (OmniDevice *)pEnum->nextElement ();
      char       *pszName = "Error";

      if (pOD)
      {
         pszName = pOD->getLibraryName ();
      }

      std::cout << pszName << std::endl;

      delete pOD;
   }

   Omni::terminate ();

   return 0;
}

#endif

#if 0

int
main (int argc, char *argv[])
{
   Omni::initialize ();

   Enumeration   *pEnum = DeviceDither::getAllEnumeration ();
   JobProperties *pJP   = 0;
   PSZRO          pszJP = 0;

   while (pEnum->hasMoreElements ())
   {
      pJP = (JobProperties *)pEnum->nextElement ();

      if (!pJP)
      {
         std::cerr << "Error: Enumertation returned NULL!" << std::endl;
         continue;
      }

      pszJP = pJP->getJobProperties ();

      if (!pszJP)
      {
         std::cerr << "Error: Job Properties returns NULL!" << std::endl;
         delete pJP;
         break;
      }

      std::cout << pszJP << std::endl;

      delete pJP;
      free ((void *)pszJP);
   }

   delete pEnum;

   Omni::terminate ();

   return 0;
}

#endif

#if 0

int
main (int argc, char *argv[])
{
   Omni::initialize ();

   if (2 == argc)
   {
      extern bool ditherLibraryValid (char *pszName);

      ditherLibraryValid (argv[1]);
   }
   else
   {
      extern char *queryLibrary (char *pszFillIn, char *pszID);
      char achLine[512];

      queryLibrary (achLine, "DITHER_BOB");
   }

   Omni::terminate ();

   return 0;
}

#endif
