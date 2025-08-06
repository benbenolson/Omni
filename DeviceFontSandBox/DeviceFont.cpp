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
 *   $Id: DeviceFont.cpp,v 1.5 2002/11/21 16:46:16 rcktscientist Exp $
 */

#include "DeviceFont.hpp"

#include <string>

using namespace std;
using namespace DevFont;

/**
 * The constructor just sets the all the member variables to their default
 * values.  Since all the members are public, the parser will just fill in
 * each one as the are read from the UPDF file.
 */
DeviceFont::DeviceFont():
  pszID(NULL), fontFamily(DONTCARE), pszFont_Vendor(NULL), pszEncoding(NULL),
  pszFontCommandSequenceID(NULL), pszPassive_Font(NULL),
  iGlobal_Units_Per_Em(0), iGlobal_Points_Per_Inch(0),iGlobal_Spacing(0), 
  iGlobal_Minimum_Height(0), iGlobal_Maximum_Height(0), iGlobal_HeightStep(0), iGlobal_Slant(0),
  iGlobal_WindowsWeight(0), iGlobal_Windows_Ascender(0), iGlobal_Windows_Descender(0),
  iGlobal_Mac_Ascender(0), iGlobal_Mac_Descender(0), iGlobal_Mac_Line_Gap(0),
  iGlobal_Typographic_Ascender(0), iGlobal_Typographic_Descender(0), iGlobal_Typographic_Line_Gap(0),
  iGlobal_Average_Character_Width(0), iGlobal_Maximum_Character_Increment(0), iGlobal_Cap_Height(0),
  iGlobal_x_Height(0), iGlobal_Subscript_X_Size(0), iGlobal_Subscript_Y_Size(0),
  iGlobal_Subscript_X_Offset(0), iGlobal_Subscript_Y_Offset(0), iGlobal_Superscript_X_Size(0),
  iGlobal_Superscript_Y_Size(0), iGlobal_Superscript_X_Offset(0), iGlobal_Superscript_Y_Offset(0),
  iGlobal_Underscore_Size(0), iGlobal_Underscore_Offset(0), iGlobal_Underscore_Absolute_Values(0),
  iGlobal_Strikeout_Size(0), iGlobal_Strikeout_Offset(0), iGlobal_Strikeout_Absolute_Values(0),
  iGlobal_Baseline(0), iGlobal_Aspect(0), iGlobal_Caret(0), iGlobal_Orientation(0),
  iGlobal_Degree(0), iGlobal_CharacterWidth_A_Value(0), iGlobal_CharacterWidth_B_Value(0),
  iGlobal_CharacterWidth_C_Value(0), pszFaceName_Name_ID(NULL), pszFamilyName_Name_ID(NULL),
  pszUniqueName_Name_ID(NULL), pszSlantName_Name_ID(NULL), pszWeightName_Name_ID(NULL),
  next(NULL)
{
}

/**
 * The copy constructor.  Since this recursively copies all the sibling font
 * nodes in the list, in general one should only call the copy constructor
 * on the head of the list.
 */
DeviceFont::DeviceFont(const DeviceFont &f):
  next(NULL)
{
  //
  // Make copies of the primitive types
  //
  iGlobal_Units_Per_Em = f.iGlobal_Units_Per_Em;
  iGlobal_Points_Per_Inch = f.iGlobal_Points_Per_Inch;
  iGlobal_Spacing = f.iGlobal_Spacing;
  iGlobal_Minimum_Height = f.iGlobal_Minimum_Height;
  iGlobal_Maximum_Height = f.iGlobal_Maximum_Height;
  iGlobal_HeightStep = f.iGlobal_HeightStep;
  iGlobal_Slant = f.iGlobal_Slant;
  iGlobal_WindowsWeight = f.iGlobal_WindowsWeight;
  iGlobal_Windows_Ascender = f.iGlobal_Windows_Ascender;
  iGlobal_Windows_Descender = f.iGlobal_Windows_Descender;
  iGlobal_Mac_Ascender = f.iGlobal_Mac_Ascender;
  iGlobal_Mac_Descender = f.iGlobal_Mac_Descender;
  iGlobal_Mac_Line_Gap = f.iGlobal_Mac_Line_Gap;
  iGlobal_Typographic_Ascender = f.iGlobal_Typographic_Ascender;
  iGlobal_Typographic_Descender = f.iGlobal_Typographic_Descender;
  iGlobal_Typographic_Line_Gap = f.iGlobal_Typographic_Line_Gap;
  iGlobal_Average_Character_Width = f.iGlobal_Average_Character_Width;
  iGlobal_Maximum_Character_Increment = f.iGlobal_Maximum_Character_Increment;
  iGlobal_Cap_Height = f.iGlobal_Cap_Height;
  iGlobal_x_Height = f.iGlobal_x_Height;
  iGlobal_Subscript_X_Size = f.iGlobal_Subscript_X_Size;
  iGlobal_Subscript_Y_Size = f.iGlobal_Subscript_Y_Size;
  iGlobal_Subscript_X_Offset = f.iGlobal_Subscript_X_Offset;
  iGlobal_Subscript_Y_Offset = f.iGlobal_Subscript_Y_Offset;
  iGlobal_Superscript_X_Size = f.iGlobal_Superscript_X_Size;
  iGlobal_Superscript_Y_Size = f.iGlobal_Superscript_Y_Size;
  iGlobal_Superscript_X_Offset = f.iGlobal_Superscript_X_Offset;
  iGlobal_Superscript_Y_Offset = f.iGlobal_Superscript_Y_Offset;
  iGlobal_Underscore_Size = f.iGlobal_Underscore_Size;
  iGlobal_Underscore_Offset = f.iGlobal_Underscore_Offset;
  iGlobal_Underscore_Absolute_Values = f.iGlobal_Underscore_Absolute_Values;
  iGlobal_Strikeout_Size = f.iGlobal_Strikeout_Size;
  iGlobal_Strikeout_Offset = f.iGlobal_Strikeout_Offset;
  iGlobal_Strikeout_Absolute_Values = f.iGlobal_Strikeout_Absolute_Values;
  iGlobal_Baseline = f.iGlobal_Baseline;
  iGlobal_Aspect = f.iGlobal_Aspect;
  iGlobal_Caret = f.iGlobal_Caret;
  iGlobal_Orientation = f.iGlobal_Orientation;
  iGlobal_Degree = f.iGlobal_Degree;
  iGlobal_CharacterWidth_A_Value = f.iGlobal_CharacterWidth_A_Value;
  iGlobal_CharacterWidth_B_Value = f.iGlobal_CharacterWidth_B_Value;
  iGlobal_CharacterWidth_C_Value = f.iGlobal_CharacterWidth_C_Value;
  panose = f.panose;
  //
  // Next we must make copies of the string objects
  //
  pszID = copy (f.pszID);
  pszFont_Vendor = copy (f.pszFont_Vendor);
  pszEncoding = copy (f.pszEncoding);
  pszFontCommandSequenceID = copy (f.pszFontCommandSequenceID);
  pszPassive_Font = copy (f.pszPassive_Font);
  pszFaceName_Name_ID = copy (f.pszFaceName_Name_ID);
  pszFamilyName_Name_ID = copy (f.pszFamilyName_Name_ID);
  pszUniqueName_Name_ID = copy (f.pszUniqueName_Name_ID);
  pszSlantName_Name_ID = copy (f.pszSlantName_Name_ID);
  pszWeightName_Name_ID = copy (f.pszWeightName_Name_ID);
}


/**
 * Copy a string, checking to make sure that it isn't null before we do.
 */
char *DeviceFont::copy (const char *string)
{
  if (string == NULL)
    return NULL;
  char * result = new char[1+strlen(string)];
  strcpy (result, string);
  return result;
}


/**
 * The destructor deletes the strings and the entire list of sibling font 
 * nodes.
 */
DeviceFont::~DeviceFont ()
{
  if (pszID) delete [] pszID;
  if (pszFont_Vendor) delete [] pszFont_Vendor;
  if (pszEncoding) delete [] pszEncoding;
  if (pszFontCommandSequenceID) delete [] pszFontCommandSequenceID;
  if (pszPassive_Font) delete [] pszPassive_Font;
  if (pszFaceName_Name_ID) delete [] pszFaceName_Name_ID;
  if (pszFamilyName_Name_ID) delete [] pszFamilyName_Name_ID;
  if (pszUniqueName_Name_ID) delete [] pszUniqueName_Name_ID;
  if (pszSlantName_Name_ID) delete [] pszSlantName_Name_ID;
  if (pszWeightName_Name_ID) delete [] pszWeightName_Name_ID;
  //
  // For good measure, set the pointers to NULL, so we can detect if we try
  // to use a deleted DeviceFont object.
  //
  pszID = NULL;
  pszFont_Vendor = NULL;
  pszEncoding = NULL;
  pszPassive_Font = NULL;
  pszFaceName_Name_ID = NULL;
  pszFamilyName_Name_ID = NULL;
  pszUniqueName_Name_ID = NULL;
  pszSlantName_Name_ID = NULL;
  pszWeightName_Name_ID = NULL;
  //
  // Set the next pointer to NULL (but don't delete, since we don't own the
  // next node).
  //
  next = NULL;
}


/**
 * Print out all the information in the object for debugging.
 */
void DeviceFont::print() const
{
  cout << "pszID is: " << pszID << endl;
  if (pszFont_Vendor)
    cout << "pszFont_Vendor is: " << pszFont_Vendor << endl;
  if (pszEncoding)
  cout << "pszEncoding is: " << pszEncoding << endl;
  //cout << "pszPassive_Font is: " << pszPassive_Font << endl;
  cout << "iGlobal_Units_Per_Em is: " << iGlobal_Units_Per_Em << endl;
  cout << "iGlobal_Points_Per_Inch is: " << iGlobal_Points_Per_Inch << endl;
  cout << "iGlobal_Spacing is: " << iGlobal_Spacing << endl;
  cout << "iGlobal_Minimum_Height is: " << iGlobal_Minimum_Height << endl;
  cout << "iGlobal_Maximum_Height is: " << iGlobal_Maximum_Height << endl;
  cout << "iGlobal_HeightStep is: " << iGlobal_HeightStep << endl;
  cout << "iGlobal_Slant is: " << iGlobal_Slant << endl;
  cout << "iGlobal_WindowsWeight is: " << iGlobal_WindowsWeight << endl;
  cout << "iGlobal_Windows_Ascender is: " << iGlobal_Windows_Ascender << endl;
  cout << "iGlobal_Windows_Descender is: " << iGlobal_Windows_Descender << endl;
  cout << "iGlobal_Mac_Ascender is: " << iGlobal_Mac_Ascender << endl;
  cout << "iGlobal_Mac_Descender is: " << iGlobal_Mac_Descender << endl;
  cout << "iGlobal_Mac_Line_Gap is: " << iGlobal_Mac_Line_Gap << endl;
  cout << "iGlobal_Typographic_Ascender is: " << iGlobal_Typographic_Ascender << endl;
  cout << "iGlobal_Typographic_Descender is: " << iGlobal_Typographic_Descender << endl;
  cout << "iGlobal_Typographic_Line_Gap is: " << iGlobal_Typographic_Line_Gap << endl;
  cout << "iGlobal_Average_Character_Width is: " << iGlobal_Average_Character_Width << endl;
  cout << "iGlobal_Maximum_Character_Increment is: " << iGlobal_Maximum_Character_Increment << endl;
  cout << "iGlobal_Cap_Height is: " << iGlobal_Cap_Height << endl;
  cout << "iGlobal_x_Height is: " << iGlobal_x_Height << endl;
  cout << "iGlobal_Subscript_X_Size is: " << iGlobal_Subscript_X_Size << endl;
  cout << "iGlobal_Subscript_Y_Size is: " << iGlobal_Subscript_Y_Size << endl;
  cout << "iGlobal_Subscript_X_Offset is: " << iGlobal_Subscript_X_Offset << endl;
  cout << "iGlobal_Subscript_Y_Offset is: " << iGlobal_Subscript_Y_Offset << endl;
  cout << "iGlobal_Superscript_X_Size is: " << iGlobal_Superscript_X_Size << endl;
  cout << "iGlobal_Superscript_Y_Size is: " << iGlobal_Superscript_Y_Size << endl;
  cout << "iGlobal_Superscript_X_Offset is: " << iGlobal_Superscript_X_Offset << endl;
  cout << "iGlobal_Superscript_Y_Offset is: " << iGlobal_Superscript_Y_Offset << endl;
  cout << "iGlobal_Underscore_Size is: " << iGlobal_Underscore_Size << endl;
  cout << "iGlobal_Underscore_Offset is: " << iGlobal_Underscore_Offset << endl;
  cout << "iGlobal_Underscore_Absolute_Values is: " << iGlobal_Underscore_Absolute_Values << endl;
  cout << "iGlobal_Strikeout_Size is: " << iGlobal_Strikeout_Size << endl;
  cout << "iGlobal_Strikeout_Offset is: " << iGlobal_Strikeout_Offset << endl;
  cout << "iGlobal_Strikeout_Absolute_Values is: " << iGlobal_Strikeout_Absolute_Values << endl;
  cout << "iGlobal_Baseline is: " << iGlobal_Baseline << endl;
  cout << "iGlobal_Aspect is: " << iGlobal_Aspect << endl;
  cout << "iGlobal_Caret is: " << iGlobal_Caret << endl;
  cout << "iGlobal_Orientation is: " << iGlobal_Orientation << endl;
  cout << "iGlobal_Degree is: " << iGlobal_Degree << endl;
  cout << "iGlobal_CharacterWidth_A_Value is: " << iGlobal_CharacterWidth_A_Value << endl;
  cout << "iGlobal_CharacterWidth_B_Value is: " << iGlobal_CharacterWidth_B_Value << endl;
  cout << "iGlobal_CharacterWidth_C_Value is: " << iGlobal_CharacterWidth_C_Value << endl;
 
  cout << "panose is: " << &panose << endl;

  if (pszFaceName_Name_ID)
    cout << "pszFaceName_Name_ID is: " << pszFaceName_Name_ID << endl;
  if (pszFamilyName_Name_ID)
    cout << "pszFamilyName_Name_ID is: " << pszFamilyName_Name_ID << endl;
  if (pszUniqueName_Name_ID)
    cout << "pszUniqueName_Name_ID is: " << pszUniqueName_Name_ID << endl;
  if (pszSlantName_Name_ID)
    cout << "pszSlantName_Name_ID is: " << pszSlantName_Name_ID << endl;
  if (pszWeightName_Name_ID)
    cout << "pszWeightName_Name_ID is: " << pszWeightName_Name_ID << endl;
}


/**
 * Brute-force method to return a Panose Font_Family for a given UPDF string.
 */
DeviceFont::Font_Family DeviceFont::getFontFamily (const char *str)
{
  if (strcmp("ROMAN", str) == 0)
    return ROMAN;
  else if (strcmp("SWISS", str) == 0)
    return SWISS;
  else if (strcmp("MODERN", str) == 0)
    return MODERN;
  else if (strcmp("SCRIPT", str) == 0)
    return SCRIPT;
  else if (strcmp("DECORATIVE", str) == 0)
    return DECORATIVE;
  else
    return DONTCARE;
}


/**
 * Inserts the specified node into the list after the current node.
 *
 * @param afont  the font to add
 */
void DeviceFont::setNext (DeviceFont *anode)
{
  anode->next = next;
  next = anode;
}


namespace DevFont
{
/** Print just a little font information. */
ostream & operator<< (ostream &s, const DeviceFont &f)
{
  s << "DeviceFont[" << f.pszID << "," << f.iGlobal_Spacing << "]";
  return s;
}
}
