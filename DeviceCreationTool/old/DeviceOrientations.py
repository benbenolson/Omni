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

class DeviceOrientations:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename     = filename
        self.rootElement  = rootElement
        self.orientations = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidName (self, name):
        return name in [ "Landscape",        \
                         "Portrait",         \
                         "ReverseLandscape", \
                         "ReversePortrait"   \
                       ]

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmOrientations = self.rootElement

        if elmOrientations.nodeName != "deviceOrientations":
            return "Missing <deviceOrientations>, found " + elmOrientations.nodeName

        if countChildren (elmOrientations) < 1:
            return "At least one <deviceOrientation> element is required"

        elmOrientation  = firstNode (elmOrientations.firstChild)
        elmOrientations = nextNode (elmOrientations)

        while elmOrientation != None:
            if elmOrientation.nodeName != "deviceOrientation":
                return "Missing <deviceOrientation>, found " + elmOrientation.nodeName

            elm = firstNode (elmOrientation.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if not self.isValidName (name):
                return "Invalid <name> " + name

            if elm.nodeName == "omniName":
                elm = nextNode (elm)

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

            elmOrientation = nextNode (elmOrientation)

            if elm != None:
                return "Expecting no more tags in <deviceOrientation>"

            self.orientations.append ((name, simulationRequired, deviceID))

        if elmOrientations != None:
            return "Expecting no more tags in <deviceOrientations>"

        return None

    def getOrientations (self):
        return self.orientations
