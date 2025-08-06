/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2000
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "Test_Instance.hpp"
#include "defines.hpp"

#include <iostream>
#include <sstream>

DeviceInstance *
createInstance (PrintDevice *pDevice)
{
   return new Test_Instance (pDevice);
}

void
deleteInstance (DeviceInstance *pInstance)
{
   delete pInstance;
}

Test_Instance::
Test_Instance (PrintDevice *pDevice)
   : DeviceInstance (pDevice)
{
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Test_Instance::Test_Instance ()" << std::endl;
}

Test_Instance::
~Test_Instance ()
{
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Test_Instance::~Test_Instance ()" << std::endl;
}

void Test_Instance::
initializeInstance (PSZCRO pszJobProperties)
{
}

void Test_Instance::
setupPrinter ()
{
}

bool Test_Instance::
beginJob ()
{
   return false;
}

bool Test_Instance::
newFrame ()
{
   return false;
}

bool Test_Instance::
newFrame (bool fJobPropertiesChanged)
{
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "Test_Instance::newFrame (with props)" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The job properties are as follows:" << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The orientation is = " << getCurrentOrientation() << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << std::dec << "iDitherID = " << getCurrentDitherID () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The form is = " << getCurrentForm () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The tray is = " << getCurrentTray () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The media is = " << getCurrentMedia () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The resolution is = " << getCurrentResolution () << std::endl;
   if (DebugOutput::shouldOutputInstance ()) DebugOutput::getErrorStream () << "The print mode is = " << getCurrentPrintMode () << std::endl;

   // @TBD - reinitialize with new job properties

   // Call common code
   return newFrame ();
}

bool Test_Instance::
endJob ()
{
   return false;
}

bool Test_Instance::
abortJob ()
{
   return false;
}

std::string Test_Instance::
toString (std::ostringstream& oss)
{
   oss << "{ "
       << DeviceInstance::toString (oss) // Add parent's output as well
       << " }";

   return oss.str ();
}

/* Provide a way to print out class data
*/
std::ostream&
operator<< (std::ostream& os, const Test_Instance& const_self)
{
   Test_Instance&      self = const_cast<Test_Instance&>(const_self);
   std::ostringstream  oss;

   os << self.toString (oss);

   return os;
}
