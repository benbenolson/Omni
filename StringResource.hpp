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
#ifndef _StringResource_hpp
#define _StringResource_hpp

#include "defines.hpp"
#include "Enumeration.hpp"

class StringResource
{
public:
   enum {
      LANGUAGE_UNKNOWN = 0,
      LANGUAGE_AA,          // Afar
      LANGUAGE_AB,          // Abkhazian
      LANGUAGE_AF,          // Afrikaans
      LANGUAGE_AM,          // Amharic
      LANGUAGE_AR,          // Arabic
      LANGUAGE_AS,          // Assamese
      LANGUAGE_AY,          // Aymara
      LANGUAGE_AZ,          // Azerbaijani
      LANGUAGE_BA,          // Bashkir
      LANGUAGE_BE,          // Byelorussian
      LANGUAGE_BG,          // Bulgarian
      LANGUAGE_BH,          // Bihari
      LANGUAGE_BI,          // Bislama
      LANGUAGE_BN,          // Bengali Bangla
      LANGUAGE_BO,          // Tibetan
      LANGUAGE_BR,          // Breton
      LANGUAGE_CA,          // Catalan
      LANGUAGE_CO,          // Corsican
      LANGUAGE_CS,          // Czech
      LANGUAGE_CY,          // Welsh
      LANGUAGE_DA,          // Danish
      LANGUAGE_DE,          // German
      LANGUAGE_DZ,          // Bhutani
      LANGUAGE_EL,          // Greek
      LANGUAGE_EN,          // English
      LANGUAGE_EO,          // Esperanto
      LANGUAGE_ES,          // Spanish
      LANGUAGE_ET,          // Estonian
      LANGUAGE_EU,          // Basque
      LANGUAGE_FA,          // Farsi
      LANGUAGE_FI,          // Finnish
      LANGUAGE_FJ,          // Fiji
      LANGUAGE_FO,          // Faeroese
      LANGUAGE_FR,          // French
      LANGUAGE_FY,          // Frisian
      LANGUAGE_GA,          // Irish
      LANGUAGE_GD,          // Scots Gaelic
      LANGUAGE_GL,          // Galician
      LANGUAGE_GN,          // Guarani
      LANGUAGE_GU,          // Gujarati
      LANGUAGE_GV,          // Manx Gaelic
      LANGUAGE_HA,          // Hausa
      LANGUAGE_HE,          // Hebrew
      LANGUAGE_HI,          // Hindi
      LANGUAGE_HR,          // Croatian
      LANGUAGE_HU,          // Hungarian
      LANGUAGE_HY,          // Armenian
      LANGUAGE_IA,          // Interlingua
      LANGUAGE_ID,          // Indonesian
      LANGUAGE_IE,          // Interlingue
      LANGUAGE_IK,          // Inupiak
      LANGUAGE_IN,          // Indonesian
      LANGUAGE_IS,          // Icelandic
      LANGUAGE_IT,          // Italian
      LANGUAGE_IU,          // Inuktitut
      LANGUAGE_IW,          // Hebrew
      LANGUAGE_JA,          // Japanese
      LANGUAGE_JI,          // Yiddish
      LANGUAGE_JW,          // Javanese
      LANGUAGE_KA,          // Georgian
      LANGUAGE_KK,          // Kazakh
      LANGUAGE_KL,          // Greenlandic
      LANGUAGE_KM,          // Cambodian
      LANGUAGE_KN,          // Kannada
      LANGUAGE_KO,          // Korean
      LANGUAGE_KS,          // Kashmiri
      LANGUAGE_KU,          // Kurdish
      LANGUAGE_KY,          // Kirghiz
      LANGUAGE_LA,          // Latin
      LANGUAGE_LO,          // Laothian
      LANGUAGE_LN,          // Lingala
      LANGUAGE_LT,          // Lithuanian
      LANGUAGE_LV,          // Latvian Lettish
      LANGUAGE_MG,          // Malagasy
      LANGUAGE_MI,          // Maori
      LANGUAGE_MK,          // Macedonian
      LANGUAGE_ML,          // Malayalam
      LANGUAGE_MN,          // Mongolian
      LANGUAGE_MO,          // Moldavian
      LANGUAGE_MR,          // Marathi
      LANGUAGE_MS,          // Malay
      LANGUAGE_MT,          // Maltese
      LANGUAGE_MY,          // Burmese
      LANGUAGE_NA,          // Nauru
      LANGUAGE_NE,          // Nepali
      LANGUAGE_NL,          // Dutch
      LANGUAGE_NO,          // Norwegian
      LANGUAGE_OC,          // Occitan
      LANGUAGE_OM,          // Oromo Afan
      LANGUAGE_OR,          // Oriya
      LANGUAGE_PA,          // Punjabi
      LANGUAGE_PL,          // Polish
      LANGUAGE_PS,          // Pashto Pushto
      LANGUAGE_PT,          // Portuguese
      LANGUAGE_QU,          // Quechua
      LANGUAGE_RM,          // Rhaeto Romance
      LANGUAGE_RN,          // Kirundi
      LANGUAGE_RO,          // Romanian
      LANGUAGE_RU,          // Russian
      LANGUAGE_RW,          // Kinyarwanda
      LANGUAGE_SA,          // Sanskrit
      LANGUAGE_SG,          // Sangro
      LANGUAGE_SH,          // Serbo Croatian
      LANGUAGE_SD,          // Sindhi
      LANGUAGE_SI,          // Singhalese
      LANGUAGE_SK,          // Slovak
      LANGUAGE_SL,          // Slovenian
      LANGUAGE_SM,          // Samoan
      LANGUAGE_SN,          // Shona
      LANGUAGE_SO,          // Somali
      LANGUAGE_SQ,          // Albanian
      LANGUAGE_SR,          // Serbian
      LANGUAGE_SS,          // Siswati
      LANGUAGE_ST,          // Sesotho
      LANGUAGE_SU,          // Sundanese
      LANGUAGE_SV,          // Swedish
      LANGUAGE_SW,          // Swahili
      LANGUAGE_TA,          // Tamil
      LANGUAGE_TE,          // Telugu
      LANGUAGE_TG,          // Tajik
      LANGUAGE_TH,          // Thai
      LANGUAGE_TI,          // Tigrinya
      LANGUAGE_TK,          // Turkmen
      LANGUAGE_TL,          // Tagalog
      LANGUAGE_TN,          // Setswana
      LANGUAGE_TO,          // Tonga
      LANGUAGE_TR,          // Turkish
      LANGUAGE_TS,          // Tsonga
      LANGUAGE_TT,          // Tatar
      LANGUAGE_TW,          // Twi
      LANGUAGE_UG,          // Uighur
      LANGUAGE_UK,          // Ukrainian
      LANGUAGE_UR,          // Urdu
      LANGUAGE_UZ,          // Uzbek
      LANGUAGE_VI,          // Vietnamese
      LANGUAGE_VO,          // Volapk
      LANGUAGE_WO,          // Wolof
      LANGUAGE_XH,          // Xhosa
      LANGUAGE_YI,          // Yiddish
      LANGUAGE_YO,          // Yoruba
      LANGUAGE_ZH,          // Chinese
      LANGUAGE_ZU           // Zulu
   };

   enum {
      LANGUAGE_DEFAULT = LANGUAGE_EN
   };

   enum {
      DEVICE_COMMON_UNKNOWN = 0,
      DEVICE_COMMON_UNLISTED,
      DEVICE_COMMON_BOOKLET,
      DEVICE_COMMON_COPIES,
      DEVICE_COMMON_DITHER,
      DEVICE_COMMON_FORM,
      DEVICE_COMMON_JOGGING,
      DEVICE_COMMON_MEDIA,
      DEVICE_COMMON_NUP,
      DEVICE_COMMON_NUP_DIRECTION,
      DEVICE_COMMON_ORIENTATION,
      DEVICE_COMMON_OUTPUTBIN,
      DEVICE_COMMON_PRINTMODE,
      DEVICE_COMMON_RESOLUTION,
      DEVICE_COMMON_SCALING_TYPE,
      DEVICE_COMMON_SCALING_PERCENTAGE,
      DEVICE_COMMON_SHEETCOLLATE,
      DEVICE_COMMON_SIDE,
      DEVICE_COMMON_STITCHING_POSITION,
      DEVICE_COMMON_STITCHING_REFERENCE_EDGE,
      DEVICE_COMMON_STITCHING_STITCHING_TYPE,
      DEVICE_COMMON_STITCHING_COUNT,
      DEVICE_COMMON_STITCHING_ANGLE,
      DEVICE_COMMON_TRAY,
      DEVICE_COMMON_TRIMMING,
      DEVICE_COMMON_BIDIRECTIONAL,
      DEVICE_COMMON_TRUE,
      DEVICE_COMMON_FALSE,
      DEVICE_COMMON_JOURNAL,
      DEVICE_COMMON_CUTMODE,
      DEVICE_COMMON_NONE,
      DEVICE_COMMON_FULL,
      DEVICE_COMMON_PARTIAL,
      DEVICE_COMMON_HARDWARE_SCALING,
      DEVICE_COMMON_QUALITY,
      DEVICE_COMMON_DRAFT,
      DEVICE_COMMON_STANDARD,
      DEVICE_COMMON_FINE,
      DEVICE_COMMON_SUPER_FINE,
      DEVICE_COMMON_PHOTO,
      DEVICE_COMMON_SUPER_PHOTO,
      DEVICE_COMMON_HIGH_SPEED,
      DEVICE_COMMON_ON,
      DEVICE_COMMON_OFF,
      DEVICE_COMMON_COLOR_MODE,
      DEVICE_COMMON_AUTO,
      DEVICE_COMMON_GRAPH,
      DEVICE_COMMON_MICRO_WEAVE,
      DEVICE_COMMON_RET,
      DEVICE_COMMON_ECONO_MODE,
      DEVICE_COMMON_PAGE_PROTECT,
      DEVICE_COMMON_JAM_RECOVERY,
      DEVICE_COMMON_STAPLE,
      DEVICE_COMMON_OFFSET,
      DEVICE_COMMON_FROM,
      DEVICE_COMMON_TO,
      DEVICE_COMMON_INFINITE
   };

   enum {
      STRINGGROUP_UNKNOWN = 0,
      STRINGGROUP_DEVICE_COMMON,
#ifdef INCLUDE_JP_UPDF_BOOKLET
      STRINGGROUP_BOOKLETS,
#endif
      STRINGGROUP_DITHERS,
      STRINGGROUP_FORMS,
#ifdef INCLUDE_JP_UPDF_JOGGING
      STRINGGROUP_JOGGINGS,
#endif
      STRINGGROUP_MEDIAS,
#ifdef INCLUDE_JP_COMMON_NUP
      STRINGGROUP_NUPS,
#endif
      STRINGGROUP_ORIENTATIONS,
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      STRINGGROUP_OUTPUT_BINS,
#endif
      STRINGGROUP_PRINTMODES,
      STRINGGROUP_RESOLUTIONS,
#ifdef INCLUDE_JP_COMMON_SCALING
      STRINGGROUP_SCALINGS,
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      STRINGGROUP_SHEET_COLLATES,
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
      STRINGGROUP_SIDES,
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
      STRINGGROUP_STITCHING_EDGES,
      STRINGGROUP_STITCHING_TYPES,
#endif
#ifdef INCLUDE_JP_COMMON_TRIMMING
      STRINGGROUP_TRIMMINGS,
#endif
      STRINGGROUP_TRAYS
   };

   static int                nameToID     (PSZCRO              pszId);
   static PSZCRO             IDToName     (int                 id);

   static Enumeration       *getLanguages ();

   static StringResource    *create       (int                 iLanguageID,
                                           StringResource     *pFallback);

   /* @TBD since you can't have static virual functions, what is the right way
   **      to get around it?
   */
   static  PSZCRO            getString    (StringResource     *pLanguage,
                                           int                 iGroup,
                                           int                 iID);
   static  PSZCRO            getString    (StringResource     *pLanguage,
                                           int                 iGroup,
                                           PSZCRO              pszID);

   virtual PSZCRO            getStringV   (int                 iGroup,
                                           int                 iID) = 0;
   virtual PSZCRO            getStringV   (int                 iGroup,
                                           PSZCRO              pszID) = 0;
};

#endif
