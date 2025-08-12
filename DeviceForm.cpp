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
#include "DeviceForm.hpp"
#include "BinaryData.hpp"
#include "StringResource.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <cstring>
#include <iomanip>

static PSZCRO vapszJobPropertyKeys[] = {
   "Form"
};

#define JOBPROP_FORM         vapszJobPropertyKeys[0]

typedef enum _ePrefix {
   PREFIX_NONE = -1,
   PREFIX_ISO,
   PREFIX_JIS,
   PREFIX_JPN,
   PREFIX_NA,
   PREFIX_OM,
   PREFIX_PRC,
   PREFIX_ROC
} EPREFIX;

typedef struct _PrefixEntry {
   PSZCRO  pszName;
   int     cbName;
   EPREFIX ePrefix;
} PREFIXENTRY, *PPREFIXENTRY;

static PREFIXENTRY vaPrefixes[] = {
   { "iso_", 4, PREFIX_ISO },
   { "jis_", 4, PREFIX_JIS },
   { "jpn_", 4, PREFIX_JPN },
   { "na_",  3, PREFIX_NA  },
   { "om_",  3, PREFIX_OM  },
   { "prc_", 4, PREFIX_PRC },
   { "roc_", 4, PREFIX_ROC }
};

typedef enum _eUnits {
   UNKNOWN,
   INCHES,
   MM
} EUNITS;

typedef enum _eType {
   NORMAL,
   ROLL,
   USER_DEFINED
} ETYPE;

typedef struct _FormMapping {
   PSZCRO  pszUPDFName;
   int     cx;                    // Form size is
   int     cy;                    // in thousands of a millimeter
   EPREFIX ePrefix;
   EUNITS  eExternalDisplayUnits;
   ETYPE   eType;
} FORMMAP, *PFORMMAP;

static FORMMAP vaFormMapping[] = {
   { "1",                      102000,   165000, PREFIX_PRC, MM,     NORMAL       },
   { "10",                     324000,   458000, PREFIX_PRC, MM,     NORMAL       },
   { "10x11",                  254000,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "11x12",                  279400,   304800, PREFIX_NA,  INCHES, NORMAL       },
   { "11x15",                  279400,   381000, PREFIX_NA,  INCHES, NORMAL       },
   { "12x19",                  304800,   482600, PREFIX_NA,  INCHES, NORMAL       },
   { "15x11",                  381000,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "16k",                    196850,   273050, PREFIX_ROC, MM,     NORMAL       },
   { "16k",                    146000,   215000, PREFIX_PRC, MM,     NORMAL       },
   { "170x210",                170000,   210000, PREFIX_NA,  MM,     NORMAL       },
   { "180x210",                182000,   210000, PREFIX_NA,  MM,     NORMAL       },
   { "2",                      102000,   176000, PREFIX_PRC, MM,     NORMAL       },
   { "2a0",                   1189000,  1682000, PREFIX_ISO, MM,     NORMAL       },
   { "3",                      125000,   176000, PREFIX_PRC, MM,     NORMAL       },
   { "32k",                     97000,   151000, PREFIX_PRC, MM,     NORMAL       },
   { "3x5-card",                76200,   127000, PREFIX_NA,  INCHES, NORMAL       },
   { "4",                      110000,   208000, PREFIX_PRC, MM,     NORMAL       },
   { "4a0",                   1682000,  2378000, PREFIX_ISO, MM,     NORMAL       },
   { "4x6-card",               101600,   152400, PREFIX_NA,  INCHES, NORMAL       },
   { "5",                      110000,   220000, PREFIX_PRC, MM,     NORMAL       },
   { "5x7",                    127000,   177800, PREFIX_NA,  INCHES, NORMAL       },
   { "5x8-card",               127000,   203200, PREFIX_NA,  INCHES, NORMAL       },
   { "6",                      120000,   320000, PREFIX_PRC, MM,     NORMAL       },
   { "7",                      160000,   230000, PREFIX_PRC, MM,     NORMAL       },
   { "8",                      120000,   309000, PREFIX_PRC, MM,     NORMAL       },
   { "8.25x13",                209550,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "8.5x12.4",               215900,   314960, PREFIX_NA,  INCHES, NORMAL       },
   { "8k",                     273050,   393700, PREFIX_ROC, MM,     NORMAL       },
   { "8x10-card",              203200,   254000, PREFIX_NA,  INCHES, NORMAL       },
   { "8x10.5",                 203200,   266700, PREFIX_NA,  INCHES, NORMAL       },
   { "9",                      229000,   324000, PREFIX_PRC, MM,     NORMAL       },
   { "a0",                     841000,  1189000, PREFIX_ISO, MM,     NORMAL       },
   { "a1",                     594000,   841000, PREFIX_ISO, MM,     NORMAL       },
   { "a10",                     26000,    37000, PREFIX_ISO, MM,     NORMAL       },
   { "a2",                     111125,   146050, PREFIX_NA,  INCHES, NORMAL       },
   { "a2",                     420000,   594000, PREFIX_ISO, MM,     NORMAL       },
   { "a3",                     297000,   420000, PREFIX_ISO, MM,     NORMAL       },
   { "a3-extra",               322000,   445000, PREFIX_ISO, MM,     NORMAL       },
   { "a3-wide",                330200,   482600, PREFIX_ISO, MM,     NORMAL       },
   { "a4",                     210000,   297000, PREFIX_ISO, MM,     NORMAL       },
   { "a4-extra",               235500,   322300, PREFIX_ISO, MM,     NORMAL       },
   { "a4-tab",                 225000,   297000, PREFIX_ISO, MM,     NORMAL       },
   { "a4-wide",                223500,   355600, PREFIX_ISO, MM,     NORMAL       },
   { "a5",                     148000,   210000, PREFIX_ISO, MM,     NORMAL       },
   { "a5-extra",               174000,   235000, PREFIX_ISO, MM,     NORMAL       },
   { "a6",                     105000,   148000, PREFIX_ISO, MM,     NORMAL       },
   { "a6-card",                104980,   148090, PREFIX_NA,  INCHES, NORMAL       },
   { "a7",                      74000,   105000, PREFIX_ISO, MM,     NORMAL       },
   { "a8",                      52000,    74000, PREFIX_ISO, MM,     NORMAL       },
   { "a9",                      37000,    52000, PREFIX_ISO, MM,     NORMAL       },
   { "arch-a",                 228600,   304800, PREFIX_NA,  INCHES, NORMAL       },
   { "arch-b",                 304800,   457200, PREFIX_NA,  INCHES, NORMAL       },
   { "arch-c",                 457200,   609600, PREFIX_NA,  INCHES, NORMAL       },
   { "arch-d",                 609600,   914400, PREFIX_NA,  INCHES, NORMAL       },
   { "arch-e",                 914400,  1219200, PREFIX_NA,  INCHES, NORMAL       },
   { "b-plus",                 304800,   486918, PREFIX_NA,  INCHES, NORMAL       },
   { "b0",                    1000000,  1414000, PREFIX_ISO, MM,     NORMAL       },
   { "b0",                    1030000,  1456000, PREFIX_JIS, MM,     NORMAL       },
   { "b1",                     707000,  1000000, PREFIX_ISO, MM,     NORMAL       },
   { "b1",                     728000,  1030000, PREFIX_JIS, MM,     NORMAL       },
   { "b10",                     31000,    44000, PREFIX_ISO, MM,     NORMAL       },
   { "b10",                     32000,    45000, PREFIX_JIS, MM,     NORMAL       },
   { "b2",                     500000,   707000, PREFIX_ISO, MM,     NORMAL       },
   { "b2",                     515000,   728000, PREFIX_JIS, MM,     NORMAL       },
   { "b3",                     353000,   500000, PREFIX_ISO, MM,     NORMAL       },
   { "b3",                     364000,   515000, PREFIX_JIS, MM,     NORMAL       },
   { "b4",                     250000,   353000, PREFIX_ISO, MM,     NORMAL       },
   { "b4",                     257000,   364000, PREFIX_JIS, MM,     NORMAL       },
   { "b5",                     176000,   250000, PREFIX_ISO, MM,     NORMAL       },
   { "b5",                     182000,   257000, PREFIX_JIS, MM,     NORMAL       },
   { "b5-extra",               201000,   276000, PREFIX_ISO, MM,     NORMAL       },
   { "b6",                     125000,   176000, PREFIX_ISO, MM,     NORMAL       },
   { "b6",                     128000,   182000, PREFIX_JIS, MM,     NORMAL       },
   { "b6c4",                   125000,   324000, PREFIX_ISO, MM,     NORMAL       },
   { "b7",                      88000,   125000, PREFIX_ISO, MM,     NORMAL       },
   { "b7",                      91000,   128000, PREFIX_JIS, MM,     NORMAL       },
   { "b8",                      62000,    88000, PREFIX_ISO, MM,     NORMAL       },
   { "b8",                      64000,    91000, PREFIX_JIS, MM,     NORMAL       },
   { "b9",                      44000,    62000, PREFIX_ISO, MM,     NORMAL       },
   { "b9",                      45000,    64000, PREFIX_JIS, MM,     NORMAL       },
   { "c0",                     917000,  1297000, PREFIX_ISO, MM,     NORMAL       },
   { "c1",                     648000,   917000, PREFIX_ISO, MM,     NORMAL       },
   { "c10",                    104700,   241300, PREFIX_NA,  INCHES, NORMAL       },
   { "c10",                     28000,    40000, PREFIX_ISO, MM,     NORMAL       },
   { "c2",                     458000,   648000, PREFIX_ISO, MM,     NORMAL       },
   { "c3",                     324000,   458000, PREFIX_ISO, MM,     NORMAL       },
   { "c4",                     229000,   324000, PREFIX_ISO, MM,     NORMAL       },
   { "c5",                     162000,   229000, PREFIX_NA,  INCHES, NORMAL       },
   { "c5",                     162000,   229000, PREFIX_ISO, MM,     NORMAL       },
   { "c6",                     114000,   162000, PREFIX_NA,  INCHES, NORMAL       },
   { "c6",                     114000,   162000, PREFIX_ISO, MM,     NORMAL       },
   { "c6c5",                   114000,   229000, PREFIX_ISO, MM,     NORMAL       },
   { "c7",                      98400,   190500, PREFIX_NA,  INCHES, NORMAL       },
   { "c7",                      81000,   114000, PREFIX_ISO, MM,     NORMAL       },
   { "c7c6",                    81000,   162000, PREFIX_ISO, MM,     NORMAL       },
   { "c8",                      57000,    81000, PREFIX_ISO, MM,     NORMAL       },
   { "c9",                      98400,   225400, PREFIX_NA,  INCHES, NORMAL       },
   { "c9",                      40000,    57000, PREFIX_ISO, MM,     NORMAL       },
   { "card-148",               148000,   105000, PREFIX_NA,  MM,     NORMAL       },
   { "chou2",                  111100,   146000, PREFIX_JPN, MM,     NORMAL       },
   { "chou3",                  120000,   235000, PREFIX_JPN, MM,     NORMAL       },
   { "chou4",                   90000,   205000, PREFIX_JPN, MM,     NORMAL       },
   { "d5",                     176000,   250000, PREFIX_NA,  INCHES, NORMAL       },
   { "dai-pa-kai",             275000,   395000, PREFIX_OM,  MM,     NORMAL       },
   { "disk-labels",             54000,    70000, PREFIX_NA,  INCHES, NORMAL       },
   { "dl",                     110000,   220000, PREFIX_NA,  INCHES, NORMAL       },
   { "e1",                     711200,  1016000, PREFIX_ISO, MM,     NORMAL       },
   { "edp",                    279400,   355600, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-10x13",         254000,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-10x14",         254000,   355600, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-10x15",         254000,   381000, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-132x220",       132000,   220000, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-6.5",           165100,    92070, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-6x9",           152400,   228600, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-7x9",           177800,   228600, PREFIX_NA,  INCHES, NORMAL       },
   { "envelope-9x11",          228600,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "eur-edp",                294000,   355600, PREFIX_NA,  INCHES, NORMAL       },
   { "euro-labels",             36000,    89000, PREFIX_NA,  INCHES, NORMAL       },
   { "exec",                   216000,   330000, PREFIX_JIS, MM,     NORMAL       },
   { "executive",              184150,   266700, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-1",              368300,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-2",              304800,   215900, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-3",              241300,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-4",              215900,   304800, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-5",              215900,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-eur",            215900,   304800, PREFIX_NA,  INCHES, NORMAL       },
   { "fanfold-us",             377825,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "folio",                  215900,   330000, PREFIX_OM,  MM,     NORMAL       },
   { "folio-sp",               215000,   315000, PREFIX_OM,  MM,     NORMAL       },
   { "foolscap",               203200,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "foolscap-wide",          215900,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "german-12x250-fanfold",  304800,   240000, PREFIX_NA,  MM,     NORMAL       },
   { "german-legal-fanfold",   215900,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "govt-legal",             203200,   330200, PREFIX_NA,  INCHES, NORMAL       },
   { "govt-letter",            203200,   254000, PREFIX_NA,  INCHES, NORMAL       },
   { "hagaki",                 100000,   148000, PREFIX_JPN, MM,     NORMAL       },
   { "half-letter",            139690,   215970, PREFIX_NA,  INCHES, NORMAL       },
   { "index-4x6-ext",          152400,   203200, PREFIX_NA,  INCHES, NORMAL       },
   { "invite",                 220000,   220000, PREFIX_OM,  MM,     NORMAL       },
   { "italian",                100000,   230000, PREFIX_OM,  MM,     NORMAL       },
   { "juuro-ku-kai",           198000,   275000, PREFIX_OM,  MM,     NORMAL       },
   { "kahu",                   240000,   322100, PREFIX_JPN, MM,     NORMAL       },
   { "kaku2",                  240000,   332000, PREFIX_JPN, MM,     NORMAL       },
   { "ledger",                 431800,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "legal",                  215900,   355600, PREFIX_NA,  INCHES, NORMAL       },
   { "legal-extra",            241300,   381000, PREFIX_NA,  INCHES, NORMAL       },
   { "letter",                 215900,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "letter-extra",           241300,   304800, PREFIX_NA,  INCHES, NORMAL       },
   { "letter-plus",            215900,   322326, PREFIX_NA,  INCHES, NORMAL       },
   { "letter-wide",            228400,   337800, PREFIX_NA,  INCHES, NORMAL       },
   { "monarch",                 98425,   190500, PREFIX_NA,  INCHES, NORMAL       },
   { "number-10",              104775,   241300, PREFIX_NA,  INCHES, NORMAL       },
   { "number-11",              114300,   263525, PREFIX_NA,  INCHES, NORMAL       },
   { "number-12",              120650,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "number-14",              127000,   292100, PREFIX_NA,  INCHES, NORMAL       },
   { "number-9",                98425,   225425, PREFIX_NA,  INCHES, NORMAL       },
   { "oufuku",                 148000,   200000, PREFIX_JPN, MM,     NORMAL       },
   { "pa-kai",                 267000,   389000, PREFIX_OM,  MM,     NORMAL       },
   { "panoramic",              210000,   594000, PREFIX_NA,  MM,     NORMAL       },
   { "personal",                92075,   165100, PREFIX_NA,  INCHES, NORMAL       },
   { "photo-100x150",          100000,   150000, PREFIX_NA,  MM,     NORMAL       },
   { "photo-200x300",          200000,   300000, PREFIX_NA,  MM,     NORMAL       },
   { "photo-4x6",              101600,   152400, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-a",         215900,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-b",         279400,   431800, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-c",         431800,   558800, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-d",         558800,   863600, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-e",         863690,  1117600, PREFIX_NA,  INCHES, NORMAL       },
   { "plotter-size-f",        1117600,  1727200, PREFIX_NA,  INCHES, NORMAL       },
   { "postcard",                99836,   147030, PREFIX_NA,  INCHES, NORMAL       },
   { "postfix",                114000,   229000, PREFIX_OM,  MM,     NORMAL       },
   { "quarto",                 215900,   275082, PREFIX_NA,  INCHES, NORMAL       },
   { "ra0",                    860000,  1220000, PREFIX_ISO, MM,     NORMAL       },
   { "ra1",                    610000,   860000, PREFIX_ISO, MM,     NORMAL       },
   { "ra2",                    430000,   610000, PREFIX_ISO, MM,     NORMAL       },
   { "roll-69.5",               69500,        0, PREFIX_NA,  MM,     ROLL         },
   { "roll-76.2",               76200,        0, PREFIX_NA,  MM,     ROLL         },
   { "shipping-labels",         54000,   101000, PREFIX_NA,  INCHES, NORMAL       },
   { "sra0",                   900000,  1280000, PREFIX_ISO, MM,     NORMAL       },
   { "sra1",                   640000,   900000, PREFIX_ISO, MM,     NORMAL       },
   { "sra2",                   450000,   640000, PREFIX_ISO, MM,     NORMAL       },
   { "standard-labels-clear",   28000,    89000, PREFIX_NA,  INCHES, NORMAL       },
   { "standard-labels-white",   28000,    89000, PREFIX_NA,  INCHES, NORMAL       },
   { "statement",              139700,   215900, PREFIX_NA,  INCHES, NORMAL       },
   { "super-a",                227076,   355600, PREFIX_NA,  MM,     NORMAL       },
   { "super-a3-b",             328920,   483020, PREFIX_NA,  MM,     NORMAL       },
   { "super-b",                330200,   482600, PREFIX_NA,  MM,     NORMAL       },
   { "tabloid",                279400,   431800, PREFIX_NA,  INCHES, NORMAL       },
   { "test",                    69500,    76200, PREFIX_NA,  INCHES, NORMAL       },
   { "universal",              297040,   431800, PREFIX_NA,  INCHES, NORMAL       },
   { "user-defined",                0,        0, PREFIX_NA,  INCHES, USER_DEFINED },
   { "wide-format",            345000,   279400, PREFIX_NA,  INCHES, NORMAL       },
   { "you4",                   105000,   235000, PREFIX_JPN, MM,     NORMAL       }
};

/* Function prototypes...
*/
int                findEntry           (PSZCRO  pszFormName,
                                        EPREFIX ePrefix);
PSZCRO             writeFormName       (int     indexForm,
                                        bool    fInJPFormat,
                                        bool    fInLongFormat);
PSZCRO             writeFormName       (int     indexForm,
                                        bool    fInJPFormat,
                                        bool    fInLongFormat,
                                        int     iX,
                                        int     iY);
bool               getFormSize         (PSZ     pszRest,
                                        int    *piFormSizeX,
                                        int    *piFormSizeY);

DeviceForm::
DeviceForm (Device      *pDevice,
            PSZRO        pszJobProperties,
            int          iCapabilities,
            BinaryData  *data,
            HardCopyCap *hcInfo)
{
   pDevice_d       = pDevice;
   pszForm_d       = 0;
   indexForm_d     = -1;
   iCapabilities_d = iCapabilities;
   data_d          = data;
   hcInfo_d        = hcInfo;

   iXOverride_d    = 0;
   iYOverride_d    = 0;

   iCX_d           = 0;
   iCY_d           = 0;

   if (  pszJobProperties
      && *pszJobProperties
      )
   {
      int iFormSizeX = 0;
      int iFormSizeY = 0;

      if (getComponents (pszJobProperties,
                         &pszForm_d,
                         &indexForm_d,
                         &iFormSizeX,
                         &iFormSizeY))
      {
         if (ROLL == vaFormMapping[indexForm_d].eType)
         {
            iCX_d = vaFormMapping[indexForm_d].cx;
            iCY_d = iFormSizeY;
         }
         else
         {
            iCX_d = vaFormMapping[indexForm_d].cx;
            iCY_d = vaFormMapping[indexForm_d].cy;
         }
      }
   }

   if (hcInfo)
   {
      hcInfo->setOwner (this);
   }
}

DeviceForm::
~DeviceForm ()
{
   if (pszForm_d)
   {
      free ((void *)pszForm_d);
   }

   delete data_d;
   delete hcInfo_d;

   pDevice_d       = 0;
   indexForm_d     = -1;
   iCapabilities_d = 0;
   data_d          = 0;
   hcInfo_d        = 0;
   iXOverride_d    = 0;
   iYOverride_d    = 0;
   iCX_d           = 0;
   iCY_d           = 0;
}

bool DeviceForm::
isValid (PSZCRO pszJobProperties)
{
   return getComponents (pszJobProperties, 0, 0, 0, 0);
}

bool DeviceForm::
isEqual (PSZCRO pszJobProperties)
{
   int indexForm = -1;

   if (getComponents (pszJobProperties,
                      0,
                      &indexForm,
                      0,
                      0))
   {
      return indexForm == indexForm_d;
   }

   return false;
}

std::string * DeviceForm::
getCreateHash ()
{
   std::ostringstream oss;

   oss << "DFO1_"
       << indexForm_d;

   return new std::string (oss.str ());
}

DeviceForm * DeviceForm::
createWithHash (Device *pDevice,
                PSZCRO  pszCreateHash)
{
   DeviceForm *pFormRet  = 0;
   int         indexForm = -1;
   int         iX        = 0;
   int         iY        = 0;
   PSZRO       pszFormJP = 0;

   if (  !pszCreateHash
      || !*pszCreateHash
      )
   {
      return 0;
   }

   if (0 == strncmp (pszCreateHash, "DFO1_", 5))
   {
      if (1 == sscanf (pszCreateHash, "DFO1_%d_%d_%d", &indexForm, &iX, &iY))
      {
         pszFormJP = writeFormName (indexForm, true, true, iX, iY);

         if (pszFormJP)
         {
            pFormRet = create (pDevice, pszFormJP);
         }
      }
   }

   if (pszFormJP)
   {
      free ((void *)pszFormJP);
   }

   return pFormRet;
}

bool DeviceForm::
handlesKey (PSZCRO pszKey)
{
   if (  !pszKey
      || !*pszKey
      )
   {
      return false;
   }

   for (int i = 0; i < (int)dimof (vapszJobPropertyKeys); i++)
   {
      if (0 == strcmp (pszKey, vapszJobPropertyKeys[i]))
      {
         return true;
      }
   }

   return false;
}

std::string * DeviceForm::
getJobPropertyType (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_FORM))
   {
      if (pszForm_d)
      {
         std::ostringstream oss;

         oss << "string " << pszForm_d;

         return new std::string (oss.str ());
      }
   }

   return 0;
}

std::string * DeviceForm::
getJobProperty (PSZCRO pszKey)
{
   if (0 == strcmp (pszKey, JOBPROP_FORM))
   {
      if (pszForm_d)
      {
         return new std::string (pszForm_d);
      }
   }

   return 0;
}

std::string * DeviceForm::
translateKeyValue (PSZCRO pszKey,
                   PSZCRO pszValue)
{
   std::string *pRet = 0;

   if (0 == strcasecmp (JOBPROP_FORM, pszKey))
   {
      PSZRO pszXLateKey = 0;

      pszXLateKey = StringResource::getString (pDevice_d->getLanguageResource (),
                                               StringResource::STRINGGROUP_DEVICE_COMMON,
                                               StringResource::DEVICE_COMMON_FORM);
      if (pszXLateKey)
      {
         pRet = new std::string (pszXLateKey);
      }

      if (  pszValue
         && *pszValue
         && pRet
         )
      {
         PSZRO pszXLateValue = 0;

         pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                                    StringResource::STRINGGROUP_FORMS,
                                                    pszValue);

         if (pszXLateValue)
         {
            *pRet += "=";
            *pRet += pszXLateValue;
         }
      }
   }

   return pRet;
}

std::string * DeviceForm::
getAllTranslation ()
{
   std::ostringstream oss;
   PSZRO              pszXLateValue = 0;

   pszXLateValue = StringResource::getString (pDevice_d->getLanguageResource (),
                                              StringResource::STRINGGROUP_FORMS,
                                              pszForm_d);

   if (pszXLateValue)
   {
      oss << pszXLateValue;
   }

   return new std::string (oss.str ());
}

std::string * DeviceForm::
getJobProperties (bool fInDeviceSpecific)
{
   PSZRO pszFormID = 0;

   if (fInDeviceSpecific)
   {
      pszFormID = getDeviceID ();
   }

   if (!pszFormID)
   {
      pszFormID = pszForm_d;
   }

   if (pszFormID)
   {
      std::ostringstream oss;

      oss << JOBPROP_FORM << "=" << pszFormID;

      return new std::string (oss.str ());
   }

   return 0;
}

bool DeviceForm::
hasCapability (int iFlag)
{
   return 0 != (iFlag & iCapabilities_d);
}

int DeviceForm::
getCapabilities ()
{
   return iCapabilities_d;
}

BinaryData * DeviceForm::
getData ()
{
   return data_d;
}

HardCopyCap * DeviceForm::
getHardCopyCap ()
{
   return hcInfo_d;
}

PSZCRO DeviceForm::
getDeviceID ()
{
   return 0;
}

std::string * DeviceForm::
getForm ()
{
   if (pszForm_d)
   {
      return new std::string (pszForm_d);
   }

   return 0;
}

void DeviceForm::
associateWith (DeviceResolution *pResolution)
{
   hcInfo_d->associateWith (pResolution);
}

int DeviceForm::
getCx ()
{
   if (iXOverride_d)
   {
      return iXOverride_d;
   }

   return iCX_d;
}

int DeviceForm::
getCy ()
{
   if (iYOverride_d)
   {
      return iYOverride_d;
   }

   return iCY_d;
}

void DeviceForm::
setCx (int iXOverride)
{
   iXOverride_d = iXOverride;
}

void DeviceForm::
setCy (int iYOverride)
{
   iYOverride_d = iYOverride;
}

int DeviceForm::
getOverrideX ()
{
   return iXOverride_d;
}

int DeviceForm::
getOverrideY ()
{
   return iYOverride_d;
}

PSZCRO DeviceForm::
getLongFormName (PSZCRO pszLongName)
{
   PSZRO   pszStart = pszLongName;
   PSZRO   pszRet   = 0;
   EPREFIX ePrefix  = PREFIX_NONE;

   for (int i = 0; i < (int)dimof (vaPrefixes); i++)
   {
      if (0 == strncmp (pszLongName,
                        vaPrefixes[i].pszName,
                        vaPrefixes[i].cbName))
      {
         ePrefix   = vaPrefixes[i].ePrefix;
         pszStart += vaPrefixes[i].cbName;
         break;
      }
   }

   char *pszFormName = (char *)malloc (strlen (pszStart) + 1);

   if (pszFormName)
   {
      char *pszUnderscore = 0;
      int   indexForm     = -1;

      strcpy (pszFormName, pszStart);

      pszUnderscore = strchr (pszFormName, '_');

      if (pszUnderscore)
      {
         *pszUnderscore = '\0';
      }

      indexForm = findEntry (pszFormName, ePrefix);

      if (-1 != indexForm)
      {
         int iFormSizeX = 0;
         int iFormSizeY = 0;

         if (  ROLL == vaFormMapping[indexForm].eType
            && pszUnderscore
            && *(pszUnderscore + 1)
            && getFormSize (pszUnderscore + 1, &iFormSizeX, &iFormSizeY)
            )
         {
            pszRet = writeFormName (indexForm, false, true, iFormSizeX, iFormSizeY);
         }
         else
         {
            pszRet = writeFormName (indexForm, false, true);
         }
      }

      free ((void *)pszFormName);
   }

   return pszRet;
}

PSZCRO DeviceForm::
getShortFormName (PSZCRO pszLongName)
{
   PSZRO   pszStart = pszLongName;
   PSZRO   pszRet   = 0;
   EPREFIX ePrefix  = PREFIX_NONE;

   for (int i = 0; i < (int)dimof (vaPrefixes); i++)
   {
      if (0 == strncmp (pszLongName,
                        vaPrefixes[i].pszName,
                        vaPrefixes[i].cbName))
      {
         ePrefix   = vaPrefixes[i].ePrefix;
         pszStart += vaPrefixes[i].cbName;
         break;
      }
   }

   char *pszFormName = (char *)malloc (strlen (pszStart) + 1);

   if (pszFormName)
   {
      char *pszUnderscore = 0;
      int   indexForm     = -1;

      strcpy (pszFormName, pszStart);

      pszUnderscore = strchr (pszFormName, '_');

      if (pszUnderscore)
      {
         *pszUnderscore = '\0';
      }

      indexForm = findEntry (pszFormName, ePrefix);

      if (-1 != indexForm)
      {
         pszRet = writeFormName (indexForm, false, false);
      }

      free ((void *)pszFormName);
   }

   return pszRet;
}

typedef struct _ReservedMap {
   PSZCRO pszName;
   unsigned int iValue;
} RESERVEDMAP, *PRESERVEDMAP;

static const RESERVEDMAP vaReservedKeywords[] = {
   { "NO_CAPABILITIES",             0x00000000 },
   { "FORM_CAPABILITY_ROLL",        0x80000000 },
   { "FORM_CAPABILITY_USERDEFINED", 0x40000000 }
};

bool DeviceForm::
isReservedKeyword (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return true;
   }

   return false;
}

unsigned int DeviceForm::
getReservedValue (PSZCRO pszId)
{
   for (int i = 0; i < (int)dimof (vaReservedKeywords); i++)
   {
      if (0 == strcmp (pszId, vaReservedKeywords[i].pszName))
         return vaReservedKeywords[i].iValue;
   }

   return 0;
}

class FormEnumerator : public Enumeration
{
public:

   FormEnumerator ()
   {
      iIndex_d = 0;
   }

   virtual ~
   FormEnumerator ()
   {
   }

   virtual bool
   hasMoreElements ()
   {
      return iIndex_d < (int)dimof (vaFormMapping);
   }

   virtual void *
   nextElement ()
   {
      if (!hasMoreElements ())
      {
         return 0;
      }

      PSZCRO  pszJP = writeFormName (iIndex_d++, true, true);
      void   *pvRet = 0;

      if (pszJP)
      {
         pvRet = (void *)new JobProperties (pszJP);

         free ((void *)pszJP);
      }

      return pvRet;
   }

private:
   int     iIndex_d;
};

Enumeration * DeviceForm::
getAllEnumeration ()
{
   return new FormEnumerator ();
}

#ifndef RETAIL

void DeviceForm::
outputSelf ()
{
   DebugOutput::getErrorStream () << *this << std::endl;
}

#endif

std::string DeviceForm::
toString (std::ostringstream& oss)
{
   oss << "{DeviceForm: "
       << "pszForm_d = " << SAFE_PRINT_PSZ (pszForm_d)
       << ", iCapabilities = 0x" << std::hex << iCapabilities_d << std::dec
       << ", hcInfo_d = " << *hcInfo_d
       << "}";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const DeviceForm& const_self)
{
   DeviceForm&        self = const_cast<DeviceForm&>(const_self);
   std::ostringstream oss;

   os << self.toString (oss);

   return os;
}

int
findEntry (PSZCRO  pszFormName,
           EPREFIX ePrefix)
{
   int   iLow     = 0;
   int   iMid     = (int)dimof (vaFormMapping) / 2;
   int   iHigh    = (int)dimof (vaFormMapping) - 1;
   int   iResult;

   if (  !pszFormName
      || !*pszFormName
      )
   {
      return -1;
   }

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszFormName, vaFormMapping[iMid].pszUPDFName);

      if (0 == iResult)
      {
         if (PREFIX_NONE == ePrefix)
         {
            return iMid;
         }

         if (ePrefix == vaFormMapping[iMid].ePrefix)
         {
            return iMid;
         }

         int iTest = iMid - 1;

         while (  0 <= iTest
               && 0 == strcmp (pszFormName, vaFormMapping[iTest].pszUPDFName)
               )
         {
            if (ePrefix == vaFormMapping[iTest].ePrefix)
            {
               return iTest;
            }

            iTest--;
         }

         iTest = iMid + 1;

         while (  (int)dimof (vaFormMapping) > iTest
               && 0 == strcmp (pszFormName, vaFormMapping[iTest].pszUPDFName)
               )
         {
            if (ePrefix == vaFormMapping[iTest].ePrefix)
            {
               return iTest;
            }

            iTest++;
         }

         return -1;
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

   return -1;
}

PSZCRO
writeFormName (int  indexForm,
               bool fInJPFormat,
               bool fInLongFormat)
{
   std::ostringstream  oss;
   std::string         stringName;
   char               *pszFormName = 0;

   if (  indexForm <  0
      || indexForm >= (int)dimof (vaFormMapping)
      )
   {
      return 0;
   }

   if (fInJPFormat)
   {
      oss << JOBPROP_FORM << "=";
   }

   oss << vaPrefixes[vaFormMapping[indexForm].ePrefix].pszName
       << vaFormMapping[indexForm].pszUPDFName;

   if (fInLongFormat)
   {
      oss << "_";

      if (INCHES == vaFormMapping[indexForm].eExternalDisplayUnits)
      {
         oss << std::setiosflags (std::ios::fixed)
             << std::setprecision (2)
             << (vaFormMapping[indexForm].cx / 25400.0)
             << "x"
             << (vaFormMapping[indexForm].cy / 25400.0)
             << "in";
      }
      else if (MM == vaFormMapping[indexForm].eExternalDisplayUnits)
      {
         oss << std::setiosflags (std::ios::fixed)
             << std::setprecision (2)
             << (vaFormMapping[indexForm].cx / 1000.0)
             << "x"
             << (vaFormMapping[indexForm].cy / 1000.0)
             << "mm";
      }
   }

   stringName = oss.str ();

   pszFormName = (char *)malloc (stringName.length () + 1);
   if (pszFormName)
   {
      strcpy (pszFormName, stringName.c_str ());
   }

   return pszFormName;
}

PSZCRO
writeFormName (int  indexForm,
               bool fInJPFormat,
               bool fInLongFormat,
               int  iX,
               int  iY)
{
   std::ostringstream  oss;
   std::string         stringName;
   char               *pszFormName = 0;

   if (  indexForm <  0
      || indexForm >= (int)dimof (vaFormMapping)
      )
   {
      return 0;
   }

   if (fInJPFormat)
   {
      oss << JOBPROP_FORM << "=";
   }

   oss << vaPrefixes[vaFormMapping[indexForm].ePrefix].pszName
       << vaFormMapping[indexForm].pszUPDFName;

   if (fInLongFormat)
   {
      oss << "_";

      if (INCHES == vaFormMapping[indexForm].eExternalDisplayUnits)
      {
         oss << std::setiosflags (std::ios::fixed)
             << std::setprecision (2)
             << (iX / 25400.0)
             << "x"
             << (iY / 25400.0)
             << "in";
      }
      else if (MM == vaFormMapping[indexForm].eExternalDisplayUnits)
      {
         oss << std::setiosflags (std::ios::fixed)
             << std::setprecision (2)
             << (iX / 1000.0)
             << "x"
             << (iY / 1000.0)
             << "mm";
      }
   }

   stringName = oss.str ();

   pszFormName = (char *)malloc (stringName.length () + 1);
   if (pszFormName)
   {
      strcpy (pszFormName, stringName.c_str ());
   }

   return pszFormName;
}

bool
getFormSize (PSZ   pszRest,
             int  *piFormSizeX,
             int  *piFormSizeY)
{
   float flFormSizeX = 0.0;
   float flFormSizeY = 0.0;

///std::cerr << __FUNCTION__ << ": pszRest = " << SAFE_PRINT_PSZ(pszRest) << std::endl;

   if (  !pszRest
      || !*pszRest
      )
   {
      return false;
   }

   PSZ pszX = strtok (pszRest, "Xx");

   if (!pszX)
   {
      return false;
   }

   pszX = strtok (NULL, "Xx");

   if (0 == sscanf (pszRest, "%f", &flFormSizeX))
   {
      return false;
   }
   if (0 == sscanf (pszX, "%f", &flFormSizeY))
   {
      return false;
   }

   PSZ    pszUnits = pszX + strlen (pszX) - 2;
   EUNITS eUnits   = UNKNOWN;

   if (0 == strcasecmp (pszUnits, "in"))
   {
      eUnits       = INCHES;
      flFormSizeX *= 25400.0;
      flFormSizeY *= 25400.0;
   }
   else if (0 == strcasecmp (pszUnits, "mm"))
   {
      eUnits       = MM;
      flFormSizeX *= 1000.0;
      flFormSizeY *= 1000.0;
   }
   else
   {
      return false;
   }

///std::cerr << "flFormSizeX = " << flFormSizeX << std::endl;
///std::cerr << "flFormSizeY = " << flFormSizeY << std::endl;

   if (piFormSizeX)
   {
      *piFormSizeX = (int)flFormSizeX;
   }

   if (piFormSizeY)
   {
      *piFormSizeY = (int)flFormSizeY;
   }

   return true;
}

bool DeviceForm::
getComponents (PSZCRO  pszJobProperties,
               PSZRO  *ppszForm,
               int    *pindexForm,
               int    *piFormSizeX,
               int    *piFormSizeY)
{
   JobProperties          jobProp (pszJobProperties);
   JobPropertyEnumerator *pEnum                      = 0;
   bool                   fRet                       = false;

   pEnum = jobProp.getEnumeration ();

   while (pEnum->hasMoreElements ())
   {
      PSZCRO pszKey   = pEnum->getCurrentKey ();
      PSZCRO pszValue = pEnum->getCurrentValue ();

      if (0 == strcmp (pszKey, JOBPROP_FORM))
      {
         PSZRO   pszStart = pszValue;
         EPREFIX ePrefix  = PREFIX_NONE;

         for (int i = 0; i < (int)dimof (vaPrefixes); i++)
         {
            if (0 == strncmp (pszValue,
                              vaPrefixes[i].pszName,
                              vaPrefixes[i].cbName))
            {
               ePrefix   = vaPrefixes[i].ePrefix;
               pszStart += vaPrefixes[i].cbName;
               break;
            }
         }

         char *pszFormName = (char *)malloc (strlen (pszStart) + 1);

         if (pszFormName)
         {
            char *pszUnderscore = 0;
            int   indexForm     = -1;

            strcpy (pszFormName, pszStart);

            pszUnderscore = strchr (pszFormName, '_');

            if (pszUnderscore)
            {
               *pszUnderscore = '\0';
            }

            indexForm = findEntry (pszFormName, ePrefix);

            if (-1 != indexForm)
            {
               if (  pszUnderscore
                  && *(pszUnderscore + 1)
                  )
               {
                  getFormSize (pszUnderscore + 1, piFormSizeX, piFormSizeY);
               }
               if (ppszForm)
               {
                  *ppszForm = getLongFormName (pszValue);
               }
               if (pindexForm)
               {
                  *pindexForm = indexForm;
               }

               fRet = true;
            }

            free ((void *)pszFormName);
         }
      }

      pEnum->nextElement ();
   }

   delete pEnum;

   return fRet;
}
