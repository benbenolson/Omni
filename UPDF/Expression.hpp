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
#ifndef _Expression_hpp
#define _Expression_hpp

#include "ExpressionException.hpp"

#include <string>
#include <sstream>

typedef enum _eType {
   TYPE_FREED,
   TYPE_STRING,
   TYPE_HEX
} ETYPE, *PETYPE;

typedef unsigned char byte;

class Expression
{
public:
                        Expression           (Expression         *pOld);
                        Expression           (char               *pszValue,
                                              bool                fLiteral);
                        Expression           (char               *pszStart,
                                              char               *pszEnd,
                                              bool                fLiteral);
   virtual             ~Expression           ();

   void                 convertToHex         (bool                fBigEndian,
                                              int                 iNumBytes)         throw (ExpressionException *);

   bool                 containsExpression   ();

   void                 append               (Expression         *pNew)              throw (ExpressionException *);
   void                 lessThan             (Expression         *pNew)              throw (ExpressionException *);
   void                 lessThanEqualTo      (Expression         *pNew)              throw (ExpressionException *);
   void                 greaterThan          (Expression         *pNew)              throw (ExpressionException *);
   void                 greaterThanEqualTo   (Expression         *pNew)              throw (ExpressionException *);
   void                 equalTo              (Expression         *pNew)              throw (ExpressionException *);
   void                 notEqualTo           (Expression         *pNew)              throw (ExpressionException *);

   void                 add                  (Expression         *pNew)              throw (ExpressionException *);
   void                 subtract             (Expression         *pNew)              throw (ExpressionException *);
   void                 multiply             (Expression         *pNew)              throw (ExpressionException *);
   void                 divide               (Expression         *pNew)              throw (ExpressionException *);

   void                 resizeIntPadLeft     (int                 iNewLength)        throw (ExpressionException *);
   void                 resizeDoublePadRight (int                 iNewLength)        throw (ExpressionException *);

   int                  getSize              ();
   int                  getLength            ();
   ETYPE                getType              ();

   char                *getString            ();
   byte                *getBinaryData        ();
   bool                 getBooleanResult     ()                                      throw (ExpressionException *);
   long int             getIntegerResult     ()                                      throw (ExpressionException *);
   double               getDoubleResult      ()                                      throw (ExpressionException *);

   virtual std::string  toString             (std::ostringstream& oss);
   friend std::ostream&      operator<<      (std::ostream&       os,
                                              const Expression&   self);

private:
   enum {
      BLOCK_SIZE = 64
   };

   int                  commonInitString     (char               *pszValue,
                                              int                 cbValue,
                                              bool                fLiteral);

   void                 reallocData          (int                 cbSize)            throw (ExpressionException *);

   void                 setDecimalPoint      (int                 iNewDecimalPlaces) throw (ExpressionException *);

   bool                 isValidBoolean       ();
   bool                 isValidInt           ();
   bool                 isValidDouble        ();
   bool                 isValidString        ();

   void                 setBooleanValue      (bool                fVaule);
   void                 setDoubleValue       (double              dValue);

   char *pszData_d;
   int   cbData_d;
   int   iEnd_d;
   ETYPE eType_d;
};

#endif
