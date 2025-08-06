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
#ifndef _ExpressionException_hpp
#define _ExpressionException_hpp

#include <string>
#include <sstream>

typedef enum _ExceptionType {
   TYPE_INVALID_EXPRESSION = 1,
   TYPE_NULL_EXPRESSION,
   TYPE_MEMORY_ALLOCATION_ERROR,
   TYPE_VARIABLE_NOT_FOUND,
   TYPE_INVALID_NUMBER,
   TYPE_INVALID_DOUBLE,
   TYPE_INVALID_BOOLEAN,
   TYPE_INVALID_A_EXPRESSION,
   TYPE_A_ON_EXPRESSION,
   TYPE_INVALID_F_EXPRESSION,
   TYPE_INVALID_H_EXPRESSION,
   TYPE_INVALID_L_EXPRESSION,
   TYPE_I_NEEDS_PARENTHESIS,
   TYPE_I_IF_REQUIRED,
   TYPE_UNKNOWN_PERCENT_OPERATOR,
   TYPE_MISSING_COMPARISON_EXPRESSION,
   TYPE_NOT_NOT_EQUALS,
   TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_STRING,
   TYPE_EXPRESSION_CONSTRUCTOR_NEEDS_TWO_STRINGS,
   TYPE_NOT_A_BOOL_INT_DOUBLE_STRING,
   TYPE_EXPRESSIONS_MUST_BE_NUMBERS
} EXCEPTIONTYPE;

class ExpressionException
{
public:
                        ExpressionException  (EXCEPTIONTYPE              type,
                                              char                      *pszFile,
                                              int                        iLine);

   EXCEPTIONTYPE        getType              ();
   bool                 isType               (EXCEPTIONTYPE              type);

   virtual std::string  toString             (std::ostringstream&        oss);
   friend std::ostream& operator<<           (std::ostream&              os,
                                              const ExpressionException& self);

private:
   EXCEPTIONTYPE  eType_d;
   char          *pszFile_d;
   int            iLine_d;
};

#endif
