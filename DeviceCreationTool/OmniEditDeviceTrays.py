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

<deviceTrays
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <deviceTray>
      <name>AutoSelect</name>
      <trayType>TRAY_TYPE_AUTO</trayType>
      <command>_NUL_</command>
   </deviceTray>
</deviceTrays>"""

def isValidName (name):
    return name in [ "AnyLargeFormat",
                     "AnySmallFormat",
                     "AutoSelect",
                     "Bottom",
                     "BypassTray",
                     "BypassTray-1",
                     "BypassTray-2",
                     "BypassTray-3",
                     "BypassTray-4",
                     "BypassTray-5",
                     "BypassTray-6",
                     "BypassTray-7",
                     "BypassTray-8",
                     "BypassTray-9",
                     "Continuous",
                     "Disc",
                     "Disc-1",
                     "Disc-2",
                     "Disc-3",
                     "Disc-4",
                     "Disc-5",
                     "Disc-6",
                     "Disc-7",
                     "Disc-8",
                     "Disc-9",
                     "Envelope",
                     "Envelope-1",
                     "Envelope-2",
                     "Envelope-3",
                     "Envelope-4",
                     "Envelope-5",
                     "Envelope-6",
                     "Envelope-7",
                     "Envelope-8",
                     "Envelope-9",
                     "Front",
                     "InsertTray",
                     "InsertTray-1",
                     "InsertTray-2",
                     "InsertTray-3",
                     "InsertTray-4",
                     "InsertTray-5",
                     "InsertTray-6",
                     "InsertTray-7",
                     "InsertTray-8",
                     "InsertTray-9",
                     "LargeCapacity",
                     "LargeCapacity-1",
                     "LargeCapacity-2",
                     "LargeCapacity-3",
                     "LargeCapacity-4",
                     "LargeCapacity-5",
                     "LargeCapacity-6",
                     "LargeCapacity-7",
                     "LargeCapacity-8",
                     "LargeCapacity-9",
                     "Left",
                     "Middle",
                     "PanelSelect",
                     "Rear",
                     "Right",
                     "Roll",
                     "Roll-1",
                     "Roll-2",
                     "Roll-3",
                     "Roll-4",
                     "Roll-5",
                     "Roll-6",
                     "Roll-7",
                     "Roll-8",
                     "Roll-9",
                     "Side",
                     "Top",
                     "Tray",
                     "Tray-1",
                     "Tray-2",
                     "Tray-3",
                     "Tray-4",
                     "Tray-5",
                     "Tray-6",
                     "Tray-7",
                     "Tray-8",
                     "Tray-9"
                   ]

def isValidOmniName (omniName):
    if omniName == None:
        return True

    return omniName in [ "TRAY_UNLISTED",
                         "TRAY_NONE",
                         "TRAY_AUTO",
                         "TRAY_USE_PRINTER_SETTING",
                         "TRAY_AUTO_FEEDER",
                         "TRAY_AUTOSWITCH",
                         "TRAY_TRAY",
                         "TRAY_UPPER_TRAY",
                         "TRAY_LOWER_TRAY",
                         "TRAY_MULTI_TRAY",
                         "TRAY_CASSETTE",
                         "TRAY_UPPER_CASSETTE",
                         "TRAY_LOWER_CASSETTE",
                         "TRAY_MULTI_CASSETTE",
                         "TRAY_OPTION_CASSETTE",
                         "TRAY_MANUAL_FEEDER",
                         "TRAY_MANUAL_ENVELOPE",
                         "TRAY_PAPER_FEEDER",
                         "TRAY_ENVELOPE_FEEDER",
                         "TRAY_CSF",
                         "TRAY_FRONT_CONTINUOUS",
                         "TRAY_REAR_CONTINUOUS",
                         "TRAY_SINGLE_SHEET",
                         "TRAY_SHEET_FEEDER",
                         "TRAY_BIN_1",
                         "TRAY_BIN_2",
                         "TRAY_PORTABLE_SHEET",
                         "TRAY_TRAY_1",
                         "TRAY_TRAY_2",
                         "TRAY_TRAY_3",
                         "TRAY_TRAY_4",
                         "TRAY_TRAY_5",
                         "TRAY_TRAY_6",
                         "TRAY_CASSETTE1",
                         "TRAY_CASSETTE2",
                         "TRAY_CASSETTE3",
                         "TRAY_CASSETTE4",
                         "TRAY_CASSETTE5",
                         "TRAY_FRONT_TRAY",
                         "TRAY_OPTION_MULTI_SF",
                         "TRAY_MULTI_FEEDER",
                         "TRAY_PAPER_DECK",
                         "TRAY_TRACTOR_UNIT",
                         "TRAY_ROLL_1",
                         "TRAY_ROLL_2",
                         "TRAY_ROLL_3",
                         "TRAY_ROLL_4",
                         "TRAY_ROLL_5",
                         "TRAY_AUXILIARY_TRAY",
                         "TRAY_LARGE_CAPACITY_TRAY",
                         "TRAY_HIGH_CAPACITY_FEEDER",
                         "TRAY_ZERO_MARGINS"
                       ]

def isValidTrayType (trayType):
    return trayType in [ "TRAY_TYPE_AUTO",
                         "TRAY_TYPE_MANUAL",
                         "TRAY_TYPE_ZERO_MARGINS"
                       ]

class OmniEditDeviceTray:
    def __init__ (self, tray):
        if type (tray) == types.InstanceType:
            self.setName (tray.getName ())
            self.setOmniName (tray.getOmniName ())
            self.setTrayType (tray.getTrayType ())
            self.setCommand (tray.getCommand ())
            self.setDeviceID (tray.getDeviceID ())
        elif type (tray) == types.ListType or type (tray) == types.TupleType:
            if len (tray) != 5:
                raise Exception ("Error: OmniEditDeviceTray: expecting 5 elements, received " + str (len (tray)) + " of " + str (tray))
            if not self.setName (tray[0]):
                raise Exception ("Error: OmniEditDeviceTray: can't set name (" + tray[0] + ") !")
            if not self.setOmniName (tray[1]):
                raise Exception ("Error: OmniEditDeviceTray: can't set omniName (" + tray[1] + ") !")
            if not self.setTrayType (tray[2]):
                raise Exception ("Error: OmniEditDeviceTray: can't set trayType (" + tray[2] + ") !")
            if not self.setCommand (tray[3]):
                raise Exception ("Error: OmniEditDeviceTray: can't set command! (" + tray[3] + ") ")
            if not self.setDeviceID (tray[4]):
                raise Exception ("Error: OmniEditDeviceTray: can't set deviceID (" + tray[4] + ") !")
        else:
            raise Exception ("Expecting OmniEditDeviceTray, list, or tuple.  Got " + str (type (tray)))

    def getName (self):
        return self.name

    def setName (self, name):
        if isValidName (name):
            self.name = name
            return True
        else:
            print "OmniEditDeviceTray::setName: Error: (%s) is not a valid name!" % (name)
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
            print "OmniEditDeviceTray::setOmniName: Error: (%s) is not a valid omni name!" % (omniName)
            return False

    def getTrayType (self):
        return self.trayType

    def setTrayType (self, trayType):
        if isValidTrayType (trayType):
            self.trayType = trayType
            return True
        else:
            print "OmniEditDeviceTray::setTrayType: Error: (%s) is not a valid tray type!" % (trayType)
            return False

    def getCommand (self):
        return self.command

    def setCommand (self, command):
        if getCommandString (command) != None:
            self.command = command
            return True
        else:
            print "OmniEditDeviceTray::setCommand: Error: (%s) is not a valid command!" % (command)
            return False

    def getDeviceID (self):
        return self.deviceID

    def setDeviceID (self, deviceID):
        if deviceID == "":
            deviceID = None

        self.deviceID = deviceID

        return True

    def setDeviceTray (self, tray):
        try:
            if type (tray) == types.InstanceType:
                self.setName (tray.getName ())
                self.setOmniName (tray.getOmniName ())
                self.setTrayType (tray.getTrayType ())
                self.setCommand (tray.getCommand ())
                self.setDeviceID (tray.getDeviceID ())
            else:
                print "OmniEditDeviceTray::setDeviceTray: Error: Expecting OmniEditDeviceTray.  Got ", str (type (tray))
                return False
        except Exception, e:
            print "OmniEditDeviceTray::setDeviceTray: Error: caught " + e
            return False

        return True

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s]" % (self.getName (),
                                        self.getOmniName (),
                                        self.getTrayType (),
                                        self.getCommand (),
                                        self.getDeviceID ()),
        if fNewLine:
            print

class OmniEditDeviceTrays:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.trays       = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceTrays::isValid"):
#           print self.__class__.__name__ + ".isValid"

        elmTrays = self.rootElement

        if elmTrays.nodeName != "deviceTrays":
            return "Missing <deviceTrays>, found " + elmTrays.nodeName

        if countChildren (elmTrays) < 1:
            return "At least one <deviceTray> element is required"

        elmTray  = firstNode (elmTrays.firstChild)
        elmTrays = nextNode (elmTrays)

        while elmTray != None:
            if elmTray.nodeName != "deviceTray":
                return "Missing <deviceTray>, found " + elmTray.nodeName

            elm = firstNode (elmTray.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if not isValidName (name):
                return "Invalid <name> " + name

            omniName = None
            if elm.nodeName == "omniName":
                omniName = getValue (elm)
                elm      = nextNode (elm)

            if elm.nodeName != "trayType":
                return "Missing <trayType>, found " + elm.nodeName

            trayType = getValue (elm)
            elm      = nextNode (elm)

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getValue (elm) # getCommand (elm)
            elm     = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmTray = nextNode (elmTray)

            if elm != None:
                return "Expecting no more tags in <deviceTray>"

            self.trays.append (OmniEditDeviceTray ((name,
                                                    omniName,
                                                    trayType,
                                                    command,
                                                    deviceID)))

        if elmTrays != None:
            return "Expecting no more tags in <deviceTrays>"

        return None

    def getFileName (self):
        return self.filename

    def getTrays (self):
        return self.trays

    def printSelf (self):
        print "[",
        for tray in self.getTrays ():
            tray.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceTrays xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for tray in self.getTrays ():
            name     = tray.getName ()
            omniName = tray.getOmniName ()
            trayType = tray.getTrayType ()
            command  = tray.getCommand ()
            deviceID = tray.getDeviceID ()

            xmlData += """   <deviceTray>
      <name>""" + str (name) + """</name>
"""

            if omniName != None:
                xmlData += "      <omniName>" + omniName + """</omniName>
"""
            xmlData += "      <trayType>""" + str (trayType) + """</trayType>
      <command>""" + convertToXMLString (command) + """</command>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceTray>
"""

        xmlData += """</deviceTrays>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceTrays::save"):
            print "OmniEditDeviceTrays::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceTraysDialog (self)
        else:
            return OmniEditDeviceTrayDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceTraysWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceTraysWindow')

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
        if shouldOutputDebug ("OmniEditDeviceTraysWindow::on_window_delete"):
            print "OmniEditDeviceTraysWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceTraysWindow::on_window_destroy"):
            print "OmniEditDeviceTraysWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceTraysDialog:
    def __init__ (self, tray):
        self.tray = tray

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceTraysFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceTrayDialog:
    def __init__ (self, trays, child):
        tray = OmniEditDeviceTray (child)

        self.trays     = trays
        self.tray      = tray
        self.child     = child
        self.fChanged  = False
        self.fModified = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceTrayFrame')

        self.xml                = xml
        self.window             = window
        self.EntryName          = xml.get_widget ('EntryTrayName')
        self.EntryOmniName      = xml.get_widget ('EntryTrayOmniName')
        self.EntryTrayType      = xml.get_widget ('EntryTrayTrayType')
        self.EntryCommand       = xml.get_widget ('EntryTrayCommand')
        self.EntryDeviceID      = xml.get_widget ('EntryTrayDeviceID')
        self.CheckButtonDefault = xml.get_widget ('CheckButtonTrayDefault')

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
                "on_ButtonStringCCAdd_clicked":                            self.on_ButtonStringCCAdd_clicked,
                "on_ButtonStringCCModify_clicked":                         self.on_ButtonStringCCModify_clicked,
                "on_ButtonStringCCDelete_clicked":                         self.on_ButtonStringCCDelete_clicked,
                "on_ButtonStringKeyAdd_clicked":                           self.on_ButtonStringKeyAdd_clicked,
                "on_ButtonStringKeyDelete_clicked":                        self.on_ButtonStringKeyDelete_clicked,
                "on_ButtonStringKeyModify_clicked":                        self.on_ButtonStringKeyModify_clicked,
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
#       if shouldOutputDebug ("OmniEditDeviceTraysDialog::__init__"):
#           print "OmniEditDeviceTraysDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryName.set_text (str (tray.getName ()))
        name = tray.getOmniName ()
        if name == None:
            name = ""
        self.EntryOmniName.set_text (str (name))
        self.EntryTrayType.set_text (str (tray.getTrayType ()))
        self.EntryCommand.set_text (str (tray.getCommand ()))
        name = tray.getDeviceID ()
        if name == None:
            name = ""
        self.EntryDeviceID.set_text (str (name))
        fDefault           = False
        device             = trays.getDevice ()
        deviceInfo         = device.getDeviceInformation ()
        defaultTray = deviceInfo.getDefaultJobProperty ("InputTray")
        if defaultTray != None and defaultTray == tray.getName ():
            fDefault = True
        self.CheckButtonDefault.set_active (fDefault)

        self.fChanged = False

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceTraysDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceTraysDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceTraysDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceTraysDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceTraysDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceTraysDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceTraysDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceTraysDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceTraysDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceTraysDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceTraysDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceTraysDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceTraysDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceTraysDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceTraysDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceTraysDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceTraysDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceTraysDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceTraysDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceTraysDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceTraysDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceTraysDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceTraysDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceTraysDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceTraysDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceTraysDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceTraysDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceTraysDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceTraysDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceTraysDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceTraysDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceTraysDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceTraysDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceTraysDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceTraysDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceTraysDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceTraysDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceTraysDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceTraysDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceTraysDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceTraysDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceTraysDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceTraysDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceTraysDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceTraysDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceTraysDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceTraysDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceTraysDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceTraysDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceTraysDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceTraysDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceTraysDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceTraysDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceTraysDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceTraysDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceTraysDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceTraysDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceTraysDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceTraysDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceTraysDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceTraysDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceTraysDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceTraysDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceTraysDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceTraysDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceTraysDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceTraysDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceTraysDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceTraysDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceTraysDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceTraysDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceTraysDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceTraysDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceTraysDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceTraysDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceTraysDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceTraysDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceTraysDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceTraysDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceTraysDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceTraysDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceTraysDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceTraysDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceTraysDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceTraysDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceTraysDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceTraysDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceTraysDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceTraysDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceTraysDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceTraysDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceTraysDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceTraysDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceTraysDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceTraysDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceTraysDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceTraysDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceTraysDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceTraysDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonTrayModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayModify_clicked"):
            print "OmniEditDeviceTraysDialog::on_ButtonTrayModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.tray.setName (self.EntryName.get_text ()):
                pszError = "Invlaid name"
            if not self.tray.setOmniName (self.EntryOmniName.get_text ()):
                pszError = "Invlaid omni name"
            if not self.tray.setTrayType (self.EntryTrayType.get_text ()):
                pszError = "Invlaid tray type"
            if not self.tray.setCommand (self.EntryCommand.get_text ()):
                pszError = "Invlaid command"
            if not self.tray.setDeviceID (self.EntryDeviceID.get_text ()):
                pszError = "Invlaid device ID"

            if pszError == None:
                matchingTray = self.findTray (self.tray)

                if     matchingTray != None \
                   and matchingTray != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             self.tray.getName () + " already exists!")
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceTray (self.tray)
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

        if self.CheckButtonDefault.get_active ():
            name = self.tray.getName ()
            if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayModify_clicked"):
                print "OmniEditDeviceTraysDialog::on_ButtonTrayModify_clicked: Setting %s to default" % (name)
            device     = self.trays.getDevice ()
            dialog     = device.getDeviceDialog ()
            deviceInfo = device.getDeviceInformation ()
            deviceInfo.setDefaultJobProperty ("InputTray", name)
            dialog.setModified (True)

    def on_ButtonTrayAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayAdd_clicked"):
            print "OmniEditDeviceTraysDialog::on_ButtonTrayAdd_clicked:", widget

    def on_ButtonTrayDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked"):
            print "OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked:", widget

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
            (rc, iTraysLeft) = self.deleteTray (self.child)
            if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked"):
                print "OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked: deleteTray rc = %s, iTraysLeft = %d" % (rc, iTraysLeft)

            device    = self.trays.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iTraysLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked"):
                    print "OmniEditDeviceTraysDialog::on_ButtonTrayDelete_clicked: Last tray deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceTrays":
                        treestore.remove (row.iter)
                        device.setDeviceTrays (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_EntryTrayName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_EntryTrayName_changed"):
            print "OmniEditDeviceTraysDialog::on_EntryTrayName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryTrayOmniName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_EntryTrayOmniName_changed"):
            print "OmniEditDeviceTraysDialog::on_EntryTrayOmniName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryTrayTrayType_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_EntryTrayTrayType_changed"):
            print "OmniEditDeviceTraysDialog::on_EntryTrayTrayType_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryTrayCommand_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_EntryTrayCommand_changed"):
            print "OmniEditDeviceTraysDialog::on_EntryTrayCommand_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryTrayDeviceID_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_EntryTrayDeviceID_changed"):
            print "OmniEditDeviceTraysDialog::on_EntryTrayDeviceID_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_CheckButtonTrayDefault_toggled (self, widget):
        if shouldOutputDebug ("OmniEditDeviceTraysDialog::on_CheckButtonTrayDefault_toggled"):
            print "OmniEditDeviceTraysDialog::on_CheckButtonTrayDefault_toggled:", widget

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.trays.save ()

        self.fModified = False

    def findTray (self, matchTray):
        for tray in self.trays.getTrays ():
            if matchTray.getName () == tray.getName ():
                return tray

        return None

    def deleteTray (self, matchTray):
        foundTray = None
        foundRow  = None
        trays     = self.trays.getTrays ()
        for tray in trays:
#           if shouldOutputDebug ("OmniEditDeviceTraysDialog::deleteTray"):
#               print "OmniEditDeviceTraysDialog::deleteTray: matching1 %s vs %s" % (matchTray.getName (), tray.getName ())
            if matchTray.getName () == tray.getName ():
                foundTray = tray

        device    = self.trays.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceTraysDialog::deleteTray"):
#                       print "OmniEditDeviceTraysDialog::deleteTray: matching2 %s vs %s" % (childRow[1], matchTray)
                    if childRow[1] == matchTray:
                        foundRow = childRow

        if     foundTray != None \
           and foundRow  != None:
            if shouldOutputDebug ("OmniEditDeviceTraysDialog::deleteTray"):
                print "OmniEditDeviceTraysDialog::deleteTray: Removing %s from trays" % (foundTray.getName ())
            trays.remove (foundTray)

            if shouldOutputDebug ("OmniEditDeviceTraysDialog::deleteTray"):
                print "OmniEditDeviceTraysDialog::deleteTray: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.trays.setModified (True)

            return (True, len (trays))
        else:
            if foundTray == None:
                print "OmniEditDeviceTraysDialog::deleteTray: Error: Did not find %s in trays!" % (matchTray.getName ())
            if foundRow == None:
                print "OmniEditDeviceTraysDialog::deleteTray: Error: Did not find %s in listbox!" % (str (matchTray))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Stylus 200 Trays.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        trays = OmniEditDeviceTrays (fname, doc.documentElement, None)

        print "OmniEditDeviceTrays.py::__main__: trays = ",
        lastTray = trays.getTrays ()[-1]
        for tray in trays.getTrays ():
            tray.printSelf (False),
            if tray != lastTray:
                print ",",
        print

        dialog = OmniEditDeviceTrayDialog (trays, trays.getTrays ()[0])
        window = OmniEditDeviceTraysWindow (dialog)

        gtk.main ()

        print trays.toXML ()

    except Exception, e:
        print "OmniEditDeviceTrays.py::__main__: Error: Caught", e
