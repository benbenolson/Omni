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
#ifndef _XMLDeviceTrimming
#define _XMLDeviceTrimming

#include <DeviceTrimming.hpp>

#include <XMLDevice.hpp>

class XMLDeviceTrimming : public DeviceTrimming
{
public:
                               XMLDeviceTrimming       (Device                    *pDevice,
                                                        PSZRO                      pszJobProperties,
                                                        BinaryData                *pbdData,
                                                        XmlNodePtr                 node);
   virtual                    ~XMLDeviceTrimming       ();

   virtual DeviceTrimming     *create                  (Device                    *pDevice,
                                                        PSZCRO                     pszJobProperties);
   static DeviceTrimming      *createS                 (Device                    *pDevice,
                                                        PSZCRO                     pszJobProperties);

   virtual bool                isSupported             (PSZCRO                     pszJobProperties);

   virtual PSZCRO              getDeviceID             ();

   virtual Enumeration        *getEnumeration          (bool                       fInDeviceSpecific);

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&        oss);
   friend std::ostream&        operator<<              (std::ostream&              os,
                                                        const XMLDeviceTrimming&   self);
private:
   static XmlNodePtr           findTrimming            (XMLDevice                 *pXMLDevice);

   XmlNodePtr node_d;
   PSZRO      pszDeviceID_d;
};

#endif
