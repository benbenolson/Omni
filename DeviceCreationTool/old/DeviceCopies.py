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

from Utils import *

class DeviceCopies:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmCopies = self.rootElement

        if elmCopies.nodeName != "deviceCopies":
            return "Missing <deviceCopies>, found " + elmCopies.nodeName

        if countChildren (elmCopies) != 1:
            return "Only one <deviceCopy> element allowed"

        elmCopy   = firstNode (elmCopies.firstChild)
        elmCopies = nextNode (elmCopies)

        if elmCopy.nodeName != "deviceCopy":
            return "Missing <deviceCopy>, found " + elmCopy.nodeName

        elm = firstNode (elmCopy.firstChild)

        if elm.nodeName != "minimum":
            return "Missing <minimum>, found " + elm.nodeName

        self.minimum = int (getValue (elm))

        elm = nextNode (elm)

        if elm.nodeName != "maximum":
            return "Missing <maximum>, found " + elm.nodeName

        self.maximum = int (getValue (elm))

        elm = nextNode (elm)

        if elm.nodeName != "command":
            return "Missing <command>, found " + elm.nodeName

        self.command = getCommand (elm)

        elm = nextNode (elm)

        if elm.nodeName != "simulationRequired":
            return "Missing <simulationRequired>, found " + elm.nodeName

        self.simulationRequired = getValue (elm)

        elm = nextNode (elm)

        self.deviceID = None
        if elm != None:
            if elm.nodeName != "deviceID":
                return "Missing <deviceID>, found " + elm.nodeName

            self.deviceID = getValue (elm)

            elm = nextNode (elm)

        if elm != None:
            return "Expecting no more tags in <deviceCopy>"

        if elmCopies != None:
            return "Expecting no more tags in <deviceCopies>"

        return None

    def getMinimum (self):
        return self.minimum

    def getMaximum (self):
        return self.maximum

    def getCommand (self):
        return self.command

    def getSimulationRequired (self):
        return self.simulationRequired

    def getDeviceID (self):
        return self.deviceID

    def getCopies (self):
        return (self.minimum, self.maximum, self.command, self.simulationRequired, self.deviceID)

    def getFrame (self, root):
        Label (root, text="minimum:").grid (row=0, sticky=W)
        Label (root, text="maximum:").grid (row=1, sticky=W)
        Label (root, text="command:").grid (row=2, sticky=W)
        Label (root, text="simulation required:").grid (row=3, sticky=W)
        Label (root, text="device id:").grid (row=4, sticky=W)

        entry = Entry (root)
        entry.insert (END, copies.getMinimum ())
        entry.grid (row=0, column=1)
        entry = Entry (root)
        entry.insert (END, copies.getMaximum ())
        entry.grid (row=1, column=1)
        entry = Entry (root)
        entry.insert (END, Utils.encodeCommand (copies.getCommand ()))
        entry.grid (row=2, column=1)
        entry = Entry (root)
        entry.insert (END, copies.getSimulationRequired ())
        entry.grid (row=3, column=1)
        entry = Entry (root)
        entry.insert (END, copies.getDeviceID ())
        entry.grid (row=4, column=1)

    def getLabelEntry (self, root, labelText, entryValue):
        frame = Frame (root)
        label = Label (frame, text=labelText)
        entry = Entry (frame)

        entry.insert (END, entryValue)
        label.pack (side=LEFT)
        entry.pack (side=LEFT)
        frame.pack ()

        return frame

    def getMinimumFrame (self, root):
        return self.getLabelEntry (root, "minimum:", copies.getMinimum ())

    def getMaximumFrame (self, root):
        return self.getLabelEntry (root, "maximum:", copies.getMaximum ())

    def getCommandFrame (self, root):
        return self.getLabelEntry (root, "command:", encodeCommand (copies.getCommand ()))

    def getSimulationRequiredFrame (self, root):
        return self.getLabelEntry (root, "simulation required:", copies.getSimulationRequired ())

    def getDeviceIDFrame (self, root):
        return self.getLabelEntry (root, "device id:", copies.getDeviceID ())

    def toXML (self):
        return   XMLHeader () \
               + """
<deviceCopies
   xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
   xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
   xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
   <deviceCopy>""" \
               + ("      <minimum>%d</minimum>" % self.getMinimum ()) \
               + "\n" \
               + ("      <maximum>%d</maximum>" % self.getMaximum ()) \
               + "\n" \
               + ("      <command>%s</command>" % encodeCommand (self.getCommand ())) \
               + "\n" \
               + ("      <simulationRequired>%s</simulationRequired>" % self.getSimulationRequired ()) \
               + "\n" \
               + ("      <deviceID>%s</deviceID>" % self.getDeviceID ()) \
               + "\n" \
               + """      </deviceCopy>
</deviceCopies>"""

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "."
    else:
        rootPath = "/home/Omni/new/XMLParser"
        filename = "Epson Copies.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        copies = DeviceCopies (fname, doc.documentElement)

        print "copies = ", copies.getCopies ()

        from Tkinter import *
        root = Tk ()

        if False:
            copies.getFrame (root)
        else:
            copies.getMinimumFrame (root)
            copies.getMaximumFrame (root)
            copies.getCommandFrame (root)
            copies.getSimulationRequiredFrame (root)
            copies.getDeviceIDFrame (root)

        root.mainloop ()

        print copies.toXML ()

    except Exception, e:
        print "Caught", e
