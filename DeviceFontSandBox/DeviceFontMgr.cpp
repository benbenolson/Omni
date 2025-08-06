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
 *   Sept. 25, 2002
 *   $Id: DeviceFontMgr.cpp,v 1.10 2002/12/06 06:03:36 rcktscientist Exp $
 */

#include "DeviceFontMgr.hpp"

#include "ByteArray.hpp"
#include "CommandSequence.hpp"
#include "DeviceFont.hpp"
#include "Event.hpp"
#include "Panose.hpp"
#include "Parameter.hpp"
#include "UPDFFontParser.hpp"

#include <iostream>
#include <stdio.h>
#include <string>

using namespace DevFont;
using namespace std;

/**
 * The constructor takes the device name and the path to the UPDF Configuration
 * file for the printer.
 *
 * @param deviceName  name of device
 * @param path        location of the device configuration file
 */
DeviceFontMgr::DeviceFontMgr (const char *deviceName, const char *path):
  fonts(NULL), cmdseqs(NULL), events(NULL), params(NULL), current(NULL),
  DRV_FontHeight(0), DRV_HorPos(0), DRV_VertPos(0), DRV_PageNo(0)
{
  setDeviceName (deviceName);
  parse (path);
}


/**
 * This constructor takes the device name and the paths to the UPDF Device
 * Description and Command Sequences files for the printer.
 *
 * @param deviceName  name of device
 * @param ddPath  path to the UPDF Device Description file
 * @param csPath  path to the UPDF Command Sequences file
 */
DeviceFontMgr::DeviceFontMgr (const char *deviceName,
                              const char *ddPath, const char *csPath):
fonts(NULL), cmdseqs(NULL), events(NULL), params(NULL), current(NULL),
DRV_FontHeight(0), DRV_HorPos(0), DRV_VertPos(0), DRV_PageNo(0)
{
    setDeviceName (deviceName);
    parse (ddPath, csPath);
}


/**
 * The constructor takes the device name, and assumes that this is the same
 * as the name of the UPDF file to use.  This constructor isn't very useful,
 * since the XML file would have to be in a standard location.  We might also
 * want to construct the path from "UPDF Device Configuration-" + deviceName +
 * ".xml".
 *
 * @param deviceName  name of the device
 */
DeviceFontMgr::DeviceFontMgr (const char *deviceName):
  fonts(NULL), cmdseqs(NULL), events(NULL), params(NULL), current(NULL),
  DRV_FontHeight(0), DRV_HorPos(0), DRV_VertPos(0), DRV_PageNo(0)
{
  setDeviceName (deviceName);
  //
  // Add the ".xml" extension to the devicename to get the UPDF file name
  //
  char *path = new char[5+strlen(deviceName)];
  strcpy (path, deviceName);
  strcat (path, ".xml");
  //
  // Now get the font information from the file
  //
  parse (path);
  //
  // Clean up our memory
  //
  delete [] path;
}


/**
 * Set the device name.
 *
 * @param deviceName  name of the device
 */
void DeviceFontMgr::setDeviceName (const char *deviceName)
{
  name = new char[1+strlen(deviceName)];
  strcpy (name, deviceName);
}


/**
 * Execute the parser to get the font information from the UPDF file.
 *
 * @param path  path to the UPDF Device Configuration file
 */
void DeviceFontMgr::parse (const char *path)
{
#ifdef DEBUG
cout << "DeviceFontMgr::parse(" << path << ") called" << endl;
#endif
  //
  // Create a new parser and read the UPDF file
  //
  UPDFFontParser parser(path);
  //
  // Ask the parser for the list of fonts and command sequences.  We now own
  // these objects and we are responsible for deleting them.
  //
  fonts = parser.getFonts();
  cmdseqs = parser.getCmdSeqs();
  events = parser.getEvents();
  params = parser.getParameters();
}


/**
 * Execute the parser to get the font information from the UPDF files.
 *
 * @param ddPath  path to the UPDF Device Description file
 * @param csPath  path to the UPDF Command Sequences file
 */
void DeviceFontMgr::parse (const char *ddPath, const char *csPath)
{
  //
  // Create a new parser and read the UPDF file
  //
  UPDFFontParser parser (ddPath, csPath);
  //
  // Ask the parser for the list of fonts and command sequences.  We now own
  // these objects and we are responsible for deleting them.
  //
  fonts = parser.getFonts();
  cmdseqs = parser.getCmdSeqs();
  events = parser.getEvents();
  params = parser.getParameters();
}


/**
 * The destructor should delete all the objects that we allocated (or are
 * otherwise responsible for).
 */
DeviceFontMgr::~DeviceFontMgr()    
{
  current = NULL;
  delete [] name;
  //
  // Delete all the fonts
  //
  if (fonts != NULL)
    {
      const DeviceFont *afont = fonts;
      while (afont != NULL)
        {
          const DeviceFont *temp = afont->getNext();
          delete afont;
          afont = temp;
        }
    }
  //
  // Delete all the command sequences
  //
  if (cmdseqs != NULL)
    {
      const CommandSequence *acmdseq = cmdseqs;
      while (acmdseq != NULL)
        {
          const CommandSequence *temp = acmdseq->getNext();
          delete acmdseq;
          acmdseq = temp;
        }
    }
  //
  // Delete all the events
  //
  if (events != NULL)
    {
      const Event *anevent = events;
      while (anevent != NULL)
        {
          const Event *temp = anevent->getNext();
          delete anevent;
          anevent = temp;
        }
    }
  //
  // Delete all the parameters
  //
  if (params != NULL)
  {
      const Parameter *aparam = params;
      while (aparam != NULL)
      {
          const Parameter *temp = aparam->getNext();
          delete aparam;
          aparam = temp;
      }
  }
  //
  // Set these to null for good measure
  //
  name = NULL;
  fonts = NULL;
  cmdseqs = NULL;
  events = NULL;
  params = NULL;
  //
  // If there are any bytes left in the list of command sequences, then (1)
  // the caller didn't do its job properly, and (2) we have to delete them
  // ourself.
  //
  int n = data.size();
  for (int i = 0; i < n; i++)
    {
      const ByteArray *ba = data.front();
      delete ba;
      data.pop_front();
    }
}


/**
 * Get a font with the given name (or NULL if there is no font with that name).
 * This is the actual node in the linked list, so the caller must not delete 
 * it.
 *
 * @param name  name of the font
 */
const DeviceFont *DeviceFontMgr::getFont (const char *name) const
{
  const DeviceFont *afont = fonts;
  while (afont != NULL)
    {
      if (strcmp(afont->pszID,name) == 0)
        return afont;
      afont = afont->getNext();
    }
  return NULL;
}


/**
 * Get a font with the closest match to the given Panose value.  Returns
 * NULL if there are no fonts defined in the UPDF file.
 *
 * @param p  the Panose value for the matching
 */
const DeviceFont *DeviceFontMgr::getFont (const Panose *p) const
{
  int bestvalue = 999999999;
  const DeviceFont *bestmatch = NULL;
  //
  // Loop over all the fonts
  //
  const DeviceFont *afont = fonts;
  while (afont != NULL)
    {
      int value = p->diff (afont->panose);
      if (value < bestvalue)
      {
        bestvalue = value;
        bestmatch = afont;
      }
      afont = afont->getNext();
    }
  return bestmatch;
}


/**
 * List all the fonts known by this DeviceFontMgr object to stdout.
 */
void DeviceFontMgr::listFonts() const
{
  cout << "\n" << "\n" <<"\n"
       << "******************************************************************* \n";
  cout << "Device: " << name << " has the following fonts: \n";

  const DeviceFont *afont = fonts;
  while (afont != NULL)
    {
      cout << afont->pszID << endl;
      afont = afont->getNext();
    }
}


/**
 * Return a string of bytes that, when sent to the printer, will set the
 * font.  Returns NULL if there is no string that can be used to set the
 * font; this is generally an error.  The caller is responsible for deleting
 * the ByteArray object when it is done with it.
 *
 * @param font  the font to set
 * @param size  the point size to use (in hundredths of a point)
 */
const ByteArray *DeviceFontMgr::getFontCommand (const DeviceFont *font, 
                                                int size) const
{
  //
  // Get the name of the font command sequence that we want
  //
  const char *cmdseqID = font->pszFontCommandSequenceID;
  if (cmdseqID == NULL)
    return NULL;
  //
  // Look through the command sequences to find the one that will return
  // the string we want.
  //
  const CommandSequence *cmdseq = getCommandSequence (cmdseqID);
  if (cmdseq != NULL)
      return getBytes (cmdseq);
  //
  // We did not find a command sequence name that matched that specified by
  // the font.  The caller should handle this error in whatever way is
  // appropriate.
  //
  return NULL;
}


/** Return a command sequence by its name (or NULL if there is none). */
const CommandSequence * DeviceFontMgr::getCommandSequence (const char *cmdseqID) const
{
    const CommandSequence *cmdseq = cmdseqs;
    while (cmdseq != NULL)
    {
        const char *id = cmdseq->getName();
        if (strcmp(id,cmdseqID) == 0)
            return cmdseq;
        cmdseq = cmdseq->getNext();
    }
    return NULL;
}


/** Return an event by its position and type (or NULL if there is none). */
const Event *DeviceFontMgr::getEvent (const char *position, const char *type) const
{
    const Event *event = events;
    while (event != NULL)
    {
        const char *s = event->getStartEnd();
        const char *t = event->getType();
        if (strcmp(position,s) == 0 && strcmp(type,t) == 0)
            return event;
        event = event->getNext();
    }
    return NULL;
}


/** Run the parameter converter to get a ByteArray. */
const ByteArray * DeviceFontMgr::getBytes (const CommandSequence *cmdseq) const
{
    //
    // Here we cast away the const'ness of the string, since the
    // Parameter Converter doesn't have the correct signature; but
    // that's out of our control.
    //
    char *value = const_cast<char *>(cmdseq->getValue());
    //
    // Create a symbol table for the driver variables
    //
    SYMBOLTABLE symbolTable = createSymbolTable ();
    //
    // Run the parameter converter
    //
    ByteArray *result = NULL;
    try
    {
      Expression *expression = convert (&value, true, false, &symbolTable);
      //
      // Now convert the expression into an array of bytes and stuff it
      // into a ByteArray.
      //
      unsigned char * data = expression->getBinaryData();
      result = new ByteArray (data, expression->getLength());
    }
    catch (ExpressionException *e)
    {
        cout << "Caught ExpressionException of type " << e->getType() << endl;
    }
    //
    // We are done with the symbol table; delete the memory its
    // components (that is, the Expression objects) use.  Do not attempt
    // to use the SYMBOLTABLE again after this, though.  (Remember that
    // the map destructor destroys objects but doesn't delete pointers,
    // so we don't worry about the string, but we have to manually
    // delete the Expression*.)
    //
    for (SYMBOLTABLE::iterator next = symbolTable.begin () ;
         next != symbolTable.end () ;
         next++ )
    {
        //cout << *((*next).second) << endl;
        delete (*next).second;
    }
    return result;
}


/** Create a new symbol table with all relevant values filled in. */
SYMBOLTABLE & DeviceFontMgr::createSymbolTable () const
{
    SYMBOLTABLE & symbolTable = *(new SYMBOLTABLE());
    //
    // First copy the driver variables (DRV_*) to the symbol table
    //
    char buf[40];
    sprintf (buf, "%d", DRV_FontHeight);
    symbolTable[string("DRV_FontHeight")] = new Expression (buf, true);
    sprintf (buf, "%d", DRV_HorPos);
    symbolTable[string("DRV_HorPos")] = new Expression (buf, true);
    sprintf (buf, "%d", DRV_VertPos);
    symbolTable[string("DRV_VertPos")] = new Expression (buf, true);
    sprintf (buf, "%d", DRV_PageNo);
    symbolTable[string("DRV_PageNo")] = new Expression (buf, true);
    //
    // Now copy all the parameter to the symbol table as well
    //
    const Parameter *p = params;
    while (p != NULL)
    {
        //
        // Get the name of the parameter (we have to remove the "Param_" from
        // the front).
        //
        const char *name1 = p->getName();
        char *name2 = new char[strlen(name1) - 5];
        strcpy (name2, &name1[6]);
        //
        // Throw away the constness so we can create the Expression; let's hope
        // and pray that the Expression constructor doesn't modify its arguments.
        //
        char * value = const_cast<char *>(p->getValue());
        // cout << "name = " << name << ", value = " << value << endl;
        Expression *e = new Expression (value, true);
        
        symbolTable[string(name2)] = e;
        delete [] name2;
        p = p->getNext();
    }
    symbolTable[string("Positioning")] = new Expression ("%H1(42)p", false);
    return symbolTable;
}


/** Start a new job. */
void DeviceFontMgr::startJob ()
{
  pushBytesForEvent (Event::START, "Job");
}


/** Start a new document. */
void DeviceFontMgr::startDocument ()
{
  pushBytesForEvent (Event::START, "Document");
}


/** Start a new page. */
void DeviceFontMgr::startPage ()
{
  pushBytesForEvent (Event::START, "PhysicalPage");
}


/**
 * Draw some text into the currently open segment.  This implementation is
 * far from complete.  It needs to:
 *
 * (1) Have some way of accumulating ByteArray objects
 * (2) Remember the old font, and only generate a font switch command if
 *     that changes
 *
 * This might be better done in a Segment class or something like that.  In
 * the meantime, we are just testing to see if we can do the Event lookup
 * properly.
 */
void DeviceFontMgr::drawText (int x, int y, const char *text)
{
  //
  // First copy the coordinates to the appropriate driver variables
  //
  DRV_HorPos = x;
  DRV_VertPos = y;
  //
  // Next, check to see if we have already set the requested font
  //
  if (requested == NULL)
      cerr << "Warning: requested font is NULL; use setFont() before drawText()" << endl;
  if (current != requested ||
      DRV_FontHeight != req_FontHeight)
    {
      //
      // Set the current font and height
      //
      current = requested;
      DRV_FontHeight = req_FontHeight;
      const ByteArray *ba = getFontCommand (current, DRV_FontHeight);
      if (ba != NULL)
          data.push_back (ba);
      else
          cerr << "Warning: Couldn't find bytes for setting font " << *current << endl;
    }
  //
  // Now get the command for the beginning of font commands
  //
  pushBytesForEvent (Event::START, "FontObjects");
  //
  // Now add the text to the bytes; first create a copy that the
  // ByteArray will own.
  //
  char *text2 = new char[strlen(text) + 1];
  strcpy (text2, text);
  ByteArray *ba = new ByteArray ((unsigned char *)text2, strlen(text));
  if (ba != NULL)
      data.push_back (ba);
  //
  // Get the command for the end of font commands
  //
  pushBytesForEvent (Event::END, "FontObjects");
}


/** End the page. */
void DeviceFontMgr::endPage ()
{
  pushBytesForEvent (Event::END, "PhysicalPage");
}


/** End the document. */
void DeviceFontMgr::endDocument ()
{
  pushBytesForEvent (Event::END, "Document");
}


/** End the job. */
void DeviceFontMgr::endJob ()
{
  pushBytesForEvent (Event::END, "Job");
}


/** Get an array of bytes for a given event. */
void DeviceFontMgr::pushBytesForEvent (const char *position, const char *type)
{
    const Event *event = events;
    while (event != NULL)
    {
        const char *s = event->getStartEnd();
        const char *t = event->getType();
        if (strcmp(position,s) == 0 && strcmp(type,t) == 0)
	  {
	    //
	    // Found the event; get the bytes for the event
	    //
	    const CommandSequence *cmdseq =
	      getCommandSequence (event->getCmdSeq());
	    if (cmdseq != NULL)
	      {
		const ByteArray *ba1 = getBytes(cmdseq);
                if (ba1 != NULL)
                    data.push_back (ba1);
	      }
	  }
        event = event->getNext();
    }
}


/** Return the bytes accumulated so far and empty the list. */
const ByteArray * DeviceFontMgr::getBytes()
{
    ByteArray *result = NULL;
    int n = data.size();
    for (int i = 0; i < n; i++)
    {
        const ByteArray *ba = data.front();
        data.pop_front();
        if (result == NULL)
            result = new ByteArray (*ba);
        else
            *result = *result + *ba;
        delete ba;
    }
    return result;
}
