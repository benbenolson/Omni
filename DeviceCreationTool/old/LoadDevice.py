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

#from xml.dom import Node
#from xml.dom.ext import Print

from Utils import *
import DeviceCommands
import DeviceConnections
import DeviceCopies
import DeviceDatas
import DeviceForms
import DeviceMedias
import DeviceNumberUps
import DeviceOrientations
import DeviceOutputBins
import DevicePrintModes
import DeviceResolutions
import DeviceScalings
import DeviceSheetCollates
import DeviceSides
import DeviceStitchings
import DeviceStrings
import DeviceTrays
import DeviceTrimmings

import LoadGtk

if not LoadGtk.loadGtk20 ():
    raise SystemExit

import gtk, gobject

class Driver:
    def __init__ (self, rootPath, filename):
        print 'Parsing "%s%s%s"' % (rootPath, os.sep, filename)

        try:
            file = open (rootPath + os.sep + filename)

        except IOError, e:
            print "File '%s' does not exist!" % filename
            raise e

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        self.doc = reader.fromStream (file)

        file.close ()

        self.rootPath            = rootPath
        self.rootElement         = self.doc.documentElement
        self.driverName          = ""
        self.deviceName          = ""
        self.capabilities        = []
        self.rasterCapabilities  = []
        self.deviceOptions       = []
        self.pdlLevel            = ""
        self.pdlSubLevel         = ""
        self.pdlMajor            = ""
        self.pdlMinor            = ""
        self.jobProperties       = {}
        self.deviceCommands      = None
        self.deviceConnections   = None
        self.deviceCopies        = None
        self.deviceDatas         = None
        self.deviceForms         = None
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

    def addComplexJobProperties (self, elm):
        elm = firstNode (elm.firstChild)

        while elm != None:
           if validSingleElement (elm):
               self.jobProperties[elm.nodeName] = getValue (elm)
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
                       self.jobProperties[key] = value

           elm = nextNode (elm)

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elm = self.rootElement

        if elm.nodeName != "Device":
            return "Missing <Device>"

        attr = elm.attributes.getNamedItem ("name")
        if attr == None:
            return "Missing name= attribute in <Device>"

        self.deviceName = attr.nodeValue

        elm = firstNode (elm.firstChild)

        if elm.nodeName != "DriverName":
            return "Missing <DriverName>"

        if not validSingleElement (elm):
            return "<DriverName> should only have one element"

        self.driverName = getValue (elm)

        elm = nextNode (elm)

        while elm.nodeName == "Capability":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <Capability>"

                self.capabilities.append (attr.nodeValue)

            else:
                return "<Capability> node should only have 1 type= attribute"

            elm = nextNode (elm)

        while elm.nodeName == "RasterCapabilities":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <RasterCapabilies>"

                self.rasterCapabilities.append (attr.nodeValue)

            else:
                return "<RasterCapabilities> node should only have 1 type= attribute"

            elm = nextNode (elm)

        while elm.nodeName == "DeviceOptions":
            if len (elm.attributes) == 1:
                attr = elm.attributes.getNamedItem ("type")

                if attr == None:
                    return "Missing type= attribute in <DeviceOptions>"

                self.deviceOptions.append (attr.nodeValue)

            else:
                return "<DeviceOptions> node should only have 1 type attribute"

            elm = nextNode (elm)

        if elm.nodeName == "PDL":
            if len (elm.attributes) != 4:
                return "<PDL> node should only have 4 type attributes (level, sublevel, major, minor)"

            try:
                attr = elm.attributes.getNamedItem ("level")

                self.pdlLevel = attr.nodeValue
            except Exception, e:
                return "<PDL> missing level attribute"

            try:
                attr = elm.attributes.getNamedItem ("sublevel")

                self.pdlSubLevel = attr.nodeValue
            except Exception, e:
                return "<PDL> missing sublevel attribute"

            try:
                attr = elm.attributes.getNamedItem ("major")

                self.pdlMajor = attr.nodeValue
            except Exception, e:
                return "<PDL> missing major attribute"

            try:
                attr = elm.attributes.getNamedItem ("minor")

                self.pdlMinor = attr.nodeValue
            except Exception, e:
                return "<PDL> missing minor attribute"

            elm = nextNode (elm)

        if     elm.nodeName != "Has"  \
           and elm.nodeName != "Uses":
            return "Missing <Has>/<Uses>"

        while    elm.nodeName == "Has"  \
              or elm.nodeName == "Uses":

            if not validSingleElement (elm):
                return "<Has>/<Uses> should only have one element"

            xmlFilename = getValue (elm)
            filename    = self.rootPath + os.sep + xmlFilename

            try:
                file = open (filename)

                from xml.dom.ext.reader import Sax2
                # create Reader object
                reader = Sax2.Reader (False, True)

                # parse the document
                doc = reader.fromStream (file)

                name = doc.documentElement.nodeName

                if name == "deviceCommands":
                    self.deviceCommands = DeviceCommands.DeviceCommands (filename, doc.documentElement)
                elif name == "deviceConnections":
                    self.deviceConnections = DeviceConnections.DeviceConnections (filename, doc.documentElement)
                elif name == "deviceCopies":
                    self.deviceCopies = DeviceCopies.DeviceCopies (filename, doc.documentElement)
                elif name == "deviceDatas":
                    self.deviceDatas = DeviceDatas.DeviceDatas (filename, doc.documentElement)
                elif name == "deviceForms":
                    self.deviceForms = DeviceForms.DeviceForms (filename, doc.documentElement)
                elif name == "deviceGammaTables":
                    pass
                elif name == "deviceMedias":
                    self.deviceMedias = DeviceMedias.DeviceMedias (filename, doc.documentElement)
                elif name == "deviceNumberUps":
                    self.deviceNumberUps = DeviceNumberUps.DeviceNumberUps (filename, doc.documentElement)
                elif name == "deviceOrientations":
                    self.deviceOrientations = DeviceOrientations.DeviceOrientations (filename, doc.documentElement)
                elif name == "deviceOutputBins":
                    self.deviceOutputBins = DeviceOutputBins.DeviceOutputBins (filename, doc.documentElement)
                elif name == "devicePrintModes":
                    self.devicePrintModes = DevicePrintModes.DevicePrintModes (filename, doc.documentElement)
                elif name == "deviceResolutions":
                    self.deviceResolutions = DeviceResolutions.DeviceResolutions (filename, doc.documentElement)
                elif name == "deviceScalings":
                    self.deviceScalings = DeviceScalings.DeviceScalings (filename, doc.documentElement)
                elif name == "deviceSheetCollates":
                    self.deviceSheetCollates = DeviceSheetCollates.DeviceSheetCollates (filename, doc.documentElement)
                elif name == "deviceSides":
                    self.deviceSides = DeviceSides.DeviceSides (filename, doc.documentElement)
                elif name == "deviceStitchings":
                    self.deviceStitchings = DeviceStitchings.DeviceStitchings (filename, doc.documentElement)
                elif name == "deviceStrings":
                    self.deviceStrings = DeviceStrings.DeviceStrings (filename, doc.documentElement)
                elif name == "deviceTrays":
                    self.deviceTrays = DeviceTrays.DeviceTrays (filename, doc.documentElement)
                elif name == "deviceTrimmings":
                    self.deviceTrimmings = DeviceTrimmings.DeviceTrimmings (filename, doc.documentElement)

                file.close ()

            except IOError, e:
                print "File '%s' does not exist!" % filename
                raise e

            elm = nextNode (elm)

        if elm.nodeName == "Instance":

            self.instance1 = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName != "Instance":
                return "2 <Instance>s not found"

            self.instance2 = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName != "Blitter":
                return "<Blitter> missing"

            self.blitter1 = getValue (elm)
            elm           = nextNode (elm)

            if elm.nodeName != "Blitter":
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

            if elm.nodeName != "PluggableBlitter":
                return "<PluggableBlitter> missing"

            elm = nextNode (elm)

        if elm.nodeName != "DefaultJobProperties":
            return "missing <DefaultJobProperties>"

        elmSave = nextNode (elm)
        elm     = firstNode (elm.firstChild)

        if elm.nodeName == "Copies":
            if not validSingleElement (elm):
                return "<Copies> should only have one element"

            self.jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if elm.nodeName != "dither":
            return "missing <dither>"

        if not validSingleElement (elm):
            return "<dither> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName != "Form":
            return "missing <Form>"

        if not validSingleElement (elm):
            return "<Form> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName == "omniForm":
            elm = nextNode (elm)

        if elm.nodeName != "media":
            return "missing <media>"

        if not validSingleElement (elm):
            return "<media> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName == "NumberUp":
            self.addComplexJobProperties (elm)

            elm = nextNode (elm)

        if elm.nodeName != "Rotation":
            return "missing <Rotation>"

        if not validSingleElement (elm):
            return "<Rotation> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName == "omniOrientation":
            elm = nextNode (elm)

        if elm.nodeName == "OutputBin":
            if not validSingleElement (elm):
                return "<OutputBin> should only have one element"

            self.jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if elm.nodeName != "printmode":
            return "missing <printmode>"

        if not validSingleElement (elm):
            return "<printmode> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName != "Resolution":
            return "missing <Resolution>"

        if not validSingleElement (elm):
            return "<Resolution> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName == "omniResolution":
            elm = nextNode (elm)

        if elm.nodeName == "Scaling":
            self.addComplexJobProperties (elm)

            elm = nextNode (elm)

        if elm.nodeName == "SheetCollate":
            if not validSingleElement (elm):
                return "<SheetCollate> should only have one element"

            self.jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if elm.nodeName == "Sides":
            if not validSingleElement (elm):
                return "<Sides> should only have one element"

            self.jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if elm.nodeName == "Stitching":
            self.addComplexJobProperties (elm)

            elm = nextNode (elm)

        if elm.nodeName != "InputTray":
            return "missing <InputTray>"

        if not validSingleElement (elm):
            return "<InputTray> should only have one element"

        self.jobProperties[elm.nodeName] = getValue (elm)

        elm = nextNode (elm)

        if elm.nodeName == "omniTray":
            elm = nextNode (elm)

        if elm.nodeName == "Trimming":
            if not validSingleElement (elm):
                return "<Trimming> should only have one element"

            self.jobProperties[elm.nodeName] = getValue (elm)

            elm = nextNode (elm)

        if elm.nodeName == "other":
            while     elm != None             \
                  and elm.nodeName == "other":
                if not validSingleElement (elm):
                    return "<other> should only have one element"

                self.jobProperties[elm.nodeName] = getValue (elm)

                elm = nextNode (elm)

        if elm != None:
            return "Expecting no more tags in <DefaultJobProperties>"

        if elmSave != None:
            return "Expecting no more tags in <Device>"

        return None

    def getDriverName (self):
        return self.driverName

    def getDeviceName (self):
        return self.deviceName

    def getCapabilities (self):
        return self.capabilities

    def getRasterCapabilities (self):
        return self.rasterCapabilities

    def getDeviceOptions (self):
        return self.deviceOptions

    def getPDL (self):
        return (self.pdlLevel, self.pdlSubLevel, self.pdlMajor, self.pdlMinor)

    def getJobProperties (self):
        return self.jobProperties

    def getDeviceCommands (self):
        return self.deviceCommands

    def getDeviceConnections (self):
        return self.deviceConnections

    def getDeviceCopies (self):
        return self.deviceCopies

    def getDeviceDatas (self):
        return self.deviceDatas

    def getDeviceForms (self):
        return self.deviceForms

    def getDeviceMedias (self):
        return self.deviceMedias

    def getDeviceNumberUps (self):
        return self.deviceNumberUps

    def getDeviceOrientations (self):
        return self.deviceOrientations

    def getDeviceOutputBins (self):
        return self.deviceOutputBins

    def getDevicePrintModes (self):
        return self.devicePrintModes

    def getDeviceResolutions (self):
        return self.deviceResolutions

    def getDeviceScalings (self):
        return self.deviceScalings

    def getDeviceSheetCollates (self):
        return self.deviceSheetCollates

    def getDeviceSides (self):
        return self.deviceSides

    def getDeviceStitchings (self):
        return self.deviceStitchings

    def getDeviceStrings (self):
        return self.deviceStrings

    def getDeviceTrays (self):
        return self.deviceTrays

    def getDeviceTrimmings (self):
        return self.deviceTrimmings

    def createWindow (self):
        print self.__class__.__name__ + ".createWindow"

        window = gtk.Window (gtk.WINDOW_TOPLEVEL)
        window.set_title ("Device")
        window.connect ("destroy", gtk.mainquit)

        # Create a new scrolled window, with scrollbars only if needed
        scrolled_window = gtk.ScrolledWindow ()
        scrolled_window.set_policy (gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)

        model = gtk.ListStore (gobject.TYPE_STRING)
        tree_view = gtk.TreeView (model)
        scrolled_window.add_with_viewport (tree_view)
        tree_view.show ()

        # create a vpaned widget and add it to our toplevel window
        vpaned = gtk.VPaned ()
        window.add (vpaned)
        vpaned.show ()

        vpaned.add1 (scrolled_window)
        scrolled_window.show ()

        window.show ()

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "."
    else:
        rootPath = "/home/Omni/new/XMLParser"
        filename = "Epson Stylus Color 760.xml"

    try:
        driver = Driver (rootPath, filename)

        print "Driver successfully loaded!"
        print
        print "%s.%s" % (driver.getDriverName (), driver.getDeviceName ())
        print "capabilities =", driver.getCapabilities ()
        print "rasterCapabilities =", driver.getRasterCapabilities ()
        print "deviceOptions =", driver.getDeviceOptions ()
        print "pdl =", driver.getPDL ()
        print "job properties =", driver.getJobProperties ()
        print "commands = ", driver.getDeviceCommands ().getCommands ()
        print "connections = ", driver.getDeviceConnections ().getConnections ()
        print "copies = ", driver.getDeviceCopies ().getCopies ()
        print "datas = ", driver.getDeviceDatas ().getDatas ()
        print "forms = ", driver.getDeviceForms ().getForms ()
        print "medias = ", driver.getDeviceMedias ().getMedias ()
        print "numberups = ", driver.getDeviceNumberUps ().getNumberUps ()
        print "orientations = ", driver.getDeviceOrientations ().getOrientations ()
        print "outputbins = ", driver.getDeviceOutputBins ().getOutputBins ()
        print "printmodes = ", driver.getDevicePrintModes ().getPrintModes ()
        print "resolutions = ", driver.getDeviceResolutions ().getResolutions ()
        print "scalings = ", driver.getDeviceScalings ().getScalings ()
        print "sheetcollates = ", driver.getDeviceSheetCollates ().getSheetCollates ()
        print "sides = ", driver.getDeviceSides ().getSides ()
        print "stitchings = ", driver.getDeviceStitchings ().getStitchings ()
        print "strings = ", driver.getDeviceStrings ().getStrings ()
        print "trays = ", driver.getDeviceTrays ().getTrays ()
        print "trimmings = ", driver.getDeviceTrimmings ().getTrimmings ()

        driver.createWindow ()

        gtk.main ()

    except Exception, e:
        print "Caught", e
