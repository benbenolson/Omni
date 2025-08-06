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
 *   $Id: Parameter.cpp,v 1.1 2002/10/30 23:50:09 rcktscientist Exp $
 */

#include "Parameter.hpp"

#include <string>

using namespace std;
using namespace DevFont;

/**
 * The default constructor just records the name and value of this node.
 *
 * @param n  name of this node; this Parameter owns this string now
 * @param v  value of this node; this Parameter owns this string now
 */
Parameter::Parameter (const char *n, const char *v):
  name(n), value(v), next(NULL)
{
}


/**
 * The copy constructor just copies all the data (but not the next node).
 *
 * @param node  the node to copy
 */
Parameter::Parameter (const Parameter &node):
  next(NULL)
{
  //
  // We must allocate new memory for the strings
  //
  char *s = new char[1+strlen(node.name)];
  strcpy (s, node.name);
  name = s;
  s = new char[1+strlen(node.value)];
  strcpy (s, node.value);
  value = s;
}


/**
 * Since we overrode the copy constructor, we must also override the assignment
 * operator.
 *
 * @param node  the node to copy
 */
Parameter & Parameter::operator= (const Parameter & node)
{
  //
  // Check to see if we are assigning to ourself
  //
  if (this == &node)
    return *this;
  //
  // Delete the old values
  //
  if (name)  delete [] name;
  if (value) delete [] value;
  next = NULL;
  //
  // Copy the new values
  //
  name = copy (node.name);
  value = copy (node.value);
  //
  // And return a reference to ourself
  //
  return *this;
}


/**
 * Utility function to copy a string.
 */
const char * Parameter::copy (const char *string)
{
  if (string == NULL)
    return NULL;
  char * s = new char[1+strlen(string)];
  strcpy (s, string);
  return s;
}


/**
 * This destructor deletes the DeviceFont object as well as the siblings of
 * this node.  This should only be called on the head of the list (since the
 * previous node will then have a dangling pointer otherwise).
 */
Parameter::~Parameter()
{
  delete [] name;
  delete [] value;
  name = NULL;
  value = NULL;
  //
  // Set the next pointer to null, but don't delete it since we don't own it.
  //
  next = NULL;
}


/**
 * Insert the given node into the linked list of nodes.
 *
 * @param anode  the node to add
 */
void Parameter::setNext (Parameter *anode)
{
  anode->next = next;
  next = anode;
}


/**
 * For debugging: print out the information in this node and all following
 * nodes (most useful when applied to the node which is the list head).
 */
void Parameter::print() const
{
  const Parameter *par = this;
  int i=1;
  while (par != NULL)
    {
      cout << i << ": " << par->name << " --> " << par->value << endl;
      i++;
      par = par->next;
    }
}

namespace DevFont
{
/**
 * Print out the command sequence for debugging.
 */
ostream & operator<< (ostream & s, const Parameter & par)
{
  s << "Parameter[\"" << par.name << "\",\"" << par.value << "\"]";
  return s;
}
}
