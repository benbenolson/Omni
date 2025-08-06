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
#ifndef _JobProperties
#define _JobProperties

#include "Device.hpp"

#include <map>

class JobPropertyEnumerator
{
public:
                          JobPropertyEnumerator  (PSZCRO               pszJobProperties,
                                                  PSZCRO               pszKey = 0,
                                                  bool                 fChange = true);
   virtual               ~JobPropertyEnumerator  ();

   bool                   hasMoreElements        ();
   void                   nextElement            ();
   PSZCRO                 getCurrentKey          ();
   PSZCRO                 getCurrentValue        ();
   PSZCRO                 getCurrentMinimumValue ();
   PSZCRO                 getCurrentMaximumValue ();

private:
   void                   readNextKey            ();

   char  *pszJobProperties_d;
   char  *pszKey_d;
   char  *pszCurrentProperty_d;
   char  *pszCurrentKey_d;
   char  *pszCurrentValue_d;
   char  *pszCurrentMinValue_d;
   char  *pszCurrentMaxValue_d;
   bool   fChange_d;
};

class JobProperties
{
public:
                          JobProperties         (PSZCRO               pszJobProperties = 0);
                          JobProperties         (std::string          stringJobProperties);
                          JobProperties         (Device              *pDevice);
   virtual               ~JobProperties         ();

   JobProperties&         operator=             (const JobProperties& jp);

   JobPropertyEnumerator *getEnumeration        (PSZCRO               pszKey = 0);

   PSZCRO                 getJobProperties      (bool                 fChange = true);

   void                   setJobProperty        (PSZCRO               pszKeyValue);
   void                   setJobProperty        (PSZCRO               pszKey,
                                                 PSZCRO               pszValue);

   void                   setJobProperties      (PSZCRO               pszJobProperties);

   bool                   hasJobProperty        (PSZCRO               pszKey);

   void                   applyAllDebugOutput   ();

   int                    getNumProperties      ();

   static void            standarizeJPOrder     (std::ostringstream&  oss,
                                                 std::string          stringJobProperties);
#ifndef RETAIL
   void                   outputSelf            ();
#endif
   virtual std::string    toString              (std::ostringstream&  oss);
   friend std::ostream&   operator<<            (std::ostream&        os,
                                                 const JobProperties& self);

private:
   typedef std::map <std::string, std::string> DJPMap;

   DJPMap&                getDJPMap             ();

   DJPMap mapDJP_d;
};

class MultiJobPropertyEnumerator : public Enumeration
{
public:
                 MultiJobPropertyEnumerator ();
   virtual      ~MultiJobPropertyEnumerator ();
   void          addElement                 (JobProperties *pJP);
   virtual bool  hasMoreElements            ();
   virtual void *nextElement                ();

private:
   typedef std::vector <JobProperties *> vectorElements;

   vectorElements vectorElements_d;
   int            iCurrentElement_d;
};

#endif
