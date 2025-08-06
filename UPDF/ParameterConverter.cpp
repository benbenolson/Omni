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
#include <string.h>
#include <ctype.h>
#include <iostream>

#include "ParameterConverter.hpp"

const bool vfDebugOutput = false;

#define min(a,b) ((a) < (b) ? (a) : (b))

typedef enum {
   STATEMENT_A,
   STATEMENT_F,
   STATEMENT_H,
   STATEMENT_L,
   STATEMENT_K,
   STATEMENT_IF,
   STATEMENT_ELSE
} ESTATEMENT, *PESTATEMENT;

typedef enum {
   OPERATOR_NONE,
   OPERATOR_EQUALS,
   OPERATOR_NOT_EQUALS,
   OPERATOR_LESSTHAN,
   OPERATOR_GREATERTHAN,
   OPERATOR_LESSTHAN_EQUALS,
   OPERATOR_GREATERTHAN_EQUALS,
   OPERATOR_TIMES,
   OPERATOR_DIVIDE
} OPERATOR, *POPERATOR;

bool        statementIs        (char          *pszParameter,
                                ESTATEMENT     eType);
bool        evaluateExpression (char         **ppszExpression,
                                PSYMBOLTABLE   pSymbolTable)                throw (ExpressionException *);
Expression *lookupVariable     (char         **ppszParameter,
                                PSYMBOLTABLE   pSymbolTable)                throw (ExpressionException *);
int         number             (char         **ppszParameter)               throw (ExpressionException *);
bool        isPrintable        (char          *pszString);
void        outputString       (char          *pszString);

bool
statementIs (char *pszParameter, ESTATEMENT eType)
{
   if (  !pszParameter
      || !*pszParameter
      )
   {
      return false;
   }

   while (  *pszParameter
         && *pszParameter == ' '
         )
   {
      pszParameter++;
   }

   if (  *pszParameter == '%'
      && *(pszParameter + 1)
      )
   {
      pszParameter++;

      switch (*pszParameter)
      {
      case 'A': return (eType == STATEMENT_A)    ? true : false;
      case 'F': return (eType == STATEMENT_F)    ? true : false;
      case 'H': return (eType == STATEMENT_H)    ? true : false;
      case 'L': return (eType == STATEMENT_L)    ? true : false;
      case 'K': return (eType == STATEMENT_K)    ? true : false;
      case 'I': return (eType == STATEMENT_IF)   ? true : false;
      case 'E': return (eType == STATEMENT_ELSE) ? true : false;
      }
   }

   return false;
}

bool
evaluateExpression (char **ppszExpression, PSYMBOLTABLE pSymbolTable) throw (ExpressionException *)
{
   if (  !ppszExpression
      || !*ppszExpression
      || !**ppszExpression
      || **ppszExpression != '('
      )
   {
      throw new ExpressionException (TYPE_INVALID_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   Expression *pResult = convert (ppszExpression, false, false, pSymbolTable);
   bool        fResult = false;

   if (!pResult)
   {
      throw new ExpressionException (TYPE_NULL_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   fResult = pResult->getBooleanResult ();

   delete pResult;

   return fResult;
}

Expression *
lookupVariable (char **ppszParameter, PSYMBOLTABLE pSymbolTable) throw (ExpressionException *)
{
   char        achName[128];
   char       *pszName      = achName;
   char       *pszParameter = 0;
   Expression *pReturn      = 0;

   if (  !ppszParameter
      || !*ppszParameter
      || !**ppszParameter
      || **ppszParameter != '('
      )
   {
      throw new ExpressionException (TYPE_INVALID_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }

   pszParameter = *ppszParameter;

   pszParameter++;

   while (  *pszParameter
         && *pszParameter != ')'
         )
   {
      if (*pszParameter == ' ')
      {
         pszParameter++;
      }
      else
      {
         *pszName++ = *pszParameter++;
      }
   }

   *pszName++ = 0;

   if (*pszParameter != ')')
   {
      throw new ExpressionException (TYPE_INVALID_EXPRESSION,
                                     __FILE__,
                                     __LINE__);
   }
   pszParameter++;

   if (vfDebugOutput) std::cerr << "Name = " << achName << std::endl;

   std::string stringName (achName);

   pReturn = (*pSymbolTable)[stringName];

   if (pReturn)
   {
      pReturn = new Expression (pReturn); // Create a copy
   }

   *ppszParameter = pszParameter;

   if (vfDebugOutput) std::cerr << "lookupVariable returning " << *pReturn << std::endl;

   if (!pReturn)
   {
      throw new ExpressionException (TYPE_VARIABLE_NOT_FOUND,
                                     __FILE__,
                                     __LINE__);
   }

   return pReturn;
}

int
number (char **ppszParameter) throw (ExpressionException *)
{
   int   iReturn      = 0;
   char *pszParameter = 0;

   if (  !ppszParameter
      || !*ppszParameter
      || !**ppszParameter
      || '0' > **ppszParameter
      || '9' < **ppszParameter
      )
   {
      throw new ExpressionException (TYPE_INVALID_NUMBER,
                                     __FILE__,
                                     __LINE__);
   }

   pszParameter = *ppszParameter;

   while (  *pszParameter
         && '0' <= *pszParameter
         && *pszParameter <= '9'
         )
   {
      iReturn = iReturn * 10 + (*pszParameter - '0');

      pszParameter++;
   }

   *ppszParameter = pszParameter;

   if (vfDebugOutput) std::cerr << "number returning " << iReturn << std::endl;

   return iReturn;
}

Expression *
merge (Expression *pExpOld,
       Expression *pExpNew,
       OPERATOR    op) throw (ExpressionException *)
{
   if (pExpNew)
   {
      if (pExpOld)
      {
         switch (op)
         {
         case OPERATOR_NONE:
         {
            if (vfDebugOutput) std::cerr << "appending" << std::endl;
            pExpOld->append (pExpNew);
            break;
         }

         case OPERATOR_EQUALS:
         {
            if (vfDebugOutput) std::cerr << "equals" << std::endl;
            pExpOld->equalTo (pExpNew);
            break;
         }

         case OPERATOR_NOT_EQUALS:
         {
            if (vfDebugOutput) std::cerr << "not equals" << std::endl;
            pExpOld->notEqualTo (pExpNew);
            break;
         }

         case OPERATOR_LESSTHAN:
         {
            if (vfDebugOutput) std::cerr << "less than" << std::endl;
            pExpOld->lessThan (pExpNew);
            break;
         }

         case OPERATOR_GREATERTHAN:
         {
            if (vfDebugOutput) std::cerr << "greater than" << std::endl;
            pExpOld->greaterThan (pExpNew);
            break;
         }

         case OPERATOR_LESSTHAN_EQUALS:
         {
            if (vfDebugOutput) std::cerr << "less than equals" << std::endl;
            pExpOld->lessThanEqualTo (pExpNew);
            break;
         }

         case OPERATOR_GREATERTHAN_EQUALS:
         {
            if (vfDebugOutput) std::cerr << "greater than equals" << std::endl;
            pExpOld->greaterThanEqualTo (pExpNew);
            break;
         }

         case OPERATOR_TIMES:
         {
            if (vfDebugOutput) std::cerr << "times" << std::endl;
            pExpOld->multiply (pExpNew);
            break;
         }

         case OPERATOR_DIVIDE:
         {
            if (vfDebugOutput) std::cerr << "divide" << std::endl;
            pExpOld->divide (pExpNew);
            break;
         }
         }

         if (vfDebugOutput) std::cerr << "Result now contains " << *pExpOld << std::endl;

         delete pExpNew;
      }
      else
      {
         pExpOld = pExpNew;
      }

      pExpNew = 0;
   }
   else if (!pExpOld)
   {
      pExpOld = pExpNew;
   }

   if (pExpNew)
   {
      delete pExpNew;
   }

   return pExpOld;
}

Expression *
convert (char         **ppszParameter,
         bool           fRootLevel,
         bool           fFromIf,
         PSYMBOLTABLE   pSymbolTable) throw (ExpressionException *)
{
   Expression *pResult            = 0;
   Expression *pNew               = 0;
   bool        fContinue          = true;
   char       *pszParameter       = 0;
   char       *pszStartExpression = 0;
   int         iLength            = 0;
   bool        fFirstExpression   = true;
   OPERATOR    opFound            = OPERATOR_NONE;

   if (  !ppszParameter
      || !*ppszParameter
      || !**ppszParameter
      )
   {
      goto done;
   }

   pszParameter = *ppszParameter;

   while (  fContinue
         && *pszParameter
         )
   {
      char *pszCurrent = pszParameter;

      pszStartExpression = pszParameter;

      try
      {
         while (*pszCurrent)
         {
            if (  '%' == *pszCurrent
               || '+' == *pszCurrent
               || '-' == *pszCurrent
               || '*' == *pszCurrent
               || '/' == *pszCurrent
               || '=' == *pszCurrent
               || '!' == *pszCurrent
               || '<' == *pszCurrent
               || '>' == *pszCurrent
               || '(' == *pszCurrent
               || ')' == *pszCurrent
               )
            {
               break;
            }
            else
            {
               pszCurrent++;
            }
         }

         if (pszCurrent - pszParameter > 0)
         {
            pNew = new Expression (pszParameter, pszCurrent, false);

            if (vfDebugOutput) std::cerr << "Created literal " << *pNew << std::endl;
         }

         pResult = merge (pResult, pNew, opFound);
         pNew    = 0;
         opFound = OPERATOR_NONE;

         pszParameter = pszCurrent;

         switch (*pszParameter)
         {
         case '%':
         {
            pszParameter++;

            switch (*pszParameter)
            {
            case '%':
            {
               pszParameter++;
               break;
            }

            case 'A':
            {
               pszParameter++;

               iLength = number (&pszParameter);
               pNew    = convert (&pszParameter, false, false, pSymbolTable);

               if (!pNew)
               {
                  throw new ExpressionException (TYPE_INVALID_A_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
               }

               if (pNew->containsExpression ())
               {
                  throw new ExpressionException (TYPE_A_ON_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
               }
               else
               {
                  pNew->resizeIntPadLeft (iLength);
               }
               break;
            }

            case 'F':
            {
               pszParameter++;

               iLength = number (&pszParameter);
               pNew    = convert (&pszParameter, false, false, pSymbolTable);

               if (!pNew)
               {
                  throw new ExpressionException (TYPE_INVALID_F_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
               }

               pNew->resizeDoublePadRight (iLength);
               break;
            }

            case 'H':
            {
               pszParameter++;

               iLength = number (&pszParameter);
               pNew    = convert (&pszParameter, false, false, pSymbolTable);

               if (!pNew)
               {
                  throw new ExpressionException (TYPE_INVALID_H_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
               }
               else
               {
                  pNew->convertToHex (true, iLength);
               }
               break;
            }

            case 'L':
            {
               pszParameter++;

               iLength = number (&pszParameter);
               pNew    = convert (&pszParameter, false, false, pSymbolTable);

               if (!pNew)
               {
                  throw new ExpressionException (TYPE_INVALID_L_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
               }
               else
               {
                  pNew->convertToHex (false, iLength);
               }
               break;
            }

            case 'K':
            {
               pszParameter++;

               pResult = lookupVariable (&pszParameter, pSymbolTable);
               break;
            }

            case 'I':
            {
               Expression *pIfStatement   = 0;
               Expression *pElseStatement = 0;
               bool        fResult        = false;

               pszParameter++;

               fResult = evaluateExpression (&pszParameter, pSymbolTable);

               if ('(' != *pszParameter)
               {
                  throw new ExpressionException (TYPE_I_NEEDS_PARENTHESIS,
                                                 __FILE__,
                                                 __LINE__);
               }

               pIfStatement = convert (&pszParameter, false, false, pSymbolTable);

               if (statementIs (pszParameter, STATEMENT_ELSE))
               {
                  pElseStatement = convert (&pszParameter, false, true, pSymbolTable);
               }

               if (!pIfStatement)
               {
                  throw new ExpressionException (TYPE_I_IF_REQUIRED,
                                                 __FILE__,
                                                 __LINE__);
               }

               if (fResult)
               {
                  pResult = pIfStatement;

                  delete pElseStatement;
                  pElseStatement = 0;
               }
               else
               {
                  if (pElseStatement)
                  {
                     pResult = pElseStatement;
                  }

                  delete pIfStatement;
                  pIfStatement = 0;
               }

               if (!fRootLevel)
               {
                  fContinue = false;
               }
               break;
            }

            case 'E':
            {
               if (!fFromIf)
               {
                  pszParameter--;
                  fContinue = false;
                  break;
               }

               pszParameter++;

               pNew = convert (&pszParameter, false, false, pSymbolTable);
               break;
            }

            default:
            {
               if (vfDebugOutput) std::cerr << "Error: % \"" << *pszParameter << "\" is not understood!" << std::endl;

               throw new ExpressionException (TYPE_UNKNOWN_PERCENT_OPERATOR,
                                                 __FILE__,
                                                 __LINE__);
               break;
            }
            }
            break;
         }

         case '+':
         {
            pszParameter++;

            pNew = convert (&pszParameter, false, false, pSymbolTable);

            pResult->add (pNew);

            delete pNew;
            continue;
         }

         case '-':
         {
            pszParameter++;

            pNew = convert (&pszParameter, false, false, pSymbolTable);

            pResult->subtract (pNew);

            delete pNew;
            continue;
         }

         case '*':
         {
            opFound = OPERATOR_TIMES;
            pszParameter++;
            continue;
         }

         case '/':
         {
            opFound = OPERATOR_DIVIDE;
            pszParameter++;
            continue;
         }

         case '=':
         case '!':
         case '<':
         case '>':
         {
            if (OPERATOR_NONE != opFound)
            {
               throw new ExpressionException (TYPE_MISSING_COMPARISON_EXPRESSION,
                                                 __FILE__,
                                                 __LINE__);
            }

            switch (*pszParameter)
            {
            case '=':
            {
               opFound = OPERATOR_EQUALS;
               break;
            }

            case '!':
            {
               pszParameter++;
               if ('=' == *pszParameter)
               {
                  opFound = OPERATOR_NOT_EQUALS;
               }
               else
               {
                  throw new ExpressionException (TYPE_NOT_NOT_EQUALS,
                                                 __FILE__,
                                                 __LINE__);
               }
               break;
            }

            case '<':
            {
               if ('=' == pszParameter[1])
               {
                  pszParameter++;
                  opFound = OPERATOR_LESSTHAN_EQUALS;
               }
               else
               {
                  opFound = OPERATOR_LESSTHAN;
               }
               break;
            }

            case '>':
            {
               if ('=' == pszParameter[1])
               {
                  pszParameter++;
                  opFound = OPERATOR_GREATERTHAN_EQUALS;
               }
               else
               {
                  opFound = OPERATOR_GREATERTHAN;
               }
               break;
            }
            }

            pszParameter++;
            continue;
         }

         case '(':
         {
            pszParameter++;

            if (fFirstExpression)
            {
               fFirstExpression = false;
            }
            else
            {
               pNew = convert (&pszParameter, false, false, pSymbolTable);
            }
            break;
         }

         case ')':
         {
            pszParameter++;

            if (!fRootLevel)
            {
               fContinue = false;
            }
            break;
         }
         }
      }
      catch (ExpressionException *pException)
      {
         if (  pException->isType (TYPE_VARIABLE_NOT_FOUND)
            || pException->isType (TYPE_INVALID_NUMBER)
            || pException->isType (TYPE_INVALID_DOUBLE)
            || pException->isType (TYPE_INVALID_BOOLEAN)
            || pException->isType (TYPE_A_ON_EXPRESSION)
            )
         {
            if (vfDebugOutput) std::cerr << "Caught " << *pException << std::endl;
            if (vfDebugOutput) std::cerr << "pszStartExpression = " << pszStartExpression << std::endl;
            if (vfDebugOutput) std::cerr << "pszParameter = " << pszParameter << std::endl;

            pNew = new Expression (pszStartExpression, pszParameter, true);

            delete pException;
         }
         else
         {
            throw;
         }
      }

      pResult = merge (pResult, pNew, opFound);
      pNew    = 0;
      opFound = OPERATOR_NONE;
   }

done:
   *ppszParameter = pszParameter;

   if (vfDebugOutput) std::cerr << "convert returning \"" << *pResult << "\"" << std::endl;

   return pResult;
}
