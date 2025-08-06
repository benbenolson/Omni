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
 *   $Id: Event.cpp,v 1.2 2002/10/30 23:48:58 rcktscientist Exp $
 */

#include "Event.hpp"

#include <string>

using namespace std;
using namespace DevFont;

const char * Event::START = "AtStartOf";
const char * Event::END = "AtEndOf";

/**
 * The constructor just sets the all the member variables to their default
 * values.
 */
Event::Event (const char * i, const char * s, const char * t, const char * c):
  id(i), startend(s), type(t), cmdseq(c), next(NULL)
{
}


/**
 * The copy constructor.
 */
Event::Event (const Event &event):
  next(NULL)
{
  id       = copy (event.id);
  startend = copy (event.startend);
  type     = copy (event.type);
  cmdseq   = copy (event.cmdseq);
}


/**
 * Copy a string.
 */
const char * Event::copy (const char *string)
{
  if (string == NULL)
    return NULL;
  char * s = new char[1+strlen(string)];
  strcpy (s, string);
  return s;
}


/**
 * Since we overrode the copy constructor, we must also override the assignment
 * operator.
 *
 * @param node  the node to copy
 */
Event & Event::operator= (const Event & event)
{
  //
  // Check to see if we are assigning to ourself
  //
  if (this == &event)
    return *this;
  //
  // Delete the old values
  //
  if (id)       delete [] id;
  if (startend) delete [] startend;
  if (type)     delete [] type;
  if (cmdseq)   delete [] cmdseq;
  next = NULL;
  //
  // And now set the new values
  //
  id       = copy (event.id);
  startend = copy (event.startend);
  type     = copy (event.type);
  cmdseq   = copy (event.cmdseq);
  //
  // And return a reference to ourself
  //
  return *this;
}


/**
 * The destructor deallocates the memory that we own.
 */
Event::~Event ()
{
  delete [] id;
  delete [] startend;
  delete [] type;
  delete [] cmdseq;
  //
  // And null these for good measure
  //
  id = NULL;
  startend = NULL;
  type = NULL;
  cmdseq = NULL;
  next = NULL;
}


/**
 * Insert the given node into the linked list of nodes.
 *
 * @param anode  the node to add
 */
void Event::setNext (Event *anode)
{
  anode->next = next;
  next = anode;
}


namespace DevFont
{
/** Print out the event for debugging. */
ostream & DevFont::operator<< (ostream & s, const Event & ev)
{
  s << "Event[" << ev.id << " = \"" << ev.cmdseq << "\" ("
    << ev.startend << " " << ev.type << ")]";
  return s;
}
}
