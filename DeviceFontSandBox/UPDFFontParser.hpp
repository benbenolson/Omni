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
 *   Nov. 13, 2002
 *   $Id: UPDFFontParser.hpp,v 1.8 2004/02/17 16:23:56 hamzy Exp $
 */

#ifndef _UPDFFontParser
#define _UPDFFontParser
#include "FontParser.hpp"
#include "XmlFile.hpp"

namespace DevFont
{

  class UPDFFontParser : public FontParser
  {
  public:
    /** Construct a new parser to read a particular file. */
    explicit UPDFFontParser (const char *path);

    /** Construct a parser that reads the two files. */
    UPDFFontParser (const char *ddPath, const char *csPath);

    /** Delete the parser when we are done. */
    virtual ~UPDFFontParser();

    /** Return a list of all the fonts. */
    virtual DeviceFont *getFonts() const;

    /** Return a list of command sequences. */
    virtual CommandSequence *getCmdSeqs() const;

    /** Return a list of events. */
    virtual Event *getEvents() const;

    /** Return a list of parameters. */
    virtual Parameter *getParameters() const;

  private:
    /** The head of the list of fonts. */
    DeviceFont *myFonts;

    /** The head of the list of the command sequences. */
    CommandSequence *myCmdSeqs;

    /** The head of the list of events. */
    Event *myEvents;

    /** The head of the list of parameters. */
    Parameter *myParameters;

    /** Common initialization. */
    void init (const char *ddPath, const char *csPath);

    /** Disallow calling the copy constructor. */
    UPDFFontParser (const UPDFFontParser &);

    /** Disallow calling the assignment operator. */
    void operator= (const UPDFFontParser &);

    /** Find all the font Elements. */
    DeviceFont *findFonts (const XmlNodePtr) const;

    /** Find all the command sequence Elements. */
    CommandSequence *findCmdSeqs (const XmlNodePtr) const;

    /** Find all the event Elements. */
    Event *findEvents (const XmlNodePtr) const;

    /** Find all the parameter Elements. */
    Parameter *findParameters (const XmlNodePtr) const;

    /** Convert a DeviceFont Element into a DeviceFont object. */
    DeviceFont *getDeviceFont (const XmlNodePtr) const;

    /** Convert a CommandSequence Element into a CommandSequence object. */
    CommandSequence *getCommandSequence (const XmlNodePtr) const;

    /** Convert an Event Element into an Event object. */
    Event *getEvent (const XmlNodePtr) const;

    /** Convert an Parameter Element into an Parameter object. */
    Parameter *getParameter (const XmlNodePtr) const;

    /** Copy global metrics information from the Element to the DeviceFont. */
    void getGlobalMetrics (DeviceFont*, const XmlNodePtr) const;

    /** Copy panose information from the Element to the DeviceFont. */
    void getPanose(DeviceFont*, const XmlNodePtr) const;

    /** Copy name ID information from the Element to the DeviceFont. */
    void getNameIDs(DeviceFont*, const XmlNodePtr) const;

    /** Convert an attribute of an Element into an integer. */
    int getInt (const XmlNodePtr, const char *) const;

    /** Convert an attribute of an element into a character array. */
    char *getAttribute (const XmlNodePtr, const char*) const;
  };

}

#endif
