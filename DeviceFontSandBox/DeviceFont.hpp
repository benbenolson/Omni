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
 *   $Id: DeviceFont.hpp,v 1.3 2002/10/30 23:48:58 rcktscientist Exp $
 */

#ifndef _DeviceFont
#define _DeviceFont

#include <iostream>

#include "Panose.hpp"

namespace DevFont
{

/**
 * This class represents a single device font.  It is mostly just a translation
 * of parameters read from a UPDF file.  The data members are public, so it
 * really acts like a struct.
 *
 * @author Julie Zhuo
 * @version 1.0
 */
class DeviceFont
{
public:
  /** Enumeration of all the font families. */
  enum Font_Family
  {
    DONTCARE,
    ROMAN,
    SWISS,
    MODERN,
    SCRIPT,
    DECORATIVE
  };

  /** The default constructor creates an object with all fields set to 0. */
  DeviceFont();

  /** The copy constructor. */
  DeviceFont (const DeviceFont &);

  /** The destructor. */
  ~DeviceFont();
  
  /** 
   * Translate the given string into a font family id.
   *
   * @param str  string to translate
   */
  static Font_Family getFontFamily (const char *str);

  /**
   * Inserts the specified node into the list after the current node.
   *
   * @param afont  the font to add
   */
  void setNext (DeviceFont *afont);

  /** Get the next node in the linked list. */
  const DeviceFont * getNext () const { return next; }

  /** Print out all the information of this font for debugging. */
  void print() const;

  //Have all font attributes public to avoid creating too many getters and setters.

  /** The name of the font (UTF-8 encoded). */
  char* pszID;

  /** The family of the font. */
  Font_Family fontFamily;

  /** The vendor of the font. */
  char* pszFont_Vendor;

  /** The encoding of the font. */
  char* pszEncoding;

  /** The ID of the command sequence that sets the font. */
  char *pszFontCommandSequenceID;

  char* pszPassive_Font;

  // The following are GlobalMetrics Info
  int iGlobal_Units_Per_Em;
  int iGlobal_Points_Per_Inch;
  int iGlobal_Spacing;
  int iGlobal_Minimum_Height;
  int iGlobal_Maximum_Height;
  int iGlobal_HeightStep;
  int iGlobal_Slant;
  int iGlobal_WindowsWeight;
  int iGlobal_Windows_Ascender;
  int iGlobal_Windows_Descender;
  int iGlobal_Mac_Ascender;
  int iGlobal_Mac_Descender;
  int iGlobal_Mac_Line_Gap;
  int iGlobal_Typographic_Ascender;
  int iGlobal_Typographic_Descender;
  int iGlobal_Typographic_Line_Gap;
  int iGlobal_Average_Character_Width;
  int iGlobal_Maximum_Character_Increment;
  int iGlobal_Cap_Height;
  int iGlobal_x_Height;
  int iGlobal_Subscript_X_Size;
  int iGlobal_Subscript_Y_Size;
  int iGlobal_Subscript_X_Offset;
  int iGlobal_Subscript_Y_Offset;
  int iGlobal_Superscript_X_Size;
  int iGlobal_Superscript_Y_Size;
  int iGlobal_Superscript_X_Offset;
  int iGlobal_Superscript_Y_Offset;
  int iGlobal_Underscore_Size;
  int iGlobal_Underscore_Offset;
  int iGlobal_Underscore_Absolute_Values;
  int iGlobal_Strikeout_Size;
  int iGlobal_Strikeout_Offset;
  int iGlobal_Strikeout_Absolute_Values;
  int iGlobal_Baseline;
  int iGlobal_Aspect;
  int iGlobal_Caret;
  int iGlobal_Orientation;
  int iGlobal_Degree;
  int iGlobal_CharacterWidth_A_Value;
  int iGlobal_CharacterWidth_B_Value;
  int iGlobal_CharacterWidth_C_Value;
  
  /* Embed a Panose object. */
  Panose panose;
  
  // The following are NameID Info
  char* pszFaceName_Name_ID;
  char* pszFamilyName_Name_ID;
  char* pszUniqueName_Name_ID;
  //char* strAliasName_Name_ID[];  // unbounded item
  char* pszSlantName_Name_ID;
  char* pszWeightName_Name_ID;

private:
  /** Forbid calling the assignment operator. */
  DeviceFont & operator= (const DeviceFont &);

  /** Copy all the font information from the specified node. */
  char * copy (const char *);

  /** The next node in the linked list of DeviceFont objects. */
  DeviceFont *next;
};

/** Print out a selection of information about the font. */
std::ostream & operator<< (std::ostream &, const DeviceFont &);

}
    
#endif
