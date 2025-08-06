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
#ifndef _DeviceString
#define _DeviceString

#include "Enumeration.hpp"
#include "StringResource.hpp"

#include <map>
#include <iostream>
#include <sstream>
#include <string>

class DeviceString : public StringResource
{
public:
                        DeviceString        ();
   virtual             ~DeviceString        ();

   bool                 setLanguage         (int                  iLanguageID);
   Enumeration         *getLanguages        ();
   StringResource      *getLanguageResource ();

   PSZCRO               getStringV          (int                  iGroup,
                                             int                  iID);
   PSZCRO               getStringV          (int                  iGroup,
                                             PSZCRO               pszID);

   virtual std::string  toString            (std::ostringstream&  oss);
   friend std::ostream& operator<<          (std::ostream&        os,
                                             const DeviceString&  self);

   void                 add                 (PSZCRO               pszLanguage,
                                             PSZCRO               pszInternal,
                                             PSZCRO               pszExternal);

   typedef std::map <std::string, std::string>        StringMapElement;
   typedef std::map <std::string, StringMapElement *> StringMapLanguage;

   StringMapLanguage  languages_d;
   StringMapElement  *psmeCurrent_d;
};

#endif
