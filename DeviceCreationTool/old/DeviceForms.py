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

class DeviceForms:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.forms       = []

        error = self.isValid ()
        if error != None:
            e =  "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidCapabilities (self, capabilities):
        return capabilities in [ "NO_CAPABILITIES",            \
                                 "FORM_CAPABILITY_ROLL",       \
                                 "FORM_CAPABILITY_USERDEFINED" \
                               ]                               \

    def isValid (self):
#       print self.__class__.__name__ + "isValid"

        elmForms = self.rootElement

        if elmForms.nodeName != "deviceForms":
            return "Missing <deviceForms>, found " + elmForms.nodeName

        if countChildren (elmForms) < 1:
            return "At least one <deviceForm> element is required"

        elmForm  = firstNode (elmForms.firstChild)
        elmForms = nextNode (elmForms)

        while elmForm != None:
            if elmForm.nodeName != "deviceForm":
                return "Missing <deviceForm>, found " + elmForm.nodeName

            elm = firstNode (elmForm.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            omniName = None
            if elm.nodeName == "omniName":
                omniName = getValue (elm)
                elm      = nextNode (elm)

            if elm.nodeName != "formCapabilities":
                return "Missing <formCapabilities>, found " + elm.nodeName

            capabilities = getValue (elm)
            elm          = nextNode (elm)

            if not self.isValidCapabilities (capabilities):
                return "Invalid <formCapabilities>"

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "hardCopyCap":
                return "Missing <hardCopyCap>, found " + elm.nodeName

            if countChildren (elm) != 4:
                return "Found %d elements and required 4 in <hardCopyCap>." % (countChildren (elm), )

            elmHardCopyCap = firstNode (elm.firstChild)
            elm            = nextNode (elm)

            if elmHardCopyCap.nodeName != "hardCopyCapLeft":
                return "Missing <hardCopyCapLeft>, found " + elmHardCopyCap.nodeName

            hccLeft        = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapTop":
                return "Missing <hardCopyCapTop>, found " + elmHardCopyCap.nodeName

            hccTop         = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapRight":
                return "Missing <hardCopyCapRight>, found " + elmHardCopyCap.nodeName

            hccRight       = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap.nodeName != "hardCopyCapBottom":
                return "Missing <hardCopyCapBottom>, found " + elmHardCopyCap.nodeName

            hccBottom      = getValue (elmHardCopyCap)
            elmHardCopyCap = nextNode (elmHardCopyCap)

            if elmHardCopyCap != None:
                return "Expecting no more tags in <hardCopyCap>"

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmForm = nextNode (elmForm)

            if elm != None:
                return "Expecting no more tags in <deviceForm>"

            self.forms.append ((name, omniName, capabilities, command, (hccLeft, hccTop, hccRight, hccBottom), deviceID))

        if elmForms != None:
            return "Expecting no more tags in <deviceForms>"

        return None

    def getForms (self):
        return self.forms

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceForms xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for form in self.getForms ():
            (name, omniName, capabilities, command, (hccLeft, hccTop, hccRight, hccBottom), deviceID) = form

            xmlData += """   <deviceForm>
      <name>""" + name + """</name>
"""

            if omniName != None:
                xmlData += "      <omniName>" + omniName + """</omniName>
"""

            xmlData += "      <formCapabilities>" \
                     + capabilities \
                     + """</formCapabilities>
"""                  + "      <command>" \
                     + encodeCommand (command) \
                     + """</command>
      <hardCopyCap>
         <hardCopyCapLeft>""" \
                     + hccLeft \
                     + """</hardCopyCapLeft>
         <hardCopyCapTop>""" \
                     + hccTop \
                     + """</hardCopyCapTop>
         <hardCopyCapRight>""" \
                     + hccRight \
                     + """</hardCopyCapRight>
         <hardCopyCapBottom>""" \
                     + hccBottom \
                     + """</hardCopyCapBottom>
      </hardCopyCap>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceForm>
"""

        xmlData += """</deviceForms>"""

        return xmlData

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "."
    else:
        rootPath = "/home/Omni/new/XMLParser"
        filename = "Epson Stylus Color 660 Forms.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        forms = DeviceForms (fname, doc.documentElement)

        print "forms = ", forms.getForms ()

        from Tkinter import *
        root = Tk ()

        root.mainloop ()

        print forms.toXML ()

    except Exception, e:
        print "Caught", e
