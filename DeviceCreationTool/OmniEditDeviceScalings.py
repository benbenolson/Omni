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

<deviceScalings
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <deviceScaling>
      <default>100</default>
      <minimum>100</minimum>
      <maximum>100</maximum>
      <command>_NUL_</command>
      <allowedType>Clip</allowedType>
   </deviceScaling>
</deviceScalings>"""

def isValidType (name):
    return name in [ "Clip",
                     "FitToPage",
                     "RotateAndOrFit",
                     "None"
                   ]

class OmniEditDeviceScaling:
    def __init__ (self, scaling):
        if type (scaling) == types.InstanceType:
            self.setDefault (scaling.getDefault ())
            self.setMinimum (scaling.getMinimum ())
            self.setMaximum (scaling.getMaximum ())
            self.setCommand (scaling.getCommand ())
            self.setAllowedType (scaling.getAllowedType ())
            self.setDeviceID (scaling.getDeviceID ())
        elif type (scaling) == types.ListType or type (scaling) == types.TupleType:
            if len (scaling) != 6:
                raise Exception ("Error: OmniEditDeviceScaling: expecting 6 elements, received " + str (len (scaling)) + " of " + str (scaling))
            if not self.setDefault (scaling[0]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set default (" + scaling[0] + ") !")
            if not self.setMinimum (scaling[1]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set minimum (" + scaling[1] + ") !")
            if not self.setMaximum (scaling[2]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set maximum (" + scaling[2] + ") !")
            if not self.setCommand (scaling[3]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set command (" + scaling[3] + ") !")
            if not self.setAllowedType (scaling[4]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set allowedType (" + scaling[4] + ") !")
            if not self.setDeviceID (scaling[5]):
                raise Exception ("Error: OmniEditDeviceScaling: can't set deviceID! (" + scaling[5] + ") ")
        else:
            raise Exception ("Expecting OmniEditDeviceScaling, list, or tuple.  Got " + str (type (scaling)))

    def getDefault (self):
        return self.default

    def setDefault (self, default):
        try:
            self.default = convertToIntegerValue (default)
            return True
        except Exception, e:
            print "OmniEditDeviceScaling::setDefault: Error: (%s) is not an integer!" % (default)
            return False

    def getMinimum (self):
        return self.minimum

    def setMinimum (self, minimum):
        try:
            iMinimum = convertToIntegerValue (minimum)
            if iMinimum > 0:
                self.minimum = iMinimum
                return True
            else:
                print "OmniEditDeviceScaling::setMinimum: Error: (%s) is not a positive integer > 0!" % (minimum)
                return False
        except Exception, e:
            print "OmniEditDeviceScaling::setMinimum: Error: (%s) is not an integer!" % (minimum)
            return False

    def getMaximum (self):
        return self.maximum

    def setMaximum (self, maximum):
        try:
            iMaximum = convertToIntegerValue (maximum)
            if iMaximum > 0:
                self.maximum = iMaximum
                return True
            else:
                print "OmniEditDeviceScaling::setMaximum: Error: (%s) is not a positive integer > 0!" % (maximum)
                return False
        except Exception, e:
            print "OmniEditDeviceScaling::setMaximum: Error: (%s) is not an integer!" % (maximum)
            return False

    def getCommand (self):
        return self.command

    def setCommand (self, command):
        if getCommandString (command) != None:
            self.command = command
            return True
        else:
            print "OmniEditDeviceScaling::setCommand: Error: (%s) is not a valid command!" % (command)
            return False

    def getAllowedType (self):
        return self.allowedType

    def setAllowedType (self, allowedType):
        if isValidType (allowedType):
            self.allowedType = allowedType
            return True
        else:
            print "OmniEditDeviceScaling::setAllowedType: Error: (%s) is not valid type!" % (allowedType)
            return False

    def getDeviceID (self):
        return self.deviceID

    def setDeviceID (self, deviceID):
        if deviceID == "":
            deviceID = None

        self.deviceID = deviceID

        return True

    def setDeviceScaling (self, scaling):
        try:
            if type (scaling) == types.InstanceType:
                self.setDefault (scaling.getDefault ())
                self.setMinimum (scaling.getMinimum ())
                self.setMaximum (scaling.getMaximum ())
                self.setCommand (scaling.getCommand ())
                self.setAllowedType (scaling.getAllowedType ())
                self.setDeviceID (scaling.getDeviceID ())
            else:
                print "OmniEditDeviceScaling::setDeviceScaling: Error: Expecting OmniEditDeviceScaling.  Got ", str (type (scaling))
                return False
        except Exception, e:
            print "OmniEditDeviceScaling::setDeviceScaling: Error: caught " + e
            return False

        return True

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s, %s]" % (self.getDefault (),
                                            self.getMinimum (),
                                            self.getMaximum (),
                                            self.getCommand (),
                                            self.getAllowedType (),
                                            self.getDeviceID ()),
        if fNewLine:
            print

class OmniEditDeviceScalings:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.scalings    = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceScalings::isValid"):
#           print self.__class__.__name__ + ".isValid"

        elmScalings = self.rootElement

        if elmScalings.nodeName != "deviceScalings":
            return "Missing <deviceScalings>, found " + elmScalings.nodeName

        if countChildren (elmScalings) < 1:
            return "At least one <deviceScaling> element is required"

        elmScaling  = firstNode (elmScalings.firstChild)
        elmScalings = nextNode (elmScalings)

        while elmScaling != None:
            if elmScaling.nodeName != "deviceScaling":
                return "Missing <deviceScaling>, found " + elmScaling.nodeName

            elm = firstNode (elmScaling.firstChild)

            if elm.nodeName != "default":
                return "Missing <default>, found " + elm.nodeName

            default = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "minimum":
                return "Missing <minimum>, found " + elm.nodeName

            minimum = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "maximum":
                return "Missing <maximum>, found " + elm.nodeName

            maximum = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getValue (elm) # getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "allowedType":
                return "Missing <allowedType>, found " + elm.nodeName

            allowedType = getValue (elm)
            elm         = nextNode (elm)

            if not isValidType (allowedType):
                return "Invalid <allowedType> " + default

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmScaling = nextNode (elmScaling)

            if elm != None:
                return "Expecting no more tags in <deviceScaling>"

            self.scalings.append (OmniEditDeviceScaling ((default,
                                                          minimum,
                                                          maximum,
                                                          command,
                                                          allowedType,
                                                          deviceID)))

        if elmScalings != None:
            return "Expecting no more tags in <deviceScalings>"

        return None

    def getFileName (self):
        return self.filename

    def getScalings (self):
        return self.scalings

    def printSelf (self):
        print "[",
        for scaling in self.getScalings ():
            scaling.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceScalings xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for scaling in self.getScalings ():
            default     = scaling.getDefault ()
            minimum     = scaling.getMinimum ()
            maximum     = scaling.getMaximum ()
            command     = scaling.getCommand ()
            allowedType = scaling.getAllowedType ()
            deviceID    = scaling.getDeviceID ()

            xmlData += """   <deviceScaling>
      <default>""" + str (default) + """</default>
      <minimum>""" + str (minimum) + """</minimum>
      <maximum>""" + str (maximum) + """</maximum>
      <command>""" + convertToXMLString (command) + """</command>
      <allowedType>""" + str (allowedType) + """</allowedType>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceScaling>
"""

        xmlData += """</deviceScalings>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceScalings::save"):
            print "OmniEditDeviceScalings::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceScalingsDialog (self)
        else:
            return OmniEditDeviceScalingDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceScalingsWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceScalingsWindow')

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
        if shouldOutputDebug ("OmniEditDeviceScalingsWindow::on_window_delete"):
            print "OmniEditDeviceScalingsWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceScalingsWindow::on_window_destroy"):
            print "OmniEditDeviceScalingsWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceScalingsDialog:
    def __init__ (self, scaling):
        self.scaling = scaling

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceScalingsFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceScalingDialog:
    def __init__ (self, scalings, child):
        scaling = OmniEditDeviceScaling (child)

        self.scalings  = scalings
        self.scaling   = scaling
        self.child     = child
        self.fChanged  = False
        self.fModified = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceScalingFrame')

        self.xml                           = xml
        self.window                        = window
        self.EntryDefault                  = xml.get_widget ('EntryScalingDefault')
        self.EntryMinimum                  = xml.get_widget ('EntryScalingMinimum')
        self.EntryMaximum                  = xml.get_widget ('EntryScalingMaximum')
        self.EntryCommand                  = xml.get_widget ('EntryScalingCommand')
        self.EntryAllowedType              = xml.get_widget ('EntryScalingAllowedType')
        self.EntryDeviceID                 = xml.get_widget ('EntryScalingDeviceID')
        self.CheckButtonDefault            = xml.get_widget ('CheckButtonScalingDefault')
        self.EntryScalingDefaultPercentage = xml.get_widget ('EntryScalingDefaultPercentage')

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
#       if shouldOutputDebug ("OmniEditDeviceScalingsDialog::__init__"):
#           print "OmniEditDeviceScalingsDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryDefault.set_text (str (scaling.getDefault ()))
        self.EntryMinimum.set_text (str (scaling.getMinimum ()))
        self.EntryMaximum.set_text (str (scaling.getMaximum ()))
        self.EntryCommand.set_text (str (scaling.getCommand ()))
        self.EntryAllowedType.set_text (str (scaling.getAllowedType ()))
        name = scaling.getDeviceID ()
        if name == None:
            name = ""
        self.EntryDeviceID.set_text (str (name))
        fDefault                 = False
        device                   = scalings.getDevice ()
        deviceInfo               = device.getDeviceInformation ()
        defaultScalingPercentage = deviceInfo.getDefaultJobProperty ("ScalingPercentage")
        defaultScalingType       = deviceInfo.getDefaultJobProperty ("ScalingType")
        if     defaultScalingPercentage != None \
           and defaultScalingType != None \
           and defaultScalingType == scaling.getAllowedType ():
            fDefault = True
        if fDefault:
            self.EntryScalingDefaultPercentage.set_text (defaultScalingPercentage)
        else:
            self.EntryScalingDefaultPercentage.set_text (str (scaling.getMinimum ()))
        self.CheckButtonDefault.set_active (fDefault)

        self.fChanged = False

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceScalingsDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceScalingsDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceScalingsDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceScalingsDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceScalingsDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceScalingsDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceScalingsDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceScalingsDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceScalingsDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceScalingsDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceScalingsDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceScalingsDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceScalingsDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceScalingsDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceScalingsDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceScalingsDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceScalingsDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceScalingsDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceScalingsDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceScalingsDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceScalingsDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceScalingsDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceScalingsDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceScalingsDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceScalingsDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceScalingsDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceScalingsDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceScalingsDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceScalingsDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceScalingsDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceScalingsDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceScalingsDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceScalingsDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceScalingsDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceScalingsDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceScalingsDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceScalingsDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceScalingsDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceScalingsDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceScalingsDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceScalingsDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceScalingsDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceScalingsDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceScalingsDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceScalingsDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceScalingsDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceScalingsDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceScalingsDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceScalingsDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceScalingsDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceScalingsDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceScalingsDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceScalingsDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceScalingsDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceScalingsDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceScalingsDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceScalingsDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceScalingsDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceScalingsDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceScalingsDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceScalingsDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceScalingsDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceScalingsDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceScalingsDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceScalingsDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceScalingsDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceScalingsDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceScalingsDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceScalingsDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceScalingsDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceScalingsDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceScalingsDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceScalingsDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceScalingsDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceScalingsDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceScalingsDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceScalingsDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceScalingsDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceScalingsDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceScalingsDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceScalingsDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceScalingsDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceScalingsDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceScalingsDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceScalingsDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceScalingsDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceScalingsDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceScalingsDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceScalingsDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceScalingsDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceScalingsDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceScalingsDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceScalingsDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceScalingsDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceScalingsDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceScalingsDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceScalingsDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceScalingsDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonScalingModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked"):
            print "OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.scaling.setDefault (self.EntryDefault.get_text ()):
                pszError = "Invlaid default"
            if not self.scaling.setMinimum (self.EntryMinimum.get_text ()):
                pszError = "Invlaid minimum"
            if not self.scaling.setMaximum (self.EntryMaximum.get_text ()):
                pszError = "Invlaid maximum"
            if not self.scaling.setCommand (self.EntryCommand.get_text ()):
                pszError = "Invlaid command"
            if not self.scaling.setAllowedType (self.EntryAllowedType.get_text ()):
                pszError = "Invlaid allowed type"
            if not self.scaling.setDeviceID (self.EntryDeviceID.get_text ()):
                pszError = "Invlaid device ID"

            if pszError == None:
                matchingScaling = self.findScaling (self.scaling)

                if     matchingScaling != None \
                   and matchingScaling != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             "%s %d already exists!" % (self.scaling.getAllowedType (), self.scaling.getDefault ()))
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceScaling (self.scaling)
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
            device                 = self.scalings.getDevice ()
            dialog                 = device.getDeviceDialog ()
            deviceInfo             = device.getDeviceInformation ()
            scalingPercentage      = deviceInfo.getDefaultJobProperty ("ScalingPercentage")
            scalingType            = deviceInfo.getDefaultJobProperty ("ScalingType")
            entryScalingPercentage = self.EntryScalingDefaultPercentage.get_text ()
            newScalingType         = self.scaling.getAllowedType ()
            iMinimumPercentage     = self.scaling.getMinimum ()
            iMaximumPercentage     = self.scaling.getMaximum ()
            try:
                iDefaultPercentage = convertToIntegerValue (entryScalingPercentage)
            except Exception, e:
                print "OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked: Error: Could not convert %s to an integer!" % (entryScalingPercentage)
                iDefaultPercentage = iMinimumPercentage
            if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked"):
                print "OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked: %s <= %s <= %s" % (iMinimumPercentage, iDefaultPercentage, iMaximumPercentage)
            if iDefaultPercentage < iMinimumPercentage:
                iDefaultPercentage = iMinimumPercentage
            if iDefaultPercentage > iMaximumPercentage:
                iDefaultPercentage = iMaximumPercentage
            newDefaultPercentage = str (iDefaultPercentage)
            if    newDefaultPercentage != scalingPercentage \
               or newScalingType != scalingType :
                if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked"):
                    print "OmniEditDeviceScalingsDialog::on_ButtonScalingModify_clicked: Setting %s %s to default" % (newScalingType, newDefaultPercentage)
                deviceInfo.setDefaultJobProperty ("ScalingPercentage", newDefaultPercentage)
                deviceInfo.setDefaultJobProperty ("ScalingType", newScalingType)
                dialog.setModified (True)

    def on_ButtonScalingAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingAdd_clicked"):
            print "OmniEditDeviceScalingsDialog::on_ButtonScalingAdd_clicked:", widget

    def on_ButtonScalingDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked"):
            print "OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked:", widget

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
            (rc, iScalingsLeft) = self.deleteScaling (self.child)
            if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked"):
                print "OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked: deleteScaling rc = %s, iScalingsLeft = %d" % (rc, iScalingsLeft)

            device    = self.scalings.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iScalingsLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked"):
                    print "OmniEditDeviceScalingsDialog::on_ButtonScalingDelete_clicked: Last scaling deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceScalings":
                        treestore.remove (row.iter)
                        device.setDeviceScalings (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_EntryScalingDefault_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingDefault_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingDefault_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryScalingMinimum_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingMinimum_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingMinimum_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryScalingMaximum_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingMaximum_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingMaximum_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryScalingCommand_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingCommand_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingCommand_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryScalingAllowedType_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingAllowedType_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingAllowedType_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryScalingDeviceID_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingDeviceID_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingDeviceID_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_CheckButtonScalingDefault_toggled (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_CheckButtonScalingDefault_toggled"):
            print "OmniEditDeviceScalingsDialog::on_CheckButtonScalingDefault_toggled:", widget

    def on_EntryScalingDefaultPercentage_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceScalingsDialog::on_EntryScalingDefaultPercentage_changed"):
            print "OmniEditDeviceScalingsDialog::on_EntryScalingDefaultPercentage_changed:", widget

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.scalings.save ()

        self.fModified = False

    def findScaling (self, matchScaling):
        for scaling in self.scalings.getScalings ():
            if     matchScaling.getAllowedType () == scaling.getAllowedType () \
               and matchScaling.getDefault ()     == scaling.getDefault ():
                return scaling

        return None

    def deleteScaling (self, matchScaling):
        foundScaling = None
        foundRow     = None
        scalings     = self.scalings.getScalings ()
        for scaling in scalings:
#           if shouldOutputDebug ("OmniEditDeviceScalingsDialog::deleteScaling"):
#               print "OmniEditDeviceScalingsDialog::deleteScaling: matching1 (%s %s) vs (%s %s)" % (matchScaling.getAllowedType (), matchScaling.getDefault (), scaling.getAllowedType (), scaling.getDefault ())
            if     matchScaling.getAllowedType () == scaling.getAllowedType () \
               and matchScaling.getDefault ()     == scaling.getDefault ():
                foundScaling = scaling

        device    = self.scalings.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceScalingsDialog::deleteScaling"):
#                       print "OmniEditDeviceScalingsDialog::deleteScaling: matching2 %s vs %s" % (childRow[1], matchScaling)
                    if childRow[1] == matchScaling:
                        foundRow = childRow

        if     foundScaling != None \
           and foundRow  != None:
            if shouldOutputDebug ("OmniEditDeviceScalingsDialog::deleteScaling"):
                print "OmniEditDeviceScalingsDialog::deleteScaling: Removing (%s %s) from scalings" % (foundScaling.getAllowedType (), foundScaling.getDefault ())
            scalings.remove (foundScaling)

            if shouldOutputDebug ("OmniEditDeviceScalingsDialog::deleteScaling"):
                print "OmniEditDeviceScalingsDialog::deleteScaling: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.scalings.setModified (True)

            return (True, len (scalings))
        else:
            if foundScaling == None:
                print "OmniEditDeviceScalingsDialog::deleteScaling: Error: Did not find (%s %s) in scalings!" % (matchScaling.getAllowedType (), matchScaling.getDefault ())
            if foundRow == None:
                print "OmniEditDeviceScalingsDialog::deleteScaling: Error: Did not find %s in listbox!" % (str (matchScaling))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Scalings.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        scalings = OmniEditDeviceScalings (fname, doc.documentElement, None)

        print "OmniEditDeviceScalings.py::__main__: scalings = ",
        lastScaling = scalings.getScalings ()[-1]
        for scaling in scalings.getScalings ():
            scaling.printSelf (False),
            if scaling != lastScaling:
                print ",",
        print

        dialog = OmniEditDeviceScalingDialog (scalings, scalings.getScalings ()[0])
        window = OmniEditDeviceScalingsWindow (dialog)

        gtk.main ()

        print scalings.toXML ()

    except Exception, e:
        print "OmniEditDeviceScalings.py::__main__: Error: Caught", e
