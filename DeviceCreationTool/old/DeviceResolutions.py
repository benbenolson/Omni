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

class DeviceResolutions:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.resolutions = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmResolutions = self.rootElement

        if elmResolutions.nodeName != "deviceResolutions":
            return "Missing <deviceResolutions>, found " + elmResolutions.nodeName

        if countChildren (elmResolutions) < 1:
            return "At least one <deviceResolution> element is required"

        elmResolution  = firstNode (elmResolutions.firstChild)
        elmResolutions = nextNode (elmResolutions)

        while elmResolution != None:
            if elmResolution.nodeName != "deviceResolution":
                return "Missing <deviceResolution>, found " + elmResolution.nodeName

            elm = firstNode (elmResolution.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName == "omniName":
                elm = nextNode (elm)

            if elm.nodeName != "xRes":
                return "Missing <xRes>, found " + elm.nodeName

            xRes = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName != "yRes":
                return "Missing <yRes>, found " + elm.nodeName

            yRes = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName == "xInternalRes":
                xInternalRes = getValue (elm)
                elm          = nextNode (elm)

                if elm.nodeName != "yInternalRes":
                    return "Missing <yInternalRes>, found " + elm.nodeName

                yInternalRes = getValue (elm)
                elm          = nextNode (elm)

            else:
                xInternalRes = 0
                yInternalRes = 0

            if elm.nodeName != "command":
                return "Missing <command>, found " + elm.nodeName

            command = getCommand (elm)
            elm     = nextNode (elm)

            if elm.nodeName != "resolutionCapability":
                return "Missing <resolutionCapability>, found " + elm.nodeName

            resolutionCapability = getValue (elm)
            elm                  = nextNode (elm)

            if elm.nodeName != "resolutionDestinationBitsPerPel":
                return "Missing <resolutionDestinationBitsPerPel>, found " + elm.nodeName

            resolutionDestinationBitsPerPel = getValue (elm)
            elm                             = nextNode (elm)

            if elm.nodeName != "resolutionScanlineMultiple":
                return "Missing <resolutionScanlineMultiple>, found " + elm.nodeName

            resolutionScanlineMultiple = getValue (elm)
            elm                        = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmResolution = nextNode (elmResolution)

            if elm != None:
                return "Expecting no more tags in <deviceResolution>"

            self.resolutions.append ((name, xRes, yRes, xInternalRes, yInternalRes, command, resolutionCapability, resolutionDestinationBitsPerPel, resolutionScanlineMultiple, deviceID))

        if elmResolutions != None:
            return "Expecting no more tags in <deviceResolutions>"

        return None

    def getResolutions (self):
        return self.resolutions
