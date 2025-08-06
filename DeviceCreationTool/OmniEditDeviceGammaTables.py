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
import OmniEditDeviceResolutions
import OmniEditDeviceMedias
import OmniEditDevicePrintModes

import gtk
import gtk.glade
import gobject
import types

def getDefaultXML (jobProperties):
    return XMLHeader () + """

<deviceGammaTables
   xmlns:omni="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
   xsi:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd"
   xmlns:targetNamespace="omni">
   <deviceGammaTable>
      <Resolution>""" + jobProperties["Resolution"] + """</Resolution>
      <media>""" + jobProperties["media"] + """</media>
      <printmode>""" + jobProperties["printmode"] + """</printmode>
      <gammaTableDitherCatagory>DITHER_CATAGORY_MATRIX</gammaTableDitherCatagory>
      <gammaTableCGamma>0</gammaTableCGamma>
      <gammaTableMGamma>0</gammaTableMGamma>
      <gammaTableYGamma>0</gammaTableYGamma>
      <gammaTableKGamma>0</gammaTableKGamma>
      <gammaTableCBias>0</gammaTableCBias>
      <gammaTableMBias>0</gammaTableMBias>
      <gammaTableYBias>0</gammaTableYBias>
      <gammaTableKBias>0</gammaTableKBias>
   </deviceGammaTable>
</deviceGammaTables>"""

def isValidDitherName (name):
    return name in [ "DITHER_LEVEL",
                     "DITHER_SNAP",
                     "DITHER_DITHER_4x4",
                     "DITHER_DITHER_8x8",
                     "DITHER_STUCKI_DIFFUSION",
                     "DITHER_STUCKI_BIDIFFUSION",
                     "DITHER_MAGIC_SQUARES",
                     "DITHER_ORDERED_SQUARES",
                     "DITHER_FAST_DIFFUSION",
                     "DITHER_STEINBERG_DIFFUSION",
                     "DITHER_SMOOTH_DIFFUSION",
                     "DITHER_HSV_DIFFUSION",
                     "DITHER_HSV_BIDIFFUSION",
                     "DITHER_CMYK_DIFFUSION",
                     "DITHER_VOID_CLUSTER",
                     "DITHER_JANIS_STUCKI",
                     "DITHER_ESTUCKI_DIFFUSION"
                   ]

def isValidDitherCatagory (name):
    return name in [ "DITHER_CATAGORY_MATRIX",
                     "DITHER_CATAGORY_DIFFUSION",
                     "DITHER_CATAGORY_HSV_DIFFUSION",
                     "DITHER_CATAGORY_CMYK_DIFFUSION",
                     "DITHER_CATAGORY_VOID_CLUSTER",
                     "DITHER_CATAGORY_NEW_DIFFUSION",
                   ]

class OmniEditDeviceGammaTable:
    def __init__ (self, gamma):
        if type (gamma) == types.InstanceType:
            self.setResolution (gamma.getResolution ())
            self.setOmniResolution (gamma.getOmniResolution ())
            self.setMedia (gamma.getMedia ())
            self.setPrintMode (gamma.getPrintMode ())
            self.setDitherCatagory (gamma.getDitherCatagory ())
            self.setCGamma (gamma.getCGamma ())
            self.setMGamma (gamma.getMGamma ())
            self.setYGamma (gamma.getYGamma ())
            self.setKGamma (gamma.getKGamma ())
            self.setCBias (gamma.getCBias ())
            self.setMBias (gamma.getMBias ())
            self.setYBias (gamma.getYBias ())
            self.setKBias (gamma.getKBias ())
        elif type (gamma) == types.ListType or type (gamma) == types.TupleType:
            if len (gamma) != 13:
                raise Exception ("Error: OmniEditDeviceGammaTable: expecting 13 elements, received " + str (len (gamma)) + " of " + str (gamma))
            if not self.setResolution (gamma[0]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set resolution (" + gamma[0] + ") !")
            if not self.setOmniResolution (gamma[1]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set omniResolution (" + gamma[1] + ") !")
            if not self.setMedia (gamma[2]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set media (" + gamma[2] + ") !")
            if not self.setPrintMode (gamma[3]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set printMode (" + gamma[3] + ") !")
            if not self.setDitherCatagory (gamma[4]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set ditherCatagory (" + gamma[4] + ") !")
            if not self.setCGamma (gamma[5]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set cGamma (" + gamma[5] + ") !")
            if not self.setMGamma (gamma[6]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set mGamma (" + gamma[6] + ") !")
            if not self.setYGamma (gamma[7]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set yGamma (" + gamma[7] + ") !")
            if not self.setKGamma (gamma[8]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set kGamma (" + gamma[8] + ") !")
            if not self.setCBias (gamma[9]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set cBias (" + gamma[9] + ") !")
            if not self.setMBias (gamma[10]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set mBias (" + gamma[10] + ") !")
            if not self.setYBias (gamma[11]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set yBias (" + gamma[11] + ") !")
            if not self.setKBias (gamma[12]):
                raise Exception ("Error: OmniEditDeviceGammaTable: can't set kBias (" + gamma[12] + ") !")
        else:
            raise Exception ("Expecting OmniEditDeviceGammaTable, list, or tuple.  Got " + str (type (gamma)))

    def getResolution (self):
        return self.resolution

    def setResolution (self, resolution):
        (fValid, x, y) = OmniEditDeviceResolutions.isValidResolution (resolution)
        if fValid:
            self.resolution = resolution
            return True
        else:
            print "OmniEditDeviceGammaTable::setResolution: Error: (%s) is not a valid resolution!" % (resolution)
            return False

    def getOmniResolution (self):
        return self.omniResolution

    def setOmniResolution (self, omniResolution):
        if omniResolution == "":
            omniResolution = None

        if OmniEditDeviceResolutions.isValidOmniName (omniResolution):
            self.omniResolution = omniResolution
            return True
        else:
            print "OmniEditDeviceGammaTable::setOmniResolution: Error: (%s) is not a valid omni name!" % (omniResolution)
            return False

    def getMedia (self):
        return self.media

    def setMedia (self, media):
        if OmniEditDeviceMedias.isValidName (media):
            self.media = media
            return True
        else:
            print "OmniEditDeviceGammaTable::setMedia: Error: (%s) is not a valid name!" % (media)
            return False

    def getPrintMode (self):
        return self.printMode

    def setPrintMode (self, printMode):
        if OmniEditDevicePrintModes.isValidName (printMode):
            self.printMode = printMode
            return True
        else:
            print "OmniEditDeviceGammaTable::setPrintMode: Error: (%s) is not a valid name!" % (printMode)
            return False

    def getDitherCatagory (self):
        return self.ditherCatagory

    def setDitherCatagory (self, ditherCatagory):
        if isValidDitherCatagory (ditherCatagory):
            self.ditherCatagory = ditherCatagory
            return True
        else:
            print "OmniEditDeviceGammaTable::setDitherCatagory: Error: (%s) is not a valid name!" % (ditherCatagory)
            return False

    def getCGamma (self):
        return self.cGamma

    def setCGamma (self, cGamma):
        try:
            self.cGamma = convertToIntegerValue (cGamma)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setCGamma: Error: (%s) is not an integer!" % (cGamma)
            return False

    def getMGamma (self):
        return self.mGamma

    def setMGamma (self, mGamma):
        try:
            self.mGamma = convertToIntegerValue (mGamma)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setMGamma: Error: (%s) is not an integer!" % (mGamma)
            return False

    def getYGamma (self):
        return self.yGamma

    def setYGamma (self, yGamma):
        try:
            self.yGamma = convertToIntegerValue (yGamma)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setYGamma: Error: (%s) is not an integer!" % (yGamma)
            return False

    def getKGamma (self):
        return self.kGamma

    def setKGamma (self, kGamma):
        try:
            self.kGamma = convertToIntegerValue (kGamma)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setKGamma: Error: (%s) is not an integer!" % (kGamma)
            return False

    def getCBias (self):
        return self.cBias

    def setCBias (self, cBias):
        try:
            self.cBias = convertToIntegerValue (cBias)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setCBias: Error: (%s) is not an integer!" % (cBias)
            return False

    def getMBias (self):
        return self.mBias

    def setMBias (self, mBias):
        try:
            self.mBias = convertToIntegerValue (mBias)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setMBias: Error: (%s) is not an integer!" % (mBias)
            return False

    def getYBias (self):
        return self.yBias

    def setYBias (self, yBias):
        try:
            self.yBias = convertToIntegerValue (yBias)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setYBias: Error: (%s) is not an integer!" % (yBias)
            return False

    def getKBias (self):
        return self.kBias

    def setKBias (self, kBias):
        try:
            self.kBias = convertToIntegerValue (kBias)
            return True
        except Exception, e:
            print "OmniEditDeviceGammaTable::setKBias: Error: (%s) is not an integer!" % (kBias)
            return False

    def setDeviceGammaTable (self, gamma):
        try:
            if type (gamma) == types.InstanceType:
                self.setResolution (gamma.getResolution ())
                self.setOmniResolution (gamma.getOmniResolution ())
                self.setMedia (gamma.getMedia ())
                self.setPrintMode (gamma.getPrintMode ())
                self.setDitherCatagory (gamma.getDitherCatagory ())
                self.setCGamma (gamma.getCGamma ())
                self.setMGamma (gamma.getMGamma ())
                self.setYGamma (gamma.getYGamma ())
                self.setKGamma (gamma.getKGamma ())
                self.setCBias (gamma.getCBias ())
                self.setMBias (gamma.getMBias ())
                self.setYBias (gamma.getYBias ())
                self.setKBias (gamma.getKBias ())
            else:
                print "OmniEditDeviceGammaTable::setDeviceGammaTable: Error: Expecting OmniEditDeviceGammaTable.  Got ", str (type (gamma))
                return False
        except Exception, e:
            print "OmniEditDeviceGammaTable::setDeviceGammaTable: Error: caught " + e
            return False

    def printSelf (self, fNewLine = True):
        print "[%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s]" % (self.getResolution (),
                                                                        self.getOmniResolution (),
                                                                        self.getMedia (),
                                                                        self.getPrintMode (),
                                                                        self.getDitherCatagory (),
                                                                        self.getCGamma (),
                                                                        self.getMGamma (),
                                                                        self.getYGamma (),
                                                                        self.getKGamma (),
                                                                        self.getCBias (),
                                                                        self.getMBias (),
                                                                        self.getYBias (),
                                                                        self.getKBias ()),
        if fNewLine:
            print

class OmniEditDeviceGammaTables:
    def __init__ (self, filename, rootElement, device):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.gammas      = []
        self.device      = device
        self.fModified   = False

        error = self.isValid ()
        if error != None:
            e =  "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       if shouldOutputDebug ("OmniEditDeviceGammaTables::isValid"):
#           print self.__class__.__name__ + "isValid"

        elmGammaTables = self.rootElement

        if elmGammaTables.nodeName != "deviceGammaTables":
            return "Missing <deviceGammaTables>, found " + elmGammaTables.nodeName

        if countChildren (elmGammaTables) < 1:
            return "At least one <deviceGammaTable> element is required"

        elmGammaTable  = firstNode (elmGammaTables.firstChild)
        elmGammaTables = nextNode (elmGammaTables)

        while elmGammaTable != None:
            if elmGammaTable.nodeName != "deviceGammaTable":
                return "Missing <deviceGammaTable>, found " + elmGammaTable.nodeName

            elm = firstNode (elmGammaTable.firstChild)

            if elm.nodeName != "Resolution":
                return "Missing <Resolution>, found " + elm.nodeName

            resolution = getValue (elm)
            elm        = nextNode (elm)

            omniResolution = None
            if elm.nodeName == "omniResolution":
                omniResolution = getValue (elm)
                elm            = nextNode (elm)

            if elm.nodeName != "media":
                return "Missing <media>, found " + elm.nodeName

            media = getValue (elm)
            elm   = nextNode (elm)

            if elm.nodeName != "printmode":
                return "Missing <printmode>, found " + elm.nodeName

            printmode = getValue (elm)
            elm       = nextNode (elm)

            if elm.nodeName != "gammaTableDitherCatagory":
                return "Missing <gammaTableDitherCatagory>, found " + elm.nodeName

            ditherCatagory = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName != "gammaTableCGamma":
                return "Missing <gammaTableCGamma>, found " + elm.nodeName

            cGamma = getValue (elm)
            elm    = nextNode (elm)

            if elm.nodeName != "gammaTableMGamma":
                return "Missing <gammaTableMGamma>, found " + elm.nodeName

            mGamma = getValue (elm)
            elm    = nextNode (elm)

            if elm.nodeName != "gammaTableYGamma":
                return "Missing <gammaTableYGamma>, found " + elm.nodeName

            yGamma = getValue (elm)
            elm    = nextNode (elm)

            if elm.nodeName != "gammaTableKGamma":
                return "Missing <gammaTableKGamma>, found " + elm.nodeName

            kGamma = getValue (elm)
            elm    = nextNode (elm)

            if elm.nodeName != "gammaTableCBias":
                return "Missing <gammaTableCBias>, found " + elm.nodeName

            cBias = getValue (elm)
            elm   = nextNode (elm)

            if elm.nodeName != "gammaTableMBias":
                return "Missing <gammaTableMBias>, found " + elm.nodeName

            mBias = getValue (elm)
            elm   = nextNode (elm)

            if elm.nodeName != "gammaTableYBias":
                return "Missing <gammaTableYBias>, found " + elm.nodeName

            yBias = getValue (elm)
            elm   = nextNode (elm)

            if elm.nodeName != "gammaTableKBias":
                return "Missing <gammaTableKBias>, found " + elm.nodeName

            kBias = getValue (elm)
            elm   = nextNode (elm)

            elmGammaTable = nextNode (elmGammaTable)

            if elm != None:
                return "Expecting no more tags in <deviceGammaTable>"

            self.gammas.append (OmniEditDeviceGammaTable ((resolution,
                                                           omniResolution,
                                                           media,
                                                           printmode,
                                                           ditherCatagory,
                                                           cGamma,
                                                           mGamma,
                                                           yGamma,
                                                           kGamma,
                                                           cBias,
                                                           mBias,
                                                           yBias,
                                                           kBias)))

        if elmGammaTables != None:
            return "Expecting no more tags in <deviceGammaTables>"

        return None

    def getFileName (self):
        return self.filename

    def getGammaTables (self):
        return self.gammas

    def printSelf (self):
        print "[",
        for gamma in self.getGammaTables ():
            gamma.printSelf (False)
        print "]"

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceGammaTables xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
                   xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
                   xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for gamma in self.getGammaTables ():
            resolution     = gamma.getResolution ()
            omniResolution = gamma.getOmniResolution ()
            media          = gamma.getMedia ()
            printmode      = gamma.getPrintMode ()
            ditherCatagory = gamma.getDitherCatagory ()
            cGamma         = gamma.getCGamma ()
            mGamma         = gamma.getMGamma ()
            yGamma         = gamma.getYGamma ()
            kGamma         = gamma.getKGamma ()
            cBias          = gamma.getCBias ()
            mBias          = gamma.getMBias ()
            yBias          = gamma.getYBias ()
            kBias          = gamma.getKBias ()

            xmlData += """   <deviceGammaTable>
      <Resolution>""" + resolution + """</Resolution>
"""

            if omniResolution != None:
                xmlData += "      <omniResolution>" + omniResolution + """</omniResolution>
"""

            xmlData += "      <media>" + media + """</media>
      <printmode>""" + printmode + """</printmode>
      <gammaTableDitherCatagory>""" + ditherCatagory + """</gammaTableDitherCatagory>
      <gammaTableCGamma>""" + str (cGamma) + """</gammaTableCGamma>
      <gammaTableMGamma>""" + str (mGamma) + """</gammaTableMGamma>
      <gammaTableYGamma>""" + str (yGamma) + """</gammaTableYGamma>
      <gammaTableKGamma>""" + str (kGamma) + """</gammaTableKGamma>
      <gammaTableCBias>""" + str (cBias) + """</gammaTableCBias>
      <gammaTableMBias>""" + str (mBias) + """</gammaTableMBias>
      <gammaTableYBias>""" + str (yBias) + """</gammaTableYBias>
      <gammaTableKBias>""" + str (kBias) + """</gammaTableKBias>
"""

            xmlData += """   </deviceGammaTable>
"""

        xmlData += """</deviceGammaTables>
"""

        return xmlData

    def save (self):
        pszFileName = self.getFileName ()

        if saveNewFiles ():
            pszFileName += ".new"

        if shouldOutputDebug ("OmniEditDeviceGammaTables::save"):
            print "OmniEditDeviceGammaTables::save: \"%s\"" % (pszFileName)

        import os

        fp = os.open (pszFileName, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0660)

        os.write (fp, self.toXML ())

        os.close (fp)

    def getDialog (self, child):
        if child == None:
            return OmniEditDeviceGammaTablesDialog (self)
        else:
            return OmniEditDeviceGammaTableDialog (self, child)

    def getDevice (self):
        return self.device

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

class OmniEditDeviceGammaTablesWindow:
    def __init__ (self, dialog):
        self.dialog = dialog

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceGammaTablesWindow')

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
        if shouldOutputDebug ("OmniEditDeviceGammaTablesWindow::on_window_delete"):
            print "OmniEditDeviceGammaTablesWindow::on_window_delete:", args

        return gtk.FALSE

    def on_window_destroy (self, window):
        """Callback for the window being destroyed."""
        if shouldOutputDebug ("OmniEditDeviceGammaTablesWindow::on_window_destroy"):
            print "OmniEditDeviceGammaTablesWindow::on_window_destroy:", window

        window.hide ()
        gtk.mainquit ()

class OmniEditDeviceGammaTablesDialog:
    def __init__ (self, gamma):
        self.gamma = gamma

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceGammaTablesFrame')

        self.xml    = xml
        self.window = window

    def getWindow (self):
        return self.window

class OmniEditDeviceGammaTableDialog:
    def __init__ (self, gammas, child):
        gamma = OmniEditDeviceGammaTable (child)

        self.gammas    = gammas
        self.gamma     = gamma
        self.child     = child
        self.fChanged  = False
        self.fModified = False

        filename = findDataFile ("device.glade")
        xml      = gtk.glade.XML (filename)
        window   = xml.get_widget ('DeviceGammaTableFrame')

        self.xml                 = xml
        self.window              = window
        self.EntryResolution     = xml.get_widget ('EntryGammaTableResolution')
        self.EntryOmniResolution = xml.get_widget ('EntryGammaTableOmniResolution')
        self.EntryMedia          = xml.get_widget ('EntryGammaTableMedia')
        self.EntryPrintMode      = xml.get_widget ('EntryGammaTablePrintMode')
        self.EntryDitherCatagory = xml.get_widget ('EntryGammaTableDitherCatagory')
        self.EntryCGamma         = xml.get_widget ('EntryGammaTableCGamma')
        self.EntryMGamma         = xml.get_widget ('EntryGammaTableMGamma')
        self.EntryYGamma         = xml.get_widget ('EntryGammaTableYGamma')
        self.EntryKGamma         = xml.get_widget ('EntryGammaTableKGamma')
        self.EntryCBias          = xml.get_widget ('EntryGammaTableCBias')
        self.EntryMBias          = xml.get_widget ('EntryGammaTableMBias')
        self.EntryYBias          = xml.get_widget ('EntryGammaTableYBias')
        self.EntryKBias          = xml.get_widget ('EntryGammaTableKBias')

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
#       if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::__init__"):
#           print "OmniEditDeviceGammaTablesDialog::__init__: dic =", dic

        xml.signal_autoconnect (dic)

        self.EntryResolution.set_text (str (gamma.getResolution ()))
        name = gamma.getOmniResolution ()
        if name == None:
            name = ""
        self.EntryOmniResolution.set_text (str (name))
        self.EntryMedia.set_text (str (gamma.getMedia ()))
        self.EntryPrintMode.set_text (str (gamma.getPrintMode ()))
        self.EntryDitherCatagory.set_text (str (gamma.getDitherCatagory ()))
        self.EntryCGamma.set_text (str (gamma.getCGamma ()))
        self.EntryMGamma.set_text (str (gamma.getMGamma ()))
        self.EntryYGamma.set_text (str (gamma.getYGamma ()))
        self.EntryKGamma.set_text (str (gamma.getKGamma ()))
        self.EntryCBias.set_text (str (gamma.getCBias ()))
        self.EntryMBias.set_text (str (gamma.getMBias ()))
        self.EntryYBias.set_text (str (gamma.getYBias ()))
        self.EntryKBias.set_text (str (gamma.getKBias ()))

        self.fChanged = False

    def getWindow (self):
        return self.window

    def on_ButtonCommandAdd_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonCommandAdd_clicked: HACK"
    def on_ButtonCommandDelete_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonCommandDelete_clicked: HACK"
    def on_ButtonCommandModify_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonCommandModify_clicked: HACK"
    def on_ButtonConnectionAdd_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonConnectionAdd_clicked: HACK"
    def on_ButtonConnectionDelete_clicked (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_ButtonConnectionDelete_clicked: HACK"
    def on_ButtonConnectionModify_clicked (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_ButtonConnectionModify_clicked: HACK"
    def on_ButtonCopyModify_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonCopyModify_clicked: HACK"
    def on_ButtonCopyDelete_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonCopyDelete_clicked: HACK"
    def on_ButtonDataAdd_clicked (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_ButtonDataAdd_clicked: HACK"
    def on_ButtonDataDelete_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonDataDelete_clicked: HACK"
    def on_ButtonDataModify_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonDataModify_clicked: HACK"
    def on_ButtonDeviceInformationCancel_clicked (self, window):                print "OmniEditDeviceGammaTablesDialog::on_ButtonDeviceInformationCancel_clicked: HACK"
    def on_ButtonDeviceInformationCapabilitiesEdit_clicked (self, window):      print "OmniEditDeviceGammaTablesDialog::on_ButtonDeviceInformationCapabilitiesEdit_clicked: HACK"
    def on_ButtonDeviceInformationDeviceOptionsEdit_clicked (self, window):     print "OmniEditDeviceGammaTablesDialog::on_ButtonDeviceInformationDeviceOptionsEdit_clicked: HACK"
    def on_ButtonDeviceInformationModify_clicked (self, window):                print "OmniEditDeviceGammaTablesDialog::on_ButtonDeviceInformationModify_clicked: HACK"
    def on_ButtonDeviceInformationRasterCapabilitesEdit_clicked (self, window): print "OmniEditDeviceGammaTablesDialog::on_ButtonDeviceInformationRasterCapabilitesEdit_clicked: HACK"
    def on_ButtonFormAdd_clicked (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_ButtonFormAdd_clicked: HACK"
    def on_ButtonFormDelete_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonFormDelete_clicked: HACK"
    def on_ButtonFormModify_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonFormModify_clicked: HACK"
    def on_ButtonMediaAdd_clicked (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_ButtonMediaAdd_clicked: HACK"
    def on_ButtonMediaDelete_clicked (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_ButtonMediaDelete_clicked: HACK"
    def on_ButtonMediaModify_clicked (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_ButtonMediaModify_clicked: HACK"
    def on_ButtonNumberUpAdd_clicked (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_ButtonNumberUpAdd_clicked: HACK"
    def on_ButtonNumberUpDelete_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonNumberUpDelete_clicked: HACK"
    def on_ButtonNumberUpModify_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonNumberUpModify_clicked: HACK"
    def on_ButtonOptionModify_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonOptionModify_clicked: HACK"
    def on_ButtonOptionAdd_clicked (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_ButtonOptionAdd_clicked: HACK"
    def on_ButtonOptionDelete_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonOptionDelete_clicked: HACK"
    def on_ButtonOptionOk_clicked (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_ButtonOptionOk_clicked: HACK"
    def on_ButtonOptionCancel_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonOptionCancel_clicked: HACK"
    def on_ButtonOrientationAdd_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonOrientationAdd_clicked: HACK"
    def on_ButtonOrientationDelete_clicked (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_ButtonOrientationDelete_clicked: HACK"
    def on_ButtonOrientationModify_clicked (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_ButtonOrientationModify_clicked: HACK"
    def on_ButtonOutputBinAdd_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonOutputBinAdd_clicked: HACK"
    def on_ButtonOutputBinDelete_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonOutputBinDelete_clicked: HACK"
    def on_ButtonOutputBinModify_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonOutputBinModify_clicked: HACK"
    def on_ButtonPrintModeAdd_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonPrintModeAdd_clicked: HACK"
    def on_ButtonPrintModeDelete_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonPrintModeDelete_clicked: HACK"
    def on_ButtonPrintModeModify_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonPrintModeModify_clicked: HACK"
    def on_ButtonResolutionAdd_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonResolutionAdd_clicked: HACK"
    def on_ButtonResolutionDelete_clicked (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_ButtonResolutionDelete_clicked: HACK"
    def on_ButtonResolutionModify_clicked (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_ButtonResolutionModify_clicked: HACK"
    def on_ButtonScalingAdd_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonScalingAdd_clicked: HACK"
    def on_ButtonScalingDelete_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonScalingDelete_clicked: HACK"
    def on_ButtonScalingModify_clicked (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_ButtonScalingModify_clicked: HACK"
    def on_ButtonSheetCollateAdd_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonSheetCollateAdd_clicked: HACK"
    def on_ButtonSheetCollateDelete_clicked (self, window):                     print "OmniEditDeviceGammaTablesDialog::on_ButtonSheetCollateDelete_clicked: HACK"
    def on_ButtonSheetCollateModify_clicked (self, window):                     print "OmniEditDeviceGammaTablesDialog::on_ButtonSheetCollateModify_clicked: HACK"
    def on_ButtonSideAdd_clicked (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_ButtonSideAdd_clicked: HACK"
    def on_ButtonSideDelete_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonSideDelete_clicked: HACK"
    def on_ButtonSideModify_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonSideModify_clicked: HACK"
    def on_ButtonStitchingAdd_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonStitchingAdd_clicked: HACK"
    def on_ButtonStitchingDelete_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonStitchingDelete_clicked: HACK"
    def on_ButtonStitchingModify_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonStitchingModify_clicked: HACK"
    def on_ButtonStringCCAdd_clicked (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_ButtonStringCCAdd_clicked: HACK"
    def on_ButtonStringCCModify_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonStringCCModify_clicked: HACK"
    def on_ButtonStringCCDelete_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonStringCCDelete_clicked: HACK"
    def on_ButtonStringKeyAdd_clicked (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_ButtonStringKeyAdd_clicked: HACK"
    def on_ButtonStringKeyDelete_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonStringKeyDelete_clicked: HACK"
    def on_ButtonStringKeyModify_clicked (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_ButtonStringKeyModify_clicked: HACK"
    def on_ButtonTrayAdd_clicked (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_ButtonTrayAdd_clicked: HACK"
    def on_ButtonTrayDelete_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonTrayDelete_clicked: HACK"
    def on_ButtonTrayModify_clicked (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_ButtonTrayModify_clicked: HACK"
    def on_ButtonTrimmingAdd_clicked (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_ButtonTrimmingAdd_clicked: HACK"
    def on_ButtonTrimmingDelete_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonTrimmingDelete_clicked: HACK"
    def on_ButtonTrimmingModify_clicked (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_ButtonTrimmingModify_clicked: HACK"
    def on_CheckButtonFormDefault_toggled (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_CheckButtonFormDefault_toggled: HACK"
    def on_CheckButtonMediaDefault_toggled (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_CheckButtonMediaDefault_toggled: HACK"
    def on_CheckButtonNumberUpDefault_toggled (self, window):                   print "OmniEditDeviceGammaTablesDialog::on_CheckButtonNumberUpDefault_toggled: HACK"
    def on_CheckButtonOrientationDefault_toggled (self, window):                print "OmniEditDeviceGammaTablesDialog::on_CheckButtonOrientationDefault_toggled: HACK"
    def on_CheckButtonOutputBinDefault_toggled (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_CheckButtonOutputBinDefault_toggled: HACK"
    def on_CheckButtonPrintModeDefault_toggled (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_CheckButtonPrintModeDefault_toggled: HACK"
    def on_CheckButtonResolutionDefault_toggled (self, window):                 print "OmniEditDeviceGammaTablesDialog::on_CheckButtonResolutionDefault_toggled: HACK"
    def on_CheckButtonScalingDefault_toggled (self, window):                    print "OmniEditDeviceGammaTablesDialog::on_CheckButtonScalingDefault_toggled: HACK"
    def on_CheckButtonSheetCollateDefault_toggled (self, window):               print "OmniEditDeviceGammaTablesDialog::on_CheckButtonSheetCollateDefault_toggled: HACK"
    def on_CheckButtonSideDefault_toggled (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_CheckButtonSideDefault_toggled: HACK"
    def on_CheckButtonStitchingDefault_toggled (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_CheckButtonStitchingDefault_toggled: HACK"
    def on_CheckButtonTrayDefault_toggled (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_CheckButtonTrayDefault_toggled: HACK"
    def on_CheckButtonTrimmingDefault_toggled (self, window):                   print "OmniEditDeviceGammaTablesDialog::on_CheckButtonTrimmingDefault_toggled: HACK"
    def on_EntryCommandName_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryCommandName_changed: HACK"
    def on_EntryCommandCommand_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryCommandCommand_changed: HACK"
    def on_EntryConnectionName_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionName_changed: HACK"
    def on_EntryConnectionForm_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionForm_changed: HACK"
    def on_EntryConnectionOmniForm_changed (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionOmniForm_changed: HACK"
    def on_EntryConnectionTray_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionTray_changed: HACK"
    def on_EntryConnectionOmniTray_changed (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionOmniTray_changed: HACK"
    def on_EntryConnectionMedia_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryConnectionMedia_changed: HACK"
    def on_EntryCopiesDefault_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryCopiesDefault_changed: HACK"
    def on_EntryCopyCommand_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryCopyCommand_changed: HACK"
    def on_EntryCopyDeviceID_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryCopyDeviceID_changed: HACK"
    def on_EntryCopyMaximum_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryCopyMaximum_changed: HACK"
    def on_EntryCopyMinimum_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryCopyMinimum_changed: HACK"
    def on_EntryCopySimulationRequired_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntryCopySimulationRequired_changed: HACK"
    def on_EntryDataName_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntryDataName_changed: HACK"
    def on_EntryDataType_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntryDataType_changed: HACK"
    def on_EntryDataData_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntryDataData_changed: HACK"
    def on_EntryDeviceInformationDeviceName_changed (self, window):             print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationDeviceName_changed: HACK"
    def on_EntryDeviceInformationDriverName_changed (self, window):             print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationDriverName_changed: HACK"
    def on_EntryDeviceInformationPDLLevel_changed (self, window):               print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationPDLLevel_changed: HACK"
    def on_EntryDeviceInformationPDLMajor_changed (self, window):               print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationPDLMajor_changed: HACK"
    def on_EntryDeviceInformationPDLMinor_changed (self, window):               print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationPDLMinor_changed: HACK"
    def on_EntryDeviceInformationPDLSubLevel_changed (self, window):            print "OmniEditDeviceGammaTablesDialog::on_EntryDeviceInformationPDLSubLevel_changed: HACK"
    def on_EntryFormCapabilities_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntryFormCapabilities_changed: HACK"
    def on_EntryFormCommand_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryFormCommand_changed: HACK"
    def on_EntryFormDeviceID_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryFormDeviceID_changed: HACK"
    def on_EntryFormHCCBottom_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryFormHCCBottom_changed: HACK"
    def on_EntryFormHCCLeft_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryFormHCCLeft_changed: HACK"
    def on_EntryFormHCCRight_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryFormHCCRight_changed: HACK"
    def on_EntryFormHCCTop_changed (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_EntryFormHCCTop_changed: HACK"
    def on_EntryFormName_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntryFormName_changed: HACK"
    def on_EntryFormOmniName_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryFormOmniName_changed: HACK"
    def on_EntryMediaAbsorption_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryMediaAbsorption_changed: HACK"
    def on_EntryMediaColorAdjustRequired_changed (self, window):                print "OmniEditDeviceGammaTablesDialog::on_EntryMediaColorAdjustRequired_changed: HACK"
    def on_EntryMediaCommand_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryMediaCommand_changed: HACK"
    def on_EntryMediaDeviceID_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryMediaDeviceID_changed: HACK"
    def on_EntryMediaName_changed (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_EntryMediaName_changed: HACK"
    def on_EntryNumberUpCommand_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpCommand_changed: HACK"
    def on_EntryNumberUpDeviceID_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpDeviceID_changed: HACK"
    def on_EntryNumberUpDirection_changed (self, window):           print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpDirection_changed: HACK"
    def on_EntryNumberUpSimulationRequired_changed (self, window):              print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpSimulationRequired_changed: HACK"
    def on_EntryNumberUpX_changed (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpX_changed: HACK"
    def on_EntryNumberUpY_changed (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_EntryNumberUpY_changed: HACK"
    def on_EntryOptionName_changed (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_EntryOptionName_changed: HACK"
    def on_EntryOrientationDeviceID_changed (self, window):                     print "OmniEditDeviceGammaTablesDialog::on_EntryOrientationDeviceID_changed: HACK"
    def on_EntryOrientationName_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryOrientationName_changed: HACK"
    def on_EntryOrientationOmniName_changed (self, window):                     print "OmniEditDeviceGammaTablesDialog::on_EntryOrientationOmniName_changed: HACK"
    def on_EntryOrientationSimulationRequired_changed (self, window):           print "OmniEditDeviceGammaTablesDialog::on_EntryOrientationSimulationRequired_changed: HACK"
    def on_EntryOutputBinCommand_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntryOutputBinCommand_changed: HACK"
    def on_EntryOutputBinDeviceID_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryOutputBinDeviceID_changed: HACK"
    def on_EntryOutputBinName_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryOutputBinName_changed: HACK"
    def on_EntryPrintModeDeviceID_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryPrintModeDeviceID_changed: HACK"
    def on_EntryPrintModeLogicalCount_changed (self, window):                   print "OmniEditDeviceGammaTablesDialog::on_EntryPrintModeLogicalCount_changed: HACK"
    def on_EntryPrintModeName_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryPrintModeName_changed: HACK"
    def on_EntryPrintModePhysicalCount_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntryPrintModePhysicalCount_changed: HACK"
    def on_EntryPrintModePlanes_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryPrintModePlanes_changed: HACK"
    def on_EntryResolutionCapability_changed (self, window):                    print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionCapability_changed: HACK"
    def on_EntryResolutionCommand_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionCommand_changed: HACK"
    def on_EntryResolutionDestinationBitsPerPel_changed (self, window):         print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionDestinationBitsPerPel_changed: HACK"
    def on_EntryResolutionScanlineMultiple_changed (self, window):              print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionScanlineMultiple_changed: HACK"
    def on_EntryResolutionDeviceID_changed (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionDeviceID_changed: HACK"
    def on_EntryResolutionName_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionName_changed: HACK"
    def on_EntryResolutionOmniName_changed (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionOmniName_changed: HACK"
    def on_EntryResolutionXInternalRes_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionXInternalRes_changed: HACK"
    def on_EntryResolutionXRes_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionXRes_changed: HACK"
    def on_EntryResolutionYInternalRes_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionYInternalRes_changed: HACK"
    def on_EntryResolutionYRes_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryResolutionYRes_changed: HACK"
    def on_EntryScalingAllowedType_changed (self, window):                      print "OmniEditDeviceGammaTablesDialog::on_EntryScalingAllowedType_changed: HACK"
    def on_EntryScalingCommand_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryScalingCommand_changed: HACK"
    def on_EntryScalingDefault_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryScalingDefault_changed: HACK"
    def on_EntryScalingDeviceID_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryScalingDeviceID_changed: HACK"
    def on_EntryScalingMaximum_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryScalingMaximum_changed: HACK"
    def on_EntryScalingMinimum_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryScalingMinimum_changed: HACK"
    def on_EntryScalingDefaultPercentage_changed (self, window):                print "OmniEditDeviceGammaTablesDialog::on_EntryScalingDefaultPercentage_changed: HACK"
    def on_EntrySheetCollateCommand_changed (self, window):                     print "OmniEditDeviceGammaTablesDialog::on_EntrySheetCollateCommand_changed: HACK"
    def on_EntrySheetCollateDeviceID_changed (self, window):                    print "OmniEditDeviceGammaTablesDialog::on_EntrySheetCollateDeviceID_changed: HACK"
    def on_EntrySheetCollateName_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntrySheetCollateName_changed: HACK"
    def on_EntrySideCommand_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntrySideCommand_changed: HACK"
    def on_EntrySideDeviceID_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntrySideDeviceID_changed: HACK"
    def on_EntrySideName_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntrySideName_changed: HACK"
    def on_EntrySideSimulationRequired_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntrySideSimulationRequired_changed: HACK"
    def on_EntryStitchingAngle_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingAngle_changed: HACK"
    def on_EntryStitchingCommand_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingCommand_changed: HACK"
    def on_EntryStitchingCount_changed (self, window):                          print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingCount_changed: HACK"
    def on_EntryStitchingDeviceID_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingDeviceID_changed: HACK"
    def on_EntryStitchingPosition_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingPosition_changed: HACK"
    def on_EntryStitchingReferenceEdge_changed (self, window):                  print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingReferenceEdge_changed: HACK"
    def on_EntryStitchingType_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryStitchingType_changed: HACK"
    def on_EntryStringCountryCode_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryStringCountryCode_changed: HACK"
    def on_EntryStringKeyName_changed (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_EntryStringKeyName_changed: HACK"
    def on_EntryStringTranslation_changed (self, window):                       print "OmniEditDeviceGammaTablesDialog::on_EntryStringTranslation_changed: HACK"
    def on_EntryTrayCommand_changed (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_EntryTrayCommand_changed: HACK"
    def on_EntryTrayDeviceID_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryTrayDeviceID_changed: HACK"
    def on_EntryTrayName_changed (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_EntryTrayName_changed: HACK"
    def on_EntryTrayOmniName_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryTrayOmniName_changed: HACK"
    def on_EntryTrayTrayType_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryTrayTrayType_changed: HACK"
    def on_EntryTrimmingCommand_changed (self, window):                         print "OmniEditDeviceGammaTablesDialog::on_EntryTrimmingCommand_changed: HACK"
    def on_EntryTrimmingDeviceID_changed (self, window):                        print "OmniEditDeviceGammaTablesDialog::on_EntryTrimmingDeviceID_changed: HACK"
    def on_EntryTrimmingName_changed (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_EntryTrimmingName_changed: HACK"
    def on_MenuAbout_activate (self, window):                                   print "OmniEditDeviceGammaTablesDialog::on_MenuAbout_activate: HACK"
    def on_MenuCommands_activate (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_MenuCommands_activate: HACK"
    def on_MenuConnections_activate (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_MenuConnections_activate: HACK"
    def on_MenuCopies_activate (self, window):                                  print "OmniEditDeviceGammaTablesDialog::on_MenuCopies_activate: HACK"
    def on_MenuDatas_activate (self, window):                                   print "OmniEditDeviceGammaTablesDialog::on_MenuDatas_activate: HACK"
    def on_MenuForms_activate (self, window):                                   print "OmniEditDeviceGammaTablesDialog::on_MenuForms_activate: HACK"
    def on_MenuGammaTables_activate (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_MenuGammaTables_activate: HACK"
    def on_MenuMedias_activate (self, window):                                  print "OmniEditDeviceGammaTablesDialog::on_MenuMedias_activate: HACK"
    def on_MenuNumberUps_activate (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_MenuNumberUps_activate: HACK"
    def on_MenuOrientations_activate (self, window):                            print "OmniEditDeviceGammaTablesDialog::on_MenuOrientations_activate: HACK"
    def on_MenuOutputBins_activate (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_MenuOutputBins_activate: HACK"
    def on_MenuPrintModes_activate (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_MenuPrintModes_activate: HACK"
    def on_MenuResolutions_activate (self, window):                             print "OmniEditDeviceGammaTablesDialog::on_MenuResolutions_activate: HACK"
    def on_MenuScalings_activate (self, window):                                print "OmniEditDeviceGammaTablesDialog::on_MenuScalings_activate: HACK"
    def on_MenuSheetCollates_activate (self, window):                           print "OmniEditDeviceGammaTablesDialog::on_MenuSheetCollates_activate: HACK"
    def on_MenuSides_activate (self, window):                                   print "OmniEditDeviceGammaTablesDialog::on_MenuSides_activate: HACK"
    def on_MenuStitchings_activate (self, window):                              print "OmniEditDeviceGammaTablesDialog::on_MenuStitchings_activate: HACK"
    def on_MenuStrings_activate (self, window):                                 print "OmniEditDeviceGammaTablesDialog::on_MenuStrings_activate: HACK"
    def on_MenuTrays_activate (self, window):                                   print "OmniEditDeviceGammaTablesDialog::on_MenuTrays_activate: HACK"
    def on_MenuTrimmings_activate (self, window):                               print "OmniEditDeviceGammaTablesDialog::on_MenuTrimmings_activate: HACK"

    def on_ButtonGammaTableModify_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableModify_clicked"):
            print "OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableModify_clicked:", widget

        if self.fChanged:
            pszError = None
            if not self.gamma.setResolution (self.EntryResolution.get_text ()):
                pszError = "Invlaid resolution"
            if not self.gamma.setOmniResolution (self.EntryOmniResolution.get_text ()):
                pszError = "Invlaid omni resolution"
            if not self.gamma.setMedia (self.EntryMedia.get_text ()):
                pszError = "Invlaid media"
            if not self.gamma.setPrintMode (self.EntryPrintMode.get_text ()):
                pszError = "Invlaid print mode"
            if not self.gamma.setDitherCatagory (self.EntryDitherCatagory.get_text ()):
                pszError = "Invlaid dither catagory"
            if not self.gamma.setCGamma (self.EntryCGamma.get_text ()):
                pszError = "Invlaid cyan gamma"
            if not self.gamma.setMGamma (self.EntryMGamma.get_text ()):
                pszError = "Invlaid magenta gamma"
            if not self.gamma.setYGamma (self.EntryYGamma.get_text ()):
                pszError = "Invlaid yellow gamma"
            if not self.gamma.setKGamma (self.EntryKGamma.get_text ()):
                pszError = "Invlaid black gamma"
            if not self.gamma.setCBias (self.EntryCBias.get_text ()):
                pszError = "Invlaid cyan bias"
            if not self.gamma.setMBias (self.EntryMBias.get_text ()):
                pszError = "Invlaid magenta bias"
            if not self.gamma.setYBias (self.EntryYBias.get_text ()):
                pszError = "Invlaid yellow bias"
            if not self.gamma.setKBias (self.EntryKBias.get_text ()):
                pszError = "Invlaid black bias"

            if pszError == None:
                matchingGamma = self.findGammaTable (self.gamma)

                if     matchingGamma != None \
                   and matchingGamma != self.child:
                    ask = gtk.MessageDialog (None,
                                             0,
                                             gtk.MESSAGE_QUESTION,
                                             gtk.BUTTONS_NONE,
                                             "%s %s %s %s already exists!" % (self.gamma.getResolution (), self.gamma.getMedia (), self.gamma.getPrintMode (), self.gamma.getDitherCatagory ()))
                    ask.add_button ("_Ok", gtk.RESPONSE_YES)
                    response = ask.run ()
                    ask.destroy ()
                else:
                    self.child.setDeviceGammaTable (self.gamma)
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

    def on_ButtonGammaTableAdd_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableAdd_clicked"):
            print "OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableAdd_clicked:", widget

    def on_ButtonGammaTableDelete_clicked (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked"):
            print "OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked:", widget

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
            (rc, iGammaTablesLeft) = self.deleteGammaTable (self.child)
            if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked"):
                print "OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked: deleteGammaTable rc = %s, iGammaTablesLeft = %d" % (rc, iGammaTablesLeft)

            device    = self.gammas.getDevice ()
            dialog    = device.getDeviceDialog ()
            treestore = dialog.getTreeStore ()
            treeview  = dialog.getTreeView ()

            if     rc \
               and iGammaTablesLeft == 0:
                if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked"):
                    print "OmniEditDeviceGammaTablesDialog::on_ButtonGammaTableDelete_clicked: Last gammatable deleted"
                for row in treestore:
                    length = 0
                    for childRow in row.iterchildren ():
                        length += 1

                    if     length == 0 \
                       and row[3] == "DeviceGammaTables":
                        treestore.remove (row.iter)
                        device.setDeviceGammaTables (None)
                        dialog.setModified (True)

            treeview.get_selection ().select_iter (treestore[0].iter)

    def on_EntryGammaTableResolution_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableResolution_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableResolution_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableOmniResolution_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableOmniResolution_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableOmniResolution_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableMedia_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMedia_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMedia_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTablePrintMode_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTablePrintMode_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTablePrintMode_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableDitherCatagory_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableDitherCatagory_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableDitherCatagory_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableCGamma_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCGamma_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCGamma_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableMGamma_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMGamma_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMGamma_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableYGamma_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYGamma_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYGamma_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableKGamma_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKGamma_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKGamma_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableCBias_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCBias_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableCBias_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableMBias_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMBias_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableMBias_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableYBias_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYBias_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableYBias_changed:", widget.get_text (), widget

        self.fChanged = True

    def on_EntryGammaTableKBias_changed (self, widget):
        if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKBias_changed"):
            print "OmniEditDeviceGammaTablesDialog::on_EntryGammaTableKBias_changed:", widget.get_text (), widget

        self.fChanged = True

    def isModified (self):
        return self.fModified

    def setModified (self, fModified):
        self.fModified = fModified

    def save (self):
        self.gammas.save ()

        self.fModified = False

    def findGammaTable (self, matchGamma):
        for gamma in self.gammas.getGammaTables ():
            if     matchGamma.getResolution ()     == gamma.getResolution () \
               and matchGamma.getMedia ()          == gamma.getMedia () \
               and matchGamma.getPrintMode ()      == gamma.getPrintMode () \
               and matchGamma.getDitherCatagory () == gamma.getDitherCatagory ():
                return gamma

        return None

    def deleteGammaTable (self, matchGamma):
        foundGammaTable = None
        foundRow        = None
        gammaTables     = self.gammas.getGammaTables ()
        for gammaTable in gammaTables:
#           if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::deleteGammaTable"):
#               print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: matching1 (%s %s %s %s) vs (%s %s %s %s)" % (matchGamma.getResolution (), matchGamma.getMedia (), matchGamma.getPrintMode (), matchGamma.getDitherCatagory (), gammaTable.getResolution (), gammaTable.getMedia (), gammaTable.getPrintMode (), gammaTable.getDitherCatagory ())
            if     matchGamma.getResolution ()     == gammaTable.getResolution () \
               and matchGamma.getMedia ()          == gammaTable.getMedia () \
               and matchGamma.getPrintMode ()      == gammaTable.getPrintMode () \
               and matchGamma.getDitherCatagory () == gammaTable.getDitherCatagory ():
                foundGammaTable = gammaTable

        device    = self.gammas.getDevice ()
        dialog    = device.getDeviceDialog ()
        treestore = dialog.getTreeStore ()
        for row in treestore:
            length = 0
            for childRow in row.iterchildren ():
                length += 1

            if length > 0:
                for childRow in row.iterchildren ():
#                   if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::deleteGammaTable"):
#                       print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: matching2 %s vs %s" % (childRow[1], matchGamma)
                    if childRow[1] == matchGamma:
                        foundRow = childRow

        if     foundGammaTable != None \
           and foundRow        != None:
            if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::deleteGammaTable"):
                print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: Removing (%s %s %s %s) from gammaTables" % (foundGammaTable.getResolution (), foundGammaTable.getMedia (), foundGammaTable.getPrintMode (), foundGammaTable.getDitherCatagory ())
            gammaTables.remove (foundGammaTable)

            if shouldOutputDebug ("OmniEditDeviceGammaTablesDialog::deleteGammaTable"):
                print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: Removing %s from listbox" % (foundRow[3])
            treestore.remove (foundRow.iter)

            self.gammas.setModified (True)

            return (True, len (gammaTables))
        else:
            if foundGammaTable == None:
                print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: Error: Did not find (%s %s %s %s) in gammaTables!" % (matchGamma.getResolution (), matchGamma.getMedia (), matchGamma.getPrintMode (), matchGamma.getDitherCatagory ())
            if foundRow == None:
                print "OmniEditDeviceGammaTablesDialog::deleteGammaTable: Error: Did not find %s in listbox!" % (str (matchGamma))

        return (False, 0)

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "../XMLParser"
    else:
        rootPath = "../XMLParser"
        filename = "Epson Stylus Color 760 Gammas.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        gammas = OmniEditDeviceGammaTables (fname, doc.documentElement, None)

        print "OmniEditDeviceGammaTablesDialog::__main__: gammas = ",
        lastGamma = gammas.getGammaTables ()[-1]
        for gamma in gammas.getGammaTables ():
            gamma.printSelf (False),
            if gamma != lastGamma:
                print ",",
        print

        dialog = OmniEditDeviceGammaTableDialog (gammas, gammas.getGammaTables ()[0])
        window = OmniEditDeviceGammaTablesWindow (dialog)

        gtk.main ()

        print gammas.toXML ()

    except Exception, e:
        print "OmniEditDeviceGammaTables.py::__main__: Error: Caught", e
