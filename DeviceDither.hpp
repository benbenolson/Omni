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
#ifndef _DeviceDither
#define _DeviceDither

#define JOBPROP_DITHER "dither"

class DeviceDither
{
public:
   enum {
      DITHER_UNLISTED = -1
   };
   enum {
      DITHER_CATAGORY_UNLISTED = -1
   };

   static std::string         *getDitherValue                 (PSZRO             pszJobProperties);
   static std::string         *getDitherJobProperties         (PSZRO             pszDitherID);

                               DeviceDither                   (Device           *pDevice);
   virtual                    ~DeviceDither                   ();

   static DeviceDither        *createDitherInstance           (PSZCRO            pszDitherType,
                                                               Device           *pDevice,
                                                               PSZCRO            pszOptions);

   static std::string         *getCreateHash                  (PSZRO             pszJobProperties);
   static PSZCRO               getIDFromCreateHash            (std::string      *pstringHash);

   virtual void                ditherRGBtoCMYK                (PBITMAPINFO2      pbmi2,
                                                               PBYTE             pbStart) = 0;

   virtual bool                ditherAllPlanesBlank           () = 0;
   virtual bool                ditherCPlaneBlank              () = 0;
   virtual bool                ditherMPlaneBlank              () = 0;
   virtual bool                ditherYPlaneBlank              () = 0;
   virtual bool                ditherKPlaneBlank              () = 0;
   virtual bool                ditherLCPlaneBlank             () = 0;
   virtual bool                ditherLMPlaneBlank             () = 0;

   virtual void                newFrame                       () = 0;

   virtual BinaryData         *getCPlane                      () = 0;
   virtual BinaryData         *getMPlane                      () = 0;
   virtual BinaryData         *getYPlane                      () = 0;
   virtual BinaryData         *getKPlane                      () = 0;
   virtual BinaryData         *getLCPlane                     () = 0;
   virtual BinaryData         *getLMPlane                     () = 0;

   virtual PSZCRO              getID                          () = 0;
   PSZCRO                      getName                        ();

   static PSZCRO               getName                        (Device           *pDevice,
                                                               PSZCRO            pszJobProperties);

   static PSZCRO               IDToName                       (PSZCRO            pszId);
   static bool                 ditherNameValid                (PSZCRO            pszId);
   static bool                 ditherCatagoryValid            (PSZCRO            pszId);
   static PSZCRO               getDitherCatagory              (PSZCRO            pszId);

   static Enumeration         *getAllEnumeration              ();

protected:
   Device  *pDevice_d;
};

#endif
