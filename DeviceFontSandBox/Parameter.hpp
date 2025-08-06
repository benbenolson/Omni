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
 *   Oct. 30, 2002
 *   $Id: Parameter.hpp,v 1.3 2002/11/05 07:01:47 rcktscientist Exp $
 */

#ifndef _Parameter
#define _Parameter

#include <iostream>

namespace DevFont
{

/**
 * This class represents a single node in a linked list of command sequence
 * information.
 *
 * @author David L. Wagner
 * @version 1.0
 */
class Parameter
{
public:
  /**
   * The constructor just records the name and value of this node.
   *
   * @param n  name of this node; this Parameter owns this string now
   * @param v  value of this node; this Parameter owns this string now
   */
  Parameter(const char *n, const char *v);

  /** The copy constructor. */
  Parameter (const Parameter &);

  /** The assignment operator. */
  Parameter & operator= (const Parameter &);

  /** The destructor. */
  ~Parameter();

  /** Add a node to the end of the list. */
  void setNext (Parameter *);

  /** Gets the next node in the list. */
  const Parameter *getNext() const { return next; }

  /** Get the name of this node. */
  const char *getName () const { return name; }

  /** Get the value of this node. */
  const char *getValue() const { return value; }

  /** Print out the entire linked list of nodes. */
  void print () const;

  /** Print out the structure for debugging. */
  friend std::ostream & operator<< (std::ostream &, const Parameter &);

private:
  /** The name of this node. */
  const char *name;

  /** The value of this node. */
  const char *value;

  /**
   * The next node in the list.  The tail of the list will have this pointer
   * set to null.
   */
  Parameter *next;

  /** Function to copy a string. */
  const char * copy (const char *);
};

/** Print out the structure for debugging. */
std::ostream & operator<< (std::ostream &, const Parameter &);

}

#endif
