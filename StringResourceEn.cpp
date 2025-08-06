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
#include "Device.hpp"
#include "StringResourceEn.hpp"

StringResourceEn::
StringResourceEn (StringResource *pFallback)
{
   pFallback_d = pFallback;
}

PSZCRO StringResourceEn::
getStringV (int iGroup, int iID)
{
   PSZCRO *apszStrings = 0;
   int     iNumStrings = 0;
   int     iOffset     = 0;

   switch (iGroup)
   {
   case STRINGGROUP_DEVICE_COMMON:
   {
      static PSZCRO apszDeviceCommonNames[] = {
      /*                                           Translation start: */
      /* DEVICE_COMMON_UNKNOWN                  */ "Unknown",
      /* DEVICE_COMMON_UNLISTED                 */ "Unlisted",
      /* DEVICE_COMMON_BOOKLET                  */ "Booklet",
      /* DEVICE_COMMON_COPIES                   */ "Copies",
      /* DEVICE_COMMON_DITHER                   */ "Dither",
      /* DEVICE_COMMON_FORM                     */ "Form",
      /* DEVICE_COMMON_JOGGING                  */ "Jogging",
      /* DEVICE_COMMON_MEDIA                    */ "Media",
      /* DEVICE_COMMON_NUP                      */ "N-Up",
      /* DEVICE_COMMON_NUP_DIRECTION            */ "N-Up Direction",
      /* DEVICE_COMMON_ORIENTATION              */ "Orientation",
      /* DEVICE_COMMON_OUTPUTBIN                */ "Output Bin",
      /* DEVICE_COMMON_PRINTMODE                */ "Print Mode",
      /* DEVICE_COMMON_RESOLUTION               */ "Resolution",
      /* DEVICE_COMMON_SCALING_TYPE             */ "Scaling Type",
      /* DEVICE_COMMON_SCALING_PERCENTAGE       */ "Scaling Percentage",
      /* DEVICE_COMMON_SHEETCOLLATE             */ "Sheet Collate",
      /* DEVICE_COMMON_SIDE                     */ "Side",
      /* DEVICE_COMMON_STITCHING_POSITION       */ "Stitching Position",
      /* DEVICE_COMMON_STITCHING_REFERENCE_EDGE */ "Stitching Edge",
      /* DEVICE_COMMON_STITCHING_STITCHING_TYPE */ "Stitching Type",
      /* DEVICE_COMMON_STITCHING_COUNT          */ "Stitching Count",
      /* DEVICE_COMMON_STITCHING_ANGLE          */ "Stitching Angle",
      /* DEVICE_COMMON_TRAY                     */ "Tray",
      /* DEVICE_COMMON_TRIMMING                 */ "Trimming",
      /* DEVICE_COMMON_BIDIRECTIONAL            */ "Bi-Directional",
      /* DEVICE_COMMON_TRUE                     */ "True",
      /* DEVICE_COMMON_FALSE                    */ "False",
      /* DEVICE_COMMON_JOURNAL                  */ "Journal",
      /* DEVICE_COMMON_CUTMODE                  */ "Cut Mode",
      /* DEVICE_COMMON_NONE                     */ "None",
      /* DEVICE_COMMON_FULL                     */ "Full",
      /* DEVICE_COMMON_PARTIAL                  */ "Partial",
      /* DEVICE_COMMON_HARDWARE_SCALING         */ "Hardware Scaling",
      /* DEVICE_COMMON_QUALITY                  */ "Quality",
      /* DEVICE_COMMON_DRAFT                    */ "Draft",
      /* DEVICE_COMMON_STANDARD                 */ "Standard",
      /* DEVICE_COMMON_FINE                     */ "Fine",
      /* DEVICE_COMMON_SUPER_FINE               */ "Super Fine",
      /* DEVICE_COMMON_PHOTO                    */ "Photo",
      /* DEVICE_COMMON_SUPER_PHOTO              */ "Super Photo",
      /* DEVICE_COMMON_HIGH_SPEED               */ "High Speed",
      /* DEVICE_COMMON_ON                       */ "On",
      /* DEVICE_COMMON_OFF                      */ "Off",
      /* DEVICE_COMMON_COLOR_MODE               */ "Color Mode",
      /* DEVICE_COMMON_AUTO                     */ "Automatic",
      /* DEVICE_COMMON_GRAPH                    */ "Graph",
      /* DEVICE_COMMON_MICRO_WEAVE              */ "Micro Weave",
      /* DEVICE_COMMON_RET                      */ "RET (Resolution Enhancement Technology)",
      /* DEVICE_COMMON_ECONO_MODE               */ "Economy Mode",
      /* DEVICE_COMMON_PAGE_PROTECT             */ "Page Protect",
      /* DEVICE_COMMON_JAM_RECOVERY             */ "Jam Recovery",
      /* DEVICE_COMMON_STAPLE                   */ "Staple",
      /* DEVICE_COMMON_OFFSET                   */ "Offset",
      /* DEVICE_COMMON_FROM                     */ "From",
      /* DEVICE_COMMON_TO                       */ "to",
      /* DEVICE_COMMON_INFINITE                 */ "infinite"
      /*                                           Translation end! */
      };

      apszStrings = apszDeviceCommonNames;
      iNumStrings = dimof (apszDeviceCommonNames);
      iOffset     = 0;
      break;
   }

   default:
      return 0;
   }

   iID -= iOffset;

   if (  apszStrings
      && iID < iNumStrings
      )
      return apszStrings[iID];
   else
      return 0;
}

PSZCRO StringResourceEn::
getStringV (int iGroup, PSZRO pszID)
{
   typedef struct _StringMap {
      PSZCRO pszFrom;
      PSZCRO pszTo;
   } STRINGMAP, *PSTRINGMAP;

   PSTRINGMAP pStringMap = 0;
   int        iLow       = 0;
   int        iMid       = 0;
   int        iHigh      = 0;
   int        iResult    = 0;
   PSZRO      pszReturn  = 0;
   bool       fFreeID    = false;

   switch (iGroup)
   {
   case STRINGGROUP_DITHERS:
   {
      static STRINGMAP aDitherNames[] = {
      /*   Translation ignore!           Translation start: */
         { "DITHER_CMYK_DIFFUSION",      "Cyan Magenta Yellow Black Diffusion" },
         { "DITHER_DITHER_4x4",          "4x4 Grid"                            },
         { "DITHER_DITHER_8x8",          "8x8 Grid"                            },
         { "DITHER_ESTUCKI_DIFFUSION",   "Enhanced Stucki"                     },
         { "DITHER_FAST_DIFFUSION",      "Fast Diffusion"                      },
         { "DITHER_HSV_BIDIFFUSION",     "Hue Saturaion Value Bidiffusion"     },
         { "DITHER_HSV_DIFFUSION",       "Hue Saturaion Value Diffusion"       },
         { "DITHER_JANIS_STUCKI",        "Janis Stucki"                        },
         { "DITHER_LEVEL",               "Level"                               },
         { "DITHER_MAGIC_SQUARES",       "Magic Square"                        },
         { "DITHER_ORDERED_SQUARES",     "Ordered Square"                      },
         { "DITHER_SMOOTH_DIFFUSION",    "Smooth Diffusion"                    },
         { "DITHER_SNAP",                "Snap"                                },
         { "DITHER_STEINBERG_DIFFUSION", "Steinberg Diffusion"                 },
         { "DITHER_STUCKI_BIDIFFUSION",  "Stucki Bidiffusion"                  },
         { "DITHER_STUCKI_DIFFUSION",    "Stucki Diffusion"                    },
         { "DITHER_UNLISTED",            "Unlisted"                            },
         { "DITHER_VOID_CLUSTER",        "Void Cluster"                        }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aDitherNames) / 2;
      iHigh      = (int)dimof (aDitherNames) - 1;
      pStringMap = aDitherNames;
      break;
   }

   case STRINGGROUP_FORMS:
   {
      static STRINGMAP aFormNames[] = {
      /*   Translation ignore!                           Translation start: */
         { "iso_2a0",                                    "2A0"                        },
         { "iso_4a0",                                    "4A0"                        },
         { "iso_a0",                                     "A0"                         },
         { "iso_a1",                                     "A1"                         },
         { "iso_a10",                                    "A10"                        },
         { "iso_a2",                                     "A2"                         },
         { "iso_a3",                                     "A3"                         },
         { "iso_a3-extra",                               "A3 extra"                   },
         { "iso_a3-wide",                                "A3 Wide"                    },
         { "iso_a4",                                     "A4"                         },
         { "iso_a4-extra",                               "A4 extra"                   },
         { "iso_a4-tab",                                 "A4 tab"                     },
         { "iso_a4-wide",                                "A4 Wide"                    },
         { "iso_a5",                                     "A5"                         },
         { "iso_a5-extra",                               "A5 extra"                   },
         { "iso_a6",                                     "A6"                         },
         { "iso_a7",                                     "A7"                         },
         { "iso_a8",                                     "A8"                         },
         { "iso_a9",                                     "A9"                         },
         { "iso_b0",                                     "B0"                         },
         { "iso_b1",                                     "B1"                         },
         { "iso_b10",                                    "B10"                        },
         { "iso_b2",                                     "B2"                         },
         { "iso_b3",                                     "B3"                         },
         { "iso_b4",                                     "B4 Envelope"                },
         { "iso_b5",                                     "B5 Envelope"                },
         { "iso_b5-extra",                               "B5 extra"                   },
         { "iso_b6",                                     "B6 Envelope"                },
         { "iso_b6c4",                                   "B6/C4 Envelope"             },
         { "iso_b7",                                     "B7"                         },
         { "iso_b8",                                     "B8"                         },
         { "iso_b9",                                     "B9"                         },
         { "iso_c0",                                     "C0"                         },
         { "iso_c1",                                     "C1"                         },
         { "iso_c10",                                    "C10"                        },
         { "iso_c2",                                     "C2"                         },
         { "iso_c3",                                     "C3"                         },
         { "iso_c4",                                     "C4"                         },
         { "iso_c5",                                     "C5"                         },
         { "iso_c6",                                     "C6"                         },
         { "iso_c6c5",                                   "C6/C5"                      },
         { "iso_c7",                                     "C7"                         },
         { "iso_c7c6",                                   "C7/C6"                      },
         { "iso_c8",                                     "C8"                         },
         { "iso_c9",                                     "C9"                         },
         { "iso_e1",                                     "E1"                         },
         { "iso_ra0",                                    "RA0"                        },
         { "iso_ra1",                                    "RA1"                        },
         { "iso_ra2",                                    "RA2"                        },
         { "iso_sra0",                                   "SRA0"                       },
         { "iso_sra1",                                   "SRA1"                       },
         { "iso_sra2",                                   "SRA2"                       },
         { "jis_b0",                                     "JIS B0"                     },
         { "jis_b1",                                     "JIS B1"                     },
         { "jis_b10",                                    "JIS B10"                    },
         { "jis_b2",                                     "JIS B2"                     },
         { "jis_b3",                                     "JIS B3"                     },
         { "jis_b4",                                     "JIS B4"                     },
         { "jis_b5",                                     "JIS B5"                     },
         { "jis_b6",                                     "JIS B6"                     },
         { "jis_b7",                                     "JIS B7"                     },
         { "jis_b8",                                     "JIS B8"                     },
         { "jis_b9",                                     "JIS B9"                     },
         { "jis_exec",                                   "JIS Exec"                   },
         { "jpn_chou2",                                  "Chou2 Envelope"             },
         { "jpn_chou3",                                  "Chou3 Envelope"             },
         { "jpn_chou4",                                  "Chou4 Envelope"             },
         { "jpn_hagaki",                                 "Hagaki Card"                },
         { "jpn_kahu",                                   "Kahu Envelope"              },
         { "jpn_kaku2",                                  "Kaku2 Envelope"             },
         { "jpn_oufuku",                                 "Oufuku Card"                },
         { "jpn_you4",                                   "You4 Envelope"              },
         { "na_10x11",                                   "10 x 11"                    },
         { "na_11x12",                                   "11 x 12"                    },
         { "na_11x15",                                   "11 x 15"                    },
         { "na_12x19",                                   "12 x 19"                    },
         { "na_15x11",                                   "15 x 11"                    },
         { "na_170x210",                                 "170 mm x 210 mm"            },
         { "na_180x210",                                 "182 mm x 210 mm"            },
         { "na_3x5-card",                                "3 x 5 Card"                 },
         { "na_4x6-card",                                "4 x 6 Card"                 },
         { "na_5x7",                                     "5 x 7"                      },
         { "na_5x8-card",                                "5 x 8 Card"                 },
         { "na_8.25x13",                                 "8.25 x 13"                  },
         { "na_8.5x12.4",                                "8.5 x 12.4"                 },
         { "na_8x10-card",                               "8 x 10 Card"                },
         { "na_8x10.5",                                  "8 x 10.5"                   },
         { "na_a2",                                      "A2 Envelope"                },
         { "na_a6-card",                                 "A6 Card"                    },
         { "na_arch-a",                                  "US A Architectural"         },
         { "na_arch-b",                                  "US B Architectural"         },
         { "na_arch-c",                                  "US C Architectural"         },
         { "na_arch-d",                                  "US D Architectural"         },
         { "na_arch-e",                                  "US E Architectural"         },
         { "na_b-plus",                                  "B plus"                     },
         { "na_c10",                                     "C10 Envelope"               },
         { "na_c5",                                      "C5 Envelope"                },
         { "na_c6",                                      "C6 Envelope"                },
         { "na_c7",                                      "C7 Envelope"                },
         { "na_c9",                                      "C9 Envelope"                },
         { "na_card-148",                                "Card 148"                   },
         { "na_d5",                                      "D5 Envelope"                },
         { "na_disk-labels",                             "Disk Labels"                },
         { "na_dl",                                      "Dl Envelope"                },
         { "na_edp",                                     "EDP"                        },
         { "na_envelope-10x13",                          "Envelope 10 x 13"           },
         { "na_envelope-10x14",                          "Envelope 10 x 14"           },
         { "na_envelope-10x15",                          "Envelope 10 x 15"           },
         { "na_envelope-132x220",                        "Envelope 132 x 220"         },
         { "na_envelope-6.5",                            "Envelope 6 1/2"             },
         { "na_envelope-6x9",                            "Envelope 6 x 9"             },
         { "na_envelope-7x9",                            "Envelope 7 x 9"             },
         { "na_envelope-9x11",                           "Envelope 9 x 11"            },
         { "na_eur-edp",                                 "European EDP"               },
         { "na_euro-labels",                             "Euro Labels"                },
         { "na_executive",                               "Executive"                  },
         { "na_fanfold-1",                               "Fanfold #1"                 },
         { "na_fanfold-2",                               "Fanfold #2"                 },
         { "na_fanfold-3",                               "Fanfold #3"                 },
         { "na_fanfold-4",                               "Fanfold #4"                 },
         { "na_fanfold-5",                               "Fanfold #5"                 },
         { "na_fanfold-eur",                             "European Fanfold"           },
         { "na_fanfold-us",                              "Fanfold"                    },
         { "na_foolscap",                                "Foolscap"                   },
         { "na_foolscap-wide",                           "Foolscap Wide"              },
         { "na_german-12x250-fanfold",                   "German Fanfold 12 x 240 mm" },
         { "na_german-legal-fanfold",                    "German Legal Fanfold"       },
         { "na_govt-legal",                              "Government Letter"          },
         { "na_govt-letter",                             "Government Legal"           },
         { "na_half-letter",                             "Half Letter"                },
         { "na_index-4x6-ext",                           "Index 4 x 6 Ext"            },
         { "na_ledger",                                  "Ledger"                     },
         { "na_legal",                                   "Legal"                      },
         { "na_legal-extra",                             "Legal extra"                },
         { "na_letter",                                  "Letter"                     },
         { "na_letter-extra",                            "Letter extra"               },
         { "na_letter-plus",                             "Letter plus"                },
         { "na_letter-wide",                             "Letter Wide"                },
         { "na_monarch",                                 "Monarch Envelope"           },
         { "na_number-10",                               "Number 10 Envelope"         },
         { "na_number-11",                               "Number 11 Envelope"         },
         { "na_number-12",                               "Number 12 Envelope"         },
         { "na_number-14",                               "Number 14 Envelope"         },
         { "na_number-9",                                "Number 9 Envelope"          },
         { "na_panoramic",                               "Panoramic"                  },
         { "na_personal",                                "Personal Envelope"          },
         { "na_photo-100x150",                           "Photo 100x150"              },
         { "na_photo-200x300",                           "Photo 200x300"              },
         { "na_photo-4x6",                               "Photo 4x6"                  },
         { "na_plotter-size-a",                          "Plotter A paper"            },
         { "na_plotter-size-b",                          "Plotter B paper"            },
         { "na_plotter-size-c",                          "Plotter C paper"            },
         { "na_plotter-size-d",                          "Plotter D paper"            },
         { "na_plotter-size-e",                          "Plotter E paper"            },
         { "na_plotter-size-f",                          "Plotter F paper"            },
         { "na_postcard",                                "Postcard"                   },
         { "na_quarto",                                  "Quarto"                     },
         { "na_roll-69.5",                               "69.5 mm Roll"               },
         { "na_roll-76.2",                               "76.2 mm Roll"               },
         { "na_shipping-labels",                         "Shipping Labels"            },
         { "na_standard-labels-clear",                   "Standard Labels Clear"      },
         { "na_standard-labels-white",                   "Standard Labels White"      },
         { "na_statement",                               "Statement"                  },
         { "na_super-a",                                 "Super A"                    },
         { "na_super-a3-b",                              "Super A3-B"                 },
         { "na_super-b",                                 "Super B"                    },
         { "na_tabloid",                                 "Tabloid"                    },
         { "na_universal",                               "Universal"                  },
         { "na_wide-format",                             "Wide"                       },
         { "om_dai-pa-kai",                              "Dai Pa Kai"                 },
         { "om_folio",                                   "Folio"                      },
         { "om_folio-sp",                                "Folio sp"                   },
         { "om_invite",                                  "Invite Envelope"            },
         { "om_italian",                                 "Italian Envelope"           },
         { "om_juuro-ku-kai",                            "Jurro Ku Kai"               },
         { "om_pa-kai",                                  "Pa Kai"                     },
         { "om_postfix",                                 "Postfix Envelope"           },
         { "prc_1",                                      "PRC1 Envelope"              },
         { "prc_10",                                     "PRC10 Envelope"             },
         { "prc_16k",                                    "PRC 16K"                    },
         { "prc_2",                                      "PRC2 Envelope"              },
         { "prc_3",                                      "PRC3 Envelope"              },
         { "prc_32k",                                    "PRC 32K"                    },
         { "prc_4",                                      "PRC4 Envelope"              },
         { "prc_5",                                      "PRC5 Envelope"              },
         { "prc_6",                                      "PRC6 Envelope"              },
         { "prc_7",                                      "PRC7 Envelope"              },
         { "prc_8",                                      "PRC8 Envelope"              },
         { "prc_9",                                      "PRC9 Envelope"              },
         { "roc_16k",                                    "Roc 16K"                    },
         { "roc_8k",                                     "Roc 8K"                     },
      /*   Translation ignore!                           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aFormNames) / 2;
      iHigh      = (int)dimof (aFormNames) - 1;
      pStringMap = aFormNames;
      pszID      = DeviceForm::getShortFormName (pszID);
      fFreeID    = true;
      break;
   }

   case STRINGGROUP_MEDIAS:
   {
      static STRINGMAP aMediaNames[] = {
      /*   Translation ignore!                    Translation start: */
         { "MEDIA_ARCHIVAL_MATTE_PAPER",          "Archival Matte Paper"           },
         { "MEDIA_AUTO",                          "Auto"                           },
         { "MEDIA_BACKPRINT",                     "Backprint"                      },
         { "MEDIA_BOND",                          "Bond"                           },
         { "MEDIA_BRIGHT_WHITE_INKJET_PAPER",     "Bright White Ink Jet Paper"     },
         { "MEDIA_CARDBOARD",                     "Cardboard"                      },
         { "MEDIA_CARDSTOCK",                     "Cardstock"                      },
         { "MEDIA_CD_MASTER",                     "CD-master"                      },
         { "MEDIA_CLOTH",                         "Cloth"                          },
         { "MEDIA_COATED",                        "Coated"                         },
         { "MEDIA_COLOR",                         "Color"                          },
         { "MEDIA_COLORLIFE_PHOTO_PAPER",         "ColorLife Photo Paper"          },
         { "MEDIA_CUSTOM_1",                      "Custom 1"                       },
         { "MEDIA_CUSTOM_2",                      "Custom 2"                       },
         { "MEDIA_CUSTOM_3",                      "Custom 3"                       },
         { "MEDIA_CUSTOM_4",                      "Custom 4"                       },
         { "MEDIA_CUSTOM_5",                      "Custom 5"                       },
         { "MEDIA_DOUBLE_SIDED_MATTE_PAPER",      "Double-Sided Matte Paper"       },
         { "MEDIA_ENVELOPE",                      "Envelope"                       },
         { "MEDIA_GLOSSY",                        "Glossy"                         },
         { "MEDIA_HEAVYWEIGH_MATTE_PAPER",        "Heavyweight Matte Paper"        },
         { "MEDIA_HIGH_GLOSS_FILM",               "High Gloss Film"                },
         { "MEDIA_HIGH_RESOLUTION",               "High Resolution"                },
         { "MEDIA_HP_PHOTOGRAPHIC_PAPER",         "HP Photographic Paper"          },
         { "MEDIA_HP_PREMIUM_PAPER",              "HP Premium Paper"               },
         { "MEDIA_IRON_ON",                       "Iron-on"                        },
         { "MEDIA_LABECA",                        "Labeca"                         },
         { "MEDIA_LABELS",                        "Labels"                         },
         { "MEDIA_LETTERHEAD",                    "Letter Head"                    },
         { "MEDIA_NONE",                          "None"                           },
         { "MEDIA_OTHER",                         "Other"                          },
         { "MEDIA_PHOTOGRAPHIC_INKJET_PAPER",     "Photo Quality Ink Jet Paper"    },
         { "MEDIA_PHOTOGRAPHIC_LABEL",            "Photographic Label"             },
         { "MEDIA_PHOTOGRAPHIC_PAPER",            "Photographic Paper"             },
         { "MEDIA_PLAIN",                         "Plain"                          },
         { "MEDIA_PLAIN_ENHANCED",                "Plain Enhanced"                 },
         { "MEDIA_POSTCARD",                      "Postcard"                       },
         { "MEDIA_PREM_SEMIGLOSS_PHOTO_PAPER",    "Premium Semigloss Photo Paper"  },
         { "MEDIA_PREPRINTED",                    "Preprinted"                     },
         { "MEDIA_PREPUNCHED",                    "Prepunched"                     },
         { "MEDIA_RECYCLED",                      "Recycled"                       },
         { "MEDIA_ROUGH",                         "Rough"                          },
         { "MEDIA_SPECIAL",                       "Special"                        },
         { "MEDIA_SPECIAL_360",                   "Special 360"                    },
         { "MEDIA_SPECIAL_720",                   "Special 720"                    },
         { "MEDIA_SPECIAL_BLUE",                  "Blue"                           },
         { "MEDIA_SPECIAL_GREEN",                 "Green"                          },
         { "MEDIA_SPECIAL_GREY",                  "Grey"                           },
         { "MEDIA_SPECIAL_IVORY",                 "Ivory"                          },
         { "MEDIA_SPECIAL_LETTERHEAD",            "Letterhead"                     },
         { "MEDIA_SPECIAL_ORANGE",                "Orange"                         },
         { "MEDIA_SPECIAL_PINK",                  "Pink"                           },
         { "MEDIA_SPECIAL_PURPLE",                "Purple"                         },
         { "MEDIA_SPECIAL_RED",                   "Red"                            },
         { "MEDIA_SPECIAL_USER_COLOR",            "User Color"                     },
         { "MEDIA_SPECIAL_YELLOW",                "Yellow"                         },
         { "MEDIA_TABSTOCK",                      "Tabstock"                       },
         { "MEDIA_TABSTOCK_2",                    "2-Tab"                          },
         { "MEDIA_TABSTOCK_3",                    "3-Tab"                          },
         { "MEDIA_TABSTOCK_4",                    "4-Tab"                          },
         { "MEDIA_TABSTOCK_5",                    "5-Tab"                          },
         { "MEDIA_TABSTOCK_6",                    "6-Tab"                          },
         { "MEDIA_TABSTOCK_7",                    "7-Tab"                          },
         { "MEDIA_TABSTOCK_8",                    "8-Tab"                          },
         { "MEDIA_TABSTOCK_9",                    "9-Tab"                          },
         { "MEDIA_THERMAL",                       "Thermal"                        },
         { "MEDIA_THICK",                         "Thick"                          },
         { "MEDIA_THICK_1",                       "Thick 1"                        },
         { "MEDIA_THICK_2",                       "Thick 2"                        },
         { "MEDIA_THICK_3",                       "Thick 3"                        },
         { "MEDIA_THICK_BLUE",                    "Thick Blue"                     },
         { "MEDIA_THICK_GREEN",                   "Thick Green"                    },
         { "MEDIA_THICK_GREY",                    "Thick Grey"                     },
         { "MEDIA_THICK_IVORY",                   "Thick Ivory"                    },
         { "MEDIA_THICK_LETTERHEAD",              "Thick Letterhead"               },
         { "MEDIA_THICK_ORANGE",                  "Thick Orange"                   },
         { "MEDIA_THICK_PINK",                    "Thick Pink"                     },
         { "MEDIA_THICK_PURPLE",                  "Thick Purple"                   },
         { "MEDIA_THICK_RED",                     "Thick Red"                      },
         { "MEDIA_THICK_USER_COLOR",              "Thick User Color"               },
         { "MEDIA_THICK_YELLOW",                  "Thick Yellow"                   },
         { "MEDIA_TRANSLUCENT",                   "Translucent"                    },
         { "MEDIA_TRANSPARENCY",                  "Transparency"                   },
         { "MEDIA_UNLISTED",                      "Unlisted"                       },
         { "MEDIA_USE_PRINTER_SETTING",           "Use printer setting"            }
      /*   Translation ignore!                    Translation start: */
      };

      iLow       = 0;
      iMid       = (int)dimof (aMediaNames) / 2;
      iHigh      = (int)dimof (aMediaNames) - 1;
      pStringMap = aMediaNames;
      break;
   }

#ifdef INCLUDE_JP_COMMON_NUP
   case STRINGGROUP_NUPS:
   {
      static STRINGMAP aNUpNames[] = {
      /*   Translation ignore!           Translation start: */
         { "TobottomToleft",             "To bottom To left"  },
         { "TobottomToright",            "To bottom To right" },
         { "ToleftTobottom",             "To left To bottom"  },
         { "ToleftTotop",                "To left To top"     },
         { "TorightTobottom",            "To right To bottom" },
         { "TorightTotop",               "To right To top"    },
         { "TotopToleft",                "To top To left"     },
         { "TotopToright",               "To top To right"    }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aNUpNames) / 2;
      iHigh      = (int)dimof (aNUpNames) - 1;
      pStringMap = aNUpNames;
      break;
   }
#endif

   case STRINGGROUP_ORIENTATIONS:
   {
      static STRINGMAP aOrientationNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Landscape",                  "Landscape"         },
         { "Portrait",                   "Portrait"          },
         { "ReverseLandscape",           "Reverse Landscape" },
         { "ReversePortrait",            "Reverse Portrait"  }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aOrientationNames) / 2;
      iHigh      = (int)dimof (aOrientationNames) - 1;
      pStringMap = aOrientationNames;
      break;
   }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   case STRINGGROUP_OUTPUT_BINS:
   {
      static STRINGMAP aOutputBinNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Booklet",                    "Booklet"        },
         { "Bottom",                     "Bottom"         },
         { "Center",                     "Center"         },
         { "FaceDown",                   "Face Down"      },
         { "FaceUp",                     "Face Up"        },
         { "FitMedia",                   "Fit the Media"  },
         { "LargeCapacity",              "Large Capacity" },
         { "Left",                       "Left"           },
         { "Mailbox-1",                  "Mailbox #1"     },
         { "Mailbox-2",                  "Mailbox #2"     },
         { "Mailbox-3",                  "Mailbox #3"     },
         { "Mailbox-4",                  "Mailbox #4"     },
         { "Mailbox-5",                  "Mailbox #5"     },
         { "Mailbox-6",                  "Mailbox #6"     },
         { "Mailbox-7",                  "Mailbox #7"     },
         { "Mailbox-8",                  "Mailbox #8"     },
         { "Mailbox-9",                  "Mailbox #9"     },
         { "Middle",                     "Middle"         },
         { "MyMailbox",                  "My Mailbox"     },
         { "Rear",                       "Rear"           },
         { "Right",                      "Right"          },
         { "Side",                       "Side"           },
         { "Stacker-1",                  "Stacker #1"     },
         { "Stacker-2",                  "Stacker #2"     },
         { "Stacker-3",                  "Stacker #3"     },
         { "Stacker-4",                  "Stacker #4"     },
         { "Stacker-5",                  "Stacker #5"     },
         { "Stacker-6",                  "Stacker #6"     },
         { "Stacker-7",                  "Stacker #7"     },
         { "Stacker-8",                  "Stacker #8"     },
         { "Stacker-9",                  "Stacker #9"     },
         { "Top",                        "Top"            },
         { "Tray-1",                     "Tray #1"        },
         { "Tray-2",                     "Tray #2"        },
         { "Tray-3",                     "Tray #3"        },
         { "Tray-4",                     "Tray #4"        },
         { "Tray-5",                     "Tray #5"        },
         { "Tray-6",                     "Tray #6"        },
         { "Tray-7",                     "Tray #7"        },
         { "Tray-8",                     "Tray #8"        },
         { "Tray-9",                     "Tray #9"        }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aOutputBinNames) / 2;
      iHigh      = (int)dimof (aOutputBinNames) - 1;
      pStringMap = aOutputBinNames;
      break;
   }
#endif

   case STRINGGROUP_PRINTMODES:
   {
      static STRINGMAP aPrintModeNames[] = {
      /*   Translation ignore!           Translation start: */
         { "PRINT_MODE_1_ANY",           "1-bit Monochrome",                                },
         { "PRINT_MODE_24_CMY",          "24-bit Color 3 Color with composite Black", },
         { "PRINT_MODE_24_CMYK",         "24-bit Color 4 Color with separate Black",  },
         { "PRINT_MODE_24_CcMmYK",       "24-bit 6 Color",                            },
         { "PRINT_MODE_24_CcMmYyK",      "24-bit 7 Color",                            },
         { "PRINT_MODE_24_K",            "24-bit Monochrome",                                },
         { "PRINT_MODE_24_RGB",          "24-bit RGB Color"                           },
         { "PRINT_MODE_8_CMY",           "8-bit 3 Color with composite Black",        },
         { "PRINT_MODE_8_CMYK",          "8-bit 4 Color with separate Black",         },
         { "PRINT_MODE_8_CcMmYK",        "8-bit 6 Color",                             },
         { "PRINT_MODE_8_CcMmYyK",       "8-bit 7 Color",                             },
         { "PRINT_MODE_8_K",             "8-bit Monochrome",                                },
         { "PRINT_MODE_8_RGB",           "8-bit RGB Color",                           },
         { "PRINT_MODE_NONE",            "None",                                      },
         { "PRINT_MODE_UNLISTED",        "Unlisted",                                  }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aPrintModeNames) / 2;
      iHigh      = (int)dimof (aPrintModeNames) - 1;
      pStringMap = aPrintModeNames;
      break;
   }

   case STRINGGROUP_RESOLUTIONS:
   {
      static STRINGMAP aResolutionNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Unlisted",                   "Unlisted" },
         { "None",                       "None"     }
      /*   Translation ignore!           Translation end! */
      };
      iLow       = 0;
      iMid       = (int)dimof (aResolutionNames) / 2;
      iHigh      = (int)dimof (aResolutionNames) - 1;
      pStringMap = aResolutionNames;
      // NOTE: Default to input because resolution names are untranslatable (ex: 180x180)
      pszReturn  = pszID;
      break;
   }

#ifdef INCLUDE_JP_COMMON_SCALING
   case STRINGGROUP_SCALINGS:
   {
      static STRINGMAP aScalingNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Clip",                       "Clip"              },
         { "FitToPage",                  "Fit to the Page"   },
         { "RotateAndOrFit",             "Rotate and or Fit" }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aScalingNames) / 2;
      iHigh      = (int)dimof (aScalingNames) - 1;
      pStringMap = aScalingNames;
      break;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   case STRINGGROUP_SHEET_COLLATES:
   {
      static STRINGMAP aSheetCollateNames[] = {
      /*   Translation ignore!           Translation start: */
         { "SheetAndJobCollated",        "Sheet and Job Collated" },
         { "SheetCollated",              "Sheet Collated"         },
         { "SheetUncollated",            "Sheet Uncollated"       }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aSheetCollateNames) / 2;
      iHigh      = (int)dimof (aSheetCollateNames) - 1;
      pStringMap = aSheetCollateNames;
      break;
   }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
   case STRINGGROUP_SIDES:
   {
      static STRINGMAP aSideNames[] = {
      /*   Translation ignore!           Translation start: */
         { "OneSidedBackflipX",          "One-sided Backflip on the X axis" },
         { "OneSidedBackflipY",          "One-sided Backflip on the Y axis" },
         { "OneSidedFront",              "One-sided on the Front"           },
         { "TwoSidedFlipX",              "Two-sided Flip on the X axis"     },
         { "TwoSidedFlipY",              "Two-sided Flip on the Y axis"     }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aSideNames) / 2;
      iHigh      = (int)dimof (aSideNames) - 1;
      pStringMap = aSideNames;
      break;
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   case STRINGGROUP_STITCHING_EDGES:
   {
      static STRINGMAP aStitchingEdgeNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Bottom",                     "Bottom"           },
         { "Left",                       "Left"             },
         { "Right",                      "Right"            },
         { "Top",                        "Top"              }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aStitchingEdgeNames) / 2;
      iHigh      = (int)dimof (aStitchingEdgeNames) - 1;
      pStringMap = aStitchingEdgeNames;
      break;
   }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
   case STRINGGROUP_STITCHING_TYPES:
   {
      static STRINGMAP aStitchingTypeNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Corner",                     "Corner"           },
         { "Saddle",                     "Saddle"           },
         { "Side",                       "Side"             }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aStitchingTypeNames) / 2;
      iHigh      = (int)dimof (aStitchingTypeNames) - 1;
      pStringMap = aStitchingTypeNames;
      break;
   }
#endif

   case STRINGGROUP_TRAYS:
   {
      static STRINGMAP aTrayNames[] = {
      /*   Translation ignore!           Translation start: */
         { "AnyLargeFormat",             "Any large format tray"  },
         { "AnySmallFormat",             "Any small format tray"  },
         { "AutoSelect",                 "Auto select tray"       },
         { "Bottom",                     "Bottom tray"            },
         { "BypassTray",                 "Bypass tray"            },
         { "BypassTray-1",               "Bypass tray #1"         },
         { "BypassTray-2",               "Bypass tray #2"         },
         { "BypassTray-3",               "Bypass tray #3"         },
         { "BypassTray-4",               "Bypass tray #4"         },
         { "BypassTray-5",               "Bypass tray #5"         },
         { "BypassTray-6",               "Bypass tray #6"         },
         { "BypassTray-7",               "Bypass tray #7"         },
         { "BypassTray-8",               "Bypass tray #8"         },
         { "BypassTray-9",               "Bypass tray #9"         },
         { "Continuous",                 "Continuous Tray"        },
         { "Disc",                       "Compact Disc/DVD"       },
         { "Disc-1",                     "Compact Disc/DVD #1"    },
         { "Disc-2",                     "Compact Disc/DVD #2"    },
         { "Disc-3",                     "Compact Disc/DVD #3"    },
         { "Disc-4",                     "Compact Disc/DVD #4"    },
         { "Disc-5",                     "Compact Disc/DVD #5"    },
         { "Disc-6",                     "Compact Disc/DVD #6"    },
         { "Disc-7",                     "Compact Disc/DVD #7"    },
         { "Disc-8",                     "Compact Disc/DVD #8"    },
         { "Disc-9",                     "Compact Disc/DVD #9"    },
         { "Envelope",                   "Envelope tray"          },
         { "Envelope-1",                 "Envelope tray #1"       },
         { "Envelope-2",                 "Envelope tray #2"       },
         { "Envelope-3",                 "Envelope tray #3"       },
         { "Envelope-4",                 "Envelope tray #4"       },
         { "Envelope-5",                 "Envelope tray #5"       },
         { "Envelope-6",                 "Envelope tray #6"       },
         { "Envelope-7",                 "Envelope tray #7"       },
         { "Envelope-8",                 "Envelope tray #8"       },
         { "Envelope-9",                 "Envelope tray #9"       },
         { "Front",                      "Front tray"             },
         { "InsertTray",                 "Insert tray"            },
         { "InsertTray-1",               "Insert tray #1"         },
         { "InsertTray-2",               "Insert tray #2"         },
         { "InsertTray-3",               "Insert tray #3"         },
         { "InsertTray-4",               "Insert tray #4"         },
         { "InsertTray-5",               "Insert tray #5"         },
         { "InsertTray-6",               "Insert tray #6"         },
         { "InsertTray-7",               "Insert tray #7"         },
         { "InsertTray-8",               "Insert tray #8"         },
         { "InsertTray-9",               "Insert tray #9"         },
         { "LargeCapacity",              "Large capacity tray"    },
         { "LargeCapacity-1",            "Large capacity tray #1" },
         { "LargeCapacity-2",            "Large capacity tray #2" },
         { "LargeCapacity-3",            "Large capacity tray #3" },
         { "LargeCapacity-4",            "Large capacity tray #4" },
         { "LargeCapacity-5",            "Large capacity tray #5" },
         { "LargeCapacity-6",            "Large capacity tray #6" },
         { "LargeCapacity-7",            "Large capacity tray #7" },
         { "LargeCapacity-8",            "Large capacity tray #8" },
         { "LargeCapacity-9",            "Large capacity tray #9" },
         { "Left",                       "Left tray"              },
         { "Middle",                     "Middle tray"            },
         { "PanelSelect",                "Panel Select"           },
         { "Rear",                       "Rear tray"              },
         { "Right",                      "Right tray"             },
         { "Roll",                       "Roll tray"              },
         { "Roll-1",                     "Roll tray #1"           },
         { "Roll-2",                     "Roll tray #2"           },
         { "Roll-3",                     "Roll tray #3"           },
         { "Roll-4",                     "Roll tray #4"           },
         { "Roll-5",                     "Roll tray #5"           },
         { "Roll-6",                     "Roll tray #6"           },
         { "Roll-7",                     "Roll tray #7"           },
         { "Roll-8",                     "Roll tray #8"           },
         { "Roll-9",                     "Roll tray #9"           },
         { "Side",                       "Side tray"              },
         { "Top",                        "Top tray"               },
         { "Tray",                       "Tray"                   },
         { "Tray-1",                     "Tray #1"                },
         { "Tray-2",                     "Tray #2"                },
         { "Tray-3",                     "Tray #3"                },
         { "Tray-4",                     "Tray #4"                },
         { "Tray-5",                     "Tray #5"                },
         { "Tray-6",                     "Tray #6"                },
         { "Tray-7",                     "Tray #7"                },
         { "Tray-8",                     "Tray #8"                },
         { "Tray-9",                     "Tray #9"                }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aTrayNames) / 2;
      iHigh      = (int)dimof (aTrayNames) - 1;
      pStringMap = aTrayNames;
      break;
   }

#ifdef INCLUDE_JP_COMMON_TRIMMING
   case STRINGGROUP_TRIMMINGS:
   {
      static STRINGMAP aTrimmingNames[] = {
      /*   Translation ignore!           Translation start: */
         { "Face",                       "Face"           },
         { "Gutter",                     "Gutter"         },
         { "None",                       "None"           },
         { "Tab",                        "Tab"            },
         { "Trim",                       "Trim"           }
      /*   Translation ignore!           Translation end! */
      };

      iLow       = 0;
      iMid       = (int)dimof (aTrimmingNames) / 2;
      iHigh      = (int)dimof (aTrimmingNames) - 1;
      pStringMap = aTrimmingNames;
      break;
   }
#endif

   default:
      return 0;
   }

   while (iLow <= iHigh)
   {
      iResult = strcmp (pszID, pStringMap[iMid].pszFrom);

      // DebugOutput::getErrorStream () << "strcmp (" << pszID << ", " << pStringMap[iMid].pszFrom << ") = " << iResult << std::endl;

      if (0 == iResult)
      {
         pszReturn = pStringMap[iMid].pszTo;
         break;
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

   if (fFreeID)
   {
      free ((void *)pszID);
   }

   return pszReturn;
}
