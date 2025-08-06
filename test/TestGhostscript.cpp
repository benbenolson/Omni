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
#include <iostream>

#include <sys/stat.h>

#include "Omni.hpp"
#include "JobProperties.hpp"

void
printUsage (char *pszProgramName)
{
   std::cerr << "Usage: "
             << pszProgramName
             << " ( --buildOnly | --cout print.out | --cerr err.out | '-sproperties=\"...\"' )+"
             << std::endl;
}

int
main (int argc, char *argv[])
{
   PSZ            pszCout           = "output.prn";
   PSZ            pszCerr           = 0;
   bool           fBuildOnly        = false;
   bool           fDelete           = false;
   JobProperties  jobProp;
   Enumeration   *pEnum             = 0;
   int            rc                = 0;

   if (!g_module_supported ())
   {
      DebugOutput::getErrorStream () << "Error: This program needs glib's module routines!" << std::endl;

      return __LINE__;
   }

   if (2 > argc)
   {
      printUsage (argv[0]);

      return __LINE__;
   }

   Omni::initialize ();

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp ("--buildOnly", argv[i]))
      {
         fBuildOnly = true;
      }
      else if (0 == strcmp ("--cout", argv[i]))
      {
         pszCout = argv[++i];
      }
      else if (0 == strcmp ("--cerr", argv[i]))
      {
         pszCerr = argv[++i];
      }
      else if (0 == strcmp ("--delete", argv[i]))
      {
         fDelete = true;
      }
      else if (0 == strncasecmp ("-sproperties=", argv[i], 13))
      {
         char *pszJobProp = argv[i] + 13;

         jobProp.setJobProperties (pszJobProp);
      }
   }

   pEnum = Omni::listDevices (fBuildOnly);

   while (pEnum->hasMoreElements ())
   {
      OmniDevice *pOD = (OmniDevice *)pEnum->nextElement ();

      if (pOD)
      {
         PSZCRO             pszLibName           = pOD->getLibraryName ();
         PSZRO              pszJobProps          = pOD->getJobProperties ();
         PSZ                pszShortLibName      = 0;
         PSZRO              pszSlash             = 0;
         JobProperties      jobPropNew           = jobProp;
         std::ostringstream oss;
         std::string        stringOutputFilename;
         std::string        stringErrorFilename;
         GModule           *phmodDevice          = 0;
         Device            *pDevice              = 0;
         int                rcGS                 = 0;

         pDevice = Omni::createDevice (pOD, &phmodDevice);

         if (pDevice)
         {
            oss << pDevice->getShortName ()
                << ".out";

            stringOutputFilename = oss.str ();

            if (pszCerr)
            {
               oss.str ("");

               oss << pDevice->getShortName ()
                   << ".err";

               stringErrorFilename = oss.str ();
            }

            oss.str ("");
            delete pDevice;
         }
         else
         {
            if (pszCout)
            {
               stringOutputFilename = pszCout;
            }
            if (pszCerr)
            {
               stringErrorFilename = pszCerr;
            }
         }

         g_module_close (phmodDevice);

         jobPropNew.setJobProperties (pszJobProps);

         pszJobProps = jobPropNew.getJobProperties ();

         pszSlash = strchr (pszLibName, '/');
         if (pszSlash)
         {
            pszSlash++;
         }
         while (pszSlash)
         {
            PSZRO pszNewSlash = strchr (pszSlash, '/');

            if (pszNewSlash)
            {
               pszSlash = pszNewSlash + 1;
            }
            else
            {
               break;
            }
         }
         if (!pszSlash)
         {
            pszSlash = pszLibName;
         }
         if (0 == strncmp (pszSlash, "lib", 3))
         {
            pszSlash += 3;
         }
         pszShortLibName = (PSZ)malloc (strlen (pszSlash) + 1);

         if (pszShortLibName)
         {
            int cbShortLibName = 0;
            PSZ pszEnd         = 0;

            strcpy (pszShortLibName, pszSlash);

            cbShortLibName = strlen (pszShortLibName);

            if (3 < cbShortLibName)
            {
               pszEnd = pszShortLibName + cbShortLibName - 3;
            }

            if (  pszEnd
               && 0 == strcmp (pszEnd, ".so")
               )
            {
               *pszEnd = '\0';
            }

            oss << "gs -dNOPAUSE -dBATCH -sDEVICE=omni -sDeviceName="
                << pszShortLibName;
            if (pszJobProps)
            {
               oss << " -sproperties='"
                   << pszJobProps
                   << "'";
            }
            if (stringOutputFilename[0])
            {
               oss << " -sOutputFile=" << stringOutputFilename;

               unlink (stringOutputFilename.c_str ());
            }
            if (stringErrorFilename[0])
            {
               oss << " -sdbgout=" << stringErrorFilename;

               unlink (stringErrorFilename.c_str ());
            }
            oss << " test.ps";

            rcGS = Omni::my_system (oss.str ().c_str ());

            std::cout << oss.str () << " = " << rcGS << std::endl;

            if (  pszCout
               || pszCerr
               )
            {
               struct stat statOut;
               struct stat statErr;
               int         cbOut    = 0;
               int         cbErr    = 0;

               if (  stringOutputFilename[0]
                  && 0 == stat (stringOutputFilename.c_str (), &statOut)
                  )
               {
                  cbOut = statOut.st_size;
               }
               if (  stringErrorFilename[0]
                  && 0 == stat (stringErrorFilename.c_str (), &statErr)
                  )
               {
                  cbErr = statErr.st_size;
               }

               std::cout << "out = " << cbOut;
               if (pszCerr)
               {
                  std::cout << ", err = " << cbErr;
               }
               std::cout << std::endl;
            }

            if (fDelete)
            {
               if (stringOutputFilename[0])
               {
                  unlink (stringOutputFilename.c_str ());
               }
               if (stringErrorFilename[0])
               {
                  unlink (stringErrorFilename.c_str ());
               }
            }

            free ((void *)pszShortLibName);
         }

         if (pszJobProps)
         {
            free ((void*)pszJobProps);
            pszJobProps = 0;
         }

         delete pOD;
      }
      else
      {
         std::cerr << "Error: No OmniDevice was returned!" << std::endl;

         rc = __LINE__;
      }
   }

   // Clean up!
   delete pEnum;
   pEnum = 0;

   Omni::terminate ();

   return rc;
}
