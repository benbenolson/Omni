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
 *   $Id: DeviceFontMgr.hpp,v 1.8 2002/12/06 06:04:06 rcktscientist Exp $
 */

#ifndef _DeviceFontMgr
#define _DeviceFontMgr

#include "ParameterConverter.hpp"

#include <list>

namespace DevFont
{

/**
 * This class is the main class that the user will use for querying and setting
 * font information.  There should be one DeviceFontMgr object for every
 * print device (that is, for every UPDF file).  This 
 *
 * @author Julie Zhuo
 * @version 2.0
 */

class ByteArray;
class CommandSequence;
class DeviceFont;
class Event;
class Panose;
class Parameter;

class DeviceFontMgr
{
public:

  /**
   * The constructor takes the device name, and assumes that this is the same
   * as the name of the UPDF file to use.  This constructor isn't very useful,
   * since the XML file would have to be in a standard location.
   *
   * @param deviceName  name of the device
   */
  explicit DeviceFontMgr (const char *deviceName);

  /**
   * The constructor takes the device name and the path to the UPDF 
   * Configuration file for the printer.
   *
   * @param deviceName  name of device
   * @param path        location of the device configuration file
   */
  DeviceFontMgr (const char *deviceName, const char *path);

  /**
   * The constructor takes the device name and the path to the UPDF 
   * Configuration file for the printer.
   *
   * @param deviceName  name of device
   * @param ddPath      location of the device description file
   * @param csPath      location of the command sequences file
   */
  DeviceFontMgr (const char *deviceName, const char *ddPath, const char *csPath);

  /** Release the objects that we own. */
  ~DeviceFontMgr();

  /** Returns a linked list of DeviceFont objects. */
  inline const DeviceFont* getFonts () const { return fonts; }

  /** For debugging only: return the command sequences. */
  inline const CommandSequence *getCommandSequences () const { return cmdseqs; }

  /** For debugging only: return the events. */
  inline const Event *getEvents () const { return events; }

  /** For debugging only: return the events. */
  inline const Parameter *getParameters () const { return params; }

  /**
   * Get a font with the given name (or NULL if there is no font with that
   * name). This is the actual node in the linked list, so the caller must not 
   * delete it.
   *
   * @param name  UTF-8 encoded name of the font
   */
  const DeviceFont * getFont (const char *name) const;

  /**
   * Get a font with the closest match to the given Panose value.  Returns
   * NULL if there are no fonts defined in the UPDF file.
   *
   * @param p  the Panose value for the matching
   */
  const DeviceFont * getFont (const Panose *p) const;

  /**
   * Return a string that will set the current font in the printer.  Returns
   * NULL if there is no string that can be used to set the font; this is 
   * generally an error.  The caller is responsible for deleting the
   * ByteArray object when it is done with it.
   *
   * Note that it might seem like this would be a method that should be part
   * of DeviceFont, but that class doesn't have access to the actual
   * CommandSequence objects, since those are stored separately.
   *
   * @param font  the font to get
   * @param size  the point size to use (in hundredths of a point)
   */
  const ByteArray * getFontCommand (const DeviceFont *font, int size) const;

  /**
   * Start a print job.
   */
  void startJob();

  /**
   * Start a document within a print job.
   */
  void startDocument();

  /**
   * Start a page within a docment.
   */
  void startPage ();

  /**
   * Sets the font to use for the next text command.
   *
   * @param font  the font to set.
   */
  inline void setFont (const DeviceFont *font) { requested = font; }

  /**
   * Return the font that will be used for the next text.  Technically
   * speaking, it may not be the "current" font at the time of this call,
   * but this class behaves as if it is.
   */
  inline const DeviceFont * getCurrentFont() const { return requested; }

  /**
   * Sets the size of the current font.
   *
   * @param size  size of the font
   */
  inline void setSize (int size) { req_FontHeight = size; }

  /**
   * Returns the font size that will be used for the next text command.
   */
  inline int getSize () const { return req_FontHeight; }

  /** 
   * Draw some text into the currently open segment.  If necessary, it will
   * set the font and size before drawing the text.
   *
   * @param x     x-coordinate of the text on the page
   * @param y     y-coordinate of the text on the page
   * @param text  text to draw
   */
  void drawText (int x, int y, const char *text);

  /**
   * End the current page.
   */
  void endPage ();

  /**
   * End the current document.
   */
  void endDocument ();

  /**
   * End the current job.
   */
  void endJob ();

  /**
   * Get the bytes that have been accumulated so far.
   */
  const ByteArray * getBytes ();

  /** List all the fonts for a given device to stdout. */
  void listFonts () const;

private:
  /** The name of this device. */
  char * name;

  /** A pointer to the head of the list of device font nodes. */
  DeviceFont *fonts;

  /** A pointer to the head of the list of command sequences. */
  CommandSequence *cmdseqs;

  /** A pointer to the head of the list of events. */
  Event *events;

  /** A pointer to the head of the list of parameters. */
  Parameter *params;

  /** 
   * This is the currently selected font.  (Since this is a pointer to a node
   * in the fonts linked list, don't delete this in the destructor.)
   */
  const DeviceFont *current;

  /**
   * The currently requested font.  (Since this is a pointer to a node
   * in the fonts linked list, don't delete this in the destructor.)
   */
  const DeviceFont *requested;

  /** The current font size (in hundredths of a point). */
  int DRV_FontHeight;

  /** The requested font size (in hundredths of a point). */
  int req_FontHeight;

  /** The current horizontal position. */
  int DRV_HorPos;

  /** The current vertical position. */
  int DRV_VertPos;

  /** The current page number. */
  int DRV_PageNo;

  /** The bytes that have been accumulated so far; used as a queue. */
  std::list <const ByteArray *> data;

  /** Forbid calling the copy constructor. */
  DeviceFontMgr (const DeviceFontMgr &);

  /** Forbid calling the assignment operator. */
  DeviceFontMgr & operator= (const DeviceFontMgr &);

  /**
   * Set the device name.
   *
   * @param name  new name for the device
   */
  void setDeviceName (const char *name);

  /**
   * Run the parser on the indicated file.
   *
   * @param path  path the UPDF Device Configuration file
   */
  void parse (const char *path);

  /**
   * Run the parser on the indicated files.
   *
   * @param ddPath  path the UPDF Device Description file
   * @param csPath  path the UPDF Command Sequences file
   */
  void parse (const char *ddPath, const char *csPath);

  /**
   * Create a new symbol table with all values and parameters filled in.
   */
  SYMBOLTABLE & createSymbolTable () const;

  /**
   * Run the parameter converter on the given CommandSequence to get its
   * converted bytes.
   *
   * @param cmdseq  the CommandSequence whose value is to be converted
   */
  const ByteArray * getBytes (const CommandSequence *cmdseq) const;

  /**
   * Return a command sequence by its name (or NULL if there is none).
   *
   * @param cmdseqID  name of the command sequence
   */
  const CommandSequence * getCommandSequence (const char *cmdseqID) const;

  /**
   * Return an event by its position and event type.
   *
   * @param position  position (start or end) of the event
   * @param type      type of trigger for this event
   */
  const Event * getEvent (const char *position, const char *type) const;

  /**
   * Utility routine that will push all the required bytes for an event type
   * onto the queue.
   *
   * @param position  position (start or end) of the event
   * @param type      type of trigger for this event
   */
  void pushBytesForEvent (const char *position, const char *type);
};

}

#endif
