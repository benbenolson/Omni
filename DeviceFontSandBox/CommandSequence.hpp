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
 *   $Id: CommandSequence.hpp,v 1.3 2002/11/05 07:01:47 rcktscientist Exp $
 */

#ifndef _CommandSequence
#define _CommandSequence

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
class CommandSequence
{
public:
  /** The constructor. */
  CommandSequence(const char *, const char *);

  /** The copy constructor. */
  CommandSequence (const CommandSequence &);

  /** The assignment operator. */
  CommandSequence & operator= (const CommandSequence &);

  /** The destructor. */
  ~CommandSequence();

  /** Add a node to the end of the list. */
  void setNext (CommandSequence *);

  /** Gets the next node in the list. */
  const CommandSequence *getNext() const { return next; }

  /** Get the name of this node. */
  const char *getName () const { return name; }

  /** Get the value of this node. */
  const char *getValue() const { return value; }

  /** Print out the entire linked list of nodes. */
  void print () const;

  /** Print out the structure for debugging. */
  friend std::ostream & operator<< (std::ostream &, const CommandSequence &);

private:
  /** The name of this node. */
  const char *name;

  /** The value of this node. */
  const char *value;

  /**
   * The next node in the list.  The tail of the list will have this pointer
   * set to null.
   */
  CommandSequence *next;

  /** Function to copy a string. */
  const char * copy (const char *);
};

std::ostream & operator<< (std::ostream &, const CommandSequence &);

}

#endif
