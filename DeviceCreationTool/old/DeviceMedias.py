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

class DeviceMedias:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.medias      = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidName (self, name):
        return name in [ "MEDIA_ARCHIVAL_MATTE_PAPER",
                         "MEDIA_AUTO",
                         "MEDIA_BACKPRINT",
                         "MEDIA_BOND",
                         "MEDIA_BRIGHT_WHITE_INK_JET_PAPER",
                         "MEDIA_CARDBOARD",
                         "MEDIA_CARDSTOCK",
                         "MEDIA_CD_MASTER",
                         "MEDIA_CLOTH",
                         "MEDIA_COATED",
                         "MEDIA_COLOR",
                         "MEDIA_COLORLIFE_PHOTO_PAPER",
                         "MEDIA_CUSTOM_1",
                         "MEDIA_CUSTOM_2",
                         "MEDIA_CUSTOM_3",
                         "MEDIA_CUSTOM_4",
                         "MEDIA_CUSTOM_5",
                         "MEDIA_DOUBLE_SIDED_MATTE_PAPER",
                         "MEDIA_ENVELOPE",
                         "MEDIA_GLOSSY",
                         "MEDIA_HEAVYWEIGH_MATTE_PAPER",
                         "MEDIA_HIGH_GLOSS_FILM",
                         "MEDIA_HIGH_RESOLUTION",
                         "MEDIA_HP_PHOTOGRAPHIC_PAPER",
                         "MEDIA_HP_PREMIUM_PAPER",
                         "MEDIA_IRON_ON",
                         "MEDIA_LABECA",
                         "MEDIA_LABELS",
                         "MEDIA_LETTERHEAD",
                         "MEDIA_OTHER",
                         "MEDIA_PHOTOGRAPHIC_INK_JET_PAPER",
                         "MEDIA_PHOTOGRAPHIC_LABEL",
                         "MEDIA_PHOTOGRAPHIC_PAPER",
                         "MEDIA_PLAIN",
                         "MEDIA_PLAIN_ENHANCED",
                         "MEDIA_POSTCARD",
                         "MEDIA_PREMIUM_SEMIGLOSS_PHOTO_PAPER",
                         "MEDIA_PREPRINTED",
                         "MEDIA_PREPUNCHED",
                         "MEDIA_RECYCLED",
                         "MEDIA_ROUGH",
                         "MEDIA_SPECIAL",
                         "MEDIA_SPECIAL_360",
                         "MEDIA_SPECIAL_720",
                         "MEDIA_SPECIAL_BLUE",
                         "MEDIA_SPECIAL_GREEN",
                         "MEDIA_SPECIAL_GREY",
                         "MEDIA_SPECIAL_IVORY",
                         "MEDIA_SPECIAL_LETTERHEAD",
                         "MEDIA_SPECIAL_ORANGE",
                         "MEDIA_SPECIAL_PINK",
                         "MEDIA_SPECIAL_PURPLE",
                         "MEDIA_SPECIAL_RED",
                         "MEDIA_SPECIAL_USER_COLOR",
                         "MEDIA_SPECIAL_YELLOW",
                         "MEDIA_TABSTOCK",
                         "MEDIA_TABSTOCK_2",
                         "MEDIA_TABSTOCK_3",
                         "MEDIA_TABSTOCK_4",
                         "MEDIA_TABSTOCK_5",
                         "MEDIA_TABSTOCK_6",
                         "MEDIA_TABSTOCK_7",
                         "MEDIA_TABSTOCK_8",
                         "MEDIA_TABSTOCK_9",
                         "MEDIA_THERMAL",
                         "MEDIA_THICK",
                         "MEDIA_THICK_1",
                         "MEDIA_THICK_2",
                         "MEDIA_THICK_3",
                         "MEDIA_THICK_BLUE",
                         "MEDIA_THICK_GREEN",
                         "MEDIA_THICK_GREY",
                         "MEDIA_THICK_IVORY",
                         "MEDIA_THICK_LETTERHEAD",
                         "MEDIA_THICK_ORANGE",
                         "MEDIA_THICK_PINK",
                         "MEDIA_THICK_PURPLE",
                         "MEDIA_THICK_RED",
                         "MEDIA_THICK_USER_COLOR",
                         "MEDIA_THICK_YELLOW",
                         "MEDIA_TRANSLUCENT",
                         "MEDIA_TRANSPARENCY",
                         "MEDIA_USE_PRINTER_SETTING"
                        ]

    def isValidAbsorption (self, absorption):
        return absorption in [ "MEDIA_NO_ABSORPTION",
                               "MEDIA_LIGHT_ABSORPTION",
                               "MEDIA_HEAVY_ABSORPTION"
                             ]
    def isValid (self):

#       print self.__class__.__name__ + ".isValid"

        elmMedias = self.rootElement

        if elmMedias.nodeName != "deviceMedias":
            return "Missing <deviceMedias>, found " + elmMedias.nodeName

        if countChildren (elmMedias) < 1:
            return "At least one <deviceMedia> element is required"

        elmMedia  = firstNode (elmMedias.firstChild)
        elmMedias = nextNode (elmMedias)

        while elmMedia != None:
            if elmMedia.nodeName != "deviceMedia":
                return "Missing <deviceMedia>, found " + elmMedia.nodeName

            elm = firstNode (elmMedia.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if not self.isValidName (name):
                return "Invalid <name> " + name

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "mediaColorAdjustRequired":
                return "Missing <mediaColorAdjustRequired>, found " + elm.nodeName

            colorAdjustRequired = getValue (elm)
            elm                 = nextNode (elm)

            if elm.nodeName != "mediaAbsorption":
                return "Missing <mediaAbsorption>, found " + elm.nodeName

            absorption = getValue (elm)
            elm        = nextNode (elm)

            if not self.isValidAbsorption (absorption):
                return "Invalid <mediaAbsorption> " + absorption

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmMedia = nextNode (elmMedia)

            if elm != None:
                return "Expecting no more tags in <deviceMedia>"

            self.medias.append ((name, command, colorAdjustRequired, absorption, deviceID))

        if elmMedias != None:
            return "Expecting no more tags in <deviceMedias>"

        return None

    def getMedias (self):
        return self.medias

    def toXML (self):
        xmlData = XMLHeader () \
                + """

<deviceMedias xmlns="http://www.ibm.com/linux/ltc/projects/omni/"
             xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
             xs:schemaLocation="http://www.ibm.com/linux/ltc/projects/omni/ ../OmniDevice.xsd">
"""

        for media in self.getMedias ():
            (name, command, colorAdjustRequired, absorption, deviceID) = media

            xmlData += """   <deviceMedia>
      <name>""" + name + """</name>
"""

            xmlData += "      <command>" \
                     + encodeCommand (command) \
                     + """</command>
      <mediaColorAdjustRequired>""" \
                     + colorAdjustRequired \
                     + """</mediaColorAdjustRequired>
      <mediaAbsorption>""" \
                     + absorption \
                     + """</mediaAbsorption>
"""

            if deviceID != None:
                xmlData += "      <deviceID>" + deviceID + """</deviceID>
"""

            xmlData += """   </deviceMedia>
"""

        xmlData += """</deviceMedias>"""

        return xmlData

def ParseViaSAX (filename):
    try:
        file  = open (fname)

        import xml.sax
        class MyHandler (xml.sax.ContentHandler):
            def startDocument (self):
                print "startDocument"

            def endDocument (self):
                print "endDocument"

            def startElement (self, tag, attrs):
                print "startElement", tag, attrs

            def endElement (self, tag):
                print "endElement", tag

            def characters (self, data):
                print "characters (((%s))))" % data

        xml.sax.parse (file, MyHandler ())

        file.close ()

    except Exception, e:
        print "Caught", e

if __name__ == "__main__":
    import sys, os
    if 1 == len (sys.argv[1:2]):
        (rootPath, filename) = os.path.split (sys.argv[1])

        if rootPath == None:
            rootPath = "."
    else:
        rootPath = "/home/Omni/new/XMLParser"
        filename = "Epson Stylus 200 Medias.xml"

    try:
        fname = rootPath + os.sep + filename
        file  = open (fname)

        from xml.dom.ext.reader import Sax2
        # create Reader object
        reader = Sax2.Reader (False, True)

        # parse the document
        doc = reader.fromStream (file)

        file.close ()

        medias = DeviceMedias (fname, doc.documentElement)

        print "medias = ", medias.getMedias ()

        from Tkinter import *
        root = Tk ()

        root.mainloop ()

        print medias.toXML ()

    except Exception, e:
        print "Caught", e
