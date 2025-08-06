/*
 *   IBM Omni driver
 *   Copyright (c) International Business Machines Corp., 2002
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
#ifndef _DeviceJogging
#define _DeviceJogging

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceJogging
{
public:
   enum {
      JOGGING_UNLISTED          = -1,
      JOGGING_OFF,
      JOGGING_ON
   };
   enum {
      COUNT = JOGGING_ON - JOGGING_UNLISTED + 1
   };
                             DeviceJogging        (Device                *pDevice,
                                                   int                    id,
                                                   BinaryData            *pbdData,
                                                   bool                   fSimulationRequired = false);
   virtual                  ~DeviceJogging        ();

   static DeviceJogging     *create               (int                    id)
   {
      return (DeviceJogging *)0;
   }
   DeviceJogging            *create               (PSZCRO                 pszId);
   static int                nameToID             (PSZCRO                 pszId);
   static PSZCRO             IDToName             (int                    id);

   static PSZCRO             getName              (Device                *pDevice,
                                                   int                    id);
   PSZCRO                    getName              ();

   bool                      isID                 (int                    id);
   int                       getID                ();

   virtual PSZCRO            getDeviceID          ();
   BinaryData               *getData              ();

   bool                      needsSimulation      ();

   virtual bool              isJoggingSupported   (int                    id) = 0;

   virtual Enumeration      *getEnumeration       () = 0;

#ifndef RETAIL
   void                      outputSelf           ();
#endif
   virtual std::string       toString             (std::ostringstream&    oss);
   friend std::ostream&      operator<<           (std::ostream&          os,
                                                   const DeviceJogging&   self);

protected:
   virtual DeviceJogging    *createV              (int                    id) = 0;

   Device     *pDevice_d;
   int         id_d;
   BinaryData *pbdData_d;
   bool        fSimulationRequired_d;
};

#endif
