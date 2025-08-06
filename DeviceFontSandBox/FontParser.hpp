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
 *   $Id: FontParser.hpp,v 1.2 2002/11/04 06:12:18 rcktscientist Exp $
 */

#ifndef _FontParser
#define _FontParser

namespace DevFont
{

class CommandSequence;
class DeviceFont;
class Event;
class Parameter;

class FontParser
{
public:
  /** Specify a default constructor (since we have other private constructors). */
  FontParser () {}

  /** Constructor is virtual since this is a base class. */
  virtual ~FontParser ();

  /** Return a list of all the fonts. This is a pure virtual method*/
  virtual DeviceFont *getFonts() const = 0;

  /** Return a list of command sequences. A pure virtual method*/
  virtual CommandSequence *getCmdSeqs() const = 0;

  /** Return a list of events. A pure virtual method*/
  virtual Event *getEvents() const = 0;

  /** Return a list of parameters. A pure virtual method*/
  virtual Parameter *getParameters() const = 0;

  /**
   * Construct a path to one of the subsidiary files based on the directory
   * that the primary file is in.  Here is one of the Unix/Linux dependencies:
   * we are assuming that the path separator is the "/" character.  The caller
   * is responsible for deleting this string.
   *
   * @param path  full path to the primary file
   * @param name  name of the secondary file
   */
   const char *getPath (const char *path, const char *name) const;
  
private:
  /** Disallow calling the copy constructor. */
  FontParser (const FontParser &);

  /** Disallow calling the assignment operator. */
  void operator= (const FontParser &);
};

}

#endif
