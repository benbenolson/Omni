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

class DeviceConnections:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.connections = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmConnections = self.rootElement

        if elmConnections.nodeName != "deviceConnections":
            return "Missing <deviceConnections>, found " + elmConnections.nodeName

        if countChildren (elmConnections) < 1:
            return "At least one <deviceConnection> element is required"

        elmConnection  = firstNode (elmConnections.firstChild)
        elmConnections = nextNode (elmConnections)

        while elmConnection != None:
            if elmConnection.nodeName != "deviceConnection":
                return "Missing <deviceConnection>, found " + elmConnection.nodeName

            elm = firstNode (elmConnection.firstChild)

            if elm.nodeName != "name":
                return "Missing <name>, found " + elm.nodeName

            name = getValue (elm)
            elm  = nextNode (elm)

            if elm.nodeName != "connectionForm":
                return "Missing <connectionForm>, found " + elm.nodeName

            connectionForm = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName == "omniForm":
                elm = nextNode (elm)

            if elm.nodeName != "connectionTray":
                return "Missing <connectionTray>, found " + elm.nodeName

            connectionTray = getValue (elm)
            elm            = nextNode (elm)

            if elm.nodeName == "omniTray":
                elm = nextNode (elm)

            if elm.nodeName != "connectionMedia":
                return "Missing <connectionMedia>, found " + elm.nodeName

            connectionMedia = getValue (elm)
            elm             = nextNode (elm)

            elmConnection = nextNode (elmConnection)

            if elm != None:
                return "Expecting no more tags in <deviceConnection>"

            self.connections.append ((name, connectionForm, connectionTray, connectionMedia))

        if elmConnections != None:
            return "Expecting no more tags in <deviceConnections>"

        return None

    def getConnections (self):
        return self.connections
