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
#include "DeviceString.hpp"
#include "DebugOutput.hpp"

#include <vector>

DeviceString::
DeviceString ()
{
   psmeCurrent_d = 0;
}

DeviceString::
~DeviceString ()
{
   for ( StringMapLanguage::iterator nextLanguage = languages_d.begin () ;
         nextLanguage != languages_d.end () ;
         nextLanguage++ )
   {
      StringMapElement *pEntries = (StringMapElement *)nextLanguage->second;

      delete pEntries;
   }

   psmeCurrent_d = 0;
}

bool DeviceString::
setLanguage (int iLanguageID)
{
   PSZCRO            pszLanguageID      = StringResource::IDToName (iLanguageID);
   StringMapElement *psme               = 0;
   std::string       stringLanguageName = pszLanguageID;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceString ()) DebugOutput::getErrorStream () << "DeviceString::" << __FUNCTION__ << ": setLanguage (" << iLanguageID << ") pszLanguageID = " << pszLanguageID << std::endl;
#endif

   psme = languages_d[stringLanguageName];

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceString ()) DebugOutput::getErrorStream () << "DeviceString::" << __FUNCTION__ << ": psme = " << psme << std::endl;
#endif

   if (psme)
   {
      psmeCurrent_d = psme;

      return true;
   }
   else
   {
      return false;
   }
}

class LanguageEnumeration : public Enumeration
{
public:
   LanguageEnumeration ()
   {
      iCurrentElement_d = 0;
   }

   virtual
   ~LanguageEnumeration ()
   {
   }

   bool
   hasMoreElements ()
   {
      return iCurrentElement_d < (int)vectorElements_d.size ();
   }

   void *
   nextElement ()
   {
      const std::string *pstringElement = 0;
      void              *pvRet          = 0;

      if (iCurrentElement_d >= (int)vectorElements_d.size ())
      {
         return 0;
      }

      pstringElement = vectorElements_d[iCurrentElement_d++];

      if (pstringElement)
      {
         pvRet = (void *)pstringElement->c_str ();
      }

      return pvRet;
   }

   void
   addElement (const std::string *pstringElement)
   {
      vectorElements_d.push_back (pstringElement);
   }

private:
   std::vector <const std::string *> vectorElements_d;
   int                               iCurrentElement_d;
};

Enumeration * DeviceString::
getLanguages ()
{
   LanguageEnumeration *pRet = new LanguageEnumeration ();

   for ( StringMapLanguage::iterator nextLanguage = languages_d.begin () ;
         nextLanguage != languages_d.end () ;
         nextLanguage++ )
   {
      pRet->addElement (&nextLanguage->first);
   }

   return pRet;
}

StringResource * DeviceString::
getLanguageResource ()
{
   return this;
}

PSZCRO DeviceString::
getStringV (int iGroup,
            int iID)
{
   return 0;
}

PSZCRO DeviceString::
getStringV (int               iGroup,
            PSZCRO pszID)
{
   if (  psmeCurrent_d
      && pszID
      && *pszID
      )
   {
      std::string  stringInternal  = pszID;
      std::string *pstringExternal = 0;

      pstringExternal = &(*psmeCurrent_d)[stringInternal];

#ifndef RETAIL
      if (DebugOutput::shouldOutputDeviceString ()) DebugOutput::getErrorStream () << "DeviceString::" << __FUNCTION__ << ": getStringV (" << stringInternal << ") = " << (pstringExternal ? *pstringExternal : "NULL") << std::endl;
#endif

      if (pstringExternal)
      {
         return pstringExternal->c_str ();
      }
   }

   return 0;
}

void DeviceString::
add (PSZCRO pszLanguage,
     PSZCRO pszInternal,
     PSZCRO pszExternal)
{
   if (  !pszLanguage
      || !pszInternal
      || !pszExternal
      || !*pszLanguage
      || !*pszInternal
      || !*pszExternal
      )
   {
      return;
   }

   std::string       stringLanguageName = pszLanguage;
   std::string       stringInternal     = pszInternal;
   std::string       stringExternal     = pszExternal;
   StringMapElement *psme               = 0;

   psme = languages_d[stringLanguageName];

   if (!psme)
   {
      psme = new StringMapElement ();

      languages_d[stringLanguageName] = psme;
   }

   (*psme)[stringInternal] = stringExternal;

#ifndef RETAIL
   if (DebugOutput::shouldOutputDeviceString ()) DebugOutput::getErrorStream () << "DeviceString::" << __FUNCTION__ << ": add (" << stringLanguageName << ", " << stringInternal << ", " << stringExternal << ")" << std::endl;
#endif
}

std::string DeviceString::
toString (std::ostringstream& oss)
{
   oss << "{ "
       << "# languages = "
       << languages_d.size ()
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceString& const_self)
{
   DeviceString&      self = const_cast<DeviceString&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
