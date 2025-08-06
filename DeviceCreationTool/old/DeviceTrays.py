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

class DeviceTrays:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.trays       = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmTrays = self.rootElement

        if elmTrays.nodeName != "deviceTrays":
            return "Missing <deviceTrays>, found " + elmTrays.nodeName

        if countChildren (elmTrays) < 1:
            return "At least one <deviceTray> element is required"

        elmTray  = firstNode (elmTrays.firstChild)
        elmTrays = nextNode (elmTrays)

        while elmTray != None:
            if elmTray.nodeName != "deviceTray":
                return "Missing <deviceTray>, found " + elmTray.nodeName

            elm = firstNode (elmTray.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName == "omniName":
                elm = nextNode (elm)

            if elm.nodeName != "trayType":
                return "Missing <trayType>, found " + elm.nodeName

            trayType = getValue (elm)
            elm      = nextNode (elm)

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmTray = nextNode (elmTray)

            if elm != None:
                return "Expecting no more tags in <deviceTray>"

            self.trays.append ((name, trayType, command, deviceID))

        if elmTrays != None:
            return "Expecting no more tags in <deviceTrays>"

        return None

    def getTrays (self):
        return self.trays
