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
#include "ExpressionException.hpp"

ExpressionException::
ExpressionException (EXCEPTIONTYPE  type,
                     char          *pszFile,
                     int            iLine)
{
   eType_d   = type;
   pszFile_d = pszFile;
   iLine_d   = iLine;
}

EXCEPTIONTYPE ExpressionException::
getType ()
{
   return eType_d;
}

bool ExpressionException::
isType (EXCEPTIONTYPE type)
{
   return type == eType_d;
}

std::string ExpressionException::
toString (std::ostringstream& oss)
{
   oss << "{";

   switch (eType_d)
   {
   case TYPE_INVALID_EXPRESSION:                       oss << "Invalid expression"; break;
   case TYPE_NULL_EXPRESSION:                          oss << "Expression is null"; break;
   case TYPE_MEMORY_ALLOCATION_ERROR:                  oss << "Memory allocation error"; break;
   case TYPE_VARIABLE_NOT_FOUND:                       oss << "Variable not found"; break;
   case TYPE_INVALID_NUMBER:                           oss << "Expression is not a number"; break;
   case TYPE_INVALID_DOUBLE:                           oss << "Expression is not a double"; break;
   case TYPE_INVALID_BOOLEAN:                          oss << "Expression is not a boolean"; break;
   case TYPE_INVALID_A_EXPRESSION:                     oss << "No expression after %A"; break;
   case TYPE_A_ON_EXPRESSION:                          oss << "Expression in %A contains an expression"; break;
   case TYPE_INVALID_F_EXPRESSION:                     oss << "No expression after %F"; break;
   case TYPE_INVALID_H_EXPRESSION:                     oss << "No expression after %H"; break;
   case TYPE_INVALID_L_EXPRESSION:                     oss << "No expression after %L"; break;
   case TYPE_I_NEEDS_PARENTHESIS:                      oss << "Parenthesis needed for if statement"; break;
   case TYPE_I_IF_REQUIRED:                            oss << "Statement needed for if"; break;
   case TYPE_UNKNOWN_PERCENT_OPERATOR:                 oss << "Unknown percent operator"; break;
   case TYPE_MISSING_COMPARISON_EXPRESSION:            oss << "Missing comparison expression"; break;
   case TYPE_NOT_NOT_EQUALS:                           oss << "no equals after exclamation point"; break;
   case TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_STRING:      oss << "Expression::Expression(char *pszValue) constructor needs a string!"; break;
   case TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_TWO_STRINGS: oss << "Expression::Expression(char *pszStart, char *pszEnd) constructor needs two strings!"; break;
   case TYPE_NOT_A_BOOL_INT_DOUBLE_STRING:             oss << "Expression is not a boolean, int, double, or string!"; break;
   case TYPE_EXPRESSIONS_MUST_BE_NUMBERS:              oss << "Both expressions must be numbers!"; break;
   }

   oss << " @ " << pszFile_d << ":" << iLine_d;

   oss << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const ExpressionException& const_self)
{
   ExpressionException& self = const_cast<ExpressionException&>(const_self);
   std::ostringstream   oss;

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
