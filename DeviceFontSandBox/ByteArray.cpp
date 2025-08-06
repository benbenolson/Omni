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
 *   Author: David L. Wagner
 *   Sept. 25, 2002
 *   $Id: ByteArray.cpp,v 1.4 2002/11/12 06:46:18 rcktscientist Exp $
 */

#include "ByteArray.hpp"

using namespace DevFont;
using namespace std;

/**
 * The constructor just remembers the data and its length.  After this call,
 * we own the data.
 *
 * @param d  array of bytes (constructed, of course, with new[])
 * @param l  length of the array of bytes
 */
ByteArray::ByteArray (const unsigned char * d, int l)
{
  data = d;
  length = l;
}

/**
 * The copy constructor.
 */
ByteArray::ByteArray (const ByteArray &ba)
{
  length = ba.getLength();
  const unsigned char *olddata = ba.getBytes();
  //
  // Create a copy of the old data
  //
  unsigned char *newdata = new unsigned char[length];
  for (int i = 0; i < length; i++)
    newdata[i] = olddata[i];
  data = newdata;
}

/**
 * The assignment operator.
 */
ByteArray & ByteArray::operator= (const ByteArray &ba)
{
  if (this == &ba)
    return *this;
  //
  // Delete the old data
  //
  delete [] data;
  //
  // Now copy the old value to the new values
  //
  length = ba.getLength();
  const unsigned char *olddata = ba.getBytes();
  //
  // Create a copy of the old data
  //
  unsigned char *newdata = new unsigned char[length];
  for (int i = 0; i < length; i++)
    newdata[i] = olddata[i];
  data = newdata;
  return *this;
}

/**
 * We own the data, so we must destroy it.
 */
ByteArray::~ByteArray ()
{
  delete[] data;
}

namespace DevFont
{
/**
 * Concatenate two ByteArray objects.  This might be useful at some time.
 */
ByteArray operator+ (const ByteArray &lhs, const ByteArray &rhs)
{
  //
  // Get the length of the resulting data
  //
  int len1 = lhs.getLength();
  int len2 = rhs.getLength();
  int len = len1 + len2;
  //
  // Create the new data and copy it
  //
  const unsigned char *data1 = lhs.getBytes();
  const unsigned char *data2 = rhs.getBytes();
  unsigned char *data = new unsigned char[len];
  for (int i = 0; i < len1; i++)
    data[i] = data1[i];
  for (int i = 0; i < len2; i++)
    data[i+len1] = data2[i];
  return ByteArray (data, len);
}

/** Print the bytes in this byte array. */
std::ostream & operator<< (std::ostream &s, const ByteArray &ba)
{
    s << "ByteArray[";
    for (int i = 0; i < ba.length; i++)
    {
        if (i > 0)
            s << " ";
        int x = (int)ba.data[i];
        s << std::hex << x;
    }
    s << "]";
    return s;
}
}


/** Get an array of ASCII characters (for debugging only). */
char * ByteArray::getASCII () const
{
    char *result = new char[length+1];
    for (int i = 0; i < length; i++)
        if (data[i] >= 32 && data[i] < 127)
            result[i] = static_cast<char>(data[i]);
        else
            result[i] = '.';
    result[length] = 0;
    return result;
}
