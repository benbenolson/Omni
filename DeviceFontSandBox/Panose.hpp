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
 *   October 16, 2002
 *   $Id: Panose.hpp,v 1.5 2002/11/14 05:58:55 rcktscientist Exp $
 */

#ifndef _Panose
#define _Panose

#include <iostream>

namespace DevFont
{

/**
 * This class represents all the Panose 1.0 values for Latin Text.  It defines
 * ten  enumerations for the ten characteristics.  These values probably don't
 * make sense for non-Latin Text (i.e., Family_Type != FAMILY_TEXT_DISPLAY).
 *
 * The constants are based on their definitions from the UPDF schema (with
 * some redundant "PAN_" prefixes and "_enum" suffixes removed for brevity).
 * The diff() method calculates the difference between two Panose values
 * (but not very well at the moment).
 *
 * @author David L. Wagner
 * @version 1.0
 */
class Panose
{
public:
enum Family_Type
  {
    FAMILY_ANY,
    FAMILY_NO_FIT,
    FAMILY_TEXT_DISPLAY,
    FAMILY_SCRIPT,
    FAMILY_DECORATIVE,
    FAMILY_PICTORIAL
  };

enum Serif_Style
  {
    SERIF_ANY,
    SERIF_NO_FIT,
    SERIF_COVE,
    SERIF_OBTUSE_COVE,
    SERIF_SQARE_COVE,
    SERIF_OBTUSE_SQARE_COVE,
    SERIF_SQARE,
    SERIF_THIN,
    SERIF_BONE,
    SERIF_EXAGGERATED,
    SERIF_TRIANGLE,
    SERIF_NORMAL_SANS,
    SERIF_OBTUSE_SANS,
    SERIF_PERP_SANS,
    SERIF_FLARED,
    SERIF_ROUNDED
  };

enum Weight
  {
    WEIGHT_ANY,
    WEIGHT_NO_FIT,
    WEIGHT_VERY_LIGHT,
    WEIGHT_LIGHT,
    WEIGHT_THIN,
    WEIGHT_BOOK,
    WEIGHT_MEDIUM,
    WEIGHT_DEMI,
    WEIGHT_BOLD,
    WEIGHT_HEAVY,
    WEIGHT_BLACK,
    WEIGHT_NORD
  };

enum Proportion
  {
    PROP_ANY,
    PROP_NO_FIT,
    PROP_OLD_STYLE,
    PROP_MODERN,
    PROP_EVEN_WIDTH,
    PROP_EXPANDED,
    PROP_CONDENSED,
    PROP_VERY_EXPANDED,
    PROP_VERY_CONDENSED,
    PROP_MONOSPACED
  };

enum Contrast
  {
    CONTRAST_ANY,
    CONTRAST_NO_FIT,
    CONTRAST_NONE,
    CONTRAST_VERY_LOW,
    CONTRAST_LOW,
    CONTRAST_MEDIUM_LOW,
    CONTRAST_MEDIUM,
    CONTRAST_MEDIUM_HIGH,
    CONTRAST_HIGH,
    CONTRAST_VERY_HIGH
  };

enum Stroke_Variation
  {
    STROKE_ANY,
    STROKE_NO_FIT,
    STROKE_GRADUAL_DIAG,
    STROKE_GRADUAL_TRAN,
    STROKE_GRADUAL_VERT,
    STROKE_GRADUAL_HORZ,
    STROKE_RAPID_VERT,
    STROKE_RAPID_HORZ,
    STROKE_INSTANT_VERT
  };

enum Arm_Style
  {
    ARMS_ANY,
    ARMS_NO_FIT,
    STRAIGHT_ARMS_HORZ,
    STRAIGHT_ARMS_WEDGE,
    STRAIGHT_ARMS_VERT,
    STRAIGHT_ARMS_SINGLE_SERIF,
    STRAIGHT_ARMS_DOUBLE_SERIF,
    BENT_ARMS_HORZ,
    BENT_ARMS_WEDGE,
    BENT_ARMS_VERT,
    BENT_ARMS_SINGLE_SERIF,
    BENT_ARMS_DOUBLE_SERIF
  };

enum Letterform
  {
    LETT_ANY,
    LETT_NO_FIT,
    LETT_NORMAL_CONTACT,
    LETT_NORMAL_WEIGHTED,
    LETT_NORMAL_BOXED,
    LETT_NORMAL_FLATTENED,
    LETT_NORMAL_ROUNDED,
    LETT_NORMAL_OFF_CENTER,
    LETT_NORMAL_SQARE,
    LETT_OBLIQUE_CONTACT,
    LETT_OBLIQUE_WEIGHTED,
    LETT_OBLIQUE_BOXED,
    LETT_OBLIQUE_FLATTENED,
    LETT_OBLIQUE_ROUNDED,
    LETT_OBLIQUE_OFF_CENTER,
    LETT_OBLIQUE_SQARE
  };

enum Midline
  {
    MIDLINE_ANY,
    MIDLINE_NO_FIT,
    MIDLINE_STANDARD_TRIMMED,
    MIDLINE_STANDARD_POINTED,
    MIDLINE_STANDARD_SERIFED,
    MIDLINE_HIGH_TRIMMED,
    MIDLINE_HIGH_POINTED,
    MIDLINE_HIGH_SERIFED,
    MIDLINE_CONSTANT_TRIMMED,
    MIDLINE_CONSTANT_POINTED,
    MIDLINE_CONSTANT_SERIFED,
    MIDLINE_LOW_TRIMMED,
    MIDLINE_LOW_POINTED,
    MIDLINE_LOW_SERIFED
  };

enum X_Height
  {
    XHEIGHT_ANY,
    XHEIGHT_NO_FIT,
    XHEIGHT_CONSTANT_SMALL,
    XHEIGHT_CONSTANT_STD,
    XHEIGHT_CONSTANT_LARGE,
    XHEIGHT_DUCKING_SMALL,
    XHEIGHT_DUCKING_STD,
    XHEIGHT_DUCKING_LARGE
  };
  
  /** The constructor. */
  Panose();
  
  /** This constructor fills in all the member variables. */
  Panose (Family_Type, Serif_Style, Weight, Proportion, Contrast,
          Stroke_Variation, Arm_Style, Letterform, Midline, X_Height);

  /** This constructor takes all the members as ints and converts them. */
  Panose (int, int, int, int, int, int, int, int, int, int);

  /** The copy constructor. */
  Panose (const Panose &);
  
  /** The assignment operator. */
  Panose & operator= (const Panose &);

  /** Convert a string into a Family_Type. */
  static Family_Type getFamilyType (const char *);

  /** Convert a string into a Serif_Style. */
  static Serif_Style getSerifStyle (const char *);

  /** Convert a string into a Weight. */
  static Weight getWeight (const char *);

  /** Convert a string into a Proportion. */
  static Proportion getProportion (const char *);

  /** Convert a string into a Contrast. */
  static Contrast getContrast (const char *);

  /** Convert a string into a Stroke_Variation. */
  static Stroke_Variation getStrokeVariation (const char *);

  /** Convert a string into an Arm_Style. */
  static Arm_Style getArmStyle (const char *);

  /** Convert a string into a Letterform. */
  static Letterform getLetterform (const char *);

  /** Convert a string into a Midline. */
  static Midline getMidline (const char *);

  /** Convert a string into an X_Height. */
  static X_Height getXHeight (const char *);
  
  /** Determine the difference between two Panose values. */
  int diff (const Panose &) const;

  /** 
   * All the different values that describe this Panose value.  Note that these
   * are public, to avoid having an excessive number of accessor methods; this
   * really isn't much of a problem, since one would have to know that Panose
   * specification anyway.
   */
  Family_Type familyType;
  Serif_Style serifStyle;
  Weight weight;
  Proportion proportion;
  Contrast contrast;
  Stroke_Variation strokeVariation;
  Arm_Style armStyle;
  Letterform letterform;
  Midline midline;
  X_Height xHeight;

private:
  /** Used by copy constructor and assignment operator. */
  void copy (const Panose &);

  /** Calculate the square of the difference between two Panose values. */
  static int calcdiffm (int, int, int);

  /** Calculate the difference between two discrete Panose values. */
  static int calcdiffd (int, int);

  /** Calculate the difference between two Serif Style values. */
  static int calcdiffss (int, int);

  /** Calculate the difference between two Letterform values. */
  static int calcdifflf (int, int);
};

/** Print out a single node. */
std::ostream &operator<< (std::ostream &, const Panose &);

}

#endif
