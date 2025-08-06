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
 *   Oct. 29, 2002
 *   $Id: FontParser.cpp,v 1.2 2002/11/04 06:12:17 rcktscientist Exp $
 */

#include "FontParser.hpp"
#include <string>

using namespace std;
using namespace DevFont;

/**
 * Virtual destructor.
 */
FontParser::~FontParser ()
{
}


/**
 * Construct a path to one of the subsidiary files based on the directory
 * that the primary file is in.  Here is one of the Unix/Linux dependencies:
 * we are assuming that the path separator is the "/" character.  The caller
 * is responsible for deleting this string.
 *
 * @param path  full path to the primary file
 * @param name  name of the secondary file
 */
const char *FontParser::getPath (const char *path, const char *name) const
{
  //
  // First find the last "/" character, if any
  //
  int len = strlen (path);
  int index = -1;
  for (int i = len-1; i >= 0; i--)
    if (path[i] == '/')
      {
        index = i;
        break;
      }
  //
  // Now copy the characters
  //
  int rlen = (index+1) + strlen(name) + 1 + 4;
  char * result = new char[rlen];
  result[0] = 0;
  if (index > -1)
    strncpy (result,path,index+1);
  strcpy (result,name);
  //
  // Add .xml to the end
  //
  strcat (result, ".xml");
  return result;
}
