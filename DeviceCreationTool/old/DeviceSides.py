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

class DeviceSides:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.sides       = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmSides = self.rootElement

        if elmSides.nodeName != "deviceSides":
            return "Missing <deviceSides>, found " + elmSides.nodeName

        if countChildren (elmSides) < 1:
            return "At least one <deviceSide> element is required"

        elmSide  = firstNode (elmSides.firstChild)
        elmSides = nextNode (elmSides)

        while elmSide != None:
            if elmSide.nodeName != "deviceSide":
                return "Missing <deviceSide>, found " + elmSide.nodeName

            elm = firstNode (elmSide.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "simulationRequired":
                return "Missing <simulationRequired>, found " + elm.nodeName

            simulationRequired = getValue (elm)
            elm                = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmSide = nextNode (elmSide)

            if elm != None:
                return "Expecting no more tags in <deviceSide>"

            self.sides.append ((name, command, simulationRequired, deviceID))

        if elmSides != None:
            return "Expecting no more tags in <deviceSides>"

        return None

    def getSides (self):
        return self.sides
