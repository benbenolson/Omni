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

class DevicePrintModes:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.printmodes  = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValidName (self, name):
        return name in [ "PRINT_MODE_1_ANY",      \
                         "PRINT_MODE_24_CMY",     \
                         "PRINT_MODE_24_CMYK",    \
                         "PRINT_MODE_24_CcMmYK",  \
                         "PRINT_MODE_24_CcMmYyK", \
                         "PRINT_MODE_24_K",       \
                         "PRINT_MODE_24_RGB",     \
                         "PRINT_MODE_8_CMY",      \
                         "PRINT_MODE_8_CMYK",     \
                         "PRINT_MODE_8_CcMmYK",   \
                         "PRINT_MODE_8_CcMmYyK",  \
                         "PRINT_MODE_8_K",        \
                         "PRINT_MODE_8_RGB",      \
                       ]

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmPrintModes = self.rootElement

        if elmPrintModes.nodeName != "devicePrintModes":
            return "Missing <devicePrintModes>, found " + elmPrintModes.nodeName

        if countChildren (elmPrintModes) < 1:
            return "At least one <devicePrintMode> element is required"

        elmPrintMode  = firstNode (elmPrintModes.firstChild)
        elmPrintModes = nextNode (elmPrintModes)

        while elmPrintMode != None:
            if elmPrintMode.nodeName != "devicePrintMode":
                return "Missing <devicePrintMode>, found " + elmPrintMode.nodeName

            elm = firstNode (elmPrintMode.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if not self.isValidName (name):
                return "Invalid <name> " + name

            if elm.nodeName != "printModePhysicalCount":
                return "Missing <printModePhysicalCount>, found " + elm.nodeName

            printModePhysicalCount = getValue (elm)
            elm                    = nextNode (elm)

            if elm.nodeName != "printModeLogicalCount":
                return "Missing <printModeLogicalCount>, found " + elm.nodeName

            printModeLogicalCount = getValue (elm)
            elm                   = nextNode (elm)

            if elm.nodeName != "printModePlanes":
                return "Missing <printModePlanes>, found " + elm.nodeName

            printModePlanes = getValue (elm)
            elm             = nextNode (elm)

            deviceID = None
            if elm != None:
                if elm.nodeName != "deviceID":
                    return "Missing <deviceID>, found " + elm.nodeName

                deviceID = getValue (elm)
                elm      = nextNode (elm)

            elmPrintMode = nextNode (elmPrintMode)

            if elm != None:
                return "Expecting no more tags in <devicePrintMode>"

            self.printmodes.append ((name, printModePhysicalCount, printModeLogicalCount, printModePlanes, deviceID))

        if elmPrintModes != None:
            return "Expecting no more tags in <devicePrintModes>"

        return None

    def getPrintModes (self):
        return self.printmodes
