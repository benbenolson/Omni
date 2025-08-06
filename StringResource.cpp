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
#include "StringResource.hpp"
#include "StringResourceEn.hpp"
#include "defines.hpp"

#include <cstring>
#include <cstdio>

typedef struct _NameToID {
   PSZCRO pszName;
   int    iID;
} NAMETOID, *PNAMETOID;

static NAMETOID aStringToIDMapping[] = {
   { "aa", StringResource::LANGUAGE_AA },
   { "ab", StringResource::LANGUAGE_AB },
   { "af", StringResource::LANGUAGE_AF },
   { "am", StringResource::LANGUAGE_AM },
   { "ar", StringResource::LANGUAGE_AR },
   { "as", StringResource::LANGUAGE_AS },
   { "ay", StringResource::LANGUAGE_AY },
   { "az", StringResource::LANGUAGE_AZ },
   { "ba", StringResource::LANGUAGE_BA },
   { "be", StringResource::LANGUAGE_BE },
   { "bg", StringResource::LANGUAGE_BG },
   { "bh", StringResource::LANGUAGE_BH },
   { "bi", StringResource::LANGUAGE_BI },
   { "bn", StringResource::LANGUAGE_BN },
   { "bo", StringResource::LANGUAGE_BO },
   { "br", StringResource::LANGUAGE_BR },
   { "ca", StringResource::LANGUAGE_CA },
   { "co", StringResource::LANGUAGE_CO },
   { "cs", StringResource::LANGUAGE_CS },
   { "cy", StringResource::LANGUAGE_CY },
   { "da", StringResource::LANGUAGE_DA },
   { "de", StringResource::LANGUAGE_DE },
   { "dz", StringResource::LANGUAGE_DZ },
   { "el", StringResource::LANGUAGE_EL },
   { "en", StringResource::LANGUAGE_EN },
   { "eo", StringResource::LANGUAGE_EO },
   { "es", StringResource::LANGUAGE_ES },
   { "et", StringResource::LANGUAGE_ET },
   { "eu", StringResource::LANGUAGE_EU },
   { "fa", StringResource::LANGUAGE_FA },
   { "fi", StringResource::LANGUAGE_FI },
   { "fj", StringResource::LANGUAGE_FJ },
   { "fo", StringResource::LANGUAGE_FO },
   { "fr", StringResource::LANGUAGE_FR },
   { "fy", StringResource::LANGUAGE_FY },
   { "ga", StringResource::LANGUAGE_GA },
   { "gd", StringResource::LANGUAGE_GD },
   { "gl", StringResource::LANGUAGE_GL },
   { "gn", StringResource::LANGUAGE_GN },
   { "gu", StringResource::LANGUAGE_GU },
   { "gv", StringResource::LANGUAGE_GV },
   { "ha", StringResource::LANGUAGE_HA },
   { "he", StringResource::LANGUAGE_HE },
   { "hi", StringResource::LANGUAGE_HI },
   { "hr", StringResource::LANGUAGE_HR },
   { "hu", StringResource::LANGUAGE_HU },
   { "hy", StringResource::LANGUAGE_HY },
   { "ia", StringResource::LANGUAGE_IA },
   { "id", StringResource::LANGUAGE_ID },
   { "ie", StringResource::LANGUAGE_IE },
   { "ik", StringResource::LANGUAGE_IK },
   { "in", StringResource::LANGUAGE_IN },
   { "is", StringResource::LANGUAGE_IS },
   { "it", StringResource::LANGUAGE_IT },
   { "iu", StringResource::LANGUAGE_IU },
   { "iw", StringResource::LANGUAGE_IW },
   { "ja", StringResource::LANGUAGE_JA },
   { "ji", StringResource::LANGUAGE_JI },
   { "jw", StringResource::LANGUAGE_JW },
   { "ka", StringResource::LANGUAGE_KA },
   { "kk", StringResource::LANGUAGE_KK },
   { "kl", StringResource::LANGUAGE_KL },
   { "km", StringResource::LANGUAGE_KM },
   { "kn", StringResource::LANGUAGE_KN },
   { "ko", StringResource::LANGUAGE_KO },
   { "ks", StringResource::LANGUAGE_KS },
   { "ku", StringResource::LANGUAGE_KU },
   { "ky", StringResource::LANGUAGE_KY },
   { "la", StringResource::LANGUAGE_LA },
   { "ln", StringResource::LANGUAGE_LO },
   { "lo", StringResource::LANGUAGE_LN },
   { "lt", StringResource::LANGUAGE_LT },
   { "lv", StringResource::LANGUAGE_LV },
   { "mg", StringResource::LANGUAGE_MG },
   { "mi", StringResource::LANGUAGE_MI },
   { "mk", StringResource::LANGUAGE_MK },
   { "ml", StringResource::LANGUAGE_ML },
   { "mn", StringResource::LANGUAGE_MN },
   { "mo", StringResource::LANGUAGE_MO },
   { "mr", StringResource::LANGUAGE_MR },
   { "ms", StringResource::LANGUAGE_MS },
   { "mt", StringResource::LANGUAGE_MT },
   { "my", StringResource::LANGUAGE_MY },
   { "na", StringResource::LANGUAGE_NA },
   { "ne", StringResource::LANGUAGE_NE },
   { "nl", StringResource::LANGUAGE_NL },
   { "no", StringResource::LANGUAGE_NO },
   { "oc", StringResource::LANGUAGE_OC },
   { "om", StringResource::LANGUAGE_OM },
   { "or", StringResource::LANGUAGE_OR },
   { "pa", StringResource::LANGUAGE_PA },
   { "pl", StringResource::LANGUAGE_PL },
   { "ps", StringResource::LANGUAGE_PS },
   { "pt", StringResource::LANGUAGE_PT },
   { "qu", StringResource::LANGUAGE_QU },
   { "rm", StringResource::LANGUAGE_RM },
   { "rn", StringResource::LANGUAGE_RN },
   { "ro", StringResource::LANGUAGE_RO },
   { "ru", StringResource::LANGUAGE_RU },
   { "rw", StringResource::LANGUAGE_RW },
   { "sa", StringResource::LANGUAGE_SA },
   { "sd", StringResource::LANGUAGE_SG },
   { "sg", StringResource::LANGUAGE_SH },
   { "sh", StringResource::LANGUAGE_SD },
   { "si", StringResource::LANGUAGE_SI },
   { "sk", StringResource::LANGUAGE_SK },
   { "sl", StringResource::LANGUAGE_SL },
   { "sm", StringResource::LANGUAGE_SM },
   { "sn", StringResource::LANGUAGE_SN },
   { "so", StringResource::LANGUAGE_SO },
   { "sq", StringResource::LANGUAGE_SQ },
   { "sr", StringResource::LANGUAGE_SR },
   { "ss", StringResource::LANGUAGE_SS },
   { "st", StringResource::LANGUAGE_ST },
   { "su", StringResource::LANGUAGE_SU },
   { "sv", StringResource::LANGUAGE_SV },
   { "sw", StringResource::LANGUAGE_SW },
   { "ta", StringResource::LANGUAGE_TA },
   { "te", StringResource::LANGUAGE_TE },
   { "tg", StringResource::LANGUAGE_TG },
   { "th", StringResource::LANGUAGE_TH },
   { "ti", StringResource::LANGUAGE_TI },
   { "tk", StringResource::LANGUAGE_TK },
   { "tl", StringResource::LANGUAGE_TL },
   { "tn", StringResource::LANGUAGE_TN },
   { "to", StringResource::LANGUAGE_TO },
   { "tr", StringResource::LANGUAGE_TR },
   { "ts", StringResource::LANGUAGE_TS },
   { "tt", StringResource::LANGUAGE_TT },
   { "tw", StringResource::LANGUAGE_TW },
   { "ug", StringResource::LANGUAGE_UG },
   { "uk", StringResource::LANGUAGE_UK },
   { "ur", StringResource::LANGUAGE_UR },
   { "uz", StringResource::LANGUAGE_UZ },
   { "vi", StringResource::LANGUAGE_VI },
   { "vo", StringResource::LANGUAGE_VO },
   { "wo", StringResource::LANGUAGE_WO },
   { "xh", StringResource::LANGUAGE_XH },
   { "yi", StringResource::LANGUAGE_YI },
   { "yo", StringResource::LANGUAGE_YO },
   { "zh", StringResource::LANGUAGE_ZH },
   { "zu", StringResource::LANGUAGE_ZU }
};

int StringResource::
nameToID (PSZCRO pszId)
{
   if (  !pszId
      || !*pszId
      )
   {
      return StringResource::LANGUAGE_UNKNOWN;
   }

   int iLow  = 0;
   int iMid  = (int)dimof (aStringToIDMapping) / 2;
   int iHigh = (int)dimof (aStringToIDMapping) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszId, aStringToIDMapping[iMid].pszName);

      if (0 == iResult)
      {
         return aStringToIDMapping[iMid].iID;
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

   return StringResource::LANGUAGE_UNKNOWN;
}

PSZCRO apszIDToStringMapping[] = {
   "\?\?",
   "aa",
   "ab",
   "af",
   "am",
   "ar",
   "as",
   "ay",
   "az",
   "ba",
   "be",
   "bg",
   "bh",
   "bi",
   "bn",
   "bo",
   "br",
   "ca",
   "co",
   "cs",
   "cy",
   "da",
   "de",
   "dz",
   "el",
   "en",
   "eo",
   "es",
   "et",
   "eu",
   "fa",
   "fi",
   "fj",
   "fo",
   "fr",
   "fy",
   "ga",
   "gd",
   "gl",
   "gn",
   "gu",
   "gv",
   "ha",
   "he",
   "hi",
   "hr",
   "hu",
   "hy",
   "ia",
   "id",
   "ie",
   "ik",
   "in",
   "is",
   "it",
   "iu",
   "iw",
   "ja",
   "ji",
   "jw",
   "ka",
   "kk",
   "kl",
   "km",
   "kn",
   "ko",
   "ks",
   "ku",
   "ky",
   "la",
   "ln",
   "lo",
   "lt",
   "lv",
   "mg",
   "mi",
   "mk",
   "ml",
   "mn",
   "mo",
   "mr",
   "ms",
   "mt",
   "my",
   "na",
   "ne",
   "nl",
   "no",
   "oc",
   "om",
   "or",
   "pa",
   "pl",
   "ps",
   "pt",
   "qu",
   "rm",
   "rn",
   "ro",
   "ru",
   "rw",
   "sa",
   "sd",
   "sg",
   "sh",
   "si",
   "sk",
   "sl",
   "sm",
   "sn",
   "so",
   "sq",
   "sr",
   "ss",
   "st",
   "su",
   "sv",
   "sw",
   "ta",
   "te",
   "tg",
   "th",
   "ti",
   "tk",
   "tl",
   "tn",
   "to",
   "tr",
   "ts",
   "tt",
   "tw",
   "ug",
   "uk",
   "ur",
   "uz",
   "vi",
   "vo",
   "wo",
   "xh",
   "yi",
   "yo",
   "zh",
   "zu"
};

PSZCRO StringResource::
IDToName (int id)
{
   id -= StringResource::LANGUAGE_UNKNOWN;

   if (  0 <= id
      && id < (int)dimof (apszIDToStringMapping)
      )
   {
      return apszIDToStringMapping[id];
   }
   else
   {
      static char achUnknown[21];

      sprintf (achUnknown, "Unknown (%d)", id + StringResource::LANGUAGE_UNKNOWN);

      return achUnknown;
   }
}

static int vaiLanguagesSupported[] = {
   StringResource::LANGUAGE_EN
};

StringResource * StringResource::
create (int             iLanguageID,
        StringResource *pFallback)
{
   switch (iLanguageID)
   {
   case LANGUAGE_EN: return new StringResourceEn (pFallback);
   default:          return 0;
   }
}

class LanguageEnumerator : public Enumeration
{
public:
   LanguageEnumerator (int cLanguages, int *aLanguages)
   {
      iLanguage_d  = 0;
      cLanguages_d = cLanguages;
      aLanguages_d = aLanguages;
   }

   virtual bool hasMoreElements ()
   {
      if (iLanguage_d < cLanguages_d)
         return true;
      else
         return false;
   }

   virtual void *nextElement ()
   {
      if (iLanguage_d > cLanguages_d - 1)
         return 0;

      return (void *)aLanguages_d[iLanguage_d++];
   }

private:
   int    iLanguage_d;
   int    cLanguages_d;
   int   *aLanguages_d;
};

Enumeration * StringResource::
getLanguages ()
{
   return new LanguageEnumerator (dimof (vaiLanguagesSupported), vaiLanguagesSupported);
}

PSZCRO StringResource::
getString (StringResource *pLanguage,
           int             iGroup,
           int             iID)
{
   if (pLanguage)
      return pLanguage->getStringV (iGroup, iID);
   else
      return 0;
}

PSZCRO StringResource::
getString (StringResource *pLanguage,
           int             iGroup,
           PSZCRO          pszID)
{
   if (pLanguage)
      return pLanguage->getStringV (iGroup, pszID);
   else
      return 0;
}
