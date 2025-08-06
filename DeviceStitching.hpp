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
#ifndef _DeviceStitching
#define _DeviceStitching

#include "Device.hpp"

#include <iostream>
#include <sstream>
#include <string>

class DeviceStitching
{
public:
   enum {
      STITCHING_EDGE_BOTTOM = 0,
      STITCHING_EDGE_TOP,
      STITCHING_EDGE_LEFT,
      STITCHING_EDGE_RIGHT
   };
   enum {
      STITCHING_TYPE_CORNER = 0,
      STITCHING_TYPE_SADDLE,
      STITCHING_TYPE_SIDE
   };

                               DeviceStitching           (Device                *pDevice,
                                                          PSZRO                  pszJobProperties,
                                                          BinaryData            *pbdData);
   virtual                    ~DeviceStitching           ();

   static bool                 isValid                   (PSZCRO                 pszJobProperties);
   bool                        isEqual                   (PSZCRO                 pszJobProperties);

   virtual DeviceStitching    *create                    (Device                *pDevice,
                                                          PSZCRO                 pszJobProperties) = 0;

   std::string                *getCreateHash             ();
   DeviceStitching            *createWithHash            (Device                *pDevice,
                                                          PSZCRO                 pszCreateHash);

   virtual bool                isSupported               (PSZCRO                 pszJobProperties) = 0;

   bool                        handlesKey                (PSZCRO                 pszKey);

   std::string                *getJobPropertyType        (PSZCRO                 pszKey);
   std::string                *getJobProperty            (PSZCRO                 pszKey);
   std::string                *translateKeyValue         (PSZCRO                 pszKey,
                                                          PSZCRO                 pszValue);
   std::string                *getAllTranslation         ();
   std::string                *getJobProperties          (bool                   fInDeviceSpecific = false);

   BinaryData                 *getData                   ();
   virtual PSZCRO              getDeviceID               ();

   int                         getStitchingPosition      ();
   std::string                *getStitchingReferenceEdge ();
   std::string                *getStitchingType          ();
   int                         getStitchingCount         ();
   int                         getStitchingAngle         ();

   static int                  referenceEdgeIndex        (PSZCRO                 pszReferenceEdge);
   static int                  typeIndex                 (PSZCRO                 pszType);

   virtual Enumeration        *getEnumeration            (bool                   fInDeviceSpecific = false) = 0;

#ifndef RETAIL
   void                        outputSelf                ();
#endif
   virtual std::string         toString                  (std::ostringstream&    oss);
   friend std::ostream&        operator<<                (std::ostream&          os,
                                                          const DeviceStitching& self);

protected:

   static bool                 getComponents             (PSZCRO                 pszJobProperties,
                                                          int                   *iStitchingPosition,
                                                          PSZRO                 *pszStitchingReferenceEdge,
                                                          int                   *pindexStitchingReferenceEdge,
                                                          PSZRO                 *pszStitchingType,
                                                          int                   *pindexStitchingType,
                                                          int                   *iStitchingCount,
                                                          int                   *iStitchingAngle);

   Device     *pDevice_d;
   int         iStitchingPosition_d;
   PSZRO       pszStitchingReferenceEdge_d;
   int         indexStitchingReferenceEdge_d;
   PSZRO       pszStitchingType_d;
   int         indexStitchingType_d;
   int         iStitchingCount_d;
   int         iStitchingAngle_d;
   BinaryData *pbdData_d;
};

class DefaultStitching : public DeviceStitching
{
public:
   enum {
      DEFAULT_STITCHING_POSITION             = 0,
      DEFAULT_STITCHING_INDEX_REFERENCE_EDGE = 0,
      DEFAULT_STITCHING_INDEX_TYPE           = 0,
      DEFAULT_STITCHING_COUNT                = 0,
      DEFAULT_STITCHING_ANGLE                = 0
   };

                               DefaultStitching        (Device                 *pDevice,
                                                        PSZRO                   pszJobProperties);

   DeviceStitching            *create                  (Device                 *pDevice,
                                                        PSZCRO                  pszJobProperties);
   static DeviceStitching     *createS                 (Device                 *pDevice,
                                                        PSZCRO                  pszJobProperties);

   bool                        isSupported             (PSZCRO                  pszJobProperties);

   Enumeration                *getEnumeration          (bool                    fInDeviceSpecific = false);

   static void                 writeDefaultJP          (std::ostringstream&     oss);

#ifndef RETAIL
   void                        outputSelf              ();
#endif
   virtual std::string         toString                (std::ostringstream&     oss);
   friend std::ostream&        operator<<              (std::ostream&           os,
                                                        const DefaultStitching& self);
};

#endif
