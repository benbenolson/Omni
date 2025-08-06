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
#ifndef _PrinterCommand
#define _PrinterCommand

#include "defines.hpp"
#include "OmniServer.hpp"

#include <string>
#include <sstream>

class PrinterCommand
{
public:
                        PrinterCommand   (PSZCRO                pszProgram);
   virtual             ~PrinterCommand   ();

   bool                 setCommand       (PDC_CMD               eCommand);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          char                 *pszLine);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          PSZCRO                pszLine);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          std::string          *pstring);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          std::string           string);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          char                **pszLine);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          bool                  fValue);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          int                   iValue);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          long int              lValue);
   bool                 setCommand       (PDC_CMD               eCommand,
                                          PBYTE                 pbData,
                                          int                   cbData);
   bool                 appendCommand    (PSZCRO                pszLine);

   PPDC_PACKET          getCommand       ();
   char                *getCommandString (bool                  fNewCopy);
   bool                 getCommandBool   (bool&                 fValue);
   bool                 getCommandInt    (int&                  iValue);
   bool                 getCommandLong   (long int&             lValue);
   PBYTE                getCommandData   ();
   PDC_CMD              getCommandType   ();
   size_t               getCommandLength ();

   bool                 readCommand      (int                   fd);
   bool                 sendCommand      (int                   fd);

#ifndef RETAIL
   void                 outputSelf       ();
#endif
   virtual std::string  toString         (std::ostringstream&   oss);
   friend std::ostream& operator<<       (std::ostream&         os,
                                          const PrinterCommand& self);

private:
   bool                 resizeCommand    (size_t                iNewCmdLength);

   PSZCRO               commandToString  (PDC_CMD               eCommand);

   PPDC_PACKET      pCmd_d;
   size_t           iCurrentLength_d;
   size_t           iAllocatedLength_d;
   char            *pszProgram_d;
};

#endif
