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

class DeviceDatas:
    def __init__ (self, filename, rootElement):
        print 'Parsing "%s"' % filename

        self.filename    = filename
        self.rootElement = rootElement
        self.datas       = []

        error = self.isValid ()
        if error != None:
            e = "Error: '%s' is not a valid device. %s" % (filename, error)
            raise Exception, e

    def isValid (self):
#       print self.__class__.__name__ + ".isValid"

        elmDatas = self.rootElement

        if elmDatas.nodeName != "deviceDatas":
            return "Missing <deviceDatas>, found " + elmDatas.nodeName

        if countChildren (elmDatas) < 1:
            return "At least one <data> element is required"

        elmData  = firstNode (elmDatas.firstChild)
        elmDatas = nextNode (elmDatas)

        while elmData != None:
            if elmData.nodeName != "deviceData":
                return "Missing <deviceData>, found " + elmData.nodeName

            attr = elmData.attributes.getNamedItem ("name")
            if attr == None:
                return "Missing name= attribute in <data>"

            name = attr.nodeValue

            attr = elmData.attributes.getNamedItem ("type")
            if attr == None:
                return "Missing type= attribute in <data>"

            type = attr.nodeValue

            if     type != "string"    \
               and type != "boolean"   \
               and type != "integer"   \
               and type != "byte"      \
               and type != "binary"    \
               and type != "bytearray":
                return "Incorrect type=" + type + " attribute in <data>"

            data    = getValue (elmData)
            elmData = nextNode (elmData)

            self.datas.append ((name, type, data))

        if elmDatas != None:
            return "Expecting no more tags in <deviceDatas>"

        return None

    def getDatas (self):
        return self.datas
