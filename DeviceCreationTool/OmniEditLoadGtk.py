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
#
# http://www.daa.com.au/~james/software/pygtk/
#
# python -c "import gtk; print '%d.%d.%d' % gtk.gtk_version"

import sys

def loadGtk20 (argv = None):
    if argv is None:
        argv = sys.argv

    success = False

    try:
        import pygtk

        try:
            pygtk.require ('2.0')

            try:
                import gtk

                success = True

            except Exception, e:
                print "Error: Could not load gtk! (", e, ")"

        except Exception, e:
            print "Error: Could not require 2.0 functionality from pygtk! (", e, ")"

    except Exception, e:
        print "Error: Could not load pygtk! (", e, ")"

    return success

def loadGtk12 (argv = None):
    if argv is None:
        argv = sys.argv

    success = False

    try:
        import pygtk

        try:
            pygtk.require ('1.2')

            try:
                import gtk

                success = True

            except Exception, e:
                print "Error: Could not load gtk! (", e, ")"

        except Exception, e:
            print "Error: Could not require 1.2 functionality from pygtk! (", e, ")"

    except ImportError:
        try:
            import gtk

            if hasattr (gtk, "GtkWindow"):
                success = True

        except Exception, e:
            print "Error: Could not load gtk! (", e, ")"

    except Exception, e:
        print "Error: Could not load pygtk! (", e, ")"

    return success

if __name__ == "__main__":
    sys.exit (loadGtk20 ())
