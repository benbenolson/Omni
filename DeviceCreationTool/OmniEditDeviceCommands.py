#!/usr/bin/python
#
#    IBM Omni driver
#    Commandright (c) International Business Machines Corp., 2000-2004
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
#    You should have received a command of the GNU Lesser General Public License
#    along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

from OmniEditUtils import *

import gtk
import gtk.glade
import gobject
import types

def getDefaultXML (jobProperties):
    return XMLHeader () + """

<deviceCommands
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <command name="cmdNone">_NUL_</command>
</deviceCommands>"""

class OmniEditDeviceCommand:
    def __init__ (self, command):
        if type (command) == types.InstanceType:
            self.setName (command.getName ())
            self.setCommand (command.getCommand ())
        elif type (command) == types.ListType or type (command) == types.TupleType:
            if len (command) != 2:
                raise Exception ("Error: OmniEditDeviceCommand: expecting 2 elements, received " + str (len (command)) + " of " + str (command))
            if not self.setName (command[0]):
                raise Exception ("Error: OmniEditDeviceCommand: can't set name (" + command[0] + ") !")
            if not self.setCommand (command[1]):
                raise Exception ("Error: OmniEditDeviceCommand: can't set command (" + command[1] + ") !")
        else:
            raise Exception ("Expecting OmniEditDeviceCommand, list, or tuple.  Got " + str (type (command)))

    def getName (self):
        return self.name

    def setName (self, name):
        self.name = name
        return True

    def getCommand (self):
        return self.command

    def setCommand (self, command):
        if getCommandString (command) != None:
            self.command = command
            return True
        else:
            print "OmniEditDeviceCommand::setCommand: Error: (%s) is not a valid command!" % (command)
            return False

    def setDeviceCommand (self, command):
        try:
            if type (command) == types.InstanceType:
                self.setName (command.getName ())
                self.setCommand (command.getCommand ())
            else:
                print "OmniEditDeviceCommand::setDeviceCommand: Error: Expecting OmniEditDeviceCommand.  Got ", str (type (command))
                return False
        except Exception, e:
            print "OmniEditDeviceCommand::setDeviceCommand: Error: caught " + e
            return False

        return True

    def printSelf (self, fNewLine = True):
        print "[%s, %s]" % (self.getName (),
                            self.getCommand ()),
        if fNewLine:
            print

class OmniEditDeviceCommands:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.commands    = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceCommands::isValid"):
#           print self.__class__.__name__ + ".isValid"

        elmCommands = self.rootElement

        if elmCommands.nodeName != "deviceCommands":
            return "Missing <deviceCommands>, found " + elmCommands.nodeName

        if countChildren (elmCommands) < 1:
            return "At least one <command> element is required"

        elmCommand  = firstNode (elmCommands.firstChild)
        elmCommands = nextNode (elmCommands)

        while elmCommand != None:
            if elmCommand.nodeName != "command":
                return "Missing <command>, found " + elmCommand.nodeName

            attr = elmCommand.attributes.getNamedItem ("name")
            if attr == None:
                return "Missing name= attribute in <command>"

            name       = attr.nodeValue
            command    = getValue (elmCommand) # getCommand (elmCommand)
            elmCommand = nextNode (elmCommand)

            self.commands.append (OmniEditDeviceCommand ((name, command)))

        if elmCommands != None:
            return "Expecting no more tags in <deviceCommands>"

        return None

    def getFileName (self):
        return self.filename

    def getCommands (self):
        return self.commands

    def printSelf (self):
        print "[",
        for command in self.getCommands ():
            command.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """
<deviceCommands
   xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
   xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for command in self.getCommands ():
            name    = command.getName ()
            command = command.getCommand ()

            xmlData += '   <command name="' + name + '">' + convertToXMLString (command) + """</command>
"""
        xmlData += """</deviceCommands>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceCommands::save"):
            print "OmniEditDeviceCommands::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceCommandsDialog (self)
        else:
            return OmniEditDeviceCommandDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceCommandsWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceCommandsWindow')

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
        if shouldOutputDebug ("OmniEditDeviceCommandsWindow::on_window_delete"):
            print "OmniEditDeviceCommandsWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceCommandsWindow::on_window_destroy"):
            print "OmniEditDeviceCommandsWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceCommandsDialog:
    def __init__ (self, command):
        self.command = command

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceCommandsFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceCommandDialog:
    def __init__ (self, commands, child):
        command = OmniEditDeviceCommand (child)

        self.commands  = commands
        self.command   = command
        self.child     = child
        self.fChanged  = False
        self.fModified = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceCommandFrame')

        self.xml          = xml
        self.window       = window
        self.EntryName    = xml.get_widget ('EntryCommandName')
        self.EntryCommand = xml.get_widget ('EntryCommandCommand')

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
#       if shouldOutputDebug ("OmniEditDeviceCommandsDialog::__init__"):
#           print "OmniEditDeviceCommandsDialog::__init__ dic =", dic

#       window.connect ('destroy', self.on_destroy)
        xml.signal_autoconnect (dic)

        self.EntryName.set_text (str (command.getName ()))
        self.EntryCommand.set_text (str (command.getCommand ()))

        self.fChanged = False

        window.show ()

    def getWindow (self):
        return self.window

    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceCommandsDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceCommandsDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceCommandsDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceCommandsDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceCommandsDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceCommandsDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceCommandsDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceCommandsDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceCommandsDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceCommandsDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceCommandsDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceCommandsDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceCommandsDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceCommandsDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceCommandsDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceCommandsDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceCommandsDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceCommandsDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceCommandsDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceCommandsDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceCommandsDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceCommandsDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceCommandsDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceCommandsDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceCommandsDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceCommandsDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceCommandsDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceCommandsDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceCommandsDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceCommandsDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceCommandsDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceCommandsDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceCommandsDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceCommandsDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceCommandsDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceCommandsDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceCommandsDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceCommandsDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceCommandsDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceCommandsDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceCommandsDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceCommandsDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceCommandsDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceCommandsDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceCommandsDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceCommandsDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceCommandsDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceCommandsDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceCommandsDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceCommandsDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceCommandsDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceCommandsDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceCommandsDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceCommandsDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceCommandsDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceCommandsDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceCommandsDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceCommandsDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceCommandsDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceCommandsDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceCommandsDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceCommandsDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceCommandsDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceCommandsDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceCommandsDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceCommandsDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceCommandsDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceCommandsDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceCommandsDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceCommandsDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceCommandsDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceCommandsDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceCommandsDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceCommandsDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceCommandsDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceCommandsDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceCommandsDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceCommandsDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceCommandsDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceCommandsDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceCommandsDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceCommandsDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceCommandsDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceCommandsDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceCommandsDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceCommandsDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceCommandsDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceCommandsDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceCommandsDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceCommandsDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceCommandsDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceCommandsDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceCommandsDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceCommandsDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceCommandsDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceCommandsDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceCommandsDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceCommandsDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceCommandsDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceCommandsDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceCommandsDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonCommandModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_ButtonCommandModify_clicked"):
            print "OmniEditDeviceCommandsDialog::on_ButtonCommandModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.command.setName (self.EntryName.get_text ()):
                pszError = "Invalid name"
            if not self.command.setCommand (self.EntryCommand.get_text ()):
                pszError = "Invalid command"

            if pszError == None:
                matchingCommand = self.findCommand (self.command)

                if     matchingCommand != None \
                   and matchingCommand != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             self.command.getName () + " already exists!")
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceCommand (self.command)
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

    def on_ButtonCommandAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_ButtonCommandAdd_clicked"):
            print "OmniEditDeviceCommandsDialog::on_ButtonCommandAdd_clicked:", widget

    def on_ButtonCommandDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked"):
            print "OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked:", widget

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
            (rc, iCommandsLeft) = self.deleteCommand (self.child)
            if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked"):
                print "OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked: deleteCommand rc = %s, iCommandsLeft = %d" % (rc, iCommandsLeft)

            device    = self.commands.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iCommandsLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked"):
                    print "OmniEditDeviceCommandsDialog::on_ButtonCommandDelete_clicked: Last command deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceCommands":
                        treestore.remove (row.iter)
                        device.setDeviceCommands (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_destroy (self, window):
        """Callback for the window being deleted."""
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_destroy"):
            print "OmniEditDeviceCommandsDialog::on_destroy:", window

        window.hide ()
        gtk.mainquit ()

    def on_EntryCommandName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_EntryCommandName_changed"):
            print "OmniEditDeviceCommandsDialog::on_EntryCommandName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryCommandCommand_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceCommandsDialog::on_EntryCommandCommand_changed"):
            print "OmniEditDeviceCommandsDialog::on_EntryCommandCommand_changed:", widget.get_text (), widget

        self.fChanged = True

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.commands.save ()

        self.fModified = False

    def findCommand (self, matchCommand):
        for command in self.commands.getCommands ():
            if matchCommand.getName () == command.getName ():
                return command

        return None

    def deleteCommand (self, matchCommand):
        foundCommand = None
        foundRow     = None
        commands     = self.commands.getCommands ()
        for command in commands:
#           if shouldOutputDebug ("OmniEditDeviceCommandsDialog::deleteCommand"):
#               print "OmniEditDeviceCommandsDialog::deleteCommand: matching1 %s vs %s" % (matchCommand.getName (), command.getName ())
            if matchCommand.getName () == command.getName ():
                foundCommand = command

        device    = self.commands.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceCommandsDialog::deleteCommand"):
#                       print "OmniEditDeviceCommandsDialog::deleteCommand: matching2 %s vs %s" % (childRow[1], matchCommand)
                    if childRow[1] == matchCommand:
                        foundRow = childRow

        if     foundCommand != None \
           and foundRow  != None:
            if shouldOutputDebug ("OmniEditDeviceCommandsDialog::deleteCommand"):
                print "OmniEditDeviceCommandsDialog::deleteCommand: Removing %s from commands" % (foundCommand.getName ())
            commands.remove (foundCommand)

            if shouldOutputDebug ("OmniEditDeviceCommandsDialog::deleteCommand"):
                print "OmniEditDeviceCommandsDialog::deleteCommand: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.commands.setModified (True)

            return (True, len (commands))
        else:
            if foundCommand == None:
                print "OmniEditDeviceCommandsDialog::deleteCommand: Error: Did not find %s in commands!" % (matchCommand.getName ())
            if foundRow == None:
                print "OmniEditDeviceCommandsDialog::deleteCommand: Error: Did not find %s in listbox!" % (str (matchCommand))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Generic ESC2P Commands.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        commands = OmniEditDeviceCommands (fname, doc.documentElement, None)

        print "OmniEditDeviceCommands.py::__main__: commands = ",
        lastCommand = commands.getCommands ()[-1]
        for command in commands.getCommands ():
            command.printSelf (False),
            if command != lastCommand:
                print ",",
        print

        dialog = OmniEditDeviceCommandDialog (commands, commands.getCommands ()[0])
        window = OmniEditDeviceCommandsWindow (dialog)

        gtk.main ()

        print commands.toXML ()

    except Exception, e:
        print "OmniEditDeviceCommands.py::__main__: Error: Caught", e
