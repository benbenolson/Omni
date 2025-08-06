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

class DeviceStitchings:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.stitchings  = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmStitchings = self.rootElement

        if elmStitchings.nodeName != "deviceStitchings":
            return "Missing <deviceStitchings>, found " + elmStitchings.nodeName

        if countChildren (elmStitchings) < 1:
            return "At least one <deviceStitching> element is required"

        elmStitching  = firstNode (elmStitchings.firstChild)
        elmStitchings = nextNode (elmStitchings)

        while elmStitching != None:
            if elmStitching.nodeName != "deviceStitching":
                return "Missing <deviceStitching>, found " + elmStitching.nodeName

            elm = firstNode (elmStitching.firstChild)

            if elm.nodeName != "StitchingPosition":
                return "Missing <StitchingPosition>, found " + elm.nodeName

            stitchingPosition = getValue (elm)
            elm               = nextNode (elm)

            if elm.nodeName != "StitchingReferenceEdge":
                return "Missing <StitchingReferenceEdge>, found " + elm.nodeName

            stitchingReferenceEdge = getValue (elm)
            elm                    = nextNode (elm)

            if elm.nodeName != "StitchingType":
                return "Missing <StitchingType>, found " + elm.nodeName

            stitchingType = getValue (elm)
            elm           = nextNode (elm)

            if elm.nodeName != "StitchingCount":
                return "Missing <StitchingCount>, found " + elm.nodeName

            stitchingCount = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName != "StitchingAngle":
                return "Missing <StitchingAngle>, found " + elm.nodeName

            stitchingAngle = getValue (elm)
            elm            = nextNode (elm)

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

            elmStitching = nextNode (elmStitching)

            if elm != None:
                return "Expecting no more tags in <deviceStitching>"

            self.stitchings.append ((stitchingPosition, stitchingReferenceEdge, stitchingType, stitchingCount, stitchingAngle, command, deviceID))

        if elmStitchings != None:
            return "Expecting no more tags in <deviceStitchings>"

        return None

    def getStitchings (self):
        return self.stitchings
