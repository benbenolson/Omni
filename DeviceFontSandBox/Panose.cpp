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
 *   Oct 16, 2002
 *   $Id: Panose.cpp,v 1.5 2002/11/14 05:58:55 rcktscientist Exp $
 */

#include "Panose.hpp"

#include <string>

using namespace std;
using namespace DevFont;

/**
 * The default constructor just initializes all the variables.
 */
Panose::Panose ():
  familyType(FAMILY_ANY), serifStyle(SERIF_ANY),
  weight(WEIGHT_ANY), proportion(PROP_ANY), contrast(CONTRAST_ANY),
  strokeVariation(STROKE_ANY), armStyle(ARMS_ANY), letterform(LETT_ANY),
  midline(MIDLINE_ANY), xHeight(XHEIGHT_ANY)
{
}


/**
 * This constructor takes all ten values (of their appropriate enumerations).
 */
Panose::Panose (Family_Type ft, Serif_Style ss, Weight w, Proportion p, 
                Contrast c, Stroke_Variation sv, Arm_Style as, Letterform l,
                Midline m, X_Height xh):
  familyType(ft), serifStyle(ss),  weight(w), proportion(p), contrast(c),
  strokeVariation(sv), armStyle(as), letterform(l), midline(m), xHeight(xh)
{
}


/**
 * This constructor takes all ten values (as ints).  Panose defines particular
 * values for each member of the enumeration, so we can just cast these values.
 */
Panose::Panose (int ft, int ss, int w, int p, int c,
                int sv, int as, int l, int m, int xh):
  familyType(static_cast<Family_Type>(ft)),
  serifStyle(static_cast<Serif_Style>(ss)),
  weight(static_cast<Weight>(w)),
  proportion(static_cast<Proportion>(p)),
  contrast(static_cast<Contrast>(c)),
  strokeVariation(static_cast<Stroke_Variation>(sv)),
  armStyle(static_cast<Arm_Style>(as)),
  letterform(static_cast<Letterform>(l)),
  midline(static_cast<Midline>(m)),
  xHeight(static_cast<X_Height>(xh))
{
}


/**
 * The copy constructor copies all the variables.
 */
Panose::Panose (const Panose & p)
{
  copy (p);
}

/**
 * Since we supplied a copy constructor, we must define an equality operator.
 */
Panose & Panose::operator= (const Panose &p)
{
  if (this == &p)
    return *this;
  copy (p);
  return *this;
}


/**
 * Copy the values from the given Panose object to the current one; used by
 * both the copy constructor and the assignment operator.
 */
void Panose::copy (const Panose &p)
{
  familyType = p.familyType;
  serifStyle = p.serifStyle;
  weight = p.weight;
  proportion = p.proportion;
  contrast = p.contrast;
  strokeVariation = p.strokeVariation;
  armStyle = p.armStyle;
  letterform = p.letterform;
  midline = p.midline;
  xHeight = p.xHeight;
}


/**
 * Brute-force method to return a Panose Family_Type for a given UPDF string.
 */
Panose::Family_Type Panose::getFamilyType (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return FAMILY_NO_FIT;
  else if (strcmp("PAN_FAMILY_TEXT_DISPLAY", str) == 0)
    return FAMILY_TEXT_DISPLAY;
  else if (strcmp("PAN_FAMILY_SCRIPT", str) == 0)
    return FAMILY_SCRIPT;
  else if (strcmp("PAN_FAMILY_DECORATIVE", str) == 0)
    return FAMILY_DECORATIVE;
  else if (strcmp("PAN_FAMILY_PICTORIAL", str) == 0)
    return FAMILY_PICTORIAL;
  else
    return FAMILY_ANY;
}


/**
 * Brute-force method to return a Panose Family_Type for a given UPDF string.
 */
Panose::Serif_Style Panose::getSerifStyle (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return SERIF_NO_FIT;
  else if (strcmp("PAN_SERIF_COVE", str) == 0)
    return SERIF_COVE;
  else if (strcmp("PAN_SERIF_OBTUSE_COVE", str) == 0)
    return SERIF_OBTUSE_COVE;
  else if (strcmp("PAN_SERIF_SQARE_COVE", str) == 0)
    return SERIF_SQARE_COVE;
  else if (strcmp("PAN_SERIF_OBTUSE_SQARE_COVE", str) == 0)
    return SERIF_OBTUSE_SQARE_COVE;
  else if (strcmp("PAN_SERIF_SQARE", str) == 0)
    return SERIF_SQARE;
  else if (strcmp("PAN_SERIF_THIN", str) == 0)
    return SERIF_THIN;
  else if (strcmp("PAN_SERIF_BONE", str) == 0)
    return SERIF_BONE;
  else if (strcmp("PAN_SERIF_EXAGGERATED,", str) == 0)
    return SERIF_EXAGGERATED;
  else if (strcmp("PAN_SERIF_TRIANGLE", str) == 0)
    return SERIF_TRIANGLE;
  else if (strcmp("PAN_SERIF_NORMAL_SANS", str) == 0)
    return SERIF_NORMAL_SANS;
  else if (strcmp("PAN_SERIF_OBTUSE_SANS", str) == 0)
    return SERIF_OBTUSE_SANS;
  else if (strcmp("PAN_SERIF_PERP_SANS", str) == 0)
    return SERIF_PERP_SANS;
  else if (strcmp("PAN_SERIF_FLARED", str) == 0)
    return SERIF_FLARED;
  else if (strcmp("PAN_SERIF_ROUNDED", str) == 0)
    return SERIF_ROUNDED;
  else
    return SERIF_ANY;
}


/**
 * Brute-force method to return a Panose Weight for a given UPDF string.
 */
Panose::Weight Panose::getWeight (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return WEIGHT_NO_FIT;
  else if (strcmp("PAN_WEIGHT_VERY_LIGHT", str) == 0)
    return WEIGHT_VERY_LIGHT;
  else if (strcmp("PAN_WEIGHT_LIGHT", str) == 0)
    return WEIGHT_LIGHT;
  else if (strcmp("PAN_WEIGHT_THIN", str) == 0)
    return WEIGHT_THIN;
  else if (strcmp("PAN_WEIGHT_BOOK", str) == 0)
    return WEIGHT_BOOK;
  else if (strcmp("PAN_WEIGHT_MEDIUM", str) == 0)
    return WEIGHT_MEDIUM;
  else if (strcmp("PAN_WEIGHT_DEMI", str) == 0)
    return WEIGHT_DEMI;
  else if (strcmp("PAN_WEIGHT_BOLD", str) == 0)
    return WEIGHT_BOLD;
  else if (strcmp("PAN_WEIGHT_HEAVY", str) == 0)
    return WEIGHT_HEAVY;
  else if (strcmp("PAN_WEIGHT_BLACK", str) == 0)
    return WEIGHT_BLACK;
  else if (strcmp("PAN_WEIGHT_NORD", str) == 0)
    return WEIGHT_NORD;
  else
    return WEIGHT_ANY;
}

/**
 * Brute-force method to return a Panose Proportion for a given UPDF string.
 */
Panose::Proportion Panose::getProportion (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return PROP_NO_FIT;
  else if (strcmp("PAN_PROP_OLD_STYLE", str) == 0)
    return PROP_OLD_STYLE;
  else if (strcmp("PAN_PROP_MODERN", str) == 0)
    return PROP_MODERN;
  else if (strcmp("PAN_PROP_EVEN_WIDTH", str) == 0)
    return PROP_EVEN_WIDTH;
  else if (strcmp("PAN_PROP_EXPANDED", str) == 0)
    return PROP_EXPANDED;
  else if (strcmp("PAN_PROP_CONDENSED", str) == 0)
    return PROP_CONDENSED;
  else if (strcmp("PAN_PROP_VERY_EXPANDED", str) == 0)
    return PROP_VERY_EXPANDED;
  else if (strcmp("PAN_PROP_VERY_CONDENSED", str) == 0)
    return PROP_VERY_CONDENSED;
  else if (strcmp("PAN_PROP_MONOSPACED", str) == 0)
    return PROP_MONOSPACED;
  else
    return PROP_ANY;
}

/**
 * Brute-force method to return a Panose Contrast for a given UPDF string.
 */
Panose::Contrast Panose::getContrast (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return CONTRAST_NO_FIT;
  else if (strcmp("PAN_CONTRAST_NONE", str) == 0)
    return CONTRAST_NONE;
  else if (strcmp("PAN_CONTRAST_VERY_LOW", str) == 0)
    return CONTRAST_VERY_LOW;
  else if (strcmp("PAN_CONTRAST_LOW", str) == 0)
    return CONTRAST_LOW;
  else if (strcmp("PAN_CONTRAST_MEDIUM_LOW", str) == 0)
    return CONTRAST_MEDIUM_LOW;
  else if (strcmp("PAN_CONTRAST_MEDIUM", str) == 0)
    return CONTRAST_MEDIUM;
  else if (strcmp("PAN_CONTRAST_MEDIUM_HIGH", str) == 0)
    return CONTRAST_MEDIUM_HIGH;
  else if (strcmp("PAN_CONTRAST_HIGH", str) == 0)
    return CONTRAST_HIGH;
  else if(strcmp("PAN_CONTRAST_VERY_HIGH", str) == 0)
    return CONTRAST_VERY_HIGH;
  else
    return CONTRAST_ANY;
}

/**
 * Brute-force method to return a Panose Contrast for a given UPDF string.
 */
Panose::Stroke_Variation Panose::getStrokeVariation (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return STROKE_NO_FIT;
  else if (strcmp("PAN_STROKE_GRADUAL_DIAG", str) == 0)
    return STROKE_GRADUAL_DIAG;
  else if (strcmp("PAN_STROKE_GRADUAL_TRAN", str) == 0)
    return STROKE_GRADUAL_TRAN;
  else if (strcmp("PAN_STROKE_GRADUAL_VERT", str) == 0)
    return STROKE_GRADUAL_VERT;
  else if (strcmp("PAN_STROKE_GRADUAL_HORZ", str) == 0)
    return STROKE_GRADUAL_HORZ;
  else if (strcmp("PAN_STROKE_RAPID_VERT", str) == 0)
    return STROKE_RAPID_VERT;
  else if (strcmp("PAN_STROKE_RAPID_HORZ", str) == 0)
    return STROKE_RAPID_HORZ;
  else if (strcmp("PAN_STROKE_INSTANT_VERT", str) == 0)
    return STROKE_INSTANT_VERT;
  else
    return STROKE_ANY;
}

/**
 * Brute-force method to return a Panose Contrast for a given UPDF string.
 */
Panose::Arm_Style Panose::getArmStyle (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return ARMS_NO_FIT;
  else if (strcmp("PAN_STRAIGHT_ARMS_HORZ", str) == 0)
    return STRAIGHT_ARMS_HORZ;
  else if (strcmp("PAN_STRAIGHT_ARMS_WEDGE", str) == 0)
    return STRAIGHT_ARMS_WEDGE;
  else if (strcmp("PAN_STRAIGHT_ARMS_VERT", str) == 0)
    return STRAIGHT_ARMS_VERT;
  else if (strcmp("PAN_STRAIGHT_ARMS_SINGLE_SERIF", str) == 0)
    return STRAIGHT_ARMS_SINGLE_SERIF;
  else if (strcmp("PAN_STRAIGHT_ARMS_DOUBLE_SERIF", str) == 0)
    return STRAIGHT_ARMS_DOUBLE_SERIF;
  else if (strcmp("PAN_BENT_ARMS_HORZ", str) == 0)
    return BENT_ARMS_HORZ;
  else if (strcmp("PAN_BENT_ARMS_WEDGE", str) == 0)
    return BENT_ARMS_WEDGE;
  else if (strcmp("PAN_BENT_ARMS_VERT", str) == 0)
    return BENT_ARMS_VERT;
  else if (strcmp("PAN_BENT_ARMS_SINGLE_SERIF", str) == 0)
    return BENT_ARMS_SINGLE_SERIF;
  else if (strcmp("PAN_BENT_ARMS_DOUBLE_SERIF", str) == 0)
    return BENT_ARMS_DOUBLE_SERIF;
  else
    return ARMS_ANY;
}

/**
 * Brute-force method to return a Panose Letterform for a given UPDF string.
 */
Panose::Letterform Panose::getLetterform (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return LETT_NO_FIT;
  else if (strcmp("PAN_LETT_NORMAL_CONTACT", str) == 0)
    return LETT_NORMAL_CONTACT;
  else if (strcmp("PAN_LETT_NORMAL_WEIGHTED", str) == 0)
    return LETT_NORMAL_WEIGHTED;
  else if (strcmp("PAN_LETT_NORMAL_BOXED", str) == 0)
    return LETT_NORMAL_BOXED;
  else if (strcmp("PAN_LETT_NORMAL_FLATTENED", str) == 0)
    return LETT_NORMAL_FLATTENED;
  else if (strcmp("PAN_LETT_NORMAL_ROUNDED", str) == 0)
    return LETT_NORMAL_ROUNDED;
  else if (strcmp("PAN_LETT_NORMAL_OFF_CENTER", str) == 0)
    return LETT_NORMAL_OFF_CENTER;
  else if (strcmp("PAN_LETT_NORMAL_SQARE", str) == 0)
    return LETT_NORMAL_SQARE;
  else if (strcmp("PAN_LETT_OBLIQUE_CONTACT", str) == 0)
    return LETT_OBLIQUE_CONTACT;
  else if (strcmp("PAN_LETT_OBLIQUE_WEIGHTED", str) == 0)
    return LETT_OBLIQUE_WEIGHTED;
  else if (strcmp("PAN_LETT_OBLIQUE_BOXED", str) == 0)
    return LETT_OBLIQUE_BOXED;
  else if (strcmp("PAN_LETT_OBLIQUE_FLATTENED", str) == 0)
    return LETT_OBLIQUE_FLATTENED;
  else if (strcmp("PAN_LETT_OBLIQUE_ROUNDED", str) == 0)
    return LETT_OBLIQUE_ROUNDED;
  else if (strcmp("PAN_LETT_OBLIQUE_OFF_CENTER", str) == 0)
    return LETT_OBLIQUE_OFF_CENTER;
  else if (strcmp("PAN_LETT_OBLIQUE_SQARE", str) == 0)
    return LETT_OBLIQUE_SQARE;
  else
    return LETT_ANY;
}

/**
 * Brute-force method to return a Panose Midline for a given UPDF string.
 */
Panose::Midline Panose::getMidline (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return MIDLINE_NO_FIT;
  else if (strcmp("PAN_MIDLINE_STANDARD_TRIMMED", str) == 0)
    return MIDLINE_STANDARD_TRIMMED;
  else if (strcmp("PAN_MIDLINE_STANDARD_POINTED", str) == 0)
    return MIDLINE_STANDARD_POINTED;
  else if (strcmp("PAN_MIDLINE_STANDARD_SERIFED", str) == 0)
    return MIDLINE_STANDARD_SERIFED;
  else if (strcmp("PAN_MIDLINE_HIGH_TRIMMED", str) == 0)
    return MIDLINE_HIGH_TRIMMED;
  else if (strcmp("PAN_MIDLINE_HIGH_POINTED", str) == 0)
    return MIDLINE_HIGH_POINTED;
  else if (strcmp("PAN_MIDLINE_HIGH_SERIFED", str) == 0)
    return MIDLINE_HIGH_SERIFED;
  else if (strcmp("PAN_MIDLINE_CONSTANT_TRIMMED", str) == 0)
    return MIDLINE_CONSTANT_TRIMMED;
  else if (strcmp("PAN_MIDLINE_CONSTANT_POINTED", str) == 0)
    return MIDLINE_CONSTANT_POINTED;
  else if (strcmp("PAN_MIDLINE_CONSTANT_SERIFED", str) == 0)
    return MIDLINE_CONSTANT_SERIFED;
  else if (strcmp("PAN_MIDLINE_LOW_TRIMMED", str) == 0)
    return MIDLINE_LOW_TRIMMED;
  else if (strcmp("PAN_MIDLINE_LOW_POINTED", str) == 0)
    return MIDLINE_LOW_POINTED;
  else if (strcmp("PAN_MIDLINE_LOW_SERIFED", str) == 0)
    return MIDLINE_LOW_SERIFED;
  else
    return MIDLINE_ANY;
}

/**
 * Brute-force method to return a Panose Letterform for a given UPDF string.
 */
Panose::X_Height Panose::getXHeight (const char *str)
{
  if (strcmp("PAN_NO_FIT", str) == 0)
    return XHEIGHT_NO_FIT;
  else if (strcmp("PAN_XHEIGHT_CONSTANT_SMALL", str) == 0)
    return XHEIGHT_CONSTANT_SMALL;
  else if (strcmp("PAN_XHEIGHT_CONSTANT_STD", str) == 0)
    return XHEIGHT_CONSTANT_STD;
  else if (strcmp("PAN_XHEIGHT_CONSTANT_LARGE", str) == 0)
    return XHEIGHT_CONSTANT_LARGE;
  else if (strcmp("PAN_XHEIGHT_DUCKING_SMALL", str) == 0)
    return XHEIGHT_DUCKING_SMALL;
  else if (strcmp("PAN_XHEIGHT_DUCKING_STD", str) == 0)
    return XHEIGHT_DUCKING_STD;
  else if (strcmp("PAN_XHEIGHT_DUCKING_LARGE", str) == 0)
    return XHEIGHT_DUCKING_LARGE;
  else
    return XHEIGHT_ANY;
}


/**
 * Returns the difference between the significant fields of the current Panose
 * object and the given one.  The contract for this method says that an exact
 * match will produce the lowest possible integer value, and fonts that differ
 * will produce larger ones.
 *
 * In fact, this implementation produces 0 for an exact match, and calculates
 * the sum of the squares of the differences between the significant fields.
 *
 * This implementation is completely wrong, however.  Some things are
 * continuous (such as weight) but some are discrete (like family type), so we
 * really should use the sum-of-the-squares thing for continuous values, and
 * have sort of large penalty for discrete ones.
 *
 * @param p  the Panose object to compare to
 */
int Panose::diff (const Panose &p) const
{
  int result = 0;
  //
  // If they aren't in the same family, then it really makes no sense to
  // compare them at all, so throw in some big penalty.
  //
  if (familyType != FAMILY_ANY && p.familyType != FAMILY_ANY &&
      familyType != p.familyType)
    result = 5000;
  //
  // It's debatable whether the Serif_Style number is monotonic, but we'll
  // treat it as such in this implementation.
  //
  result += calcdiffm (static_cast<int>(weight),
                       static_cast<int>(p.weight), 12);
  result += calcdiffm (static_cast<int>(contrast),
                       static_cast<int>(p.contrast), 10);
  result += calcdiffm (static_cast<int>(strokeVariation),
                       static_cast<int>(p.strokeVariation), 9);
  //
  // The Proportion, Arm_Style, Letterform, Midline and X_Height values aren't
  // monotonically increasing values, so we shouldn't give much weight to
  // to these values (although we could probably do some rearranging to get the
  // Proportion values to be approximately monotonic).
  //
  result += calcdiffss (static_cast<int>(serifStyle),
			static_cast<int>(p.serifStyle));
  result += calcdiffd (static_cast<int>(proportion),
                       static_cast<int>(p.proportion));
  result += calcdiffd (static_cast<int>(armStyle),
                       static_cast<int>(p.armStyle));
  result += calcdifflf (static_cast<int>(letterform),
			static_cast<int>(p.letterform));
  result += calcdiffd (static_cast<int>(midline),
                       static_cast<int>(p.midline));
  result += calcdiffd (static_cast<int>(xHeight),
                       static_cast<int>(p.xHeight));
  return result;
}


/**
 * Calculate the square of the difference between two monotonic Panose values
 * (as integers), with a weight applied.  If either value is 0 (xxx_ANY), then
 * value returned is zero.  If one value is 1 (xxx_NO_FIT), then they can't
 * can't be classified, so we give a penalty.  This method will return some
 * value between 0 and 1000.  This method only makes sense when the values
 * are in a continuous range.
 *
 * @param p1      first Panose value
 * @param p2      second Panose value
 * @param weight  the number of Panose values in this enumeration
 */
int Panose::calcdiffm (int p1, int p2, int weight)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Now return some number between 0 and 1000 (the value "weight-3" is the
  // maximum difference between p1 and p2).
  //
  return 1000 * (p1-p2)*(p1-p2) / (weight-3)*(weight-3);
}


/**
 * Calculate the square of the difference between two discrete (non-monotonic)
 * Panose values (as integers).  If either value is 0 (xxx_ANY), then value
 * returned is zero.  If one value is 1 (xxx_NO_FIT), then they can't can't be
 * classified, so we give a penalty.  This method will return some
 * value between 0 and 100.
 *
 * @param p1      first Panose value
 * @param p2      second Panose value
 */
int Panose::calcdiffd (int p1, int p2)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0 || p1 == p2)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Otherwise, we really can't compare the values sensible, so just return
  // some small penalty.
  //
  return 100;
}


/**
 * Calculate the difference between two serif styles; we treat differences
 * any two serifs or sans-serifs as small, and the difference between any
 * serif and any sans-serif as large.
 */
int Panose::calcdiffss (int p1, int p2)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0 || p1 == p2)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Otherwise, check to see if they are serif or sans-serif
  //
  if (p1 >= (int)SERIF_NORMAL_SANS && p1 <= (int)SERIF_PERP_SANS &&
      p2 >= (int)SERIF_NORMAL_SANS && p2 <= (int)SERIF_PERP_SANS)
    return 10;
  if ((p1 < (int)SERIF_NORMAL_SANS || p1 > (int)SERIF_PERP_SANS) &&
      (p2 < (int)SERIF_NORMAL_SANS || p2 > (int)SERIF_PERP_SANS))
    return 10;
  else
    return 100;
}


/**
 * Calculate the difference between two letter forms; we treat differences
 * any two normals or obliques as small, and the difference between any
 * normal and any oblique as large.
 */
int Panose::calcdifflf (int p1, int p2)
{
  //
  // If they can have any value, then they automatically match
  //
  if (p1 == 0 || p2 == 0 || p1 == p2)
    return 0;
  //
  // If one (but not both) value is xxx_NO_FIT, then they can't match
  //
  if ((p1 == 1 && p2 != 1) || (p2 == 1 && p1 != 1))
    return 1000;
  //
  // Otherwise, check to see if they are normal or oblique
  //
  if ((p1 <  (int)LETT_OBLIQUE_CONTACT && p2 <  (int)LETT_OBLIQUE_CONTACT) ||
      (p1 >= (int)LETT_OBLIQUE_CONTACT && p2 >= (int)LETT_OBLIQUE_CONTACT))
    return 10;
  else
    return 100;
}





namespace DevFont
{
/**
 * Write the Panose information to the given output stream.
 */
std::ostream & operator<< (std::ostream &o, const Panose &p)
{
    o << "Panose[" << p.familyType << ","
      << p.serifStyle << ","
      << p.weight << ","
      << p.proportion << ","
      << p.contrast << ","
      << p.strokeVariation << ","
      << p.armStyle << ","
      << p.letterform << ","
      << p.midline << ","
      << p.xHeight << "]";
  return o;
}
}
