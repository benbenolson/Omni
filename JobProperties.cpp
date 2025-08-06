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
#include "JobProperties.hpp"

#include <iostream>
#include <sstream>
#include <cstdlib>

#define PRINT_VAR(x)   std::cerr << #x " = " << SAFE_PRINT_PSZ(x) << std::endl

#ifndef RETAIL
static const bool vfDebug = false;
#endif

JobProperties::
JobProperties (PSZCRO pszJobProperties)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "New: (" << std::hex << &mapDJP_d << std::dec << ") " << std::endl; // @TBD
#endif

   setJobProperties (pszJobProperties);
}

JobProperties::
JobProperties (std::string stringJobProperties)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "New: (" << std::hex << &mapDJP_d << std::dec << ") " << std::endl; // @TBD
#endif

   setJobProperties (stringJobProperties.c_str ());
}

JobProperties::
JobProperties (Device *pDevice)
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "New: (" << std::hex << &mapDJP_d << std::dec << ") " << std::endl; // @TBD
#endif

   if (pDevice)
   {
      std::string *pstrJobProps = pDevice->getJobProperties (false);

      if (pstrJobProps)
      {
         PSZCRO pszJobProperties = pstrJobProps->c_str ();

         setJobProperties (pszJobProperties);

         delete pstrJobProps;
      }
   }
}

JobProperties::
~JobProperties ()
{
#ifndef RETAIL
   if (vfDebug) std::cerr << "Free: (" << std::hex << &mapDJP_d << std::dec << ") " << std::endl; // @TBD
#endif
}

JobProperties& JobProperties::
operator= (const JobProperties& const_jp)
{
   JobProperties& jp      = const_cast<JobProperties&>(const_jp);
   DJPMap&        newDJPs = jp.getDJPMap ();

   mapDJP_d.clear ();

   for ( DJPMap::iterator next = newDJPs.begin ();
         next != newDJPs.end () ;
         next++ )
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << "Adding: (" << std::hex << &mapDJP_d << std::dec << ") " << next->first << "=\"" << next->second << "\"" << std::endl; // @TBD
#endif

      mapDJP_d[next->first] = next->second;
   }

   return *this;
}

JobPropertyEnumerator * JobProperties::
getEnumeration (PSZCRO pszKey)
{
   JobPropertyEnumerator *pRet             = 0;
   PSZCRO                 pszJobProperties = getJobProperties (false);

   pRet = new JobPropertyEnumerator (pszJobProperties, pszKey, true);

   if (pszJobProperties)
   {
      free ((void *)pszJobProperties);
   }

   return pRet;
}

PSZCRO JobProperties::
getJobProperties (bool fChange)
{
   std::ostringstream oss;
   bool               fFirstItem = true;

   for ( DJPMap::iterator next = mapDJP_d.begin ();
         next != mapDJP_d.end () ;
         next++ )
   {
      bool fShouldQuote = false;

#ifndef RETAIL
      if (vfDebug) std::cerr << "Querying: (" << std::hex << &mapDJP_d << std::dec << ") " << next->first << "=\"" << next->second << "\"" << std::endl; // @TBD
#endif

      if (fFirstItem)
      {
         fFirstItem = false;
      }
      else
      {
         oss << " ";
      }

      oss << next->first
          << "=";

      if (  std::string::npos != next->second.find (' ')
         && '\"' != next->second[0]
         )
      {
         fShouldQuote = true;
      }

      if (fShouldQuote)
      {
         oss << '\"';
      }

      if (  fChange
         && '{' == next->second[0]
         )
      {
         std::string::size_type posEnd = next->second.find (',');

         if (std::string::npos == posEnd)
         {
            posEnd = next->second.find ('}');
         }

         oss << next->second.substr (1, posEnd - 1);
      }
      else
      {
         oss << next->second;
      }

      if (fShouldQuote)
      {
         oss << '\"';
      }
   }

   std::string stringReturn = oss.str ();
   PSZCRO      pszFrom      = stringReturn.c_str ();

   if (  pszFrom
      && *pszFrom
      )
   {
      char *pszTo = 0;

#ifndef RETAIL
      if (vfDebug) std::cerr << "Returning (" << std::hex << &mapDJP_d << std::dec << ") \"" << pszFrom << "\"" << std::endl; // @TBD
#endif

      pszTo = (char *)malloc (strlen (pszFrom) + 1);
      if (pszTo)
      {
         strcpy (pszTo, pszFrom);
      }

      return pszTo;
   }
   else
   {
      return 0;
   }
}

void JobProperties::
setJobProperty (PSZCRO pszKeyValue)
{
   if (  pszKeyValue
      && *pszKeyValue
      )
   {
      std::string            stringKeyValue (pszKeyValue);
      std::string::size_type idx                          = std::string::npos;

      idx = stringKeyValue.find ('=');

      if (std::string::npos != idx)
      {
         setJobProperty (stringKeyValue.substr (0, idx).c_str (),
                         stringKeyValue.substr (idx + 1).c_str ());
      }
   }
}

void JobProperties::
setJobProperty (PSZCRO pszKey,
                PSZCRO pszValue)
{
   if (  pszKey
      && *pszKey
      && pszValue
      && *pszValue
      )
   {
      std::string stringKey = pszKey;

#ifndef RETAIL
      if (vfDebug) std::cerr << "Adding: (" << std::hex << &mapDJP_d << std::dec << ") " << pszKey << "=\"" << pszValue << "\"" << std::endl; // @TBD
#endif

      mapDJP_d[stringKey] = std::string (pszValue);
   }
}

void JobProperties::
setJobProperties (PSZCRO pszJobProperties)
{
   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      JobPropertyEnumerator *pEnum = 0;

      pEnum = new JobPropertyEnumerator (pszJobProperties, 0, false);

      while (pEnum->hasMoreElements ())
      {
         PSZCRO pszKey   = pEnum->getCurrentKey ();
         PSZCRO pszValue = pEnum->getCurrentValue ();

#ifndef RETAIL
         if (vfDebug) std::cerr << "Adding: (" << std::hex << &mapDJP_d << std::dec << ") " << pszKey << "=\"" << pszValue << "\"" << std::endl; // @TBD
#endif

         mapDJP_d[std::string (pszKey)] = std::string (pszValue);

         pEnum->nextElement ();
      }

      delete pEnum;
   }
}

bool JobProperties::
hasJobProperty (PSZCRO pszKey)
{
   return mapDJP_d.find (std::string (pszKey)) != mapDJP_d.end ();
}

void JobProperties::
applyAllDebugOutput ()
{
   for ( DJPMap::iterator next = mapDJP_d.begin ();
         next != mapDJP_d.end () ;
         next++ )
   {
#ifndef RETAIL
      if (vfDebug) std::cerr << "DebugOutput: (" << std::hex << &mapDJP_d << std::dec << ") " << next->first << "=\"" << next->second << "\"" << std::endl; // @TBD
#endif

      if (next->first.compare ("debugoutput"))
      {
         DebugOutput::setDebugOutput (next->second.c_str ());
      }
   }
}

int JobProperties::
getNumProperties ()
{
   return mapDJP_d.size ();
}

void JobProperties::
standarizeJPOrder (std::ostringstream& oss,
                   std::string         stringJobProperties)
{
   JobProperties jp (stringJobProperties.c_str ());
   PSZCRO        pszJP                             = jp.getJobProperties ();

   if (pszJP)
   {
      oss << pszJP;

      free ((void *)pszJP);
   }
}

JobProperties::DJPMap& JobProperties::
getDJPMap ()
{
   return mapDJP_d;
}

#ifndef RETAIL

void JobProperties::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string JobProperties::
toString (std::ostringstream& oss)
{
   std::ostringstream oss2;

   oss << "{JobProperties: ";

   for ( DJPMap::iterator next = mapDJP_d.begin ();
         next != mapDJP_d.end () ;
         next++ )
   {
      oss << "[" << next->first << " = \"" << next->second << "\"]";
   }

   oss << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream&        os,
            const JobProperties& const_self)
{
   JobProperties&     self = const_cast<JobProperties&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

JobPropertyEnumerator::
JobPropertyEnumerator (PSZRO  pszJobProperties,
                       PSZCRO pszKey,
                       bool   fChange)
{
   pszJobProperties_d   = 0;
   pszKey_d             = 0;
   pszCurrentProperty_d = 0;
   pszCurrentKey_d      = 0;
   pszCurrentValue_d    = 0;
   pszCurrentMinValue_d = 0;
   pszCurrentMaxValue_d = 0;
   fChange_d            = fChange;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      int cbJobProperties = strlen (pszJobProperties) + 1;

      // Skip double quotes around everything
      while (  0 < cbJobProperties
            && '"' == *pszJobProperties
            && '"' == *(pszJobProperties + cbJobProperties - 2)
            )
      {
         pszJobProperties++;
         cbJobProperties -= 2;
      }

      if (0 < cbJobProperties)
      {
         pszJobProperties_d = (char *)malloc (cbJobProperties);

         if (pszJobProperties_d)
         {
            strncpy (pszJobProperties_d, pszJobProperties, cbJobProperties);
            pszJobProperties_d[cbJobProperties - 1] = '\0';

            pszCurrentProperty_d = pszJobProperties_d;
         }
      }
   }
   if (  pszKey
      && *pszKey
      )
   {
      pszKey_d = (char *)malloc (strlen (pszKey) + 1);

      if (pszKey_d)
      {
         strcpy (pszKey_d, pszKey);
      }
   }

#ifndef RETAIL
///if (vfDebug) PRINT_VAR (pszJobProperties_d);
///if (vfDebug) PRINT_VAR (pszKey_d);
///if (vfDebug) PRINT_VAR (pszCurrentProperty_d);
#endif

   nextElement ();
}

JobPropertyEnumerator::
~JobPropertyEnumerator ()
{
   if (pszJobProperties_d)
   {
      free (pszJobProperties_d);
      pszJobProperties_d = 0;
   }
   if (pszKey_d)
   {
      free (pszKey_d);
      pszKey_d = 0;
   }
}

bool JobPropertyEnumerator::
hasMoreElements ()
{
   if (  pszCurrentKey_d
      && pszCurrentValue_d
      )
   {
      return true;
   }
   else
   {
      return false;
   }
}

void JobPropertyEnumerator::
nextElement ()
{
   do
   {
      readNextKey ();

#ifndef RETAIL
//////if (vfDebug) PRINT_VAR (pszKey_d);
//////if (vfDebug) PRINT_VAR (pszCurrentProperty_d);
//////if (vfDebug) PRINT_VAR (pszCurrentKey_d);
//////if (vfDebug) PRINT_VAR (pszCurrentValue_d);
//////if (vfDebug) std::cerr << "hasMoreElements () = " << hasMoreElements () << std::endl;
//////if (vfDebug) std::cerr << "---------------------" << std::endl;
#endif

   } while (  hasMoreElements ()
           && (  pszKey_d
              && 0 != strcmp (pszCurrentKey_d, pszKey_d)
              )
           );
}

PSZCRO JobPropertyEnumerator::
getCurrentKey ()
{
   return pszCurrentKey_d;
}

PSZCRO JobPropertyEnumerator::
getCurrentValue ()
{
   return pszCurrentValue_d;
}

PSZCRO JobPropertyEnumerator::
getCurrentMinimumValue ()
{
#ifndef RETAIL
   if (vfDebug)
   {
      if (  pszCurrentMinValue_d
         && 0 != strcmp (pszCurrentMinValue_d, "1")
         )
      {
         PRINT_VAR (pszCurrentMinValue_d);
      }
   }
#endif

   return pszCurrentMinValue_d;
}

PSZCRO JobPropertyEnumerator::
getCurrentMaximumValue ()
{
#ifndef RETAIL
   if (vfDebug)
   {
      if (  pszCurrentMaxValue_d
         && 0 != strcmp (pszCurrentMaxValue_d, "99999")
         )
      {
         PRINT_VAR (pszCurrentMaxValue_d);
      }
   }
#endif

   return pszCurrentMaxValue_d;
}

void JobPropertyEnumerator::
readNextKey ()
{
   if (  fChange_d
      && pszCurrentMinValue_d
      )
   {
      pszCurrentValue_d[-1]    = '{';
      pszCurrentMinValue_d[-1] = ',';

      if (pszCurrentMaxValue_d)
      {
         pszCurrentMaxValue_d[-1] = ',';
         pszCurrentMaxValue_d[strlen (pszCurrentMaxValue_d) + 1] = '}';
      }
   }

   pszCurrentKey_d      = 0;
   pszCurrentValue_d    = 0;
   pszCurrentMinValue_d = 0;
   pszCurrentMaxValue_d = 0;

   while (  pszCurrentProperty_d
         && *pszCurrentProperty_d
         && !pszCurrentKey_d
         && !pszCurrentValue_d
         )
   {
      char *pszSep    = pszCurrentProperty_d;
      char *pszEquals = 0;

      // Skip white space
      while (  ' '  == *pszSep
            || '\t' == *pszSep
            || '\r' == *pszSep
            || '\n' == *pszSep
            )
         *pszSep++ = '\0';

      pszEquals = strpbrk (pszSep, "=");

      if (pszEquals)
      {
         *pszEquals = '\0';

         pszCurrentKey_d   = pszSep;
         pszCurrentValue_d = pszEquals + 1;

         if ('\"' == *pszCurrentValue_d)
         {
            pszCurrentValue_d++;

            pszSep = pszCurrentValue_d;
            while (  *pszSep
                  && '\"' != *pszSep
                  )
            {
               pszSep++;
            }

            if ('\"' == *pszSep)
            {
               *pszSep++ = '\0';
            }
            else
            {
               // Error
               pszSep               = 0;
               pszCurrentKey_d      = 0;
               pszCurrentValue_d    = 0;
               pszCurrentMinValue_d = 0;
               pszCurrentMaxValue_d = 0;
            }
         }
         else
         {
            pszSep = strpbrk (pszCurrentValue_d, " \n\r\t");

            if (pszSep)
            {
               *pszSep++ = '\0';
            }
         }

         if (  fChange_d
            && pszCurrentValue_d
            )
         {
            PSZ pszLBrace = strchr (pszCurrentValue_d, '{');
            PSZ pszRBrace = strchr (pszCurrentValue_d, '}');

#ifndef RETAIL
////////////if (vfDebug) PRINT_VAR (pszLBrace);
////////////if (vfDebug) PRINT_VAR (pszRBrace);
#endif

            if (  pszLBrace
               && pszRBrace
               && pszLBrace < pszRBrace
               )
            {
               PSZ pszComma = 0;

               *pszLBrace = '\0';
               *pszRBrace = '\0';
               pszCurrentValue_d++;

               pszComma = strchr (pszCurrentValue_d, ',');

#ifndef RETAIL
///////////////if (vfDebug) PRINT_VAR (pszComma);
#endif

               if (pszComma)
               {
                  *pszComma = '\0';

                  pszCurrentMinValue_d = pszComma + 1;

                  pszComma = strchr (pszCurrentMinValue_d, ',');

#ifndef RETAIL
//////////////////if (vfDebug) PRINT_VAR (pszComma);
#endif

                  if (pszComma)
                  {
                     *pszComma = '\0';

                     pszCurrentMaxValue_d = pszComma + 1;
                  }
               }
            }
         }
      }
      else
      {
#ifndef RETAIL
         if (vfDebug) std::cerr << "Skipping: " << pszSep << std::endl; // @TBD
#endif

         pszSep = strpbrk (pszSep, " \n\r\t");

         if (pszSep)
         {
            *pszSep++ = '\0';
         }
      }

      if (  pszSep
         && *pszSep
         )
         pszCurrentProperty_d = pszSep;
      else
         pszCurrentProperty_d = 0;
   }
}

MultiJobPropertyEnumerator::
MultiJobPropertyEnumerator ()
{
   iCurrentElement_d = 0;
}

MultiJobPropertyEnumerator::
~MultiJobPropertyEnumerator ()
{
   for ( vectorElements::iterator next = vectorElements_d.begin () ;
         next != vectorElements_d.end () ;
         next++ )
   {
      JobProperties *pJP = (JobProperties *)*next;

      delete pJP;
   }
}

void MultiJobPropertyEnumerator::
addElement (JobProperties *pJP)
{
   if (pJP)
   {
      vectorElements_d.push_back (pJP);
   }
}

bool MultiJobPropertyEnumerator::
hasMoreElements ()
{
   return iCurrentElement_d < (int)vectorElements_d.size ();
}

void * MultiJobPropertyEnumerator::
nextElement ()
{
   void *pvRet = 0;

   if (iCurrentElement_d < (int)vectorElements_d.size ())
   {
      pvRet = (void *)vectorElements_d[iCurrentElement_d];

      // Caller is expected to free this, so zero it out now
      vectorElements_d[iCurrentElement_d] = 0;

      iCurrentElement_d++;
   }

   return pvRet;
}

#if 0
int
main (int argc, char *argv[])
{
   JobProperties          jobProp ("a={1,1,100} b=2 c=3");
   PSZRO                  pszJobProperties                 = 0;
   JobPropertyEnumerator *pEnum                            = 0;

   pszJobProperties = jobProp.getJobProperties ();

   if (pszJobProperties)
   {
      std::cout << "pszJobProperties = \"" << pszJobProperties << "\"" << std::endl;

      free ((void *)pszJobProperties);
   }

   pEnum = jobProp.getEnumeration ();

   if (pEnum)
   {
      while (pEnum->hasMoreElements ())
      {
         PSZCRO pszKey      = pEnum->getCurrentKey          ();
         PSZCRO pszValue    = pEnum->getCurrentValue        ();
         PSZCRO pszMinValue = pEnum->getCurrentMinimumValue ();
         PSZCRO pszMaxValue = pEnum->getCurrentMaximumValue ();

         std::cout << "pszKey      = \"" << SAFE_PRINT_PSZ(pszKey) << "\"" << std::endl;
         std::cout << "pszValue    = \"" << SAFE_PRINT_PSZ(pszValue) << "\"" << std::endl;
         std::cout << "pszMinValue = \"" << SAFE_PRINT_PSZ(pszMinValue) << "\"" << std::endl;
         std::cout << "pszMaxValue = \"" << SAFE_PRINT_PSZ(pszMaxValue) << "\"" << std::endl;

         pEnum->nextElement ();
      }

      delete pEnum;
   }

   return 0;
}
#endif
