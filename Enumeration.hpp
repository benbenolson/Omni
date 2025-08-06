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
#ifndef _Enunmeration
#define _Enunmeration

#include "defines.hpp"

#include <vector>
#include <string>

class Enumeration
{
public:
   virtual
   ~Enumeration ()
   {
   }

   virtual bool  hasMoreElements () = 0;
   virtual void *nextElement     () = 0;
};

class ArrayEnumerator : public Enumeration
{
public:
                 ArrayEnumerator          (int     cKeys,
                                           PSZCRO *aKeys);
   virtual      ~ArrayEnumerator          ();
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   int     iKey_d;
   int     cKeys_d;
   PSZCRO *aKeys_d;
};

class SpaceEnumerator : public Enumeration
{
public:
                 SpaceEnumerator          (PSZRO   pszKeys);
   virtual      ~SpaceEnumerator          ();
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   char *pszKeys_d;
   char *pszCurrent_d;
};

class NullEnumerator : public Enumeration
{
public:
                 NullEnumerator           ();
   virtual      ~NullEnumerator           ();
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();
};

class ListEnumerator : public Enumeration
{
public:
                 ListEnumerator           ();
   virtual      ~ListEnumerator           ();
   void          addElement               (PSZCRO  pszValue);
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   std::vector<std::string> list_d;
   int                      iCurrentElement_d;
};

class EnumEnumerator : public Enumeration
{
public:
                 EnumEnumerator           ();
   virtual      ~EnumEnumerator           ();
   void          addElement               (Enumeration *pEnum);
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   typedef std::vector <Enumeration *> vectorElements;

   vectorElements vectorElements_d;
   int            iCurrentElement_d;
};

class ObjectEnumerator : public Enumeration
{
public:
                 ObjectEnumerator         ();
   virtual      ~ObjectEnumerator         ();
   void          addElement               (void *pvObject);
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   typedef std::vector <void *> vectorElements;

   vectorElements vectorElements_d;
   int            iCurrentElement_d;
};

class StringArrayJPEnumeration : public Enumeration
{
public:
                 StringArrayJPEnumeration (PSZRO   pszArray,
                                           int     cbArray);
   virtual      ~StringArrayJPEnumeration ();
   virtual bool  hasMoreElements          ();
   virtual void *nextElement              ();

private:
   bool              fValid_d;
   PSZ               pszAllocated_d;
   PSZ               pszCurrent_d;
   int               cbLength_d;
};

#endif
