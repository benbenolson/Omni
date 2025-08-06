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

<deviceResolutions
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <deviceResolution>
      <name>180x180</name>
      <xRes>180</xRes>
      <yRes>180</yRes>
      <xInternalRes>180</xInternalRes>
      <yInternalRes>180</yInternalRes>
      <command>_NUL_</command>
      <resolutionCapability>0</resolutionCapability>
      <resolutionDestinationBitsPerPel>1</resolutionDestinationBitsPerPel>
      <resolutionScanlineMultiple>1</resolutionScanlineMultiple>
   </deviceResolution>
</deviceResolutions>"""

def isValidResolution (resolution):
    if resolution == None:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: resolution is None"
        return (False, 0, 0)

    iPos1 = resolution.find ("x")

    if iPos1 == -1:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: cannot find a x in resolution (%s)" % (resolution)
        return (False, 0, 0)

    iPos2 = resolution.find ("x", iPos1 + 1)

    if iPos2 != -1:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: Too many x's in resolution (%s)" % (resolution)
        return (False, 0, 0)

    pszXRes = resolution[:iPos1]
    pszYRes = resolution[iPos1 + 1:]

    try:
        iXRes = convertToIntegerValue (pszXRes)
    except Exception, e:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: %s is not an integer" % (pszXRes)
        return (False, 0, 0)

    try:
        iYRes = convertToIntegerValue (pszYRes)
    except Exception, e:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: %s is not an integer" % (pszXRes)
        return (False, 0, 0)

    if iXRes <= 0:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: %d is not an positive integer greater than 0" % (iXRes)
        return (False, 0, 0)

    if iYRes <= 0:
        print "OmniEditDeviceResolutions.py::isValidResolution: Error: %d is not an positive integer greater than 0" % (iYRes)
        return (False, 0, 0)

    return (True, iXRes, iYRes)

def isValidOmniName (omniName):
    if omniName == None:
        return True

    return omniName in [ "RESOLUTION_UNLISTED",
                         "RESOLUTION_NONE",
                         "RESOLUTION_60_X_60",
                         "RESOLUTION_60_X_72",
                         "RESOLUTION_60_X_180",
                         "RESOLUTION_60_X_360",
                         "RESOLUTION_72_X_72",
                         "RESOLUTION_75_X_75",
                         "RESOLUTION_80_X_60",
                         "RESOLUTION_80_X_72",
                         "RESOLUTION_90_X_180",
                         "RESOLUTION_90_X_360",
                         "RESOLUTION_90_X_60",
                         "RESOLUTION_90_X_72",
                         "RESOLUTION_90_X_90",
                         "RESOLUTION_100_X_100",
                         "RESOLUTION_120_X_60",
                         "RESOLUTION_120_X_72",
                         "RESOLUTION_120_X_120",
                         "RESOLUTION_120_X_144",
                         "RESOLUTION_120_X_180",
                         "RESOLUTION_120_X_360",
                         "RESOLUTION_144_X_72",
                         "RESOLUTION_150_X_150",
                         "RESOLUTION_180_X_180",
                         "RESOLUTION_180_X_360",
                         "RESOLUTION_200_X_200",
                         "RESOLUTION_240_X_60",
                         "RESOLUTION_240_X_72",
                         "RESOLUTION_240_X_144",
                         "RESOLUTION_240_X_240",
                         "RESOLUTION_300_X_300",
                         "RESOLUTION_360_X_180",
                         "RESOLUTION_360_X_360",
                         "RESOLUTION_360_X_720",
                         "RESOLUTION_400_X_400",
                         "RESOLUTION_600_X_300",
                         "RESOLUTION_600_X_600",
                         "RESOLUTION_720_X_360",
                         "RESOLUTION_720_X_720",
                         "RESOLUTION_1200_X_600",
                         "RESOLUTION_1200_X_1200",
                         "RESOLUTION_1440_X_720",
                         "RESOLUTION_2880_X_720",
                         "RESOLUTION_DRAFT",
                         "RESOLUTION_FINE",
                         "RESOLUTION_GROUP_3",
                         "RESOLUTION_GROUP_4",
                         "RESOLUTION_HIGH",
                         "RESOLUTION_LOW",
                         "RESOLUTION_MEDIUM",
                         "RESOLUTION_NORMAL",
                         "RESOLUTION_PHOTO_QUALITY",
                         "RESOLUTION_PRESENTATION"
                       ]

def isValidCapabilities (capabilities):
    return capabilities in [ "NO_CAPABILITIES"
                           ]

class OmniEditDeviceResolution:
    def __init__ (self, resolution):
        self.iNameXRes = 0
        self.iNameYRes = 0

        if type (resolution) == types.InstanceType:
            self.setName (resolution.getName ())
            self.setOmniName (resolution.getOmniName ())
            self.setXRes (resolution.getXRes ())
            self.setYRes (resolution.getYRes ())
            self.setXInternalRes (resolution.getXInternalRes ())
            self.setYInternalRes (resolution.getYInternalRes ())
            self.setCommand (resolution.getCommand ())
            self.setCapability (resolution.getCapability ())
            self.setDestinationBitsPerPel (resolution.getDestinationBitsPerPel ())
            self.setScanlineMultiple (resolution.getScanlineMultiple ())
            self.setDeviceID (resolution.getDeviceID ())
        elif type (resolution) == types.ListType or type (resolution) == types.TupleType:
            if len (resolution) != 11:
                raise Exception ("Error: OmniEditDeviceResolution: expecting 11 elements, received " + str (len (resolution)) + " of " + str (resolution))
            if not self.setName (resolution[0]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set name (" + resolution[0] + ") !")
            if not self.setOmniName (resolution[1]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set omniName (" + resolution[1] + ") !")
            if not self.setXRes (resolution[2]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set xRes (" + resolution[2] + ") !")
            if not self.setYRes (resolution[3]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set yRes (" + resolution[3] + ") !")
            if not self.setXInternalRes (resolution[4]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set xInternalRes (" + resolution[4] + ") !")
            if not self.setYInternalRes (resolution[5]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set xInternalRes (" + resolution[5] + ") !")
            if not self.setCommand (resolution[6]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set command (" + resolution[6] + ") !")
            if not self.setCapability (resolution[7]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set capability (" + resolution[7] + ") !")
            if not self.setDestinationBitsPerPel (resolution[8]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set destinationBitsPerPel (" + resolution[8] + ") !")
            if not self.setScanlineMultiple (resolution[9]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set scanlineMultiple (" + resolution[9] + ") !")
            if not self.setDeviceID (resolution[10]):
                raise Exception ("Error: OmniEditDeviceResolution: can't set deviceID (" + resolution[10] + ") !")
        else:
            raise Exception ("Expecting OmniEditDeviceResolution, list, or tuple.  Got " + str (type (resolution)))

    def getName (self):
        return self.name

    def setName (self, name):
        (fValidResolution, iXRes, iYRes) = isValidResolution (name)
        if fValidResolution:
            self.name = name
            self.iNameXRes = iXRes
            self.iNameYRes = iYRes
            return True
        else:
            print "OmniEditDeviceResolution::setName: Error: (%s) is not a valid name!" % (name)
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
            print "OmniEditDeviceResolution::setOmniName: Error: (%s) is not a valid omni name!" % (omniName)
            return False

    def getXRes (self):
        return self.xRes

    def setXRes (self, xRes):
        try:
            iXRes = convertToIntegerValue (xRes)
            if iXRes == self.iNameXRes:
                self.xRes = iXRes
                return True
            else:
                print "OmniEditDeviceResolution::setXRes: Error: (%d) does not name x component of name (%d)" % (iXRes, self.iNameXRes)
                return False
        except Exception, e:
            print "OmniEditDeviceResolution::setXRes: Error: (%s) is not an integer!" % (xRes)
            return False

    def getYRes (self):
        return self.yRes

    def setYRes (self, yRes):
        try:
            iYRes = convertToIntegerValue (yRes)
            if iYRes == self.iNameYRes:
                self.yRes = iYRes
                return True
            else:
                print "OmniEditDeviceResolution::setYRes: Error: (%d) does not name y component of name (%d)" % (iYRes, self.iNameYRes)
                return False
        except Exception, e:
            print "OmniEditDeviceResolution::setYRes: Error: (%s) is not an integer!" % (yRes)
            return False

    def getXInternalRes (self):
        return self.xInternalRes

    def setXInternalRes (self, xInternalRes):
        try:
            iXInternalRes = convertToIntegerValue (xInternalRes)
            self.xInternalRes = iXInternalRes
            return True
        except Exception, e:
            print "OmniEditDeviceResolution::setXInternalRes: Error: (%s) is not an integer!" % (xInternalRes)
            return False

    def getYInternalRes (self):
        return self.yInternalRes

    def setYInternalRes (self, yInternalRes):
        try:
            iYInternalRes = convertToIntegerValue (yInternalRes)
            self.yInternalRes = iYInternalRes
            return True
        except Exception, e:
            print "OmniEditDeviceResolution::setYInternalRes: Error: (%s) is not an integer!" % (yInternalRes)
            return False

    def getCommand (self):
        return self.command

    def setCommand (self, command):
        if getCommandString (command) != None:
            self.command = command
            return True
        else:
            print "OmniEditDeviceResolution::setCommand: Error: (%s) is not a valid command!" % (command)
            return False

    def getCapability (self):
        return self.capability

    def setCapability (self, capability):
        try:
            iCapability = convertToIntegerValue (capability)
            self.capability = iCapability
            return True
        except Exception, e:
            print "OmniEditDeviceResolution::setCapability: Error: (%s) is not an integer!" % (capability)
            return False

    def getDestinationBitsPerPel (self):
        return self.destinationBitsPerPel

    def setDestinationBitsPerPel (self, destinationBitsPerPel):
        try:
            iDestinationBitsPerPel = convertToIntegerValue (destinationBitsPerPel)
            self.destinationBitsPerPel = iDestinationBitsPerPel
            return True
        except Exception, e:
            print "OmniEditDeviceResolution::setDestinationBitsPerPel: Error: (%s) is not an integer!" % (destinationBitsPerPel)
            return False

    def getScanlineMultiple (self):
        return self.scanlineMultiple

    def setScanlineMultiple (self, scanlineMultiple):
        try:
            iScanlineMultiple = convertToIntegerValue (scanlineMultiple)
            self.scanlineMultiple = iScanlineMultiple
            return True
        except Exception, e:
            print "OmniEditDeviceResolution::setScanlineMultiple: Error: (%s) is not an integer!" % (scanlineMultiple)
            return False

    def getDeviceID (self):
        return self.deviceID

    def setDeviceID (self, deviceID):
        if deviceID == "":
            deviceID = None

        self.deviceID = deviceID

        return True

    def setDeviceResolution (self, resolution):
        try:
            if type (resolution) == types.InstanceType:
                self.setName (resolution.getName ())
                self.setOmniName (resolution.getOmniName ())
                self.setXRes (resolution.getXRes ())
                self.setYRes (resolution.getYRes ())
                self.setXInternalRes (resolution.getXInternalRes ())
                self.setYInternalRes (resolution.getYInternalRes ())
                self.setCommand (resolution.getCommand ())
                self.setCapability (resolution.getCapability ())
                self.setDestinationBitsPerPel (resolution.getDestinationBitsPerPel ())
                self.setScanlineMultiple (resolution.getScanlineMultiple ())
                self.setDeviceID (resolution.getDeviceID ())
            else:
                print "OmniEditDeviceResolution::setDeviceResolution: Error: Expecting OmniEditDeviceResolution.  Got ", str (type (resolution))
                return False
        except Exception, e:
            print "OmniEditDeviceResolution::setDeviceResolution: Error: caught " + e
            return False

        return True

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s]" % (self.getName (),
                                                                self.getOmniName (),
                                                                self.getXRes (),
                                                                self.getYRes (),
                                                                self.getXInternalRes (),
                                                                self.getYInternalRes (),
                                                                self.getCommand (),
                                                                self.getCapability (),
                                                                self.getDestinationBitsPerPel (),
                                                                self.getScanlineMultiple (),
                                                                self.getDeviceID ()),
        if fNewLine:
            print

class OmniEditDeviceResolutions:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.resolutions = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceResolutions::isValid"):
#           print self.__class__.__name__ + ".isValid"

        elmResolutions = self.rootElement

        if elmResolutions.nodeName != "deviceResolutions":
            return "Missing <deviceResolutions>, found " + elmResolutions.nodeName

        if countChildren (elmResolutions) < 1:
            return "At least one <deviceResolution> element is required"

        elmResolution  = firstNode (elmResolutions.firstChild)
        elmResolutions = nextNode (elmResolutions)

        while elmResolution != None:
            if elmResolution.nodeName != "deviceResolution":
                return "Missing <deviceResolution>, found " + elmResolution.nodeName

            elm = firstNode (elmResolution.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            omniName = None
            if elm.nodeName == "omniName":
                omniName = getValue (elm)
                elm      = nextNode (elm)

            if elm.nodeName != "xRes":
                return "Missing <xRes>, found " + elm.nodeName

            xRes = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName != "yRes":
                return "Missing <yRes>, found " + elm.nodeName

            yRes = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName == "xInternalRes":
                xInternalRes = getValue (elm)
                elm          = nextNode (elm)

                if elm.nodeName != "yInternalRes":
                    return "Missing <yInternalRes>, found " + elm.nodeName

                yInternalRes = getValue (elm)
                elm          = nextNode (elm)

            else:
                xInternalRes = 0
                yInternalRes = 0

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getValue (elm) # getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "resolutionCapability":
                return "Missing <resolutionCapability>, found " + elm.nodeName

            capability = getValue (elm)
            elm        = nextNode (elm)

            if elm.nodeName != "resolutionDestinationBitsPerPel":
                return "Missing <resolutionDestinationBitsPerPel>, found " + elm.nodeName

            destinationBitsPerPel = getValue (elm)
            elm                   = nextNode (elm)

            if elm.nodeName != "resolutionScanlineMultiple":
                return "Missing <resolutionScanlineMultiple>, found " + elm.nodeName

            scanlineMultiple = getValue (elm)
            elm              = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmResolution = nextNode (elmResolution)

            if elm != None:
                return "Expecting no more tags in <deviceResolution>"

            self.resolutions.append (OmniEditDeviceResolution ((name,
                                                                omniName,
                                                                xRes,
                                                                yRes,
                                                                xInternalRes,
                                                                yInternalRes,
                                                                command,
                                                                capability,
                                                                destinationBitsPerPel,
                                                                scanlineMultiple,
                                                                deviceID)))

        if elmResolutions != None:
            return "Expecting no more tags in <deviceResolutions>"

        return None

    def getFileName (self):
        return self.filename

    def getResolutions (self):
        return self.resolutions

    def printSelf (self):
        print "[",
        for resolution in self.getResolutions ():
            resolution.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceResolutions xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for resolution in self.getResolutions ():
            name                  = resolution.getName ()
            omniName              = resolution.getOmniName ()
            xRes                  = resolution.getXRes ()
            yRes                  = resolution.getYRes ()
            xInternalRes          = resolution.getXInternalRes ()
            yInternalRes          = resolution.getYInternalRes ()
            command               = resolution.getCommand ()
            capability            = resolution.getCapability ()
            destinationBitsPerPel = resolution.getDestinationBitsPerPel ()
            scanlineMultiple      = resolution.getScanlineMultiple ()
            deviceID              = resolution.getDeviceID ()

            xmlData += """   <deviceResolution>
      <name>""" + name + """</name>
"""

            if omniName != None:
                xmlData += "      <omniName>" + omniName + """</omniName>
"""
            xmlData += """      <xRes>""" + str (xRes) + """</xRes>
      <yRes>""" + str (yRes) + """</yRes>
"""
            if xInternalRes != 0 and xInternalRes != 0:
                xmlData += """      <xInternalRes>""" + str (yInternalRes) + """</xInternalRes>
      <yInternalRes>""" + str (yInternalRes) + """</yInternalRes>
"""
            xmlData += """      <command>""" + convertToXMLString (command) + """</command>
      <resolutionCapability>""" + str (capability) + """</resolutionCapability>
      <resolutionDestinationBitsPerPel>""" + str (destinationBitsPerPel) + """</resolutionDestinationBitsPerPel>
      <resolutionScanlineMultiple>""" + str (scanlineMultiple) + """</resolutionScanlineMultiple>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceResolution>
"""

        xmlData += """</deviceResolutions>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceResolutions::save"):
            print "OmniEditDeviceResolutions::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceResolutionsDialog (self)
        else:
            return OmniEditDeviceResolutionDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceResolutionsWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceResolutionsWindow')

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
        if shouldOutputDebug ("OmniEditDeviceResolutionsWindow::on_window_delete"):
            print "OmniEditDeviceResolutionsWindow::on_window_delete", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceResolutionsWindow::on_window_destroy"):
            print "OmniEditDeviceResolutionsWindow::on_window_destroy", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceResolutionsDialog:
    def __init__ (self, resolution):
        self.resolution = resolution

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceResolutionsFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceResolutionDialog:
    def __init__ (self, resolutions, child):
        resolution = OmniEditDeviceResolution (child)

        self.resolutions = resolutions
        self.resolution  = resolution
        self.child       = child
        self.fChanged    = False
        self.fModified   = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceResolutionFrame')

        self.xml                        = xml
        self.window                     = window
        self.EntryName                  = xml.get_widget ('EntryResolutionName')
        self.EntryOmniName              = xml.get_widget ('EntryResolutionOmniName')
        self.EntryXRes                  = xml.get_widget ('EntryResolutionXRes')
        self.EntryYRes                  = xml.get_widget ('EntryResolutionYRes')
        self.EntryXInternalRes          = xml.get_widget ('EntryResolutionXInternalRes')
        self.EntryYInternalRes          = xml.get_widget ('EntryResolutionYInternalRes')
        self.EntryCommand               = xml.get_widget ('EntryResolutionCommand')
        self.EntryCapability            = xml.get_widget ('EntryResolutionCapability')
        self.EntryDestinationBitsPerPel = xml.get_widget ('EntryResolutionDestinationBitsPerPel')
        self.EntryScanlineMultiple      = xml.get_widget ('EntryResolutionScanlineMultiple')
        self.EntryDeviceID              = xml.get_widget ('EntryResolutionDeviceID')
        self.CheckButtonDefault         = xml.get_widget ('CheckButtonResolutionDefault')

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
#       if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::__init__"):
#           print "OmniEditDeviceResolutionsDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryName.set_text (str (resolution.getName ()))
        name = resolution.getOmniName ()
        if name == None:
            name = ""
        self.EntryOmniName.set_text (str (name))
        self.EntryXRes.set_text (str (resolution.getXRes ()))
        self.EntryYRes.set_text (str (resolution.getYRes ()))
        self.EntryXInternalRes.set_text (str (resolution.getXInternalRes ()))
        self.EntryYInternalRes.set_text (str (resolution.getYInternalRes ()))
        self.EntryCommand.set_text (str (resolution.getCommand ()))
        self.EntryCapability.set_text (str (resolution.getCapability ()))
        self.EntryDestinationBitsPerPel.set_text (str (resolution.getDestinationBitsPerPel ()))
        self.EntryScanlineMultiple.set_text (str (resolution.getScanlineMultiple ()))
        name = resolution.getDeviceID ()
        if name == None:
            name = ""
        self.EntryDeviceID.set_text (str (name))
        fDefault          = False
        device            = resolutions.getDevice ()
        deviceInfo        = device.getDeviceInformation ()
        defaultResolution = deviceInfo.getDefaultJobProperty ("Resolution")
        if defaultResolution != None and defaultResolution == resolution.getName ():
            fDefault = True
        self.CheckButtonDefault.set_active (fDefault)

        self.fChanged = False

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceResolutionsDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceResolutionsDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceResolutionsDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceResolutionsDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceResolutionsDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceResolutionsDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceResolutionsDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceResolutionsDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceResolutionsDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonGammaTableAdd_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonGammaTableAdd_clicked: HACK"
    def on_ButtonGammaTableDelete_clicked (self, window):                       print "OmniEditDeviceResolutionsDialog::on_ButtonGammaTableDelete_clicked: HACK"
    def on_ButtonGammaTableModify_clicked (self, window):                       print "OmniEditDeviceResolutionsDialog::on_ButtonGammaTableModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceResolutionsDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceResolutionsDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceResolutionsDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceResolutionsDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceResolutionsDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceResolutionsDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceResolutionsDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceResolutionsDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceResolutionsDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceResolutionsDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceResolutionsDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceResolutionsDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceResolutionsDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceResolutionsDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceResolutionsDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceResolutionsDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceResolutionsDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceResolutionsDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceResolutionsDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceResolutionsDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceResolutionsDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceResolutionsDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceResolutionsDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceResolutionsDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceResolutionsDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceResolutionsDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceResolutionsDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceResolutionsDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceResolutionsDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceResolutionsDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceResolutionsDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceResolutionsDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceResolutionsDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceResolutionsDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceResolutionsDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceResolutionsDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryGammaTableResolution_changed (self, window):                    print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableResolution_changed: HACK"
    def on_EntryGammaTableOmniResolution_changed (self, window):                print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableOmniResolution_changed: HACK"
    def on_EntryGammaTableMedia_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableMedia_changed: HACK"
    def on_EntryGammaTablePrintMode_changed (self, window):                     print "OmniEditDeviceResolutionsDialog::on_EntryGammaTablePrintMode_changed: HACK"
    def on_EntryGammaTableDitherCatagory_changed (self, window):                print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableDitherCatagory_changed: HACK"
    def on_EntryGammaTableCGamma_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableCGamma_changed: HACK"
    def on_EntryGammaTableMGamma_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableMGamma_changed: HACK"
    def on_EntryGammaTableYGamma_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableYGamma_changed: HACK"
    def on_EntryGammaTableKGamma_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableKGamma_changed: HACK"
    def on_EntryGammaTableCBias_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableCBias_changed: HACK"
    def on_EntryGammaTableMBias_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableMBias_changed: HACK"
    def on_EntryGammaTableYBias_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableYBias_changed: HACK"
    def on_EntryGammaTableKBias_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryGammaTableKBias_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceResolutionsDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceResolutionsDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceResolutionsDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceResolutionsDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceResolutionsDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceResolutionsDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceResolutionsDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceResolutionsDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceResolutionsDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceResolutionsDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceResolutionsDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceResolutionsDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceResolutionsDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceResolutionsDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceResolutionsDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceResolutionsDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceResolutionsDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceResolutionsDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceResolutionsDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceResolutionsDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceResolutionsDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceResolutionsDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceResolutionsDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceResolutionsDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceResolutionsDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceResolutionsDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceResolutionsDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceResolutionsDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceResolutionsDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceResolutionsDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceResolutionsDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceResolutionsDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceResolutionsDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceResolutionsDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceResolutionsDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceResolutionsDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceResolutionsDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceResolutionsDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceResolutionsDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceResolutionsDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceResolutionsDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceResolutionsDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceResolutionsDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonResolutionModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionModify_clicked"):
            print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.resolution.setName (self.EntryName.get_text ()):
                pszError = "Invlaid name"
            if not self.resolution.setOmniName (self.EntryOmniName.get_text ()):
                pszError = "Invlaid omni name"
            if not self.resolution.setXRes (self.EntryXRes.get_text ()):
                pszError = "Invlaid x resolution"
            if not self.resolution.setYRes (self.EntryYRes.get_text ()):
                pszError = "Invlaid y resolution"
            if not self.resolution.setXInternalRes (self.EntryXInternalRes.get_text ()):
                pszError = "Invlaid x internal resolution"
            if not self.resolution.setYInternalRes (self.EntryYInternalRes.get_text ()):
                pszError = "Invlaid y internal resolution"
            if not self.resolution.setCommand (self.EntryCommand.get_text ()):
                pszError = "Invlaid command"
            if not self.resolution.setCapability (self.EntryCapability.get_text ()):
                pszError = "Invlaid capability"
            if not self.resolution.setDestinationBitsPerPel (self.EntryDestinationBitsPerPel.get_text ()):
                pszError = "Invlaid destination bits per pel"
            if not self.resolution.setScanlineMultiple (self.EntryScanlineMultiple.get_text ()):
                pszError = "Invlaid scanline multiple"
            if not self.resolution.setDeviceID (self.EntryDeviceID.get_text ()):
                pszError = "Invlaid device ID"

            if pszError == None:
                matchingResolution = self.findResolution (self.resolution)

                if     matchingResolution != None \
                   and matchingResolution != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             self.resolution.getName () + " already exists!")
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceResolution (self.resolution)
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
            name = self.resolution.getName ()
            if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionModify_clicked"):
                print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionModify_clicked: Setting %s to default" % (name)
            device     = self.resolutions.getDevice ()
            dialog     = device.getDeviceDialog ()
            deviceInfo = device.getDeviceInformation ()
            deviceInfo.setDefaultJobProperty ("Resolution", name)
            dialog.setModified (True)

    def on_ButtonResolutionAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionAdd_clicked"):
            print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionAdd_clicked:", widget

    def on_ButtonResolutionDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked"):
            print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked:", widget

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
            (rc, iResolutionsLeft) = self.deleteResolution (self.child)
            if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked"):
                print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked: deleteResolution rc = %s, iResolutionsLeft = %d" % (rc, iResolutionsLeft)

            device    = self.resolutions.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iResolutionsLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked"):
                    print "OmniEditDeviceResolutionsDialog::on_ButtonResolutionDelete_clicked: Last resolution deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceResolutions":
                        treestore.remove (row.iter)
                        device.setDeviceResolutions (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_EntryResolutionName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionName_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionOmniName_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionOmniName_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionOmniName_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionXRes_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionXRes_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionXRes_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionYRes_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionYRes_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionYRes_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionXInternalRes_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionXInternalRes_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionXInternalRes_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionYInternalRes_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionYInternalRes_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionYInternalRes_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionCommand_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionCommand_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionCommand_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionCapability_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionCapability_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionCapability_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionDestinationBitsPerPel_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionDestinationBitsPerPel_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionDestinationBitsPerPel_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionScanlineMultiple_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionScanlineMultiple_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionScanlineMultiple_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryResolutionDeviceID_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_EntryResolutionDeviceID_changed"):
            print "OmniEditDeviceResolutionsDialog::on_EntryResolutionDeviceID_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_CheckButtonResolutionDefault_toggled (self, widget):
        if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::on_CheckButtonResolutionDefault_toggled"):
            print "OmniEditDeviceResolutionsDialog::on_CheckButtonResolutionDefault_toggled:", widget

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.resolutions.save ()

        self.fModified = False

    def findResolution (self, matchResolution):
        for resolution in self.resolutions.getResolutions ():
            if matchResolution.getName () == resolution.getName ():
                return resolution

        return None

    def deleteResolution (self, matchResolution):
        foundResolution = None
        foundRow        = None
        resolutions     = self.resolutions.getResolutions ()
        for resolution in resolutions:
#           if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::deleteResolution"):
#               print "OmniEditDeviceResolutionsDialog::deleteResolution: matching1 %s vs %s" % (matchResolution.getName (), resolution.getName ())
            if matchResolution.getName () == resolution.getName ():
                foundResolution = resolution

        device    = self.resolutions.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::deleteResolution"):
#                       print "OmniEditDeviceResolutionsDialog::deleteResolution: matching2 %s vs %s" % (childRow[1], matchResolution)
                    if childRow[1] == matchResolution:
                        foundRow = childRow

        if     foundResolution != None \
           and foundRow  != None:
            if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::deleteResolution"):
                print "OmniEditDeviceResolutionsDialog::deleteResolution: Removing %s from resolutions" % (foundResolution.getName ())
            resolutions.remove (foundResolution)

            if shouldOutputDebug ("OmniEditDeviceResolutionsDialog::deleteResolution"):
                print "OmniEditDeviceResolutionsDialog::deleteResolution: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.resolutions.setModified (True)

            return (True, len (resolutions))
        else:
            if foundResolution == None:
                print "OmniEditDeviceResolutionsDialog::deleteResolution: Error: Did not find %s in resolutions!" % (matchResolution.getName ())
            if foundRow == None:
                print "OmniEditDeviceResolutionsDialog::deleteResolution: Error: Did not find %s in listbox!" % (str (matchResolution))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Stylus Color 660 Resolutions.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        resolutions = OmniEditDeviceResolutions (fname, doc.documentElement, None)

        print "OmniEditDeviceResolutions.py::__main__: resolutions = ",
        lastResolution = resolutions.getResolutions ()[-1]
        for resolution in resolutions.getResolutions ():
            resolution.printSelf (False),
            if resolution != lastResolution:
                print ",",
        print

        dialog = OmniEditDeviceResolutionDialog (resolutions, resolutions.getResolutions ()[0])
        window = OmniEditDeviceResolutionsWindow (dialog)

        gtk.main ()

        print resolutions.toXML ()

    except Exception, e:
        print "OmniEditDeviceResolutions.py::__main__: Error: Caught", e
