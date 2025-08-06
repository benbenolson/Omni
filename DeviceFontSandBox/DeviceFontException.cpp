/*
 *   IBM Linux Device Font Library
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
 *
 *   Author: Julie Zhuo
 *   Nov. 07, 2002
 *   $Id: DeviceFontException.cpp,v 1.2 2002/11/08 23:37:05 jzhuo Exp $
 */


#include "DeviceFontException.hpp"

#include <string>


using namespace std;
using namespace DevFont;

/** Constructor */
DeviceFontException::DeviceFontException (const char* m)
{
  message = new char[strlen(m) + 1];
  strcpy (message, m);
}

/** Copy constructor */
DeviceFontException::DeviceFontException (const DeviceFontException& e)
{
  message = new char [strlen(e.message) + 1];
  strcpy (message, e.message);
}


/** Destructor */
DeviceFontException::~DeviceFontException ()
{
  if (message!=NULL)
    delete [] message;
  message = NULL;
}


