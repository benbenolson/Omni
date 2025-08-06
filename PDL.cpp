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
#include "PDL.hpp"
#include "defines.hpp"
#include "DebugOutput.hpp"

#include <string.h>

PDL::
PDL (int iPDLLevel,
     int iPDLSubLevel,
     int iMajorRevisionLevel,
     int iMinorRevisionLevel)
{
   iPDLLevel_d           = iPDLLevel;
   iPDLSubLevel_d        = iPDLSubLevel;
   iMajorRevisionLevel_d = iMajorRevisionLevel;
   iMinorRevisionLevel_d = iMinorRevisionLevel;
}

int PDL::
getPDLLevel ()
{
   return iPDLLevel_d;
}

int PDL::
getPDLSubLevel ()
{
   return iPDLSubLevel_d;
}

int PDL::
getPDLMajorRevisionLevel ()
{
   return iMajorRevisionLevel_d;
}

int PDL::
getPDLMinorRevisionLevel ()
{
   return iMinorRevisionLevel_d;
}

typedef struct _ReservedMap {
   const char *pszName;
   int         iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "LEVEL_ASCII_JISASCII",    4 },
   { "LEVEL_ASCII_PROPRINTER",  2 },
   { "LEVEL_ASCII_QUITWRITER",  3 },
   { "LEVEL_ASCII_TEXT",        1 },
   { "LEVEL_DESKJET",           1 },
   { "LEVEL_DESKJETJ",          2 },
   { "LEVEL_ESC",               1 },
   { "LEVEL_ESCP",              2 },
   { "LEVEL_ESCP_2",            3 },
   { "LEVEL_ESCP_2J",           4 },
   { "LEVEL_HPGL1",             1 },
   { "LEVEL_HPGL2",             2 },
   { "LEVEL_HPGL2_MC",          5 },
   { "LEVEL_HPGL2_PCLRTL",      4 },
   { "LEVEL_HPGL2_RTL",         3 },
   { "LEVEL_PCL2",              1 },
   { "LEVEL_PCL3",              2 },
   { "LEVEL_PCL3C",             3 },
   { "LEVEL_PCL4",              4 },
   { "LEVEL_PCL5",              5 },
   { "LEVEL_PCL5C",             6 },
   { "LEVEL_PCL5E",             8 },
   { "LEVEL_PCL6",              7 },
   { "LEVEL_PS_LEVEL1",         1 },
   { "LEVEL_PS_LEVEL2",         2 },
   { "PDL_ALPS",               54 },
   { "PDL_ART",                48 },
   { "PDL_Automatic",          37 },
   { "PDL_CCITT",              26 },
   { "PDL_CPAP",               28 },
   { "PDL_CaPSL",              43 },
   { "PDL_CodeV",              22 },
   { "PDL_DDIF",               11 },
   { "PDL_DOC",                32 },
   { "PDL_DONTCARE",            0 },
   { "PDL_DSCDSE",             23 },
   { "PDL_DecPPL",             29 },
   { "PDL_Deskjet",            56 },
   { "PDL_Diagnostic",         41 },
   { "PDL_EXCL",               44 },
   { "PDL_Epson",              10 },
   { "PDL_EscapeP",             9 },
   { "PDL_HPGL",                4 },
   { "PDL_IDP",                52 },
   { "PDL_IGP",                21 },
   { "PDL_IPDS",                7 },
   { "PDL_ISO6429",            13 },
   { "PDL_Interpress",         12 },
   { "PDL_LCDS",               45 },
   { "PDL_LIPS",               39 },
   { "PDL_LN03",               25 },
   { "PDL_LineData",           14 },
   { "PDL_LinePrinter",        51 },
   { "PDL_MODCA",              15 },
   { "PDL_NEC201PL",           36 },
   { "PDL_NPAP",               31 },
   { "PDL_NPDL",               35 },
   { "PDL_Olivetti",           55 },
   { "PDL_PCL",                 3 },
   { "PDL_PCLXL",              47 },
   { "PDL_PDS",                20 },
   { "PDL_PJL",                 5 },
   { "PDL_PPDS",                8 },
   { "PDL_PS",                  6 },
   { "PDL_PSPrinter",          42 },
   { "PDL_Pages",              38 },
   { "PDL_Paintjet",           57 },
   { "PDL_PassThru",           59 },
   { "PDL_Pinwriter",          34 },
   { "PDL_Prescribe",          50 },
   { "PDL_QUIC",               27 },
   { "PDL_REGIS",              16 },
   { "PDL_RPDL",               60 },
   { "PDL_SCS",                17 },
   { "PDL_SPDL",               18 },
   { "PDL_Seiko",              58 },
   { "PDL_SimpleText",         30 },
   { "PDL_TEK4014",            19 },
   { "PDL_TIFF",               40 },
   { "PDL_TIPSI",              49 },
   { "PDL_WPS",                24 },
   { "PDL_XES",                46 },
   { "PDL_XJCL",               53 },
   { "PDL_imPress",            33 },
   { "PDL_other",               1 },
};

bool PDL::
isReservedKeyword (const char *pszId)
{
   int   iLow          = 0;
   int   iMid          = (int)dimof (vaReservedKeywords) / 2;
   int   iHigh         = (int)dimof (vaReservedKeywords) - 1;
   int   iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId,
                        vaReservedKeywords[iMid].pszName);

      if (0 == iResult)
      {
         return true;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return false;
}

int PDL::
getReservedValue (const char *pszId)
{
   int   iLow          = 0;
   int   iMid          = (int)dimof (vaReservedKeywords) / 2;
   int   iHigh         = (int)dimof (vaReservedKeywords) - 1;
   int   iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId,
                        vaReservedKeywords[iMid].pszName);

      if (0 == iResult)
      {
         return vaReservedKeywords[iMid].iValue;
      }
      else if (0 > iResult)
      {
         // str1 < str2
         iHigh = iMid - 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
      else // (0 < iResult)
      {
         // str1 > str2
         iLow  = iMid + 1;
         iMid  = iLow + (iHigh - iLow) / 2;
      }
   }

   return 0;
}

#ifndef RETAIL

void PDL::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string PDL::
toString (std::ostringstream& oss)
{
   oss << "{PDL: "
       << "iPDLLevel_d = "
       << iPDLLevel_d
       << ", iPDLSubLevel_d = "
       << iPDLSubLevel_d
       << ", iMajorRevisionLevel_d = "
       << iMajorRevisionLevel_d
       << ", iMinorRevisionLevel_d = "
       << iMinorRevisionLevel_d
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const PDL& const_self)
{
   PDL&               self = const_cast<PDL&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}
