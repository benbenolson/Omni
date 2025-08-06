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

class DeviceStrings:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.strings     = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmStrings = self.rootElement

        if elmStrings.nodeName != "deviceStrings":
            return "Missing <deviceStrings>, found " + elmStrings.nodeName

        if countChildren (elmStrings) < 1:
            return "At least one <deviceString> element is required"

        elmString  = firstNode (elmStrings.firstChild)
        elmStrings = nextNode (elmStrings)

        while elmString != None:
            if elmString.nodeName != "deviceString":
                return "Missing <deviceString>, found " + elmString.nodeName

            elm = firstNode (elmString.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName != "languages":
                return "Missing <languages>, found " + elm.nodeName

            if countChildren (elm) != 2:
                return "Only 2 children are allowed in a <String> element"

            elmLanguage = firstNode (elm.firstChild)
            elm         = nextNode (elm)
            languages   = []

            while elmLanguage != None:
                key   = elmLanguage.nodeName
                value = getValue (elmLanguage)

                languages.append ((key, value))

                elmLanguage = nextNode (elmLanguage)

            elmString = nextNode (elmString)

            if elm != None:
                return "Expecting no more tags in <deviceString>"

            self.strings.append ((name, languages))

        if elmStrings != None:
            return "Expecting no more tags in <deviceStrings>"

        return None

    def getStrings (self):
        return self.strings
