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
#ifndef _com_ibm_ltc_omni_guitool_DeviceToolConstants_hpp
#define _com_ibm_ltc_omni_guitool_DeviceToolConstants_hpp

/*
   ************* IMPORTANT NOTE *****************

   When editing this file, do not change the names of the #defined variables.
   Change only values, DO NOT CHANGE VARIABLE NAMES.
   Also, When changing any value, use the C++ escape sequences for
   specifying \ (backspash) and " (double quotes):
   ie Enter \" for " and \\ for \

   DO NOT CHANGE namespace AND ifndef VALUES
*/

namespace OmniDeviceCreationTool {

#define EXT ".xml" // file extension

// TAB1 = 3 blank spaces
#define TAB1 "   "
// TAB2 = 6 blank spaces
#define TAB2 "      "
// TAB3 = 9 blank spaces
#define TAB3 "         "

// TAB1 TAB2 and TAB3 are levels of indentation -blanks are inserted

#define XMLHEADER "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"

#define COPYRIGHT "<!--\n" \
                  "     IBM Omni driver\n" \
                  "     Copyright (c) International Business Machines Corp., 2000\n" \
                  "\n" \
                  "     This library is free software; you can redistribute it and/or modify\n" \
                  "     it under the terms of the GNU Lesser General Public License as published\n" \
                  "     by the Free Software Foundation; either version 2.1 of the License, or\n" \
                  "     (at your option) any later version.\n" \
                  "\n" \
                  "     This library is distributed in the hope that it will be useful,\n" \
                  "     but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
                  "     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See\n" \
                  "     the GNU Lesser General Public License for more details.\n" \
                  "\n" \
                  "     You should have received a copy of the GNU Lesser General Public License\n" \
                  "     along with this library; if not, write to the Free Software\n" \
                  "     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA\n" \
                  "-->"

// Defining DOCTYPE for different XML files

#define DEVICE_DOCTYPE       "<!DOCTYPE Device SYSTEM \"../OmniDevice.dtd\" >"
#define COMMANDS_DOCTYPE     "<!DOCTYPE deviceCommands SYSTEM \"../OmniDevice.dtd\">"
#define RESOLUTIONS_DOCTYPE  "<!DOCTYPE deviceResolutions SYSTEM \"../OmniDevice.dtd\">"
#define PRINTMODES_DOCTYPE   "<!DOCTYPE devicePrintModes SYSTEM \"../OmniDevice.dtd\">"
#define TRAYS_DOCTYPE        "<!DOCTYPE deviceTrays SYSTEM \"../OmniDevice.dtd\">"
#define FORMS_DOCTYPE        "<!DOCTYPE deviceFroms SYSTEM \"../OmniDevice.dtd\">"
#define MEDIAS_DOCTYPE       "<!DOCTYPE deviceMedias SYSTEM \"../OmniDevice.dtd\">"
#define CONNECTIONS_DOCTYPE  "<!DOCTYPE deviceConnections SYSTEM \"../OmniDevice.dtd\">"
#define DATAS_DOCTYPE        "<!DOCTYPE deviceDatas SYSTEM \"../OmniDevice.dtd\">"
#define GAMMAS_DOCTYPE       "<!DOCTYPE deviceGammaTables SYSTEM \"../OmniDevice.dtd\">"
#define ORIENTATIONS_DOCTYPE "<!DOCTYPE deviceOrientations SYSTEM \"../OmniDevice.dtd\">"

// Defining File Names for different XML files
// the file names will take the form xxxDevice xxxProperty.xml
// eg - "xxxDeviceName Resolutions.xml"

#define COMMANDS_FILENAME     " Commands.xml"
#define RESOLUTIONS_FILENAME  " Resolutions.xml"
#define PRINTMODES_FILENAME   " PrintModes.xml"
#define TRAYS_FILENAME        " Trays.xml"
#define FORMS_FILENAME        " Forms.xml"
#define MEDIAS_FILENAME       " Medias.xml"
#define CONNECTIONS_FILENAME  " Connections.xml"
#define DATAS_FILENAME        " Data.xml"
#define GAMMAS_FILENAME       " Gammas.xml"
#define ORIENTATIONS_FILENAME " Orientations.xml"

/*************************************************************/
// DO NOT EDIT THE FOLLOWING LINES
// EDITING THE FOLLOWING LINES MAY RESULT
// IN CHANGES IN THE PROGRAM BEHAVIOUR

#define DEVICE            "DeviceHead"
#define GENERALPROPERTIES "GeneralProperties"
#define RESOLUTIONS       "Resolutions"
#define COMMANDS          "Commands"
#define PRINTMODES        "PrintModes"
#define TRAYS             "Trays"
#define FORMS             "Forms"
#define MEDIAS            "Medias"
#define CONNECTIONS       "Connections"
#define GAMMAS            "Gammas"
#define DATA              "Data"
#define DEFAULTJP         "DefaultJobProperties"
#define ORIENTATIONS      "Orientations"

/**************************************************************/

}; // end of namespace

#endif
