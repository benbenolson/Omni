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

import string
from xml.dom import Node
from xml.dom.ext import PrettyPrint

def saveNewFiles ():
    return True

def fileExists (pszFileName):
    import os

    try:
        (st_mode,
         st_ino,
         st_dev,
         st_nlink,
         st_uid,
         st_gid,
         st_size,
         st_atime,
         st_mtime,
         st_ctime) = os.stat (pszFileName)

        return True
    except:
        return False

def convertToBooleanValue (value):
    lowerCase = value.lower ()
    if cmp (lowerCase, "true") == 0:
        return True
    elif cmp (lowerCase, "false") == 0:
        return False
    else:
        raise Exception (value + " is not a boolean value!")

def convertToIntegerValue (value):
    try:
        return int (value)
    except Exception, e:
        raise Exception (value + " is not an integer value!")

def convertToFloatValue (value):
    try:
        return float (value)
    except Exception, e:
        raise Exception (value + " is not an float value!")

def firstNode (elm):
#   print "OmniEditUtils.py::firstNode: (", elm, ")"

    while     elm != None                          \
          and (  elm.nodeType == Node.TEXT_NODE    \
              or elm.nodeType == Node.COMMENT_NODE \
              ):
        elm = elm.nextSibling

#   print "OmniEditUtils.py::firstNode: returning", elm

    return elm

def nextNode (elm):
#   print "OmniEditUtils.py::nextNode: (", elm, ")"

    if elm != None:
        elm = elm.nextSibling

    while     elm != None                          \
          and (  elm.nodeType == Node.TEXT_NODE    \
              or elm.nodeType == Node.COMMENT_NODE \
              ):
        elm = elm.nextSibling

#   print "OmniEditUtils.py::nextNode: returning", elm

    return elm

def validSingleElement (elm):
    try:
        if len (elm.childNodes) != 1:
            return False

        elm = elm.firstChild

        if elm.nodeType != Node.TEXT_NODE:
            return False

        return True

    except Exception, e:
        return False

def getValue (elm):
    try:
        if len (elm.childNodes) == 1:
            elm = elm.firstChild

            return elm.nodeValue

    except Exception, e:
        pass

    return None

def getIntValue (elm):
    value = getValue (elm)
    if value == None:
        return value

    return 0

def convertToXMLString (string):
    list = { '"': '&quot;',
             '&': '&amp;',
             '<': '&lt;',
             '>': '&gt;',
             "'": '&apos;'
           }

    returnedString = ""

    for character in string:
        if character in list:
            returnedString += list[character]
        else:
            returnedString += character

    return returnedString

def convertFromXMLString (string):
    list = [ ('&quot;', '"'),
             ('&amp;',  '&'),
             ('&lt;',   '<'),
             ('&gt;',   '>'),
             ('&apos;', "'")
           ]

    for elm in list :
        (frm, to) = elm

        string = string.replace (frm, to)

    return string

def validHexChar (string):
    if string == "0":
        return 0
    elif string == "1":
        return 1
    elif string == "2":
        return 2
    elif string == "3":
        return 3
    elif string == "4":
        return 4
    elif string == "5":
        return 5
    elif string == "6":
        return 6
    elif string == "7":
        return 7
    elif string == "8":
        return 8
    elif string == "9":
        return 9
    elif string == "a":
        return 10
    elif string == "b":
        return 11
    elif string == "c":
        return 12
    elif string == "d":
        return 13
    elif string == "e":
        return 14
    elif string == "f":
        return 15
    elif string == "A":
        return 10
    elif string == "B":
        return 11
    elif string == "C":
        return 12
    elif string == "D":
        return 13
    elif string == "E":
        return 14
    elif string == "F":
        return 15
    else:
        return None

def parseHexGroup (string):
    if len (string) < 2:
        return (None, False)

    digitH = validHexChar (string[0])
    if digitH == None:
        return (None, False)

    digitL = validHexChar (string[1])
    if digitL == None:
        return (None, False)

    return (digitH * 16 + digitL, True)

def getCommand (elm):
    string = getValue (elm)
    return getCommandString (string)

def getCommandString (string):
    string         = convertFromXMLString (string)
    ptrString      = 0
    inString       = False
    char           = None
    nextChar       = None
    charFound      = False
    nextCharFound  = False
    command        = []
    shouldContinue = True
    pszError       = None

    try:
        while shouldContinue:
#           print "OmniEditUtils.py::getCommandString: string[%d] = %s, charFound = %d, char = %s, nextCharFound = %d, nextChar = %s, inString = %d" % (ptrString, string[ptrString], charFound, char, nextCharFound, nextChar, inString)

            if nextCharFound:
                char          = nextChar
                nextCharFound = False
                charFound     = True

            else:
                if inString:
                    if "\\" == string[ptrString]:
                        pass
                    elif '"' == string[ptrString]:
                        ptrString += 1
                        inString   = False
                        continue
                    else:
                        char       = string[ptrString]
                        ptrString += 1
                        charFound  = True
                else:
                    while    string[ptrString] == "\r" \
                          or string[ptrString] == "\n" \
                          or string[ptrString] == " ":
                        ptrString += 1

                    if string[ptrString] == "_":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "ESC_":
                            char      = "\x1b"
                            charFound = True
                            ptrString += 4

                        elif string[ptrString:ptrString + 4] == "NUL_":
                            char      = "\0"
                            charFound = True
                            ptrString += 4

                        elif string[ptrString:ptrString + 3] == "LF_":
                            char      = "\x0a"
                            charFound = True
                            ptrString += 3

                        elif string[ptrString:ptrString + 3] == "CR_":
                            char      = "\x0d"
                            charFound = True
                            ptrString += 3

                        elif string[ptrString:ptrString + 3] == "FF_":
                            char      = "\x0c"
                            charFound = True
                            ptrString += 3

                        else:
                            pszError = "Error! Unknown define " + string
                            shouldContinue = False

                    elif string[ptrString] == "H":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "EX2S":
                            ptrString += 4

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                (data, success) = parseHexGroup (string[ptrString:])

                                if success:
                                    ptrString += 2

                                    while string[ptrString] == " ":
                                        ptrString += 1

                                    if string[ptrString] == ",":
                                        ptrString += 1

                                        while string[ptrString] == " ":
                                            ptrString += 1

                                        (data2, success) = parseHexGroup (string[ptrString:])

                                        if success:
                                            ptrString += 2

                                            while string[ptrString] == " ":
                                                ptrString += 1

                                            if string[ptrString] == ")":
                                                ptrString += 1

                                                char           = unichr (data)
                                                nextChar       = unichr (data2)
                                                charFound      = True
                                                nextCharFound  = True

                                            else:
                                                pszError = "Error: Expecting ')', found " + string[ptrString]
                                                shouldContinue = False

                                        else:
                                            pszError = "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                            shouldContinue = False

                                    else:
                                        pszError = "Error: Expecting ',', found " + string[ptrString]
                                        shouldContinue = False

                                else:
                                    pszError = "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                    shouldContinue = False

                            else:
                                pszError = "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                        elif string[ptrString:ptrString + 2] == "EX":
                            ptrString += 2

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                (data, success) = parseHexGroup (string[ptrString:])

                                if success:
                                    ptrString += 2

                                    while string[ptrString] == " ":
                                        ptrString += 1

                                    if string[ptrString] == ")":
                                        ptrString += 1

                                        char      = unichr (data)
                                        charFound = True

                                    else:
                                        pszError = "Error: Expecting ')', found " + string[ptrString]
                                        shouldContinue = False

                                else:
                                    pszError = "Error: Couldn't parse hexgroup " + string[ptrString : ptrString + 2]
                                    shouldContinue = False

                            else:
                                pszError = "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                    elif string[ptrString] == "A":
                        ptrString += 1

                        if string[ptrString:ptrString + 4] == "SCII":
                            ptrString += 4

                            while string[ptrString] == " ":
                                ptrString += 1

                            if string[ptrString] == "(":
                                ptrString += 1

                                while string[ptrString] == " ":
                                    ptrString += 1

                                char      = string[ptrString]
                                charFound = True

                                while string[ptrString] == " ":
                                    ptrString += 1

                                if string[ptrString] == ")":
                                    ptrString += 1

                                else:
                                    pszError = "Error: Expecting ')', found " + string[ptrString]
                                    shouldContinue = False

                            else:
                                pszError = "Error: Expecting '(', found " + string[ptrString]
                                shouldContinue = False

                    elif string[ptrString] == '"':
                        ptrString += 1
                        inString   = True

            if charFound:
                command.append (char)

                charFound = False
    except Exception, e:
        pszError = "Caught exception"

    if pszError != None:
        return command
    else:
        return None

def encodeCommand (command):
    if len (command) == 0:
        return '""'

#   print "OmniEditUtils.py::encodeCommand: command =", command

    encodedCommand = ""
    fInString      = False
    fInSpecial     = False

    for cmd in command:
#       print "OmniEditUtils.py::encodeCommand: cmd = %s, fInString = %s, encodedCommand = %s, printable = %s, not \\v\\f\\n\\a\\r\\b\\t = %s" % (cmd, fInString, encodedCommand, cmd in string.printable, not cmd in "\v\f\n\a\r\b\t")

        if fInSpecial:
            if cmd == "_":
                fInSpecial = False

            encodedCommand += cmd

        elif     cmd in string.printable \
             and not cmd in "\v\f\n\a\r\b\t_":
            if not fInString:
                # Begin the string
                fInString       = True
                encodedCommand += '"'

            encodedCommand += cmd

        else:
            if fInString:
                # End the string
                encodedCommand += '" '
                fInString       = False

            if cmd == "\x1b":
                encodedCommand += "_ESC_ "
            elif cmd == "\0":
                encodedCommand += "_NUL_ "
            elif cmd == "\x0a":
                encodedCommand += "_LF_ "
            elif cmd == "\x0c":
                encodedCommand += "_FF_ "
            elif cmd == "\x0d":
                encodedCommand += "_CR_ "
            elif cmd == "_":
                fInSpecial = True
                encodedCommand += cmd
            else:
                encodedCommand += "HEX (" + hex (ord (cmd))[2:].zfill (2) + ") "

    if fInString:
        # End the string
        encodedCommand += '"'

    return encodedCommand.strip ()

def countChildren (elm):
    elm   = firstNode (elm.firstChild)
    count = 0

    while elm != None:
        count += 1

        elm = nextNode (elm)

    return count

def XMLHeader ():
    return """<?xml version="1.0" encoding="UTF-8"?>
<!--
     IBM Omni driver
     Copyright (c) International Business Machines Corp., 2000-2004

     This library is free software; you can redistribute it and/or modify
     it under the terms of the GNU Lesser General Public License as published
     by the Free Software Foundation; either version 2.1 of the License, or
     (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
     the GNU Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public License
     along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
-->"""

aDataPaths = [ '.',
               '/usr/share/Omni'
             ]

def addToDataPath (path):
    aDataPaths.append (path)

def findDataFile (pszFileName):
    import os

    for pszDataPath in aDataPaths:
        try:
            pszFileNameTest = "%s/%s" % (pszDataPath, pszFileName)

            (st_mode,
             st_ino,
             st_dev,
             st_nlink,
             st_uid,
             st_gid,
             st_size,
             st_atime,
             st_mtime,
             st_ctime) = os.stat (pszFileNameTest)

            return pszFileNameTest
        except:
            pass

    import sys

    pszErrorMsg = "Error: Cannot locate file %s in [" % (pszFileName)
    pszPaths    = None

    for pszDataPath in aDataPaths:
        if pszPaths == None:
            pszPaths = "'" + pszDataPath + "'"
        else:
            pszPaths += ", '" + pszDataPath + "'"

    if pszPaths != None:
        pszErrorMsg += pszPaths
    pszErrorMsg += "]"

    sys.exit (pszErrorMsg)

debugDict = {
  'DeviceInformationDialog::__init__':                                                False,
  'DeviceInformationDialog::on_ButtonDeviceInformationCancel_clicked':                False,
  'DeviceInformationDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked':      False,
  'DeviceInformationDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked':     False,
  'DeviceInformationDialog::on_ButtonDeviceInformationModify_clicked':                False,
  'DeviceInformationDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked': False,
  'DeviceInformationDialog::on_EntryDeviceInformationDeviceName_changed':             False,
  'DeviceInformationDialog::on_EntryDeviceInformationDriverName_changed':             False,
  'DeviceInformationDialog::on_EntryDeviceInformationPDLLevel_changed':               False,
  'DeviceInformationDialog::on_EntryDeviceInformationPDLMajor_changed':               False,
  'DeviceInformationDialog::on_EntryDeviceInformationPDLMinor_changed':               False,
  'DeviceInformationDialog::on_EntryDeviceInformationPDLSubLevel_changed':            False,
  'DeviceInformationWindow::on_window_delete':                                        False,
  'DeviceInformationWindow::on_window_destroy':                                       False,
  'Driver::isValid':                                                                  False,
  'Driver::toXML':                                                                    False,
  'OmniEditDevice.py::__main__':                                                      False,
  'OmniEditDeviceCommands::isValid':                                                  False,
  'OmniEditDeviceCommands::save':                                                     False,
  'OmniEditDeviceCommandsDialog::__init__':                                           False,
  'OmniEditDeviceCommandsDialog::deleteCommand':                                      False,
  'OmniEditDeviceCommandsDialog::on_ButtonCommandAdd_clicked':                        False,
  'OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked':                     False,
  'OmniEditDeviceCommandsDialog::on_ButtonCommandModify_clicked':                     False,
  'OmniEditDeviceCommandsDialog::on_EntryCommandCommand_changed':                     False,
  'OmniEditDeviceCommandsDialog::on_EntryCommandName_changed':                        False,
  'OmniEditDeviceCommandsDialog::on_destroy':                                         False,
  'OmniEditDeviceCommandsWindow::on_window_delete':                                   False,
  'OmniEditDeviceCommandsWindow::on_window_destroy':                                  False,
  'OmniEditDeviceConnectionDialog::__init__':                                         False,
  'OmniEditDeviceConnectionDialog::deleteConnection':                                 False,
  'OmniEditDeviceConnectionDialog::on_ButtonConnectionAdd_clicked':                   False,
  'OmniEditDeviceConnectionDialog::on_ButtonConnectionDelete_clicked':                False,
  'OmniEditDeviceConnectionDialog::on_ButtonConnectionModify_clicked':                False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionForm_changed':                   False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionMedia_changed':                  False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionName_changed':                   False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionOmniForm_changed':               False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionOmniTray_changed':               False,
  'OmniEditDeviceConnectionDialog::on_EntryConnectionTray_changed':                   False,
  'OmniEditDeviceConnections::isValid':                                               False,
  'OmniEditDeviceConnections::save':                                                  False,
  'OmniEditDeviceConnectionsWindow::on_window_delete':                                False,
  'OmniEditDeviceConnectionsWindow::on_window_destroy':                               False,
  'OmniEditDeviceCopies::isValid':                                                    False,
  'OmniEditDeviceCopies::save':                                                       False,
  'OmniEditDeviceCopiesDialog::__init__':                                             False,
  'OmniEditDeviceCopiesDialog::on_ButtonCopyDelete_clicked':                          False,
  'OmniEditDeviceCopiesDialog::on_ButtonCopyModify_clicked':                          False,
  'OmniEditDeviceCopiesDialog::on_EntryCopiesDefault_changed':                        False,
  'OmniEditDeviceCopiesDialog::on_EntryCopyCommand_changed':                          False,
  'OmniEditDeviceCopiesDialog::on_EntryCopyDeviceID_changed':                         False,
  'OmniEditDeviceCopiesDialog::on_EntryCopyMaximum_changed':                          False,
  'OmniEditDeviceCopiesDialog::on_EntryCopyMinimum_changed':                          False,
  'OmniEditDeviceCopiesDialog::on_EntryCopySimulationRequired_changed':               False,
  'OmniEditDeviceCopiesWindow::on_window_delete':                                     False,
  'OmniEditDeviceCopiesWindow::on_window_destroy':                                    False,
  'OmniEditDeviceDataDialog::__init':                                                 False,
  'OmniEditDeviceDatas::isValid':                                                     False,
  'OmniEditDeviceDatasDialog::deleteData':                                            False,
  'OmniEditDeviceDatasDialog::on_ButtonDataAdd_clicked':                              False,
  'OmniEditDeviceDatasDialog::on_ButtonDataDelete_clicked':                           False,
  'OmniEditDeviceDatasDialog::on_ButtonDataModify_clicked':                           False,
  'OmniEditDeviceDatasDialog::on_EntryDataData_changed':                              False,
  'OmniEditDeviceDatasDialog::on_EntryDataName_changed':                              False,
  'OmniEditDeviceDatasDialog::on_EntryTypeType_changed':                              False,
  'OmniEditDeviceDatasDialog::save':                                                  False,
  'OmniEditDeviceDatasWindow::on_window_delete':                                      False,
  'OmniEditDeviceDatasWindow::on_window_destroy':                                     False,
  'OmniEditDeviceDialog:%s':                                                          False,
  'OmniEditDeviceDialog::__init__':                                                   False,
  'OmniEditDeviceDialog::isSafeToExit':                                               False,
  'OmniEditDeviceDialog::on_DeviceWindow_delete_event':                               False,
  'OmniEditDeviceDialog::on_DeviceWindow_destroy_event':                              False,
  'OmniEditDeviceDialog::on_MenuAbout_activate':                                      False,
  'OmniEditDeviceDialog::on_MenuCommands_activate':                                   False,
  'OmniEditDeviceDialog::on_MenuConnections_activate':                                False,
  'OmniEditDeviceDialog::on_MenuCopies_activate':                                     False,
  'OmniEditDeviceDialog::on_MenuDatas_activate':                                      False,
  'OmniEditDeviceDialog::on_MenuForms_activate':                                      False,
  'OmniEditDeviceDialog::on_MenuGammaTables_activate':                                False,
  'OmniEditDeviceDialog::on_MenuMedias_activate':                                     False,
  'OmniEditDeviceDialog::on_MenuNumberUps_activate':                                  False,
  'OmniEditDeviceDialog::on_MenuOrientations_activate':                               False,
  'OmniEditDeviceDialog::on_MenuOutputBins_activate':                                 False,
  'OmniEditDeviceDialog::on_MenuPrintModes_activate':                                 False,
  'OmniEditDeviceDialog::on_MenuResolutions_activate':                                False,
  'OmniEditDeviceDialog::on_MenuScalings_activate':                                   False,
  'OmniEditDeviceDialog::on_MenuSheetCollates_activate':                              False,
  'OmniEditDeviceDialog::on_MenuSides_activate':                                      False,
  'OmniEditDeviceDialog::on_MenuStitchings_activate':                                 False,
  'OmniEditDeviceDialog::on_MenuStrings_activate':                                    False,
  'OmniEditDeviceDialog::on_MenuTrays_activate':                                      False,
  'OmniEditDeviceDialog::on_MenuTrimmings_activate':                                  False,
  'OmniEditDeviceDialog::on_TreeView_changed':                                        False,
  'OmniEditDeviceDialog::save':                                                       False,
  'OmniEditDeviceDialog:on_MenuPrintModes_activate':                                  False,
  'OmniEditDeviceForms.py::isValidName':                                              False,
  'OmniEditDeviceForms::isValid':                                                     False,
  'OmniEditDeviceForms::save':                                                        False,
  'OmniEditDeviceFormsDialog::__init__':                                              False,
  'OmniEditDeviceFormsDialog::deleteForm':                                            False,
  'OmniEditDeviceFormsDialog::on_ButtonFormAdd_clicked':                              False,
  'OmniEditDeviceFormsDialog::on_ButtonFormDelete_clicked':                           False,
  'OmniEditDeviceFormsDialog::on_ButtonFormModify_clicked':                           False,
  'OmniEditDeviceFormsDialog::on_CheckButtonFormDefault_toggled':                     False,
  'OmniEditDeviceFormsDialog::on_EntryFormCapabilities_changed':                      False,
  'OmniEditDeviceFormsDialog::on_EntryFormCommand_changed':                           False,
  'OmniEditDeviceFormsDialog::on_EntryFormDeviceID_changed':                          False,
  'OmniEditDeviceFormsDialog::on_EntryFormHCCBottom_changed':                         False,
  'OmniEditDeviceFormsDialog::on_EntryFormHCCLeft_changed':                           False,
  'OmniEditDeviceFormsDialog::on_EntryFormHCCRight_changed':                          False,
  'OmniEditDeviceFormsDialog::on_EntryFormHCCTop_changed':                            False,
  'OmniEditDeviceFormsDialog::on_EntryFormName_changed':                              False,
  'OmniEditDeviceFormsDialog::on_EntryFormOmniName_changed':                          False,
  'OmniEditDeviceFormsWindow::on_window_delete':                                      False,
  'OmniEditDeviceFormsWindow::on_window_destroy':                                     False,
  'OmniEditDeviceGammaTables::isValid':                                               False,
  'OmniEditDeviceGammaTables::save':                                                  False,
  'OmniEditDeviceGammaTablesDialog::__init__':                                        False,
  'OmniEditDeviceGammaTablesDialog::deleteGammaTable':                                False,
  'OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableAdd_clicked':                  False,
  'OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked':               False,
  'OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableModify_clicked':               False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCBias_changed':                 False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCGamma_changed':                False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableDitherCatagory_changed':        False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKBias_changed':                 False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKGamma_changed':                False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMBias_changed':                 False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMGamma_changed':                False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMedia_changed':                 False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableOmniResolution_changed':        False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTablePrintMode_changed':             False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableResolution_changed':            False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYBias_changed':                 False,
  'OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYGamma_changed':                False,
  'OmniEditDeviceGammaTablesWindow::on_window_delete':                                False,
  'OmniEditDeviceGammaTablesWindow::on_window_destroy':                               False,
  'OmniEditDeviceMedias::isValid':                                                    False,
  'OmniEditDeviceMedias::save':                                                       False,
  'OmniEditDeviceMediasDialog::__init__':                                             False,
  'OmniEditDeviceMediasDialog::deleteMedia':                                          False,
  'OmniEditDeviceMediasDialog::on_ButtonMediaAdd_clicked':                            False,
  'OmniEditDeviceMediasDialog::on_ButtonMediaDelete_clicked':                         False,
  'OmniEditDeviceMediasDialog::on_ButtonMediaModify_clicked':                         False,
  'OmniEditDeviceMediasDialog::on_CheckButtonMediaDefault_toggled':                   False,
  'OmniEditDeviceMediasDialog::on_EntryMediaAbsorption_changed':                      False,
  'OmniEditDeviceMediasDialog::on_EntryMediaColorAdjustRequired_changed':             False,
  'OmniEditDeviceMediasDialog::on_EntryMediaCommand_changed':                         False,
  'OmniEditDeviceMediasDialog::on_EntryMediaDeviceID_changed':                        False,
  'OmniEditDeviceMediasDialog::on_EntryMediaName_changed':                            False,
  'OmniEditDeviceMediasWindow::on_window_delete':                                     False,
  'OmniEditDeviceMediasWindow:on_window_destroy':                                     False,
  'OmniEditDeviceNumberUps::isValid':                                                 False,
  'OmniEditDeviceNumberUps::save':                                                    False,
  'OmniEditDeviceNumberUpsDialog::__init__':                                          False,
  'OmniEditDeviceNumberUpsDialog::deleteNup':                                         False,
  'OmniEditDeviceNumberUpsDialog::findNUp':                                           False,
  'OmniEditDeviceNumberUpsDialog::on_ButtonNumberUpAdd_clicked':                      False,
  'OmniEditDeviceNumberUpsDialog::on_ButtonNumberUpDelete_clicked':                   False,
  'OmniEditDeviceNumberUpsDialog::on_ButtonNumberUpModify_clicked':                   False,
  'OmniEditDeviceNumberUpsDialog::on_CheckButtonNumberUpDefault_toggled':             False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpCommand_changed':                   False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpDeviceID_changed':                  False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpDirection_changed':     False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpSimulationRequired_changed':        False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpX_changed':                         False,
  'OmniEditDeviceNumberUpsDialog::on_EntryNumberUpY_changed':                         False,
  'OmniEditDeviceNumberUpsWindow:on_window_delete':                                   False,
  'OmniEditDeviceNumberUpsWindow:on_window_destroy':                                  False,
  'OmniEditDeviceOrientationDialog::__init__':                                        False,
  'OmniEditDeviceOrientations::isValid':                                              False,
  'OmniEditDeviceOrientations::save':                                                 False,
  'OmniEditDeviceOrientationsDialog::deleteOrientation':                              False,
  'OmniEditDeviceOrientationsDialog::on_ButtonOrientationAdd_clicked':                False,
  'OmniEditDeviceOrientationsDialog::on_ButtonOrientationDelete_clicked':             False,
  'OmniEditDeviceOrientationsDialog::on_ButtonOrientationModify_clicked':             False,
  'OmniEditDeviceOrientationsDialog::on_CheckButtonOrientationDefault_toggled':       False,
  'OmniEditDeviceOrientationsDialog::on_EntryOrientationDeviceID_changed':            False,
  'OmniEditDeviceOrientationsDialog::on_EntryOrientationName_changed':                False,
  'OmniEditDeviceOrientationsDialog::on_EntryOrientationOmniName_changed':            False,
  'OmniEditDeviceOrientationsDialog::on_EntryOrientationSimulationRequired_changed':  False,
  'OmniEditDeviceOrientationsWindow::on_window_delete':                               False,
  'OmniEditDeviceOrientationsWindow::on_window_destroy':                              False,
  'OmniEditDeviceOutputBins::isValid':                                                False,
  'OmniEditDeviceOutputBins::save':                                                   False,
  'OmniEditDeviceOutputBinsDialog::__init__':                                         False,
  'OmniEditDeviceOutputBinsDialog::deleteOutputBin':                                  False,
  'OmniEditDeviceOutputBinsDialog::on_ButtonOutputBinAdd_clicked':                    False,
  'OmniEditDeviceOutputBinsDialog::on_ButtonOutputBinDelete_clicked':                 False,
  'OmniEditDeviceOutputBinsDialog::on_ButtonOutputBinModify_clicked':                 False,
  'OmniEditDeviceOutputBinsDialog::on_CheckButtonOutputBinDefault_toggled':           False,
  'OmniEditDeviceOutputBinsDialog::on_EntryOutputBinCommand_changed':                 False,
  'OmniEditDeviceOutputBinsDialog::on_EntryOutputBinDeviceID_changed':                False,
  'OmniEditDeviceOutputBinsDialog::on_EntryOutputBinName_changed':                    False,
  'OmniEditDeviceOutputBinsWindow::on_window_delete':                                 False,
  'OmniEditDeviceOutputBinsWindow::on_window_destroy':                                False,
  'OmniEditDevicePrintModes::isValid':                                                False,
  'OmniEditDevicePrintModes::save':                                                   False,
  'OmniEditDevicePrintModesDialog::__init__':                                         False,
  'OmniEditDevicePrintModesDialog::deletePrintMode':                                  False,
  'OmniEditDevicePrintModesDialog::on_ButtonPrintModeAdd_clicked':                    False,
  'OmniEditDevicePrintModesDialog::on_ButtonPrintModeDelete_clicked':                 False,
  'OmniEditDevicePrintModesDialog::on_ButtonPrintModeModify_clicked':                 False,
  'OmniEditDevicePrintModesDialog::on_CheckButtonPrintModeDefault_toggled':           False,
  'OmniEditDevicePrintModesDialog::on_EntryPrintModeDeviceID_changed':                False,
  'OmniEditDevicePrintModesDialog::on_EntryPrintModeLogicalCount_changed':            False,
  'OmniEditDevicePrintModesDialog::on_EntryPrintModeName_changed':                    False,
  'OmniEditDevicePrintModesDialog::on_EntryPrintModePhysicalCount_changed':           False,
  'OmniEditDevicePrintModesDialog::on_EntryPrintModePlanes_changed':                  False,
  'OmniEditDevicePrintModesWindow::on_window_delete':                                 False,
  'OmniEditDevicePrintModesWindow::on_window_destroy':                                False,
  'OmniEditDeviceResolutions::isValid':                                               False,
  'OmniEditDeviceResolutions::save':                                                  False,
  'OmniEditDeviceResolutionsDialog::__init__':                                        False,
  'OmniEditDeviceResolutionsDialog::deleteResolution':                                False,
  'OmniEditDeviceResolutionsDialog::on_ButtonResolutionAdd_clicked':                  False,
  'OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked':               False,
  'OmniEditDeviceResolutionsDialog::on_ButtonResolutionModify_clicked':               False,
  'OmniEditDeviceResolutionsDialog::on_CheckButtonResolutionDefault_toggled':         False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionCapability_changed':            False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionCommand_changed':               False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionDestinationBitsPerPel_changed': False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionDeviceID_changed':              False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionName_changed':                  False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionOmniName_changed':              False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionScanlineMultiple_changed':      False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionXInternalRes_changed':          False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionXRes_changed':                  False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionYInternalRes_changed':          False,
  'OmniEditDeviceResolutionsDialog::on_EntryResolutionYRes_changed':                  False,
  'OmniEditDeviceResolutionsWindow::on_window_delete':                                False,
  'OmniEditDeviceResolutionsWindow::on_window_destroy':                               False,
  'OmniEditDeviceScalings::isValid':                                                  False,
  'OmniEditDeviceScalings::save':                                                     False,
  'OmniEditDeviceScalingsDialog::__init__':                                           False,
  'OmniEditDeviceScalingsDialog::deleteScaling':                                      False,
  'OmniEditDeviceScalingsDialog::on_ButtonScalingAdd_clicked':                        False,
  'OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked':                     False,
  'OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked':                     False,
  'OmniEditDeviceScalingsDialog::on_CheckButtonScalingDefault_toggled':               False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingAllowedType_changed':                 False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingCommand_changed':                     False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingDefaultPercentage_changed':           False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingDefault_changed':                     False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingDeviceID_changed':                    False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingMaximum_changed':                     False,
  'OmniEditDeviceScalingsDialog::on_EntryScalingMinimum_changed':                     False,
  'OmniEditDeviceScalingsWindow::on_window_delete':                                   False,
  'OmniEditDeviceScalingsWindow::on_window_destroy':                                  False,
  'OmniEditDeviceSheetCollates::isValid':                                             False,
  'OmniEditDeviceSheetCollates::save':                                                False,
  'OmniEditDeviceSheetCollatesDialog::__init__':                                      False,
  'OmniEditDeviceSheetCollatesDialog::deleteSheetCollate':                            False,
  'OmniEditDeviceSheetCollatesDialog::on_ButtonSheetCollateAdd_clicked':              False,
  'OmniEditDeviceSheetCollatesDialog::on_ButtonSheetCollateDelete_clicked':           False,
  'OmniEditDeviceSheetCollatesDialog::on_ButtonSheetCollateModify_clicked':           False,
  'OmniEditDeviceSheetCollatesDialog::on_CheckButtonSheetCollateDefault_toggled':     False,
  'OmniEditDeviceSheetCollatesDialog::on_EntrySheetCollateCommand_changed':           False,
  'OmniEditDeviceSheetCollatesDialog::on_EntrySheetCollateDeviceID_changed':          False,
  'OmniEditDeviceSheetCollatesDialog::on_EntrySheetCollateName_changed':              False,
  'OmniEditDeviceSheetCollatesWindow::on_window_delete':                              False,
  'OmniEditDeviceSheetCollatesWindow::on_window_destroy':                             False,
  'OmniEditDeviceSides::isValid':                                                     False,
  'OmniEditDeviceSides::save':                                                        False,
  'OmniEditDeviceSidesDialog::__init__':                                              False,
  'OmniEditDeviceSidesDialog::deleteSide':                                            False,
  'OmniEditDeviceSidesDialog::on_ButtonSideAdd_clicked':                              False,
  'OmniEditDeviceSidesDialog::on_ButtonSideDelete_clicked':                           False,
  'OmniEditDeviceSidesDialog::on_ButtonSideModify_clicked':                           False,
  'OmniEditDeviceSidesDialog::on_CheckButtonSideDefault_toggled':                     False,
  'OmniEditDeviceSidesDialog::on_EntrySideCommand_changed':                           False,
  'OmniEditDeviceSidesDialog::on_EntrySideDeviceID_changed':                          False,
  'OmniEditDeviceSidesDialog::on_EntrySideName_changed':                              False,
  'OmniEditDeviceSidesDialog::on_EntrySideSimulationRequired_changed':                False,
  'OmniEditDeviceSidesWindow::on_window_delete':                                      False,
  'OmniEditDeviceSidesWindow::on_window_destroy':                                     False,
  'OmniEditDeviceStitchings::isValid':                                                False,
  'OmniEditDeviceStitchings::save':                                                   False,
  'OmniEditDeviceStitchingsDialog::__init__':                                         False,
  'OmniEditDeviceStitchingsDialog::deleteStitching':                                  False,
  'OmniEditDeviceStitchingsDialog::on_ButtonStitchingAdd_clicked':                    False,
  'OmniEditDeviceStitchingsDialog::on_ButtonStitchingDelete_clicked':                 False,
  'OmniEditDeviceStitchingsDialog::on_ButtonStitchingModify_clicked':                 False,
  'OmniEditDeviceStitchingsDialog::on_CheckButtonStitchingDefault_toggled':           False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingAngle_changed':                   False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingCommand_changed':                 False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingCount_changed':                   False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingDeviceID_changed':                False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingPosition_changed':                False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingReferenceEdge_changed':           False,
  'OmniEditDeviceStitchingsDialog::on_EntryStitchingType_changed':                    False,
  'OmniEditDeviceStitchingsWindow::on_window_delete':                                 False,
  'OmniEditDeviceStitchingsWindow::on_window_destroy':                                False,
  'OmniEditDeviceStringDialog::__init__':                                             False,
  'OmniEditDeviceStringDialog::on_ButtonStringCCAdd_clicked':                         False,
  'OmniEditDeviceStringDialog::on_ButtonStringCCDelete_clicked':                      False,
  'OmniEditDeviceStringDialog::on_ButtonStringCCModify_clicked':                      False,
  'OmniEditDeviceStringDialog::on_ButtonStringKeyAdd_clicked':                        False,
  'OmniEditDeviceStringDialog::on_ButtonStringKeyDelete_clicked':                     False,
  'OmniEditDeviceStringDialog::on_ButtonStringKeyModify_clicked':                     False,
  'OmniEditDeviceStringDialog::on_ButtonStringModify_clicked':                        False,
  'OmniEditDeviceStringDialog::on_EntryStringCountryCode_changed':                    False,
  'OmniEditDeviceStringDialog::on_EntryStringKeyName_changed':                        False,
  'OmniEditDeviceStringDialog::on_EntryStringTranslation_changed':                    False,
  'OmniEditDeviceStringDialog::on_TreeViewString_changed':                            False,
  'OmniEditDeviceStrings::isValid':                                                   False,
  'OmniEditDeviceStrings::save':                                                      False,
  'OmniEditDeviceStringsWindow::on_window_delete':                                    False,
  'OmniEditDeviceStringsWindow::on_window_destroy':                                   False,
  'OmniEditDeviceTrays::isValid':                                                     False,
  'OmniEditDeviceTrays::save':                                                        False,
  'OmniEditDeviceTraysDialog::__init__':                                              False,
  'OmniEditDeviceTraysDialog::deleteTray':                                            False,
  'OmniEditDeviceTraysDialog::on_ButtonTrayAdd_clicked':                              False,
  'OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked':                           False,
  'OmniEditDeviceTraysDialog::on_ButtonTrayModify_clicked':                           False,
  'OmniEditDeviceTraysDialog::on_CheckButtonTrayDefault_toggled':                     False,
  'OmniEditDeviceTraysDialog::on_EntryTrayCommand_changed':                           False,
  'OmniEditDeviceTraysDialog::on_EntryTrayDeviceID_changed':                          False,
  'OmniEditDeviceTraysDialog::on_EntryTrayName_changed':                              False,
  'OmniEditDeviceTraysDialog::on_EntryTrayOmniName_changed':                          False,
  'OmniEditDeviceTraysDialog::on_EntryTrayTrayType_changed':                          False,
  'OmniEditDeviceTraysWindow::on_window_delete':                                      False,
  'OmniEditDeviceTraysWindow::on_window_destroy':                                     False,
  'OmniEditDeviceTrimmings::isValid':                                                 False,
  'OmniEditDeviceTrimmings::save':                                                    False,
  'OmniEditDeviceTrimmingsDialog::__init__':                                          False,
  'OmniEditDeviceTrimmingsDialog::deleteTrimming':                                    False,
  'OmniEditDeviceTrimmingsDialog::on_ButtonTrimmingAdd_clicked':                      False,
  'OmniEditDeviceTrimmingsDialog::on_ButtonTrimmingDelete_clicked':                   False,
  'OmniEditDeviceTrimmingsDialog::on_ButtonTrimmingModify_clicked':                   False,
  'OmniEditDeviceTrimmingsDialog::on_CheckButtonTrimmingDefault_toggled':             False,
  'OmniEditDeviceTrimmingsDialog::on_EntryTrimmingCommand_changed':                   False,
  'OmniEditDeviceTrimmingsDialog::on_EntryTrimmingDeviceID_changed':                  False,
  'OmniEditDeviceTrimmingsDialog::on_EntryTrimmingName_changed':                      False,
  'OmniEditDeviceTrimmingsWindow::on_window_delete':                                  False,
  'OmniEditDeviceTrimmingsWindow::on_window_destroy':                                 False
}

def shouldOutputDebug (key):
    if debugDict.has_key (key):
        return debugDict[key]

    return True

if __name__ == "__main__":
    commands  = [ "_ESC_ &quot;(e&quot; HEX(02) HEX(00) HEX(01) HEX(00)" ]
#   commands  = [ '""',
#                 '"BJLEND" _LF_',
#                 '"%c"',
#                 '"%c%c"',
#                 '"ControlMode=BJ" _LF_',
#                 '"ControlMode=Common" _LF_',
#                 '"ControlMode=LQ" _LF_',
#                 '_CR_',
#                 '_CR_ _LF_',
#                 '_ESC_ "@"',
#                 '_ESC_ "%-12345X"',
#                 '_ESC_ "%-12345X@PJL ENTER LANGUAGE=PCL3GUI" _LF_',
#                 '_ESC_ "%-12345X@PJL ENTER LANGUAGE=PCL3" _LF_',
#                 '_ESC_ "%-12345X@PJL ENTER LANGUAGE=PCL " _LF_',
#                 '_ESC_ "3%c"',
#                 '_ESC_ "(a" HEX (01) _NUL_ _NUL_ _ESC_ "(b" HEX (01) _NUL_ _NUL_ _ESC_ "@"',
#                 '_ESC_  "&amp;k0W"',
#                 '_ESC_  "&amp;K1W"',
#                 '_ESC_  "&amp;k5W"',
#                 '_ESC_  "&amp;k6W"',
#                 '_ESC_  "&amp;l0E"',
#                 '_ESC_  "&amp;l0M"',
#                 '_ESC_  "&amp;l0O"',
#                 '_ESC_  "&amp;l1O"',
#                 '_ESC_  "&amp;l2M"',
#                 '_ESC_  "&amp;l3M"',
#                 '_ESC_  "&amp;l4M"',
#                 '_ESC_  "&amp;s0C"',
#                 '_ESC_  "&amp;s1C"',
#                 '_ESC_ "&amp;u%nD"',
#                 '_ESC_ "(A" "%w%c"',
#                 '_ESC_ "*b%dM"',
#                 '_ESC_ "*b%dV"',
#                 '_ESC_ "*b%dW"',
#                 '_ESC_ "*b%dY"',
#                 '_ESC_ "(b" HEX (01) _NUL_ _NUL_ _ESC_ "@"',
#                 '_ESC_ "(b" HEX2S(00,01) "%c"',
#                 '_ESC_ "*b%nM"',
#                 '_ESC_ "*b%nV"',
#                 '_ESC_ "*b%nW"',
#                 '_ESC_ "*b%nY"',
#                 '_ESC_ "(c" HEX2S(00,01) HEX(01)',
#                 '_ESC_ "(c" HEX2S(00,03) HEX(10) HEX(11) HEX(10)',
#                 '_ESC_ "E"',
#                 '_ESC_ "E" _ESC_  "&amp;k0G"',
#                 '_ESC_ "(e" HEX (02) _NUL_ _NUL_ HEX (01)',
#                 '_ESC_ "(e" HEX(02) _NUL_ _NUL_ "%w"',
#                 '_ESC_ "(e" HEX2S(00,02) "%W"',
#                 '_ESC_ "*g%nW"',
#                 '_ESC_ "~" HEX (01) HEX (00) HEX (00)',
#                 '_ESC_ HEX (19) &quot;0&quot;',
#                 '_ESC_ HEX (19) &quot;1&quot;',
#                 '_ESC_ HEX (19) &quot;2&quot;',
#                 '_ESC_ HEX (19) &quot;B&quot;',
#                 '_ESC_ HEX (19) &quot;F&quot;',
#                 '_ESC_ HEX (19) &quot;?&quot;',
#                 '_ESC_ "~" HEX (1C) HEX (00) HEX (03) HEX (03) "%W"',
#                 '_ESC_ "~" HEX (1D) HEX (00) HEX (03) HEX (03) "%W"',
#                 '_ESC_ "~" HEX (1D) HEX (00) HEX (03) HEX (04) "%W"',
#                 '_ESC_ HEX (24) HEX (25) HEX (77)',
#                 '_ESC_ HEX (28) HEX (47) HEX (01) _NUL_ HEX (01)',
#                 '_ESC_ HEX (28) HEX (56) HEX (02) _NUL_ HEX (25) HEX (77)',
#                 '_ESC_ "~" HEX (28) "%W"',
#                 '_ESC_ "%%" HEX (31) "%c%c"',
#                 '_ESC_ "%%" HEX (35) "%c%c"',
#                 '_ESC_ HEX (40)',
#                 '_ESC_ "~" HEX (50) HEX (00) HEX (01) HEX (00)',
#                 '_ESC_ "~" HEX (50) HEX (00) HEX (01) HEX (03)',
#                 '_ESC_ HEX (55) HEX (25) HEX (63)',
#                 '_ESC_ HEX (5C) &quot;%w&quot;',
#                 '_ESC_ HEX(7E) "F" HEX(00) HEX(05) HEX(00) HEX(00) "%c" HEX(00) HEX(00)',
#                 '_ESC_ HEX(7E) HEX(04) HEX(00) HEX(03) HEX(00) "%c%c"',
#                 '_ESC_ HEX (7e) HEX (0e) HEX (00) HEX (01) HEX (00)""',
#                 '_ESC_ HEX(7E) HEX(10) HEX(00) HEX(01) "%c"',
#                 '_ESC_ HEX (7e) HEX (11) HEX (00) HEX (01) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (12) HEX (00) HEX (01) HEX (11)""',
#                 '_ESC_ HEX (7e) HEX (13) HEX (00) HEX (01) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (13) HEX (00) HEX (04) HEX (03) HEX (00) HEX (81) HEX (5c)""',
#                 '_ESC_ HEX (7e) HEX (1e) HEX (00) HEX (02) HEX (00) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (1f) HEX (00) HEX (02) HEX (00) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (33) HEX (00) HEX (02) HEX (01) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (3b) HEX (00) HEX (04) HEX (00) HEX (00) HEX (00) "%c" ""',
#                 '_ESC_ HEX (7e) HEX (3b) HEX (00) HEX (04) HEX (00) HEX (00) HEX (00) HEX (01)""',
#                 '_ESC_ HEX (7e) HEX (3c) HEX (00) HEX (08) HEX (00) HEX (00) HEX (00) HEX (01) HEX (00) HEX (00) HEX (00) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (74) HEX (00) HEX (05) HEX (00) HEX (00) HEX (00) HEX (00) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (76) HEX (00) HEX (01) HEX (00)""',
#                 '_ESC_ HEX (7e) HEX (84) "%W%c%W%W"',
#                 '_ESC_ "~" HEX (89) "%W"',
#                 '_ESC_ "%k%dG"',
#                 '_ESC_ "%k%nG"',
#                 '_ESC_ "*l%nW"',
#                 '_ESC_ "@" _NUL_ _NUL_',
#                 '_ESC_ "*p%dX"',
#                 '_ESC_ "*p%dx%dY"',
#                 '_ESC_ "*p%nX"',
#                 '_ESC_ "*p%nx%nY"',
#                 '_ESC_ "*p%nY"',
#                 '_ESC_ "(q" HEX2S(00,01) HEX(01)',
#                 '_ESC_ &quot;2&quot;',
#                 '_ESC_ &quot;3%c&quot;',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX (13) HEX(EE) HEX (1E) HEX(90) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX(1E) HEX(8F) &quot;,K&quot; HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) HEX(26) HEX(17) HEX(36) HEX(B3) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;6&quot; HEX(B3) &quot;Ne&quot; HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;,K?&quot;HEX(8F) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;?&quot; HEX (8F) &quot;Z&quot; HEX(CC) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;-&quot; HEX(99) &quot;L&quot; HEX(89) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) HEX (01) HEX (1B) HEX (01) HEX (1B) &quot;-&quot; HEX(99) &quot;;&quot; HEX(A9) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) HEX (14) &quot;_&quot; HEX (1F) HEX (01) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) HEX (1F) HEX (01) &quot;,&quot; HEX (BB) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;&amp;&quot; HEX (88) &quot;7$&quot; HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;&lt;&quot; HEX (1A) &quot;]&quot; HEX (DA) HEX (02)',
#                 "_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1)  &quot;'&quot; HEX (02) &quot;9J&quot; HEX (02)",
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;.&quot; HEX (A0) &quot;L&quot; HEX (FA) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;.&quot; HEX (A0)  &quot;&lt;&quot; HEX (1A) HEX (02)',
#                 '_ESC_ &quot;~8&quot; _NUL_ HEX (09) _NUL_ HEX (E1) _NUL_ HEX (E1) &quot;,&quot; HEX (BC) &quot;@&quot; _NUL_ HEX (02)',
#                 '_ESC_ &quot;&amp;l0H&quot;',
#                 '_ESC_ &quot;&amp;l0M&quot;',
#                 '_ESC_ &quot;&amp;l100A&quot;',
#                 '_ESC_ &quot;&amp;l100H&quot;',
#                 '_ESC_ &quot;&amp;l101A&quot;',
#                 '_ESC_  &quot;&amp;l109A&quot;',
#                 '_ESC_ &quot;&amp;l10H&quot;',
#                 '_ESC_  &quot;&amp;l15A&quot;',
#                 '_ESC_  &quot;&amp;l16A&quot;',
#                 '_ESC_  &quot;&amp;l1A&quot;',
#                 '_ESC_ &quot;&amp;l1A&quot;',
#                 '_ESC_  &quot;&amp;l1H&quot;',
#                 '_ESC_ &quot;&amp;l1H&quot;',
#                 '_ESC_ &quot;&amp;l1J&quot;',
#                 '_ESC_ &quot;&amp;l2000A&quot;',
#                 '_ESC_ &quot;&amp;l2001A&quot;',
#                 '_ESC_ &quot;&amp;l2007A&quot;',
#                 '_ESC_ &quot;&amp;l2008A&quot;',
#                 '_ESC_ &quot;&amp;l2009A&quot;',
#                 '_ESC_ &quot;&amp;l2011A&quot;',
#                 '_ESC_ &quot;&amp;l2012A&quot;',
#                 '_ESC_ &quot;&amp;l2030A&quot;',
#                 '_ESC_ &quot;&amp;l2032A&quot;',
#                 '_ESC_ &quot;&amp;l2033A&quot;',
#                 '_ESC_ &quot;&amp;l2034A&quot;',
#                 '_ESC_ &quot;&amp;l2036A&quot;',
#                 '_ESC_ &quot;&amp;l2037A&quot;',
#                 '_ESC_ &quot;&amp;l2038A&quot;',
#                 '_ESC_ &quot;&amp;l2039A&quot;',
#                 '_ESC_ &quot;&amp;l2040A&quot;',
#                 '_ESC_ &quot;&amp;l2041A&quot;',
#                 '_ESC_  &quot;&amp;l25A&quot;',
#                 '_ESC_ &quot;&amp;l25A&quot;',
#                 '_ESC_  &quot;&amp;l26A&quot;',
#                 '_ESC_ &quot;&amp;l26A&quot;',
#                 '_ESC_ &quot;&amp;l26A&quot;_ESC_ &quot;&amp;l-190U&quot;',
#                 '_ESC_  &quot;&amp;l27A&quot;',
#                 '_ESC_ &quot;&amp;l27A&quot;',
#                 '_ESC_ &quot;&amp;l28A&quot;',
#                 '_ESC_  &quot;&amp;l29A&quot;',
#                 '_ESC_  &quot;&amp;l2A&quot;',
#                 '_ESC_ &quot;&amp;l2A&quot;',
#                 '_ESC_ &quot;&amp;l2A&quot;_ESC_ &quot;&amp;l-230U&quot;',
#                 '_ESC_  &quot;&amp;l2H&quot;',
#                 '_ESC_ &quot;&amp;l2H&quot;',
#                 '_ESC_ &quot;&amp;l30025A&quot;',
#                 '_ESC_ &quot;&amp;l30092A&quot;',
#                 '_ESC_ &quot;&amp;l37A&quot;',
#                 '_ESC_  &quot;&amp;l3A&quot;',
#                 '_ESC_ &quot;&amp;l3A&quot;',
#                 '_ESC_  &quot;&amp;l3H&quot;',
#                 '_ESC_ &quot;&amp;l3H&quot;',
#                 '_ESC_  &quot;&amp;l45A&quot;',
#                 '_ESC_ &quot;&amp;l45A&quot;',
#                 '_ESC_  &quot;&amp;l46A&quot;',
#                 '_ESC_ &quot;&amp;l46A&quot;',
#                 '_ESC_ &quot;&amp;l4H&quot;',
#                 '_ESC_ &quot;&amp;l5H&quot;',
#                 '_ESC_ &quot;&amp;l60P&quot;',
#                 '_ESC_ &quot;&amp;l65A&quot;',
#                 '_ESC_ &quot;&amp;l66P&quot;',
#                 '_ESC_ &quot;&amp;l6A&quot;',
#                 '_ESC_  &quot;&amp;l6A&quot; _NUL_',
#                 '_ESC_ &quot;&amp;l6H&quot;',
#                 '_ESC_ &quot;&amp;l70A&quot;',
#                 '_ESC_ &quot;&amp;l70P&quot;',
#                 '_ESC_  &quot;&amp;l71A&quot;',
#                 '_ESC_ &quot;&amp;l71A&quot;',
#                 '_ESC_ &quot;&amp;l72A&quot;',
#                 '_ESC_  &quot;&amp;l73A&quot;',
#                 '_ESC_  &quot;&amp;l74A&quot;',
#                 '_ESC_  &quot;&amp;l75A&quot;',
#                 '_ESC_ &quot;&amp;l7H&quot;',
#                 '_ESC_ &quot;&amp;l80A&quot;',
#                 '_ESC_  &quot;&amp;l-81A&quot;',
#                 '_ESC_  &quot;&amp;l81A&quot;',
#                 '_ESC_ &quot;&amp;l81A&quot;',
#                 '_ESC_  &quot;&amp;l-81&quot;',
#                 '_ESC_ &quot;&amp;l84P&quot;',
#                 '_ESC_ &quot;&amp;l89A&quot;',
#                 '_ESC_  &quot;&amp;l8H&quot;',
#                 '_ESC_ &quot;&amp;l8H&quot;',
#                 '_ESC_  &quot;&amp;l-90A&quot;',
#                 '_ESC_  &quot;&amp;l90A&quot;',
#                 '_ESC_ &quot;&amp;l90A&quot;',
#                 '_ESC_  &quot;&amp;l91A&quot;',
#                 '_ESC_ &quot;&amp;l91A&quot;',
#                 '_ESC_  &quot;&amp;l92A&quot;',
#                 '_ESC_  &quot;&amp;l93A&quot;',
#                 '_ESC_  &quot;&amp;l9H&quot;',
#                 '_ESC_ &quot;&amp;l9H&quot;',
#                 '_ESC_ &quot;&amp;n10WdCardstock&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock2&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock3&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock4&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock5&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock6&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock7&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock8&quot;',
#                 '_ESC_ &quot;&amp;n10WdTabstock9&quot;',
#                 '_ESC_ &quot;&amp;n10WdThickBlue&quot;',
#                 '_ESC_ &quot;&amp;n10WdThickGrey&quot;',
#                 '_ESC_ &quot;&amp;n10WdThickNone&quot;',
#                 '_ESC_ &quot;&amp;n10WdThickPink&quot;',
#                 '_ESC_ &quot;&amp;n11WdLetterhead&quot;',
#                 '_ESC_ &quot;&amp;n11WdPreprinted&quot;',
#                 '_ESC_ &quot;&amp;n11WdPrepunched&quot;',
#                 '_ESC_ &quot;&amp;n11WdSpecialRed&quot;',
#                 '_ESC_ &quot;&amp;n11WdThickGreen&quot;',
#                 '_ESC_ &quot;&amp;n11WdThickIvory&quot;',
#                 '_ESC_ &quot;&amp;n12WdSpecialBlue&quot;',
#                 '_ESC_ &quot;&amp;n12WdSpecialGrey&quot;',
#                 '_ESC_ &quot;&amp;n12WdSpecialPink&quot;',
#                 '_ESC_ &quot;&amp;n12WdThickOrange&quot;',
#                 '_ESC_ &quot;&amp;n12WdThickPurple&quot;',
#                 '_ESC_ &quot;&amp;n12WdThickYellow&quot;',
#                 '_ESC_ &quot;&amp;n12WdTranslucent&quot;',
#                 '_ESC_ &quot;&amp;n13WdSpecialGreen&quot;',
#                 '_ESC_ &quot;&amp;n13WdSpecialIvory&quot;',
#                 '_ESC_ &quot;&amp;n13WdTransparency&quot;',
#                 '_ESC_ &quot;&amp;n14WdSpecialOrange&quot;',
#                 '_ESC_ &quot;&amp;n14WdSpecialPurple&quot;',
#                 '_ESC_ &quot;&amp;n14WdSpecialYellow&quot;',
#                 '_ESC_ &quot;&amp;n15WdThickUserColor&quot;',
#                 '_ESC_ &quot;&amp;n16WdThickLetterhead&quot;',
#                 '_ESC_ &quot;&amp;n17WdSpecialUserColor&quot;',
#                 '_ESC_ &quot;&amp;n18WdSpecialLetterhead&quot;',
#                 '_ESC_ &quot;&amp;n5WdBond&quot;',
#                 '_ESC_ &quot;&amp;n6WdColor&quot;',
#                 '_ESC_ &quot;&amp;n6WdPlain&quot;',
#                 '_ESC_ &quot;&amp;n7WdLabels&quot;',
#                 '_ESC_ &quot;&amp;n7WdThick1&quot;',
#                 '_ESC_ &quot;&amp;n7WdThick2&quot;',
#                 '_ESC_ &quot;&amp;n7WdThick3&quot;',
#                 '_ESC_ &quot;&amp;n8WdCustom1&quot;',
#                 '_ESC_ &quot;&amp;n8WdCustom2&quot;',
#                 '_ESC_ &quot;&amp;n8WdCustom3&quot;',
#                 '_ESC_ &quot;&amp;n8WdCustom4&quot;',
#                 '_ESC_ &quot;&amp;n8WdCustom5&quot;',
#                 '_ESC_ &quot;&amp;n8WdSpecial&quot;',
#                 '_ESC_ &quot;&amp;n9WdEnvelope&quot;',
#                 '_ESC_ &quot;&amp;n9WdRecycled&quot;',
#                 '_ESC_ &quot;&amp;n9WdTabstock&quot;',
#                 '_ESC_ &quot;&amp;n9WdThickRed&quot;',
#                 '_ESC_ &quot;*&amp;&quot;',
#                 '_ESC_ &quot;*&amp;%w&quot;',
#                 '_ESC_ &quot;*A&quot;',
#                 '_ESC_ &quot;*A%w&quot;',
#                 '_ESC_ &quot;.%c%c%c%c%w&quot;',
#                 '_ESC_ &quot;C%c&quot;',
#                 '_ESC_ &quot;(C&quot; HEX(02) HEX(00) &quot;%w&quot;',
#                 '_ESC_ &quot;(C&quot; HEX(02) HEX(00) &quot;%W&quot;',
#                 '_ESC_ &quot;*%c%w&quot;',
#                 '_ESC_ &quot;(d&quot; HEX (02) _NUL_ HEX (01) &quot;h&quot;',
#                 '_ESC_ &quot;(d&quot; HEX (02) _NUL_ _NUL_ HEX (B4)',
#                 '_ESC_ &quot;(d&quot; HEX (02) _NUL_ _NUL_ &quot;Z&quot;',
#                 '_ESC_ &quot;(D&quot; HEX (04) HEX (00) &quot;%w%c%c&quot;',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (01) &quot;h&quot; HEX (02) HEX (D0)',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (01) &quot;,&quot; HEX (01) &quot;,&quot;',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) HEX (D0) HEX (02) HEX (D0)',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) HEX (D0) HEX (05) HEX (A0)',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) &quot;X&quot; HEX (02) &quot;X&quot;',
#                 '_ESC_ &quot;(d&quot; HEX (04) _NUL_ HEX (02) &quot;X&quot; HEX (04) HEX (B0)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(00) HEX(B4) HEX(00) HEX(B4)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(01) HEX(2C) HEX(01) HEX(2C)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(01) HEX(68) HEX(01) HEX(68)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(58) HEX(02) HEX(58)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(D0) HEX(02) HEX(D0)',
#                 '_ESC_ &quot;(d&quot; HEX2S(00,04) HEX(02) HEX(D0) HEX(05) HEX(A0)',
#                 '_ESC_ &quot;(e&quot; HEX(02) HEX(00) HEX(00) &quot;%c&quot;',
#                 '_ESC_ &quot;(e&quot; HEX(02) HEX(00) HEX(01) HEX(00)',
#                 '_ESC_ &quot;(e&quot; HEX(02) HEX(00) &quot;%w&quot;',
#                 '_ESC_ &quot;*F&quot;',
#                 '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (01) _NUL_ _NUL_',
#                 '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (02) _NUL_ _NUL_',
#                 '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (03) _NUL_ _NUL_',
#                 '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ HEX (FE) _NUL_ _NUL_',
#                 '_ESC_ &quot;~F&quot; _NUL_ HEX (05) _NUL_ _NUL_ _NUL_ _NUL_ _NUL_',
#                 '_ESC_ &quot;*F%w&quot;',
#                 '_ESC_ &quot;*g32W&lt;&quot; HEX (05) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02) HEX (01) &quot;,&quot; HEX (01) &quot;,&quot; _NUL_ HEX (02)',
#                 '_ESC_ &quot;*g32W&lt;&quot; HEX (05) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) HEX (02) &quot;X&quot; HEX (02) &quot;X&quot; _NUL_ HEX (02) _ESC_ &quot;*p2N&quot;',
#                 '_ESC_ &quot;*G&quot;',
#                 '_ESC_ &quot;(G&quot; HEX (01) _NUL_ HEX (01)',
#                 '_ESC_ &quot;*G%w&quot;',
#                 '_ESC_ &quot;i%c%c%c%w%w&quot;',
#                 '_ESC_ &quot;i&quot;',
#                 '_ESC_ &quot;*I&quot;',
#                 '_ESC_ &quot;(i&quot; HEX(01) HEX(00) &quot;%c&quot;',
#                 '_ESC_ &quot;*I%w&quot;',
#                 '_ESC_ &quot;J%c&quot;',
#                 '_ESC_ &quot;K%w&quot;',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (11)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (12)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (14)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (15)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (18)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (19)',
#                 '_ESC_ &quot;(l&quot; HEX (01) _NUL_ HEX (1F)',
#                 '_ESC_ &quot;L%w&quot;',
#                 '_ESC_ &quot;m&quot;',
#                 '_ESC_ &quot;n&quot;',
#                 '_ESC_ &quot;*o0M&quot;',
#                 '_ESC_ &quot;*o1M&quot;',
#                 '_ESC_ &quot;*o-1&quot;',
#                 '_ESC_ &quot;@&quot;',
#                 '_ESC_ &quot;* &quot;',
#                 '_ESC_ &quot;*!&quot;',
#                 '_ESC_ &quot;*(&quot;',
#                 '_ESC_ &quot;*@&quot;',
#                 '_ESC_ &quot;@&quot; _ESC_ &quot;@&quot;',
#                 '_ESC_ &quot;*&quot; HEX (02)',
#                 '_ESC_ &quot;*&quot; HEX (02) &quot;%w&quot;',
#                 '_ESC_ &quot;*&quot; HEX (03)',
#                 '_ESC_ &quot;*&quot; HEX (03) &quot;%w&quot;',
#                 '_ESC_ &quot;*&quot; HEX (04)',
#                 '_ESC_ &quot;($&quot; HEX (04) HEX (00) HEX (00) HEX (00) HEX (00) HEX (00)',
#                 '_ESC_ &quot;($&quot; HEX (04) HEX (00) &quot;%c%c%c%c&quot;',
#                 '_ESC_ &quot;*&quot; HEX (04) &quot;%w&quot;',
#                 '_ESC_ &quot;*&quot; HEX (05)',
#                 '_ESC_ &quot;*&quot; HEX (05) &quot;%w&quot;',
#                 '_ESC_ &quot;*&quot; HEX (06)',
#                 '_ESC_ &quot;*&quot; HEX (06) &quot;%w&quot;',
#                 '_ESC_ &quot;*&quot; HEX (07) &quot;%w&quot;',
#                 '_ESC_ &quot;~&quot; HEX(10) HEX (00) HEX (01) HEX (01)',
#                 '_ESC_ &quot;~&quot; HEX(10) HEX (00) HEX (01) HEX (02)',
#                 '_ESC_ &quot;*&quot; _NUL_',
#                 '_ESC_ &quot;@&quot; _NUL_ _NUL_',
#                 '_ESC_ &quot;*&quot; _NUL_ &quot;%w&quot;',
#                 '_ESC_ &quot;*r1Q&quot;',
#                 '_ESC_ &quot;*r2Q&quot;',
#                 '_ESC_ &quot;r%c&quot;',
#                 '_ESC_ &quot;r%d&quot;',
#                 '_ESC_ &quot;(R&quot; HEX (08) HEX(00) HEX(00) &quot;REMOTE1PM&quot; HEX (02) HEX (00) HEX (00) HEX (00) &quot;SN&quot; HEX (03) HEX (00) HEX (00) HEX (00) HEX (01) &quot;SN&quot; HEX (03) HEX (00) HEX (00) HEX (01) HEX (00) &quot;SN&quot; HEX (03) HEX (00) HEX (00) HEX (02) HEX (00) _ESC_ HEX (00) HEX (00) HEX (00)',
#                 '_ESC_ &quot;(S&quot; HEX (08) HEX (00) &quot;%d%d&quot;',
#                 '_ESC_ &quot;(S&quot; HEX(08) HEX(00) &quot;%W&quot; HEX(00) HEX(00) &quot;%W&quot; HEX(00) HEX(00)',
#                 '_ESC_ &quot;*t100R&quot;',
#                 '_ESC_ &quot;*t1200R&quot;',
#                 '_ESC_ &quot;*t150R&quot;',
#                 '_ESC_ &quot;*t200R&quot;',
#                 '_ESC_ &quot;*t300R&quot;',
#                 '_ESC_ &quot;*t400R&quot;',
#                 '_ESC_ &quot;*t600R&quot;',
#                 '_ESC_ &quot;*t75R&quot;',
#                 '_ESC_ &quot;U%c&quot;',
#                 '_ESC_ &quot;(U&quot; HEX (01) _NUL_ &quot;%c&quot;',
#                 '_ESC_ &quot;(U&quot; HEX (05) HEX (00) &quot;%c%c%c%w&quot;',
#                 '_ESC_ &quot;(V&quot; HEX (02) _NUL_ &quot;%w&quot;',
#                 '_ESC_ &quot;(v&quot; HEX (04) HEX (00) &quot;%d&quot;',
#                 '_ESC_ &quot;$%w&quot;',
#                 '_ESC_ &quot;* %w&quot;',
#                 '_ESC_ &quot;*!%w&quot;',
#                 "_ESC_ &quot;*'%w&quot;",
#                 '_ESC_ &quot;*(%w&quot;',
#                 '_ESC_ &quot;*@%w&quot;',
#                 '_ESC_ &quot;z%c&quot;',
#                 '_ESC_ "*r1A"',
#                 '_ESC_ "*rbC"',
#                 '_ESC_ "r%c"',
#                 '_ESC_ "*rC"',
#                 '_ESC_ "*r%nA"',
#                 '_ESC_ "*r%nS"',
#                 '_ESC_ "*r%nT"',
#                 '_ESC_ "*t%dR"',
#                 '_ESC_ "*t%fH"',
#                 '_ESC_ "*t%fV"',
#                 '_ESC_ "*t%nR"',
#                 '_ESC_ "U%c"',
#                 '_ESC_ "*v6W" HEX (00) HEX (01) HEX (01) HEX (08) HEX (08) HEX (08)',
#                 '_ESC_ "*v6W" HEX (00) HEX (03) HEX (00) HEX (08) HEX (08) HEX (08)',
#                 '_ESC_ "$%w"',
#                 '_ESC_ "Z"',
#                 '_FF_',
#                 '_FF_ _ESC_ "@"',
#                 '_FF_ _ESC_ "~" HEX (01) HEX (00) HEX (00)',
#                 '_FF_ _ESC_ HEX (40)',
#                 '_FF_ _ESC_ &quot;@&quot;',
#                 'HEX(00) HEX(00) HEX(00) _ESC_ HEX(01) &quot;@EJL 1284.4&quot; _LF_ &quot;@EJL     &quot; _LF_',
#                 'HEX(00) HEX(00) HEX(00) _ESC_ HEX(01) &quot;@EJL&quot; HEX(20) &quot;1284.4&quot; _LF_ &quot;@EJL&quot; HEX(20) HEX(20) HEX(20) HEX(20) HEX(20) _LF_',
#                 'HEX (00) HEX (0B) HEX (AC) HEX (6B) "%W%W%W%c"',
#                 'HEX (00) HEX (0F) HEX (A6) HEX (FB) "%W%c%W%W%W%W"',
#                 'HEX (01)',
#                 'HEX (02)',
#                 'HEX (03)',
#                 'HEX (04)',
#                 'HEX (05)',
#                 'HEX (06)',
#                 'HEX (07)',
#                 'HEX (08)',
#                 'HEX (09)',
#                 'HEX (10) HEX (00) "%W%c%W%W%W%c%W%W"',
#                 'HEX (70) HEX (04) "%d"',
#                 'HEX (71) HEX (00)',
#                 'HEX (91) HEX (01) "%c"',
#                 'HEX (93) HEX (00)',
#                 'HEX (94) HEX (09) "%c%W%W%W%W"',
#                 'HEX (95) HEX (02) "%c%c"',
#                 'HEX (FE) HEX (92) "%W"',
#                 '_LF_',
#                 '_NUL_',
#                 '_NUL_ _NUL_ _NUL_ "BJLSTART" _LF_',
#                 '_NUL_ _NUL_ _NUL_ _ESC_ "[K" HEX (02) _NUL_ _NUL_ HEX (0F)',
#                 '_NUL_ _NUL_ _NUL_ _ESC_ "[K" HEX (02) _NUL_ _NUL_ HEX (0F) _ESC_ "(a" HEX (01) _NUL_ HEX (01)',
#                 '_NUL_ _NUL_ _NUL_ _ESC_ "[K" HEX2S(00,02) HEX(00) HEX(1f)',
#                 '"@PJL ENTER LANGUAGE=PAGES" _CR_ _LF_',
#                 '"@PJL EOJ" _CR_ _LF_',
#                 '"@PJL JOB NAME=LINUX JOB" _CR_ _LF_',
#                 '"@PJL SET ECONOMODE=OFF" _CR_ _LF_',
#                 '"@PJL SET ECONOMODE=ON" _CR_ _LF_',
#                 '"@PJL SET =OFF" _CR_ _LF_',
#                 '"@PJL SET =ON" _CR_ _LF_',
#                 '"@PJL SET =ONEPORT" _CR_ _LF_',
#                 '"@PJL SET PAGEPROTECT=AUTO" _CR_ _LF_',
#                 '"@PJL SET PAGEPROTECT=OFF" _CR_ _LF_',
#                 '"@PJL SET PAGEPROTECT=ON" _CR_ _LF_',
#                 '"@PJL SET RET=OFF" _CR_ _LF_',
#                 '"@PJL SET RET=ON" _CR_ _LF_',
#                 '"@PJL SET =TWOPORT" _CR_ _LF_',
#                 '&quot;&quot;',
#                 '&quot;!R!PSRC1;EXIT;&quot;',
#                 '&quot;!R!PSRC99;EXIT;&quot;',
#                 '&quot;!R! SPSZ 12; EXIT;&quot;',
#                 '&quot;!R!spsz13;exit;&quot;',
#                 '&quot;!R!spsz14;exit;&quot;',
#                 '&quot;!R!spsz15;exit;&quot;',
#                 '&quot;!R!spsz16;exit;&quot;',
#                 '&quot;!R!spsz18;exit;&quot;',
#                 '"SMOSER=" "%i"',
#                 '"%W%W%W"',
#               ]

    for xmlCommand in commands:
        command     = convertFromXMLString (xmlCommand)
        cmd         = getCommandString (command)
        command2    = encodeCommand (cmd)
        xmlCommand2 = convertToXMLString (command2)
        command2    = convertFromXMLString (xmlCommand2)
        cmd2        = getCommandString (command2)

        print command, '<-', command2
        print cmd, '<-', cmd2
        print xmlCommand, '<-', xmlCommand2

        if command != command2:
            print "Error: String commands are not equal"
            raise SystemExit

        if cmd != cmd2:
            print "Error: Commands are not equal"
            raise SystemExit

        if xmlCommand != xmlCommand2:
            print "Error: XML commands are not equal"
            raise SystemExit
