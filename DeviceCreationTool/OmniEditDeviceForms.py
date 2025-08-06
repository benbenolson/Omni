#!/usr/bin/python
#
#    IBM Omni driver
#    Copyright (c) International Business Machines Corp., 2000-2004
#
#    This library is free software; you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published
#    by the Free Software Foundation; either version 2.1 of the License, or
#    (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

from OmniEditUtils import *

import gtk
import gtk.glade
import gobject
import types

def getDefaultXML (jobProperties):
    return XMLHeader () + """

<deviceForms
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <deviceForm>
      <name>na_letter_8.50x11.00in</name>
      <formCapabilities>NO_CAPABILITIES</formCapabilities>
      <command>_NUL_</command>
      <hardCopyCap>
         <hardCopyCapLeft>0</hardCopyCapLeft>
         <hardCopyCapTop>0</hardCopyCapTop>
         <hardCopyCapRight>0</hardCopyCapRight>
         <hardCopyCapBottom>0</hardCopyCapBottom>
      </hardCopyCap>
   </deviceForm>
</deviceForms>"""

def isValidCapabilities (capabilities):
    return capabilities in [ "NO_CAPABILITIES",
                             "FORM_CAPABILITY_ROLL",
                             "FORM_CAPABILITY_USERDEFINED"
                           ]

def isValidName (name):
    if name == None:
        print "OmniEditDeviceForms.py::isValidName: Error: name is None"
        return False

    iPos1 = name.find ("_")

    if iPos1 == -1:
        print "OmniEditDeviceForms.py::isValidName: Error: cannot find a _ in name (%s)" % (name)
        return False

    iPos2 = name.find ("_", iPos1 + 1)

    if iPos2 == -1:
        fShortName = True
        pszSearch = name + "_"
        pszRest   = None
    else:
        pszSearch = name[0:iPos2 + 1]
        pszRest   = name[iPos2 + 1:]

#   if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#       print "OmniEditDeviceForms.py::isValidName: pszSearch =", pszSearch
    # @TBD - optimize. make global one time init
    prefix = {}
    prefix["asme_f_"                  ] = (    28,     40, False, "in") # asme_f_28x40in
    prefix["iso_2a0_"                 ] = (  1189,   1682, False, "mm") # iso_2a0_1189x1682mm
    prefix["iso_a0_"                  ] = (   841,   1189, False, "mm") # iso_a0_841x1189mm
    prefix["iso_a0x3_"                ] = (  1189,   2523, False, "mm") # iso_a0x3_1189x2523mm
    prefix["iso_a1_"                  ] = (   594,    841, False, "mm") # iso_a1_594x841mm
    prefix["iso_a10_"                 ] = (    26,     37, False, "mm") # iso_a10_26x37mm
    prefix["iso_a1x3_"                ] = (   841,   1783, False, "mm") # iso_a1x3_841x1783mm
    prefix["iso_a1x4_"                ] = (   841,   2378, False, "mm") # iso_a1x4_841x2378mm
    prefix["iso_a2_"                  ] = (   420,    594, False, "mm") # iso_a2_420x594mm
    prefix["iso_a2x3_"                ] = (   594,   1261, False, "mm") # iso_a2x3_594x1261mm
    prefix["iso_a2x4_"                ] = (   594,   1682, False, "mm") # iso_a2x4_594x1682mm
    prefix["iso_a2x5_"                ] = (   594,   2102, False, "mm") # iso_a2x5_594x2102mm
    prefix["iso_a3_"                  ] = (   297,    420, False, "mm") # iso_a3_297x420mm
    prefix["iso_a3-extra_"            ] = (   322,    445, False, "mm") # iso_a3-extra_322x445mm
    prefix["iso_a3x3_"                ] = (   420,    891, False, "mm") # iso_a3x3_420x891mm
    prefix["iso_a3x4_"                ] = (   420,   1189, False, "mm") # iso_a3x4_420x1189mm
    prefix["iso_a3x5_"                ] = (   420,   1486, False, "mm") # iso_a3x5_420x1486mm
    prefix["iso_a3x6_"                ] = (   420,   1783, False, "mm") # iso_a3x6_420x1783mm
    prefix["iso_a3x7_"                ] = (   420,   2080, False, "mm") # iso_a3x7_420x2080mm
    prefix["iso_a4_"                  ] = (   210,    297, False, "mm") # iso_a4_210x297mm
    prefix["iso_a4-extra_"            ] = ( 235.5,  322.3, False, "mm") # iso_a4-extra_235.5x322.3mm
    prefix["iso_a4-tab_"              ] = (   225,    297, False, "mm") # iso_a4-tab_225x297mm
    prefix["iso_a4x3_"                ] = (   297,    630, False, "mm") # iso_a4x3_297x630mm
    prefix["iso_a4x4_"                ] = (   297,    841, False, "mm") # iso_a4x4_297x841mm
    prefix["iso_a4x5_"                ] = (   297,   1051, False, "mm") # iso_a4x5_297x1051mm
    prefix["iso_a4x6_"                ] = (   297,   1261, False, "mm") # iso_a4x6_297x1261mm
    prefix["iso_a4x7_"                ] = (   297,   1471, False, "mm") # iso_a4x7_297x1471mm
    prefix["iso_a4x8_"                ] = (   297,   1682, False, "mm") # iso_a4x8_297x1682mm
    prefix["iso_a4x9_"                ] = (   297,   1892, False, "mm") # iso_a4x9_297x1892mm
    prefix["iso_a5_"                  ] = (   148,    210, False, "mm") # iso_a5_148x210mm
    prefix["iso_a5-extra_"            ] = (   174,    235, False, "mm") # iso_a5-extra_174x235mm
    prefix["iso_a6_"                  ] = (   105,    148, False, "mm") # iso_a6_105x148mm
    prefix["iso_a7_"                  ] = (    74,    105, False, "mm") # iso_a7_74x105mm
    prefix["iso_a8_"                  ] = (    52,     74, False, "mm") # iso_a8_52x74mm
    prefix["iso_a9_"                  ] = (    37,     52, False, "mm") # iso_a9_37x52mm
    prefix["iso_b0_"                  ] = (  1000,   1414, False, "mm") # iso_b0_1000x1414mm
    prefix["iso_b1_"                  ] = (   707,   1000, False, "mm") # iso_b1_707x1000mm
    prefix["iso_b10_"                 ] = (    31,     44, False, "mm") # iso_b10_31x44mm
    prefix["iso_b2_"                  ] = (   500,    707, False, "mm") # iso_b2_500x707mm
    prefix["iso_b3_"                  ] = (   353,    500, False, "mm") # iso_b3_353x500mm
    prefix["iso_b4_"                  ] = (   250,    353, False, "mm") # iso_b4_250x353mm
    prefix["iso_b5_"                  ] = (   176,    250, False, "mm") # iso_b5_176x250mm
    prefix["iso_b5-extra_"            ] = (   201,    276, False, "mm") # iso_b5-extra_201x276mm
    prefix["iso_b6_"                  ] = (   125,    176, False, "mm") # iso_b6_125x176mm
    prefix["iso_b6c4_"                ] = (   125,    324, False, "mm") # iso_b6c4_125x324mm
    prefix["iso_b7_"                  ] = (    88,    125, False, "mm") # iso_b7_88x125mm
    prefix["iso_b8_"                  ] = (    62,     88, False, "mm") # iso_b8_62x88mm
    prefix["iso_b9_"                  ] = (    44,     62, False, "mm") # iso_b9_44x62mm
    prefix["iso_c0_"                  ] = (   917,   1297, False, "mm") # iso_c0_917x1297mm
    prefix["iso_c1_"                  ] = (   648,    917, False, "mm") # iso_c1_648x917mm
    prefix["iso_c10_"                 ] = (    28,     40, False, "mm") # iso_c10_28x40mm
    prefix["iso_c2_"                  ] = (   458,    648, False, "mm") # iso_c2_458x648mm
    prefix["iso_c3_"                  ] = (   324,    458, False, "mm") # iso_c3_324x458mm
    prefix["iso_c4_"                  ] = (   229,    324, False, "mm") # iso_c4_229x324mm
    prefix["iso_c5_"                  ] = (   162,    229, False, "mm") # iso_c5_162x229mm
    prefix["iso_c6_"                  ] = (   114,    162, False, "mm") # iso_c6_114x162mm
    prefix["iso_c6c5_"                ] = (   114,    229, False, "mm") # iso_c6c5_114x229mm
    prefix["iso_c7_"                  ] = (    81,    114, False, "mm") # iso_c7_81x114mm
    prefix["iso_c7c6_"                ] = (    81,    162, False, "mm") # iso_c7c6_81x162mm
    prefix["iso_c8_"                  ] = (    57,     81, False, "mm") # iso_c8_57x81mm
    prefix["iso_c9_"                  ] = (    40,     57, False, "mm") # iso_c9_40x57mm
    prefix["iso_dl_"                  ] = (   110,    220, False, "mm") # iso_dl_110x220mm
    prefix["iso_ra0_"                 ] = (   860,   1220, False, "mm") # iso_ra0_860x1220mm
    prefix["iso_ra1_"                 ] = (   610,    860, False, "mm") # iso_ra1_610x860mm
    prefix["iso_ra2_"                 ] = (   430,    610, False, "mm") # iso_ra2_430x610mm
    prefix["iso_sra0_"                ] = (   900,   1280, False, "mm") # iso_sra0_900x1280mm
    prefix["iso_sra1_"                ] = (   640,    900, False, "mm") # iso_sra1_640x900mm
    prefix["iso_sra2_"                ] = (   450,    640, False, "mm") # iso_sra2_450x640mm
    prefix["jis_b0_"                  ] = (  1030,   1456, False, "mm") # jis_b0_1030x1456mm
    prefix["jis_b1_"                  ] = (   728,   1030, False, "mm") # jis_b1_728x1030mm
    prefix["jis_b10_"                 ] = (    32,     45, False, "mm") # jis_b10_32x45mm
    prefix["jis_b2_"                  ] = (   515,    728, False, "mm") # jis_b2_515x728mm
    prefix["jis_b3_"                  ] = (   364,    515, False, "mm") # jis_b3_364x515mm
    prefix["jis_b4_"                  ] = (   257,    364, False, "mm") # jis_b4_257x364mm
    prefix["jis_b5_"                  ] = (   182,    257, False, "mm") # jis_b5_182x257mm
    prefix["jis_b6_"                  ] = (   128,    182, False, "mm") # jis_b6_128x182mm
    prefix["jis_b7_"                  ] = (    91,    128, False, "mm") # jis_b7_91x128mm
    prefix["jis_b8_"                  ] = (    64,     91, False, "mm") # jis_b8_64x91mm
    prefix["jis_b9_"                  ] = (    45,     64, False, "mm") # jis_b9_45x64mm
    prefix["jis_exec_"                ] = (   216,    330, False, "mm") # jis_exec_216x330mm
    prefix["jpn_chou2_"               ] = ( 111.1,    146, False, "mm") # jpn_chou2_111.1x146mm
    prefix["jpn_chou3_"               ] = (   120,    235, False, "mm") # jpn_chou3_120x235mm
    prefix["jpn_chou4_"               ] = (    90,    205, False, "mm") # jpn_chou4_90x205mm
    prefix["jpn_hagaki_"              ] = (   100,    148, False, "mm") # jpn_hagaki_100x148mm
    prefix["jpn_kahu_"                ] = (   240,  322.1, False, "mm") # jpn_kahu_240x322.1mm
    prefix["jpn_kaku2_"               ] = (   240,    332, False, "mm") # jpn_kaku2_240x332mm
    prefix["jpn_oufuku_"              ] = (   148,    200, False, "mm") # jpn_oufuku_148x200mm
    prefix["jpn_you4_"                ] = (   105,    235, False, "mm") # jpn_you4_105x235mm
    prefix["na_10x11_"                ] = (    10,     11, False, "in") # na_10x11_10x11in
    prefix["na_10x13_"                ] = (    10,     13, False, "in") # na_10x13_10x13in
    prefix["na_10x14_"                ] = (    10,     14, False, "in") # na_10x14_10x14in
    prefix["na_10x15_"                ] = (    10,     15, False, "in") # na_10x15_10x15in
    prefix["na_10x15_"                ] = (    10,     15, False, "in") # na_10x15_10x15in
    prefix["na_11x12_"                ] = (    11,     12, False, "in") # na_11x12_11x12in
    prefix["na_11x15_"                ] = (    11,     15, False, "in") # na_11x15_11x15in
    prefix["na_12x19_"                ] = (    12,     19, False, "in") # na_12x19_12x19in
    prefix["na_5x7_"                  ] = (     5,      7, False, "in") # na_5x7_5x7in
    prefix["na_6x9_"                  ] = (     6,      9, False, "in") # na_6x9_6x9in
    prefix["na_7x9_"                  ] = (     7,      9, False, "in") # na_7x9_7x9in
    prefix["na_9x11_"                 ] = (     9,     11, False, "in") # na_9x11_9x11in
    prefix["na_a2_"                   ] = ( 4.375,   5.75, False, "in") # na_a2_4.375x5.75in
    prefix["na_arch-a_"               ] = (     9,     12, False, "in") # na_arch-a_9x12in
    prefix["na_arch-b_"               ] = (    12,     18, False, "in") # na_arch-b_12x18in
    prefix["na_arch-c_"               ] = (    18,     24, False, "in") # na_arch-c_18x24in
    prefix["na_arch-d_"               ] = (    24,     36, False, "in") # na_arch-d_24x36in
    prefix["na_arch-e_"               ] = (    36,     48, False, "in") # na_arch-e_36x48in
    prefix["na_b-plus_"               ] = (    12,  19.17, False, "in") # na_b-plus_12x19.17in
    prefix["na_c_"                    ] = (    17,     22, False, "in") # na_c_17x22in
    prefix["na_c5_"                   ] = (   6.5,    9.5, False, "in") # na_c5_6.5x9.5in
    prefix["na_d_"                    ] = (    22,     34, False, "in") # na_d_22x34in
    prefix["na_e_"                    ] = (    34,     44, False, "in") # na_e_34x44in
    prefix["na_edp_"                  ] = (    11,     14, False, "in") # na_edp_11x14in
    prefix["na_eur-edp_"              ] = (    12,     14, False, "in") # na_eur-edp_12x14in
    prefix["na_executive_"            ] = (  7.25,   10.5, False, "in") # na_executive_7.25x10.5in
    prefix["na_f_"                    ] = (    44,     68, False, "in") # na_f_44x68in
    prefix["na_fanfold-eur_"          ] = (   8.5,     12, False, "in") # na_fanfold-eur_8.5x12in
    prefix["na_fanfold-us_"           ] = (    11, 14.875, False, "in") # na_fanfold-us_11x14.875in
    prefix["na_foolscap_"             ] = (   8.5,     13, False, "in") # na_foolscap_8.5x13in
    prefix["na_govt-legal_"           ] = (     8,     13, False, "in") # na_govt-legal_8x13in
    prefix["na_govt-letter_"          ] = (     8,     10, False, "in") # na_govt-letter_8x10in
    prefix["na_index-3x5_"            ] = (     3,      5, False, "in") # na_index-3x5_3x5in
    prefix["na_index-4x6_"            ] = (     4,      6, False, "in") # na_index-4x6_4x6in
    prefix["na_index-4x6-ext_"        ] = (     6,      8, False, "in") # na_index-4x6-ext_6x8in
    prefix["na_index-5x8_"            ] = (     5,      8, False, "in") # na_index-5x8_5x8in
    prefix["na_invoice_"              ] = (   5.5,    8.5, False, "in") # na_invoice_5.5x8.5in
    prefix["na_ledger_"               ] = (    11,     17, False, "in") # na_ledger_11x17in
    prefix["na_legal_"                ] = (   8.5,     14, False, "in") # na_legal_8.5x14in
    prefix["na_legal-extra_"          ] = (   9.5,     15, False, "in") # na_legal-extra_9.5x15in
    prefix["na_letter_"               ] = (   8.5,     11, False, "in") # na_letter_8.5x11in
    prefix["na_letter-extra_"         ] = (   9.5,     12, False, "in") # na_letter-extra_9.5x12in
    prefix["na_letter-plus_"          ] = (   8.5,  12.69, False, "in") # na_letter-plus_8.5x12.69in
    prefix["na_monarch_"              ] = ( 3.875,    7.5, False, "in") # na_monarch_3.875x7.5in
    prefix["na_number-10_"            ] = ( 4.125,    9.5, False, "in") # na_number-10_4.125x9.5in
    prefix["na_number-11_"            ] = (   4.5, 10.375, False, "in") # na_number-11_4.5x10.375in
    prefix["na_number-12_"            ] = (  4.75,     11, False, "in") # na_number-12_4.75x11in
    prefix["na_number-14_"            ] = (     5,   11.5, False, "in") # na_number-14_5x11.5in
    prefix["na_number-9_"             ] = ( 3.875,  8.875, False, "in") # na_number-9_3.875x8.875in
    prefix["na_personal_"             ] = ( 3.625,    6.5, False, "in") # na_personal_3.625x6.5in
    prefix["na_quarto_"               ] = (   8.5,  10.83, False, "in") # na_quarto_8.5x10.83in
    prefix["na_super-a_"              ] = (  8.94,     14, False, "in") # na_super-a_8.94x14in
    prefix["na_super-b_"              ] = (    13,     19, False, "in") # na_super-b_13x19in
    prefix["na_wide-format_"          ] = (    30,     42, False, "in") # na_wide-format_30x42in
    prefix["om_dai-pa-kai_"           ] = (   275,    395, False, "mm") # om_dai-pa-kai_275x395mm
    prefix["om_folio_"                ] = (   210,    330, False, "mm") # om_folio_210x330mm
    prefix["om_folio-sp_"             ] = (   215,    315, False, "mm") # om_folio-sp_215x315mm
    prefix["om_invite_"               ] = (   220,    220, False, "mm") # om_invite_220x220mm
    prefix["om_italian_"              ] = (   110,    230, False, "mm") # om_italian_110x230mm
    prefix["om_juuro-ku-kai_"         ] = (   198,    275, False, "mm") # om_juuro-ku-kai_198x275mm
    prefix["om_large-photo_"          ] = (   200,    300, False, "mm") # om_large-photo_200x300mm
    prefix["om_pa-kai_"               ] = (   267,    389, False, "mm") # om_pa-kai_267x389mm
    prefix["om_postfix_"              ] = (   114,    229, False, "mm") # om_postfix_114x229mm
    prefix["om_small-photo_"          ] = (   100,    150, False, "mm") # om_small-photo_100x150mm
    prefix["prc_1_"                   ] = (   102,    165, False, "mm") # prc_1_102x165mm
    prefix["prc_10_"                  ] = (   324,    458, False, "mm") # prc_10_324x458mm
    prefix["prc_16k_"                 ] = (   146,    215, False, "mm") # prc_16k_146x215mm
    prefix["prc_2_"                   ] = (   102,    176, False, "mm") # prc_2_102x176mm
    prefix["prc_3_"                   ] = (   125,    176, False, "mm") # prc_3_125x176mm
    prefix["prc_32k_"                 ] = (    97,    151, False, "mm") # prc_32k_97x151mm
    prefix["prc_4_"                   ] = (   110,    208, False, "mm") # prc_4_110x208mm
    prefix["prc_5_"                   ] = (   110,    220, False, "mm") # prc_5_110x220mm
    prefix["prc_6_"                   ] = (   120,    320, False, "mm") # prc_6_120x320mm
    prefix["prc_7_"                   ] = (   160,    230, False, "mm") # prc_7_160x230mm
    prefix["prc_8_"                   ] = (   120,    309, False, "mm") # prc_8_120x309mm
    prefix["roc_16k_"                 ] = (  7.75,  10.75, False, "in") # roc_16k_7.75x10.75in
    prefix["roc_8k_"                  ] = ( 10.75,   15.5, False, "in") # roc_8k_10.75x15.5in
    prefix["na_half-letter_"          ] = (   5.5,    8.5, False, "in") # na_half-letter_5.50x8.50in               # @ADD
    prefix["na_a6-card_"              ] = (  4.13,   5.83, False, "in") # na_a6-card_4.13x5.83in                   # @ADD
    prefix["na_5x8-card_"             ] = (     5,      8, False, "in") # na_5x8-card_5.00x8.00in                  # @ADD
    prefix["na_8x10-card_"            ] = (     8,     10, False, "in") # na_5x10-card_5.00x10.00in                # @ADD
    prefix["na_c10_"                  ] = (  4.12,    9.5, False, "in") # na_c10_4.12x9.50in                       # @ADD
    prefix["na_dl_"                   ] = (  4.33,   8.66, False, "in") # na_dl_4.33x8.66in                        # @ADD
    prefix["na_c6_"                   ] = (  4.49,   6.38, False, "in") # na_c6_4.49x6.38in                        # @ADD
    prefix["na_envelope-132x220_"     ] = (   5.2,   8.66, False, "in") # na_envelope-132x220_5.20x8.66in          # @ADD
    prefix["na_photo-4x6_"            ] = (     4,      6, False, "in") # na_photo-4x6_4.00x6.00in                 # @ADD
    prefix["na_photo-100x150_"        ] = (   100,    150, False, "mm") # na_photo-100x150_100.00x150.00mm         # @ADD
    prefix["na_photo-200x300_"        ] = (   200,    300, False, "mm") # na_photo-200x300_200.00x300.00mm         # @ADD
    prefix["na_panoramic_"            ] = (   210,    594, False, "mm") # na_panoramic_210.00x594.00mm             # @ADD
    prefix["na_ledger_"               ] = (    17,     11, False, "in") # na_ledger_17.00x11.00in                  # @ADD
    prefix["na_statement_"            ] = (   5.5,    8.5, False, "in") # na_statement_5.50x8.50in                 # @ADD
    prefix["na_tabloid_"              ] = (    11,     17, False, "in") # na_tabloid_11.00x17.00in                 # @ADD
    prefix["na_universal_"            ] = ( 11.69,     17, False, "in") # na_universal_11.69x17.00in               # @ADD
    prefix["na_foolscap_"             ] = (     8,     13, False, "in") # na_foolscap_8.00x13.00in                 # @ADD
    prefix["om_folio_"                ] = ( 215.9,    330, False, "mm") # om_folio_215.90x330.00mm                 # @ADD
    prefix["na_eur-edp_"              ] = ( 11.57,     14, False, "in") # na_eur-edp_11.57x14.00in                 # @ADD
#   prefix["na_wide-format2_"         ] = ( 13.58,     11, False, "in") # na_wide-format2_13.58x11in               # @ADD
    prefix["na_letter-wide_"          ] = (  8.99,   13.3, False, "in") # na_letter-wide_8.99x13.30in              # @ADD
    prefix["na_foolscap-wide_"        ] = (   8.5,     13, False, "in") # na_foolscap-wide_8.50x13.00in            # @ADD
    prefix["iso_a3-wide_"             ] = ( 330.2,  482.6, False, "mm") # iso_a3-wide_330.20x482.60mm              # @ADD
    prefix["iso_a4-wide_"             ] = ( 223.5,  355.6, False, "mm") # iso_a4-wide_223.50x355.60mm              # @ADD
    prefix["na_15x11_"                ] = (    15,     11, False, "in") # na_15x11_15.00x11.00in                   # @ADD
    prefix["na_3x5-card_"             ] = (     3,      5, False, "in") # na_3x5-card_3.00x5.00in                  # @ADD
    prefix["na_4x6-card_"             ] = (     4,      6, False, "in") # na_3x5-card_4.00x6.00in                  # @ADD
    prefix["na_card-148_"             ] = (   148,    105, False, "mm") # na_card-148_148.00x105.00mm              # @ADD
    prefix["na_postcard_"             ] = (  3.93,   5.79, False, "in") # na_postcard_3.93x5.79in                  # @ADD
    prefix["na_a2_"                   ] = (  4.38,   5.75, False, "in") # na_a2_4.38x5.75in                        # @ADD
    prefix["na_c5_"                   ] = (  6.38,   9.02, False, "in") # na_c5_6.38x9.02in                        # @ADD
    prefix["na_c7_"                   ] = (  3.87,   7.50, False, "in") # na_c7_3.87x7.50in                        # @ADD
    prefix["na_c9_"                   ] = (  3.87,   8.87, False, "in") # na_c9_3.87x8.87in                        # @ADD
    prefix["na_d5_"                   ] = (  6.93,   9.84, False, "in") # na_d5_6.93x9.84in                        # @ADD
    prefix["na_envelope-6x9_"         ] = (     6,      9, False, "in") # na_envelope-6x9_6.00x9.00in              # @ADD
    prefix["na_envelope-7x9_"         ] = (     7,      9, False, "in") # na_envelope-7x9_7.00x9.00in              # @ADD
    prefix["na_envelope-6.5_"         ] = (   6.5,   3.62, False, "in") # na_envelope-6.5_6.50x3.62in              # @ADD
    prefix["na_envelope-9x11_"        ] = (     9,     11, False, "in") # na_envelope-9x11_9.00x11.00in            # @ADD
    prefix["na_envelope-10x13_"       ] = (    10,     13, False, "in") # na_envelope-10x13_10.00x13.00in          # @ADD
    prefix["na_envelope-10x14_"       ] = (    10,     14, False, "in") # na_envelope-10x14_10.00x14.00in          # @ADD
    prefix["na_envelope-10x15_"       ] = (    10,     15, False, "in") # na_envelope-10x15_10.00x15.00in          # @ADD
    prefix["na_monarch_"              ] = (  3.88,    7.5, False, "in") # na_monarch_3.88x7.50in                   # @ADD
    prefix["na_personal_"             ] = (  3.62,    6.5, False, "in") # na_personal_3.62x6.50in                  # @ADD
    prefix["na_number-9_"             ] = (  3.88,   8.88, False, "in") # na_number-9_3.88x8.88in                  # @ADD
    prefix["na_number-10_"            ] = (  4.12,    9.5, False, "in") # na_number-10_4.12x9.50in                 # @ADD
    prefix["na_number-11_"            ] = (   4.5,  10.38, False, "in") # na_number-11_4.50x10.38in                # @ADD
    prefix["om_italian_"              ] = (   100,    230, False, "mm") # om_italian_100.00x230.00mm               # @ADD
    prefix["prc_9_"                   ] = (   229,    324, False, "mm") # prc_9_229.00x324.00mm                    # @ADD
    prefix["na_disk-labels_"          ] = (  2.13,   2.76, False, "in") # na_disk-labels_2.13x2.76in               # @ADD
    prefix["na_euro-labels_"          ] = (  1.42,    3.5, False, "in") # na_euro-labels_1.42x3.50in               # @ADD
    prefix["na_shipping-labels_"      ] = (  2.13,   3.98, False, "in") # na_shipping-labels_2.13x3.98in           # @ADD
    prefix["na_standard-labels-clear_"] = (  1.10,    3.5, False, "in") # na_standard-labels-clear_1.10x3.50in     # @ADD
    prefix["na_standard-labels-white_"] = (  1.10,    3.5, False, "in") # na_standard-labels-white_1.10x3.50in     # @ADD
    prefix["iso_4a0_"                 ] = (  1682,   2378, False, "mm") # iso_4a0_1682.00x2378.00mm                # @ADD
    prefix["iso_e1_"                  ] = ( 711.2,   1016, False, "mm") # iso_e1_711.20x1016.00mm                  # @ADD
    prefix["na_fanfold-us_"           ] = ( 14.88,     11, False, "in") # na_fanfold-us_14.88x11.00in              # @ADD
    prefix["na_fanfold-1_"            ] = (  14.5,     11, False, "in") # na_fanfold-1_14.50x11.00in               # @ADD
    prefix["na_fanfold-2_"            ] = (    12,    8.5, False, "in") # na_fanfold-2_12.00x8.50in                # @ADD
    prefix["na_fanfold-3_"            ] = (   9.5,     11, False, "in") # na_fanfold-3_9.50x11.00in                # @ADD
    prefix["na_fanfold-4_"            ] = (   8.5,     12, False, "in") # na_fanfold-4_8.50x12.00in                # @ADD
    prefix["na_fanfold-5_"            ] = (   8.5,     11, False, "in") # na_fanfold-5_8.50x11.00in                # @ADD
    prefix["na_german-12x250-fanfold_"] = ( 304.8,    240, False, "mm") # na_german-12x250-fanfold_304.80x240.00mm # @ADD
    prefix["na_german-legal-fanfold_" ] = (   8.5,     13, False, "in") # na_german-legal-fanfold_8.50x13.00in     # @ADD
    prefix["na_plotter-size-a_"       ] = (   8.5,     11, False, "in") # na_plotter-size-a_8.50x11.00in           # @ADD
    prefix["na_plotter-size-b_"       ] = (    11,     17, False, "in") # na_plotter-size-b_11.00x17.00in          # @ADD
    prefix["na_plotter-size-c_"       ] = (    17,     22, False, "in") # na_plotter-size-c_17.00x22.00in          # @ADD
    prefix["na_plotter-size-d_"       ] = (    22,     34, False, "in") # na_plotter-size-d_22.00x34.00in          # @ADD
    prefix["na_plotter-size-e_"       ] = (    34,     44, False, "in") # na_plotter-size-e_34.00x44.00in          # @ADD
    prefix["na_plotter-size-f_"       ] = (    44,     68, False, "in") # na_plotter-size-f_44.00x68.00in          # @ADD
    prefix["na_super-a3-b_"           ] = (328.92, 483.02, False, "mm") # na_super-a3-b_328.92x483.02mm            # @ADD
    prefix["na_super-a_"              ] = (227.08,  355.6, False, "mm") # na_super-a_227.08x355.60mm               # @ADD
    prefix["na_8.5x12.4_"             ] = (   8.5,   12.4, False, "in") # na_8.5x12.4_8.50x12.40in                 # @ADD
    prefix["na_8x10.5_"               ] = (     8,   10.5, False, "in") # na_8x10.5_8.00x10.50in                   # @ADD
    prefix["na_8.25x13_"              ] = (  8.25,     13, False, "in") # na_8.25x13_8.25x13.00in                  # @ADD
    prefix["na_170x210_"              ] = (   170,    210, False, "mm") # na_170x210_170.00x210.00mm               # @ADD
    prefix["na_180x210_"              ] = (   182,    210, False, "mm") # na_180x210_182.00x210.00mm               # @ADD @WEIRD
    prefix["na_roll-76.2_"            ] = (   762,      0,  True, "mm") # na_roll-76.2_762.00x0.00mm               # @ADD
    prefix["na_roll-69.5_"            ] = (   695,      0,  True, "mm") # na_roll-69.5_695.00x0.00mm               # @ADD

    try:
        (iX, iY, fRoll, pszUnits) = prefix[pszSearch]
    except Exception, e:
        print "OmniEditDeviceForms.py::isValidName: Error: Caught exception", e
        return False

    pszEnd = name[-len(pszUnits):]
#   if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#       print "OmniEditDeviceForms.py::isValidName: pszEnd  =", pszEnd

    if pszRest != None:
        pszRest = pszRest[:-len(pszUnits)]
#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: pszRest =", pszRest

        iPosX = pszRest.find ("x")
#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: iPosX =", iPosX

        if iPosX == -1:
            print "OmniEditDeviceForms.py::isValidName: Error: cannot find a x in pszRest (%s)" % (pszRest)
            return False

        if pszRest.find ("x", iPosX + 1) != -1:
            print "OmniEditDeviceForms.py::isValidName: Error: found many x's in pszRest (%s)" % (pszRest)
            return False

        pszNumX = pszRest[:iPosX]
        pszNumY = pszRest[iPosX + 1:]
#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: pszNumX =", pszNumX
#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: pszNumY =", pszNumY

        try:
            flNumX = float (pszNumX)
            flNumY = float (pszNumY)
        except Exception, e:
            print "OmniEditDeviceForms.py::isValidName: Error: Caught exception", e
            return False

#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: flNumX =", flNumX
#       if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#           print "OmniEditDeviceForms.py::isValidName: flNumY =", flNumY

        if pszUnits != pszEnd:
#           if shouldOutputDebug ("OmniEditDeviceForms.py::isValidName"):
#               print "OmniEditDeviceForms.py::isValidName: Units mismatch %s vs %s" % (pszUnits, pszEnd)

            if pszEnd == "mm":
                flNumX = (float (int ((flNumX / 25.4) * 1000000))) / 1000000
                flNumY = (float (int ((flNumY / 25.4) * 1000000))) / 1000000
            elif pszEnd == "in":
                flNumX = (float (int ((flNumX * 25.4) * 1000000))) / 1000000
                flNumY = (float (int ((flNumY * 25.4) * 1000000))) / 1000000

        if fRoll:
            iY = flNumY

        if    flNumX != iX \
           or flNumY != iY:
            print "OmniEditDeviceForms.py::isValidName: Error: Number mismatch [%s%s] (%f, %f) vs (%f, %f) = (%d, %d)" % (pszSearch, pszRest, flNumX, flNumY, iX, iY, flNumX == iX, flNumY == iY)

        return     (flNumX == iX) \
               and (flNumY == iY)
    else:
        return True

def isValidOmniName (omniName):
    if omniName == None:
        return True

    return omniName in [ "FORM_UNLISTED",
                         "FORM_NONE",
                         "FORM_USER_DEFINED",
                         "FORM_TEST",
                         "FORM_TEST2",
                         "FORM_LEDGER",
                         "FORM_LEGAL",
                         "FORM_LEGAL_EXTRA",
                         "FORM_LETTER",
                         "FORM_LETTER_PLUS",
                         "FORM_LETTER_EXTRA",
                         "FORM_HALF_LETTER",
                         "FORM_EXECUTIVE",
                         "FORM_STATEMENT",
                         "FORM_TABLOID",
                         "FORM_UNIVERSAL",
                         "FORM_FOOLSCAP",
                         "FORM_QUARTO",
                         "FORM_FOLIO",
                         "FORM_FOLIO_SP",
                         "FORM_ROC_8K",
                         "FORM_ROC_16K",
                         "FORM_GOVERNMENT_LETTER",
                         "FORM_GOVERNMENT_LEGAL",
                         "FORM_EDP",
                         "FORM_EUROPEAN_EDP",
                         "FORM_PRC_16K",
                         "FORM_PRC_32K",
                         "FORM_DAI_PA_KAI",
                         "FORM_JUURO_KU_KAI",
                         "FORM_PA_KAI",
                         "FORM_WIDE",
                         "FORM_LETTER_WIDE",
                         "FORM_FOOLSCAP_WIDE",
                         "FORM_A3_WIDE",
                         "FORM_A4_WIDE",
                         "FORM_WIDE_FORMAT",
                         "FORM_10_X_11",
                         "FORM_11_X_12",
                         "FORM_11_X_15",
                         "FORM_12_X_19",
                         "FORM_15_X_11",
                         "FORM_5_X_7",
                         "FORM_3_X_5_CARD",
                         "FORM_4_X_6_CARD",
                         "FORM_5_X_8_CARD",
                         "FORM_8_X_10_CARD",
                         "FORM_A6_CARD",
                         "FORM_CARD_148",
                         "FORM_HAGAKI_CARD",
                         "FORM_OUFUKU_CARD",
                         "FORM_POSTCARD",
                         "FORM_INDEX_4_X_6_EXT",
                         "FORM_A2_ENVELOPE",
                         "FORM_C5_ENVELOPE",
                         "FORM_C6_ENVELOPE",
                         "FORM_C7_ENVELOPE",
                         "FORM_C9_ENVELOPE",
                         "FORM_C10_ENVELOPE",
                         "FORM_D5_ENVELOPE",
                         "FORM_DL_ENVELOPE",
                         "FORM_ENVELOPE_132_X_220",
                         "FORM_ENVELOPE_6_X_9",
                         "FORM_ENVELOPE_7_X_9",
                         "FORM_ENVELOPE_6_1_2",
                         "FORM_ENVELOPE_9_X_11",
                         "FORM_ENVELOPE_10_X_13",
                         "FORM_ENVELOPE_10_X_14",
                         "FORM_ENVELOPE_10_X_15",
                         "FORM_MONARCH_ENVELOPE",
                         "FORM_PERSONAL_ENVELOPE",
                         "FORM_NUMBER_9_ENVELOPE",
                         "FORM_NUMBER_10_ENVELOPE",
                         "FORM_NUMBER_11_ENVELOPE",
                         "FORM_NUMBER_12_ENVELOPE",
                         "FORM_NUMBER_14_ENVELOPE",
                         "FORM_ITALIAN_ENVELOPE",
                         "FORM_POSTFIX_ENVELOPE",
                         "FORM_INVITE_ENVELOPE",
                         "FORM_CHOU2_ENVELOPE",
                         "FORM_CHOU3_ENVELOPE",
                         "FORM_CHOU4_ENVELOPE",
                         "FORM_KAHU_ENVELOPE",
                         "FORM_KAKU2_ENVELOPE",
                         "FORM_YOU4_ENVELOPE",
                         "FORM_PRC1_ENVELOPE",
                         "FORM_PRC2_ENVELOPE",
                         "FORM_PRC3_ENVELOPE",
                         "FORM_PRC4_ENVELOPE",
                         "FORM_PRC5_ENVELOPE",
                         "FORM_PRC6_ENVELOPE",
                         "FORM_PRC7_ENVELOPE",
                         "FORM_PRC8_ENVELOPE",
                         "FORM_PRC9_ENVELOPE",
                         "FORM_PRC10_ENVELOPE",
                         "FORM_DISK_LABELS",
                         "FORM_EURO_LABELS",
                         "FORM_SHIPPING_LABELS",
                         "FORM_STANDARD_LABELS_CLEAR",
                         "FORM_STANDARD_LABELS_WHITE",
                         "FORM_2A0",
                         "FORM_4A0",
                         "FORM_A0",
                         "FORM_A1",
                         "FORM_A2",
                         "FORM_A3",
                         "FORM_A3_EXTRA",
                         "FORM_A4",
                         "FORM_A4_EXTRA",
                         "FORM_A4_TAB",
                         "FORM_A5",
                         "FORM_A5_EXTRA",
                         "FORM_A6",
                         "FORM_A7",
                         "FORM_A8",
                         "FORM_A9",
                         "FORM_A10",
                         "FORM_B_PLUS",
                         "FORM_B0",
                         "FORM_B1",
                         "FORM_B2",
                         "FORM_B3",
                         "FORM_B4",
                         "FORM_B5",
                         "FORM_B5_EXTRA",
                         "FORM_B6",
                         "FORM_B6_C4",
                         "FORM_B7",
                         "FORM_B8",
                         "FORM_B9",
                         "FORM_B10",
                         "FORM_JIS_B0",
                         "FORM_JIS_B1",
                         "FORM_JIS_B2",
                         "FORM_JIS_B3",
                         "FORM_JIS_B4",
                         "FORM_JIS_B5",
                         "FORM_JIS_B6",
                         "FORM_JIS_B7",
                         "FORM_JIS_B8",
                         "FORM_JIS_B9",
                         "FORM_JIS_B10",
                         "FORM_JIS_EXEC",
                         "FORM_C0",
                         "FORM_C1",
                         "FORM_C2",
                         "FORM_C3",
                         "FORM_C4",
                         "FORM_C5",
                         "FORM_C6",
                         "FORM_C6_C5",
                         "FORM_C7",
                         "FORM_C7_C6",
                         "FORM_C8",
                         "FORM_C9",
                         "FORM_C10",
                         "FORM_E1",
                         "FORM_RA0",
                         "FORM_RA1",
                         "FORM_RA2",
                         "FORM_SRA0",
                         "FORM_SRA1",
                         "FORM_SRA2",
                         "FORM_US_FANFOLD",
                         "FORM_FANFOLD_1",
                         "FORM_FANFOLD_2",
                         "FORM_FANFOLD_3",
                         "FORM_FANFOLD_4",
                         "FORM_FANFOLD_5",
                         "FORM_GERMAN_LEGAL_FANFOLD",
                         "FORM_EUROPEAN_FANFOLD",
                         "FORM_PANORAMIC",
                         "FORM_PHOTO_100_150",
                         "FORM_PHOTO_200_300",
                         "FORM_PHOTO_4_6",
                         "FORM_PLOTTER_SIZE_A",
                         "FORM_PLOTTER_SIZE_B",
                         "FORM_PLOTTER_SIZE_C",
                         "FORM_PLOTTER_SIZE_D",
                         "FORM_PLOTTER_SIZE_E",
                         "FORM_PLOTTER_SIZE_F",
                         "FORM_US_A_ARCHITECTURAL",
                         "FORM_US_B_ARCHITECTURAL",
                         "FORM_US_C_ARCHITECTURAL",
                         "FORM_US_D_ARCHITECTURAL",
                         "FORM_US_E_ARCHITECTURAL",
                         "FORM_SUPER_A3_B",
                         "FORM_SUPER_A",
                         "FORM_SUPER_B",
                         "FORM_8_5_X_12_4",
                         "FORM_8_X_10_5",
                         "FORM_8_25_X_13",
                         "FORM_170_X_210",
                         "FORM_182_X_210",
                         "FORM_ROLL_76_2MM",
                         "FORM_ROLL_69_5MM",
                         "FORM_GERMAN_12_X_250_FANFOLD"
                       ]

class OmniEditDeviceForm:
    def __init__ (self, form):
        if type (form) == types.InstanceType:
            self.setName (form.getName ())
            self.setOmniName (form.getOmniName ())
            self.setCapabilities (form.getCapabilities ())
            self.setCommand (form.getCommand ())
            self.setHccLeft (form.getHccLeft ())
            self.setHccTop (form.getHccTop ())
            self.setHccRight (form.getHccRight ())
            self.setHccBottom (form.getHccBottom ())
            self.setDeviceID (form.getDeviceID ())
        elif type (form) == types.ListType or type (form) == types.TupleType:
            if len (form) != 6:
                raise Exception ("Error: OmniEditDeviceForm: expecting 6 elements, received " + str (len (form)) + " of " + str (form))
            if not self.setName (form[0]):
                raise Exception ("Error: OmniEditDeviceForm: can't set name (" + form[0] + ") !")
            if not self.setOmniName (form[1]):
                raise Exception ("Error: OmniEditDeviceForm: can't set omniName (" + form[1] + ") !")
            if not self.setCapabilities (form[2]):
                raise Exception ("Error: OmniEditDeviceForm: can't set capabilities (" + form[2] + ") !")
            if not self.setCommand (form[3]):
                raise Exception ("Error: OmniEditDeviceForm: can't set command (" + form[3] + ") !")
            if not self.setHccLeft (form[4][0]):
                raise Exception ("Error: OmniEditDeviceForm: can't set hccLeft (" + form[4][0] + ") !")
            if not self.setHccTop (form[4][1]):
                raise Exception ("Error: OmniEditDeviceForm: can't set hccTop (" + form[4][1] + ") !")
            if not self.setHccRight (form[4][2]):
                raise Exception ("Error: OmniEditDeviceForm: can't set hccRight (" + form[4][2] + ") !")
            if not self.setHccBottom (form[4][3]):
                raise Exception ("Error: OmniEditDeviceForm: can't set hccBottom (" + form[4][3] + ") !")
            if not self.setDeviceID (form[5]):
                raise Exception ("Error: OmniEditDeviceForm: can't set deviceID (" + form[5] + ") !")
        else:
            raise Exception ("Expecting OmniEditDeviceForm, list, or tuple.  Got " + str (type (form)))

    def getName (self):
        return self.name

    def setName (self, name):
        if isValidName (name):
            self.name = name
            return True
        else:
            print "OmniEditDeviceForm::setName: Error: (%s) is not a valid name!" % (name)
            return False

    def getOmniName (self):
        return self.omniName

    def setOmniName (self, omniName):
        if omniName == "":
            omniName = None

        if isValidOmniName (omniName):
            self.omniName = omniName
            return True
        else:
            print "OmniEditDeviceForm::setOmniName: Error: (%s) is not a valid omni name!" % (omniName)
            return False

    def getCapabilities (self):
        return self.capabilities

    def setCapabilities (self, capabilities):
        if isValidCapabilities (capabilities):
            self.capabilities = capabilities
            return True
        else:
            print "OmniEditDeviceForm::setCapabilities: Error: (%s) is not a valid capability!" % (capabilities)
            return False

    def getCommand (self):
        return self.command

    def setCommand (self, command):
        if getCommandString (command) != None:
            self.command = command
            return True
        else:
            print "OmniEditDeviceForm::setCommand: Error: (%s) is not a valid command!" % (command)
            return False

    def getHccLeft (self):
        return self.hccLeft

    def setHccLeft (self, hccLeft):
        try:
            self.hccLeft = convertToIntegerValue (hccLeft)
            return True
        except Exception, e:
            try:
                self.hccLeft = convertToFloatValue (hccLeft)
                return True
            except Exception, e:
                print "OmniEditDeviceForm::setHccLeft: Error: (%s) is not an integer or float!" % (hccLeft)
                return False

    def getHccTop (self):
        return self.hccTop

    def setHccTop (self, hccTop):
        try:
            self.hccTop = convertToIntegerValue (hccTop)
            return True
        except Exception, e:
            try:
                self.hccTop = convertToFloatValue (hccTop)
                return True
            except Exception, e:
                print "OmniEditDeviceForm::setHccTop: Error: (%s) is not an integer or float!" % (hccTop)
                return False

    def getHccRight (self):
        return self.hccRight

    def setHccRight (self, hccRight):
        try:
            self.hccRight = convertToIntegerValue (hccRight)
            return True
        except Exception, e:
            try:
                self.hccRight = convertToFloatValue (hccRight)
                return True
            except Exception, e:
                print "OmniEditDeviceForm::setHccRight: Error: (%s) is not an integer or float!" % (hccRight)
                return False

    def getHccBottom (self):
        return self.hccBottom

    def setHccBottom (self, hccBottom):
        try:
            self.hccBottom = convertToIntegerValue (hccBottom)
            return True
        except Exception, e:
            try:
                self.hccBottom = convertToFloatValue (hccBottom)
                return True
            except Exception, e:
                print "OmniEditDeviceForm::setHccBottom: Error: (%s) is not an integer or float!" % (hccBottom)
                return False

    def getDeviceID (self):
        return self.deviceID

    def setDeviceID (self, deviceID):
        if deviceID == "":
            deviceID = None

        self.deviceID = deviceID
        return True

    def setDeviceForm (self, form):
        try:
            if type (form) == types.InstanceType:
                self.setName (form.getName ())
                self.setOmniName (form.getOmniName ())
                self.setCapabilities (form.getCapabilities ())
                self.setCommand (form.getCommand ())
                self.setHccLeft (form.getHccLeft ())
                self.setHccTop (form.getHccTop ())
                self.setHccRight (form.getHccRight ())
                self.setHccBottom (form.getHccBottom ())
                self.setDeviceID (form.getDeviceID ())
            else:
                print "OmniEditDeviceForm::setDeviceForm: Error: Expecting OmniEditDeviceForm.  Got ", str (type (form))
                return False
        except Exception, e:
            print "OmniEditDeviceForm::setDeviceForm: Error: caught " + e
            return False

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s, %s, %s, %s, %s]" % (self.getName (),
                                                        self.getOmniName (),
                                                        self.getCapabilities (),
                                                        self.getCommand (),
                                                        self.getHccLeft (),
                                                        self.getHccTop (),
                                                        self.getHccRight (),
                                                        self.getHccBottom (),
                                                        self.getDeviceID ()),
        if fNewLine:
            print

class OmniEditDeviceForms:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.forms       = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e =  "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceForms::isValid"):
#           print self.__class__.__name__ + "isValid"

        elmForms = self.rootElement

        if elmForms.nodeName != "deviceForms":
            return "Missing <deviceForms>, found " + elmForms.nodeName

        if countChildren (elmForms) < 1:
            return "At least one <deviceForm> element is required"

        elmForm  = firstNode (elmForms.firstChild)
        elmForms = nextNode (elmForms)

        while elmForm != None:
            if elmForm.nodeName != "deviceForm":
                return "Missing <deviceForm>, found " + elmForm.nodeName

            elm = firstNode (elmForm.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            omniName = None
            if elm.nodeName == "omniName":
                omniName = getValue (elm)
                elm      = nextNode (elm)

            if elm.nodeName != "formCapabilities":
                return "Missing <formCapabilities>, found " + elm.nodeName

            capabilities = getValue (elm)
            elm          = nextNode (elm)

            if not isValidCapabilities (capabilities):
                return "Invalid <formCapabilities>"

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getValue (elm) # getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "hardCopyCap":
                return "Missing <hardCopyCap>, found " + elm.nodeName

            if countChildren (elm) != 4:
                return "Found %d elements and required 4 in <hardCopyCap>." % (countChildren (elm), )

            elmHardCopyCap = firstNode (elm.firstChild)
            elm            = nextNode (elm)

            if elmHardCopyCap.nodeName != "hardCopyCapLeft":
                return "Missing <hardCopyCapLeft>, found " + elmHardCopyCap.nodeName

            hccLeft        = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapTop":
                return "Missing <hardCopyCapTop>, found " + elmHardCopyCap.nodeName

            hccTop         = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapRight":
                return "Missing <hardCopyCapRight>, found " + elmHardCopyCap.nodeName

            hccRight       = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapBottom":
                return "Missing <hardCopyCapBottom>, found " + elmHardCopyCap.nodeName

            hccBottom      = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap != None:
                return "Expecting no more tags in <hardCopyCap>"

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmForm = nextNode (elmForm)

            if elm != None:
                return "Expecting no more tags in <deviceForm>"

            self.forms.append (OmniEditDeviceForm ((name,
                                                    omniName,
                                                    capabilities,
                                                    command,
                                                    (hccLeft,
                                                     hccTop,
                                                     hccRight,
                                                     hccBottom),
                                                    deviceID)))

        if elmForms != None:
            return "Expecting no more tags in <deviceForms>"

        return None

    def getFileName (self):
        return self.filename

    def getForms (self):
        return self.forms

    def printSelf (self):
        print "[",
        for form in self.getForms ():
            form.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceForms xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for form in self.getForms ():
            name         = form.getName ()
            omniName     = form.getOmniName ()
            capabilities = form.getCapabilities ()
            command      = form.getCommand ()
            hccLeft      = form.getHccLeft ()
            hccTop       = form.getHccTop ()
            hccRight     = form.getHccRight ()
            hccBottom    = form.getHccBottom ()
            deviceID     = form.getDeviceID ()

            xmlData += """   <deviceForm>
      <name>""" + name + """</name>
"""

            if omniName != None:
                xmlData += "      <omniName>" + omniName + """</omniName>
"""

            xmlData += "      <formCapabilities>" + capabilities + """</formCapabilities>
      <command>""" + convertToXMLString (command) + """</command>
      <hardCopyCap>
         <hardCopyCapLeft>""" + str (hccLeft) + """</hardCopyCapLeft>
         <hardCopyCapTop>""" + str (hccTop) + """</hardCopyCapTop>
         <hardCopyCapRight>""" + str (hccRight) + """</hardCopyCapRight>
         <hardCopyCapBottom>""" + str (hccBottom) + """</hardCopyCapBottom>
      </hardCopyCap>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceForm>
"""

        xmlData += """</deviceForms>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceForms::save"):
            print "OmniEditDeviceForms::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceFormsDialog (self)
        else:
            return OmniEditDeviceFormDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceFormsWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceFormsWindow')

        self.xml    = xml
        self.window = window

        window.add (dialog.getWindow ())
        window.connect ('delete_event', self.on_window_delete)
        window.connect ('destroy', self.on_window_destroy)
        window.show ()

    def getWindow (self):
        return self.window

    def on_window_delete (self, *args):
        """Callback for the window being deleted.

        If you return FALSE in the "delete_event" signal handler,
        GTK will emit the "destroy" signal. Returning TRUE means
        you don't want the window to be destroyed.
        """
        if shouldOutputDebug ("OmniEditDeviceFormsWindow::on_window_delete"):
            print "OmniEditDeviceFormsWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceFormsWindow::on_window_destroy"):
            print "OmniEditDeviceFormsWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceFormsDialog:
    def __init__ (self, form):
        self.form = form

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceFormsFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceFormDialog:
    def __init__ (self, forms, child):
        form = OmniEditDeviceForm (child)

        self.forms     = forms
        self.form      = form
        self.child     = child
        self.fChanged  = False
        self.fModified = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceFormFrame')

        self.xml                = xml
        self.window             = window
        self.EntryName          = xml.get_widget ('EntryFormName')
        self.EntryOmniName      = xml.get_widget ('EntryFormOmniName')
        self.EntryCapabilities  = xml.get_widget ('EntryFormCapabilities')
        self.EntryCommand       = xml.get_widget ('EntryFormCommand')
        self.EntryDeviceID      = xml.get_widget ('EntryFormDeviceID')
        self.EntryHCCLeft       = xml.get_widget ('EntryHCCLeft')
        self.EntryHCCTop        = xml.get_widget ('EntryHCCTop')
        self.EntryHCCRight      = xml.get_widget ('EntryHCCRight')
        self.EntryHCCBottom     = xml.get_widget ('EntryHCCBottom')
        self.CheckButtonDefault = xml.get_widget ('CheckButtonFormDefault')

        dic = { "on_ButtonCommandAdd_clicked":                             self.on_ButtonCommandAdd_clicked,
                "on_ButtonCommandDelete_clicked":                          self.on_ButtonCommandDelete_clicked,
                "on_ButtonCommandModify_clicked":                          self.on_ButtonCommandModify_clicked,
                "on_ButtonConnectionAdd_clicked":                          self.on_ButtonConnectionAdd_clicked,
                "on_ButtonConnectionDelete_clicked":                       self.on_ButtonConnectionDelete_clicked,
                "on_ButtonConnectionModify_clicked":                       self.on_ButtonConnectionModify_clicked,
                "on_ButtonCopyModify_clicked":                             self.on_ButtonCopyModify_clicked,
                "on_ButtonCopyDelete_clicked":                             self.on_ButtonCopyDelete_clicked,
                "on_ButtonDataAdd_clicked":                                self.on_ButtonDataAdd_clicked,
                "on_ButtonDataDelete_clicked":                             self.on_ButtonDataDelete_clicked,
                "on_ButtonDataModify_clicked":                             self.on_ButtonDataModify_clicked,
                "on_ButtonDeviceInformationCancel_clicked":                self.on_ButtonDeviceInformationCancel_clicked,
                "on_ButtonDeviceInformationCapabilitiesEdit_clicked":      self.on_ButtonDeviceInformationCapabilitiesEdit_clicked,
                "on_ButtonDeviceInformationDeviceOptionsEdit_clicked":     self.on_ButtonDeviceInformationDeviceOptionsEdit_clicked,
                "on_ButtonDeviceInformationModify_clicked":                self.on_ButtonDeviceInformationModify_clicked,
                "on_ButtonDeviceInformationRasterCapabilitesEdit_clicked": self.on_ButtonDeviceInformationRasterCapabilitesEdit_clicked,
                "on_ButtonFormAdd_clicked":                                self.on_ButtonFormAdd_clicked,
                "on_ButtonFormDelete_clicked":                             self.on_ButtonFormDelete_clicked,
                "on_ButtonFormModify_clicked":                             self.on_ButtonFormModify_clicked,
                "on_ButtonGammaTableAdd_clicked":                          self.on_ButtonGammaTableAdd_clicked,
                "on_ButtonGammaTableDelete_clicked":                       self.on_ButtonGammaTableDelete_clicked,
                "on_ButtonGammaTableModify_clicked":                       self.on_ButtonGammaTableModify_clicked,
                "on_ButtonMediaAdd_clicked":                               self.on_ButtonMediaAdd_clicked,
                "on_ButtonMediaDelete_clicked":                            self.on_ButtonMediaDelete_clicked,
                "on_ButtonMediaModify_clicked":                            self.on_ButtonMediaModify_clicked,
                "on_ButtonNumberUpAdd_clicked":                            self.on_ButtonNumberUpAdd_clicked,
                "on_ButtonNumberUpDelete_clicked":                         self.on_ButtonNumberUpDelete_clicked,
                "on_ButtonNumberUpModify_clicked":                         self.on_ButtonNumberUpModify_clicked,
                "on_ButtonOptionModify_clicked":                           self.on_ButtonOptionModify_clicked,
                "on_ButtonOptionAdd_clicked":                              self.on_ButtonOptionAdd_clicked,
                "on_ButtonOptionDelete_clicked":                           self.on_ButtonOptionDelete_clicked,
                "on_ButtonOptionOk_clicked":                               self.on_ButtonOptionOk_clicked,
                "on_ButtonOptionCancel_clicked":                           self.on_ButtonOptionCancel_clicked,
                "on_ButtonOrientationAdd_clicked":                         self.on_ButtonOrientationAdd_clicked,
                "on_ButtonOrientationDelete_clicked":                      self.on_ButtonOrientationDelete_clicked,
                "on_ButtonOrientationModify_clicked":                      self.on_ButtonOrientationModify_clicked,
                "on_ButtonOutputBinAdd_clicked":                           self.on_ButtonOutputBinAdd_clicked,
                "on_ButtonOutputBinDelete_clicked":                        self.on_ButtonOutputBinDelete_clicked,
                "on_ButtonOutputBinModify_clicked":                        self.on_ButtonOutputBinModify_clicked,
                "on_ButtonPrintModeAdd_clicked":                           self.on_ButtonPrintModeAdd_clicked,
                "on_ButtonPrintModeDelete_clicked":                        self.on_ButtonPrintModeDelete_clicked,
                "on_ButtonPrintModeModify_clicked":                        self.on_ButtonPrintModeModify_clicked,
                "on_ButtonResolutionAdd_clicked":                          self.on_ButtonResolutionAdd_clicked,
                "on_ButtonResolutionDelete_clicked":                       self.on_ButtonResolutionDelete_clicked,
                "on_ButtonResolutionModify_clicked":                       self.on_ButtonResolutionModify_clicked,
                "on_ButtonScalingAdd_clicked":                             self.on_ButtonScalingAdd_clicked,
                "on_ButtonScalingDelete_clicked":                          self.on_ButtonScalingDelete_clicked,
                "on_ButtonScalingModify_clicked":                          self.on_ButtonScalingModify_clicked,
                "on_ButtonSheetCollateAdd_clicked":                        self.on_ButtonSheetCollateAdd_clicked,
                "on_ButtonSheetCollateDelete_clicked":                     self.on_ButtonSheetCollateDelete_clicked,
                "on_ButtonSheetCollateModify_clicked":                     self.on_ButtonSheetCollateModify_clicked,
                "on_ButtonSideAdd_clicked":                                self.on_ButtonSideAdd_clicked,
                "on_ButtonSideDelete_clicked":                             self.on_ButtonSideDelete_clicked,
                "on_ButtonSideModify_clicked":                             self.on_ButtonSideModify_clicked,
                "on_ButtonStitchingAdd_clicked":                           self.on_ButtonStitchingAdd_clicked,
                "on_ButtonStitchingDelete_clicked":                        self.on_ButtonStitchingDelete_clicked,
                "on_ButtonStitchingModify_clicked":                        self.on_ButtonStitchingModify_clicked,
                "on_ButtonTrayAdd_clicked":                                self.on_ButtonTrayAdd_clicked,
                "on_ButtonTrayDelete_clicked":                             self.on_ButtonTrayDelete_clicked,
                "on_ButtonTrayModify_clicked":                             self.on_ButtonTrayModify_clicked,
                "on_ButtonTrimmingAdd_clicked":                            self.on_ButtonTrimmingAdd_clicked,
                "on_ButtonTrimmingDelete_clicked":                         self.on_ButtonTrimmingDelete_clicked,
                "on_ButtonTrimmingModify_clicked":                         self.on_ButtonTrimmingModify_clicked,
                "on_CheckButtonFormDefault_toggled":                       self.on_CheckButtonFormDefault_toggled,
                "on_CheckButtonMediaDefault_toggled":                      self.on_CheckButtonMediaDefault_toggled,
                "on_CheckButtonNumberUpDefault_toggled":                   self.on_CheckButtonNumberUpDefault_toggled,
                "on_CheckButtonOrientationDefault_toggled":                self.on_CheckButtonOrientationDefault_toggled,
                "on_CheckButtonOutputBinDefault_toggled":                  self.on_CheckButtonOutputBinDefault_toggled,
                "on_CheckButtonPrintModeDefault_toggled":                  self.on_CheckButtonPrintModeDefault_toggled,
                "on_CheckButtonResolutionDefault_toggled":                 self.on_CheckButtonResolutionDefault_toggled,
                "on_CheckButtonScalingDefault_toggled":                    self.on_CheckButtonScalingDefault_toggled,
                "on_CheckButtonSheetCollateDefault_toggled":               self.on_CheckButtonSheetCollateDefault_toggled,
                "on_CheckButtonSideDefault_toggled":                       self.on_CheckButtonSideDefault_toggled,
                "on_CheckButtonStitchingDefault_toggled":                  self.on_CheckButtonStitchingDefault_toggled,
                "on_CheckButtonTrayDefault_toggled":                       self.on_CheckButtonTrayDefault_toggled,
                "on_CheckButtonTrimmingDefault_toggled":                   self.on_CheckButtonTrimmingDefault_toggled,
                "on_EntryCommandName_changed":                             self.on_EntryCommandName_changed,
                "on_EntryCommandCommand_changed":                          self.on_EntryCommandCommand_changed,
                "on_EntryConnectionName_changed":                          self.on_EntryConnectionName_changed,
                "on_EntryConnectionForm_changed":                          self.on_EntryConnectionForm_changed,
                "on_EntryConnectionOmniForm_changed":                      self.on_EntryConnectionOmniForm_changed,
                "on_EntryConnectionTray_changed":                          self.on_EntryConnectionTray_changed,
                "on_EntryConnectionOmniTray_changed":                      self.on_EntryConnectionOmniTray_changed,
                "on_EntryConnectionMedia_changed":                         self.on_EntryConnectionMedia_changed,
                "on_EntryCopiesDefault_changed":                           self.on_EntryCopiesDefault_changed,
                "on_EntryCopyCommand_changed":                             self.on_EntryCopyCommand_changed,
                "on_EntryCopyDeviceID_changed":                            self.on_EntryCopyDeviceID_changed,
                "on_EntryCopyMaximum_changed":                             self.on_EntryCopyMaximum_changed,
                "on_EntryCopyMinimum_changed":                             self.on_EntryCopyMinimum_changed,
                "on_EntryCopySimulationRequired_changed":                  self.on_EntryCopySimulationRequired_changed,
                "on_EntryDataName_changed":                                self.on_EntryDataName_changed,
                "on_EntryDataType_changed":                                self.on_EntryDataType_changed,
                "on_EntryDataData_changed":                                self.on_EntryDataData_changed,
                "on_EntryDeviceInformationDeviceName_changed":             self.on_EntryDeviceInformationDeviceName_changed,
                "on_EntryDeviceInformationDriverName_changed":             self.on_EntryDeviceInformationDriverName_changed,
                "on_EntryDeviceInformationPDLLevel_changed":               self.on_EntryDeviceInformationPDLLevel_changed,
                "on_EntryDeviceInformationPDLMajor_changed":               self.on_EntryDeviceInformationPDLMajor_changed,
                "on_EntryDeviceInformationPDLMinor_changed":               self.on_EntryDeviceInformationPDLMinor_changed,
                "on_EntryDeviceInformationPDLSubLevel_changed":            self.on_EntryDeviceInformationPDLSubLevel_changed,
                "on_EntryFormCapabilities_changed":                        self.on_EntryFormCapabilities_changed,
                "on_EntryFormCommand_changed":                             self.on_EntryFormCommand_changed,
                "on_EntryFormDeviceID_changed":                            self.on_EntryFormDeviceID_changed,
                "on_EntryFormHCCBottom_changed":                           self.on_EntryFormHCCBottom_changed,
                "on_EntryFormHCCLeft_changed":                             self.on_EntryFormHCCLeft_changed,
                "on_EntryFormHCCRight_changed":                            self.on_EntryFormHCCRight_changed,
                "on_EntryFormHCCTop_changed":                              self.on_EntryFormHCCTop_changed,
                "on_EntryFormName_changed":                                self.on_EntryFormName_changed,
                "on_EntryFormOmniName_changed":                            self.on_EntryFormOmniName_changed,
                "on_EntryGammaTableResolution_changed":                    self.on_EntryGammaTableResolution_changed,
                "on_EntryGammaTableOmniResolution_changed":                self.on_EntryGammaTableOmniResolution_changed,
                "on_EntryGammaTableMedia_changed":                         self.on_EntryGammaTableMedia_changed,
                "on_EntryGammaTablePrintMode_changed":                     self.on_EntryGammaTablePrintMode_changed,
                "on_EntryGammaTableDitherCatagory_changed":                self.on_EntryGammaTableDitherCatagory_changed,
                "on_EntryGammaTableCGamma_changed":                        self.on_EntryGammaTableCGamma_changed,
                "on_EntryGammaTableMGamma_changed":                        self.on_EntryGammaTableMGamma_changed,
                "on_EntryGammaTableYGamma_changed":                        self.on_EntryGammaTableYGamma_changed,
                "on_EntryGammaTableKGamma_changed":                        self.on_EntryGammaTableKGamma_changed,
                "on_EntryGammaTableCBias_changed":                         self.on_EntryGammaTableCBias_changed,
                "on_EntryGammaTableMBias_changed":                         self.on_EntryGammaTableMBias_changed,
                "on_EntryGammaTableYBias_changed":                         self.on_EntryGammaTableYBias_changed,
                "on_EntryGammaTableKBias_changed":                         self.on_EntryGammaTableKBias_changed,
                "on_EntryMediaAbsorption_changed":                         self.on_EntryMediaAbsorption_changed,
                "on_EntryMediaColorAdjustRequired_changed":                self.on_EntryMediaColorAdjustRequired_changed,
                "on_EntryMediaCommand_changed":                            self.on_EntryMediaCommand_changed,
                "on_EntryMediaDeviceID_changed":                           self.on_EntryMediaDeviceID_changed,
                "on_EntryMediaName_changed":                               self.on_EntryMediaName_changed,
                "on_EntryNumberUpCommand_changed":                         self.on_EntryNumberUpCommand_changed,
                "on_EntryNumberUpDeviceID_changed":                        self.on_EntryNumberUpDeviceID_changed,
                "on_EntryNumberUpDirection_changed":           self.on_EntryNumberUpDirection_changed,
                "on_EntryNumberUpSimulationRequired_changed":              self.on_EntryNumberUpSimulationRequired_changed,
                "on_EntryNumberUpX_changed":                               self.on_EntryNumberUpX_changed,
                "on_EntryNumberUpY_changed":                               self.on_EntryNumberUpY_changed,
                "on_EntryOptionName_changed":                              self.on_EntryOptionName_changed,
                "on_EntryOrientationDeviceID_changed":                     self.on_EntryOrientationDeviceID_changed,
                "on_EntryOrientationName_changed":                         self.on_EntryOrientationName_changed,
                "on_EntryOrientationOmniName_changed":                     self.on_EntryOrientationOmniName_changed,
                "on_EntryOrientationSimulationRequired_changed":           self.on_EntryOrientationSimulationRequired_changed,
                "on_EntryOutputBinCommand_changed":                        self.on_EntryOutputBinCommand_changed,
                "on_EntryOutputBinDeviceID_changed":                       self.on_EntryOutputBinDeviceID_changed,
                "on_EntryOutputBinName_changed":                           self.on_EntryOutputBinName_changed,
                "on_EntryPrintModeDeviceID_changed":                       self.on_EntryPrintModeDeviceID_changed,
                "on_EntryPrintModeLogicalCount_changed":                   self.on_EntryPrintModeLogicalCount_changed,
                "on_EntryPrintModeName_changed":                           self.on_EntryPrintModeName_changed,
                "on_EntryPrintModePhysicalCount_changed":                  self.on_EntryPrintModePhysicalCount_changed,
                "on_EntryPrintModePlanes_changed":                         self.on_EntryPrintModePlanes_changed,
                "on_EntryResolutionCapability_changed":                    self.on_EntryResolutionCapability_changed,
                "on_EntryResolutionCommand_changed":                       self.on_EntryResolutionCommand_changed,
                "on_EntryResolutionDestinationBitsPerPel_changed":         self.on_EntryResolutionDestinationBitsPerPel_changed,
                "on_EntryResolutionScanlineMultiple_changed":              self.on_EntryResolutionScanlineMultiple_changed,
                "on_EntryResolutionDeviceID_changed":                      self.on_EntryResolutionDeviceID_changed,
                "on_EntryResolutionName_changed":                          self.on_EntryResolutionName_changed,
                "on_EntryResolutionOmniName_changed":                      self.on_EntryResolutionOmniName_changed,
                "on_EntryResolutionXInternalRes_changed":                  self.on_EntryResolutionXInternalRes_changed,
                "on_EntryResolutionXRes_changed":                          self.on_EntryResolutionXRes_changed,
                "on_EntryResolutionYInternalRes_changed":                  self.on_EntryResolutionYInternalRes_changed,
                "on_EntryResolutionYRes_changed":                          self.on_EntryResolutionYRes_changed,
                "on_EntryScalingAllowedType_changed":                      self.on_EntryScalingAllowedType_changed,
                "on_EntryScalingCommand_changed":                          self.on_EntryScalingCommand_changed,
                "on_EntryScalingDefault_changed":                          self.on_EntryScalingDefault_changed,
                "on_EntryScalingDeviceID_changed":                         self.on_EntryScalingDeviceID_changed,
                "on_EntryScalingMaximum_changed":                          self.on_EntryScalingMaximum_changed,
                "on_EntryScalingMinimum_changed":                          self.on_EntryScalingMinimum_changed,
                "on_EntryScalingDefaultPercentage_changed":                self.on_EntryScalingDefaultPercentage_changed,
                "on_EntrySheetCollateCommand_changed":                     self.on_EntrySheetCollateCommand_changed,
                "on_EntrySheetCollateDeviceID_changed":                    self.on_EntrySheetCollateDeviceID_changed,
                "on_EntrySheetCollateName_changed":                        self.on_EntrySheetCollateName_changed,
                "on_EntrySideCommand_changed":                             self.on_EntrySideCommand_changed,
                "on_EntrySideDeviceID_changed":                            self.on_EntrySideDeviceID_changed,
                "on_EntrySideName_changed":                                self.on_EntrySideName_changed,
                "on_EntrySideSimulationRequired_changed":                  self.on_EntrySideSimulationRequired_changed,
                "on_EntryStitchingAngle_changed":                          self.on_EntryStitchingAngle_changed,
                "on_EntryStitchingCommand_changed":                        self.on_EntryStitchingCommand_changed,
                "on_EntryStitchingCount_changed":                          self.on_EntryStitchingCount_changed,
                "on_EntryStitchingDeviceID_changed":                       self.on_EntryStitchingDeviceID_changed,
                "on_EntryStitchingPosition_changed":                       self.on_EntryStitchingPosition_changed,
                "on_EntryStitchingReferenceEdge_changed":                  self.on_EntryStitchingReferenceEdge_changed,
                "on_EntryStitchingType_changed":                           self.on_EntryStitchingType_changed,
                "on_EntryStringCountryCode_changed":                       self.on_EntryStringCountryCode_changed,
                "on_EntryStringKeyName_changed":                           self.on_EntryStringKeyName_changed,
                "on_EntryStringTranslation_changed":                       self.on_EntryStringTranslation_changed,
                "on_EntryTrayCommand_changed":                             self.on_EntryTrayCommand_changed,
                "on_EntryTrayDeviceID_changed":                            self.on_EntryTrayDeviceID_changed,
                "on_EntryTrayName_changed":                                self.on_EntryTrayName_changed,
                "on_EntryTrayOmniName_changed":                            self.on_EntryTrayOmniName_changed,
                "on_EntryTrayTrayType_changed":                            self.on_EntryTrayTrayType_changed,
                "on_EntryTrimmingCommand_changed":                         self.on_EntryTrimmingCommand_changed,
                "on_EntryTrimmingDeviceID_changed":                        self.on_EntryTrimmingDeviceID_changed,
                "on_EntryTrimmingName_changed":                            self.on_EntryTrimmingName_changed,
                "on_MenuAbout_activate":                                   self.on_MenuAbout_activate,
                "on_MenuCommands_activate":                                self.on_MenuCommands_activate,
                "on_MenuConnections_activate":                             self.on_MenuConnections_activate,
                "on_MenuCopies_activate":                                  self.on_MenuCopies_activate,
                "on_MenuDatas_activate":                                   self.on_MenuDatas_activate,
                "on_MenuForms_activate":                                   self.on_MenuForms_activate,
                "on_MenuGammaTables_activate":                             self.on_MenuGammaTables_activate,
                "on_MenuMedias_activate":                                  self.on_MenuMedias_activate,
                "on_MenuNumberUps_activate":                               self.on_MenuNumberUps_activate,
                "on_MenuOrientations_activate":                            self.on_MenuOrientations_activate,
                "on_MenuOutputBins_activate":                              self.on_MenuOutputBins_activate,
                "on_MenuPrintModes_activate":                              self.on_MenuPrintModes_activate,
                "on_MenuResolutions_activate":                             self.on_MenuResolutions_activate,
                "on_MenuScalings_activate":                                self.on_MenuScalings_activate,
                "on_MenuSheetCollates_activate":                           self.on_MenuSheetCollates_activate,
                "on_MenuSides_activate":                                   self.on_MenuSides_activate,
                "on_MenuStitchings_activate":                              self.on_MenuStitchings_activate,
                "on_MenuStrings_activate":                                 self.on_MenuStrings_activate,
                "on_MenuTrays_activate":                                   self.on_MenuTrays_activate,
                "on_MenuTrimmings_activate":                               self.on_MenuTrimmings_activate,
              }
#       if shouldOutputDebug ("OmniEditDeviceFormsDialog::__init__"):
#           print "OmniEditDeviceFormsDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryName.set_text (str (form.getName ()))
        name = form.getOmniName ()
        if name == None:
            name = ""
        self.EntryOmniName.set_text (str (name))
        self.EntryCapabilities.set_text (str (form.getCapabilities ()))
        self.EntryCommand.set_text (str (form.getCommand ()))
        self.EntryHCCLeft.set_text (str (form.getHccLeft ()))
        self.EntryHCCTop.set_text (str (form.getHccTop ()))
        self.EntryHCCRight.set_text (str (form.getHccRight ()))
        self.EntryHCCBottom.set_text (str (form.getHccBottom ()))
        name = form.getDeviceID ()
        if name == None:
            name = ""
        self.EntryDeviceID.set_text (str (name))
        fDefault    = False
        device      = forms.getDevice ()
        deviceInfo  = device.getDeviceInformation ()
        defaultForm = deviceInfo.getDefaultJobProperty ("Form")
        if defaultForm != None and defaultForm == form.getName ():
            fDefault = True
        self.CheckButtonDefault.set_active (fDefault)

        self.fChanged = False

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceFormsDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceFormsDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceFormsDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceFormsDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceFormsDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceFormsDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceFormsDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceFormsDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceFormsDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceFormsDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceFormsDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceFormsDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceFormsDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceFormsDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceFormsDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceFormsDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceFormsDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceFormsDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceFormsDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceFormsDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceFormsDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceFormsDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceFormsDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceFormsDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceFormsDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceFormsDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceFormsDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceFormsDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceFormsDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceFormsDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceFormsDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceFormsDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceFormsDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceFormsDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceFormsDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceFormsDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceFormsDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceFormsDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceFormsDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceFormsDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceFormsDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceFormsDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceFormsDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceFormsDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceFormsDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceFormsDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceFormsDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceFormsDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceFormsDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceFormsDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceFormsDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceFormsDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceFormsDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceFormsDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceFormsDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceFormsDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceFormsDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceFormsDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceFormsDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceFormsDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceFormsDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceFormsDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceFormsDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceFormsDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceFormsDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceFormsDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceFormsDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceFormsDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceFormsDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceFormsDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceFormsDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceFormsDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceFormsDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceFormsDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceFormsDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceFormsDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceFormsDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceFormsDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceFormsDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceFormsDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceFormsDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceFormsDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceFormsDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceFormsDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceFormsDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceFormsDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceFormsDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceFormsDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceFormsDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceFormsDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceFormsDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceFormsDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceFormsDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceFormsDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceFormsDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceFormsDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceFormsDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceFormsDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonFormModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormModify_clicked"):
            print "OmniEditDeviceFormsDialog::on_ButtonFormModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.form.setName (self.EntryName.get_text ()):
                pszError = "Invalid name"
            if not self.form.setOmniName (self.EntryOmniName.get_text ()):
                pszError = "Invalid omni name"
            if not self.form.setCapabilities (self.EntryCapabilities.get_text ()):
                pszError = "Invalid capabilities"
            if not self.form.setCommand (self.EntryCommand.get_text ()):
                pszError = "Invalid command"
            if not self.form.setDeviceID (self.EntryDeviceID.get_text ()):
                pszError = "Invalid device ID"
            if not self.form.setHccLeft (self.EntryHCCLeft.get_text ()):
                pszError = "Invalid left margin"
            if not self.form.setHccTop (self.EntryHCCTop.get_text ()):
                pszError = "Invalid top margin"
            if not self.form.setHccRight (self.EntryHCCRight.get_text ()):
                pszError = "Invalid right margin"
            if not self.form.setHccBottom (self.EntryHCCBottom.get_text ()):
                pszError = "Invalid bottom margin"

            if pszError == None:
                matchingForm = self.findForm (self.form)

                if     matchingForm != None \
                   and matchingForm != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             self.form.getName () + " already exists!")
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceForm (self.form)
                    self.fModified = True
            else:
                ask = gtk.MessageDialog (None,
                                         0,
                                         gtk.MESSAGE_QUESTION,
                                         gtk.BUTTONS_NONE,
                                         pszError)
                ask.add_button ("_Ok", gtk.RESPONSE_YES)
                response = ask.run ()
                ask.destroy ()

                return

        if self.CheckButtonDefault.get_active ():
            name = self.form.getName ()
            if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormModify_clicked"):
                print "OmniEditDeviceFormsDialog::on_ButtonFormModify_clicked: Setting %s to default" % (name)
            device     = self.forms.getDevice ()
            dialog     = device.getDeviceDialog ()
            deviceInfo = device.getDeviceInformation ()
            deviceInfo.setDefaultJobProperty ("Form", name)
            dialog.setModified (True)

            omniForm = deviceInfo.getDefaultJobProperty ("omniForm")
            if omniForm != None:
                # Currently, we cannot map from the new form name to the old one
                # so just delete the old name
                deviceInfo.deleteDefaultJobProperty ("omniForm")

    def on_ButtonFormAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormAdd_clicked"):
            print "OmniEditDeviceFormsDialog::on_ButtonFormAdd_clicked:", widget

    def on_ButtonFormDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked"):
            print "OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked:", widget

        ask = gtk.MessageDialog (None,
                                 0,
                                 gtk.MESSAGE_QUESTION,
                                 gtk.BUTTONS_NONE,
                                 "Are you sure?")
        ask.add_button ("_Yes", gtk.RESPONSE_YES)
        ask.add_button ("_No", gtk.RESPONSE_NO)
        response = ask.run ()
        ask.destroy ()

        if response == gtk.RESPONSE_YES:
            (rc, iFormsLeft) = self.deleteForm (self.child)
            if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked"):
                print "OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked: deleteForm rc = %s, iFormsLeft = %d" % (rc, iFormsLeft)

            device    = self.forms.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iFormsLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked"):
                    print "OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked: Last form deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceForms":
                        treestore.remove (row.iter)
                        device.setDeviceForms (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_EntryFormName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormName_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormOmniName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormOmniName_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormOmniName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormCapabilities_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormCapabilities_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormCapabilities_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormCommand_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormCommand_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormCommand_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormDeviceID_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormDeviceID_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormDeviceID_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormHCCLeft_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormHCCLeft_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormHCCLeft_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormHCCTop_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormHCCTop_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormHCCTop_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormHCCRight_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormHCCRight_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormHCCRight_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryFormHCCBottom_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_EntryFormHCCBottom_changed"):
            print "OmniEditDeviceFormsDialog::on_EntryFormHCCBottom_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_CheckButtonFormDefault_toggled (self, widget):
        if shouldOutputDebug ("OmniEditDeviceFormsDialog::on_CheckButtonFormDefault_toggled"):
            print "OmniEditDeviceFormsDialog::on_CheckButtonFormDefault_toggled:", widget

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.forms.save ()

        self.fModified = False

    def findForm (self, matchForm):
        for form in self.forms.getForms ():
            if matchForm.getName () == form.getName ():
                return form

        return None

    def deleteForm (self, matchForm):
        foundForm = None
        foundRow  = None
        forms     = self.forms.getForms ()
        for form in forms:
#           if shouldOutputDebug ("OmniEditDeviceFormsDialog::deleteForm"):
#               print "OmniEditDeviceFormsDialog::deleteForm: matching1 %s vs %s" % (matchForm.getName (), form.getName ())
            if matchForm.getName () == form.getName ():
                foundForm = form

        device    = self.forms.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceFormsDialog::deleteForm"):
#                       print "OmniEditDeviceFormsDialog::deleteForm: matching2 %s vs %s" % (childRow[1], matchForm)
                    if childRow[1] == matchForm:
                        foundRow = childRow

        if     foundForm != None \
           and foundRow  != None:
            if shouldOutputDebug ("OmniEditDeviceFormsDialog::deleteForm"):
                print "OmniEditDeviceFormsDialog::deleteForm: Removing %s from forms" % (foundForm.getName ())
            forms.remove (foundForm)

            if shouldOutputDebug ("OmniEditDeviceFormsDialog::deleteForm"):
                print "OmniEditDeviceFormsDialog::deleteForm: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.forms.setModified (True)

            return (True, len (forms))
        else:
            if foundForm == None:
                print "OmniEditDeviceFormsDialog::deleteForm: Error: Did not find %s in forms!" % (matchForm.getName ())
            if foundRow == None:
                print "OmniEditDeviceFormsDialog::deleteForm: Error: Did not find %s in listbox!" % (str (matchForm))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Stylus Color 660 Forms.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        forms = OmniEditDeviceForms (fname, doc.documentElement, None)

        print "OmniEditDeviceForms.py::__main__: forms = ",
        lastForm = forms.getForms ()[-1]
        for form in forms.getForms ():
            form.printSelf (False),
            if form != lastForm:
                print ",",
        print

        dialog = OmniEditDeviceFormDialog (forms, forms.getForms ()[0])
        window = OmniEditDeviceFormsWindow (dialog)

        gtk.main ()

        print forms.toXML ()

    except Exception, e:
        print "OmniEditDeviceForms.py::__main__: Error: Caught", e
