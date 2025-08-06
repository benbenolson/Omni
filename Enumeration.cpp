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
#include "Enumeration.hpp"
#include "JobProperties.hpp"

ArrayEnumerator::
ArrayEnumerator (int cKeys, PSZCRO *aKeys)
{
   iKey_d  = 0;
   cKeys_d = cKeys;
   aKeys_d = aKeys;
}

ArrayEnumerator::
~ArrayEnumerator ()
{
}

bool ArrayEnumerator::
hasMoreElements ()
{
   if (iKey_d < cKeys_d)
      return true;
   else
      return false;
}

void * ArrayEnumerator::
nextElement ()
{
   if (iKey_d > cKeys_d - 1)
      return 0;

   return (void *)aKeys_d[iKey_d++];
}

SpaceEnumerator::
SpaceEnumerator (PSZRO pszKeys)
{
   pszKeys_d    = 0;
   pszCurrent_d = 0;

   if (  pszKeys
      && *pszKeys
      )
   {
      pszKeys_d = (char *)malloc (strlen (pszKeys) + 1);
      if (pszKeys_d)
      {
         strcpy (pszKeys_d, pszKeys);
         pszCurrent_d = pszKeys_d;
      }
   }
}

SpaceEnumerator::
~SpaceEnumerator ()
{
   if (pszKeys_d)
   {
      free (pszKeys_d);
      pszKeys_d = 0;
   }
}

bool SpaceEnumerator::
hasMoreElements ()
{
   return pszCurrent_d ? true : false;
}

void * SpaceEnumerator::
nextElement ()
{
   void *pvRet = pszCurrent_d;

   if (  pszCurrent_d
      && *pszCurrent_d
      )
   {
      char *pszSpace = strchr (pszCurrent_d, ' ');

      if (pszSpace)
      {
         *pszSpace = '\0';
         pszCurrent_d = pszSpace + 1;
         while (' ' == *pszCurrent_d)
         {
            pszCurrent_d++;
         }
         if (!*pszCurrent_d)
         {
            pszCurrent_d = 0;
         }
      }
      else
      {
         pszCurrent_d = 0;
      }
   }
   else
   {
      pszCurrent_d = 0;
   }

   return pvRet;
}

NullEnumerator::
NullEnumerator ()
{
}

NullEnumerator::
~NullEnumerator ()
{
}

bool NullEnumerator::
hasMoreElements ()
{
   return false;
}

void * NullEnumerator::
nextElement ()
{
   return 0;
}

ListEnumerator::
ListEnumerator ()
{
   iCurrentElement_d = 0;
}

ListEnumerator::
~ListEnumerator ()
{
}

void ListEnumerator::
addElement (PSZCRO pszValue)
{
   list_d.push_back (std::string (pszValue));
}

bool ListEnumerator::
hasMoreElements ()
{
   return iCurrentElement_d < (int)list_d.size ();
}

void * ListEnumerator::
nextElement ()
{
   void *pvRet = 0;

   if (iCurrentElement_d < (int)list_d.size ())
   {
      std::string& pValue = list_d[iCurrentElement_d];

      pvRet = (void *)pValue.c_str ();

      iCurrentElement_d++;
   }

   return pvRet;
}

EnumEnumerator::
EnumEnumerator ()
{
   iCurrentElement_d = 0;
}

EnumEnumerator::
~EnumEnumerator ()
{
   for ( vectorElements::iterator next = vectorElements_d.begin () ;
         next != vectorElements_d.end () ;
         next++ )
   {
      Enumeration *pEnum = (Enumeration *)*next;

      delete pEnum;
   }
}

void EnumEnumerator::
addElement (Enumeration *pEnum)
{
   if (pEnum)
   {
      vectorElements_d.push_back (pEnum);
   }
}

bool EnumEnumerator::
hasMoreElements ()
{
   return iCurrentElement_d < (int)vectorElements_d.size ();
}

void * EnumEnumerator::
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

ObjectEnumerator::
ObjectEnumerator ()
{
   iCurrentElement_d = 0;
}

ObjectEnumerator::
~ObjectEnumerator ()
{
///// Unfortunately no way to call object delete?
///for ( vectorElements::iterator next = vectorElements_d.begin () ;
///      next != vectorElements_d.end () ;
///      next++ )
///{
///   void *pvObject = (void *)*next;
///
///   delete pvObject;
///}
}

void ObjectEnumerator::
addElement (void *pvObject)
{
   if (pvObject)
   {
      vectorElements_d.push_back (pvObject);
   }
}

bool ObjectEnumerator::
hasMoreElements ()
{
   return iCurrentElement_d < (int)vectorElements_d.size ();
}

void * ObjectEnumerator::
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

StringArrayJPEnumeration::
StringArrayJPEnumeration (PSZRO pszArray,
                          int   cbArray)
{
   pszAllocated_d = 0;
   pszCurrent_d   = 0;
   cbLength_d     = 0;

   if (  pszArray
      && cbArray
      )
   {
      cbLength_d     = cbArray;
      pszAllocated_d = (PSZ)malloc (cbLength_d);
      pszCurrent_d   = pszAllocated_d;

      if (pszAllocated_d)
      {
         memcpy (pszAllocated_d, pszArray, cbLength_d);
      }
   }
}

StringArrayJPEnumeration::
~StringArrayJPEnumeration ()
{
   if (pszAllocated_d)
   {
      free ((void *)pszAllocated_d);
   }
}

bool StringArrayJPEnumeration::
hasMoreElements ()
{
   if (0 == cbLength_d)
   {
      return false;
   }

   return true;
}

void * StringArrayJPEnumeration::
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
