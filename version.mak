#
# IBM Omni driver
# Copyright (c) International Business Machines Corp., 2000
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
# the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

# Since the last release:
# 1. if only source code (not the interface) has changed, set
#      OMNI_MICRO_VERSION += 1;
#      OMNI_INTERFACE_AGE += 1;
# 2. if any functions have been added, removed, or changed, set
#      OMNI_INTERFACE_AGE = 0;
#      OMNI_CURRENT_INTERFACE += 1;
# 3. if interfaces have been added, set
#      OMNI_BINARY_AGE += 1;
# 4. if interfaces have been removed, set
#      OMNI_BINARY_AGE = 0;
#
# For more detailed information, see the libtool info documentation.

OMNI_MAJOR_VERSION=0
OMNI_MINOR_VERSION=9
OMNI_MICRO_VERSION=2
OMNI_EXTRA_VERSION=
OMNI_CURRENT_INTERFACE=2
OMNI_INTERFACE_AGE=0
OMNI_BINARY_AGE=0
OMNI_VERSION=${OMNI_MAJOR_VERSION}"."${OMNI_MINOR_VERSION}"."${OMNI_MICRO_VERSION}${OMNI_EXTRA_VERSION}
