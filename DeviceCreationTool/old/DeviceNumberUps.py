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

class DeviceNumberUps:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.numberups   = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidDirection (self, direction):
        return direction in [ "TobottomToleft",  \
                              "TobottomToright", \
                              "ToleftTobottom",  \
                              "ToleftTotop",     \
                              "TorightTobottom", \
                              "TorightTotop",    \
                              "TotopToleft",     \
                              "TotopToright"     \
                            ]

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmNUps = self.rootElement

        if elmNUps.nodeName != "deviceNumberUps":
            return "Missing <deviceNumberUps>, found " + elmNUps.nodeName

        if countChildren (elmNUps) < 1:
            return "At least one <deviceNumberUp> element is required"

        elmNUp  = firstNode (elmNUps.firstChild)
        elmNUps = nextNode (elmNUps)

        while elmNUp != None:
            if elmNUp.nodeName != "deviceNumberUp":
                return "Missing <deviceNumberUp>, found " + elmNUp.nodeName

            elm = firstNode (elmNUp.firstChild)

            if elm.nodeName != "NumberUp":
                return "Missing <NumberUp>, found " + elm.nodeName

            if countChildren (elm) != 2:
                return "Only 2 children are allowed in a <NumberUp> element"

            elmNumberUp = firstNode (elm.firstChild)
            elm         = nextNode (elm)

            if elmNumberUp.nodeName != "x":
                return "Missing <x>, found " + elmNumberUp.nodeName

            x           = getValue (elmNumberUp)
            elmNumberUp = nextNode (elmNumberUp)

            if elmNumberUp.nodeName != "y":
                return "Missing <y>, found " + elmNumberUp.nodeName

            y = getValue (elmNumberUp)

            if elm.nodeName != "NumberUpDirection":
                return "Missing <NumberUpDirection>, found " + elm.nodeName

            numberUpDirection = getValue (elm)
            elm                           = nextNode (elm)

            if not self.isValidDirection (numberUpDirection):
                return "Invalid <numberUpDirection> " + numberUpDirection

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

            elmNUp = nextNode (elmNUp)

            if elm != None:
                return "Expecting no more tags in <deviceNumberUp>"

            self.numberups.append (((x, y), numberUpDirection, command, simulationRequired, deviceID))

        if elmNUps != None:
            return "Expecting no more tags in <deviceNumberUps>"

        return None

    def getNumberUps (self):
        return self.numberups
