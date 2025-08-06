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
#ifndef _UPDFDevicePrintMode
#define _UPDFDevicePrintMode

#include <DevicePrintMode.hpp>
#include <UPDFDevice.hpp>

class UPDFDevicePrintMode : public DevicePrintMode
{
public:
                             UPDFDevicePrintMode   (Device                     *pDevice,
                                                    PSZRO                       pszJobProperties,
                                                    int                         iPhysicalCount,
                                                    int                         iLogicalCount,
                                                    int                         iPlanes,
                                                    XmlNodePtr                  node);
                            ~UPDFDevicePrintMode   ();

   virtual DevicePrintMode  *create                (Device                     *pDevice,
                                                    PSZRO                       pszJobProperties);
   static DevicePrintMode   *createS               (Device                     *pDevice,
                                                    PSZRO                       pszJobProperties);

   virtual bool              isSupported           (PSZCRO                      pszJobProperties);

   virtual PSZCRO            getDeviceID           ();

   virtual Enumeration      *getEnumeration        (bool                        fInDeviceSpecific);

   static bool               mapUPDFToOmni         (PSZCRO                      pszUPDFValue,
                                                    PSZRO                      *ppszOmniValue);
   static bool               mapOmniToUPDF         (PSZCRO                      pszOmniValue,
                                                    PSZRO                      *ppszUPDFValue);

#ifndef RETAIL
   void                      outputSelf            ();
#endif

   virtual std::string       toString              (std::ostringstream&         oss);
   friend std::ostream&      operator<<            (std::ostream&               os,
                                                    const UPDFDevicePrintMode&  self);

private:
   static XmlNodePtr         findPrintModes        (UPDFDevice                 *pUPDFDevice);

   XmlNodePtr node_d;
};

#endif
