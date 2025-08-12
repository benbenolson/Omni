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
#include "DeviceInfo.hpp"
#include "OmniDomParser.hpp"

#include <sys/stat.h>
#include <fstream>

#ifdef DEBUG
const static bool fDebugOutput = false;
#endif

extern void convertFilename (char *pszFileName);

char *vpszExeName = 0;

bool
areValidFiles (std::string *pstrFileName)
{
   std::string stringFileNameHPP (pstrFileName->c_str ());
   std::string stringFileNameCPP (pstrFileName->c_str ());

   convertFilename (const_cast<char*>(stringFileNameHPP.c_str ())); // @HACK?
   convertFilename (const_cast<char*>(stringFileNameCPP.c_str ())); // @HACK?

   stringFileNameHPP.append (".hpp");
   stringFileNameCPP.append (".cpp");

   struct stat statOutHPP;
   struct stat statOutCPP;

   if (-1 == stat (stringFileNameHPP.c_str (), &statOutHPP))
      return false;

#ifdef DEBUG
   if (fDebugOutput)
   {
      std::cerr << "stats for \"" << stringFileNameHPP << "\"" << std::endl;
      std::cerr << "   atime = " << statOutHPP.st_atime << std::endl;
      std::cerr << "   mtime = " << statOutHPP.st_mtime << std::endl;
      std::cerr << "   ctime = " << statOutHPP.st_ctime << std::endl;
   }
#endif

   if (-1 == stat (stringFileNameCPP.c_str (), &statOutCPP))
      return false;

#ifdef DEBUG
   if (fDebugOutput)
   {
      std::cerr << "stats for \"" << stringFileNameCPP << "\"" << std::endl;
      std::cerr << "   atime = " << statOutCPP.st_atime << std::endl;
      std::cerr << "   mtime = " << statOutCPP.st_mtime << std::endl;
      std::cerr << "   ctime = " << statOutCPP.st_ctime << std::endl;
   }
#endif

   if (  0 == statOutHPP.st_size
      || 0 == statOutCPP.st_size
      )
      return false;
   else
      return true;
}

char *
findFullPath (char *pszExeName)
{
   const char *pszEnvPath = getenv ("PATH");
   char       *pszFQN     = 0;
   int         cbExeName  = 0;
   struct stat statExe;

   if (!pszExeName)
   {
      return 0;
   }

   cbExeName = strlen (pszExeName);

#ifdef DEBUG
   if (fDebugOutput) std::cerr << "pszEnvPath = " << pszEnvPath << std::endl;
#endif

   if (pszExeName[0] == '/')
   {
      return strdup (pszExeName);
   }
   else if (pszEnvPath)
   {
      char *pszPathCpy = (char *)malloc (strlen (pszEnvPath) + 1);

      if (pszPathCpy)
      {
         char *pszPath = pszPathCpy;

         strcpy (pszPathCpy, pszEnvPath);

         while (  pszPath
               && *pszPath
               )
         {
            char *pszColon = strchr (pszPath, ':'); // @TBD - linux

            if (pszColon)
            {
               *pszColon = '\0';
            }

            pszFQN = (char *)malloc (strlen (pszPath) + cbExeName + 2);

            if (!pszFQN)
            {
               return 0;
            }

            strcpy (pszFQN, pszPath);
            strcat (pszFQN, "/");                   // @TBD - linux
            strcat (pszFQN, pszExeName);

#ifdef DEBUG
            if (fDebugOutput) std::cerr << "pszFQN = " << pszFQN << std::endl;
#endif

            if (0 == stat (pszFQN, &statExe))
            {
               free (pszPathCpy);

               return pszFQN;
            }

            free (pszFQN);
            pszFQN = 0;

            if (pszColon)
            {
               pszPath = pszColon + 1;
            }
            else
            {
               pszPath = 0;
            }
         }

         free (pszPathCpy);
      }
   }

   return pszFQN;
}

int
main (int argc, char *argv[])
{
   bool    fErrorEncountered = false;
   bool    fAutoconf         = false;
   bool    fAutoconfNoInst   = false;
   bool    fFileModified     = false;
   FileMap mapSeenFiles;

   XMLInitialize ();

   vpszExeName = findFullPath (argv[0]);

   for (int i = 1; i < argc; i++)
   {
      if (0 == strcasecmp (argv[i], "-a"))
      {
         fAutoconf = true;
      }
      else if (0 == strcasecmp (argv[i], "-i"))
      {
         fAutoconfNoInst = true;
      }
   }

   if (fAutoconf)
   {
      std::ofstream ofstreamAM1 ("libraries1.mak", std::ios::trunc);
      std::ofstream ofstreamAM2 ("libraries2.mak", std::ios::trunc);
      std::ofstream ofstreamAM3 ("libraries3.mak", std::ios::trunc);

      ofstreamAM1.close ();
      ofstreamAM2.close ();
      ofstreamAM3.close ();
   }

   char achLine[512]; // @TBD

   while (std::cin.getline (achLine, sizeof (achLine)))
   {
//////std::cout << "line = \"" << achLine << "\"" << std::endl;

      if (  '#' != achLine[0]
         && sizeof (achLine) - 4 > strlen (achLine)
         )
      {
         DeviceInfo di;

#ifdef INCLUDE_JP_UPDF_BOOKLET
         di.pstringBookletClassName      = 0;
#endif
         di.pstringCopyClassName         = 0;
         di.pstringFormClassName         = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
         di.pstringJoggingClassName      = 0;
#endif
         di.pstringMediaClassName        = 0;
         di.pstringNUpClassName          = 0;
         di.pstringOrientationClassName  = 0;
         di.pstringOutputBinClassName    = 0;
         di.pstringPrintModeClassName    = 0;
         di.pstringResolutionClassName   = 0;
         di.pstringScalingClassName      = 0;
         di.pstringSheetCollateClassName = 0;
         di.pstringSideClassName         = 0;
         di.pstringStitchingClassName    = 0;
         di.pstringTrayClassName         = 0;
         di.pstringTrimmingClassName     = 0;
         di.pstringGammaClassName        = 0;
         di.pstringCommandClassName      = 0;
         di.pstringDataClassName         = 0;
         di.pstringStringClassName       = 0;

         di.pstringInstanceClassName     = 0;
         di.pstringBlitterClassName      = 0;

         di.pstringExeName               = 0;
         di.pstringData                  = 0;

         di.mapSeenFiles                 = &mapSeenFiles;

         strcat (achLine, ".xml");

         OmniDomParser parser (achLine,
                               &di,
                               fAutoconf,
                               fAutoconfNoInst);

         if (parser.parse ())
         {
            if (!areValidFiles (di.pstringCommandClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringCommandClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringOrientationClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringOrientationClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringResolutionClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringResolutionClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringPrintModeClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringPrintModeClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringTrayClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringTrayClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringFormClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringFormClassName << "*" << std::endl;
            }
            if (!areValidFiles (di.pstringMediaClassName))
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringMediaClassName << "*" << std::endl;
            }
////////////if (!areValidFiles (di.pstringGammaClassName))
////////////{
////////////   fErrorEncountered = true;
////////////   std::cerr << "Error: Could not create " << *di.pstringGammaClassName << "*" << std::endl;
////////////}
            if (  di.pstringInstanceClassName
               && !areValidFiles (di.pstringInstanceClassName)
               )
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringInstanceClassName << "*" << std::endl;
            }
            if (  di.pstringBlitterClassName
               && !areValidFiles (di.pstringBlitterClassName)
               )
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringBlitterClassName << "*" << std::endl;
            }
            if (  di.pstringDataClassName
               && !areValidFiles (di.pstringDataClassName)
               )
            {
               fErrorEncountered = true;
               std::cerr << "Error: Could not create " << *di.pstringDataClassName << "*" << std::endl;
            }
         }
         else
         {
            fErrorEncountered = true;
         }

         fFileModified = fFileModified || parser.fileModified ();

         // Clean up!
#ifdef INCLUDE_JP_UPDF_BOOKLET
         delete di.pstringBookletClassName;
#endif
         delete di.pstringCopyClassName;
         delete di.pstringFormClassName;
#ifdef INCLUDE_JP_UPDF_JOGGING
         delete di.pstringJoggingClassName;
#endif
         delete di.pstringMediaClassName;
         delete di.pstringNUpClassName;
         delete di.pstringOrientationClassName;
         delete di.pstringOutputBinClassName;
         delete di.pstringPrintModeClassName;
         delete di.pstringResolutionClassName;
         delete di.pstringScalingClassName;
         delete di.pstringSheetCollateClassName;
         delete di.pstringSideClassName;
         delete di.pstringStitchingClassName;
         delete di.pstringTrayClassName;
         delete di.pstringTrimmingClassName;
         delete di.pstringGammaClassName;
         delete di.pstringCommandClassName;
         delete di.pstringDataClassName;
         delete di.pstringStringClassName;

         delete di.pstringInstanceClassName;
         delete di.pstringBlitterClassName;
         delete di.pstringExeName;
         delete di.pstringData;
      }
   }

   if (vpszExeName)
   {
      free (vpszExeName);
      vpszExeName = 0;
   }

   XMLCleanup ();

   if (fAutoconf)
   {
      if (fFileModified)
      {
         return 1;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      return fErrorEncountered;
   }
}
