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
#ifndef _DeviceScaling
#define _DeviceScaling

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceScaling
{
public:

                               DeviceScaling           (Device              *pDevice,
                                                        PSZRO                pszJobProperties,
                                                        BinaryData          *pbdData = 0,
                                                        double               dMinimumScale = 1,
                                                        double               dMaximumScale = 100);
   virtual                    ~DeviceScaling           ();

   static bool                 isValid                 (PSZCRO               pszJobProperties);
   bool                        isEqual                 (PSZCRO               pszJobProperties);

   virtual DeviceScaling      *create                  (Device              *pDevice,
                                                        PSZCRO               pszJobProperties) = 0;

   std::string                *getCreateHash           ();
   DeviceScaling              *createWithHash          (Device              *pDevice,
                                                        PSZCRO               pszCreateHash);

   virtual bool                isSupported             (PSZCRO               pszJobProperties) = 0;

   bool                        handlesKey              (PSZCRO               pszKey);

   std::string                *getJobPropertyType      (PSZCRO               pszKey);
   std::string                *getJobProperty          (PSZCRO               pszKey);
   std::string                *translateKeyValue       (PSZCRO               pszKey,
                                                        PSZCRO               pszValue);
   std::string                *getAllTranslation       ();
   std::string                *getJobProperties        (bool                 fInDeviceSpecific = false);

   std::string                *getType                 ();
   BinaryData                 *getData                 ();
   double                      getMinimumPercentage    ();
   double                      getMaximumPercentage    ();
   virtual PSZCRO              getDeviceID             ();

   std::string                *getScalingType          ();
   double                      getScalingPercentage    ();

   static int                  allowedTypeIndex        (PSZCRO               pszAllowedType);

   virtual Enumeration        *getEnumeration          (bool                 fInDeviceSpecific = false) = 0;

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&  oss);
   friend std::ostream&        operator<<              (std::ostream&        os,
                                                        const DeviceScaling& self);

protected:

   static bool                 getComponents           (PSZCRO               pszJobProperties,
                                                        PSZRO               *ppszScalingType,
                                                        int                 *pindexScaling,
                                                        double              *pdPercentage);

   Device     *pDevice_d;
   PSZRO       pszScalingType_d;
   int         indexScaling_d;
   double      dScalingPercentage_d;
   double      dMinimumScale_d;
   double      dMaximumScale_d;
   BinaryData *pbdData_d;
};

class DefaultScaling : public DeviceScaling
{
public:
   enum {
      DEFAULT_SCALING_PERCENTAGE = 100,
      DEFAULT_SCALING_TYPE       = 1
   };
   #define DEFAULT_SCALING_MINIMUM  100.0
   #define DEFAULT_SCALING_MAXIMUM  100.0

                               DefaultScaling          (Device               *pDevice,
                                                        PSZRO                 pszJobProperties);

   DeviceScaling              *create                  (Device               *pDevice,
                                                        PSZCRO                pszJobProperties);
   static DeviceScaling       *createS                 (Device               *pDevice,
                                                        PSZCRO                pszJobProperties);

   bool                        isSupported             (PSZCRO                pszJobProperties);

   Enumeration                *getEnumeration          (bool                  fInDeviceSpecific = false);

   static void                 writeDefaultJP          (std::ostringstream&   oss);

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&   oss);
   friend std::ostream&        operator<<              (std::ostream&         os,
                                                        const DefaultScaling& self);
};

#endif
