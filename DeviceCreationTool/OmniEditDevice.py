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

import OmniEditLoadGtk

if not OmniEditLoadGtk.loadGtk20 ():
    raise SystemExit

import gtk
import gtk.glade
import gobject
import types
import os
from xml.dom.ext.reader import Sax2

from OmniEditUtils import *
import OmniEditDeviceCommands
import OmniEditDeviceConnections
import OmniEditDeviceCopies
import OmniEditDeviceDatas
import OmniEditDeviceForms
import OmniEditDeviceGammaTables
import OmniEditDeviceMedias
import OmniEditDeviceNumberUps
import OmniEditDeviceOrientations
import OmniEditDeviceOutputBins
import OmniEditDevicePrintModes
import OmniEditDeviceResolutions
import OmniEditDeviceScalings
import OmniEditDeviceSheetCollates
import OmniEditDeviceSides
import OmniEditDeviceStitchings
import OmniEditDeviceStrings
import OmniEditDeviceTrays
import OmniEditDeviceTrimmings
import OmniEditOption

def getDefaultXML (name):
    driverName = name
    deviceName = name
    iPosSpace = driverName.find (" ")
    if iPosSpace > -1:
        driverName = name[:iPosSpace]

    return XMLHeader () + """

<Device
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni"
   name=""" + '"' + deviceName + '"' + """>
   <DriverName>""" + driverName + """</DriverName>
   <Capability type="MONOCHROME"/>
   <RasterCapabilities type="TOP_TO_BOTTOM"/>
   <PDL level="PDL_Epson" sublevel="LEVEL_ESCP_2" major="1" minor="0"/>
   <Uses>""" + deviceName + """ Commands.xml</Uses>
   <Has>""" + deviceName + """ Forms.xml</Has>
   <Has>""" + deviceName + """ Medias.xml</Has>
   <Has>""" + deviceName + """ Orientations.xml</Has>
   <Has>""" + deviceName + """ PrintModes.xml</Has>
   <Has>""" + deviceName + """ Resolutions.xml</Has>
   <Has>""" + deviceName + """ Trays.xml</Has>
   <Instance>""" + deviceName + """ Instance.hpp</Instance>
   <Instance>""" + deviceName + """ Instance.cpp</Instance>
   <Blitter>""" + deviceName + """ Blitter.hpp</Blitter>
   <Blitter>""" + deviceName + """ Blitter.cpp</Blitter>
   <DefaultJobProperties>
      <dither>DITHER_STUCKI_DIFFUSION</dither>
      <Form>na_letter_8.50x11.00in</Form>
      <media>MEDIA_PLAIN</media>
      <Rotation>Portrait</Rotation>
      <printmode>PRINT_MODE_1_ANY</printmode>
      <Resolution>180x180</Resolution>
      <InputTray>Continuous</InputTray>
   </DefaultJobProperties>
</Device>
"""

def isValidPDLLevel (level):
    return level in [ "PDL_other",        # Not on this list
                      "PDL_PCL",          # PCL.  Starting with PCL version 5, HP-GL/2 is included as
                      "PDL_HPGL",         # Hewlett-Packard Graphics Language.  HP-GL is a registered
                      "PDL_PJL",          # Peripheral Job Language.  Appears in the data stream
                      "PDL_PS",           # PostScript Language (tm) Postscript - a trademark of Adobe
                      "PDL_IPDS",         # Intelligent Printer Data Stream Bi-directional print data
                      "PDL_PPDS",         # IBM Personal Printer Data Stream.  Originally called IBM
                      "PDL_EscapeP",      # Epson Corp.
                      "PDL_Epson",        # Epson Corp.
                      "PDL_DDIF",         # Digital Document Interchange Format Digital Equipment
                      "PDL_Interpress",   # Xerox Corp.
                      "PDL_ISO6429",      # ISO 6429.  Control functions for Coded Character Sets
                      "PDL_LineData",     # line-data : Lines of data as separate ASCII or EBCDIC
                      "PDL_MODCA",        # Mixed Object Document Content Architecture Definitions
                      "PDL_REGIS",        # Remote Graphics Instruction Set, Digital Equipment
                      "PDL_SCS",          # SNA Character String Bi-directional print data stream for
                      "PDL_SPDL",         # ISO 10180 Standard Page Description Language ISO
                      "PDL_TEK4014",      # Tektronix Corp.
                      "PDL_PDS",          #
                      "PDL_IGP",          # Printronix Corp.
                      "PDL_CodeV",        # Magnum Code-V, Image and printer control language used
                      "PDL_DSCDSE",       # DSC-DSE : Data Stream Compatible and Emulation Bi-
                      "PDL_WPS",          # Windows Printing System, Resource based command/data
                      "PDL_LN03",         # Early DEC-PPL3, Digital Equipment Corp.
                      "PDL_CCITT",        #
                      "PDL_QUIC",         # QUIC (Quality Information Code), Page Description
                      "PDL_CPAP",         # Common Printer Access Protocol Digital Equipment Corp
                      "PDL_DecPPL",       # Digital ANSI-Compliant Printing Protocol (DEC-PPL)
                      "PDL_SimpleText",   # simple-text : character coded data, including NUL,
                      "PDL_NPAP",         # Network Printer Alliance Protocol (NPAP).  This protocol
                      "PDL_DOC",          # Document Option Commands, Appears in the data stream
                      "PDL_imPress",      # imPRESS, Page description language originally
                      "PDL_Pinwriter",    # 24 wire dot matrix printer for USA, Europe, and
                      "PDL_NPDL",         # Page printer for Japanese market.  NEC
                      "PDL_NEC201PL",     # Serial printer language used in the Japanese market.
                      "PDL_Automatic",    # Automatic PDL sensing.  Automatic sensing of the
                      "PDL_Pages",        # Page printer Advanced Graphic Escape Set IBM Japan
                      "PDL_LIPS",         # LBP Image Processing System
                      "PDL_TIFF",         # Tagged Image File Format (Aldus)
                      "PDL_Diagnostic",   # A hex dump of the input to the interpreter
                      "PDL_PSPrinter",    # The PostScript Language used for control (with any
                      "PDL_CaPSL",        # Canon Print Systems Language
                      "PDL_EXCL",         # Extended Command Language Talaris Systems Inc
                      "PDL_LCDS",         # Line Conditioned Data Stream Xerox Corporatio
                      "PDL_XES",          # Xerox Escape Sequences Xerox Corporation
                      "PDL_PCLXL",        # Printer Control Language.  Extended language features
                      "PDL_ART",          # Advanced Rendering Tools (ART).  Page Description
                      "PDL_TIPSI",        # Transport Independent Printer System Interface (ref.
                      "PDL_Prescribe",    # Page description and printer control language.  It
                      "PDL_LinePrinter",  # A simple-text character stream which supports the
                      "PDL_IDP",          # Imaging Device Protocol Apple Computer.
                      "PDL_XJCL",         # Xerox Corp.
                      "PDL_ALPS",         # ALPS Corp.
                      "PDL_Olivetti",     # Olivetti Corp.
                      "PDL_Deskjet",      # Hewlett-Packard Deskjet subset (IPCL)
                      "PDL_Paintjet",     # Hewlett-Packard Paintjet subset
                      "PDL_Seiko",        # Seiko Corp.
                      "PDL_PassThru",     # Does not alter data stream
                      "PDL_5577"          # 5577 Line and Serial Impact Dot Printer IBM Japan
                    ]

def isValidPDLSublevelLevel (level, sublevel):
    if level == "PDL_PS":
        return sublevel in [ "LEVEL_PS_LEVEL1",        # Postscript Level  1
                             "LEVEL_PS_LEVEL2"         # Postscript Level  2
                           ]
    elif level == "PDL_PCL" :
        return sublevel in [ "LEVEL_PCL2",
                             "LEVEL_PCL3",
                             "LEVEL_PCL3C",
                             "LEVEL_PCL4",
                             "LEVEL_PCL5",
                             "LEVEL_PCL5C",
                             "LEVEL_PCL6",
                             "LEVEL_PCL5E"
                           ]
    elif level == "PDL_HPGL" :
        return sublevel in [ "LEVEL_HPGL1",
                             "LEVEL_HPGL2",
                             "LEVEL_HPGL2_RTL",    # HPGL2 with RTL language used for raster transfer
                             "LEVEL_HPGL2_PCLRTL", # HPGL2 with PCL used for raster transfer
                             "LEVEL_HPGL2_MC"      # HPGL2 with MC command support
                           ]
    elif level == "PDL_Epson" :
        return sublevel in [ "LEVEL_ESC",
                             "LEVEL_ESCP",
                             "LEVEL_ESCP_2",
                             "LEVEL_ESCP_2J",      # Japan version
                           ]
    elif level == "PDL_SimpleText" :
        return sublevel in [ "LEVEL_ASCII_TEXT",
                             "LEVEL_ASCII_PROPRINTER",
                             "LEVEL_ASCII_QUITWRITER",
                             "LEVEL_ASCII_JISASCII"
                           ]
    elif level == "PDL_Deskjet" :
        return sublevel in [ "LEVEL_DESKJET",
                             "LEVEL_DESKJETJ"
                           ]
    else:
        return False

class PDL:
    def __init__ (self, pdl):
        if type (pdl) == types.InstanceType:
            self.setLevel (pdl.getLevel ())
            self.setSubLevel (pdl.getSubLevel ())
            self.setMajor (pdl.getMajor ())
            self.setMinor (pdl.getMinor ())
        elif type (pdl) == types.ListType or type (pdl) == types.TupleType:
            if len (pdl) != 4:
                raise Exception ("Error: OmniEditDevicePDL: expecting 4 elements, received " + str (len (pdl)) + " of " + str (pdl))
            if not self.setLevel (pdl[0]):
                raise Exception ("Error: OmniEditDevicePDL: can't set level!")
            if not self.setSubLevel (pdl[1]):
                raise Exception ("Error: OmniEditDevicePDL: can't set sublevel!")
            if not self.setMajor (pdl[2]):
                raise Exception ("Error: OmniEditDevicePDL: can't set major!")
            if not self.setMinor (pdl[3]):
                raise Exception ("Error: OmniEditDevicePDL: can't set minor!")
        else:
            raise Exception ("Expecting OmniEditDevicePDL, list, or tuple.  Got " + str (type (pdl)))

    def getLevel (self):
        return self.level

    def setLevel (self, level):
        if isValidPDLLevel (level):
            self.level = level
            return True
        else:
            print "PDL::setLevel: Error: (%s) is not a valid pdl level!" % (level)
            return False

    def getSubLevel (self):
        return self.sublevel

    def setSubLevel (self, sublevel):
        if isValidPDLSublevelLevel (self.level, sublevel):
            self.sublevel = sublevel
            return True
        else:
            print "PDL::setSubLevel: Error: (%s) is not a valid pdl sublevel (%s)!" % (sublevel, self.level)
            return False

    def getMajor (self):
        return self.major

    def setMajor (self, major):
        try:
            self.major = convertToIntegerValue (major)
            return True
        except Exception, e:
            print "PDL::setMajor: Error: (%s) is not an integer!" % (major)
            return False

    def getMinor (self):
        return self.minor

    def setMinor (self, minor):
        try:
            self.minor = convertToIntegerValue (minor)
            return True
        except Exception, e:
            print "PDL::setMinor: Error: (%s) is not an integer!" % (minor)
            return False

class DeviceInformation:
    def __init__ (self, information):
        if type (information) == types.InstanceType:
            self.setDriver (information.getDriver ())
            self.setDriverName (information.getDriverName ())
            self.setDeviceName (information.getDeviceName ())
            self.setCapabilities (information.getCapabilities ())
            self.setRasterCapabilities (information.getRasterCapabilities ())
            self.setDeviceOptions (information.getDeviceOptions ())
            self.setPDL (information.getPDL ())
            self.setDefaultJobProperties (information.getDefaultJobProperties ())
        elif type (information) == types.ListType or type (information) == types.TupleType:
            if len (information) != 8:
                raise Exception ("Error: OmniEditDeviceInformation: expecting 8 elements, received " + str (len (information)) + " of " + str (information))
            if not self.setDriver (information[0]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set driver!")
            if not self.setDriverName (information[1]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set driver name!")
            if not self.setDeviceName (information[2]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set device name!")
            if not self.setCapabilities (information[3]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set capabilities!")
            if not self.setRasterCapabilities (information[4]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set raster capabilities!")
            if not self.setDeviceOptions (information[5]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set device options!")
            if not self.setPDL (information[6]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set pdl!")
            if not self.setDefaultJobProperties (information[7]):
                raise Exception ("Error: OmniEditDeviceInformation: can't set default job properties!")
        else:
            raise Exception ("Expecting OmniEditDeviceCommand, list, or tuple.  Got " + str (type (information)))

    def getDriver (self):
        return self.driver

    def setDriver (self, driver):
        self.driver = driver
        return True

    def getDriverName (self):
        return self.driverName

    def setDriverName (self, driverName):
        self.driverName = driverName
        return True

    def getDeviceName (self):
        return self.deviceName

    def setDeviceName (self, deviceName):
        self.deviceName = deviceName
        return True

    def getCapabilities (self):
        return self.capabilities

    def setCapabilities (self, capabilities):
        self.capabilities = capabilities
        return True

    def setCapability (self, capability):
        if self.capabilities == None:
            self.capabilities = []
        self.capabilities.append (capability)
        return True

    def deleteCapability (self, capability):
        if self.capabilities != None:
            try:
                self.capabilities.remove (capability)
            except Exception, e:
                pass

    def getRasterCapabilities (self):
        return self.rasterCapabilities

    def setRasterCapabilities (self, rasterCapabilities):
        self.rasterCapabilities = rasterCapabilities
        return True

    def setRasterCapability (self, rasterCapability):
        if self.rasterCapabilities == None:
            self.rasterCapabilities = []
        self.rasterCapabilities.append (rasterCapability)
        return True

    def deleteRasterCapability (self, rasterCapability):
        if self.rasterCapabilities != None:
            try:
                self.rasterCapabilities.remove (rasterCapability)
            except Exception, e:
                pass

    def getDeviceOptions (self):
        return self.deviceOptions

    def setDeviceOptions (self, deviceOptions):
        self.deviceOptions = deviceOptions
        return True

    def setDeviceOption (self, deviceOption):
        if self.deviceOptions == None:
            self.deviceOptions = []
        self.deviceOptions.append (deviceOption)
        return True

    def deleteDeviceOption (self, deviceOption):
        if self.deviceOptions != None:
            try:
                self.deviceOptions.remove (deviceOption)
            except Exception, e:
                pass

    def getPDL (self):
        return self.pdl

    def setPDL (self, pdl):
        self.pdl = pdl
        return True

    def getDefaultJobProperties (self):
        return self.defaultJobProperties

    def setDefaultJobProperties (self, defaultJobProperties):
        self.defaultJobProperties = defaultJobProperties
        return True

    def getDefaultJobProperty (self, defaultJobPropertyKey):
        if defaultJobPropertyKey == None:
            return None

        if self.defaultJobProperties == None:
            return None

        if not self.defaultJobProperties.has_key (defaultJobPropertyKey):
            return None

        return self.defaultJobProperties[defaultJobPropertyKey]

    def setDefaultJobProperty (self, defaultJobPropertyKey, defaultJobPropertyValue):
        if self.defaultJobProperties == None:
            self.defaultJobProperties = {}
        self.defaultJobProperties[defaultJobPropertyKey] = defaultJobPropertyValue
        return True

    def deleteDefaultJobProperty (self, defaultJobPropertyKey):
        if self.defaultJobProperties != None:
            del self.defaultJobProperties[defaultJobPropertyKey]

    def getDialog (self, child):
        return DeviceInformationDialog (self, child)

    def setDeviceInformation (self, information):
        try:
            if type (information) == types.InstanceType:
                self.setDriverName (information.getDriverName ())
                self.setDeviceName (information.getDeviceName ())
                self.setCapabilities (information.getCapabilities ())
                self.setRasterCapabilities (information.getRasterCapabilities ())
                self.setDeviceOptions (information.getDeviceOptions ())
                self.setPDL (information.getPDL ())
                self.setDefaultJobProperties (information.getDefaultJobProperties ())
            else:
                print "DeviceInformation::setDeviceInformation: Error: Expecting DeviceInformation.  Got ", str (type (information))
                return False
        except Exception, e:
            print "DeviceInformation::setDeviceInformation: Error: caught " + e
            return False

        return True

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s, %s, %s]" % (self.getDriverName (),
                                                self.getDeviceName (),
                                                self.getCapabilities (),
                                                self.getRasterCapabilities (),
                                                self.getDeviceOptions (),
                                                self.getPDL (),
                                                self.getDefaultJobProperties ()),
        if fNewLine:
            print

    def save (self):
        # This is a "container class" of OmniEditDevice, so the saving will be done in OmniEditDeviceDialog
        pass

class DeviceInformationWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceInformationWindow')

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
        if shouldOutputDebug ("DeviceInformationWindow::on_window_delete"):
            print "DeviceInformationWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("DeviceInformationWindow::on_window_destroy"):
            print "DeviceInformationWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class DeviceInformationDialog:
    def __init__ (self, informations, child):
        information = DeviceInformation (child)

        self.informations = informations
        self.information  = information
        self.child        = child
        self.fChanged     = False
        self.fModified    = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceInformationFrame')

        self.xml              = xml
        self.window           = window
        self.EntryDriverName  = xml.get_widget ('EntryDeviceInformationDriverName')
        self.EntryDeviceName  = xml.get_widget ('EntryDeviceInformationDeviceName')
        self.EntryPDLLevel    = xml.get_widget ('EntryDeviceInformationPDLLevel')
        self.EntryPDLSubLevel = xml.get_widget ('EntryDeviceInformationPDLSubLevel')
        self.EntryPDLMajor    = xml.get_widget ('EntryDeviceInformationPDLMajor')
        self.EntryPDLMinor    = xml.get_widget ('EntryDeviceInformationPDLMinor')

        dic = { "on_ButtonCommandAdd_clicked":                             self.on_ButtonCommandAdd_clicked,
                "on_ButtonCommandDelete_clicked":                          self.on_ButtonCommandDelete_clicked,
                "on_ButtonCommandModify_clicked":                          self.on_ButtonCommandModify_clicked,
                "on_ButtonCommandDelete_clicked":                          self.on_ButtonCommandDelete_clicked,
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

#       if shouldOutputDebug ("DeviceInformationDialog::__init__"):
#           print "DeviceInformationDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryDriverName.set_text (str (information.getDriverName ()))
        self.EntryDeviceName.set_text (str (information.getDeviceName ()))
        pdl = information.getPDL ()
        self.EntryPDLLevel.set_text (str (pdl.getLevel ()))
        self.EntryPDLSubLevel.set_text (str (pdl.getSubLevel ()))
        self.EntryPDLMajor.set_text (str (pdl.getMajor ()))
        self.EntryPDLMinor.set_text (str (pdl.getMinor ()))

        self.fChanged = False

        window.show ()

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):               print "DeviceInformationDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):               print "DeviceInformationDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                        print "DeviceInformationDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                        print "DeviceInformationDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):               print "DeviceInformationDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):               print "DeviceInformationDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                       print "DeviceInformationDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                    print "DeviceInformationDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                    print "DeviceInformationDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                    print "DeviceInformationDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                      print "DeviceInformationDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                       print "DeviceInformationDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):              print "DeviceInformationDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):              print "DeviceInformationDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                print "DeviceInformationDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                print "DeviceInformationDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                print "DeviceInformationDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                print "DeviceInformationDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):               print "DeviceInformationDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):               print "DeviceInformationDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                  print "DeviceInformationDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                print "DeviceInformationDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):             print "DeviceInformationDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):             print "DeviceInformationDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                        print "DeviceInformationDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                print "DeviceInformationDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                print "DeviceInformationDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                    print "DeviceInformationDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                   print "DeviceInformationDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                print "DeviceInformationDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                print "DeviceInformationDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                        print "DeviceInformationDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                     print "DeviceInformationDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                    print "DeviceInformationDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                 print "DeviceInformationDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):               print "DeviceInformationDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):              print "DeviceInformationDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):           print "DeviceInformationDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):        print "DeviceInformationDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):          print "DeviceInformationDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):          print "DeviceInformationDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):         print "DeviceInformationDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):            print "DeviceInformationDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):       print "DeviceInformationDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):               print "DeviceInformationDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):          print "DeviceInformationDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):               print "DeviceInformationDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):           print "DeviceInformationDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                     print "DeviceInformationDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                  print "DeviceInformationDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                  print "DeviceInformationDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                  print "DeviceInformationDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):              print "DeviceInformationDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                  print "DeviceInformationDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):              print "DeviceInformationDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                 print "DeviceInformationDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                   print "DeviceInformationDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                     print "DeviceInformationDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                    print "DeviceInformationDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                     print "DeviceInformationDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                     print "DeviceInformationDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):          print "DeviceInformationDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                        print "DeviceInformationDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                        print "DeviceInformationDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                        print "DeviceInformationDialog::on_EntryDataData_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                print "DeviceInformationDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                     print "DeviceInformationDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                    print "DeviceInformationDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                   print "DeviceInformationDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                     print "DeviceInformationDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                    print "DeviceInformationDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                      print "DeviceInformationDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                        print "DeviceInformationDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                    print "DeviceInformationDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):            print "DeviceInformationDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):        print "DeviceInformationDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                 print "DeviceInformationDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):             print "DeviceInformationDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):        print "DeviceInformationDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                print "DeviceInformationDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                print "DeviceInformationDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                print "DeviceInformationDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                print "DeviceInformationDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                 print "DeviceInformationDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                 print "DeviceInformationDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                 print "DeviceInformationDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                 print "DeviceInformationDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                 print "DeviceInformationDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):        print "DeviceInformationDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                    print "DeviceInformationDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                   print "DeviceInformationDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                       print "DeviceInformationDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                 print "DeviceInformationDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                print "DeviceInformationDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):   print "DeviceInformationDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):      print "DeviceInformationDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                       print "DeviceInformationDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                       print "DeviceInformationDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                      print "DeviceInformationDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):             print "DeviceInformationDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                 print "DeviceInformationDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):             print "DeviceInformationDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):   print "DeviceInformationDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                print "DeviceInformationDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):               print "DeviceInformationDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                   print "DeviceInformationDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):               print "DeviceInformationDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):           print "DeviceInformationDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                   print "DeviceInformationDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):          print "DeviceInformationDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                 print "DeviceInformationDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):            print "DeviceInformationDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):               print "DeviceInformationDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window): print "DeviceInformationDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):      print "DeviceInformationDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):              print "DeviceInformationDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                  print "DeviceInformationDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):              print "DeviceInformationDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):          print "DeviceInformationDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                  print "DeviceInformationDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):          print "DeviceInformationDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                  print "DeviceInformationDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):              print "DeviceInformationDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                  print "DeviceInformationDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                  print "DeviceInformationDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                 print "DeviceInformationDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                  print "DeviceInformationDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                  print "DeviceInformationDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):        print "DeviceInformationDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):             print "DeviceInformationDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):            print "DeviceInformationDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                print "DeviceInformationDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                     print "DeviceInformationDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                    print "DeviceInformationDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                        print "DeviceInformationDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):          print "DeviceInformationDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                  print "DeviceInformationDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                print "DeviceInformationDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                  print "DeviceInformationDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):               print "DeviceInformationDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):               print "DeviceInformationDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):          print "DeviceInformationDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                   print "DeviceInformationDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):               print "DeviceInformationDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                   print "DeviceInformationDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):               print "DeviceInformationDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                     print "DeviceInformationDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                    print "DeviceInformationDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                        print "DeviceInformationDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                    print "DeviceInformationDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                    print "DeviceInformationDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                 print "DeviceInformationDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                print "DeviceInformationDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                    print "DeviceInformationDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                           print "DeviceInformationDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                        print "DeviceInformationDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                     print "DeviceInformationDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                          print "DeviceInformationDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                           print "DeviceInformationDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                           print "DeviceInformationDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                     print "DeviceInformationDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                          print "DeviceInformationDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                       print "DeviceInformationDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                    print "DeviceInformationDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                      print "DeviceInformationDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                      print "DeviceInformationDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                     print "DeviceInformationDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                        print "DeviceInformationDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                   print "DeviceInformationDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                           print "DeviceInformationDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                      print "DeviceInformationDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                         print "DeviceInformationDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                           print "DeviceInformationDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                       print "DeviceInformationDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonDeviceInformationCancel_clicked (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_ButtonDeviceInformationCancel_clicked"):
            print "DeviceInformationDialog::on_ButtonDeviceInformationCancel_clicked:", widget

    def on_ButtonDeviceInformationModify_clicked (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_ButtonDeviceInformationModify_clicked"):
            print "DeviceInformationDialog::on_ButtonDeviceInformationModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.information.setDriverName (self.EntryDriverName.get_text ()):
                pszError = "Invalid driver name"
            if not self.information.setDeviceName (self.EntryDeviceName.get_text ()):
                pszError = "Invalid device name"
            pdl = self.information.getPDL ()
            if not pdl.setLevel (self.EntryPDLLevel.get_text ()):
                pszError = "Invalid pdl level"
            if not pdl.setSubLevel (self.EntryPDLSubLevel.get_text ()):
                pszError = "Invalid pdl sublevel"
            if not pdl.setMajor (self.EntryPDLMajor.get_text ()):
                pszError = "Invalid pdl major"
            if not pdl.setMinor (self.EntryPDLMinor.get_text ()):
                pszError = "Invalid pdl minor"

            if pszError == None:
                self.child.setDeviceInformation (self.information)
                self.fModified = True

                # Notify parent of this "container class" that it is modified
                driver = self.child.getDriver ()
                dialog = driver.getDeviceDialog ()
                dialog.setModified (True)
            else:
                ask = gtk.MessageDialog (None,
                                         0,
                                         gtk.MESSAGE_QUESTION,
                                         gtk.BUTTONS_NONE,
                                         pszError)
                ask.add_button ("_Ok", gtk.RESPONSE_YES)
                response = ask.run ()
                ask.destroy ()

    def on_EntryDeviceInformationDriverName_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationDriverName_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationDriverName_changed:", widget

        self.fChanged = True

    def on_EntryDeviceInformationDeviceName_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationDeviceName_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationDeviceName_changed:", widget

        self.fChanged = True

    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked"):
            print "DeviceInformationDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked:", widget

        dialog = OmniEditOption.OmniEditOptionDialog (self.information,
                                                      OmniEditOption.CAPABILITIES,
                                                      self)

    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked"):
            print "DeviceInformationDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked:", widget

        dialog = OmniEditOption.OmniEditOptionDialog (self.information,
                                                      OmniEditOption.RASTERCAPABILITIES,
                                                      self)

    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked"):
            print "DeviceInformationDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked:", widget

        dialog = OmniEditOption.OmniEditOptionDialog (self.information,
                                                      OmniEditOption.DEVICEOPTIONS,
                                                      self)

    def on_EntryDeviceInformationPDLLevel_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationPDLLevel_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationPDLLevel_changed:", widget

        self.fChanged = True

    def on_EntryDeviceInformationPDLSubLevel_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationPDLSubLevel_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationPDLSubLevel_changed:", widget

        self.fChanged = True

    def on_EntryDeviceInformationPDLMajor_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationPDLMajor_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationPDLMajor_changed:", widget

        self.fChanged = True

    def on_EntryDeviceInformationPDLMinor_changed (self, widget):
        if shouldOutputDebug ("DeviceInformationDialog::on_EntryDeviceInformationPDLMinor_changed"):
            print "DeviceInformationDialog::on_EntryDeviceInformationPDLMinor_changed:", widget

        self.fChanged = True

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def setChanged (self, fChanged):
        self.fChanged = fChanged

    def save (self):
        self.informations.save ()

        self.fModified = False

class Driver:
    def __init__ (self, rootPath, filename):
        self.dialog   = None
        self.filename = rootPath + os.sep + filename

        print 'Parsing "%s"' % (self.filename)

        from xml.dom.ext.reader import Sax2

        fImaginary = False
        if fileExists (self.filename):
            try:
                file = open (self.filename)

            except IOError, e:
                print "Driver::__init__: Error: File '%s' does not exist!" % self.filename
                raise e

            # create Reader object
            reader = Sax2.Reader (False, True)

            # parse the document
            self.doc = reader.fromStream (file)

            file.close ()
        else:
            fImaginary = True
            baseName   = filename.replace (".xml", "")
            pszXML     = getDefaultXML (baseName)

            # create Reader object
            reader = Sax2.Reader (False, True)

            # parse the document
            self.doc = reader.fromString (pszXML)

        self.rootPath            = rootPath
        self.rootElement         = self.doc.documentElement
        self.deviceInformation   = None
        self.deviceCommands      = None
        self.deviceConnections   = None
        self.deviceCopies        = None
        self.deviceDatas         = None
        self.deviceForms         = None
        self.deviceGammaTables   = None
        self.deviceMedias        = None
        self.deviceNumberUps     = None
        self.deviceOrientations  = None
        self.deviceOutputBins    = None
        self.devicePrintModes    = None
        self.deviceResolutions   = None
        self.deviceScalings      = None
        self.deviceSheetCollates = None
        self.deviceSides         = None
        self.deviceStitchings    = None
        self.deviceStrings       = None
        self.deviceTrays         = None
        self.deviceTrimmings     = None
        self.instance1           = None
        self.instance2           = None
        self.blitter1            = None
        self.blitter2            = None
        self.pluggableExename    = None

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

        self.fImaginary = fImaginary
        if fImaginary:
            if self.deviceCommands != None:
                self.deviceCommands.setModified (True)
            if self.deviceForms != None:
                self.deviceForms.setModified (True)
            if self.deviceMedias != None:
                self.deviceMedias.setModified (True)
            if self.deviceOrientations != None:
                self.deviceOrientations.setModified (True)
            if self.devicePrintModes != None:
                self.devicePrintModes.setModified (True)
            if self.deviceResolutions != None:
                self.deviceResolutions.setModified (True)
            if self.deviceTrays != None:
                self.deviceTrays.setModified (True)

    def isImaginary (self):
        return self.fImaginary

    def addComplexJobProperties (self, elm, jobProperties):
        elm = firstNode (elm.firstChild)

        while elm != None:
           if validSingleElement (elm):
               jobProperties[elm.nodeName] = getValue (elm)
           elif len (elm.attributes) == 1:
               attr = elm.attributes.getNamedItem ("FORMAT")

               if     attr != None            \
                  and attr.nodeValue == "XbyY":
                   key   = elm.nodeName
                   child = firstNode (elm.firstChild)

                   if     child.nodeName == "x"       \
                      and len (child.childNodes) == 1:
                       value = getValue (child)
                       value += "x"
                       child = nextNode (child)

                       if     child.nodeName == "y"       \
                          and len (child.childNodes) == 1:
                           value += getValue (child)
                       else:
                           value = None

                   if value != None:
                       jobProperties[key] = value

           elm = nextNode (elm)

    def isValid (self):
#       if shouldOutputDebug ("Driver::isValid"):
#           print self.__class__.__name__ + ".isValid"

        driverName         = ""
        deviceName         = ""
        capabilities       = []
        rasterCapabilities = []
        deviceOptions      = []
        pdl                = None
        jobProperties      = {}

        elm = self.rootElement

        if elm == None:
            return "Root element missing"

        if elm.nodeName != "Device":
            return "Missing <Device>"

        attr = elm.attributes.getNamedItem ("name")
        if attr == None:
            return "Missing name= attribute in <Device>"

        deviceName = attr.nodeValue

        elm = firstNode (elm.firstChild)

        if elm == None:
            return "Child missing"

        if elm.nodeName != "DriverName":
            return "Missing <DriverName>"

        if not validSingleElement (elm):
            return "<DriverName> should only have one element"

        driverName = getValue (elm)

        elm = nextNode (elm)

        if elm == None:
            return "Missing <Has>/<Uses>"

        while elm.nodeName == "Capability":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <Capability>"

                capabilities.append (attr.nodeValue)

            else:
                return "<Capability> node should only have 1 type= attribute"

            elm = nextNode (elm)

        if elm == None:
            return "Missing <Has>/<Uses>"

        while elm.nodeName == "RasterCapabilities":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <RasterCapabilies>"

                rasterCapabilities.append (attr.nodeValue)

            else:
                return "<RasterCapabilities> node should only have 1 type= attribute"

            elm = nextNode (elm)

        if elm == None:
            return "Missing <Has>/<Uses>"

        while elm.nodeName == "DeviceOptions":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <DeviceOptions>"

                deviceOptions.append (attr.nodeValue)

            else:
                return "<DeviceOptions> node should only have 1 type attribute"

            elm = nextNode (elm)

        if elm == None:
            return "Missing <Has>/<Uses>"

        if elm.nodeName == "PDL":
            if len (elm.attributes) != 4:
                return "<PDL> node should only have 4 type attributes (level, sublevel, major, minor)"

            try:
                attr = elm.attributes.getNamedItem ("level")

                pdlLevel = attr.nodeValue
            except Exception, e:
                return "<PDL> missing level attribute"

            try:
                attr = elm.attributes.getNamedItem ("sublevel")

                pdlSubLevel = attr.nodeValue
            except Exception, e:
                return "<PDL> missing sublevel attribute"

            try:
                attr = elm.attributes.getNamedItem ("major")

                pdlMajor = attr.nodeValue
            except Exception, e:
                return "<PDL> missing major attribute"

            try:
                attr = elm.attributes.getNamedItem ("minor")

                pdlMinor = attr.nodeValue
            except Exception, e:
                return "<PDL> missing minor attribute"

            pdl = PDL ((pdlLevel,
                        pdlSubLevel,
                        pdlMajor,
                        pdlMinor))

            elm = nextNode (elm)

        if elm == None:
            return "Missing <Has>/<Uses>"

        if     elm.nodeName != "Has"  \
           and elm.nodeName != "Uses":
            return "Missing <Has>/<Uses>"

        while    elm.nodeName == "Has"  \
              or elm.nodeName == "Uses":

            if not validSingleElement (elm):
                return "<Has>/<Uses> should only have one element"

            xmlFilename = getValue (elm)
            filename    = self.rootPath + os.sep + xmlFilename

            if fileExists (filename):
                try:
                    file = open (filename)

                    # create Reader object
                    reader = Sax2.Reader (False, True)

                    # parse the document
                    doc = reader.fromStream (file)

                    name = doc.documentElement.nodeName

                    if name == "deviceCommands":
                        self.deviceCommands = OmniEditDeviceCommands.OmniEditDeviceCommands (filename, doc.documentElement, self)
                    elif name == "deviceConnections":
                        self.deviceConnections = OmniEditDeviceConnections.OmniEditDeviceConnections (filename, doc.documentElement, self)
                    elif name == "deviceCopies":
                        self.deviceCopies = OmniEditDeviceCopies.OmniEditDeviceCopies (filename, doc.documentElement, self)
                    elif name == "deviceDatas":
                        self.deviceDatas = OmniEditDeviceDatas.OmniEditDeviceDatas (filename, doc.documentElement, self)
                    elif name == "deviceForms":
                        self.deviceForms = OmniEditDeviceForms.OmniEditDeviceForms (filename, doc.documentElement, self)
                    elif name == "deviceGammaTables":
                        self.deviceGammaTables = OmniEditDeviceGammaTables.OmniEditDeviceGammaTables (filename, doc.documentElement, self)
                    elif name == "deviceMedias":
                        self.deviceMedias = OmniEditDeviceMedias.OmniEditDeviceMedias (filename, doc.documentElement, self)
                    elif name == "deviceNumberUps":
                        self.deviceNumberUps = OmniEditDeviceNumberUps.OmniEditDeviceNumberUps (filename, doc.documentElement, self)
                    elif name == "deviceOrientations":
                        self.deviceOrientations = OmniEditDeviceOrientations.OmniEditDeviceOrientations (filename, doc.documentElement, self)
                    elif name == "deviceOutputBins":
                        self.deviceOutputBins = OmniEditDeviceOutputBins.OmniEditDeviceOutputBins (filename, doc.documentElement, self)
                    elif name == "devicePrintModes":
                        self.devicePrintModes = OmniEditDevicePrintModes.OmniEditDevicePrintModes (filename, doc.documentElement, self)
                    elif name == "deviceResolutions":
                        self.deviceResolutions = OmniEditDeviceResolutions.OmniEditDeviceResolutions (filename, doc.documentElement, self)
                    elif name == "deviceScalings":
                        self.deviceScalings = OmniEditDeviceScalings.OmniEditDeviceScalings (filename, doc.documentElement, self)
                    elif name == "deviceSheetCollates":
                        self.deviceSheetCollates = OmniEditDeviceSheetCollates.OmniEditDeviceSheetCollates (filename, doc.documentElement, self)
                    elif name == "deviceSides":
                        self.deviceSides = OmniEditDeviceSides.OmniEditDeviceSides (filename, doc.documentElement, self)
                    elif name == "deviceStitchings":
                        self.deviceStitchings = OmniEditDeviceStitchings.OmniEditDeviceStitchings (filename, doc.documentElement, self)
                    elif name == "deviceStrings":
                        self.deviceStrings = OmniEditDeviceStrings.OmniEditDeviceStrings (filename, doc.documentElement, self)
                    elif name == "deviceTrays":
                        self.deviceTrays = OmniEditDeviceTrays.OmniEditDeviceTrays (filename, doc.documentElement, self)
                    elif name == "deviceTrimmings":
                        self.deviceTrimmings = OmniEditDeviceTrimmings.OmniEditDeviceTrimmings (filename, doc.documentElement, self)

                    file.close ()

                except IOError, e:
                    print "Driver::isValid: Error: File '%s' does not exist!" % filename
                    raise e
            else:
                # @HACK begin
                if xmlFilename.find ("Commands") > -1:
                    pszXML = OmniEditDeviceCommands.getDefaultXML (None)
                elif xmlFilename.find ("Forms") > -1:
                    pszXML = OmniEditDeviceForms.getDefaultXML (None)
                elif xmlFilename.find ("Medias") > -1:
                    pszXML = OmniEditDeviceMedias.getDefaultXML (None)
                elif xmlFilename.find ("Orientations") > -1:
                    pszXML = OmniEditDeviceOrientations.getDefaultXML (None)
                elif xmlFilename.find ("PrintModes") > -1:
                    pszXML = OmniEditDevicePrintModes.getDefaultXML (None)
                elif xmlFilename.find ("Resolutions") > -1:
                    pszXML = OmniEditDeviceResolutions.getDefaultXML (None)
                elif xmlFilename.find ("Trays") > -1:
                    pszXML = OmniEditDeviceTrays.getDefaultXML (None)

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                if xmlFilename.find ("Commands") > -1:
                    self.deviceCommands = OmniEditDeviceCommands.OmniEditDeviceCommands (filename, doc.documentElement, self)
                elif xmlFilename.find ("Forms") > -1:
                    self.deviceForms = OmniEditDeviceForms.OmniEditDeviceForms (filename, doc.documentElement, self)
                elif xmlFilename.find ("Medias") > -1:
                    self.deviceMedias = OmniEditDeviceMedias.OmniEditDeviceMedias (filename, doc.documentElement, self)
                elif xmlFilename.find ("Orientations") > -1:
                    self.deviceOrientations = OmniEditDeviceOrientations.OmniEditDeviceOrientations (filename, doc.documentElement, self)
                elif xmlFilename.find ("PrintModes") > -1:
                    self.devicePrintModes = OmniEditDevicePrintModes.OmniEditDevicePrintModes (filename, doc.documentElement, self)
                elif xmlFilename.find ("Resolutions") > -1:
                    self.deviceResolutions = OmniEditDeviceResolutions.OmniEditDeviceResolutions (filename, doc.documentElement, self)
                elif xmlFilename.find ("Trays") > -1:
                    self.deviceTrays = OmniEditDeviceTrays.OmniEditDeviceTrays (filename, doc.documentElement, self)
                # @HACK end

            elm = nextNode (elm)

            if elm == None:
                return "Missing <Instance>"

        if elm.nodeName == "Instance":

            self.instance1 = getValue (elm)
            elm            = nextNode (elm)

            if    elm == None \
               or elm.nodeName != "Instance":
                return "2 <Instance>s not found"

            self.instance2 = getValue (elm)
            elm            = nextNode (elm)

            if    elm == None \
               or elm.nodeName != "Blitter":
                return "<Blitter> missing"

            self.blitter1 = getValue (elm)
            elm           = nextNode (elm)

            if    elm == None \
               or elm.nodeName != "Blitter":
                return "2 <Blitter>s not found"

            self.blitter2 = getValue (elm)
            elm           = nextNode (elm)

        elif elm.nodeName == "PluggableInstance":
            if len (elm.attributes) != 1:
                return "Only one attribute allowed for <PluggableInstance>"

            attr = elm.attributes.getNamedItem ("exename")

            if attr == None:
                return "Missing exename= attribute for <PluggableInstance>"

            self.pluggableExename = attr.nodeValue

            elm = nextNode (elm)

            if    elm == None \
               or elm.nodeName != "PluggableBlitter":
                return "<PluggableBlitter> missing"

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "DefaultJobProperties":
            return "missing <DefaultJobProperties>"

        elmSave = nextNode (elm)
        elm     = firstNode (elm.firstChild)

        if elm == None:
            return "Missing <dither>"

        if elm.nodeName == "Copies":
            if not validSingleElement (elm):
                return "<Copies> should only have one element"

            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "dither":
            return "missing <dither>"

        if not validSingleElement (elm):
            return "<dither> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "Form":
            return "missing <Form>"

        if not validSingleElement (elm):
            return "<Form> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm == None:
            return "Missing <media>"

        if elm.nodeName == "omniForm":
            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "media":
            return "missing <media>"

        if not validSingleElement (elm):
            return "<media> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm == None:
            return "Missing <Rotation>"

        if elm.nodeName == "NumberUp":
            self.addComplexJobProperties (elm, jobProperties)

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "Rotation":
            return "missing <Rotation>"

        if not validSingleElement (elm):
            return "<Rotation> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm == None:
            return "Missing <printmode>"

        if elm.nodeName == "omniOrientation":
            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "OutputBin":
            if not validSingleElement (elm):
                return "<OutputBin> should only have one element"

            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "printmode":
            return "missing <printmode>"

        if not validSingleElement (elm):
            return "<printmode> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "Resolution":
            return "missing <Resolution>"

        if not validSingleElement (elm):
            return "<Resolution> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "omniResolution":
            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "Scaling":
            self.addComplexJobProperties (elm, jobProperties)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "SheetCollate":
            if not validSingleElement (elm):
                return "<SheetCollate> should only have one element"

            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "Sides":
            if not validSingleElement (elm):
                return "<Sides> should only have one element"

            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "Stitching":
            self.addComplexJobProperties (elm, jobProperties)

            elm = nextNode (elm)

        if    elm == None \
           or elm.nodeName != "InputTray":
            return "missing <InputTray>"

        if not validSingleElement (elm):
            return "<InputTray> should only have one element"

        jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "omniTray":
            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "Trimming":
            if not validSingleElement (elm):
                return "<Trimming> should only have one element"

            jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if     elm != None \
           and elm.nodeName == "other":
            while     elm != None             \
                  and elm.nodeName == "other":
                if not validSingleElement (elm):
                    return "<other> should only have one element"

                jobProperties[elm.nodeName] = getValue (elm)

                elm = nextNode (elm)

        if elm != None:
            return "Expecting no more tags in <DefaultJobProperties>"

        if elmSave != None:
            return "Expecting no more tags in <Device>"

        self.deviceInformation = DeviceInformation ((self,
                                                     driverName,
                                                     deviceName,
                                                     capabilities,
                                                     rasterCapabilities,
                                                     deviceOptions,
                                                     pdl,
                                                     jobProperties))

        return None

    def getFileName (self):
        return self.filename

    def getDeviceInformation (self):
        return self.deviceInformation

    def getDriverName (self):
        return self.deviceInformation.getDriverName ()

    def getDeviceName (self):
        return self.deviceInformation.getDeviceName ()

    def getCapabilities (self):
        return self.deviceInformation.getCapabilities ()

    def getRasterCapabilities (self):
        return self.deviceInformation.getRasterCapabilities ()

    def getDeviceOptions (self):
        return self.deviceInformation.getDeviceOptions ()

    def getPDL (self):
        return self.deviceInformation.getPDL ()

    def getDefaultJobProperties (self):
        return self.deviceInformation.getDefaultJobProperties ()

    def setDefaultJobProperties (self, defaultJobProperties):
        self.deviceInformation.setDefaultJobProperties (defaultJobProperties)

    def setDefaultJobProperty (self, defaultJobPropertyKey, defaultJobPropertyValue):
        self.deviceInformation.setDefaultJobProperty (defaultJobPropertyKey, defaultJobPropertyValue)

    def deleteDefaultJobProperty (self, defaultJobPropertyKey):
        self.deviceInformation.deleteDefaultJobProperty (defaultJobPropertyKey)

    def getDeviceCommands (self):
        return self.deviceCommands

    def setDeviceCommands (self, deviceCommands):
        self.deviceCommands = deviceCommands

    def getDeviceConnections (self):
        return self.deviceConnections

    def setDeviceConnections (self, deviceConnections):
        self.deviceConnections = deviceConnections

    def getDeviceCopies (self):
        return self.deviceCopies

    def setDeviceCopies (self, deviceCopies):
        self.deviceCopies = deviceCopies

    def getDeviceDatas (self):
        return self.deviceDatas

    def setDeviceDatas (self, deviceDatas):
        self.deviceDatas = deviceDatas

    def getDeviceForms (self):
        return self.deviceForms

    def setDeviceForms (self, deviceForms):
        self.deviceForms = deviceForms

    def getDeviceGammaTables (self):
        return self.deviceGammaTables

    def setDeviceGammaTables (self, deviceGammaTables):
        self.deviceGammaTables = deviceGammaTables

    def getDeviceMedias (self):
        return self.deviceMedias

    def setDeviceMedias (self, deviceMedias):
        self.deviceMedias = deviceMedias

    def getDeviceNumberUps (self):
        return self.deviceNumberUps

    def setDeviceNumberUps (self, deviceNumberUps):
        self.deviceNumberUps = deviceNumberUps

    def getDeviceOrientations (self):
        return self.deviceOrientations

    def setDeviceOrientations (self, deviceOrientations):
        self.deviceOrientations = deviceOrientations

    def getDeviceOutputBins (self):
        return self.deviceOutputBins

    def setDeviceOutputBins (self, deviceOutputBins):
        self.deviceOutputBins = deviceOutputBins

    def getDevicePrintModes (self):
        return self.devicePrintModes

    def setDevicePrintModes (self, devicePrintModes):
        self.devicePrintModes = devicePrintModes

    def getDeviceResolutions (self):
        return self.deviceResolutions

    def setDeviceResolutions (self, deviceResolutions):
        self.deviceResolutions = deviceResolutions

    def getDeviceScalings (self):
        return self.deviceScalings

    def setDeviceScalings (self, deviceScalings):
        self.deviceScalings = deviceScalings

    def getDeviceSheetCollates (self):
        return self.deviceSheetCollates

    def setDeviceSheetCollates (self, deviceSheetCollates):
        self.deviceSheetCollates = deviceSheetCollates

    def getDeviceSides (self):
        return self.deviceSides

    def setDeviceSides (self, deviceSides):
        self.deviceSides = deviceSides

    def getDeviceStitchings (self):
        return self.deviceStitchings

    def setDeviceStitchings (self, deviceStitchings):
        self.deviceStitchings = deviceStitchings

    def getDeviceStrings (self):
        return self.deviceStrings

    def setDeviceStrings (self, deviceStrings):
        self.deviceStrings = deviceStrings

    def getDeviceTrays (self):
        return self.deviceTrays

    def setDeviceTrays (self, deviceTrays):
        self.deviceTrays = deviceTrays

    def getDeviceTrimmings (self):
        return self.deviceTrimmings

    def setDeviceTrimmings (self, deviceTrimmings):
        self.deviceTrimmings = deviceTrimmings

    def getDeviceFeatures (self):
        return (self.deviceCommands,
                self.deviceConnections,
                self.deviceCopies,
                self.deviceDatas,
                self.deviceForms,
                self.deviceGammaTables,
                self.deviceMedias,
                self.deviceNumberUps,
                self.deviceOrientations,
                self.deviceOutputBins,
                self.devicePrintModes,
                self.deviceResolutions,
                self.deviceScalings,
                self.deviceSheetCollates,
                self.deviceSides,
                self.deviceStitchings,
                self.deviceStrings,
                self.deviceTrays,
                self.deviceTrimmings
               )

    def printSelf (self):
        print "[",
        # @TBD
        print "]"

    def getBaseFilename (self, fileName):
        (rootFileName, baseFileName) = os.path.split (fileName)

        return baseFileName

    def toXML (self):
        xmlData = XMLHeader () \
                + """
<Device
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni"
"""

        xmlData += '   name="' + self.getDeviceName () + '">\n'
        xmlData += '   <DriverName>' + self.getDriverName () + '</DriverName>\n'
        for capability in self.getCapabilities ():
            xmlData += '   <Capability type="' + capability + '"/>\n'
        for rasterCapability in self.getRasterCapabilities ():
            xmlData += '   <RasterCapabilities type="' + rasterCapability + '"/>\n'
        for deviceOption in self.getDeviceOptions ():
            xmlData += '   <DeviceOptions type="' + deviceOption + '"/>\n'
        if self.getPDL () != None:
            pdl = self.getPDL ()
            xmlData += '   <PDL level="' \
                     + str (pdl.getLevel ()) \
                     + '" sublevel="' \
                     + str (pdl.getSubLevel ()) \
                     + '" major="' \
                     + str (pdl.getMajor ()) \
                     + '" minor="' \
                     + str (pdl.getMinor ()) \
                     + '"/>\n'
        else:
            if shouldOutputDebug ("Driver::toXML"):
                print "Driver::toXML: Error: Missing PDL"
            xmlData += '   <PDL level="PDL_DONTCARE" sublevel="PDL_DONTCARE" major="PDL_DONTCARE" minor="PDL_DONTCARE"/>\n'
        if self.deviceCommands != None:
            xmlData += '   <Uses>' + self.getBaseFilename (self.deviceCommands.getFileName ()) + '</Uses>\n'
        if self.deviceConnections != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceConnections.getFileName ()) + '</Has>\n'
        if self.deviceCopies != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceCopies.getFileName ()) + '</Has>\n'
        if self.deviceDatas != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceDatas.getFileName ()) + '</Has>\n'
        if self.deviceForms != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceForms.getFileName ()) + '</Has>\n'
        if self.deviceGammaTables != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceGammaTables.getFileName ()) + '</Has>\n'
        if self.deviceMedias != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceMedias.getFileName ()) + '</Has>\n'
        if self.deviceNumberUps != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceNumberUps.getFileName ()) + '</Has>\n'
        if self.deviceOrientations != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceOrientations.getFileName ()) + '</Has>\n'
        if self.deviceOutputBins != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceOutputBins.getFileName ()) + '</Has>\n'
        if self.devicePrintModes != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.devicePrintModes.getFileName ()) + '</Has>\n'
        if self.deviceResolutions != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceResolutions.getFileName ()) + '</Has>\n'
        if self.deviceScalings != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceScalings.getFileName ()) + '</Has>\n'
        if self.deviceSheetCollates != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceSheetCollates.getFileName ()) + '</Has>\n'
        if self.deviceSides != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceSides.getFileName ()) + '</Has>\n'
        if self.deviceStitchings != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceStitchings.getFileName ()) + '</Has>\n'
        if self.deviceStrings != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceStrings.getFileName ()) + '</Has>\n'
        if self.deviceTrays != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceTrays.getFileName ()) + '</Has>\n'
        if self.deviceTrimmings != None:
            xmlData += '   <Has>' + self.getBaseFilename (self.deviceTrimmings.getFileName ()) + '</Has>\n'
        if self.instance1 != None:
            xmlData += '   <Instance>' + self.getBaseFilename (self.instance1) + '</Instance>\n'
        if self.instance2 != None:
            xmlData += '   <Instance>' + self.getBaseFilename (self.instance2) + '</Instance>\n'
        if self.blitter1 != None:
            xmlData += '   <Blitter>' + self.getBaseFilename (self.blitter1) + '</Blitter>\n'
        if self.blitter2 != None:
            xmlData += '   <Blitter>' + self.getBaseFilename (self.blitter2) + '</Blitter>\n'
        if self.pluggableExename != None:
            xmlData += '   <PluggableInstance exename="' + self.pluggableExename + '"/>\n'
            xmlData += '   <PluggableBlitter/>\n'
        xmlData += '   <DefaultJobProperties>\n'
        jobProperties = self.getDefaultJobProperties ()
        if jobProperties.has_key ("Copies"):
            xmlData += '      <Copies>' + jobProperties["Copies"] + '</Copies>\n'
        if jobProperties.has_key ("dither"):
            xmlData += '      <dither>' + jobProperties["dither"] + '</dither>\n'
        if jobProperties.has_key ("Form"):
            xmlData += '      <Form>' + jobProperties["Form"] + '</Form>\n'
        if jobProperties.has_key ("omniForm"):
            xmlData += '      <omniForm>' + jobProperties["omniForm"] + '</omniForm>\n'
        if jobProperties.has_key ("media"):
            xmlData += '      <media>' + jobProperties["media"] + '</media>\n'
        if     jobProperties.has_key ("NumberUp") \
           and jobProperties.has_key ("NumberUpDirection"):
            xmlData += '      <NumberUp>\n' \
                     + '         <NumberUp>' + jobProperties["NumberUp"] + '</NumberUp>\n' \
                     + '         <NumberUpDirection>' + jobProperties["NumberUpDirection"] + '</NumberUpDirection>\n' \
                     + '      </NumberUp>\n'
        if jobProperties.has_key ("Rotation"):
            xmlData += '      <Rotation>' + jobProperties["Rotation"] + '</Rotation>\n'
        if jobProperties.has_key ("omniOrientation"):
            xmlData += '      <omniOrientation>' + jobProperties["omniOrientation"] + '</omniOrientation>\n'
        if jobProperties.has_key ("OutputBin"):
            xmlData += '      <OutputBin>' + jobProperties["OutputBin"] + '</OutputBin>\n'
        if jobProperties.has_key ("printmode"):
            xmlData += '      <printmode>' + jobProperties["printmode"] + '</printmode>\n'
        if jobProperties.has_key ("Resolution"):
            xmlData += '      <Resolution>' + jobProperties["Resolution"] + '</Resolution>\n'
        if jobProperties.has_key ("omniResolution"):
            xmlData += '      <omniResolution>' + jobProperties["omniResolution"] + '</omniResolution>\n'
        if     jobProperties.has_key ("ScalingPercentage") \
           and jobProperties.has_key ("ScalingType"):
            xmlData += '      <Scaling>\n' \
                     + '         <ScalingPercentage>' + jobProperties["ScalingPercentage"] + '</ScalingPercentage>\n' \
                     + '         <ScalingType>' + jobProperties["ScalingType"] + '</ScalingType>\n' \
                     + '      </Scaling>\n'
        if jobProperties.has_key ("SheetCollate"):
            xmlData += '      <SheetCollate>' + jobProperties["SheetCollate"] + '</SheetCollate>\n'
        if jobProperties.has_key ("Sides"):
            xmlData += '      <Sides>' + jobProperties["Sides"] + '</Sides>\n'
        if     jobProperties.has_key ("StitchingPosition") \
           and jobProperties.has_key ("StitchingReferenceEdge") \
           and jobProperties.has_key ("StitchingType") \
           and jobProperties.has_key ("StitchingCount") \
           and jobProperties.has_key ("StitchingAngle"):
            xmlData += '      <Stitching>\n' \
                     + '         <StitchingPosition>' + jobProperties["StitchingPosition"] + '</StitchingPosition>\n' \
                     + '         <StitchingReferenceEdge>' + jobProperties["StitchingReferenceEdge"] + '</StitchingReferenceEdge>\n' \
                     + '         <StitchingType>' + jobProperties["StitchingType"] + '</StitchingType>\n' \
                     + '         <StitchingCount>' + jobProperties["StitchingCount"] + '</StitchingCount>\n' \
                     + '         <StitchingAngle>' + jobProperties["StitchingAngle"] + '</StitchingAngle>\n' \
                     + '      </Stitching>\n'
        if jobProperties.has_key ("InputTray"):
            xmlData += '      <InputTray>' + jobProperties["InputTray"] + '</InputTray>\n'
        if jobProperties.has_key ("omniTray"):
            xmlData += '      <omniTray>' + jobProperties["omniTray"] + '</omniTray>\n'
        if jobProperties.has_key ("Trimming"):
            xmlData += '      <Trimming>' + jobProperties["Trimming"] + '</Trimming>\n'
        xmlData += '   </DefaultJobProperties>\n'
        xmlData += '</Device>\n'

        return xmlData

    def addDeviceDialog (self, dialog):
        self.dialog = dialog

    def getDeviceDialog (self):
        return self.dialog

class OmniEditDeviceDialog:
    def __init__ (self, driver):

        deviceInformation   = driver.getDeviceInformation ()
        deviceCommands      = driver.getDeviceCommands ()
        deviceConnections   = driver.getDeviceConnections ()
        deviceCopies        = driver.getDeviceCopies ()
        deviceDatas         = driver.getDeviceDatas ()
        deviceForms         = driver.getDeviceForms ()
        deviceGammaTables   = driver.getDeviceGammaTables ()
        deviceMedias        = driver.getDeviceMedias ()
        deviceNUps          = driver.getDeviceNumberUps ()
        deviceOrientations  = driver.getDeviceOrientations ()
        deviceOutputBins    = driver.getDeviceOutputBins ()
        devicePrintModes    = driver.getDevicePrintModes ()
        deviceResolutions   = driver.getDeviceResolutions ()
        deviceScalings      = driver.getDeviceScalings ()
        deviceSheetCollates = driver.getDeviceSheetCollates ()
        deviceSides         = driver.getDeviceSides ()
        deviceStitchings    = driver.getDeviceStitchings ()
        deviceStrings       = driver.getDeviceStrings ()
        deviceTrays         = driver.getDeviceTrays ()
        deviceTrimmings     = driver.getDeviceTrimmings ()

        driver.addDeviceDialog (self)

        filename  = findDataFile ("device.glade")
        xml       = gtk.glade.XML (filename)
        window    = xml.get_widget ('DeviceWindow')
        frame     = xml.get_widget ('Frame')
        treeview  = xml.get_widget ('TreeView')
        treestore = gtk.TreeStore (gobject.TYPE_PYOBJECT,
                                   gobject.TYPE_PYOBJECT,
                                   gobject.TYPE_PYOBJECT,
                                   gobject.TYPE_STRING,
                                   gobject.TYPE_STRING)
########import gtk
########import gobject
########treestore = gtk.TreeStore (gobject.TYPE_STRING)
########iter = treestore.insert_after (None, None)
########treestore.set_value (iter, 0, "Bob")
########iter = treestore.insert_after (None, iter)
########treestore.set_value (iter, 0, "Bill")

        self.driver    = driver
        self.xml       = xml
        self.window    = window
        self.frame     = frame
        self.treeview  = treeview
        self.treestore = treestore
        self.fModified = False

        if driver.isImaginary ():
            self.fModified = True

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
                "on_ButtonFileNameCancel_clicked":                         self.on_ButtonFileNameCancel_clicked,
                "on_ButtonFileNameOk_clicked":                             self.on_ButtonFileNameOk_clicked,
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
#       if shouldOutputDebug ("OmniEditDeviceDialog::__init__"):
#           print "OmniEditDeviceDialog::__init__: dic =", dic
        xml.signal_autoconnect (dic)

        treeview.set_model (treestore)
        treeview.set_headers_visible (gtk.FALSE)

#       treestore.append (None, (deviceCopies, "DeviceCopies", deviceCopies.getFileName ()))
#       treestore.append (None, (deviceForms,  "DeviceForms",  deviceForms.getFileName ()))

        iter = treestore.insert_after (None, None)

        treestore.set_value (iter, 0, deviceInformation)
        treestore.set_value (iter, 1, deviceInformation)
        treestore.set_value (iter, 2, None)
        treestore.set_value (iter, 3, "deviceInformation")
        treestore.set_value (iter, 4, driver.getFileName ())

        #
        # Insert the OmniEditDeviceCommands object
        #
        if deviceCommands != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceCommands)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceCommands")
            treestore.set_value (iter, 4, deviceCommands.getFileName ())

            #
            # Insert the OmniEditDeviceCommand children objects
            #
            iterParent = iter
            iterCommands = None
            for command in deviceCommands.getCommands ():
                iterCommands = treestore.insert_after (iterParent, iterCommands)

                commandName = command.getName ()
                treestore.set_value (iterCommands, 0, deviceCommands)
                treestore.set_value (iterCommands, 1, command)
                treestore.set_value (iterCommands, 2, None)
                treestore.set_value (iterCommands, 3, commandName)
                treestore.set_value (iterCommands, 4, "")

        #
        # Insert the OmniEditDeviceConnections object
        #
        if deviceConnections != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceConnections)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceConnections")
            treestore.set_value (iter, 4, deviceConnections.getFileName ())

            #
            # Insert the OmniEditDeviceConnection children objects
            #
            iterParent = iter
            iterConnections = None
            for connection in deviceConnections.getConnections ():
                iterConnections = treestore.insert_after (iterParent, iterConnections)

                connectionName = connection.getName ()
                treestore.set_value (iterConnections, 0, deviceConnections)
                treestore.set_value (iterConnections, 1, connection)
                treestore.set_value (iterConnections, 2, None)
                treestore.set_value (iterConnections, 3, connectionName)
                treestore.set_value (iterConnections, 4, "")

        #
        # Insert the OmniEditDeviceCopies object
        #
        if deviceCopies != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceCopies)
            treestore.set_value (iter, 1, deviceCopies.getCopies ())
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceCopies")
            treestore.set_value (iter, 4, deviceCopies.getFileName ())

        #
        # Insert the OmniEditDeviceDatas object
        #
        if deviceDatas != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceDatas)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceDatas")
            treestore.set_value (iter, 4, deviceDatas.getFileName ())

            #
            # Insert the OmniEditDeviceData children objects
            #
            iterParent = iter
            iterDatas = None
            for data in deviceDatas.getDatas ():
                iterDatas = treestore.insert_after (iterParent, iterDatas)

                dataName = data.getName ()
                treestore.set_value (iterDatas, 0, deviceDatas)
                treestore.set_value (iterDatas, 1, data)
                treestore.set_value (iterDatas, 2, None)
                treestore.set_value (iterDatas, 3, dataName)
                treestore.set_value (iterDatas, 4, "")

        #
        # Insert the OmniEditDeviceForms object
        #
        if deviceForms != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceForms)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceForms")
            treestore.set_value (iter, 4, deviceForms.getFileName ())

            #
            # Insert the OmniEditDeviceForm children objects
            #
            iterParent = iter
            iterForms = None
            for form in deviceForms.getForms ():
                iterForms = treestore.insert_after (iterParent, iterForms)

                formName = form.getName ()
                treestore.set_value (iterForms, 0, deviceForms)
                treestore.set_value (iterForms, 1, form)
                treestore.set_value (iterForms, 2, None)
                treestore.set_value (iterForms, 3, formName)
                treestore.set_value (iterForms, 4, "")

        #
        # Insert the OmniEditDeviceGammaTables object
        #
        if deviceGammaTables != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceGammaTables)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceGammaTables")
            treestore.set_value (iter, 4, deviceGammaTables.getFileName ())

            #
            # Insert the OmniEditDeviceGammaTable children objects
            #
            iterParent = iter
            iterGammaTables = None
            for gamma in deviceGammaTables.getGammaTables ():
                iterGammaTables = treestore.insert_after (iterParent, iterGammaTables)

                gammaName = "%s %s %s %s" % (gamma.getResolution (), gamma.getMedia (), gamma.getPrintMode (), gamma.getDitherCatagory ())
                treestore.set_value (iterGammaTables, 0, deviceGammaTables)
                treestore.set_value (iterGammaTables, 1, gamma)
                treestore.set_value (iterGammaTables, 2, None)
                treestore.set_value (iterGammaTables, 3, gammaName)
                treestore.set_value (iterGammaTables, 4, "")

        #
        # Insert the OmniEditDeviceMedias object
        #
        if deviceMedias != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceMedias)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceMedias")
            treestore.set_value (iter, 4, deviceMedias.getFileName ())

            #
            # Insert the OmniEditDeviceMedia children objects
            #
            iterParent = iter
            iterMedias = None
            for media in deviceMedias.getMedias ():
                iterMedias = treestore.insert_after (iterParent, iterMedias)

                mediaName = media.getName ()
                treestore.set_value (iterMedias, 0, deviceMedias)
                treestore.set_value (iterMedias, 1, media)
                treestore.set_value (iterMedias, 2, None)
                treestore.set_value (iterMedias, 3, mediaName)
                treestore.set_value (iterMedias, 4, "")

        #
        # Insert the OmniEditDeviceNUps object
        #
        if deviceNUps != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceNUps)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceNUps")
            treestore.set_value (iter, 4, deviceNUps.getFileName ())

            #
            # Insert the OmniEditDeviceNUp children objects
            #
            iterParent = iter
            iterNUps = None
            for nup in deviceNUps.getNumberUps ():
                iterNUps = treestore.insert_after (iterParent, iterNUps)

                nupName = "%dx%d %s" % (nup.getX (), nup.getY (), nup.getNumberUpDirection ())
                treestore.set_value (iterNUps, 0, deviceNUps)
                treestore.set_value (iterNUps, 1, nup)
                treestore.set_value (iterNUps, 2, None)
                treestore.set_value (iterNUps, 3, nupName)
                treestore.set_value (iterNUps, 4, "")

        #
        # Insert the OmniEditDeviceOrientations object
        #
        if deviceOrientations != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceOrientations)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceOrientations")
            treestore.set_value (iter, 4, deviceOrientations.getFileName ())

            #
            # Insert the OmniEditDeviceOrientation children objects
            #
            iterParent = iter
            iterOrientations = None
            for orientation in deviceOrientations.getOrientations ():
                iterOrientations = treestore.insert_after (iterParent, iterOrientations)

                orientationName = orientation.getName ()
                treestore.set_value (iterOrientations, 0, deviceOrientations)
                treestore.set_value (iterOrientations, 1, orientation)
                treestore.set_value (iterOrientations, 2, None)
                treestore.set_value (iterOrientations, 3, orientationName)
                treestore.set_value (iterOrientations, 4, "")

        #
        # Insert the OmniEditDeviceOutputBins object
        #
        if deviceOutputBins != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceOutputBins)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceOutputBins")
            treestore.set_value (iter, 4, deviceOutputBins.getFileName ())

            #
            # Insert the OmniEditDeviceOutputBin children objects
            #
            iterParent = iter
            iterOutputBins = None
            for outputbin in deviceOutputBins.getOutputBins ():
                iterOutputBins = treestore.insert_after (iterParent, iterOutputBins)

                outputbinName = outputbin.getName ()
                treestore.set_value (iterOutputBins, 0, deviceOutputBins)
                treestore.set_value (iterOutputBins, 1, outputbin)
                treestore.set_value (iterOutputBins, 2, None)
                treestore.set_value (iterOutputBins, 3, outputbinName)
                treestore.set_value (iterOutputBins, 4, "")

        #
        # Insert the OmniEditDevicePrintModes object
        #
        if devicePrintModes != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, devicePrintModes)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DevicePrintModes")
            treestore.set_value (iter, 4, devicePrintModes.getFileName ())

            #
            # Insert the OmniEditDevicePrintMode children objects
            #
            iterParent = iter
            iterPrintModes = None
            for printmode in devicePrintModes.getPrintModes ():
                iterPrintModes = treestore.insert_after (iterParent, iterPrintModes)

                printmodeName = printmode.getName ()
                treestore.set_value (iterPrintModes, 0, devicePrintModes)
                treestore.set_value (iterPrintModes, 1, printmode)
                treestore.set_value (iterPrintModes, 2, None)
                treestore.set_value (iterPrintModes, 3, printmodeName)
                treestore.set_value (iterPrintModes, 4, "")

        #
        # Insert the OmniEditDeviceResolutions object
        #
        if deviceResolutions != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceResolutions)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceResolutions")
            treestore.set_value (iter, 4, deviceResolutions.getFileName ())

            #
            # Insert the OmniEditDeviceResolution children objects
            #
            iterParent = iter
            iterResolutions = None
            for resolution in deviceResolutions.getResolutions ():
                iterResolutions = treestore.insert_after (iterParent, iterResolutions)

                resolutionName = resolution.getName ()
                treestore.set_value (iterResolutions, 0, deviceResolutions)
                treestore.set_value (iterResolutions, 1, resolution)
                treestore.set_value (iterResolutions, 2, None)
                treestore.set_value (iterResolutions, 3, resolutionName)
                treestore.set_value (iterResolutions, 4, "")

        #
        # Insert the OmniEditDeviceScalings object
        #
        if deviceScalings != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceScalings)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceScalings")
            treestore.set_value (iter, 4, deviceScalings.getFileName ())

            #
            # Insert the OmniEditDeviceScaling children objects
            #
            iterParent = iter
            iterScalings = None
            for scaling in deviceScalings.getScalings ():
                iterScalings = treestore.insert_after (iterParent, iterScalings)

                scalingName = "%s %d" % (scaling.getAllowedType (), scaling.getDefault ())
                treestore.set_value (iterScalings, 0, deviceScalings)
                treestore.set_value (iterScalings, 1, scaling)
                treestore.set_value (iterScalings, 2, None)
                treestore.set_value (iterScalings, 3, scalingName)
                treestore.set_value (iterScalings, 4, "")

        #
        # Insert the OmniEditDeviceSheetCollates object
        #
        if deviceSheetCollates != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceSheetCollates)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceSheetCollates")
            treestore.set_value (iter, 4, deviceSheetCollates.getFileName ())

            #
            # Insert the OmniEditDeviceSheetCollate children objects
            #
            iterParent = iter
            iterSheetCollates = None
            for sheetcollate in deviceSheetCollates.getSheetCollates ():
                iterSheetCollates = treestore.insert_after (iterParent, iterSheetCollates)

                sheetcollateName = sheetcollate.getName ()
                treestore.set_value (iterSheetCollates, 0, deviceSheetCollates)
                treestore.set_value (iterSheetCollates, 1, sheetcollate)
                treestore.set_value (iterSheetCollates, 2, None)
                treestore.set_value (iterSheetCollates, 3, sheetcollateName)
                treestore.set_value (iterSheetCollates, 4, "")

        #
        # Insert the OmniEditDeviceSides object
        #
        if deviceSides != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceSides)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceSides")
            treestore.set_value (iter, 4, deviceSides.getFileName ())

            #
            # Insert the OmniEditDeviceSide children objects
            #
            iterParent = iter
            iterSides = None
            for side in deviceSides.getSides ():
                iterSides = treestore.insert_after (iterParent, iterSides)

                sideName = side.getName ()
                treestore.set_value (iterSides, 0, deviceSides)
                treestore.set_value (iterSides, 1, side)
                treestore.set_value (iterSides, 2, None)
                treestore.set_value (iterSides, 3, sideName)
                treestore.set_value (iterSides, 4, "")

        #
        # Insert the OmniEditDeviceStitchings object
        #
        if deviceStitchings != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceStitchings)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceStitchings")
            treestore.set_value (iter, 4, deviceStitchings.getFileName ())

            #
            # Insert the OmniEditDeviceStitching children objects
            #
            iterParent = iter
            iterStitchings = None
            for stitching in deviceStitchings.getStitchings ():
                iterStitchings = treestore.insert_after (iterParent, iterStitchings)

                stitchingName = "%d %s %s %d %d" % (stitching.getPosition (), stitching.getReferenceEdge (), stitching.getType (), stitching.getCount (), stitching.getAngle ())
                treestore.set_value (iterStitchings, 0, deviceStitchings)
                treestore.set_value (iterStitchings, 1, stitching)
                treestore.set_value (iterStitchings, 2, None)
                treestore.set_value (iterStitchings, 3, stitchingName)
                treestore.set_value (iterStitchings, 4, "")

        #
        # Insert the OmniEditDeviceStrings object
        #
        if deviceStrings != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceStrings)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceStrings")
            treestore.set_value (iter, 4, deviceStrings.getFileName ())

            #
            # Insert the OmniEditDeviceString children objects
            #
            iterParent = iter
            iterStrings = None
            for string in deviceStrings.getStrings ():
                iterStrings = treestore.insert_after (iterParent, iterStrings)

                treestore.set_value (iterStrings, 0, deviceStrings)
                treestore.set_value (iterStrings, 1, string)
                treestore.set_value (iterStrings, 2, None)
                treestore.set_value (iterStrings, 3, string.getName ())
                treestore.set_value (iterStrings, 4, "")

        #
        # Insert the OmniEditDeviceTrays object
        #
        if deviceTrays != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceTrays)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceTrays")
            treestore.set_value (iter, 4, deviceTrays.getFileName ())

            #
            # Insert the OmniEditDeviceTray children objects
            #
            iterParent = iter
            iterTrays = None
            for tray in deviceTrays.getTrays ():
                iterTrays = treestore.insert_after (iterParent, iterTrays)

                trayName = tray.getName ()
                treestore.set_value (iterTrays, 0, deviceTrays)
                treestore.set_value (iterTrays, 1, tray)
                treestore.set_value (iterTrays, 2, None)
                treestore.set_value (iterTrays, 3, trayName)
                treestore.set_value (iterTrays, 4, "")

        #
        # Insert the OmniEditDeviceTrimmings object
        #
        if deviceTrimmings != None:
            iter = treestore.insert_after (None, iter)

            treestore.set_value (iter, 0, deviceTrimmings)
            treestore.set_value (iter, 1, None)
            treestore.set_value (iter, 2, None)
            treestore.set_value (iter, 3, "DeviceTrimmings")
            treestore.set_value (iter, 4, deviceTrimmings.getFileName ())

            #
            # Insert the OmniEditDeviceTrimming children objects
            #
            iterParent = iter
            iterTrimmings = None
            for trimming in deviceTrimmings.getTrimmings ():
                iterTrimmings = treestore.insert_after (iterParent, iterTrimmings)

                trimmingName = trimming.getName ()
                treestore.set_value (iterTrimmings, 0, deviceTrimmings)
                treestore.set_value (iterTrimmings, 1, trimming)
                treestore.set_value (iterTrimmings, 2, None)
                treestore.set_value (iterTrimmings, 3, trimmingName)
                treestore.set_value (iterTrimmings, 4, "")

        renderer = gtk.CellRendererText ()
        column1  = gtk.TreeViewColumn ("Feature", renderer, text = 3)
        treeview.append_column (column1)
        column2  = gtk.TreeViewColumn ("Filename", renderer, text = 4)
        treeview.append_column (column2)

        treeview.set_model (treestore)

        selection = treeview.get_selection ()
        selection.connect ('changed', self.on_TreeView_changed)

        window.connect ('delete_event', self.on_DeviceWindow_delete_event)
        window.connect ('destroy', self.on_DeviceWindow_destroy_event)
        window.show ()

    def getTreeStore (self):
        return self.treestore

    def getTreeView (self):
        return self.treeview

    def isSafeToExit (self):
        if self.fModified:
            return False

#       if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
#           print "OmniEditDeviceDialog::isSafeToExit:-------------------------------------------------------"
        fSafe = True
        for row in self.treestore:
            # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...

            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
                    #for item in childRow:
                    #    print item

                    deviceXs     = childRow[0]
                    deviceX      = childRow[1]
                    deviceDialog = childRow[2]
                    name         = childRow[3]
#                   if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
#                       print "OmniEditDeviceDialog::isSafeToExit: %s: {%s, %s, %s}" % (name, deviceXs, deviceX, deviceDialog)

                    if deviceDialog != None:
                        if deviceDialog.isModified ():
                            if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
                                print "OmniEditDeviceDialog::isSafeToExit: %s isModified = %s" % (name, deviceDialog.isModified ())

                            fSafe = False
            else:
                deviceXs     = row[0]
                deviceX      = row[1]
                deviceDialog = row[2]
                name         = row[3]
#               if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
#                   print "OmniEditDeviceDialog::isSafeToExit: %s: {%s, %s, %s}" % (name, deviceXs, deviceX, deviceDialog)

                if deviceDialog != None:
                    if deviceDialog.isModified ():
                        if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
                            print "OmniEditDeviceDialog::isSafeToExit: %s isModified = %s" % (name, deviceDialog.isModified ())

                        fSafe = False

#       if shouldOutputDebug ("OmniEditDeviceDialog::isSafeToExit"):
#           print "OmniEditDeviceDialog::isSafeToExit: -------------------------------------------------------"

        deviceXList = self.getListDeviceXs ()
        for deviceX in deviceXList:
            if deviceX.isModified ():
                fSafe = False

        return fSafe

    def getListDeviceXs (self):
        deviceXList = []

        deviceX = self.driver.getDeviceCommands ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceConnections ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceCopies ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceDatas ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceForms ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceGammaTables ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceMedias ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceNumberUps ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceOrientations ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceOutputBins ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDevicePrintModes ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceResolutions ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceScalings ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceSheetCollates ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceSides ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceStitchings ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceStrings ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceTrays ()
        if deviceX != None:
            deviceXList.append (deviceX)
        deviceX = self.driver.getDeviceTrimmings ()
        if deviceX != None:
            deviceXList.append (deviceX)

        return deviceXList

    def on_TreeView_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
            print "OmniEditDeviceDialog::on_TreeView_changed:", widget

        (model, iter) = widget.get_selected ()
        if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
            print "OmniEditDeviceDialog::on_TreeView_changed: model =", model
        if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
            print "OmniEditDeviceDialog::on_TreeView_changed: iter =", iter

        if iter != None:
            row = self.treestore[iter]
            if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
                print "OmniEditDeviceDialog::on_TreeView_changed: row =", row
            if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
                print "OmniEditDeviceDialog::on_TreeView_changed: row[0] =", row[0]

            # Create a new dialog class if needed
            if row[2] == None:
                dialogClass = row[0].getDialog (row[1])
                if dialogClass == None:
                    print "OmniEditDeviceDialog::on_TreeView_changed: Error: getDialog failed"

                row[2] = dialogClass

            dialogClass  = row[2]
            dialogWindow = dialogClass.getWindow ()
            dialogWindow.show ()

            if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
                print "OmniEditDeviceDialog::on_TreeView_changed: dialogClass =", dialogClass
            if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
                print "OmniEditDeviceDialog::on_TreeView_changed: dialogWindow =", dialogWindow

            child = self.frame.get_child ()
            if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
                print "OmniEditDeviceDialog::on_TreeView_changed: child =", child

            if child != None:
                self.frame.remove (child)
            self.frame.add (dialogWindow)

#       if iter != None:
#           row = self.treestore[iter]
#           if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
#               print "OmniEditDeviceDialog::on_TreeView_changed: row =", row
#           if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
#               print "OmniEditDeviceDialog::on_TreeView_changed: row[0] =", row[0]
#
#           dialogClass  = row[0].getDialog (row[1])
#           dialogWindow = dialogClass.getWindow ()
#           dialogWindow.show ()
#           if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
#               print "OmniEditDeviceDialog::on_TreeView_changed: dialogClass =", dialogClass
#           if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
#               print "OmniEditDeviceDialog::on_TreeView_changed: dialogWindow =", dialogWindow
#
#           child = self.frame.get_child ()
#           if shouldOutputDebug ("OmniEditDeviceDialog::on_TreeView_changed"):
#               print "OmniEditDeviceDialog::on_TreeView_changed: child =", child
#
#           if child != None:
#               self.frame.remove (child)
#           self.frame.add (dialogWindow)
#           row[2] = dialogClass

    def on_MenuAbout_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuAbout_activate"):
            print "OmniEditDeviceDialog::on_MenuAbout_activate:", window

    def isValidXMLFilename (self, pszFileName):
        # @TBD
        return True

    def on_MenuCommands_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
            print "OmniEditDeviceDialog::on_MenuCommands_activate:", window

        if self.driver.getDeviceCommands ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Commands defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Commands XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceCommands.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceCommands = OmniEditDeviceCommands.OmniEditDeviceCommands (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: deviceCommands =", deviceCommands

                deviceCommand = deviceCommands.getCommands ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: deviceCommand =", deviceCommand

                deviceCommandsDialog = deviceCommands.getDialog (deviceCommand)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: deviceCommandsDialog =", deviceCommandsDialog

                self.driver.setDeviceCommands (deviceCommands)
                deviceCommands.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                        print "OmniEditDeviceDialog::on_MenuCommands_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceCommands":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                            print "OmniEditDeviceDialog::on_MenuCommands_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceCommands)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceCommands")
                self.treestore.set_value (iter, 4, deviceCommands.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceCommands)
                self.treestore.set_value (iter, 1, deviceCommand)
                self.treestore.set_value (iter, 2, deviceCommandsDialog)
                self.treestore.set_value (iter, 3, deviceCommand.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                        print "OmniEditDeviceDialog::on_MenuCommands_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCommands_activate"):
                    print "OmniEditDeviceDialog::on_MenuCommands_activate: -------------------------------------------------------"

                # Add <Uses>...Commands...</Uses>
                self.driver.setDeviceCommands (deviceCommands)

            ask.destroy ()

    def on_MenuConnections_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
            print "OmniEditDeviceDialog::on_MenuConnections_activate:", window

        if self.driver.getDeviceConnections ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Connections defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            pszError = None
            if self.driver.getDeviceForms () == None:
                pszError = "Please add forms before adding connections"
            elif self.driver.getDeviceTrays () == None:
                pszError = "Please add trays before adding connections"
            elif self.driver.getDeviceMedias () == None:
                pszError = "Please add medias before adding connections"
            elif not jobProperties.has_key ("Form"):
                pszError = "Please have a default form before adding connections"
            elif not jobProperties.has_key ("InputTray"):
                pszError = "Please have a default input tray before adding connections"
            elif not jobProperties.has_key ("media"):
                pszError = "Please have a default media before adding connections"

            if pszError != None:
                ask = gtk.MessageDialog (self.window,
                                         0,
                                         gtk.MESSAGE_QUESTION,
                                         gtk.BUTTONS_NONE,
                                         pszError)
                ask.add_button ("_Ok", gtk.RESPONSE_YES)
                response = ask.run ()
                ask.destroy ()
            else:
                jobProperties = self.driver.getDefaultJobProperties ()

                ask = gtk.FileSelection ("Enter the Connections XML file")
                ask.hide_fileop_buttons ()
                ask.set_select_multiple (gtk.FALSE)
                response = ask.run ()
                pszFileName = ask.get_filename ()

                if      response == gtk.RESPONSE_OK \
                   and  self.isValidXMLFilename (pszFileName):
                    pszXML = OmniEditDeviceConnections.getDefaultXML (jobProperties)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: -------------------------------------------------------"
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print pszXML
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: -------------------------------------------------------"

                    # create Reader object
                    reader = Sax2.Reader (False, True)

                    # parse the document
                    doc = reader.fromString (pszXML)

                    deviceConnections = OmniEditDeviceConnections.OmniEditDeviceConnections (pszFileName, doc.documentElement, self.driver)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: deviceConnections =", deviceConnections

                    deviceConnection = deviceConnections.getConnections ()[0]
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: deviceConnection =", deviceConnection

                    deviceConnectionsDialog = deviceConnections.getDialog (deviceConnection)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: deviceConnectionsDialog =", deviceConnectionsDialog

                    self.driver.setDeviceConnections (deviceConnections)
                    deviceConnections.setModified (True)
                    self.setModified (True)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: -------------------------------------------------------"

                    insertRow = None
                    for row in self.treestore:
                        # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                            print "OmniEditDeviceDialog::on_MenuConnections_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                        if row[3] < "DeviceConnections":
                            if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                                print "OmniEditDeviceDialog::on_MenuConnections_activate: Lessthan"
                            insertRow = row.iter

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: -------------------------------------------------------"

                    iter = self.treestore.insert_after (None, insertRow)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: iter =", iter

                    self.treestore.set_value (iter, 0, deviceConnections)
                    self.treestore.set_value (iter, 1, None)
                    self.treestore.set_value (iter, 2, None)
                    self.treestore.set_value (iter, 3, "DeviceConnections")
                    self.treestore.set_value (iter, 4, deviceConnections.getFileName ())

                    iter = self.treestore.insert_after (iter, None)

                    self.treestore.set_value (iter, 0, deviceConnections)
                    self.treestore.set_value (iter, 1, deviceConnection)
                    self.treestore.set_value (iter, 2, deviceConnectionsDialog)
                    self.treestore.set_value (iter, 3, deviceConnection.getName ())
                    self.treestore.set_value (iter, 4, "")

                    for row in self.treestore:
                        # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                            print "OmniEditDeviceDialog::on_MenuConnections_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuConnections_activate"):
                        print "OmniEditDeviceDialog::on_MenuConnections_activate: -------------------------------------------------------"

                    # Add <Has>...Connections...</Has>
                    self.driver.setDeviceConnections (deviceConnections)

                ask.destroy ()

    def on_MenuCopies_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
            print "OmniEditDeviceDialog::on_MenuCopies_activate:", window

        if self.driver.getDeviceCopies ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Copies defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Copies XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceCopies.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceCopies = OmniEditDeviceCopies.OmniEditDeviceCopies (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: deviceCopies =", deviceCopies

                deviceCopiesDialog = deviceCopies.getDialog (deviceCopies.getCopies ())
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: deviceCopiesDialog =", deviceCopiesDialog

                self.driver.setDeviceCopies (deviceCopies)
                deviceCopies.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                        print "OmniEditDeviceDialog::on_MenuCopies_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceCopies":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                            print "OmniEditDeviceDialog::on_MenuCopies_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceCopies)
                self.treestore.set_value (iter, 1, deviceCopies.getCopies ())
                self.treestore.set_value (iter, 2, deviceCopiesDialog)
                self.treestore.set_value (iter, 3, "DeviceCopies")
                self.treestore.set_value (iter, 4, deviceCopies.getFileName ())

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                        print "OmniEditDeviceDialog::on_MenuCopies_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuCopies_activate"):
                    print "OmniEditDeviceDialog::on_MenuCopies_activate: -------------------------------------------------------"

                # Add <Has>...Copies...</Has>
                self.driver.setDeviceCopies (deviceCopies)
                self.driver.setDefaultJobProperty ("Copies", "1")

            ask.destroy ()

    def on_MenuDatas_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
            print "OmniEditDeviceDialog::on_MenuDatas_activate:", window

        if self.driver.getDeviceDatas ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Datas defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Datas XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceDatas.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceDatas = OmniEditDeviceDatas.OmniEditDeviceDatas (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: deviceDatas =", deviceDatas

                deviceData = deviceDatas.getDatas ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: deviceData =", deviceData

                deviceDatasDialog = deviceDatas.getDialog (deviceData)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: deviceDatasDialog =", deviceDatasDialog

                self.driver.setDeviceDatas (deviceDatas)
                deviceDatas.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                        print "OmniEditDeviceDialog::on_MenuDatas_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceDatas":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                            print "OmniEditDeviceDialog::on_MenuDatas_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceDatas)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceDatas")
                self.treestore.set_value (iter, 4, deviceDatas.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceDatas)
                self.treestore.set_value (iter, 1, deviceData)
                self.treestore.set_value (iter, 2, deviceDatasDialog)
                self.treestore.set_value (iter, 3, deviceData.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                        print "OmniEditDeviceDialog::on_MenuDatas_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuDatas_activate"):
                    print "OmniEditDeviceDialog::on_MenuDatas_activate: -------------------------------------------------------"

                # Add <Has>...Datas...</Uses>
                self.driver.setDeviceDatas (deviceDatas)

            ask.destroy ()

    def on_MenuForms_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
            print "OmniEditDeviceDialog::on_MenuForms_activate:", window

        if self.driver.getDeviceForms ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Forms defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Forms XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceForms.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceForms = OmniEditDeviceForms.OmniEditDeviceForms (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: deviceForms =", deviceForms

                deviceForm = deviceForms.getForms ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: deviceForm =", deviceForm

                deviceFormsDialog = deviceForms.getDialog (deviceForm)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: deviceFormsDialog =", deviceFormsDialog

                self.driver.setDeviceForms (deviceForms)
                deviceForms.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                        print "OmniEditDeviceDialog::on_MenuForms_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceForms":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                            print "OmniEditDeviceDialog::on_MenuForms_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceForms)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceForms")
                self.treestore.set_value (iter, 4, deviceForms.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceForms)
                self.treestore.set_value (iter, 1, deviceForm)
                self.treestore.set_value (iter, 2, deviceFormsDialog)
                self.treestore.set_value (iter, 3, deviceForm.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                        print "OmniEditDeviceDialog::on_MenuForms_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuForms_activate"):
                    print "OmniEditDeviceDialog::on_MenuForms_activate: -------------------------------------------------------"

                # Add <Has>...Forms...</Uses>
                self.driver.setDeviceForms (deviceForms)
                self.driver.setDefaultJobProperty ("Form", "na_letter_8.50x11.00in")

            ask.destroy ()

    def on_MenuGammaTables_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
            print "OmniEditDeviceDialog::on_MenuGammaTables_activate:", window

        if self.driver.getDeviceGammaTables ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have GammaTables defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            pszError = None
            if self.driver.getDeviceResolutions () == None:
                pszError = "Please add resolutions before adding gamma tables"
            elif self.driver.getDeviceMedias () == None:
                pszError = "Please add medias before adding gamma tables"
            elif self.driver.getDevicePrintModes () == None:
                pszError = "Please add print modes before adding gamma tables"
            elif not jobProperties.has_key ("Resolution"):
                pszError = "Please have a default resolution before adding connections"
            elif not jobProperties.has_key ("media"):
                pszError = "Please have a default media before adding connections"
            elif not jobProperties.has_key ("printmode"):
                pszError = "Please have a default print mode before adding connections"

            if pszError != None:
                ask = gtk.MessageDialog (self.window,
                                         0,
                                         gtk.MESSAGE_QUESTION,
                                         gtk.BUTTONS_NONE,
                                         pszError)
                ask.add_button ("_Ok", gtk.RESPONSE_YES)
                response = ask.run ()
                ask.destroy ()
            else:
                jobProperties = self.driver.getDefaultJobProperties ()

                # @TBD add dither catagories instead of hardcoding it

                ask = gtk.FileSelection ("Enter the GammaTables XML file")
                ask.hide_fileop_buttons ()
                ask.set_select_multiple (gtk.FALSE)
                response = ask.run ()
                pszFileName = ask.get_filename ()

                if      response == gtk.RESPONSE_OK \
                   and  self.isValidXMLFilename (pszFileName):
                    pszXML = OmniEditDeviceGammaTables.getDefaultXML (jobProperties)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: -------------------------------------------------------"
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print pszXML
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: -------------------------------------------------------"

                    # create Reader object
                    reader = Sax2.Reader (False, True)

                    # parse the document
                    doc = reader.fromString (pszXML)

                    deviceGammaTables = OmniEditDeviceGammaTables.OmniEditDeviceGammaTables (pszFileName, doc.documentElement, self.driver)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: deviceGammaTables =", deviceGammaTables

                    deviceGammaTable = deviceGammaTables.getGammaTables ()[0]
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: deviceGammaTable =", deviceGammaTable

                    deviceGammaTablesDialog = deviceGammaTables.getDialog (deviceGammaTable)
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: deviceGammaTablesDialog =", deviceGammaTablesDialog

                    self.driver.setDeviceGammaTables (deviceGammaTables)
                    deviceGammaTables.setModified (True)
                    self.setModified (True)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: -------------------------------------------------------"

                    insertRow = None
                    for row in self.treestore:
                        # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                            print "OmniEditDeviceDialog::on_MenuGammaTables_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                        if row[3] < "DeviceGammaTables":
                            if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                                print "OmniEditDeviceDialog::on_MenuGammaTables_activate: Lessthan"
                            insertRow = row.iter

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: -------------------------------------------------------"

                    iter = self.treestore.insert_after (None, insertRow)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: iter =", iter

                    self.treestore.set_value (iter, 0, deviceGammaTables)
                    self.treestore.set_value (iter, 1, None)
                    self.treestore.set_value (iter, 2, None)
                    self.treestore.set_value (iter, 3, "DeviceGammaTables")
                    self.treestore.set_value (iter, 4, deviceGammaTables.getFileName ())

                    iter = self.treestore.insert_after (iter, None)

                    gammaName = "%s %s %s %s" % (deviceGammaTable.getResolution (), deviceGammaTable.getMedia (), deviceGammaTable.getPrintMode (), deviceGammaTable.getDitherCatagory ())
                    self.treestore.set_value (iter, 0, deviceGammaTables)
                    self.treestore.set_value (iter, 1, deviceGammaTable)
                    self.treestore.set_value (iter, 2, deviceGammaTablesDialog)
                    self.treestore.set_value (iter, 3, gammaName)
                    self.treestore.set_value (iter, 4, "")

                    for row in self.treestore:
                        # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                            print "OmniEditDeviceDialog::on_MenuGammaTables_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuGammaTables_activate"):
                        print "OmniEditDeviceDialog::on_MenuGammaTables_activate: -------------------------------------------------------"

                    # Add <Has>...GammaTables...</Uses>
                    self.driver.setDeviceGammaTables (deviceGammaTables)

                ask.destroy ()

    def on_MenuMedias_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
            print "OmniEditDeviceDialog::on_MenuMedias_activate:", window

        if self.driver.getDeviceMedias ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Medias defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Medias XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceMedias.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceMedias = OmniEditDeviceMedias.OmniEditDeviceMedias (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: deviceMedias =", deviceMedias

                deviceMedia = deviceMedias.getMedias ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: deviceMedia =", deviceMedia

                deviceMediasDialog = deviceMedias.getDialog (deviceMedia)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: deviceMediasDialog =", deviceMediasDialog

                self.driver.setDeviceMedias (deviceMedias)
                deviceMedias.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                        print "OmniEditDeviceDialog::on_MenuMedias_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceMedias":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                            print "OmniEditDeviceDialog::on_MenuMedias_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceMedias)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceMedias")
                self.treestore.set_value (iter, 4, deviceMedias.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceMedias)
                self.treestore.set_value (iter, 1, deviceMedia)
                self.treestore.set_value (iter, 2, deviceMediasDialog)
                self.treestore.set_value (iter, 3, deviceMedia.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                        print "OmniEditDeviceDialog::on_MenuMedias_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuMedias_activate"):
                    print "OmniEditDeviceDialog::on_MenuMedias_activate: -------------------------------------------------------"

                # Add <Has>...Medias...</Uses>
                self.driver.setDeviceMedias (deviceMedias)
                self.driver.setDefaultJobProperty ("media", "MEDIA_PLAIN")

            ask.destroy ()

    def on_MenuNumberUps_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
            print "OmniEditDeviceDialog::on_MenuNumberUps_activate:", window

        if self.driver.getDeviceNumberUps ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have NumberUps defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the NumberUps XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceNumberUps.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceNumberUps = OmniEditDeviceNumberUps.OmniEditDeviceNumberUps (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: deviceNumberUps =", deviceNumberUps

                deviceNumberUp = deviceNumberUps.getNumberUps ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: deviceNumberUp =", deviceNumberUp

                deviceNumberUpsDialog = deviceNumberUps.getDialog (deviceNumberUp)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: deviceNumberUpsDialog =", deviceNumberUpsDialog

                self.driver.setDeviceNumberUps (deviceNumberUps)
                deviceNumberUps.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                        print "OmniEditDeviceDialog::on_MenuNumberUps_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceNumberUps":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                            print "OmniEditDeviceDialog::on_MenuNumberUps_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceNumberUps)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceNumberUps")
                self.treestore.set_value (iter, 4, deviceNumberUps.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                nupName = "%dx%d %s" % (deviceNumberUp.getX (), deviceNumberUp.getY (), deviceNumberUp.getNumberUpDirection ())
                self.treestore.set_value (iter, 0, deviceNumberUps)
                self.treestore.set_value (iter, 1, deviceNumberUp)
                self.treestore.set_value (iter, 2, deviceNumberUpsDialog)
                self.treestore.set_value (iter, 3, nupName)
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                        print "OmniEditDeviceDialog::on_MenuNumberUps_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuNumberUps_activate"):
                    print "OmniEditDeviceDialog::on_MenuNumberUps_activate: -------------------------------------------------------"

                # Add <Has>...NumberUps...</Uses>
                self.driver.setDeviceNumberUps (deviceNumberUps)
                self.driver.setDefaultJobProperty ("NumberUp", "1x1")
                self.driver.setDefaultJobProperty ("NumberUpDirection", "TorightTobottom")

            ask.destroy ()

    def on_MenuOrientations_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
            print "OmniEditDeviceDialog::on_MenuOrientations_activate:", window

        if self.driver.getDeviceOrientations ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Orientations defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Orientations XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceOrientations.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceOrientations = OmniEditDeviceOrientations.OmniEditDeviceOrientations (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: deviceOrientations =", deviceOrientations

                deviceOrientation = deviceOrientations.getOrientations ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: deviceOrientation =", deviceOrientation

                deviceOrientationsDialog = deviceOrientations.getDialog (deviceOrientation)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: deviceOrientationsDialog =", deviceOrientationsDialog

                self.driver.setDeviceOrientations (deviceOrientations)
                deviceOrientations.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                        print "OmniEditDeviceDialog::on_MenuOrientations_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceOrientations":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                            print "OmniEditDeviceDialog::on_MenuOrientations_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceOrientations)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceOrientations")
                self.treestore.set_value (iter, 4, deviceOrientations.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceOrientations)
                self.treestore.set_value (iter, 1, deviceOrientation)
                self.treestore.set_value (iter, 2, deviceOrientationsDialog)
                self.treestore.set_value (iter, 3, deviceOrientation.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                        print "OmniEditDeviceDialog::on_MenuOrientations_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOrientations_activate"):
                    print "OmniEditDeviceDialog::on_MenuOrientations_activate: -------------------------------------------------------"

                # Add <Has>...Orientations...</Uses>
                self.driver.setDeviceOrientations (deviceOrientations)
                self.driver.setDefaultJobProperty ("Rotation", "Portrait")

            ask.destroy ()

    def on_MenuOutputBins_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
            print "OmniEditDeviceDialog::on_MenuOutputBins_activate:", window

        if self.driver.getDeviceOutputBins ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have OutputBins defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the OutputBins XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceOutputBins.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceOutputBins = OmniEditDeviceOutputBins.OmniEditDeviceOutputBins (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: deviceOutputBins =", deviceOutputBins

                deviceOutputBin = deviceOutputBins.getOutputBins ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: deviceOutputBin =", deviceOutputBin

                deviceOutputBinsDialog = deviceOutputBins.getDialog (deviceOutputBin)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: deviceOutputBinsDialog =", deviceOutputBinsDialog

                self.driver.setDeviceOutputBins (deviceOutputBins)
                deviceOutputBins.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                        print "OmniEditDeviceDialog::on_MenuOutputBins_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceOutputBins":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                            print "OmniEditDeviceDialog::on_MenuOutputBins_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceOutputBins)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceOutputBins")
                self.treestore.set_value (iter, 4, deviceOutputBins.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceOutputBins)
                self.treestore.set_value (iter, 1, deviceOutputBin)
                self.treestore.set_value (iter, 2, deviceOutputBinsDialog)
                self.treestore.set_value (iter, 3, deviceOutputBin.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                        print "OmniEditDeviceDialog::on_MenuOutputBins_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuOutputBins_activate"):
                    print "OmniEditDeviceDialog::on_MenuOutputBins_activate: -------------------------------------------------------"

                # Add <Has>...OutputBins...</Uses>
                self.driver.setDeviceOutputBins (deviceOutputBins)
                self.driver.setDefaultJobProperty ("OutputBin", "Top")

            ask.destroy ()

    def on_MenuPrintModes_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
            print "OmniEditDeviceDialog::on_MenuPrintModes_activate:", window

        if self.driver.getDevicePrintModes ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have PrintModes defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the PrintModes XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDevicePrintModes.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                devicePrintModes = OmniEditDevicePrintModes.OmniEditDevicePrintModes (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: devicePrintModes =", devicePrintModes

                devicePrintMode = devicePrintModes.getPrintModes ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: devicePrintMode =", devicePrintMode

                devicePrintModesDialog = devicePrintModes.getDialog (devicePrintMode)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: devicePrintModesDialog =", devicePrintModesDialog

                self.driver.setDevicePrintModes (devicePrintModes)
                devicePrintModes.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                        print "OmniEditDeviceDialog::on_MenuPrintModes_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DevicePrintModes":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                            print "OmniEditDeviceDialog::on_MenuPrintModes_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: iter =", iter

                self.treestore.set_value (iter, 0, devicePrintModes)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DevicePrintModes")
                self.treestore.set_value (iter, 4, devicePrintModes.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, devicePrintModes)
                self.treestore.set_value (iter, 1, devicePrintMode)
                self.treestore.set_value (iter, 2, devicePrintModesDialog)
                self.treestore.set_value (iter, 3, devicePrintMode.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog:on_MenuPrintModes_activate"):
                        print "OmniEditDeviceDialog:%s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuPrintModes_activate"):
                    print "OmniEditDeviceDialog::on_MenuPrintModes_activate: -------------------------------------------------------"

                # Add <Has>...PrintModes...</Uses>
                self.driver.setDevicePrintModes (devicePrintModes)
                self.driver.setDefaultJobProperty ("printmode", "PRINT_MODE_1_ANY")

            ask.destroy ()

    def on_MenuResolutions_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
            print "OmniEditDeviceDialog::on_MenuResolutions_activate:", window

        if self.driver.getDeviceResolutions ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Resolutions defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Resolutions XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceResolutions.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceResolutions = OmniEditDeviceResolutions.OmniEditDeviceResolutions (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: deviceResolutions =", deviceResolutions

                deviceResolution = deviceResolutions.getResolutions ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: deviceResolution =", deviceResolution

                deviceResolutionsDialog = deviceResolutions.getDialog (deviceResolution)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: deviceResolutionsDialog =", deviceResolutionsDialog

                self.driver.setDeviceResolutions (deviceResolutions)
                deviceResolutions.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                        print "OmniEditDeviceDialog::on_MenuResolutions_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceResolutions":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                            print "OmniEditDeviceDialog::on_MenuResolutions_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceResolutions)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceResolutions")
                self.treestore.set_value (iter, 4, deviceResolutions.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceResolutions)
                self.treestore.set_value (iter, 1, deviceResolution)
                self.treestore.set_value (iter, 2, deviceResolutionsDialog)
                self.treestore.set_value (iter, 3, deviceResolution.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                        print "OmniEditDeviceDialog::on_MenuResolutions_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuResolutions_activate"):
                    print "OmniEditDeviceDialog::on_MenuResolutions_activate: -------------------------------------------------------"

                # Add <Has>...Resolutions...</Uses>
                self.driver.setDeviceResolutions (deviceResolutions)
                self.driver.setDefaultJobProperty ("Resolution", "180x180")

            ask.destroy ()

    def on_MenuScalings_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
            print "OmniEditDeviceDialog::on_MenuScalings_activate:", window

        if self.driver.getDeviceScalings ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Scalings defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Scalings XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceScalings.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceScalings = OmniEditDeviceScalings.OmniEditDeviceScalings (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: deviceScalings =", deviceScalings

                deviceScaling = deviceScalings.getScalings ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: deviceScaling =", deviceScaling

                deviceScalingsDialog = deviceScalings.getDialog (deviceScaling)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: deviceScalingsDialog =", deviceScalingsDialog

                self.driver.setDeviceScalings (deviceScalings)
                deviceScalings.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                        print "OmniEditDeviceDialog::on_MenuScalings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceScalings":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                            print "OmniEditDeviceDialog::on_MenuScalings_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceScalings)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceScalings")
                self.treestore.set_value (iter, 4, deviceScalings.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                scalingName = "%s %d" % (deviceScaling.getAllowedType (), deviceScaling.getDefault ())
                self.treestore.set_value (iter, 0, deviceScalings)
                self.treestore.set_value (iter, 1, deviceScaling)
                self.treestore.set_value (iter, 2, deviceScalingsDialog)
                self.treestore.set_value (iter, 3, scalingName)
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                        print "OmniEditDeviceDialog::on_MenuScalings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuScalings_activate"):
                    print "OmniEditDeviceDialog::on_MenuScalings_activate: -------------------------------------------------------"

                # Add <Has>...Scalings...</Uses>
                self.driver.setDeviceScalings (deviceScalings)
                self.driver.setDefaultJobProperty ("ScalingPercentage", "100")
                self.driver.setDefaultJobProperty ("ScalingType", "Clip")

            ask.destroy ()

    def on_MenuSheetCollates_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
            print "OmniEditDeviceDialog::on_MenuSheetCollates_activate:", window

        if self.driver.getDeviceSheetCollates ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have SheetCollates defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the SheetCollates XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceSheetCollates.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceSheetCollates = OmniEditDeviceSheetCollates.OmniEditDeviceSheetCollates (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: deviceSheetCollates =", deviceSheetCollates

                deviceSheetCollate = deviceSheetCollates.getSheetCollates ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: deviceSheetCollate =", deviceSheetCollate

                deviceSheetCollatesDialog = deviceSheetCollates.getDialog (deviceSheetCollate)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: deviceSheetCollatesDialog =", deviceSheetCollatesDialog

                self.driver.setDeviceSheetCollates (deviceSheetCollates)
                deviceSheetCollates.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                        print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceSheetCollates":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                            print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceSheetCollates)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceSheetCollates")
                self.treestore.set_value (iter, 4, deviceSheetCollates.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceSheetCollates)
                self.treestore.set_value (iter, 1, deviceSheetCollate)
                self.treestore.set_value (iter, 2, deviceSheetCollatesDialog)
                self.treestore.set_value (iter, 3, deviceSheetCollate.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                        print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSheetCollates_activate"):
                    print "OmniEditDeviceDialog::on_MenuSheetCollates_activate: -------------------------------------------------------"

                # Add <Has>...SheetCollates...</Uses>
                self.driver.setDeviceSheetCollates (deviceSheetCollates)
                self.driver.setDefaultJobProperty ("SheetCollate", "SheetUncollated")

            ask.destroy ()

    def on_MenuSides_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
            print "OmniEditDeviceDialog::on_MenuSides_activate:", window

        if self.driver.getDeviceSides ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Sides defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Sides XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceSides.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceSides = OmniEditDeviceSides.OmniEditDeviceSides (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: deviceSides =", deviceSides

                deviceSide = deviceSides.getSides ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: deviceSide =", deviceSide

                deviceSidesDialog = deviceSides.getDialog (deviceSide)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: deviceSidesDialog =", deviceSidesDialog

                self.driver.setDeviceSides (deviceSides)
                deviceSides.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                        print "OmniEditDeviceDialog::on_MenuSides_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceSides":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                            print "OmniEditDeviceDialog::on_MenuSides_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceSides)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceSides")
                self.treestore.set_value (iter, 4, deviceSides.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceSides)
                self.treestore.set_value (iter, 1, deviceSide)
                self.treestore.set_value (iter, 2, deviceSidesDialog)
                self.treestore.set_value (iter, 3, deviceSide.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                        print "OmniEditDeviceDialog::on_MenuSides_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuSides_activate"):
                    print "OmniEditDeviceDialog::on_MenuSides_activate: -------------------------------------------------------"

                # Add <Has>...Sides...</Uses>
                self.driver.setDeviceSides (deviceSides)
                self.driver.setDefaultJobProperty ("Sides", "OneSidedFront")

            ask.destroy ()

    def on_MenuStitchings_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
            print "OmniEditDeviceDialog::on_MenuStitchings_activate:", window

        if self.driver.getDeviceStitchings ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Stitchings defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Stitchings XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceStitchings.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceStitchings = OmniEditDeviceStitchings.OmniEditDeviceStitchings (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: deviceStitchings =", deviceStitchings

                deviceStitching = deviceStitchings.getStitchings ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: deviceStitching =", deviceStitching

                deviceStitchingsDialog = deviceStitchings.getDialog (deviceStitching)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: deviceStitchingsDialog =", deviceStitchingsDialog

                self.driver.setDeviceStitchings (deviceStitchings)
                deviceStitchings.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                        print "OmniEditDeviceDialog::on_MenuStitchings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceStitchings":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                            print "OmniEditDeviceDialog::on_MenuStitchings_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceStitchings)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceStitchings")
                self.treestore.set_value (iter, 4, deviceStitchings.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                stitchingName = "%d %s %s %d %d" % (deviceStitching.getPosition (), deviceStitching.getReferenceEdge (), deviceStitching.getType (), deviceStitching.getCount (), deviceStitching.getAngle ())
                self.treestore.set_value (iter, 0, deviceStitchings)
                self.treestore.set_value (iter, 1, deviceStitching)
                self.treestore.set_value (iter, 2, deviceStitchingsDialog)
                self.treestore.set_value (iter, 3, stitchingName)
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog:%s"):
                        print "OmniEditDeviceDialog:%s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStitchings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStitchings_activate: -------------------------------------------------------"

                # Add <Has>...Stitchings...</Uses>
                self.driver.setDeviceStitchings (deviceStitchings)
                self.driver.setDefaultJobProperty ("StitchingPosition", "1")
                self.driver.setDefaultJobProperty ("StitchingReferenceEdge", "Top")
                self.driver.setDefaultJobProperty ("StitchingType", "Corner")
                self.driver.setDefaultJobProperty ("StitchingCount", "1")
                self.driver.setDefaultJobProperty ("StitchingAngle", "0")

            ask.destroy ()

    def on_MenuStrings_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
            print "OmniEditDeviceDialog::on_MenuStrings_activate:", window

        if self.driver.getDeviceStrings ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Strings defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Strings XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceStrings.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceStrings = OmniEditDeviceStrings.OmniEditDeviceStrings (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceStrings =", deviceStrings

                deviceString = deviceStrings.getStrings ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceString =", deviceString

                deviceStringsDialog = deviceStrings.getDialog (deviceString)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceStringsDialog =", deviceStringsDialog

                self.driver.setDeviceStrings (deviceStrings)
                deviceStrings.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                        print "OmniEditDeviceDialog::on_MenuStrings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceStrings":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                            print "OmniEditDeviceDialog::on_MenuStrings_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceStrings)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceStrings")
                self.treestore.set_value (iter, 4, deviceStrings.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceStrings)
                self.treestore.set_value (iter, 1, deviceString)
                self.treestore.set_value (iter, 2, deviceStringsDialog)
                self.treestore.set_value (iter, 3, deviceString.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog:%s"):
                        print "OmniEditDeviceDialog:%s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                # Add <Has>...Strings...</Uses>
                self.driver.setDeviceStrings (deviceStrings)

            ask.destroy ()

    def on_MenuStrings_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
            print "OmniEditDeviceDialog::on_MenuStrings_activate:", window

        if self.driver.getDeviceStrings ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Strings defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Strings XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceStrings.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceStrings = OmniEditDeviceStrings.OmniEditDeviceStrings (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceStrings =", deviceStrings

                deviceString = deviceStrings.getStrings ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceString =", deviceString

                deviceStringsDialog = deviceStrings.getDialog (deviceString)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: deviceStringsDialog =", deviceStringsDialog

                self.driver.setDeviceStrings (deviceStrings)
                deviceStrings.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                        print "OmniEditDeviceDialog::on_MenuStrings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceStrings":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                            print "OmniEditDeviceDialog::on_MenuStrings_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceStrings)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceStrings")
                self.treestore.set_value (iter, 4, deviceStrings.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceStrings)
                self.treestore.set_value (iter, 1, deviceString)
                self.treestore.set_value (iter, 2, deviceStringsDialog)
                self.treestore.set_value (iter, 3, deviceString.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog:%s"):
                        print "OmniEditDeviceDialog:%s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuStrings_activate"):
                    print "OmniEditDeviceDialog::on_MenuStrings_activate: -------------------------------------------------------"

                # Add <Has>...Strings...</Uses>
                self.driver.setDeviceStrings (deviceStrings)

            ask.destroy ()

    def on_MenuTrays_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
            print "OmniEditDeviceDialog::on_MenuTrays_activate:", window

        if self.driver.getDeviceTrays ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Trays defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Trays XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceTrays.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceTrays = OmniEditDeviceTrays.OmniEditDeviceTrays (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: deviceTrays =", deviceTrays

                deviceTray = deviceTrays.getTrays ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: deviceTray =", deviceTray

                deviceTraysDialog = deviceTrays.getDialog (deviceTray)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: deviceTraysDialog =", deviceTraysDialog

                self.driver.setDeviceTrays (deviceTrays)
                deviceTrays.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                        print "OmniEditDeviceDialog::on_MenuTrays_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceTrays":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                            print "OmniEditDeviceDialog::on_MenuTrays_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceTrays)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceTrays")
                self.treestore.set_value (iter, 4, deviceTrays.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceTrays)
                self.treestore.set_value (iter, 1, deviceTray)
                self.treestore.set_value (iter, 2, deviceTraysDialog)
                self.treestore.set_value (iter, 3, deviceTray.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                        print "OmniEditDeviceDialog::on_MenuTrays_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrays_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrays_activate: -------------------------------------------------------"

                # Add <Has>...Trays...</Uses>
                self.driver.setDeviceTrays (deviceTrays)
                self.driver.setDefaultJobProperty ("InputTray", "AutoSelect")

            ask.destroy ()

    def on_MenuTrimmings_activate (self, window):
        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
            print "OmniEditDeviceDialog::on_MenuTrimmings_activate:", window

        if self.driver.getDeviceTrimmings ():
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Already have Trimmings defined for this device!")
            ask.add_button ("_Ok", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()
        else:
            jobProperties = self.driver.getDefaultJobProperties ()

            ask = gtk.FileSelection ("Enter the Trimmings XML file")
            ask.hide_fileop_buttons ()
            ask.set_select_multiple (gtk.FALSE)
            response = ask.run ()
            pszFileName = ask.get_filename ()

            if      response == gtk.RESPONSE_OK \
               and  self.isValidXMLFilename (pszFileName):
                pszXML = OmniEditDeviceTrimmings.getDefaultXML (jobProperties)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: -------------------------------------------------------"
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print pszXML
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: -------------------------------------------------------"

                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromString (pszXML)

                deviceTrimmings = OmniEditDeviceTrimmings.OmniEditDeviceTrimmings (pszFileName, doc.documentElement, self.driver)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: deviceTrimmings =", deviceTrimmings

                deviceTrimming = deviceTrimmings.getTrimmings ()[0]
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: deviceTrimming =", deviceTrimming

                deviceTrimmingsDialog = deviceTrimmings.getDialog (deviceTrimming)
                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: deviceTrimmingsDialog =", deviceTrimmingsDialog

                self.driver.setDeviceTrimmings (deviceTrimmings)
                deviceTrimmings.setModified (True)
                self.setModified (True)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: -------------------------------------------------------"

                insertRow = None
                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                        print "OmniEditDeviceDialog::on_MenuTrimmings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                    if row[3] < "DeviceTrimmings":
                        if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                            print "OmniEditDeviceDialog::on_MenuTrimmings_activate: Lessthan"
                        insertRow = row.iter

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: -------------------------------------------------------"

                iter = self.treestore.insert_after (None, insertRow)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: iter =", iter

                self.treestore.set_value (iter, 0, deviceTrimmings)
                self.treestore.set_value (iter, 1, None)
                self.treestore.set_value (iter, 2, None)
                self.treestore.set_value (iter, 3, "DeviceTrimmings")
                self.treestore.set_value (iter, 4, deviceTrimmings.getFileName ())

                iter = self.treestore.insert_after (iter, None)

                self.treestore.set_value (iter, 0, deviceTrimmings)
                self.treestore.set_value (iter, 1, deviceTrimming)
                self.treestore.set_value (iter, 2, deviceTrimmingsDialog)
                self.treestore.set_value (iter, 3, deviceTrimming.getName ())
                self.treestore.set_value (iter, 4, "")

                for row in self.treestore:
                    # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...
                    if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                        print "OmniEditDeviceDialog::on_MenuTrimmings_activate: %s: {%s, %s, %s} %s" % (row[3], row[0], row[1], row[2], row.iter)

                if shouldOutputDebug ("OmniEditDeviceDialog::on_MenuTrimmings_activate"):
                    print "OmniEditDeviceDialog::on_MenuTrimmings_activate: -------------------------------------------------------"

                # Add <Has>...Trimmings...</Uses>
                self.driver.setDeviceTrimmings (deviceTrimmings)
                self.driver.setDefaultJobProperty ("Trimming", "None")

            ask.destroy ()

    def on_DeviceWindow_delete_event (self, window, event):
        """Callback for the window being deleted."""
        if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
            print "OmniEditDeviceDialog::on_DeviceWindow_delete_event:", window, event

        fSafe = self.isSafeToExit ()
        if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
            print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: fSafe =", fSafe

        response = gtk.RESPONSE_NO
        if not fSafe:
            ask = gtk.MessageDialog (self.window,
                                     0,
                                     gtk.MESSAGE_QUESTION,
                                     gtk.BUTTONS_NONE,
                                     "Bob")
            ask.add_button ("_Don't save", gtk.RESPONSE_NO)
            ask.add_button ("_Cancel", gtk.RESPONSE_CANCEL)
            ask.add_button ("_Save", gtk.RESPONSE_YES)
            response = ask.run ()
            ask.destroy ()

        if response == gtk.RESPONSE_NO:
            if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
                print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: RESPONSE_NO"
        elif (  response == gtk.RESPONSE_CANCEL
             or response == gtk.RESPONSE_DELETE_EVENT
             ):
            if response == gtk.RESPONSE_CANCEL:
                if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
                    print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: RESPONSE_CANCEL"
            elif response == gtk.RESPONSE_DELETE_EVENT:
                if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
                    print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: RESPONSE_DELETE_EVENT"
            if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
                print "OmniEditDeviceDialog:returning gtk.TRUE"
            return gtk.TRUE
        elif response == gtk.RESPONSE_YES:
            if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
                print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: RESPONSE_YES"
            self.save ()

        if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_delete_event"):
            print "OmniEditDeviceDialog::on_DeviceWindow_delete_event: returning gtk.FALSE"
        return gtk.FALSE

    def on_DeviceWindow_destroy_event (self, *args):
        """Callback for the window being deleted."""
        if shouldOutputDebug ("OmniEditDeviceDialog::on_DeviceWindow_destroy_event"):
            print "OmniEditDeviceDialog::on_DeviceWindow_destroy_event:", args

        gtk.mainquit ()

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_ButtonFileNameCancel_clicked (self, window):                         print "OmniEditDeviceDialog::on_ButtonFileNameCancel_clicked: HACK"
    def on_ButtonFileNameOk_clicked (self, window):                             print "OmniEditDeviceDialog::on_ButtonFileNameOk_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceDialog::on_EntryTrimmingName_changed: HACK"

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
#       if shouldOutputDebug ("OmniEditDeviceDialog::save"):
#           print "OmniEditDeviceDialog::save: -------------------------------------------------------"
        for row in self.treestore:
            # ... 'iter', 'iterchildren', 'model', 'next', 'parent', 'path' ...

            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
                    #for item in childRow:
                    #    print item

                    deviceXs     = childRow[0]
                    deviceX      = childRow[1]
                    deviceDialog = childRow[2]
                    name         = childRow[3]
#                   if shouldOutputDebug ("OmniEditDeviceDialog::save"):
#                       print "OmniEditDeviceDialog::save: %s: {%s, %s, %s}" % (name, deviceXs, deviceX, deviceDialog)

                    if deviceDialog != None:
                        if deviceDialog.isModified ():
                            deviceDialog.save ()
            else:
                deviceXs     = row[0]
                deviceX      = row[1]
                deviceDialog = row[2]
                name         = row[3]
#               if shouldOutputDebug ("OmniEditDeviceDialog::save"):
#                   print "OmniEditDeviceDialog::save: %s: {%s, %s, %s}" % (name, deviceXs, deviceX, deviceDialog)

                if deviceDialog != None:
                    if deviceDialog.isModified ():
                        deviceDialog.save ()

#       if shouldOutputDebug ("OmniEditDeviceDialog::save"):
#           print "OmniEditDeviceDialog::save: -------------------------------------------------------"

        deviceXList = self.getListDeviceXs ()
        for deviceX in deviceXList:
            if deviceX.isModified ():
                deviceX.save ()

        if self.fModified:
            pszFileName = self.driver.getFileName ()

            if saveNewFiles ():
                pszFileName += ".new"

            if shouldOutputDebug ("OmniEditDeviceDialog::save"):
                print "OmniEditDeviceDialog::save: \"%s\"" % (pszFileName)

            fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

            os.write (fp, self.driver.toXML ())

            os.close (fp)

            self.fModified = False

if __name__ == "__main__":
    import sys

    pszGladePath = None
    pszFileName  = None

    argc = 1
    while argc < len (sys.argv):
        argv = sys.argv[argc]

        if argv == "--gladePath":
            argc += 1
            pszGladePath = sys.argv[argc]
        else:
            pszFileName = sys.argv[argc]

        argc += 1

    if pszFileName != None:
        (rootPath, filename) = os.path.split (pszFileName)

        if    rootPath == None \
           or rootPath == "":
            rootPath = "."
    else:
        rootPath = "."
        filename = "Epson Stylus Color 760.xml"

    if pszGladePath != None:
        addToDataPath (pszGladePath)

    try:
        driver = Driver (rootPath, filename)

#       print "Driver successfully loaded!"
#       print
#       print "%s.%s" % (driver.getDriverName (), driver.getDeviceName ())
#       print "capabilities =", driver.getCapabilities ()
#       print "rasterCapabilities =", driver.getRasterCapabilities ()
#       print "deviceOptions =", driver.getDeviceOptions ()
#       print "pdl =", driver.getPDL ()
#       print "job properties =", driver.getDefaultJobProperties ()
#       print "deviceCommands = ", driver.getDeviceCommands ().getCommands ()
#       print "deviceConnections = ", driver.getDeviceConnections ().getConnections ()
#       print "deviceCopies = ", driver.getDeviceCopies ().getCopies ()
#       print "deviceDatas = ", driver.getDeviceDatas ().getDatas ()
#       print "deviceForms = ", driver.getDeviceForms ().getForms ()
#       print "deviceGammaTables = ", driver.getDeviceGammaTables ().getGammaTables ()
#       print "deviceMedias = ", driver.getDeviceMedias ().getMedias ()
#       print "deviceNumberUps = ", driver.getDeviceNumberUps ().getNumberUps ()
#       print "deviceOrientations = ", driver.getDeviceOrientations ().getOrientations ()
#       print "deviceOutputBins = ", driver.getDeviceOutputBins ().getOutputBins ()
#       print "devicePrintModes = ", driver.getDevicePrintModes ().getPrintModes ()
#       print "deviceResolutions = ", driver.getDeviceResolutions ().getResolutions ()
#       print "deviceScalings = ", driver.getDeviceScalings ().getScalings ()
#       print "deviceSheetCollates = ", driver.getDeviceSheetCollates ().getSheetCollates ()
#       print "deviceSides = ", driver.getDeviceSides ().getSides ()
#       print "deviceStitchings = ", driver.getDeviceStitchings ().getStitchings ()
#       print "deviceStrings = ", driver.getDeviceStrings ().getStrings ()
#       print "deviceTrays = ", driver.getDeviceTrays ().getTrays ()
#       print "deviceTrimmings = ", driver.getDeviceTrimmings ().getTrimmings ()

        dialog = OmniEditDeviceDialog (driver)

        gtk.main ()

        if shouldOutputDebug ("OmniEditDevice.py::__main__"):
            print "OmniEditDevice.py::__main__: Done with dialog!"

    except Exception, e:
        print "OmniEditDevice.py::__main__: Error: Caught", e

        import traceback
        traceback.print_exc (file=sys.stderr)
