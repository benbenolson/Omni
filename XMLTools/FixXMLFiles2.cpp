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
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdarg.h>
#include <glob.h>

#include "defines.hpp"
#include "XMLInterface.hpp"

const static bool vfDebug        = false;
const static bool vfDumpFile     = false;
const static bool vfDumpElements = true;

char *
bundleStringData (XmlNodePtr nodeItem)
{
   return (char *)XMLNodeListGetString (XMLGetDocNode (nodeItem), XMLGetChildrenNode (nodeItem), 1);
}

PSZCRO
mapFormName (PSZCRO pszFrom)
{
   typedef struct _Mapping {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } MAPPING;

   static MAPPING apszNames[] = {
      { "FORM_10_X_11",                 "na_10x11_10.00x11.00in"                     },
      { "FORM_11_X_12",                 "na_11x12_11.00x12.00in"                     },
      { "FORM_11_X_15",                 "na_11x15_11.00x15.00in"                     },
      { "FORM_12_X_19",                 "na_12x19_12.00x19.00in"                     },
      { "FORM_15_X_11",                 "na_15x11_15.00x11.00in"                     },
      { "FORM_170_X_210",               "na_170x210_1700.00x2100.00mm"               },
      { "FORM_182_X_210",               "na_180x210_1820.00x2100.00mm"               },
      { "FORM_2A0",                     "iso_2a0_11890.00x16820.00mm"                },
      { "FORM_3_X_5_CARD",              "na_3x5-card_3.00x5.00in"                    },
      { "FORM_4A0",                     "iso_4a0_16820.00x23780.00mm"                },
      { "FORM_4_X_6_CARD",              "na_4x6-card_4.00x6.00in"                    },
      { "FORM_5_X_7",                   "na_5x7_5.00x7.00in"                         },
      { "FORM_5_X_8_CARD",              "na_5x8-card_5.00x8.00in"                    },
      { "FORM_8_25_X_13",               "na_8.25x13_8.25x13.00in"                    },
      { "FORM_8_5_X_12_4",              "na_8.5x12.4_8.50x12.40in"                   },
      { "FORM_8_X_10_5",                "na_8x10.5_8.00x10.50in"                     },
      { "FORM_8_X_10_CARD",             "na_8x10-card_8.00x10.00in"                  },
      { "FORM_A0",                      "iso_a0_8410.00x11890.00mm"                  },
      { "FORM_A1",                      "iso_a1_5940.00x8410.00mm"                   },
      { "FORM_A10",                     "iso_a10_260.00x370.00mm"                    },
      { "FORM_A2",                      "iso_a2_4200.00x5940.00mm"                   },
      { "FORM_A2_ENVELOPE",             "na_a2_4.38x5.75in"                          },
      { "FORM_A3",                      "iso_a3_2970.00x4200.00mm"                   },
      { "FORM_A3_EXTRA",                "iso_a3-extra_3220.00x4450.00mm"             },
      { "FORM_A3_WIDE",                 "iso_a3-wide_3302.00x4826.00mm"              },
      { "FORM_A4",                      "iso_a4_2100.00x2970.00mm"                   },
      { "FORM_A4_EXTRA",                "iso_a4-extra_2355.00x3223.00mm"             },
      { "FORM_A4_TAB",                  "iso_a4-tab_2250.00x2970.00mm"               },
      { "FORM_A4_WIDE",                 "iso_a4-wide_2235.00x3556.00mm"              },
      { "FORM_A5",                      "iso_a5_1480.00x2100.00mm"                   },
      { "FORM_A5_EXTRA",                "iso_a5-extra_1740.00x2350.00mm"             },
      { "FORM_A6",                      "iso_a6_1050.00x1480.00mm"                   },
      { "FORM_A6_CARD",                 "na_a6-card_4.13x5.83in"                     },
      { "FORM_A7",                      "iso_a7_740.00x1050.00mm"                    },
      { "FORM_A8",                      "iso_a8_520.00x740.00mm"                     },
      { "FORM_A9",                      "iso_a9_370.00x520.00mm"                     },
      { "FORM_B0",                      "iso_b0_10000.00x14140.00mm"                 },
      { "FORM_B1",                      "iso_b1_7070.00x10000.00mm"                  },
      { "FORM_B10",                     "iso_b10_310.00x440.00mm"                    },
      { "FORM_B2",                      "iso_b2_5000.00x7070.00mm"                   },
      { "FORM_B3",                      "iso_b3_3530.00x5000.00mm"                   },
      { "FORM_B4",                      "iso_b4_2500.00x3530.00mm"                   },
      { "FORM_B5",                      "iso_b5_1760.00x2500.00mm"                   },
      { "FORM_B5_EXTRA",                "iso_b5-extra_2010.00x2760.00mm"             },
      { "FORM_B6",                      "iso_b6_1250.00x1760.00mm"                   },
      { "FORM_B6_C4",                   "iso_b6c4_1250.00x3240.00mm"                 },
      { "FORM_B7",                      "iso_b7_880.00x1250.00mm"                    },
      { "FORM_B8",                      "iso_b8_620.00x880.00mm"                     },
      { "FORM_B9",                      "iso_b9_440.00x620.00mm"                     },
      { "FORM_B_PLUS",                  "na_b-plus_12.00x19.17in"                    },
      { "FORM_C0",                      "iso_c0_9170.00x12970.00mm"                  },
      { "FORM_C1",                      "iso_c1_6480.00x9170.00mm"                   },
      { "FORM_C10",                     "iso_c10_280.00x400.00mm"                    },
      { "FORM_C10_ENVELOPE",            "na_c10_4.12x9.50in"                         },
      { "FORM_C2",                      "iso_c2_4580.00x6480.00mm"                   },
      { "FORM_C3",                      "iso_c3_3240.00x4580.00mm"                   },
      { "FORM_C4",                      "iso_c4_2290.00x3240.00mm"                   },
      { "FORM_C5",                      "iso_c5_1620.00x2290.00mm"                   },
      { "FORM_C5_ENVELOPE",             "na_c5_6.38x9.02in"                          },
      { "FORM_C6",                      "iso_c6_1140.00x1620.00mm"                   },
      { "FORM_C6_C5",                   "iso_c6c5_1140.00x2290.00mm"                 },
      { "FORM_C6_ENVELOPE",             "na_c6_4.49x6.38in"                          },
      { "FORM_C7",                      "iso_c7_810.00x1140.00mm"                    },
      { "FORM_C7_C6",                   "iso_c7c6_810.00x1620.00mm"                  },
      { "FORM_C7_ENVELOPE",             "na_c7_3.87x7.50in"                          },
      { "FORM_C8",                      "iso_c8_570.00x810.00mm"                     },
      { "FORM_C9",                      "iso_c9_400.00x570.00mm"                     },
      { "FORM_C9_ENVELOPE",             "na_c9_3.87x8.87in"                          },
      { "FORM_CARD_148",                "na_card-148_1480.00x1050.00mm"              },
      { "FORM_CHOU2_ENVELOPE",          "jpn_chou2_1111.00x1460.00mm"                },
      { "FORM_CHOU3_ENVELOPE",          "jpn_chou3_1200.00x2350.00mm"                },
      { "FORM_CHOU4_ENVELOPE",          "jpn_chou4_900.00x2050.00mm"                 },
      { "FORM_D5_ENVELOPE",             "na_d5_6.93x9.84in"                          },
      { "FORM_DAI_PA_KAI",              "om_dai-pa-kai_2750.00x3950.00mm"            },
      { "FORM_DISK_LABELS",             "na_disk-labels_2.13x2.76in"                 },
      { "FORM_DL_ENVELOPE",             "na_dl_4.33x8.66in"                          },
      { "FORM_E1",                      "iso_e1_7112.00x10160.00mm"                  },
      { "FORM_EDP",                     "na_edp_11.00x14.00in"                       },
      { "FORM_ENVELOPE_10_X_13",        "na_envelope-10x13_10.00x13.00in"            },
      { "FORM_ENVELOPE_10_X_14",        "na_envelope-10x14_10.00x14.00in"            },
      { "FORM_ENVELOPE_10_X_15",        "na_envelope-10x15_10.00x15.00in"            },
      { "FORM_ENVELOPE_132_X_220",      "na_envelope-132x220_5.20x8.66in"            },
      { "FORM_ENVELOPE_6_1_2",          "na_envelope-6.5_6.50x3.62in"                },
      { "FORM_ENVELOPE_6_X_9",          "na_envelope-6x9_6.00x9.00in"                },
      { "FORM_ENVELOPE_7_X_9",          "na_envelope-7x9_7.00x9.00in"                },
      { "FORM_ENVELOPE_9_X_11",         "na_envelope-9x11_9.00x11.00in"              },
      { "FORM_EUROPEAN_EDP",            "na_eur-edp_11.57x14.00in"                   },
      { "FORM_EUROPEAN_FANFOLD",        "na_fanfold-eur_8.50x12.00in"                },
      { "FORM_EURO_LABELS",             "na_euro-labels_1.42x3.50in"                 },
      { "FORM_EXECUTIVE",               "na_executive_7.25x10.50in"                  },
      { "FORM_FANFOLD_1",               "na_fanfold-1_14.50x11.00in"                 },
      { "FORM_FANFOLD_2",               "na_fanfold-2_12.00x8.50in"                  },
      { "FORM_FANFOLD_3",               "na_fanfold-3_9.50x11.00in"                  },
      { "FORM_FANFOLD_4",               "na_fanfold-4_8.50x12.00in"                  },
      { "FORM_FANFOLD_5",               "na_fanfold-5_8.50x11.00in"                  },
      { "FORM_FOLIO",                   "om_folio_2159.00x3300.00mm"                 },
      { "FORM_FOLIO_SP",                "om_folio-sp_2150.00x3150.00mm"              },
      { "FORM_FOOLSCAP",                "na_foolscap_8.00x13.00in"                   },
      { "FORM_FOOLSCAP_WIDE",           "na_foolscap-wide_8.50x13.00in"              },
      { "FORM_GERMAN_12_X_250_FANFOLD", "na_german-12x250-fanfold_3048.00x2400.00mm" },
      { "FORM_GERMAN_LEGAL_FANFOLD",    "na_german-legal-fanfold_8.50x13.00in"       },
      { "FORM_GOVERNMENT_LEGAL",        "na_govt-letter_8.00x13.00in"                },
      { "FORM_GOVERNMENT_LETTER",       "na_govt-legal_8.00x10.00in"                 },
      { "FORM_HAGAKI_CARD",             "jpn_hagaki_1000.00x1480.00mm"               },
      { "FORM_HALF_LETTER",             "na_half-letter_5.50x8.50in"                 },
      { "FORM_INDEX_4_X_6_EXT",         "na_index-4x6-ext_6.00x8.00in"               },
      { "FORM_INVITE_ENVELOPE",         "om_invite_2200.00x2200.00mm"                },
      { "FORM_ITALIAN_ENVELOPE",        "om_italian_1000.00x2300.00mm"               },
      { "FORM_JIS_B0",                  "jis_b0_10300.00x14560.00mm"                 },
      { "FORM_JIS_B1",                  "jis_b1_7280.00x10300.00mm"                  },
      { "FORM_JIS_B10",                 "jis_b10_320.00x450.00mm"                    },
      { "FORM_JIS_B2",                  "jis_b2_5150.00x7280.00mm"                   },
      { "FORM_JIS_B3",                  "jis_b3_3640.00x5150.00mm"                   },
      { "FORM_JIS_B4",                  "jis_b4_2570.00x3640.00mm"                   },
      { "FORM_JIS_B5",                  "jis_b5_1820.00x2570.00mm"                   },
      { "FORM_JIS_B6",                  "jis_b6_1280.00x1820.00mm"                   },
      { "FORM_JIS_B7",                  "jis_b7_910.00x1280.00mm"                    },
      { "FORM_JIS_B8",                  "jis_b8_640.00x910.00mm"                     },
      { "FORM_JIS_B9",                  "jis_b9_450.00x640.00mm"                     },
      { "FORM_JIS_EXEC",                "jis_exec_2160.00x3300.00mm"                 },
      { "FORM_JUURO_KU_KAI",            "om_juuro-ku-kai_1980.00x2750.00mm"          },
      { "FORM_KAHU_ENVELOPE",           "jpn_kahu_2400.00x3221.00mm"                 },
      { "FORM_KAKU2_ENVELOPE",          "jpn_kaku2_2400.00x3320.00mm"                },
      { "FORM_LEDGER",                  "na_ledger_17.00x11.00in"                    },
      { "FORM_LEGAL",                   "na_legal_8.50x14.00in"                      },
      { "FORM_LEGAL_EXTRA",             "na_legal-extra_9.50x15.00in"                },
      { "FORM_LETTER",                  "na_letter_8.50x11.00in"                     },
      { "FORM_LETTER_EXTRA",            "na_letter-extra_9.50x12.00in"               },
      { "FORM_LETTER_PLUS",             "na_letter-plus_8.50x12.69in"                },
      { "FORM_LETTER_WIDE",             "na_letter-wide_8.99x13.30in"                },
      { "FORM_MONARCH_ENVELOPE",        "na_monarch_3.88x7.50in"                     },
      { "FORM_NUMBER_10_ENVELOPE",      "na_number-10_4.12x9.50in"                   },
      { "FORM_NUMBER_11_ENVELOPE",      "na_number-11_4.50x10.38in"                  },
      { "FORM_NUMBER_12_ENVELOPE",      "na_number-12_4.75x11.00in"                  },
      { "FORM_NUMBER_14_ENVELOPE",      "na_number-14_5.00x11.50in"                  },
      { "FORM_NUMBER_9_ENVELOPE",       "na_number-9_3.88x8.88in"                    },
      { "FORM_OUFUKU_CARD",             "jpn_oufuku_1480.00x2000.00mm"               },
      { "FORM_PANORAMIC",               "na_panoramic_2100.00x5940.00mm"             },
      { "FORM_PA_KAI",                  "om_pa-kai_2670.00x3890.00mm"                },
      { "FORM_PERSONAL_ENVELOPE",       "na_personal_3.62x6.50in"                    },
      { "FORM_PHOTO_100_150",           "na_photo-100x150_1000.00x1500.00mm"         },
      { "FORM_PHOTO_200_300",           "na_photo-200x300_2000.00x3000.00mm"         },
      { "FORM_PHOTO_4_6",               "na_photo-4x6_4.00x6.00in"                   },
      { "FORM_PLOTTER_SIZE_A",          "na_plotter-size-a_8.50x11.00in"             },
      { "FORM_PLOTTER_SIZE_B",          "na_plotter-size-b_11.00x17.00in"            },
      { "FORM_PLOTTER_SIZE_C",          "na_plotter-size-c_17.00x22.00in"            },
      { "FORM_PLOTTER_SIZE_D",          "na_plotter-size-d_22.00x34.00in"            },
      { "FORM_PLOTTER_SIZE_E",          "na_plotter-size-e_34.00x44.00in"            },
      { "FORM_PLOTTER_SIZE_F",          "na_plotter-size-f_44.00x68.00in"            },
      { "FORM_POSTCARD",                "na_postcard_3.93x5.79in"                    },
      { "FORM_POSTFIX_ENVELOPE",        "om_postfix_1140.00x2290.00mm"               },
      { "FORM_PRC10_ENVELOPE",          "prc_10_3240.00x4580.00mm"                   },
      { "FORM_PRC1_ENVELOPE",           "prc_1_1020.00x1650.00mm"                    },
      { "FORM_PRC2_ENVELOPE",           "prc_2_1020.00x1760.00mm"                    },
      { "FORM_PRC3_ENVELOPE",           "prc_3_1250.00x1760.00mm"                    },
      { "FORM_PRC4_ENVELOPE",           "prc_4_1100.00x2080.00mm"                    },
      { "FORM_PRC5_ENVELOPE",           "prc_5_1100.00x2200.00mm"                    },
      { "FORM_PRC6_ENVELOPE",           "prc_6_1200.00x3200.00mm"                    },
      { "FORM_PRC7_ENVELOPE",           "prc_7_1600.00x2300.00mm"                    },
      { "FORM_PRC8_ENVELOPE",           "prc_8_1200.00x3090.00mm"                    },
      { "FORM_PRC9_ENVELOPE",           "prc_9_2290.00x3240.00mm"                    },
      { "FORM_PRC_16K",                 "prc_16k_1460.00x2150.00mm"                  },
      { "FORM_PRC_32K",                 "prc_32k_970.00x1510.00mm"                   },
      { "FORM_QUARTO",                  "na_quarto_8.50x10.83in"                     },
      { "FORM_RA0",                     "iso_ra0_8600.00x12200.00mm"                 },
      { "FORM_RA1",                     "iso_ra1_6100.00x8600.00mm"                  },
      { "FORM_RA2",                     "iso_ra2_4300.00x6100.00mm"                  },
      { "FORM_ROC_16K",                 "roc_16k_1968.50x2730.50mm"                  },
      { "FORM_ROC_8K",                  "roc_8k_2730.50x3937.00mm"                   },
      { "FORM_SHIPPING_LABELS",         "na_shipping-labels_2.13x3.98in"             },
      { "FORM_SRA0",                    "iso_sra0_9000.00x12800.00mm"                },
      { "FORM_SRA1",                    "iso_sra1_6400.00x9000.00mm"                 },
      { "FORM_SRA2",                    "iso_sra2_4500.00x6400.00mm"                 },
      { "FORM_STANDARD_LABELS_CLEAR",   "na_standard-labels-clear_1.10x3.50in"       },
      { "FORM_STANDARD_LABELS_WHITE",   "na_standard-labels-white_1.10x3.50in"       },
      { "FORM_STATEMENT",               "na_statement_5.50x8.50in"                   },
      { "FORM_SUPER_A",                 "na_super-a_2270.76x3556.00mm"               },
      { "FORM_SUPER_A3_B",              "na_super-a3-b_3289.20x4830.20mm"            },
      { "FORM_SUPER_B",                 "na_super-b_3302.00x4826.00mm"               },
      { "FORM_TABLOID",                 "na_tabloid_11.00x17.00in"                   },
      { "FORM_UNIVERSAL",               "na_universal_11.69x17.00in"                 },
      { "FORM_US_A_ARCHITECTURAL",      "na_arch-a_9.00x12.00in"                     },
      { "FORM_US_B_ARCHITECTURAL",      "na_arch-b_12.00x18.00in"                    },
      { "FORM_US_C_ARCHITECTURAL",      "na_arch-c_18.00x24.00in"                    },
      { "FORM_US_D_ARCHITECTURAL",      "na_arch-d_24.00x36.00in"                    },
      { "FORM_US_E_ARCHITECTURAL",      "na_arch-e_36.00x48.00in"                    },
      { "FORM_US_FANFOLD",              "na_fanfold-us_14.88x11.00in"                },
      { "FORM_WIDE",                    "na_wide-format_13.58x11.00in"               },
      { "FORM_YOU4_ENVELOPE",           "jpn_you4_1050.00x2350.00mm"                 },
   };

   if (0 == strncmp ((char *)pszFrom, "FORM_ROLL_69_5MM", 16))
   {
      static char  achHack[64];
      const char  *pszComma     = 0;
      float        flY          = 0.0;
      bool         fSuccess     = false;

      pszComma = strchr ((char *)pszFrom, ',');

      if (pszComma)
      {
         if (1 == sscanf (pszComma + 1, "%f", &flY))
         {
            fSuccess = true;
         }
      }
      else
      {
         flY = 25400.0;

         fSuccess = true;
      }

      if (fSuccess)
      {
         flY /= 1000.0;

         sprintf (achHack, "na_roll-69.5_69.50x%1.2fmm", flY);

         return achHack;
      }
   }
   else if (0 == strncmp ((char *)pszFrom, "FORM_ROLL_76_2MM", 16))
   {
      static char  achHack[64];
      const char  *pszComma     = 0;
      float        flY          = 0.0;
      bool         fSuccess     = false;

      pszComma = strchr ((char *)pszFrom, ',');

      if (pszComma)
      {
         if (1 == sscanf (pszComma + 1, "%f", &flY))
         {
            fSuccess = true;
         }
      }
      else
      {
         flY = 25400.0;

         fSuccess = true;
      }

      if (fSuccess)
      {
         flY /= 1000.0;

         sprintf (achHack, "na_roll-76.2_76.20x%1.2fmm", flY);

         return achHack;
      }
   }

   int iLow  = 0;
   int iMid  = (int)dimof (apszNames) / 2;
   int iHigh = (int)dimof (apszNames) - 1;
   int iResult;

   while (iLow <= iHigh)
   {
      iResult = strcmp ((char *)pszFrom, apszNames[iMid].pszFrom);

      if (0 == iResult)
      {
         return apszNames[iMid].pszTo;
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

PSZCRO
mapTrayName (PSZCRO pszFrom)
{
   if (0 == XMLStrcmp (pszFrom, "TRAY_NONE"))
   {
      return "None";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_AUTO"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_USE_PRINTER_SETTING"))
   {
      return "PanelSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_DEPEND_ON_PANEL"))
   {
      return "PanelSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_AUTO_FEEDER"))
   {
      return "Continuous";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_AUTOSWITCH"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY"))
   {
      return "Tray";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_UPPER_TRAY"))
   {
      return "Top";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_LOWER_TRAY"))
   {
      return "Bottom";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_MULTI_TRAY"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE"))
   {
      return "Tray";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_UPPER_CASSETTE"))
   {
      return "Top";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_LOWER_CASSETTE"))
   {
      return "Bottom";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_MULTI_CASSETTE"))
   {
      return "AutoSelect";
   }
/*
"IBM/IBM 5577-H02 Connections.xml" 100 23:      <connectionTray>TRAY_OPTION_CASSETTE</connectionTray>
"IBM/IBM 5577-H02 Trays.xml" 41 13:      <name>TRAY_OPTION_CASSETTE</name>
"IBM/IBM 5584-H02 Connections.xml" 100 23:      <connectionTray>TRAY_OPTION_CASSETTE</connectionTray>
"IBM/IBM 5584-H02 Trays.xml" 41 13:      <name>TRAY_OPTION_CASSETTE</name>
"HP LaserJet/HP LaserJet 4V_4MV Trays.xml" 46 13:      <name>TRAY_OPTION_CASSETTE</name>
"HP LaserJet/HP LaserJet 4_4M Trays.xml" 46 13:      <name>TRAY_OPTION_CASSETTE</name>
"HP LaserJet/HP LaserJet 5Si_5Si Mx_5Si Mopier Trays.xml" 46 13:      <name>TRAY_OPTION_CASSETTE</name>
"HP LaserJet/HP LaserJet 5_5M Trays.xml" 46 13:      <name>TRAY_OPTION_CASSETTE</name>
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_OPTION_CASSETTE"))
   {
      return "BypassTray";
   }
/*
Too many to list!
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_MANUAL_FEEDER"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_MANUAL_ENVELOPE"))
   {
      return "Envelope";
   }
/*
None!
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_PAPER_FEEDER"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ENVELOPE_FEEDER"))
   {
      return "Envelope";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CSF"))
   {
      return "Continuous";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_FRONT_CONTINUOUS"))
   {
      return "Front";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_REAR_CONTINUOUS"))
   {
      return "Rear";
   }
/*
Too many to list!
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_SINGLE_SHEET"))
   {
      return "AutoSelect";
   }
/*
"HP DeskJet/HP DeskJet Portable Connections.xml" 52 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet Portable Connections.xml" 58 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet Portable Connections.xml" 64 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet Portable Connections.xml" 70 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 310 Connections.xml" 52 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 310 Connections.xml" 58 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 310 Connections.xml" 64 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 310 Connections.xml" 70 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 310.xml" 47 13:      <tray>TRAY_SHEET_FEEDER</tray>
"HP DeskJet/HP DeskJet 320.xml" 47 13:      <tray>TRAY_SHEET_FEEDER</tray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 76 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 82 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 88 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 94 23:      <connectionTray>TRAY_SHEET_FEEDER</connectionTray>
"HP DeskJet/HP DeskJet 340 Trays.xml" 36 13:      <name>TRAY_SHEET_FEEDER</name>
"HP DeskJet/HP DeskJet Portable Trays.xml" 31 13:      <name>TRAY_SHEET_FEEDER</name>
"HP DeskJet/HP DeskJet Portable.xml" 45 13:      <tray>TRAY_SHEET_FEEDER</tray>
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_SHEET_FEEDER"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_BIN_1"))
   {
      return "Tray-1";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_BIN_2"))
   {
      return "Tray-2";
   }
/*
"HP DeskJet/HP DeskJet 340 Connections.xml" 28 23:      <connectionTray>TRAY_PORTABLE_SHEET</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 34 23:      <connectionTray>TRAY_PORTABLE_SHEET</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 40 23:      <connectionTray>TRAY_PORTABLE_SHEET</connectionTray>
"HP DeskJet/HP DeskJet 340 Connections.xml" 46 23:      <connectionTray>TRAY_PORTABLE_SHEET</connectionTray>
"HP DeskJet/HP DeskJet 340 Trays.xml" 26 13:      <name>TRAY_PORTABLE_SHEET</name>
"HP DeskJet/HP DeskJet 340.xml" 47 13:      <tray>TRAY_PORTABLE_SHEET</tray>
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_PORTABLE_SHEET"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_1"))
   {
      return "Tray-1";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_2"))
   {
      return "Tray-2";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_3"))
   {
      return "Tray-3";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_4"))
   {
      return "Tray-4";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_5"))
   {
      return "Tray-5";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRAY_6"))
   {
      return "Tray-6";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE1"))
   {
      return "Tray-1";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE2"))
   {
      return "Tray-2";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE3"))
   {
      return "Tray-3";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE4"))
   {
      return "Tray-4";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_CASSETTE5"))
   {
      return "Tray-5";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_FRONT_TRAY"))
   {
      return "Front";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_OPTION_MULTI_SF"))
   {
      return "AutoSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_MULTI_FEEDER"))
   {
      return "AutoSelect";
   }
/*
None!
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_PAPER_DECK"))
   {
      return "Tray";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_TRACTOR_UNIT"))
   {
      return "Continuous";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ROLL_1"))
   {
      return "Roll-1";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ROLL_2"))
   {
      return "Roll-2";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ROLL_3"))
   {
      return "Roll-3";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ROLL_4"))
   {
      return "Roll-4";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ROLL_5"))
   {
      return "Roll-5";
   }
/*
"IBM/IBM Infoprint 20 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 20 Connections.xml" 82 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 20 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Infoprint 21 Connections.xml" 118 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 124 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 130 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 136 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 142 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 148 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 154 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 160 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 166 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 172 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 178 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 184 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 190 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 196 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 202 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 838 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 844 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 850 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 856 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 862 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 868 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 874 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 880 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 886 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 892 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 898 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 904 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 910 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 916 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Connections.xml" 922 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 32 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 32 Connections.xml" 76 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 21 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Infoprint 40 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 40 Connections.xml" 70 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 32 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Infoprint 70 Connections.xml" 136 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 142 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 148 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 154 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 160 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 166 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 172 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 178 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 184 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 190 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 196 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 202 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 208 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 214 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 220 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 226 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 232 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 238 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 892 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 898 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 904 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 910 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 916 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 922 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 928 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 934 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 940 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 946 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 952 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 958 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 964 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 970 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 976 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 982 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 988 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 70 Connections.xml" 994 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Infoprint 40 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Infoprint 70 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Network Printer 12 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Network Printer 12 Connections.xml" 70 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Network Printer 12 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Network Printer 17 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Network Printer 17 Connections.xml" 76 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Network Printer 17 Trays.xml" 31 13:      <name>TRAY_AUXILIARY_TRAY</name>
"IBM/IBM Network Printer 24 Connections.xml" 34 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
"IBM/IBM Network Printer 24 Connections.xml" 76 23:      <connectionTray>TRAY_AUXILIARY_TRAY</connectionTray>
*/
   else if (0 == XMLStrcmp (pszFrom, "TRAY_AUXILIARY_TRAY"))
   {
      return "BypassTray";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_LARGE_CAPACITY_TRAY"))
   {
      return "LargeCapacity";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_HIGH_CAPACITY_FEEDER"))
   {
      return "LargeCapacity";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_DEPEND_ON_PANEL"))
   {
      return "PanelSelect";
   }
   else if (0 == XMLStrcmp (pszFrom, "TRAY_ZERO_MARGINS"))
   {
      return "Tray-1";
   }

   return 0;
}

PSZCRO
mapResolutionName (PSZCRO pszFrom)
{
   if (0 == XMLStrcmp (pszFrom, "RESOLUTION_60_X_60"))
   {
      return "60x60";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_60_X_72"))
   {
      return "60x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_60_X_180"))
   {
      return "60x180";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_60_X_360"))
   {
      return "60x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_72_X_72"))
   {
      return "72x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_75_X_75"))
   {
      return "75x75";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_80_X_60"))
   {
      return "80x60";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_80_X_72"))
   {
      return "80x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_90_X_180"))
   {
      return "90x180";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_90_X_360"))
   {
      return "90x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_90_X_60"))
   {
      return "90x60";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_90_X_72"))
   {
      return "90x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_90_X_90"))
   {
      return "90x90";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_100_X_100"))
   {
      return "100x100";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_60"))
   {
      return "120x60";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_72"))
   {
      return "120x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_120"))
   {
      return "120x120";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_144"))
   {
      return "120x144";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_180"))
   {
      return "120x180";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_120_X_360"))
   {
      return "120x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_144_X_72"))
   {
      return "144x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_150_X_150"))
   {
      return "150x150";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_180_X_180"))
   {
      return "180x180";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_180_X_360"))
   {
      return "180x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_200_X_200"))
   {
      return "200x200";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_240_X_60"))
   {
      return "240x60";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_240_X_72"))
   {
      return "240x72";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_240_X_144"))
   {
      return "240x144";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_240_X_240"))
   {
      return "240x240";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_300_X_300"))
   {
      return "300x300";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_360_X_180"))
   {
      return "360x180";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_360_X_360"))
   {
      return "360x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_360_X_720"))
   {
      return "360x720";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_400_X_400"))
   {
      return "400x400";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_600_X_300"))
   {
      return "600x300";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_600_X_600"))
   {
      return "600x600";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_720_X_360"))
   {
      return "720x360";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_720_X_720"))
   {
      return "720x720";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_1200_X_600"))
   {
      return "1200x600";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_1200_X_1200"))
   {
      return "1200x1200";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_1440_X_720"))
   {
      return "1440x720";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_2880_X_720"))
   {
      return "2880x720";
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_DRAFT"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_FINE"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_GROUP_3"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_GROUP_4"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_HIGH"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_LOW"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_MEDIUM"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_NORMAL"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_PHOTO_QUALITY"))
   {
      return 0;
   }
   else if (0 == XMLStrcmp (pszFrom, "RESOLUTION_PRESENTATION"))
   {
      return 0;
   }

   return 0;
}

PSZCRO
mapOrientationName (PSZCRO pszFrom)
{
   if (0 == XMLStrcmp (pszFrom, "ORIENTATION_PORTRAIT"))
   {
      return "Portrait";
   }
   else if (0 == XMLStrcmp (pszFrom, "ORIENTATION_LANDSCAPE"))
   {
      return "Landscape";
   }
   else if (0 == XMLStrcmp (pszFrom, "ORIENTATION_REVERSE_PORTRAIT"))
   {
      return "ReversePortrait";
   }
   else if (0 == XMLStrcmp (pszFrom, "ORIENTATION_REVERSE_LANDSCAPE"))
   {
      return "ReverseLandscape";
   }

   return 0;
}

bool
processForms (XmlNodePtr nodeForms,
              bool       fUpdate)
{
   XmlNodePtr nodeForm    = 0;
   XmlNodePtr nodeElement = 0;
   bool       fRet        = true;

   while (nodeForms)
   {
      nodeForm = XMLFirstNode (XMLGetChildrenNode (nodeForms));

      while (nodeForm)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeForm));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (0 == XMLStrcmp (XMLGetName (nodeElement), "name"))
            {
               XmlNodePtr nodeNew        = 0;
               PSZRO      pszNameText    = 0;
               PSZRO      pszNewNameText = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapFormName (pszNameText);
               if (!pszNewNameText)
               {
                  std::cerr << "Error:  Cannot map \"" << pszNameText << "\" to anything!" << std::endl;
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             "omniName",
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeForm = XMLNextNode (nodeForm);
      }

      nodeForms = XMLNextNode (nodeForms);
   }

   return fRet;
}

bool
processTrays (XmlNodePtr nodeTrays,
              bool       fUpdate)
{
   XmlNodePtr nodeTray    = 0;
   XmlNodePtr nodeElement = 0;
   bool       fRet        = true;

   while (nodeTrays)
   {
      nodeTray = XMLFirstNode (XMLGetChildrenNode (nodeTrays));

      while (nodeTray)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeTray));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (0 == XMLStrcmp (XMLGetName (nodeElement), "name"))
            {
               XmlNodePtr nodeNew        = 0;
               PSZRO      pszNameText    = 0;
               PSZRO      pszNewNameText = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapTrayName (pszNameText);
               if (!pszNewNameText)
               {
                  std::cerr << "Error:  Cannot map \"" << pszNameText << "\" to anything!" << std::endl;
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             "omniName",
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeTray = XMLNextNode (nodeTray);
      }

      nodeTrays = XMLNextNode (nodeTrays);
   }

   return fRet;
}

bool
processResolutions (XmlNodePtr nodeResolutions,
                    bool       fUpdate)
{
   XmlNodePtr nodeResolution = 0;
   XmlNodePtr nodeElement    = 0;
   bool       fRet           = true;

   while (nodeResolutions)
   {
      nodeResolution = XMLFirstNode (XMLGetChildrenNode (nodeResolutions));

      while (nodeResolution)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeResolution));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (0 == XMLStrcmp (XMLGetName (nodeElement), "name"))
            {
               char       achResolution[64];
               XmlNodePtr nodeNew        = 0;
               PSZRO      pszNameText    = 0;
               PSZRO      pszNewNameText = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapResolutionName (pszNameText);

               if (  !pszNewNameText
                  || !*pszNewNameText
                  )
               {
                  XmlNodePtr nodeXRes = 0;
                  XmlNodePtr nodeYRes = 0;

                  achResolution[0] = '\0';

                  nodeXRes = XMLNextNode (nodeElement);
                  if (nodeXRes)
                  {
                     nodeYRes = XMLNextNode (nodeXRes);
                  }

                  if (  nodeXRes
                     && 0 == XMLStrcmp (XMLGetName (nodeXRes), "xRes")
                     && nodeYRes
                     && 0 == XMLStrcmp (XMLGetName (nodeYRes), "yRes")
                     )
                  {
                     PSZRO pszXRes = 0;
                     PSZRO pszYRes = 0;

                     pszXRes = XMLNodeListGetString (XMLGetDocNode (nodeXRes),
                                                     XMLGetChildrenNode (nodeXRes),
                                                     1);

                     pszYRes = XMLNodeListGetString (XMLGetDocNode (nodeYRes),
                                                     XMLGetChildrenNode (nodeYRes),
                                                     1);

/////////////////////if (vfDebug) std::cerr << "pszXRes       = " << (pszXRes ? (char *)pszXRes : "") << std::dec << std::endl;
/////////////////////if (vfDebug) std::cerr << "pszYRes       = " << (pszYRes ? (char *)pszYRes : "") << std::dec << std::endl;

                     if (  pszXRes
                        && pszYRes
                        )
                     {
                        sprintf (achResolution, "%sx%s", pszXRes, pszYRes);
                     }

                     if (pszXRes)
                     {
                        XMLFree ((void *)pszXRes);
                     }
                     if (pszYRes)
                     {
                        XMLFree ((void *)pszYRes);
                     }
                  }

                  if (!achResolution[0])
                  {
                     std::cerr << "Error:  Cannot map \"" << pszNameText << "\" to anything!" << std::endl;
                  }
                  else
                  {
                     pszNewNameText = achResolution;
                  }

//////////////////std::cerr << "nodeXRes      = " << std::hex << (int)nodeXRes << std::dec << std::endl;
//////////////////std::cerr << "nodeYRes      = " << std::hex << (int)nodeYRes << std::dec << std::endl;
//////////////////std::cerr << "achResolution = " << achResolution << std::dec << std::endl;
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             "omniName",
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeResolution = XMLNextNode (nodeResolution);
      }

      nodeResolutions = XMLNextNode (nodeResolutions);
   }

   return fRet;
}

bool
processOrientations (XmlNodePtr nodeOrientations,
                     bool       fUpdate)
{
   XmlNodePtr nodeOrientation = 0;
   XmlNodePtr nodeElement     = 0;
   bool       fRet            = true;

   while (nodeOrientations)
   {
      nodeOrientation = XMLFirstNode (XMLGetChildrenNode (nodeOrientations));

      while (nodeOrientation)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeOrientation));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (0 == XMLStrcmp (XMLGetName (nodeElement), "name"))
            {
               XmlNodePtr nodeNew        = 0;
               PSZRO      pszNameText    = 0;
               PSZRO      pszNewNameText = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapOrientationName (pszNameText);
               if (!pszNewNameText)
               {
                  std::cerr << "Error:  Cannot map \"" << pszNameText << "\" to anything!" << std::endl;
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             "omniName",
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeOrientation = XMLNextNode (nodeOrientation);
      }

      nodeOrientations = XMLNextNode (nodeOrientations);
   }

   return fRet;
}

PSZCRO
convertResolutionFromResolution (XmlDocPtr      docResolutions,
                                 PSZCRO         pszOldResolutionName)
{
   XmlNodePtr     nodeRoot             = 0;
   PSZRO          pszNewResolutionName = 0;

   if (!docResolutions)
   {
      return 0;
   }

   nodeRoot = XMLFirstNode (XMLDocGetRootElement (docResolutions));

   XmlNodePtr nodeResolutions = nodeRoot;
   XmlNodePtr nodeResolution  = 0;
   XmlNodePtr nodeElement     = 0;

   while (nodeResolutions)
   {
      nodeResolution = XMLFirstNode (XMLGetChildrenNode (nodeResolutions));

      while (nodeResolution)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeResolution));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (  0 == XMLStrcmp (XMLGetName (nodeElement), "name")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "omniName")
               )
            {
               char  achResolution[64];
               PSZRO pszNameText       = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               if (0 == XMLStrcmp (pszNameText, pszOldResolutionName))
               {
                  XmlNodePtr nodeXRes = 0;
                  XmlNodePtr nodeYRes = 0;

                  achResolution[0] = '\0';

                  nodeXRes = XMLNextNode (nodeElement);
                  if (nodeXRes)
                  {
                     nodeYRes = XMLNextNode (nodeXRes);
                  }

                  if (  nodeXRes
                     && 0 == XMLStrcmp (XMLGetName (nodeXRes), "xRes")
                     && nodeYRes
                     && 0 == XMLStrcmp (XMLGetName (nodeYRes), "yRes")
                     )
                  {
                     PSZRO pszXRes = 0;
                     PSZRO pszYRes = 0;

                     pszXRes = XMLNodeListGetString (XMLGetDocNode (nodeXRes),
                                                     XMLGetChildrenNode (nodeXRes),
                                                     1);

                     pszYRes = XMLNodeListGetString (XMLGetDocNode (nodeYRes),
                                                     XMLGetChildrenNode (nodeYRes),
                                                     1);

                     if (vfDebug) std::cerr << "pszXRes       = " << (pszXRes ? (char *)pszXRes : "") << std::dec << std::endl;
                     if (vfDebug) std::cerr << "pszYRes       = " << (pszYRes ? (char *)pszYRes : "") << std::dec << std::endl;

                     if (  pszXRes
                        && pszYRes
                        )
                     {
                        char *pszRet = 0;

                        sprintf (achResolution, "%sx%s", pszXRes, pszYRes);

                        if (vfDumpElements) std::cout << pszNameText << "->" << achResolution << std::endl;

                        pszRet = (char *)malloc (strlen (achResolution) + 1);
                        if (pszRet)
                        {
                           strcpy (pszRet, achResolution);

                           pszNewResolutionName = pszRet;
                        }
                     }

                     if (pszXRes)
                     {
                        XMLFree ((void *)pszXRes);
                     }
                     if (pszYRes)
                     {
                        XMLFree ((void *)pszYRes);
                     }
                  }

//////////////////std::cerr << "nodeXRes      = " << std::hex << (int)nodeXRes << std::dec << std::endl;
//////////////////std::cerr << "nodeYRes      = " << std::hex << (int)nodeYRes << std::dec << std::endl;
//////////////////std::cerr << "achResolution = " << achResolution << std::dec << std::endl;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeResolution = XMLNextNode (nodeResolution);
      }

      nodeResolutions = XMLNextNode (nodeResolutions);
   }

   return pszNewResolutionName;
}

PSZCRO
convertResolutionFromDevice (XmlNodePtr     nodeDevice,
                             XmlDocPtr     *pdocResolutions,
                             PSZCRO         pszOldResolutionName)
{
   if (  pdocResolutions
      && *pdocResolutions
      )
   {
      return convertResolutionFromResolution (*pdocResolutions,
                                              pszOldResolutionName);
   }

   XmlNodePtr nodeAttributes       = 0;
   PSZRO      pszNewResolutionName = 0;

   nodeAttributes = XMLFirstNode (XMLGetChildrenNode (nodeDevice));

   while (nodeAttributes)
   {
      if (  0 == XMLStrcmp (XMLGetName (nodeAttributes), "Has")
         || 0 == XMLStrcmp (XMLGetName (nodeAttributes), "Uses")
         )
      {
         XmlDocPtr      doc              = 0;
         XmlNodePtr     nodeRoot         = 0;
         PSZRO          pszResolutionXML = 0;

         pszResolutionXML = XMLNodeListGetString (XMLGetDocNode (nodeAttributes),
                                                  XMLGetChildrenNode (nodeAttributes),
                                                  1);

         if (vfDebug) std::cerr << "pszResolutionXML = " << (pszResolutionXML ? (char *)pszResolutionXML : "") << std::endl;

         if (!pszResolutionXML)
         {
            break;
         }

         doc = XMLParseFile ((char *)pszResolutionXML);

         if (!doc)
         {
            break;
         }

         nodeRoot = XMLFirstNode (XMLDocGetRootElement (doc));
         if (  nodeRoot
            && 0 == XMLStrcmp ("deviceResolutions", XMLGetName (nodeRoot))
            )
         {
            if (pdocResolutions)
            {
               *pdocResolutions = doc;
            }

            pszNewResolutionName = convertResolutionFromResolution (doc,
                                                                    pszOldResolutionName);
         }
         else
         {
            if (doc)
            {
               XMLFreeDoc (doc);
            }
         }

         if (pszResolutionXML)
         {
            XMLFree ((void *)pszResolutionXML);
         }
      }

      nodeAttributes = XMLNextNode (nodeAttributes);

      if (vfDebug) std::cerr << "nodeAttributes = " << std::hex<< (int)nodeAttributes << std::dec << std::endl;
   }

   return pszNewResolutionName;
}

PSZCRO
convertResolutionFromGamma (const char     *pszGammaFileName,
                            XmlDocPtr      *pdocResolutions,
                            PSZCRO          pszOldResolutionName)
{
   PSZRO      pszNewResolution = 0;
   XmlDocPtr  doc              = 0;
   XmlNodePtr nodeRoot         = 0;

   if (  pdocResolutions
      && *pdocResolutions
      )
   {
      return convertResolutionFromResolution (*pdocResolutions,
                                              pszOldResolutionName);
   }
   else
   {
      std::ostringstream oss;
      glob_t             globbuf;
      int                rc;

      // Call glob
      memset (&globbuf, 0, sizeof (globbuf));

      oss << "*.xml"
          << std::ends;

      rc = glob (oss.str ().c_str (), 0, NULL, &globbuf);

      if (  rc
         || 0 == globbuf.gl_pathc
         )
      {
         return 0;
      }

      // Call succeded
      for (int i = 0; i < (int)globbuf.gl_pathc && !pszNewResolution; i++)
      {
         doc = XMLParseFile (globbuf.gl_pathv[i]);
         if (!doc)
         {
            continue;
         }

         nodeRoot = XMLFirstNode (XMLDocGetRootElement (doc));
         if (!nodeRoot)
         {
            continue;
         }

         if (0 == XMLStrcmp ("Device", XMLGetName (nodeRoot)))
         {
            XmlNodePtr nodeAttributes = 0;

            nodeAttributes = XMLFirstNode (XMLGetChildrenNode (nodeRoot));

            while (  nodeAttributes
                  && !pszNewResolution
                  )
            {
               if (  0 == XMLStrcmp (XMLGetName (nodeAttributes), "Has")
                  || 0 == XMLStrcmp (XMLGetName (nodeAttributes), "Uses")
                  )
               {
                  PSZRO pszFileXML = 0;

                  pszFileXML = XMLNodeListGetString (XMLGetDocNode (nodeAttributes),
                                                     XMLGetChildrenNode (nodeAttributes),
                                                     1);

                  if (pszFileXML)
                  {
                     if (vfDebug) std::cerr << "! " << pszFileXML << " = " << pszGammaFileName << std::endl;

                     if (0 == XMLStrcmp (pszGammaFileName, pszFileXML))
                     {
                        pszNewResolution = convertResolutionFromDevice (nodeRoot,
                                                                        pdocResolutions,
                                                                        pszOldResolutionName);
                     }
                     else
                     {
                        XMLFree ((void *)pszFileXML);
                     }
                  }
               }

               nodeAttributes = XMLNextNode (nodeAttributes);
            }
         }

         XMLFreeDoc (doc);
      }

      globfree (&globbuf);
   }

   return pszNewResolution;
}

bool
processGammas (XmlNodePtr  nodeGammas,
               bool        fUpdate,
               const char *pszGammaFileName)
{
   XmlNodePtr     nodeGamma      = 0;
   XmlNodePtr     nodeElement    = 0;
   XmlDocPtr      docResolutions = 0;
   bool           fRet           = true;

   while (nodeGammas)
   {
      nodeGamma = XMLFirstNode (XMLGetChildrenNode (nodeGammas));

      while (nodeGamma)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeGamma));

         while (nodeElement)
         {
            if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;

            if (  0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableResolution")
//             || 0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableMedia")
               )
            {
               XmlNodePtr  nodeNew          = 0;
               PSZRO       pszNameText      = 0;
               PSZRO       pszNewNameText   = 0;
               const char *pszOldName       = 0;
               bool        fFreeNewNameText = false;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               if (0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableResolution"))
               {
                  pszNewNameText = mapResolutionName (pszNameText);
                  pszOldName     = "omniResolution";

                  if (  !pszNewNameText
                     || !*pszNewNameText
                     )
                  {
                     pszNewNameText   = convertResolutionFromGamma (pszGammaFileName,
                                                                    &docResolutions,
                                                                    pszNameText);
                     fFreeNewNameText = true;
                  }
               }
//             else if (0 == XMLStrcmp (XMLGetName (nodeElement), "gammaTableMedia"))
//             {
//                pszNewNameText = mapMediaName (pszNameText);
//                pszOldName     = "omniMedia";
//             }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             pszOldName,
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
               if (  fFreeNewNameText
                  && pszNewNameText
                  )
               {
                  XMLFree ((void *)pszNewNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeGamma = XMLNextNode (nodeGamma);
      }

      nodeGammas = XMLNextNode (nodeGammas);
   }

   if (docResolutions)
   {
      XMLFreeDoc (docResolutions);
   }

   return fRet;
}

bool
processConnections (XmlNodePtr  nodeConnections,
                    bool        fUpdate,
                    const char *pszFileName)
{
   XmlNodePtr nodeConnection = 0;
   XmlNodePtr nodeElement    = 0;
   bool       fRet           = true;

   while (nodeConnections)
   {
      nodeConnection = XMLFirstNode (XMLGetChildrenNode (nodeConnections));

      while (nodeConnection)
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeConnection));

         while (nodeElement)
         {
            if (  0 == XMLStrcmp (XMLGetName (nodeElement), "connectionForm")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "connectionTray")
//             || 0 == XMLStrcmp (XMLGetName (nodeElement), "connectionMedia")
               )
            {
               XmlNodePtr  nodeNew          = 0;
               PSZRO       pszNameText      = 0;
               PSZRO       pszNewNameText   = 0;
               const char *pszOldName       = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               if (0 == XMLStrcmp (XMLGetName (nodeElement), "connectionForm"))
               {
                  pszNewNameText = mapFormName (pszNameText);
                  pszOldName     = "omniForm";
               }
               else if (0 == XMLStrcmp (XMLGetName (nodeElement), "connectionTray"))
               {
                  pszNewNameText = mapTrayName (pszNameText);
                  pszOldName     = "omniTray";

                  if (!pszNewNameText)
                  {
                     std::cerr << "Error:  Cannot map \"" << pszNameText << "\" to anything!" << std::endl;
                  }
               }
///////////////else if (0 == XMLStrcmp (XMLGetName (nodeElement), "connectionMedia"))
///////////////{
///////////////   pszNewNameText = mapMediaName (pszNameText);
///////////////   pszOldName     = "omniMedia";
///////////////}

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             pszOldName,
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }

         nodeConnection = XMLNextNode (nodeConnection);
      }

      nodeConnections = XMLNextNode (nodeConnections);
   }

   return fRet;
}

bool
processDevice (XmlNodePtr nodeDevice,
               bool       fUpdate)
{
   XmlNodePtr nodeAttributes = 0;
   XmlNodePtr nodeElement    = 0;
   bool       fRet           = true;

   nodeAttributes = XMLFirstNode (XMLGetChildrenNode (nodeDevice));

   while (nodeAttributes)
   {
      if (0 == XMLStrcmp (XMLGetName (nodeAttributes), "DefaultJobProperties"))
      {
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeAttributes));

         while (nodeElement)
         {
            if (  0 == XMLStrcmp (XMLGetName (nodeElement), "orientation")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "form")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "tray")
//             || 0 == XMLStrcmp (XMLGetName (nodeElement), "media")
               || 0 == XMLStrcmp (XMLGetName (nodeElement), "resolution")
               )
            {
               XmlNodePtr  nodeNew          = 0;
               PSZRO       pszNameText      = 0;
               PSZRO       pszNewNameText   = 0;
               const char *pszOldName       = 0;
               bool        fFreeNewNameText = false;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               if (0 == XMLStrcmp (XMLGetName (nodeElement), "orientation"))
               {
                  pszNewNameText = mapOrientationName (pszNameText);
                  pszOldName     = "omniOrientation";
               }
               if (0 == XMLStrcmp (XMLGetName (nodeElement), "form"))
               {
                  pszNewNameText = mapFormName (pszNameText);
                  pszOldName     = "omniForm";
               }
               if (0 == XMLStrcmp (XMLGetName (nodeElement), "tray"))
               {
                  pszNewNameText = mapTrayName (pszNameText);
                  pszOldName     = "omniTray";
               }
//             if (0 == XMLStrcmp (XMLGetName (nodeElement), "media"))
//             {
//                pszNewNameText = mapMediaName (pszNameText);
//                pszOldName     = "omniMedia";
//             }
               if (0 == XMLStrcmp (XMLGetName (nodeElement), "resolution"))
               {
                  pszNewNameText = mapResolutionName (pszNameText);
                  pszOldName     = "omniResolution";

                  if (  !pszNewNameText
                     || !*pszNewNameText
                     )
                  {
                     pszNewNameText   = convertResolutionFromDevice (nodeDevice, 0, pszNameText);
                     fFreeNewNameText = true;
                  }
               }

///////////////if (vfDebug) std::cerr << XMLGetName (nodeElement) << std::endl;
///////////////if (vfDebug) std::cerr << "pszNameText    = " << (pszNameText ? (const char *)pszNameText : "NULL") << std::endl;
///////////////if (vfDebug) std::cerr << "pszNewNameText = " << (pszNewNameText ? (const char *)pszNewNameText : "NULL") << std::endl;
///////////////if (vfDebug) std::cerr << "pszOldName     = " << (pszOldName ? pszOldName : "NULL") << std::endl;

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  if (vfDumpElements) std::cout << pszNameText << "->" << pszNewNameText << std::endl;

                  // Create new XML tag that has the old name's contents
                  nodeNew = XMLNewTextChild (nodeElement,
                                             0,
                                             pszOldName,
                                             pszNameText);

                  // Add it right after this one and not at the end.
                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Add whitespace to match current indentation
                  nodeNew = XMLNewText ("\n      ");

                  XMLAddNextSibling (nodeElement, nodeNew);

                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);
               }
               else
               {
                  fRet = false;
               }

               // Clean up
               if (pszNameText)
               {
                  XMLFree ((void *)pszNameText);
               }
               if (  fFreeNewNameText
                  && pszNewNameText
                  )
               {
                  free ((void *)pszNewNameText);
               }
            }

            nodeElement = XMLNextNode (nodeElement);
         }
      }

      nodeAttributes = XMLNextNode (nodeAttributes);
   }

///XMLDocDump (stdout, XMLGetDocNode (nodeDevice));

   // Query the old job properties
   const char *pszOrientation     = 0;
   const char *pszForm            = 0;
   const char *pszTray            = 0;
   const char *pszMedia           = 0;
   const char *pszResolution      = 0;
   const char *pszDither          = 0;
   const char *pszPrintmode       = 0;
   const char *pszOmniOrientation = 0;
   const char *pszOmniForm        = 0;
   const char *pszOmniTray        = 0;
   const char *pszOmniMedia       = 0;
   const char *pszOmniResolution  = 0;
   const char *pszOmniDither      = 0;
   const char *pszOmniPrintmode   = 0;
   XmlNodePtr  nodeNewValue       = 0;
   XmlNodePtr  nodeNewWS          = 0;

   nodeAttributes = XMLFirstNode (XMLGetChildrenNode (nodeDevice));

   while (nodeAttributes)
   {
      if (0 == XMLStrcmp (XMLGetName (nodeAttributes), "DefaultJobProperties"))
      {
         // Get the first non-text child in the list
         nodeElement = XMLFirstNode (XMLGetChildrenNode (nodeAttributes));

         if (nodeElement == NULL)
         {
            return false;
         }

         pszOrientation = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniOrientation"))
         {
            pszOmniOrientation = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
            {
               return false;
            }
         }

         pszForm = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniForm"))
         {
            pszOmniForm = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
            {
               return false;
            }
         }

         pszTray = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniTray"))
         {
            pszOmniTray = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
            {
               return false;
            }
         }

         pszMedia = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniMedia"))
         {
            pszOmniMedia = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
            {
               return false;
            }
         }

         pszResolution = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniResolution"))
         {
            pszOmniResolution = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
            {
               return false;
            }
         }

         pszDither = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement == NULL)
         {
            return false;
         }
         if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniDither"))
         {
            pszOmniDither = bundleStringData (nodeElement);
            // Move to the next node
            nodeElement = XMLNextNode (nodeElement);
            if (nodeElement == NULL)
               return false;
         }

         pszPrintmode = bundleStringData (nodeElement);
         // Move to the next node
         nodeElement = XMLNextNode (nodeElement);
         if (nodeElement != NULL)
         {
            if (0 == XMLStrcmp (XMLGetName (nodeElement), "omniPrintmode"))
            {
               pszOmniPrintmode = bundleStringData (nodeElement);
               // Move to the next node
               nodeElement = XMLNextNode (nodeElement);
            }
         }

         XmlNodePtr nodePrevAttributes = XMLGetPreviousNode (nodeAttributes);

         // Delete the <DefaultJobProperties>...</DefaultJobProperties> tree
         XMLUnlinkNode (nodeAttributes);
         XMLFreeNodeList (nodeAttributes);

         nodeAttributes = XMLNewNode (0, "DefaultJobProperties");
         if (nodeAttributes)
         {
            XMLAddNextSibling (nodePrevAttributes, nodeAttributes);
         }
      }

      nodeAttributes = XMLNextNode (nodeAttributes);
   }

   nodeAttributes = XMLFirstNode (XMLGetChildrenNode (nodeDevice));

   while (nodeAttributes)
   {
      if (0 == XMLStrcmp (XMLGetName (nodeAttributes), "DefaultJobProperties"))
      {
         nodeNewWS = XMLNewText ("\n      ");
         if (nodeNewWS)
         {
            XMLAddChild (nodeAttributes, nodeNewWS);
         }

         nodeElement = XMLGetChildrenNode (nodeAttributes);
/////////if (vfDebug) std::cerr << "nodeElement = " << std::hex << (int)nodeElement << std::dec << std::endl;

         XmlNodePtr nodeCopies = XMLNewNode (0, "Copies");
         if (nodeCopies)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeCopies);

            nodeNewValue = XMLNewText ("1");
            if (nodeNewValue)
            {
               XMLAddChild (nodeCopies, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeCopies, nodeNewWS);
            }
         }

         XmlNodePtr nodeDither = XMLNewNode (0, "dither");
         if (nodeDither)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeDither);

            nodeNewValue = XMLNewText (pszDither);
            if (nodeNewValue)
            {
               XMLAddChild (nodeDither, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeDither, nodeNewWS);
            }
         }

         XmlNodePtr nodeForm = XMLNewNode (0, "Form");
         if (nodeForm)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeForm);

            nodeNewValue = XMLNewText (pszForm);
            if (nodeNewValue)
            {
               XMLAddChild (nodeForm, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeForm, nodeNewWS);
            }

            if (pszOmniForm)
            {
               XmlNodePtr nodeOmniForm = XMLNewNode (0, "omniForm");
               if (nodeOmniForm)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniForm);

                  nodeNewValue = XMLNewText (pszOmniForm);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniForm, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniForm, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeMedia = XMLNewNode (0, "media");
         if (nodeMedia)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeMedia);

            nodeNewValue = XMLNewText (pszMedia);
            if (nodeNewValue)
            {
               XMLAddChild (nodeMedia, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeMedia, nodeNewWS);
            }

            if (pszOmniMedia)
            {
               XmlNodePtr nodeOmniMedia = XMLNewNode (0, "omniMedia");
               if (nodeOmniMedia)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniMedia);

                  nodeNewValue = XMLNewText (pszOmniMedia);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniMedia, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniMedia, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeDNUp = XMLNewNode (0, "deviceNumberUp");
         if (nodeDNUp)
         {
            XmlNodePtr nodeNextDNUpChild = 0;

            nodeElement = XMLAddNextSibling (nodeElement, nodeDNUp);

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDNUpChild = XMLAddChild (nodeDNUp, nodeNewWS);
            }

            XmlNodePtr nodeNUp = XMLNewNode (0, "NumberUp");
            if (nodeNUp)
            {
               XmlNodePtr nodeNextNUpChild = 0;

               nodeNextDNUpChild = XMLAddNextSibling (nodeNextDNUpChild, nodeNUp);

               XMLNewProp (nodeNUp, "FORMAT", "XbyY");

               XmlNodePtr nodeX = XMLNewNode (0, "x");
               if (nodeX)
               {
                  nodeNewWS = XMLNewText ("\n            ");
                  if (nodeNewWS)
                  {
                     nodeNextNUpChild = XMLAddChild (nodeNUp, nodeNewWS);
                  }

                  nodeNextNUpChild = XMLAddNextSibling (nodeNextNUpChild, nodeX);

                  nodeNewValue = XMLNewText ("1");
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeX, nodeNewValue);
                  }
               }

               XmlNodePtr nodeY = XMLNewNode (0, "y");
               if (nodeY)
               {
                  nodeNewWS = XMLNewText ("\n            ");
                  if (nodeNewWS)
                  {
                     nodeNextNUpChild = XMLAddChild (nodeNUp, nodeNewWS);
                  }

                  nodeNextNUpChild = XMLAddNextSibling (nodeNextNUpChild, nodeY);

                  nodeNewValue = XMLNewText ("1");
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeY, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n         ");
                  if (nodeNewWS)
                  {
                     nodeNextNUpChild = XMLAddNextSibling (nodeNextNUpChild, nodeNewWS);
                  }
               }

               nodeNewWS = XMLNewText ("\n      ");
               if (nodeNewWS)
               {
                  XMLAddNextSibling (nodeNUp, nodeNewWS);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDNUpChild = XMLAddNextSibling (nodeNextDNUpChild, nodeNewWS);
            }

            XmlNodePtr nodeNUPD = XMLNewNode (0, "NumberUpPresentationDirection");
            if (nodeNUPD)
            {
               nodeNextDNUpChild = XMLAddNextSibling (nodeNextDNUpChild, nodeNUPD);

               nodeNewValue = XMLNewText ("TorightTobottom");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeNUPD, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeDNUp, nodeNewWS);
            }
         }

         XmlNodePtr nodeRotation = XMLNewNode (0, "Rotation");
         if (nodeRotation)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeRotation);

            nodeNewValue = XMLNewText (pszOrientation);
            if (nodeNewValue)
            {
               XMLAddChild (nodeRotation, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeRotation, nodeNewWS);
            }

            if (pszOmniOrientation)
            {
               XmlNodePtr nodeOmniRotation = XMLNewNode (0, "omniOrientation");
               if (nodeOmniRotation)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniRotation);

                  nodeNewValue = XMLNewText (pszOmniOrientation);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniRotation, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniRotation, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeOutputBin = XMLNewNode (0, "OutputBin");
         if (nodeOutputBin)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeOutputBin);

            nodeNewValue = XMLNewText ("Top");
            if (nodeNewValue)
            {
               XMLAddChild (nodeOutputBin, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeOutputBin, nodeNewWS);
            }
         }

         XmlNodePtr nodePrintmode = XMLNewNode (0, "printmode");
         if (nodePrintmode)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodePrintmode);

            nodeNewValue = XMLNewText (pszPrintmode);
            if (nodeNewValue)
            {
               XMLAddChild (nodePrintmode, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodePrintmode, nodeNewWS);
            }

            if (pszOmniPrintmode)
            {
               XmlNodePtr nodeOmniPrintmode = XMLNewNode (0, "omniPrintmode");
               if (nodeOmniPrintmode)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniPrintmode);

                  nodeNewValue = XMLNewText (pszOmniPrintmode);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniPrintmode, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniPrintmode, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeResolution = XMLNewNode (0, "Resolution");
         if (nodeResolution)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeResolution);

            nodeNewValue = XMLNewText (pszResolution);
            if (nodeNewValue)
            {
               XMLAddChild (nodeResolution, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeResolution, nodeNewWS);
            }

            if (pszOmniResolution)
            {
               XmlNodePtr nodeOmniResolution = XMLNewNode (0, "omniResolution");
               if (nodeOmniResolution)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniResolution);

                  nodeNewValue = XMLNewText (pszOmniResolution);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniResolution, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniResolution, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeDeviceScaling = XMLNewNode (0, "deviceScaling");
         if (nodeDeviceScaling)
         {
            XmlNodePtr nodeNextDSChild = 0;

            nodeElement = XMLAddNextSibling (nodeElement, nodeDeviceScaling);

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddChild (nodeDeviceScaling, nodeNewWS);
            }

            XmlNodePtr nodeScalingPercentage = XMLNewNode (0, "ScalingPercentage");
            if (nodeScalingPercentage)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeScalingPercentage);

               nodeNewValue = XMLNewText ("0");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeScalingPercentage, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            XmlNodePtr nodeScalingType = XMLNewNode (0, "ScalingType");
            if (nodeScalingType)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeScalingType);

               nodeNewValue = XMLNewText ("None");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeScalingType, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeDeviceScaling, nodeNewWS);
            }
         }

         XmlNodePtr nodeSheetCollate = XMLNewNode (0, "SheetCollate");
         if (nodeSheetCollate)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeSheetCollate);

            nodeNewValue = XMLNewText ("SheetUncollated");
            if (nodeNewValue)
            {
               XMLAddChild (nodeSheetCollate, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeSheetCollate, nodeNewWS);
            }
         }

         XmlNodePtr nodeSides = XMLNewNode (0, "Sides");
         if (nodeSides)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeSides);

            nodeNewValue = XMLNewText ("OneSidedFront");
            if (nodeNewValue)
            {
               XMLAddChild (nodeSides, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeSides, nodeNewWS);
            }
         }

         XmlNodePtr nodeDeviceStitching = XMLNewNode (0, "deviceStitching");
         if (nodeDeviceStitching)
         {
            XmlNodePtr nodeNextDSChild = 0;

            nodeElement = XMLAddNextSibling (nodeElement, nodeDeviceStitching);

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddChild (nodeDeviceStitching, nodeNewWS);
            }

            XmlNodePtr nodeStitchingPosition = XMLNewNode (0, "StitchingPosition");
            if (nodeStitchingPosition)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeStitchingPosition);

               nodeNewValue = XMLNewText ("0");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeStitchingPosition, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            XmlNodePtr nodeStitchingReferenceEdge = XMLNewNode (0, "StitchingReferenceEdge");
            if (nodeStitchingReferenceEdge)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeStitchingReferenceEdge);

               nodeNewValue = XMLNewText ("Top");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeStitchingReferenceEdge, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            XmlNodePtr nodeStitchingType = XMLNewNode (0, "StitchingType");
            if (nodeStitchingType)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeStitchingType);

               nodeNewValue = XMLNewText ("None");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeStitchingType, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            XmlNodePtr nodeStitchingCount = XMLNewNode (0, "StitchingCount");
            if (nodeStitchingCount)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeStitchingCount);

               nodeNewValue = XMLNewText ("0");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeStitchingCount, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n         ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            XmlNodePtr nodeStitchingAngle = XMLNewNode (0, "StitchingAngle");
            if (nodeStitchingAngle)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeStitchingAngle);

               nodeNewValue = XMLNewText ("0");
               if (nodeNewValue)
               {
                  XMLAddChild (nodeStitchingAngle, nodeNewValue);
               }
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeNextDSChild = XMLAddNextSibling (nodeNextDSChild, nodeNewWS);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeDeviceStitching, nodeNewWS);
            }
         }

         XmlNodePtr nodeTray = XMLNewNode (0, "InputTray");
         if (nodeTray)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeTray);

            nodeNewValue = XMLNewText (pszTray);
            if (nodeNewValue)
            {
               XMLAddChild (nodeTray, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n      ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeTray, nodeNewWS);
            }

            if (pszOmniTray)
            {
               XmlNodePtr nodeOmniTray = XMLNewNode (0, "omniTray");
               if (nodeOmniTray)
               {
                  nodeElement = XMLAddNextSibling (nodeElement, nodeOmniTray);

                  nodeNewValue = XMLNewText (pszOmniTray);
                  if (nodeNewValue)
                  {
                     XMLAddChild (nodeOmniTray, nodeNewValue);
                  }

                  nodeNewWS = XMLNewText ("\n      ");
                  if (nodeNewWS)
                  {
                     nodeElement = XMLAddNextSibling (nodeOmniTray, nodeNewWS);
                  }
               }
            }
         }

         XmlNodePtr nodeTrimming = XMLNewNode (0, "Trimming");
         if (nodeTrimming)
         {
            nodeElement = XMLAddNextSibling (nodeElement, nodeTrimming);

            nodeNewValue = XMLNewText ("None");
            if (nodeNewValue)
            {
               XMLAddChild (nodeTrimming, nodeNewValue);
            }

            nodeNewWS = XMLNewText ("\n   ");
            if (nodeNewWS)
            {
               nodeElement = XMLAddNextSibling (nodeTrimming, nodeNewWS);
            }
         }
      }

      nodeAttributes = XMLNextNode (nodeAttributes);
   }

   if (pszOrientation)
   {
      XMLFree ((void *)pszOrientation);
   }
   if (pszForm)
   {
      XMLFree ((void *)pszForm);
   }
   if (pszTray)
   {
      XMLFree ((void *)pszTray);
   }
   if (pszMedia)
   {
      XMLFree ((void *)pszMedia);
   }
   if (pszResolution)
   {
      XMLFree ((void *)pszResolution);
   }
   if (pszDither)
   {
      XMLFree ((void *)pszDither);
   }
   if (pszPrintmode)
   {
      XMLFree ((void *)pszPrintmode);
   }
   if (pszOmniOrientation)
   {
      XMLFree ((void *)pszOmniOrientation);
   }
   if (pszOmniForm)
   {
      XMLFree ((void *)pszOmniForm);
   }
   if (pszOmniTray)
   {
      XMLFree ((void *)pszOmniTray);
   }
   if (pszOmniMedia)
   {
      XMLFree ((void *)pszOmniMedia);
   }
   if (pszOmniResolution)
   {
      XMLFree ((void *)pszOmniResolution);
   }
   if (pszOmniDither)
   {
      XMLFree ((void *)pszOmniDither);
   }
   if (pszOmniPrintmode)
   {
      XMLFree ((void *)pszOmniPrintmode);
   }

   return true; //fRet;
}

bool
processFile (const char *pszFileName,
             bool        fUpdate)
{
#ifdef DTD
   xmlValidCtxt        validatorDTD    = {
      0,
      MyValidityErrorFunc,
      MyValidityWarningFunc
   };
#endif
   XmlDocPtr           doc             = 0;
   XmlNodePtr          nodeRoot        = 0;
   bool                fRC             = false;

   doc = XMLParseFile (pszFileName);

   if (vfDebug) std::cerr << "doc = " << std::hex << (int)doc << std::dec << std::endl;

   if (!doc)
   {
      return false;
   }

#ifdef DTD
   int iRC = xmlValidateDocument (&validatorDTD, doc);

   if (vfDebug) std::cerr << "xmlValidateDocument = " << iRC << std::endl;
#endif

   nodeRoot = XMLFirstNode (XMLDocGetRootElement (doc));
   if (!nodeRoot)
   {
      return false;
   }
   else if (0 == XMLStrcmp ("deviceForms", XMLGetName (nodeRoot)))
   {
      fRC = processForms (nodeRoot, fUpdate);
   }
   else if (0 == XMLStrcmp ("deviceTrays", XMLGetName (nodeRoot)))
   {
      fRC = processTrays (nodeRoot, fUpdate);
   }
   else if (0 == XMLStrcmp ("deviceResolutions", XMLGetName (nodeRoot)))
   {
      fRC = processResolutions (nodeRoot, fUpdate);
   }
   else if (0 == XMLStrcmp ("deviceOrientations", XMLGetName (nodeRoot)))
   {
      fRC = processOrientations (nodeRoot, fUpdate);
   }
   else if (0 == XMLStrcmp ("deviceGammaTables", XMLGetName (nodeRoot)))
   {
      fRC = processGammas (nodeRoot, fUpdate, pszFileName);
   }
   else if (0 == XMLStrcmp ("deviceConnections", XMLGetName (nodeRoot)))
   {
      fRC = processConnections (nodeRoot, fUpdate, pszFileName);
   }
   else if (0 == XMLStrcmp ("Device", XMLGetName (nodeRoot)))
   {
      fRC = processDevice (nodeRoot, fUpdate);
   }

   if (  fRC
      && vfDumpFile
      )
   {
      XMLDocDump (stdout, doc);
   }

   if (  fRC
      && fUpdate
      )
   {
      XMLSaveFile (pszFileName, doc);
   }

   XMLFreeDoc (doc);

   return true;
}

int
main (int argc, char *argv[])
{
   std::ostringstream oss;
   glob_t             globbuf;
   int                rc;

   // Call glob
   memset (&globbuf, 0, sizeof (globbuf));

   oss << "*.xml"
       << std::ends;

   rc = glob (oss.str ().c_str (), 0, NULL, &globbuf);

   if (rc)
   {
      std::cerr << "Error: glob (" << oss.str () << ") failed. rc = " << rc << std::endl;

      return __LINE__;
   }

   if (0 == globbuf.gl_pathc)
   {
      std::cerr << "Error: glob (" << oss.str () << ") returned 0 files!" << std::endl;

      return __LINE__;
   }

   // Call succeded
   for (int i = 0; i < (int)globbuf.gl_pathc; i++)
   {
      std::cout << globbuf.gl_pathv[i] << std::endl;

      processFile (globbuf.gl_pathv[i], true);
   }

   globfree (&globbuf);

   return 0;
}
