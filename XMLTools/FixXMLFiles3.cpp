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
      { "iso_2a0_11890.00x16820.00mm",                "iso_2a0_1189.00x1682.00mm"                },
      { "iso_4a0_16820.00x23780.00mm",                "iso_4a0_1682.00x2378.00mm"                },
      { "iso_a0_8410.00x11890.00mm",                  "iso_a0_841.00x1189.00mm"                  },
      { "iso_a10_260.00x370.00mm",                    "iso_a10_26.00x37.00mm"                    },
      { "iso_a1_5940.00x8410.00mm",                   "iso_a1_594.00x841.00mm"                   },
      { "iso_a2_4200.00x5940.00mm",                   "iso_a2_420.00x594.00mm"                   },
      { "iso_a3-extra_3220.00x4450.00mm",             "iso_a3-extra_322.00x445.00mm"             },
      { "iso_a3-wide_3302.00x4826.00mm",              "iso_a3-wide_330.20x482.60mm"              },
      { "iso_a3_2970.00x4200.00mm",                   "iso_a3_297.00x420.00mm"                   },
      { "iso_a4-extra_2355.00x3223.00mm",             "iso_a4-extra_235.50x322.30mm"             },
      { "iso_a4-tab_2250.00x2970.00mm",               "iso_a4-tab_225.00x297.00mm"               },
      { "iso_a4-wide_2235.00x3556.00mm",              "iso_a4-wide_223.50x355.60mm"              },
      { "iso_a4_2100.00x2970.00mm",                   "iso_a4_210.00x297.00mm"                   },
      { "iso_a5-extra_1740.00x2350.00mm",             "iso_a5-extra_174.00x235.00mm"             },
      { "iso_a5_1480.00x2100.00mm",                   "iso_a5_148.00x210.00mm"                   },
      { "iso_a6_1050.00x1480.00mm",                   "iso_a6_105.00x148.00mm"                   },
      { "iso_a7_740.00x1050.00mm",                    "iso_a7_74.00x105.00mm"                    },
      { "iso_a8_520.00x740.00mm",                     "iso_a8_52.00x74.00mm"                     },
      { "iso_a9_370.00x520.00mm",                     "iso_a9_37.00x52.00mm"                     },
      { "iso_b0_10000.00x14140.00mm",                 "iso_b0_1000.00x1414.00mm"                 },
      { "iso_b10_310.00x440.00mm",                    "iso_b10_31.00x44.00mm"                    },
      { "iso_b1_7070.00x10000.00mm",                  "iso_b1_707.00x1000.00mm"                  },
      { "iso_b2_5000.00x7070.00mm",                   "iso_b2_500.00x707.00mm"                   },
      { "iso_b3_3530.00x5000.00mm",                   "iso_b3_353.00x500.00mm"                   },
      { "iso_b4_2500.00x3530.00mm",                   "iso_b4_250.00x353.00mm"                   },
      { "iso_b5-extra_2010.00x2760.00mm",             "iso_b5-extra_201.00x276.00mm"             },
      { "iso_b5_1760.00x2500.00mm",                   "iso_b5_176.00x250.00mm"                   },
      { "iso_b6_1250.00x1760.00mm",                   "iso_b6_125.00x176.00mm"                   },
      { "iso_b6c4_1250.00x3240.00mm",                 "iso_b6c4_125.00x324.00mm"                 },
      { "iso_b7_880.00x1250.00mm",                    "iso_b7_88.00x125.00mm"                    },
      { "iso_b8_620.00x880.00mm",                     "iso_b8_62.00x88.00mm"                     },
      { "iso_b9_440.00x620.00mm",                     "iso_b9_44.00x62.00mm"                     },
      { "iso_c0_9170.00x12970.00mm",                  "iso_c0_917.00x1297.00mm"                  },
      { "iso_c10_280.00x400.00mm",                    "iso_c10_28.00x40.00mm"                    },
      { "iso_c1_6480.00x9170.00mm",                   "iso_c1_648.00x917.00mm"                   },
      { "iso_c2_4580.00x6480.00mm",                   "iso_c2_458.00x648.00mm"                   },
      { "iso_c3_3240.00x4580.00mm",                   "iso_c3_324.00x458.00mm"                   },
      { "iso_c4_2290.00x3240.00mm",                   "iso_c4_229.00x324.00mm"                   },
      { "iso_c5_1620.00x2290.00mm",                   "iso_c5_162.00x229.00mm"                   },
      { "iso_c6_1140.00x1620.00mm",                   "iso_c6_114.00x162.00mm"                   },
      { "iso_c6c5_1140.00x2290.00mm",                 "iso_c6c5_114.00x229.00mm"                 },
      { "iso_c7_810.00x1140.00mm",                    "iso_c7_81.00x114.00mm"                    },
      { "iso_c7c6_810.00x1620.00mm",                  "iso_c7c6_81.00x162.00mm"                  },
      { "iso_c8_570.00x810.00mm",                     "iso_c8_57.00x81.00mm"                     },
      { "iso_c9_400.00x570.00mm",                     "iso_c9_40.00x57.00mm"                     },
      { "iso_e1_7112.00x10160.00mm",                  "iso_e1_711.20x1016.00mm"                  },
      { "iso_ra0_8600.00x12200.00mm",                 "iso_ra0_860.00x1220.00mm"                 },
      { "iso_ra1_6100.00x8600.00mm",                  "iso_ra1_610.00x860.00mm"                  },
      { "iso_ra2_4300.00x6100.00mm",                  "iso_ra2_430.00x610.00mm"                  },
      { "iso_sra0_9000.00x12800.00mm",                "iso_sra0_900.00x1280.00mm"                },
      { "iso_sra1_6400.00x9000.00mm",                 "iso_sra1_640.00x900.00mm"                 },
      { "iso_sra2_4500.00x6400.00mm",                 "iso_sra2_450.00x640.00mm"                 },
      { "jis_b0_10300.00x14560.00mm",                 "jis_b0_1030.00x1456.00mm"                 },
      { "jis_b10_320.00x450.00mm",                    "jis_b10_32.00x45.00mm"                    },
      { "jis_b1_7280.00x10300.00mm",                  "jis_b1_728.00x1030.00mm"                  },
      { "jis_b2_5150.00x7280.00mm",                   "jis_b2_515.00x728.00mm"                   },
      { "jis_b3_3640.00x5150.00mm",                   "jis_b3_364.00x515.00mm"                   },
      { "jis_b4_2570.00x3640.00mm",                   "jis_b4_257.00x364.00mm"                   },
      { "jis_b5_1820.00x2570.00mm",                   "jis_b5_182.00x257.00mm"                   },
      { "jis_b6_1280.00x1820.00mm",                   "jis_b6_128.00x182.00mm"                   },
      { "jis_b7_910.00x1280.00mm",                    "jis_b7_91.00x128.00mm"                    },
      { "jis_b8_640.00x910.00mm",                     "jis_b8_64.00x91.00mm"                     },
      { "jis_b9_450.00x640.00mm",                     "jis_b9_45.00x64.00mm"                     },
      { "jis_exec_2160.00x3300.00mm",                 "jis_exec_216.00x330.00mm"                 },
      { "jpn_chou2_1111.00x1460.00mm",                "jpn_chou2_111.10x146.00mm"                },
      { "jpn_chou3_1200.00x2350.00mm",                "jpn_chou3_120.00x235.00mm"                },
      { "jpn_chou4_900.00x2050.00mm",                 "jpn_chou4_90.00x205.00mm"                 },
      { "jpn_hagaki_1000.00x1480.00mm",               "jpn_hagaki_100.00x148.00mm"               },
      { "jpn_kahu_2400.00x3221.00mm",                 "jpn_kahu_240.00x322.10mm"                 },
      { "jpn_kaku2_2400.00x3320.00mm",                "jpn_kaku2_240.00x332.00mm"                },
      { "jpn_oufuku_1480.00x2000.00mm",               "jpn_oufuku_148.00x200.00mm"               },
      { "jpn_you4_1050.00x2350.00mm",                 "jpn_you4_105.00x235.00mm"                 },
      { "na_170x210_1700.00x2100.00mm",               "na_170x210_170.00x210.00mm"               },
      { "na_180x210_1820.00x2100.00mm",               "na_180x210_182.00x210.00mm"               },
      { "na_card-148_1480.00x1050.00mm",              "na_card-148_148.00x105.00mm"              },
      { "na_german-12x250-fanfold_3048.00x2400.00mm", "na_german-12x250-fanfold_304.80x240.00mm" },
      { "na_panoramic_2100.00x5940.00mm",             "na_panoramic_210.00x594.00mm"             },
      { "na_photo-100x150_1000.00x1500.00mm",         "na_photo-100x150_100.00x150.00mm"         },
      { "na_photo-200x300_2000.00x3000.00mm",         "na_photo-200x300_200.00x300.00mm"         },
      { "na_super-a3-b_3289.20x4830.20mm",            "na_super-a3-b_328.92x483.02mm"            },
      { "na_super-a_2270.76x3556.00mm",               "na_super-a_227.08x355.60mm"               },
      { "na_super-b_3302.00x4826.00mm",               "na_super-b_330.20x482.60mm"               },
      { "om_dai-pa-kai_2750.00x3950.00mm",            "om_dai-pa-kai_275.00x395.00mm"            },
      { "om_folio-sp_2150.00x3150.00mm",              "om_folio-sp_215.00x315.00mm"              },
      { "om_folio_2159.00x3300.00mm",                 "om_folio_215.90x330.00mm"                 },
      { "om_invite_2200.00x2200.00mm",                "om_invite_220.00x220.00mm"                },
      { "om_italian_1000.00x2300.00mm",               "om_italian_100.00x230.00mm"               },
      { "om_juuro-ku-kai_1980.00x2750.00mm",          "om_juuro-ku-kai_198.00x275.00mm"          },
      { "om_pa-kai_2670.00x3890.00mm",                "om_pa-kai_267.00x389.00mm"                },
      { "om_postfix_1140.00x2290.00mm",               "om_postfix_114.00x229.00mm"               },
      { "prc_10_3240.00x4580.00mm",                   "prc_10_324.00x458.00mm"                   },
      { "prc_16k_1460.00x2150.00mm",                  "prc_16k_146.00x215.00mm"                  },
      { "prc_1_1020.00x1650.00mm",                    "prc_1_102.00x165.00mm"                    },
      { "prc_2_1020.00x1760.00mm",                    "prc_2_102.00x176.00mm"                    },
      { "prc_32k_970.00x1510.00mm",                   "prc_32k_97.00x151.00mm"                   },
      { "prc_3_1250.00x1760.00mm",                    "prc_3_125.00x176.00mm"                    },
      { "prc_4_1100.00x2080.00mm",                    "prc_4_110.00x208.00mm"                    },
      { "prc_5_1100.00x2200.00mm",                    "prc_5_110.00x220.00mm"                    },
      { "prc_6_1200.00x3200.00mm",                    "prc_6_120.00x320.00mm"                    },
      { "prc_7_1600.00x2300.00mm",                    "prc_7_160.00x230.00mm"                    },
      { "prc_8_1200.00x3090.00mm",                    "prc_8_120.00x309.00mm"                    },
      { "prc_9_2290.00x3240.00mm",                    "prc_9_229.00x324.00mm"                    },
      { "roc_16k_1968.50x2730.50mm",                  "roc_16k_196.85x273.05mm"                  },
      { "roc_8k_2730.50x3937.00mm",                   "roc_8k_273.05x393.70mm"                   },
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

bool
processForms (XmlNodePtr nodeForms,
              bool       fUpdate)
{
   XmlNodePtr nodeForm    = 0;
   XmlNodePtr nodeElement = 0;
   bool       fRet        = false;

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
               PSZRO pszNameText    = 0;
               PSZRO pszNewNameText = 0;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapFormName (pszNameText);

               if (pszNewNameText)
               {
                  std::cout << pszNameText << " -> " << pszNewNameText << std::endl;
               }

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);

                  fRet = true;
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
            if (0 == XMLStrcmp (XMLGetName (nodeElement), "Form"))
            {
               PSZRO pszNameText      = 0;
               PSZRO pszNewNameText   = 0;
               bool  fFreeNewNameText = false;

               // Get current name contents
               pszNameText = XMLNodeListGetString (XMLGetDocNode (nodeElement),
                                                   XMLGetChildrenNode (nodeElement),
                                                   1);

               pszNewNameText = mapFormName (pszNameText);

               if (  fUpdate
                  && pszNewNameText
                  )
               {
                  // Change the name to the newly mapped name
                  XMLNodeSetContent (nodeElement, pszNewNameText);

                  fRet = true;
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

   return true; //fRet;
}

bool
processFile (const char *pszFileName,
             bool        fUpdate)
{
   XmlDocPtr           doc             = 0;
   XmlNodePtr          nodeRoot        = 0;
   bool                fRC             = false;

   doc = XMLParseFile (pszFileName);

   if (vfDebug) std::cerr << "doc = " << std::hex << (int)doc << std::dec << std::endl;

   if (!doc)
   {
      return false;
   }

   nodeRoot = XMLFirstNode (XMLDocGetRootElement (doc));
   if (!nodeRoot)
   {
      return false;
   }
   else if (0 == XMLStrcmp ("deviceForms", XMLGetName (nodeRoot)))
   {
      fRC = processForms (nodeRoot, fUpdate);
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
