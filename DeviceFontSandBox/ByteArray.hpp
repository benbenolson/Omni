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
 *   Sept. 29, 2002
 *   $Id: ByteArray.hpp,v 1.3 2002/10/30 23:48:58 rcktscientist Exp $
 */

#ifndef _ByteArray
#define _ByteArray

#include <iostream>

namespace DevFont
{

/**
 * This class represents an array of bytes.  This is used as a
 * return value from the DeviceFontMgr setFont() method (among others) to
 * return the bytes.  We can't use a char* in this case, since the string can
 * be bytes, which might contain embedded null bytes.  In this case, there is
 * no way of getting the length of the array.
 *
 * @author David L. Wagner
 * @version 1.0
 */
class ByteArray
{
public:
  /** The constructor. */
  ByteArray (const unsigned char *, int);

  /** The copy constructor. */
  ByteArray (const ByteArray &);

  /** The assignment operator. */
  ByteArray & operator= (const ByteArray &);

  /** The destructor. */
  virtual ~ByteArray ();

  /** Return the length of data. */
  inline int getLength () const { return length; }

  /** Return the array of bytes.  The caller must not delete[] this. */
  inline const unsigned char * getBytes() const { return data; }

  /** Concatenate two ByteArray objects. */
  friend ByteArray operator+ (const ByteArray &, const ByteArray &);

  /** Print the bytes in this byte array. */
  friend std::ostream & operator<< (std::ostream &, const ByteArray &);

  /** Get an array of ASCII characters (for debugging only). */
  char * getASCII () const;

private:
  /** The length of the data. */
  int length;

  /** The actual data. */
  const unsigned char *data;
};

/** Print the bytes in this byte array. */
std::ostream & operator<< (std::ostream &, const ByteArray &);

}

#endif
