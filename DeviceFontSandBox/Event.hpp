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
 *   October 15, 2002
 *   $Id: Event.hpp,v 1.3 2002/11/05 07:01:47 rcktscientist Exp $
 */

#ifndef _Event
#define _Event

#include <iostream>

namespace DevFont
{

/**
 * This class represents a single node in a linked list of UPDF events.
 *
 * @author David L. Wagner
 * @version 1.0
 */
class Event
{
public:
  /** The command sequence occurs at the start of the event. */
  static const char *START;

  /** The command sequence occurs at the end of the event. */
  static const char *END;

  /** The constructor. */
  Event (const char *, const char *, const char *, const char *);

  /** The copy constructor. */
  Event (const Event &);

  /** The assignment operator. */
  Event & operator= (const Event &);

  /** The destructor. */
  ~Event();

  /** Add a node to the end of the list. */
  void setNext (Event *);

  /** Gets the next node in the list. */
  inline const Event* getNext() const { return next; }
  
  /** Return the ID of this event. */
  const char *getID() const { return id; }

  /** Get the position of the event. */
  inline const char *getStartEnd() const { return startend; }

  /** Get the type of the event. */
  inline const char *getType() const { return type; }

  /** Get the name of the corresponding command sequence. */
  inline const char *getCmdSeq() const { return cmdseq; }

  /** Print out the structure for debugging. */
  void print() const;
  
  /** Print out a single node. */
  friend std::ostream & operator<< (std::ostream &, const Event &);

private:
  /** The id of this node. */
  const char *id;

  /** The position of this event. */
  const char *startend;

  /** The type of event. */
  const char *type;

  /** The command sequence that contains the instructions for this event. */
  const char *cmdseq;

  /**
   * The next node in the list.  The tail of the list will have this pointer
   * set to null.
   */
  Event *next;

  /** Function to copy a string. */
  const char * copy (const char *);
};

/** Print out a single node. */
std::ostream & operator<< (std::ostream &, const Event &);

}

#endif
