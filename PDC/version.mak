# Copyright (c) 2003 International Business Machines
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Since the last release:
# 1. if only source code (not the interface) has changed, set
#      PDC_MICRO_VERSION += 1;
#      PDC_INTERFACE_AGE += 1;
# 2. if any functions have been added, removed, or changed, set
#      PDC_INTERFACE_AGE = 0;
#      PDC_CURRENT_INTERFACE += 1;
# 3. if interfaces have been added, set
#      PDC_BINARY_AGE += 1;
# 4. if interfaces have been removed, set
#      PDC_BINARY_AGE = 0;
#
# For more detailed information, see the libtool info documentation.

PDC_MAJOR_VERSION=0
PDC_MINOR_VERSION=0
PDC_MICRO_VERSION=1
PDC_EXTRA_VERSION=
PDC_CURRENT_INTERFACE=1
PDC_INTERFACE_AGE=0
PDC_BINARY_AGE=0
PDC_VERSION=${PDC_MAJOR_VERSION}"."${PDC_MINOR_VERSION}"."${PDC_MICRO_VERSION}${PDC_EXTRA_VERSION}
