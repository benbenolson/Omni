/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
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
 */
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "Expression.hpp"

const bool vfDebugOutput = false;

/* Function prototypes...
*/
int     numberOfDecimalPlaces      (char *pszNumber);

Expression::
Expression (Expression *pOld)
{
   byte *pbDataOld = pOld->getBinaryData ();

   cbData_d  = pOld->getSize ();
   iEnd_d    = pOld->getLength ();
   eType_d   = pOld->getType ();
   pszData_d = (char *)malloc (cbData_d);
   if (pszData_d)
   {
      memcpy (pszData_d, pbDataOld, iEnd_d);

      if (eType_d == TYPE_STRING)
      {
         pszData_d[iEnd_d] = '\0';
      }
   }
   else
   {
      iEnd_d = 0;
   }
}

Expression::
Expression (char *pszValue, bool fLiteral)
{
   if (!pszValue)
   {
      throw new ExpressionException (TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_STRING,
                                     __FILE__,
                                     __LINE__);
   }

   int cbValue = strlen (pszValue) + 1;

   pszData_d = 0;
   iEnd_d    = commonInitString (pszValue, cbValue, fLiteral);

   iEnd_d--;

   eType_d = TYPE_STRING;
}

Expression::
Expression (char *pszStart, char *pszEnd, bool fLiteral)
{
   if (  !pszStart
      || !pszEnd
      )
   {
      throw new ExpressionException (TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_TWO_STRINGS,
                                     __FILE__,
                                     __LINE__);
   }

   int cbValue = (pszEnd - pszStart + 1);

   pszData_d = 0;
   iEnd_d    = commonInitString (pszStart, cbValue, fLiteral);

   if (iEnd_d > 0)
   {
      iEnd_d--;
   }

   eType_d = TYPE_STRING;

   pszData_d[iEnd_d] = '\0';
}

Expression::
~Expression ()
{
   if (pszData_d)
   {
      free (pszData_d);
      pszData_d = 0;
   }
   cbData_d = 0;
   iEnd_d   = 0;
   eType_d  = TYPE_FREED;
}

void Expression::
convertToHex (bool fBigEndian, int iNumBytes) throw (ExpressionException *)
{
#define ADD_DIGIT(n,s) n = n * 10 + *s - '0'

   char  achTemp[64];
   char *pszTemp      = achTemp;
   char *pszNumber    = pszData_d;
   char *pszInTemp    = 0;
   char *pszOutTemp   = 0;
   char *pszOutNumber = pszNumber;
   int   iCurrent     = 0;

   if (!isValidInt ())
   {
      throw new ExpressionException (TYPE_INVALID_NUMBER,
                                     __FILE__,
                                     __LINE__);
   }

   iCurrent = strlen (pszNumber);
   if (iCurrent > (int)sizeof (pszTemp))
   {
      pszTemp = (char *)malloc (iCurrent + 1);
   }

   strcpy (pszTemp, pszNumber);

   do
   {
      pszInTemp  = pszTemp;
      pszOutTemp = pszTemp;
      iCurrent   = 0;

//////if (vfDebugOutput) std::cerr << pszTemp << std::endl;

      do
      {
         ADD_DIGIT (iCurrent, pszInTemp);
         pszInTemp++;

      } while (  *pszInTemp
              && iCurrent < 16
              );

      while (iCurrent >= 16)
      {
/////////if (vfDebugOutput) std::cerr << "iCurrent              = " << iCurrent << std::endl;
/////////if (vfDebugOutput) std::cerr << "(iCurrent >> 4) + '0' = " << (char)((iCurrent >> 4) + '0') << std::endl;

         *pszOutTemp++ = (iCurrent >> 4) + '0';
         iCurrent  = iCurrent % 16;

/////////if (vfDebugOutput) std::cerr << "iCurrent              = " << iCurrent << std::endl;
/////////if (vfDebugOutput) std::cerr << "------------------------" << std::endl;

         while (  *pszInTemp
               && iCurrent < 16
               )
         {
            ADD_DIGIT (iCurrent, pszInTemp);

            if (  *pszInTemp
               && iCurrent < 16
               )
            {
               *pszOutTemp++ = '0';
            }

            pszInTemp++;
         }
      }

      *pszOutTemp++ = '\0';

//////if (vfDebugOutput) std::cerr << pszTemp << ", " << iCurrent << std::endl;

      *pszOutNumber++ = (char)iCurrent;

   } while (*pszTemp);

   *pszOutNumber = 0;

   iEnd_d = 0;

   // convert from nybble per character to byte per character
   pszInTemp  = pszNumber;
   pszOutTemp = pszNumber;
   while (pszInTemp < pszOutNumber)
   {
      unsigned char uchValue = pszInTemp[0] | (pszInTemp[1] << 4);

      pszInTemp += 2;
      *pszOutTemp++ = uchValue;
      iEnd_d++;
   }

   if (iNumBytes > cbData_d)
   {
      reallocData (iNumBytes);
   }

   // swap the bytes if necessary
   if (fBigEndian)
   {
      byte *pbFrom = (byte *)pszData_d;
      byte *pbTo   = pbFrom + iEnd_d - 1;

      while (pbFrom < pbTo)
      {
         byte bSwap = *pbFrom;
         *pbFrom = *pbTo;
         *pbTo   = bSwap;
         pbFrom++;
         pbTo--;
      }
   }

   if (iNumBytes > 0)
   {
      // right shift the bytes if necessary
      if (iNumBytes > iEnd_d)
      {
         int iDifference = iNumBytes - iEnd_d;

         memmove ((byte *)pszData_d + iDifference,
                  (byte *)pszData_d,
                  iEnd_d);
         memset ((byte *)pszData_d, 0, iDifference);
      }

      iEnd_d = iNumBytes;
   }

   eType_d = TYPE_HEX;

   // clean up
   if (pszTemp != achTemp)
   {
      free (pszTemp);
   }
}

bool Expression::
containsExpression ()
{
   if (pszData_d)
   {
      for (int i = 0; i < iEnd_d; i++)
      {
         if ('%' == pszData_d[i])
         {
            i++;

            if ('%' == pszData_d[i])
               continue;

            switch (pszData_d[i])
            {
            case 'A':
            case 'F':
            case 'H':
            case 'L':
            case 'K':
            case 'I':
            case 'E':
               return true;
            }
         }
      }
   }

   return false;
}

void Expression::
append (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   int iLength = pNew->getLength ();

   if (iEnd_d + iLength > cbData_d)
   {
      reallocData (cbData_d + iLength);
   }

   switch (pNew->getType ())
   {
   case TYPE_STRING:
   {
      strcpy (pszData_d + iEnd_d, pNew->getString ());
      break;
   }

   case TYPE_HEX:
   {
      byte *pbFrom = pNew->getBinaryData ();
      byte *pbTo   = (byte *)(pszData_d + iEnd_d);
      int   cbData = iLength;

      while (cbData > 0)
      {
         if (!isprint (*pbFrom))
         {
            eType_d = TYPE_HEX;
         }
         *pbTo++ = *pbFrom++;
         cbData--;
      }
      break;
   }
   case TYPE_FREED:
      // Error!
      return;
   }

   iEnd_d += iLength;

   if (  getType () == TYPE_STRING
      && pNew->getType () == TYPE_STRING
      )
   {
      pszData_d[iEnd_d] = '\0';
   }
}

void Expression::
lessThan (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld < iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld < dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 > strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
lessThanEqualTo (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld <= iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld <= dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 >= strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
greaterThan (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld > iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld > dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 < strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
greaterThanEqualTo (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld >= iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld >= dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 <= strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
equalTo (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld == iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld == dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 == strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
notEqualTo (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   if (isValidInt ())
   {
      long int iOld = getIntegerResult ();
      long int iNew = pNew->getIntegerResult ();

      setBooleanValue (iOld != iNew);
   }
   else if (isValidDouble ())
   {
      double dOld = getDoubleResult ();
      double dNew = pNew->getDoubleResult ();

      setBooleanValue (dOld != dNew);
   }
   else if (isValidString ())
   {
      char *pszOld = getString ();
      char *pszNew = pNew->getString ();

      setBooleanValue (0 != strcmp (pszOld, pszNew));
   }
   else
   {
      throw new ExpressionException (TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
                                     __FILE__,
                                     __LINE__);
   }
}

void Expression::
add (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   // validate that they are numbers
   if (  !isValidDouble ()
      || !pNew->isValidDouble ()
      )
   {
      throw new ExpressionException (TYPE_EXPRESSIONS_MUST_BE_NUMBERS,
                                     __FILE__,
                                     __LINE__);
   }

   char *pszOldStart       = getString ();
   char *pszNewStart       = pNew->getString ();
   int   iOldDecimalPlaces = numberOfDecimalPlaces (pszOldStart);
   int   iNewDecimalPlaces = numberOfDecimalPlaces (pszNewStart);

   if (iOldDecimalPlaces != iNewDecimalPlaces)
   {
      if (iOldDecimalPlaces < iNewDecimalPlaces)
      {
         setDecimalPoint (iNewDecimalPlaces);
         pszOldStart = getString ();
      }
      else
      {
         pNew->setDecimalPoint (iOldDecimalPlaces);
         pszNewStart = pNew->getString ();
      }
   }

   char *pszOldCurrent = pszOldStart + getLength () - 1;
   char *pszNewCurrent = pszNewStart + pNew->getLength () - 1;
   int   iValue        = 0;

   while (  *pszOldCurrent
         && *pszNewCurrent
         && pszOldCurrent >= pszOldStart
         && pszNewCurrent >= pszNewStart
         )
   {
      if ('.' == *pszOldCurrent)
      {
         pszOldCurrent--;
         pszNewCurrent--;
         continue;
      }

      iValue += *pszOldCurrent - '0'
              + *pszNewCurrent - '0';

      *pszOldCurrent = (iValue % 10) + '0';

      iValue /= 10;

      pszOldCurrent--;
      pszNewCurrent--;
   }

   while (  iValue
         || pszNewCurrent >= pszNewStart
         )
   {
      if (pszOldCurrent < pszOldStart)
      {
         // regrow the string
         reallocData (getSize () + 1);
         iEnd_d++;

         // reload the pointers
         pszOldStart   = getString ();
         pszOldCurrent = pszOldStart;

         // right shift the string and put a zero
         memmove (pszOldStart + 1, pszOldStart, getLength ());
         pszOldStart[0] = '0';
      }

      iValue += *pszOldCurrent - '0';
      if (pszNewCurrent >= pszNewStart)
         iValue += *pszNewCurrent - '0';

      *pszOldCurrent = (iValue % 10) + '0';

      iValue /= 10;

      pszOldCurrent--;
      pszNewCurrent--;
   }
}

void Expression::
subtract (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   // validate that they are numbers
   if (  !isValidDouble ()
      || !pNew->isValidDouble ()
      )
   {
      throw new ExpressionException (TYPE_EXPRESSIONS_MUST_BE_NUMBERS,
                                     __FILE__,
                                     __LINE__);
   }

   double dOld = getDoubleResult ();
   double dNew = pNew->getDoubleResult ();

   // Set up an old pointer
   Expression *pOld = this;

   if (dOld < dNew)
   {
      // Swap the numbers
      pOld = pNew;
      pNew = this;
   }

   char *pszOldStart       = pOld->getString ();
   char *pszNewStart       = pNew->getString ();
   int   iOldDecimalPlaces = numberOfDecimalPlaces (pszOldStart);
   int   iNewDecimalPlaces = numberOfDecimalPlaces (pszNewStart);

   if (iOldDecimalPlaces != iNewDecimalPlaces)
   {
      if (iOldDecimalPlaces < iNewDecimalPlaces)
      {
         setDecimalPoint (iNewDecimalPlaces);
         pszOldStart = getString ();
      }
      else
      {
         pNew->setDecimalPoint (iOldDecimalPlaces);
         pszNewStart = pNew->getString ();
      }
   }

   char *pszOldCurrent = pszOldStart + pOld->getLength () - 1;
   char *pszNewCurrent = pszNewStart + pNew->getLength () - 1;
   int   iDigitOld     = 0;
   int   iDigitNew     = 0;

   while (  *pszOldCurrent
         && *pszNewCurrent
         && pszOldCurrent >= pszOldStart
         && pszNewCurrent >= pszNewStart
         )
   {
      if ('.' == *pszOldCurrent)
      {
         pszOldCurrent--;
         pszNewCurrent--;
         continue;
      }

      iDigitOld = *pszOldCurrent - '0';
      iDigitNew = *pszNewCurrent - '0';

      if (iDigitOld < iDigitNew)
      {
         char *pszBorrow = pszOldCurrent - 1;
         int   iBorrow   = 0;

         do
         {
            if ('.' == *pszBorrow)
            {
               pszBorrow--;
               continue;
            }

            iBorrow = (*pszBorrow - '0');

            if (0 == iBorrow)
            {
               pszBorrow--;
            }

         } while (0 == iBorrow);

         iDigitOld += 10;
         iBorrow--;

         *pszBorrow++ = iBorrow + '0';

         while (pszBorrow < pszOldCurrent)
         {
            if ('0' == *pszBorrow)
            {
               *pszBorrow = '9';
            }
            pszBorrow++;
         }
      }

      iDigitOld -= iDigitNew;

      *pszOldCurrent = iDigitOld + '0';

      pszOldCurrent--;
      pszNewCurrent--;
   }

   // Remove leading zeros
   iDigitOld     = 0;
   pszOldCurrent = pszOldStart;
   while ('0' == *pszOldCurrent)
   {
      pszOldCurrent++;
      iDigitOld++;
   }
   if ('.' == *pszOldCurrent)
      iDigitOld--;
   if (0 < iDigitOld)
   {
      int iNewEnd = pOld->getLength () - iDigitOld;

      if (dOld < dNew)
         pNew->reallocData (iNewEnd + 1);

      pOld->iEnd_d = iNewEnd;
      memmove (pszOldStart, pszOldStart + iDigitOld, iNewEnd);
      pszOldStart[iNewEnd] = 0;
   }

   // Place a minus sign in front
   if (dOld < dNew)
   {
      // regrow the string
      reallocData (pOld->getSize () + 1);
      iEnd_d = pOld->getLength () + 1;

      pszOldStart = getString ();

      // right shift the string and put a minus sign
      memcpy (pszOldStart + 1, pOld->getBinaryData (), pOld->getLength ());
      pszOldStart[0] = '-';
      pszOldStart[iEnd_d] = 0;
   }
}

void Expression::
multiply (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   double dOld = getDoubleResult ();
   double dNew = pNew->getDoubleResult ();

   setDoubleValue (dOld * dNew);
}

void Expression::
divide (Expression *pNew) throw (ExpressionException *)
{
   if (!pNew)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   double dOld = getDoubleResult ();
   double dNew = pNew->getDoubleResult ();

   setDoubleValue (dOld / dNew);
}

void Expression::
resizeIntPadLeft (int iNewLength) throw (ExpressionException *)
{
   if (!isValidInt ())
   {
      throw new ExpressionException (TYPE_INVALID_NUMBER,
                                     __FILE__,
                                     __LINE__);
   }

   if (iNewLength > cbData_d)
   {
      reallocData (iNewLength);
   }

   if (iNewLength > iEnd_d)
   {
      // Shift the string to the right
      char *pszFrom = pszData_d + iEnd_d - 1;
      char *pszTo   = pszData_d + iNewLength - 1;
      int   cbData  = iEnd_d;

      while (cbData > 0)
      {
         *pszTo-- = *pszFrom--;
         cbData--;
      }

      // Pad the left with zeros
      cbData = iNewLength - iEnd_d;
      pszTo  = pszData_d;

      while (cbData > 0)
      {
         *pszTo++ = '0';
         cbData--;
      }
   }
   else if (iNewLength > 0)
   {
      iEnd_d = iNewLength;
      pszData_d[iEnd_d] = 0;
   }
}

void Expression::
resizeDoublePadRight (int iNewLength) throw (ExpressionException *)
{
   if (!isValidDouble ())
   {
      throw new ExpressionException (TYPE_INVALID_DOUBLE,
                                     __FILE__,
                                     __LINE__);
   }

   int iDecimalPlaces = numberOfDecimalPlaces (pszData_d);

   if (vfDebugOutput) std::cerr << "iDecimalPlaces = " << iDecimalPlaces << std::endl;
   if (vfDebugOutput) std::cerr << "iNewLength = " << iNewLength << std::endl;

   if (iNewLength > iDecimalPlaces)
   {
      int iNewSize = iEnd_d + iNewLength - iDecimalPlaces;

      if (0 == iDecimalPlaces)
      {
         iNewSize++;
      }

      if (iNewSize > cbData_d)
      {
         reallocData (iNewSize);
      }

      iNewSize = iNewLength - iDecimalPlaces;

      if (0 == iDecimalPlaces)
      {
         pszData_d[iEnd_d++] = '.';
      }

      while (iNewSize > 0)
      {
         pszData_d[iEnd_d++] = '0';
         iNewSize--;
      }

      pszData_d[iEnd_d] = '\0';
   }
   else
   {
      iEnd_d -= iDecimalPlaces - iNewLength;

      pszData_d[iEnd_d] = '\0';
   }
}

int Expression::
getSize ()
{
   return cbData_d;
}

int Expression::
getLength ()
{
   return iEnd_d;
}

ETYPE Expression::
getType ()
{
   return eType_d;
}

char * Expression::
getString ()
{
   return pszData_d;
}

byte * Expression::
getBinaryData ()
{
   return (byte *)pszData_d;
}

bool Expression::
getBooleanResult () throw (ExpressionException *)
{
   if (!isValidBoolean ())
   {
      throw new ExpressionException (TYPE_INVALID_BOOLEAN,
                                     __FILE__,
                                     __LINE__);
   }

   if ('0' == *pszData_d)
   {
      return false;
   }
   else
   {
      return true;
   }
}

long int Expression::
getIntegerResult () throw (ExpressionException *)
{
   long int lResult = 0;

   if (!isValidInt ())
   {
      throw new ExpressionException (TYPE_INVALID_NUMBER,
                                     __FILE__,
                                     __LINE__);
   }

   char *pszData = pszData_d;

   while (*pszData)
   {
      lResult = lResult * 10 + (*pszData - '0');

      pszData++;
   }

   return lResult;
}

double Expression::
getDoubleResult () throw (ExpressionException *)
{
   double dResult = 0;

   if (!isValidDouble ())
   {
      throw new ExpressionException (TYPE_INVALID_DOUBLE,
                                     __FILE__,
                                     __LINE__);
   }

   sscanf (pszData_d, "%lf", &dResult);

   return dResult;
}

std::string Expression::
toString (std::ostringstream& oss)
{
   oss << "{";

   switch (eType_d)
   {
   case TYPE_STRING:
   {
      oss << pszData_d;
      break;
   }

   case TYPE_HEX:
   {
      oss << std::hex;
      for (int i = 0; i < iEnd_d; i++)
      {
         oss << "0x";
         oss.fill ('0');
         oss.width (2);
         oss << (short int)(pszData_d[i] & 0xFF); // @TBD - weird

         if (i + 1 < iEnd_d)
         {
            oss << " ";
         }
      }
      oss << std::dec;
      break;
   }
   default:
   {
      oss << "Unknown type " << eType_d;
   }
   }

   oss << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Expression& const_self)
{
   Expression&        self = const_cast<Expression&>(const_self);
   std::ostringstream oss;

   if (&self)
   {
      os << self.toString (oss);
   }
   else
   {
      os << "(null)";
   }

   return os;
}

int Expression::
commonInitString (char *pszValue,
                  int   cbValue,
                  bool  fLiteral)
{
   reallocData (cbValue);

   char *pszCopy = pszData_d;
   int   cbCopy  = cbValue;

   while (cbCopy > 0)
   {
      if (  !fLiteral
         && ' ' == *pszValue
         )
         cbValue--;
      else
        *pszCopy++ = *pszValue;

      pszValue++;
      cbCopy--;
   }

   return cbValue;
}

void Expression::
reallocData (int cbSize) throw (ExpressionException *)
{
   char *pszOld = pszData_d;
   int   cbOld  = iEnd_d;

   cbData_d  = (cbSize + BLOCK_SIZE - 1) / BLOCK_SIZE * BLOCK_SIZE;
   pszData_d = (char *)malloc (cbData_d);

   if (!pszData_d)
   {
      throw new ExpressionException (TYPE_MEMORY_ALLOCATION_ERROR,
                                     __FILE__,
                                     __LINE__);
   }

   if (pszOld)
   {
      memcpy (pszData_d, pszOld, cbOld);

      free (pszOld);
   }
}

void Expression::
setDecimalPoint (int iNewDecimalPlaces) throw (ExpressionException *)
{
   if (!isValidDouble ())
   {
      throw new ExpressionException (TYPE_INVALID_DOUBLE,
                                     __FILE__,
                                     __LINE__);
   }

   if (vfDebugOutput) std::cerr << "setDecimalPoint() before: " << pszData_d << std::endl;

   int iOldDecimalPlaces = numberOfDecimalPlaces (pszData_d);

   if (iOldDecimalPlaces < iNewDecimalPlaces)
   {
      int iDifference = iNewDecimalPlaces - iOldDecimalPlaces;

      if (0 == iOldDecimalPlaces)
      {
         iDifference++;
      }

      // regrow the string
      reallocData (iDifference);

      if (0 == iOldDecimalPlaces)
      {
         pszData_d[iEnd_d++] = '.';
         iDifference--;
      }

      memset (pszData_d + iEnd_d, '0', iDifference);
      iEnd_d += iDifference;

      pszData_d[iEnd_d] = '\0';
   }

   if (vfDebugOutput) std::cerr << "setDecimalPoint() after:  " << pszData_d << std::endl;
}

bool Expression::
isValidBoolean ()
{
   if (  !pszData_d
      || iEnd_d != 1
      || (  '0' != *pszData_d
         && '1' != *pszData_d
         )
      )
   {
      return false;
   }

   return true;
}

bool Expression::
isValidInt ()
{
   bool fValid = true;

   if (!pszData_d)
   {
      fValid = false;
   }
   else
   {
      for (int i = 0; i < iEnd_d; i++)
      {
         if (  '0' > pszData_d[i]
            || '9' < pszData_d[i]
            )
         {
            fValid = false;
            break;
         }
      }
   }

   return fValid;
}

bool Expression::
isValidDouble ()
{
   bool fValid            = true;
   int  iNumDecimalPoints = 0;

   if (!pszData_d)
   {
      fValid = false;
   }
   else
   {
      for (int i = 0; i < iEnd_d; i++)
      {
         if ('.' == pszData_d[i])
         {
            iNumDecimalPoints++;
         }
         else if ( !(  '0' <= pszData_d[i]
                    && '9' >= pszData_d[i]
                    )
                 )
         {
            fValid = false;
            break;
         }
      }

      if (1 < iNumDecimalPoints)
      {
         fValid = false;
      }
   }

   return fValid;
}

bool Expression::
isValidString ()
{
   if (  TYPE_STRING != eType_d
      || !pszData_d
      )
   {
      return false;
   }

   for (int i = 0; i < iEnd_d; i++)
   {
      if (!isprint (pszData_d[i]))
      {
         return false;
      }
   }

   return true;
}

void Expression::
setBooleanValue (bool fValue)
{
   if (fValue)
   {
      pszData_d[0] = '1';
   }
   else
   {
      pszData_d[0] = '0';
   }
   iEnd_d  = 1;
   eType_d = TYPE_STRING;
   pszData_d[iEnd_d] = 0;
}

void Expression::
setDoubleValue (double dValue)
{
   if ((long int)dValue == dValue)
   {
      iEnd_d = sprintf (pszData_d, "%ld", (long int)dValue);
   }
   else
   {
      iEnd_d = sprintf (pszData_d, "%lf", dValue);
   }
   eType_d = TYPE_STRING;
   pszData_d[iEnd_d] = 0;
}

int
numberOfDecimalPlaces (char *pszNumber)
{
   int iDecimalPlaces = 0;

   while (*pszNumber)
   {
      if ('.' == *pszNumber)
      {
         pszNumber++;
         break;
      }
      pszNumber++;
   }

   if (*pszNumber)
   {
      while (*pszNumber)
      {
         iDecimalPlaces++;
         pszNumber++;
      }
   }

   return iDecimalPlaces;
}
