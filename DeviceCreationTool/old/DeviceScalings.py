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

class DeviceScalings:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.scalings    = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidType (self, type):
        return type in [ "Clip",          \
                         "FitToPage",     \
                         "RotateAndOrFit" \
                       ]

    def isValid (self):

#       print self.__class__.__name__ + ".isValid"

        elmScalings = self.rootElement

        if elmScalings.nodeName != "deviceScalings":
            return "Missing <deviceScalings>, found " + elmScalings.nodeName

        if countChildren (elmScalings) < 1:
            return "At least one <deviceScaling> element is required"

        elmScaling  = firstNode (elmScalings.firstChild)
        elmScalings = nextNode (elmScalings)

        while elmScaling != None:
            if elmScaling.nodeName != "deviceScaling":
                return "Missing <deviceScaling>, found " + elmScaling.nodeName

            elm = firstNode (elmScaling.firstChild)

            if elm.nodeName != "default":
                return "Missing <default>, found " + elm.nodeName

            default = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "minimum":
                return "Missing <minimum>, found " + elm.nodeName

            minimum = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "maximum":
                return "Missing <maximum>, found " + elm.nodeName

            maximum = getValue (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "allowedType":
                return "Missing <allowedType>, found " + elm.nodeName

            allowedType = getValue (elm)
            elm         = nextNode (elm)

            if not self.isValidType (allowedType):
                return "Invalid <allowedType> " + default

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmScaling = nextNode (elmScaling)

            if elm != None:
                return "Expecting no more tags in <deviceScaling>"

            self.scalings.append ((default, minimum, maximum, command, allowedType, deviceID))

        if elmScalings != None:
            return "Expecting no more tags in <deviceScalings>"

        return None

    def getScalings (self):
        return self.scalings
