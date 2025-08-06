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
#ifndef _DeviceOutputBin
#define _DeviceOutputBin

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceOutputBin
{
public:

                               DeviceOutputBin         (Device                  *pDevice,
                                                        PSZRO                    pszJobProperties,
                                                        BinaryData              *pbdData);
   virtual                    ~DeviceOutputBin         ();

   static bool                 isValid                 (PSZCRO                   pszJobProperties);
   bool                        isEqual                 (PSZCRO                   pszJobProperties);

   virtual DeviceOutputBin    *create                  (Device                  *pDevice,
                                                        PSZCRO                   pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceOutputBin            *createWithHash          (Device                  *pDevice,
                                                        PSZCRO                   pszCreateHash);

   virtual bool                isSupported             (PSZCRO                   pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO                   pszKey);

   std::string                *getJobPropertyType      (PSZCRO                   pszKey);
   std::string                *getJobProperty          (PSZCRO                   pszKey);
   std::string                *translateKeyValue       (PSZCRO                   pszKey,
                                                        PSZCRO                   pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                     fInDeviceSpecific = false);

   BinaryData                 *getData                 ();
   virtual PSZCRO              getDeviceID             ();

   std::string                *getOutputBin            ();

   virtual Enumeration        *getEnumeration          (bool                     fInDeviceSpecific = false) = 0;
   static Enumeration         *getAllEnumeration       ();

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&      oss);
   friend std::ostream&        operator<<              (std::ostream&            os,
                                                        const DeviceOutputBin&   self);

protected:

   static bool                 getComponents           (PSZCRO                   pszJobProperties,
                                                        PSZRO                   *ppszOutputBin,
                                                        int                     *pindexOutputBin);

   Device     *pDevice_d;
   PSZRO       pszOutputBin_d;
   int         indexOutputBin_d;
   BinaryData *pbdData_d;
};

class DefaultOutputBin : public DeviceOutputBin
{
public:
   enum {
      DEFAULT_INDEX_OUTPUT_BIN = 1
   };

                               DefaultOutputBin        (Device                 *pDevice,
                                                        PSZRO                   pszJobProperties);

   DeviceOutputBin            *create                  (Device                 *pDevice,
                                                        PSZCRO                  pszJobProperties);
   static DeviceOutputBin     *createS                 (Device                 *pDevice,
                                                        PSZCRO                  pszJobProperties);

   bool                        isSupported             (PSZCRO                  pszJobProperties);

   Enumeration                *getEnumeration          (bool                    fInDeviceSpecific = false);

   static void                 writeDefaultJP          (std::ostringstream&     oss);

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&     oss);
   friend std::ostream&        operator<<              (std::ostream&           os,
                                                        const DefaultOutputBin& self);
};

#endif
