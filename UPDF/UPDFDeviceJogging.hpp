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
#ifndef _UPDFDeviceJogging
#define _UPDFDeviceJogging

#include <DeviceJogging.hpp>
#include <UPDFDevice.hpp>

class UPDFDeviceJogging : public DeviceJogging
{
public:
                          UPDFDeviceJogging  (Device                  *pDevice,
                                              int                      id,
                                              BinaryData              *data,
                                              xmlNodePtr               node);
   virtual               ~UPDFDeviceJogging  ();

   static DeviceJogging  *create             (Device                  *pDevice,
                                              int                      id);
   static DeviceJogging  *create             (Device                  *pDevice,
                                              PSZCRO                   pszId);

   virtual PSZCRO         getDeviceID        ();

   virtual bool           isJoggingSupported (int                      id);

   virtual Enumeration   *getEnumeration     ();

   virtual std::string    toString           (std::ostringstream&      oss);
   friend std::ostream&   operator<<         (std::ostream&            os,
                                              const UPDFDeviceJogging& self);

protected:
   virtual DeviceJogging *createV            (int                      id);

private:
   static xmlNodePtr      findJoggings       (UPDFDevice              *pUPDFDevice);

   xmlNodePtr node_d;
};

#endif
