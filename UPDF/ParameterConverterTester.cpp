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
#include "Expression.hpp"
#include "ParameterConverter.hpp"

#include <iostream>

bool
isPrintable (char *pszString, int cbString)
{
   while (cbString > 0)
   {
      if (  *pszString
         && !isprint (*pszString)
         )
      {
         return false;
      }

      pszString++;
      cbString--;
   }

   return true;
}

void
outputString (char *pszString, int cbString)
{
   if (isPrintable (pszString, cbString))
   {
      std::cout << "\"" << pszString << "\"";
   }
   else
   {
      std::cout << std::hex;
      while (cbString > 0)
      {
         std::cout << "0x";
         std::cout.fill ('0');
         std::cout.width (2);
         std::cout << (*(unsigned short int *)pszString & 0xFF); // @TBD - weird
         pszString++;
         cbString--;
         if (cbString > 0)
         {
            std::cout << " ";
         }
      }
      std::cout << std::dec;
   }
}

int
main (int argc, char *argv[])
{
   typedef struct _TestResult {
      char *pszTest;
      char *pszResult;
      int   cbResult;
   } TESTRESULT, *PTESTRESULT;
   Expression *pResult        = 0;
   TESTRESULT  aStatements[]  = {
      {
         "((H)(e)(l)(l)(o) %H1(32) ((W))((o))((r))((l))((d)) (((%H1(33)))))",
         "Hello World!",
         sizeof ("Hello World!")
      }
      ,
      {
         "%K(GraphicMode)",
         "1",
         sizeof ("1")
      }
      ,
      {
         "%A72(123456789)",
         "000000000000000000000000000000000000000000000000000000000000000123456789",
         sizeof ("000000000000000000000000000000000000000000000000000000000000000123456789")
      }
      ,
      {
         "%A4(123456789)",
         "1234",
         sizeof ("1234")
      }
      ,
      {
         "%H0 (1176348909019551794188001)",
         "\xF9\x1A\x00\x8C\x1D\xA5\x91\x83\xCE\xE1",
         sizeof ("\xF9\x1A\x00\x8C\x1D\xA5\x91\x83\xCE\xE1") - 1
      }
      ,
      {
         "%H4(1024)",
         "\x00\x00\x04\x00",
         sizeof ("\x00\x00\x04\x00") - 1
      }
      ,
      {
         "%L5(1024)",
         "\x00\x00\x00\x00\x04",
         sizeof ("\x00\x00\x00\x00\x04") - 1
      }
      ,
      {
         "%F3(5)",
         "5.000"
      }
      ,
      {
         "%F3(5.12345)",
         "5.123"
      }
      ,
      {
         "%F10(1/3)",
         "0.3333330000",
         sizeof ("0.3333330000")
      }
      ,
      {
         "(4 < 9)(1 <= 7)(23 > 7)(70 >= 0)",
         "1111",
         sizeof ("1111")
      }
      ,
      {
         "a < b",
         "1",
         sizeof ("1")
      }
      ,
      {
         "a <= b",
         "1",
         sizeof ("1")
      }
      ,
      {
         "a > b",
         "0",
         sizeof ("0")
      }
      ,
      {
         "a >= b",
         "0",
         sizeof ("0")
      }
      ,
      {
         "a = b",
         "0",
         sizeof ("0")
      }
      ,
      {
         "a != b",
         "1",
         sizeof ("1")
      }
      ,
      {
         "9 + 467",
         "476",
         sizeof ("476")
      }
      ,
      {
         "8 + 0.001",
         "8.001",
         sizeof ("8.001")
      }
      ,
      {
         "8.999 + 0.1",
         "9.099",
         sizeof ("9.099")
      }
      ,
      {
         "1+2+3",
         "6"
      }
      ,
      {
         "10000 - 1",
         "9999",
         sizeof ("9999")
      }
      ,
      {
         "1 - 10000",
         "-9999",
         sizeof ("-9999")
      }
      ,
      {
         "987654321 - 123456789",
         "864197532",
         sizeof ("864197532")
      }
      ,
      {
         "99 - 99.001",
         "-0.001",
         sizeof ("-0.001")
      }
      ,
      {
         "1 / 3",
         "0.333333",
         sizeof ("0.333333")
      }
      ,
      {
         "1 / 2 * 2",
         "1",
         sizeof ("1")
      }
      ,
      {
         "1 / 3 * 3",
         "1",
         sizeof ("1")
      }
      ,
      {
         "1+2*3",
         "7"
      }
      ,
      {
         "%I(%K(GraphicMode))(true)%E(false)",
         "true",
         sizeof ("true")
      }
      ,
      {
         "%I(0)(true)%E(false)",
         "false",
         sizeof ("false")
      }
      ,
      {
         "%I(0)(discard)(this)",
         "this",
         sizeof ("this")
      }
      ,
      {
         "%I(%K(GraphicMode)=1)(SD2,0,3,%F2(%K(FONTWIDTH)),4,%F2(%K(FONTHEIGHT)),5,0,6,0,7,4099SS)%E(%H1(27)%H1(40)s%F2(%K(FONTHEIGHT)) hpsb4099T)",
         "SD2,0,3,42.00,4,12.00,5,0,6,0,7,4099SS",
         sizeof ("SD2,0,3,42.00,4,12.00,5,0,6,0,7,4099SS")
      }
      ,
      {
         "%F2(%K(Bob))%A0(%K(Lives))",
         "%F2(%K(Bob))%A0(%K(Lives))",
         sizeof ("%F2(%K(Bob))%A0(%K(Lives))")
      }
   };
   SYMBOLTABLE symbolTable;

   symbolTable[std::string ("GraphicMode")] = new Expression ("1", true);
   symbolTable[std::string ("FONTWIDTH")]   = new Expression ("42", true);
   symbolTable[std::string ("FONTHEIGHT")]  = new Expression ("12", true);

   for (int i = 0; i < (int)dimof (aStatements); i++)
   {
      pResult = 0;

      try
      {
         pResult = convert (&aStatements[i].pszTest, true, false, &symbolTable);

         if (!pResult)
         {
            std::cout << "Error!  convert returns null!" << std::endl;
            continue;
         }

         switch (pResult->getType ())
         {
         case TYPE_STRING:
         {
            if (0 == strcmp (pResult->getString (), aStatements[i].pszResult))
            {
               std::cout << "Success ";
            }
            else
            {
               std::cout << "Error!  ";
            }
            break;
         }

         case TYPE_HEX:
         {
            if (0 == memcmp (pResult->getBinaryData (), aStatements[i].pszResult, pResult->getLength ()))
            {
               std::cout << "Success ";
            }
            else
            {
               std::cout << "Error!  ";
            }
            break;
         }

         case TYPE_FREED:
         {
            std::cout << "Error!  ";
            break;
         }
         }

         std::cout << "Comparing " << *pResult << " with ";
         outputString (aStatements[i].pszResult, aStatements[i].cbResult);
         std::cout << std::endl;
      }
      catch (ExpressionException *pException)
      {
         std::cout << "Caught " << *pException << " for " << aStatements[i].pszTest << std::endl;

         delete pException;
      }

      delete pResult;
   }

   for ( SYMBOLTABLE::iterator next = symbolTable.begin () ;
         next != symbolTable.end () ;
         next++ )
   {
      delete (*next).second;
   }

   return 0;
}

//void
//converBase10ToBase16 (char *pszNumber)
//{
//#define ADD_DIGIT(n,s) n = n * 10 + *s - '0'
//
//   char  achTemp[64];
//   char *pszTemp      = achTemp;
//   char *pszInTemp    = 0;
//   char *pszOutTemp   = 0;
//   char *pszOutNumber = pszNumber;
//   int   iCurrent     = 0;
//
//   strcpy (pszTemp, pszNumber);
//
//   do
//   {
//      pszInTemp  = pszTemp;
//      pszOutTemp = pszTemp;
//      iCurrent   = 0;
//
////////std::cout << pszTemp << std::endl;
//
//      do
//      {
//         ADD_DIGIT (iCurrent, pszInTemp);
//         pszInTemp++;
//
//      } while (  *pszInTemp
//              && iCurrent < 16
//              );
//
//      while (iCurrent >= 16)
//      {
///////////std::cout << "iCurrent              = " << iCurrent << std::endl;
///////////std::cout << "(iCurrent >> 4) + '0' = " << (char)((iCurrent >> 4) + '0') << std::endl;
//
//         *pszOutTemp++ = (iCurrent >> 4) + '0';
//         iCurrent  = iCurrent % 16;
//
///////////std::cout << "iCurrent              = " << iCurrent << std::endl;
///////////std::cout << "------------------------" << std::endl;
//
//         while (  *pszInTemp
//               && iCurrent < 16
//               )
//         {
//            ADD_DIGIT (iCurrent, pszInTemp);
//
//            if (  *pszInTemp
//               && iCurrent < 16
//               )
//            {
//               *pszOutTemp++ = '0';
//            }
//
//            pszInTemp++;
//         }
//      }
//
//      *pszOutTemp++ = '\0';
//
////////std::cout << pszTemp << ", " << iCurrent << std::endl;
//
//      if (iCurrent < 10)
//      {
//         *pszOutNumber++ = '0' + iCurrent;
//      }
//      else
//      {
//         *pszOutNumber++ = 'A' + iCurrent - 10;
//      }
//
//   } while (*pszTemp);
//
//   *pszOutNumber++ = '\0';
//
//   // Swap the digits
//   pszInTemp  = pszNumber;
//   pszOutTemp = pszNumber + strlen (pszNumber) - 1;
//
//   while (pszInTemp < pszOutTemp)
//   {
//      char chTemp = *pszInTemp;
//
//      *pszInTemp++  = *pszOutTemp;
//      *pszOutTemp-- = chTemp;
//   }
//
//   std::cout << pszNumber << std::endl;
//}
