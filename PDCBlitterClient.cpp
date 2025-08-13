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
#include "PDCBlitterClient.hpp"
#include "Omni.hpp"
#include "JobProperties.hpp"

#include <cstdio>
#include <stdlib.h>
#include <memory.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/shm.h>
#include <errno.h>

#ifdef INCLUDE_JP_UPDF_BOOKLET

class BCDeviceBooklet : public DeviceBooklet
{
public:
   BCDeviceBooklet (Device     *pDevice,
                    PSZRO       pszJobProperties,
                    PSZRO       pszID,
                    BinaryData *pbdData)
      : DeviceBooklet (pDevice,
                       pszJobProperties,
                       pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceBooklet ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceBooklet *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceBooklet:"
          << DeviceBooklet::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&          os,
               const BCDeviceBooklet& const_self)
   {
      BCDeviceBooklet&   self = const_cast<BCDeviceBooklet&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

#ifdef INCLUDE_JP_COMMON_COPIES

class BCDeviceCopies : public DeviceCopies
{
public:
   BCDeviceCopies (Device     *pDevice,
                   PSZRO       pszJobProperties,
                   PSZRO       pszID,
                   BinaryData *pbdData,
                   int         iMinimum = 1,
                   int         iMaximum = 65536,
                   bool        fSimulationRequired = false)
      : DeviceCopies (pDevice,
                      pszJobProperties,
                      pbdData,
                      iMinimum,
                      iMaximum,
                      fSimulationRequired)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceCopies ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceCopies *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceCopies:"
          << DeviceCopies::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&         os,
               const BCDeviceCopies& const_self)
   {
      BCDeviceCopies&    self = const_cast<BCDeviceCopies&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDeviceForm : public DeviceForm
{
public:
   BCDeviceForm (Device      *pDevice,
                 PSZRO        pszJobProperties,
                 PSZRO        pszID,
                 int          iCapabilities,
                 BinaryData  *data,
                 HardCopyCap *hcInfo)
      : DeviceForm (pDevice,
                    pszJobProperties,
                    iCapabilities,
                    data,
                    hcInfo)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceForm ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceForm *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceForm:"
          << DeviceForm::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&       os,
               const BCDeviceForm& const_self)
   {
      BCDeviceForm&      self = const_cast<BCDeviceForm&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#ifdef INCLUDE_JP_UPDF_JOGGING

class BCDeviceJogging : public DeviceJogging
{
public:
   BCDeviceJogging (Device     *pDevice,
                    PSZRO       pszJobProperties,
                    PSZRO       pszID,
                    BinaryData *pbdData)
      : DeviceJogging (pDevice,
                       pszJobProperties,
                       pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceJogging ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceJogging *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceJogging:"
          << DeviceJogging::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&          os,
               const BCDeviceJogging& const_self)
   {
      BCDeviceJogging&   self = const_cast<BCDeviceJogging&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDeviceMedia : public DeviceMedia
{
public:
   BCDeviceMedia (Device     *pDevice,
                  PSZRO       pszJobProperties,
                  PSZRO       pszID,
                  BinaryData *pbdData,
                  int         iColorAdjustRequired,
                  int         iAbsorption)
      : DeviceMedia (pDevice,
                     pszJobProperties,
                     pbdData,
                     iColorAdjustRequired,
                     iAbsorption)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceMedia ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceMedia *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceMedia:"
          << DeviceMedia::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&        os,
               const BCDeviceMedia& const_self)
   {
      BCDeviceMedia&     self = const_cast<BCDeviceMedia&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#ifdef INCLUDE_JP_COMMON_NUP

class BCDeviceNUp : public DeviceNUp
{
public:
   BCDeviceNUp (Device     *pDevice,
                PSZRO       pszJobProperties,
                PSZRO       pszID,
                BinaryData *pbdData,
                bool        fSimulationRequired = false)
      : DeviceNUp (pDevice,
                   pszJobProperties,
                   pbdData,
                   fSimulationRequired)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceNUp ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceNUp *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceNUp:"
          << DeviceNUp::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&      os,
               const BCDeviceNUp& const_self)
   {
      BCDeviceNUp&       self = const_cast<BCDeviceNUp&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDeviceOrientation : public DeviceOrientation
{
public:
   BCDeviceOrientation (Device *pDevice,
                        PSZRO   pszJobProperties,
                        PSZRO   pszID,
                        bool    fSimulationRequired = false)
      : DeviceOrientation (pDevice,
                           pszJobProperties,
                           fSimulationRequired)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceOrientation ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceOrientation *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceOrientation:"
          << DeviceOrientation::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&              os,
               const BCDeviceOrientation& const_self)
   {
      BCDeviceOrientation& self = const_cast<BCDeviceOrientation&>(const_self);
      std::ostringstream   oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

class BCDeviceOutputBin : public DeviceOutputBin
{
public:
   BCDeviceOutputBin (Device     *pDevice,
                      PSZRO       pszJobProperties,
                      PSZRO       pszID,
                      BinaryData *pbdData)
      : DeviceOutputBin (pDevice,
                         pszJobProperties,
                         pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceOutputBin ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceOutputBin *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceOutputBin:"
          << DeviceOutputBin::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&            os,
               const BCDeviceOutputBin& const_self)
   {
      BCDeviceOutputBin& self = const_cast<BCDeviceOutputBin&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDevicePrintMode : public DevicePrintMode
{
public:
   BCDevicePrintMode (Device *pDevice,
                      PSZRO   pszJobProperties,
                      PSZRO   pszID,
                      int     iPhysicalCount,
                      int     iLogicalCount,
                      int     iPlanes)
      : DevicePrintMode (pDevice,
                         pszJobProperties,
                         iPhysicalCount,
                         iLogicalCount,
                         iPlanes)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDevicePrintMode ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DevicePrintMode *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDevicePrintMode:"
          << DevicePrintMode::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&            os,
               const BCDevicePrintMode& const_self)
   {
      BCDevicePrintMode& self = const_cast<BCDevicePrintMode&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

class BCDeviceResolution : public DeviceResolution
{
public:
   BCDeviceResolution (Device     *pDevice,
                       PSZRO       pszJobProperties,
                       PSZRO       pszID,
                       int         iXInternalRes,
                       int         iYInternalRes,
                       BinaryData *pbdData,
                       int         iCapabilities,
                       int         iDestinationBitsPerPel,
                       int         iScanlineMultiple)
      : DeviceResolution (pDevice,
                          pszJobProperties,
                          iXInternalRes,
                          iYInternalRes,
                          pbdData,
                          iCapabilities,
                          iDestinationBitsPerPel,
                          iScanlineMultiple)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceResolution ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceResolution *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceResolution:"
          << DeviceResolution::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&             os,
               const BCDeviceResolution& const_self)
   {
      BCDeviceResolution& self = const_cast<BCDeviceResolution&>(const_self);
      std::ostringstream  oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#ifdef INCLUDE_JP_COMMON_SCALING

class BCDeviceScaling : public DeviceScaling
{
public:
   BCDeviceScaling (Device     *pDevice,
                    PSZRO       pszJobProperties,
                    PSZRO       pszID,
                    BinaryData *pbdData,
                    int         iMinimumScale = 1,
                    int         iMaximumScale = 100)
      : DeviceScaling (pDevice,
                       pszJobProperties,
                       pbdData,
                       iMinimumScale,
                       iMaximumScale)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceScaling ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceScaling *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceScaling:"
          << DeviceScaling::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&          os,
               const BCDeviceScaling& const_self)
   {
      BCDeviceScaling&   self = const_cast<BCDeviceScaling&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

class BCDeviceSheetCollate : public DeviceSheetCollate
{
public:
   BCDeviceSheetCollate (Device     *pDevice,
                         PSZRO       pszJobProperties,
                         PSZRO       pszID,
                         BinaryData *pbdData)
      : DeviceSheetCollate (pDevice,
                            pszJobProperties,
                            pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceSheetCollate ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceSheetCollate *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceSheetCollate:"
          << DeviceSheetCollate::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&               os,
               const BCDeviceSheetCollate& const_self)
   {
      BCDeviceSheetCollate& self = const_cast<BCDeviceSheetCollate&>(const_self);
      std::ostringstream    oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

class BCDeviceSide : public DeviceSide
{
public:
   BCDeviceSide (Device     *pDevice,
                 PSZRO       pszJobProperties,
                 PSZRO       pszID,
                 BinaryData *pbdData,
                 bool        fSimulationRequired = false)
      : DeviceSide (pDevice,
                    pszJobProperties,
                    pbdData,
                    fSimulationRequired)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceSide ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceSide *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceSide:"
          << DeviceSide::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&       os,
               const BCDeviceSide& const_self)
   {
      BCDeviceSide&      self = const_cast<BCDeviceSide&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

class BCDeviceStitching : public DeviceStitching
{
public:
   BCDeviceStitching (Device     *pDevice,
                      PSZRO       pszJobProperties,
                      PSZRO       pszID,
                      BinaryData *pbdData)
      : DeviceStitching (pDevice,
                         pszJobProperties,
                         pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceStitching ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceStitching *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceStitching:"
          << DeviceStitching::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&            os,
               const BCDeviceStitching& const_self)
   {
      BCDeviceStitching& self = const_cast<BCDeviceStitching&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDeviceTray : public DeviceTray
{
public:
   BCDeviceTray (Device     *pDevice,
                 PSZRO       pszJobProperties,
                 PSZRO       pszID,
                 int         iType,
                 BinaryData *pbdData)
      : DeviceTray (pDevice,
                    pszJobProperties,
                    iType,
                    pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceTray ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceTray *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceTray:"
          << DeviceTray::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&       os,
               const BCDeviceTray& const_self)
   {
      BCDeviceTray&      self = const_cast<BCDeviceTray&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#ifdef INCLUDE_JP_COMMON_TRIMMING

class BCDeviceTrimming : public DeviceTrimming
{
public:
   BCDeviceTrimming (Device     *pDevice,
                     PSZRO       pszJobProperties,
                     PSZRO       pszID,
                     BinaryData *pbdData)
      : DeviceTrimming (pDevice,
                        pszJobProperties,
                        pbdData)
   {
      pszID_d = 0;

      if (  pszID
         && *pszID
         )
      {
         pszID_d = (PSZRO)malloc (strlen (pszID) + 1);
         if (pszID_d)
         {
            strcpy ((char *)pszID_d, pszID);
         }
      }
   }

   virtual ~
   BCDeviceTrimming ()
   {
      if (pszID_d)
      {
         free ((void *)pszID_d);
         pszID_d = 0;
      }
   }

   DeviceTrimming *
   create (Device *pDevice, PSZCRO pszJobProperties)
   {
      return 0;
   }

   bool
   isSupported (PSZCRO pszJobProperties)
   {
      return false;
   }

   PSZCRO
   getDeviceID ()
   {
      return pszID_d;
   }

   Enumeration *
   getEnumeration (bool fInDeviceSpecific = false)
   {
      return 0;
   }

   std::string
   toString (std::ostringstream& oss)
   {
      std::ostringstream oss2;

      oss << "{BCDeviceTrimming:"
          << DeviceTrimming::toString (oss2)
          << "}";

      return oss.str ();
   }

   friend std::ostream&
   operator<< (std::ostream&           os,
               const BCDeviceTrimming& const_self)
   {
      BCDeviceTrimming&  self = const_cast<BCDeviceTrimming&>(const_self);
      std::ostringstream oss;

      os << self.toString (oss);

      return os;
   }

private:
   PSZRO pszID_d;
};

#endif

class BCDeviceGamma : public DeviceGamma
{
public:
   BCDeviceGamma (int iCGamma,
                  int iMGamma,
                  int iYGamma,
                  int iKGamma,
                  int iCBias,
                  int iMBias,
                  int iYBias,
                  int iKBias)
   : DeviceGamma (iCGamma,
                  iMGamma,
                  iYGamma,
                  iKGamma,
                  iCBias,
                  iMBias,
                  iYBias,
                  iKBias)
   {
   }
};

DeviceObjectHolder::
DeviceObjectHolder ()
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   pBooklet_d      = 0;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   pCopies_d       = 0;
#endif
   pForm_d         = 0;
#ifdef INCLUDE_JP_UPDF_JOGGING
   pJogging_d      = 0;
#endif
   pMedia_d        = 0;
#ifdef INCLUDE_JP_COMMON_NUP
   pNUp_d          = 0;
#endif
   pOrientation_d  = 0;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   pOutputBin_d    = 0;
#endif
   pPrintMode_d    = 0;
   pResolution_d   = 0;
#ifdef INCLUDE_JP_COMMON_SCALING
   pScaling_d      = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   pSheetCollate_d = 0;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   pSide_d         = 0;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   pStitching_d    = 0;
#endif
   pTray_d         = 0;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   pTrimming_d     = 0;
#endif
   pGamma_d        = 0;
}

DeviceObjectHolder::
~DeviceObjectHolder ()
{
#ifdef INCLUDE_JP_UPDF_BOOKLET
   delete pBooklet_d;
#endif
#ifdef INCLUDE_JP_COMMON_COPIES
   delete pCopies_d;
#endif
   delete pForm_d;
#ifdef INCLUDE_JP_UPDF_JOGGING
   delete pJogging_d;
#endif
   delete pMedia_d;
#ifdef INCLUDE_JP_COMMON_NUP
   delete pNUp_d;
#endif
   delete pOrientation_d;
#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
   delete pOutputBin_d;
#endif
   delete pPrintMode_d;
   delete pResolution_d;
#ifdef INCLUDE_JP_COMMON_SCALING
   delete pScaling_d;
#endif
#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
   delete pSheetCollate_d;
#endif
#ifdef INCLUDE_JP_COMMON_SIDE
   delete pSide_d;
#endif
#ifdef INCLUDE_JP_COMMON_STITCHING
   delete pStitching_d;
#endif
   delete pTray_d;
#ifdef INCLUDE_JP_COMMON_TRIMMING
   delete pTrimming_d;
#endif
   delete pGamma_d;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET

DeviceBooklet * DeviceObjectHolder::
getCurrentBooklet ()
{
   return pBooklet_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_COPIES

DeviceCopies * DeviceObjectHolder::
getCurrentCopies ()
{
   return pCopies_d;
}

#endif

DeviceForm * DeviceObjectHolder::
getCurrentForm ()
{
   return pForm_d;
}

#ifdef INCLUDE_JP_UPDF_JOGGING

DeviceJogging * DeviceObjectHolder::
getCurrentJogging ()
{
   return pJogging_d;
}

#endif

DeviceMedia * DeviceObjectHolder::
getCurrentMedia ()
{
   return pMedia_d;
}

#ifdef INCLUDE_JP_COMMON_NUP

DeviceNUp * DeviceObjectHolder::
getCurrentNUp ()
{
   return pNUp_d;
}

#endif

DeviceOrientation * DeviceObjectHolder::
getCurrentOrientation ()
{
   return pOrientation_d;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

DeviceOutputBin * DeviceObjectHolder::
getCurrentOutputBin ()
{
   return pOutputBin_d;
}

#endif

DevicePrintMode * DeviceObjectHolder::
getCurrentPrintMode ()
{
   return pPrintMode_d;
}

DeviceResolution * DeviceObjectHolder::
getCurrentResolution ()
{
   return pResolution_d;
}

#ifdef INCLUDE_JP_COMMON_SCALING

DeviceScaling * DeviceObjectHolder::
getCurrentScaling ()
{
   return pScaling_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

DeviceSheetCollate * DeviceObjectHolder::
getCurrentSheetCollate ()
{
   return pSheetCollate_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

DeviceSide * DeviceObjectHolder::
getCurrentSide ()
{
   return pSide_d;
}

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

DeviceStitching * DeviceObjectHolder::
getCurrentStitching ()
{
   return pStitching_d;
}

#endif

DeviceTray * DeviceObjectHolder::
getCurrentTray ()
{
   return pTray_d;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING

DeviceTrimming * DeviceObjectHolder::
getCurrentTrimming ()
{
   return pTrimming_d;
}

#endif

DeviceGamma * DeviceObjectHolder::
getCurrentGamma ()
{
   return pGamma_d;
}

#ifdef INCLUDE_JP_UPDF_BOOKLET

void DeviceObjectHolder::
setCurrentBooklet (DeviceBooklet *pBooklet)
{
   delete pBooklet_d;
   pBooklet_d = pBooklet;
}

#endif

#ifdef INCLUDE_JP_COMMON_COPIES

void DeviceObjectHolder::
setCurrentCopies (DeviceCopies *pCopies)
{
   delete pCopies_d;
   pCopies_d = pCopies;
}

#endif

void DeviceObjectHolder::
setCurrentForm (DeviceForm *pForm)
{
   delete pForm_d;
   pForm_d = pForm;
}

#ifdef INCLUDE_JP_UPDF_JOGGING

void DeviceObjectHolder::
setCurrentJogging (DeviceJogging *pJogging)
{
   delete pJogging_d;
   pJogging_d = pJogging;
}

#endif

void DeviceObjectHolder::
setCurrentMedia (DeviceMedia *pMedia)
{
   delete pMedia_d;
   pMedia_d = pMedia;
}

#ifdef INCLUDE_JP_COMMON_NUP

void DeviceObjectHolder::
setCurrentNUp (DeviceNUp *pNUp)
{
   delete pNUp_d;
   pNUp_d = pNUp;
}

#endif

void DeviceObjectHolder::
setCurrentOrientation (DeviceOrientation *pOrientation)
{
   delete pOrientation_d;
   pOrientation_d = pOrientation;
}

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN

void DeviceObjectHolder::
setCurrentOutputBin (DeviceOutputBin *pOutputBin)
{
   delete pOutputBin_d;
   pOutputBin_d = pOutputBin;
}

#endif

void DeviceObjectHolder::
setCurrentPrintMode (DevicePrintMode *pPrintMode)
{
   delete pPrintMode_d;
   pPrintMode_d = pPrintMode;
}

void DeviceObjectHolder::
setCurrentResolution (DeviceResolution *pResolution)
{
   delete pResolution_d;
   pResolution_d = pResolution;
}

#ifdef INCLUDE_JP_COMMON_SCALING

void DeviceObjectHolder::
setCurrentScaling (DeviceScaling *pScaling)
{
   delete pScaling_d;
   pScaling_d = pScaling;
}

#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE

void DeviceObjectHolder::
setCurrentSheetCollate (DeviceSheetCollate *pSheetCollate)
{
   delete pSheetCollate_d;
   pSheetCollate_d = pSheetCollate;
}

#endif

#ifdef INCLUDE_JP_COMMON_SIDE

void DeviceObjectHolder::
setCurrentSide (DeviceSide *pSide)
{
   delete pSide_d;
   pSide_d = pSide;
}

#endif

#ifdef INCLUDE_JP_COMMON_STITCHING

void DeviceObjectHolder::
setCurrentStitching (DeviceStitching *pStitching)
{
   delete pStitching_d;
   pStitching_d = pStitching;
}

#endif

void DeviceObjectHolder::
setCurrentTray (DeviceTray *pTray)
{
   delete pTray_d;
   pTray_d = pTray;
}

#ifdef INCLUDE_JP_COMMON_TRIMMING

void DeviceObjectHolder::
setCurrentTrimming (DeviceTrimming *pTrimming)
{
   delete pTrimming_d;
   pTrimming_d = pTrimming;
}

#endif

void DeviceObjectHolder::
setCurrentGamma (DeviceGamma *pGamma)
{
   delete pGamma_d;
   pGamma_d = pGamma;
}

PSZCRO
findSpace (PSZRO pszStart,
           int   iNumSpaces)
{
   PSZRO pszSpace = 0;

   while (0 < iNumSpaces)
   {
      if (  !pszStart
         || !*pszStart
         )
      {
         return 0;
      }

      pszSpace = strchr (pszStart, ' ');

      if (pszSpace)
      {
         pszStart = pszSpace + 1;
         iNumSpaces--;
      }
      else
      {
         break;
      }
   }

   if (pszSpace)
   {
      return pszSpace + 1;
   }
   else
   {
      return 0;
   }
}

int
main (int argc, char *argv[])
{
   char               *pszDeviceName         = 0;
   char               *pszS2C;
   char               *pszC2S;
   int                 fdS2C                 = -1;            /* server to client */
   int                 fdC2S                 = -1;            /* client to server */
   bool                fRun                  = true;
   PDC_CMD             eCommand;
   PrinterCommand     *pCmd                  = 0;
   char               *pszJobProperties      = 0;
   size_t              cbJobProperties       = 0;
   bool                fNewJobPropertiesSent = false;
   byte               *pbBuffer1             = 0;
   byte               *pbBuffer2             = 0;
   HANDLE              hInstance             = 0;
   GModule            *hmodDevice            = 0;
   int                 iCurrentLanguage      = StringResource::LANGUAGE_DEFAULT;
   StringResource     *pResource             = StringResource::create (iCurrentLanguage, 0);
   PDC_B_HOOKPOINTS    HookPoints;
   DeviceObjectHolder  DOH;
   FILE               *fpError               = 0;
   PSZRO               pszDriverName         = 0;

   Omni::initialize ();

#ifndef RETAIL
   {
      std::ostringstream oss;

      std::cerr << "Executing ";

      oss << "which ";
      oss << argv[0];
      oss << " 1>&2";

      std::cerr.flush ();

      std::string str (oss.str ());

      system (str.c_str ());
   }
#endif

   memset (&HookPoints, 0, sizeof (HookPoints));
   initializePDCBlitter (&HookPoints);

   if (getenv ("OMNI_PDC_BLITTER_PAUSE"))
   {
      // Handy to attach via gdb debugger.  do:
      // (gdb) file OmniEpsonBlitter
      // (gdb) shell ps -efl | grep OmniEpsonBlitter          to get the <pid>.
      // (gdb) attach <pid>
      // (gdb) set fPause = 0
      // (gdb) c
      bool fPause = true;

      while (fPause)
      {
      }
   }

   pszDriverName = HookPoints.pfnGetDriverName ();

   /* Get local copy. */
   pszS2C = getenv ("OMNI_BLITTER_S2C");
   pszC2S = getenv ("OMNI_BLITTER_C2S");

   if (  !pszS2C
      || !pszC2S
      )
   {
      DebugOutput::getErrorStream () << argv[0] << " must have OMNI_BLITTER_S2C and OMNI_BLITTER_C2S set in the environment!" << std::endl;

      return 1;
   }

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCBlitterClient ())
   {
      DebugOutput::getErrorStream () << "PDCBlitterClient: (" << pszDriverName << ") OMNI_BLITTER_S2C=" << pszS2C << std::endl;
      DebugOutput::getErrorStream () << "PDCBlitterClient: (" << pszDriverName << ") OMNI_BLITTER_C2S=" << pszC2S << std::endl;
      DebugOutput::setOutputPrinterCommand (true);
   }
#endif

   /* Open fifos. */
   if ((fdS2C = open (pszS2C, O_WRONLY)) < 0)
   {
      goto BUGOUT;
   }

   if ((fdC2S = open (pszC2S, O_RDONLY)) < 0)
   {
      goto BUGOUT;
   }

   pCmd = new PrinterCommand (pszDriverName);

#ifndef RETAIL
   if (DebugOutput::shouldOutputPDCBlitterClient ()) DebugOutput::getErrorStream () << "PDCBlitterClient: (" << pszDriverName << ") ready." << std::endl;
#endif

   while (fRun)
   {
      static char *pszErrorUnexpectedANU  = "Unexpected ack/nack/unsupported command";
      static char *pszErrorUnknownCommand = "Unknown command";

      if (!pCmd->readCommand (fdC2S))
         goto BUGOUT;

      eCommand = PDCCMD_NACK;

      switch (pCmd->getCommandType ())
      {
      case PDCCMD_ACK:
      case PDCCMD_NACK:
      case PDCCMD_UNSUPPORTED:
      {
         if (  !pCmd->setCommand (PDCCMD_NACK, pszErrorUnexpectedANU)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_INITIALIZE_SESSION:
      {
         if (  !pCmd->setCommand (PDCCMD_ACK, PDC_VERSION)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_CLOSE_SESSION:
      {
         fRun = false;
         break;
      }

      case PDCMD_SET_TRANSLATABLE_LANGUAGE:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            // @TBD need architecture here
            int iLangIn = StringResource::nameToID (pCmd->getCommandString (false));

            if (StringResource::LANGUAGE_UNKNOWN != iLangIn)
            {
               Enumeration *pEnum = StringResource::getLanguages ();

               while (pEnum->hasMoreElements ())
               {
                                     intptr_t iLangSupported = (intptr_t)pEnum->nextElement ();

                  if (iLangIn == iLangSupported)
                  {
                     if (iLangIn != iCurrentLanguage)
                     {
                        delete pResource; pResource = 0;

                        pResource        = StringResource::create (iLangIn, 0);
                        iCurrentLanguage = iLangIn;
                     }
                     eCommand = PDCCMD_ACK;
                  }
               }

               delete pEnum;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_SET_DEVICE_NAME:
      {
         if (  HookPoints.pfnInitializeInstance
            && pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            int cbDeviceName = strlen (pCmd->getCommandString (false)) + 1;

            pszDeviceName = (char *)malloc (cbDeviceName);

            if (pszDeviceName)
            {
               strcpy (pszDeviceName, pCmd->getCommandString (false));

               hmodDevice = HookPoints.pfnInitializeInstance (pszDeviceName, &hInstance);

               if (  hmodDevice
                  && hInstance
                  )
               {
                  eCommand = PDCCMD_ACK;
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_IS_VALID_DEVICE_NAME:
      {
         if (  HookPoints.pfnInitializeInstance
            && pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            GModule *hmodTemp      = 0;
            HANDLE   hInstanceTemp = 0;

            hmodTemp = HookPoints.pfnInitializeInstance (pCmd->getCommandString (false),
                                                         &hInstanceTemp);

            if (  hmodTemp
               && hInstanceTemp
               )
            {
               eCommand = PDCCMD_ACK;
            }

            if (  HookPoints.pfnFreeInstance
               && hmodTemp
               )
            {
               HookPoints.pfnFreeInstance (hmodTemp, hInstanceTemp);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_GET_JOB_PROPERTIES:
      {
         std::string *pstrJobProperties = 0;

         if (HookPoints.pfnGetJobProperties)
         {
            pstrJobProperties = HookPoints.pfnGetJobProperties (hInstance);

            if (pstrJobProperties)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand, pstrJobProperties)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            delete pstrJobProperties;
            goto BUGOUT;
         }
         delete pstrJobProperties;
         break;
      }

      case PDCCMD_SET_JOB_PROPERTIES:
      {
         char *pszCommandString = pCmd->getCommandString (false);

         if (  HookPoints.pfnSetJobProperties
            && pszCommandString
            && *pszCommandString
            )
         {
            char *pszDebugLook   = pszCommandString;
            char *pszDebugOutput = 0;

            do
            {
               pszDebugOutput = strstr (pszDebugLook, "debugoutput=");

               if (pszDebugOutput)
               {
                  char *pszSpace = strchr (pszDebugOutput, ' ');
                  char *pszValue = 0;

                  pszValue = pszDebugOutput + 12;

                  if (pszSpace)
                  {
                     pszDebugLook = pszSpace + 1;
                     *pszSpace = '\0';
                  }
                  else
                  {
                     pszDebugLook = 0;
                  }

                  DebugOutput::setDebugOutput (pszValue);

                  if (pszSpace)
                     *pszSpace = ' ';
               }
               else
               {
                  pszDebugLook = 0;
               }

            } while (pszDebugLook);

            if (cbJobProperties < strlen (pszCommandString) + 1)
            {
               cbJobProperties  = strlen (pszCommandString) + 1;
               pszJobProperties = (char *)realloc (pszJobProperties, cbJobProperties);
            }

            if (pszJobProperties)
            {
               strcpy (pszJobProperties, pszCommandString);
               fNewJobPropertiesSent = true;

               if (HookPoints.pfnSetJobProperties (hInstance,
                                                   pszJobProperties,
                                                   &DOH))
               {
                  eCommand = PDCCMD_ACK;
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ENUM_INSTANCE_PROPS:
      {
         PSZ *ppszResponse = 0;
         PSZ  pszCurrent   = 0;
         int  cbResponse   = 0;

         if (HookPoints.pfnEnumerateInstanceProps)
         {
            Enumeration    *pEnumGroups       = 0;
            JobProperties  *pJP               = 0;
            PSZRO           pszJP             = 0;
            bool            fInDeviceSpecific = false;

            if (pCmd->getCommandBool (fInDeviceSpecific))
            {
               pEnumGroups = HookPoints.pfnEnumerateInstanceProps (hInstance,
                                                                   fInDeviceSpecific);
            }

            while (  pEnumGroups
                  && pEnumGroups->hasMoreElements ()
                  )
            {
               Enumeration *pEnum = (Enumeration *)pEnumGroups->nextElement ();

               while (pEnum->hasMoreElements ())
               {
                  pJP = (JobProperties *)pEnum->nextElement ();
                  if (pJP)
                  {
                     pszJP = pJP->getJobProperties ();
                     if (pszJP)
                     {
                        cbResponse += strlen (pszJP) + 1;

                        free ((void *)pszJP);
                     }
                  }

                  delete pJP;
               }

               delete pEnum;

               cbResponse++;
            }

            delete pEnumGroups;

            if (0 < cbResponse)
            {
               cbResponse++;
               ppszResponse = (PSZ *)malloc (cbResponse + 1);

               if (ppszResponse)
               {
                  pszCurrent  = (PSZ)ppszResponse;
                  pEnumGroups = HookPoints.pfnEnumerateInstanceProps (hInstance,
                                                                      fInDeviceSpecific);

                  while (pEnumGroups->hasMoreElements ())
                  {
                     Enumeration *pEnum = (Enumeration *)pEnumGroups->nextElement ();

                     while (pEnum->hasMoreElements ())
                     {
                        pJP = (JobProperties *)pEnum->nextElement ();
                        if (pJP)
                        {
                           pszJP = pJP->getJobProperties ();
                           if (pszJP)
                           {
                              strcpy (pszCurrent, pszJP);
                              pszCurrent += strlen (pszJP) + 1;

                              free ((void *)pszJP);
                           }
                        }

                        delete pJP;
                     }

                     delete pEnum;

                     *pszCurrent++ = '\0';
                  }

                  *pszCurrent++ = '\0';

                  delete pEnumGroups;

                  eCommand = PDCCMD_ACK;
               }
            }
         }

         if (  !pCmd->setCommand (eCommand, (PBYTE)ppszResponse, cbResponse)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            if (ppszResponse)
            {
               free ((void *)ppszResponse);
            }

            goto BUGOUT;
         }

         if (ppszResponse)
         {
            free ((void *)ppszResponse);
         }
         break;
      }

      case PDCCMD_GET_JOB_PROPERTY:
      {
         PSZCRO pszCommandString = pCmd->getCommandString (false);
         PSZRO  pszRet           = 0;

         if (  HookPoints.pfnGetJobProperty
            && hInstance
            && pszCommandString
            && *pszCommandString
            )
         {
            pszRet = HookPoints.pfnGetJobProperty (hInstance,
                                                   pszCommandString);

            if (pszRet)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand, pszRet)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_GET_JOB_PROPERTY_TYPE:
      {
         char        *pszCommandString = pCmd->getCommandString (false);
         std::string *pstrJPT          = 0;

         if (  HookPoints.pfnGetJobPropertyType
            && hInstance
            && pszCommandString
            && *pszCommandString
            )
         {
            pstrJPT = HookPoints.pfnGetJobPropertyType (hInstance,
                                                        pszCommandString);

            if (pstrJPT)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand, pstrJPT)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            delete pstrJPT;
            goto BUGOUT;
         }
         delete pstrJPT;
         break;
      }

      case PDCMD_XLATE_JOB_PROPERTY_KEY_VALUE:
      {
         char        *pszCommandString = pCmd->getCommandString (false);
         std::string *pstrRet          = 0;

         if (  HookPoints.pfnTranslateKeyValue
            && hInstance
            && pszCommandString
            && *pszCommandString
            )
         {
            pstrRet = HookPoints.pfnTranslateKeyValue (hInstance,
                                                       pResource,
                                                       pszCommandString);

            if (pstrRet)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand, pstrRet)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            delete pstrRet;
            goto BUGOUT;
         }
         delete pstrRet;
         break;
      }

      case PDCCMD_SET_OUTPUT_STREAM:
      {
         if (  HookPoints.pfnSetOutputStream
            && pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            int fdHandle = STDOUT_FILENO;

            sscanf (pCmd->getCommandString (false), "%d", &fdHandle);

            if (HookPoints.pfnSetOutputStream (hInstance,
                                               fdHandle))
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_SET_ERROR_STREAM:
      {
         if (  pCmd->getCommandString (false)
            && *pCmd->getCommandString (false)
            )
         {
            int fdHandle = STDERR_FILENO;

            sscanf (pCmd->getCommandString (false), "%d", &fdHandle);

            fpError = fdopen (fdHandle, "wb");

            if (fpError)
            {
               DebugOutput::setErrorStream (fpError);
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_BEGIN_JOB:
      {
         if (  HookPoints.pfnBeginJob
            && hInstance
            )
         {
            if (HookPoints.pfnBeginJob (hInstance))
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_START_PAGE:
      {
         if (  HookPoints.pfnStartPage
            && hInstance
            )
         {
            if (HookPoints.pfnStartPage (hInstance))
               eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_END_PAGE:
      {
         if (  HookPoints.pfnEndPage
            && hInstance
            )
         {
            if (HookPoints.pfnEndPage (hInstance, 0))
               eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_END_JOB:
      {
         if (  HookPoints.pfnEndJob
            && hInstance
            )
         {
            if (HookPoints.pfnEndJob (hInstance))
               eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ABORT_JOB:
      {
         if (  HookPoints.pfnAbortJob
            && hInstance
            )
         {
            if (HookPoints.pfnAbortJob (hInstance))
               eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ATTACH_BUFFER1:
      {
         if (pbBuffer1)
         {
            shmdt (pbBuffer1);
         }

         int iValue = 0;

         if (pCmd->getCommandInt (iValue))
         {
            pbBuffer1 = (byte *)shmat (iValue, 0, 0);
         }
         else
         {
            pbBuffer1 = 0;
         }

                   if (pbBuffer1 != 0)
         {
            eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_ATTACH_BUFFER2:
      {
         if (pbBuffer2)
         {
            shmdt (pbBuffer2);
         }

         int iValue = 0;

         if (pCmd->getCommandInt (iValue))
         {
            pbBuffer2 = (byte *)shmat (iValue, 0, 0);
         }
         else
         {
            pbBuffer2 = 0;
         }

                   if (pbBuffer2 != 0)
         {
            eCommand = PDCCMD_ACK;
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_DETACH_BUFFER1:
      {
         if (pbBuffer1)
         {
            int rc = shmdt (pbBuffer1);

            if (0 == rc)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_DETACH_BUFFER2:
      {
         if (pbBuffer2)
         {
            int rc = shmdt (pbBuffer2);

            if (0 == rc)
            {
               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_RASTERIZE:
      {
         if (  HookPoints.pfnRasterize
            && hInstance
            )
         {
            if (  pCmd->getCommandString (false)
               && *pCmd->getCommandString (false)
               )
            {
               int          iType;
               BITBLT_TYPE  eType;
               RECTL        rectlPageLocation;

               if (5 == sscanf (pCmd->getCommandString (false),
                                "%d %d %d %d %d",
                                &iType,
                                &rectlPageLocation.xLeft,
                                &rectlPageLocation.yBottom,
                                &rectlPageLocation.xRight,
                                &rectlPageLocation.yTop)
                  )
               {
                  eType = (BITBLT_TYPE)iType;

                  if (HookPoints.pfnRasterize (hInstance,
                                               pbBuffer2,
                                               (PBITMAPINFO2)pbBuffer1,
                                               &rectlPageLocation,
                                               eType))
                  {
                     eCommand = PDCCMD_ACK;
                  }
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_UPDF_BOOKLET
      case PDCCMD_PUSH_CURRENT_BOOKLET:
      {
         char          *pszCommandString = pCmd->getCommandString (false);
         DeviceBooklet *pBookletTemp     = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pBookletTemp = new BCDeviceBooklet (0,
                                                   pszJP,
                                                   pszID);

               free ((void *)pszJP);
            }

            if (pBookletTemp)
            {
               DOH.setCurrentBooklet (pBookletTemp);

               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

#ifdef INCLUDE_JP_COMMON_COPIES
      case PDCCMD_PUSH_CURRENT_COPIES:
      {
         char         *pszCommandString = pCmd->getCommandString (false);
         DeviceCopies *pCopiesTemp      = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            const char *pszSpaceConst   = 0;
            PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
            PSZRO   pszJP               = 0;
            int     iMinimum            = 0;
            int     iMaximum            = 0;
            int     iSimulationRequired = 0;
            bool    fSimulationRequired = false;

            pszSpaceConst = strchr (pszJPQuoted, ' ');

            if (pszSpaceConst)
            {
               std::string tempQuoted(pszJPQuoted, pszSpaceConst - pszJPQuoted);

               pszJP = Omni::dequoteString (tempQuoted.c_str());

               if (!pszJP)
               {
               }

               if (  pszJP
                  && 3 == sscanf (pszSpaceConst + 1,
                                  "%d %d %d",
                                  &iMinimum,
                                  &iMaximum,
                                  &iSimulationRequired)
                  )
                              {
                   PSZCRO pszID = findSpace (pszSpaceConst + 1, 3);
 
                   if (iSimulationRequired)
                   {
                     fSimulationRequired = true;
                  }

                  pCopiesTemp = new BCDeviceCopies (0,
                                                    pszJP,
                                                    pszID,
                                                    0,
                                                    iMinimum,
                                                    iMaximum,
                                                    fSimulationRequired);
               }

               if (pCopiesTemp)
               {
                  DOH.setCurrentCopies (pCopiesTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_DITHER_ID:
      {
         if (  !pCmd->setCommand (PDCCMD_ACK)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_PUSH_CURRENT_FORM:
      {
         char       *pszCommandString = pCmd->getCommandString (false);
         DeviceForm *pFormTemp        = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            const char *pszSpaceConst = 0;
            PSZCRO  pszJPQuoted   = pCmd->getCommandString (false);
            PSZRO   pszJP         = 0;
            int     iCapabilities = 0;
            int     iLeftClip     = 0;
            int     iTopClip      = 0;
            int     iRightClip    = 0;
            int     iBottomClip   = 0;

            pszSpaceConst = strchr (pszJPQuoted, ' ');

            if (pszSpaceConst)
            {
               std::string tempQuoted(pszJPQuoted, pszSpaceConst - pszJPQuoted);

               pszJP = Omni::dequoteString (tempQuoted.c_str());

               if (!pszJP)
               {
               }

               if (  pszJP
                  && 5 == sscanf (pszSpaceConst + 1,
                                  "%d %d %d %d %d",
                                  &iCapabilities,
                                  &iLeftClip,
                                  &iTopClip,
                                  &iRightClip,
                                  &iBottomClip)
                  )
               {
                                     PSZCRO pszID = findSpace (pszSpaceConst + 1, 5);

                  pFormTemp = new BCDeviceForm (0,
                                                pszJP,
                                                pszID,
                                                iCapabilities,
                                                0,
                                                new HardCopyCap (iLeftClip,
                                                                 iTopClip,
                                                                 iRightClip,
                                                                 iBottomClip));
               }

               if (pFormTemp)
               {
                  if (DOH.getCurrentResolution ())
                     pFormTemp->associateWith (DOH.getCurrentResolution ());

                  DOH.setCurrentForm (pFormTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_UPDF_JOGGING
      case PDCCMD_PUSH_CURRENT_JOGGING:
      {
         char          *pszCommandString = pCmd->getCommandString (false);
         DeviceJogging *pJoggingTemp     = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pJoggingTemp = new BCDeviceJogging (0,
                                                   pszJP,
                                                   pszID);

               free ((void *)pszJP);
            }

            if (pJoggingTemp)
            {
               DOH.setCurrentJogging (pJoggingTemp);

               eCommand = PDCCMD_ACK;
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_MEDIA:
      {
         char        *pszCommandString = pCmd->getCommandString (false);
         DeviceMedia *pMediaTemp       = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
                        const char *pszSpaceConst    = 0;
            PSZCRO  pszJPQuoted          = pCmd->getCommandString (false);
            PSZRO   pszJP                = 0;
            int     iColorAdjustRequired = 0;
            int     iAbsorption          = 0;

            pszSpaceConst = strchr (pszJPQuoted, ' ');

            if (pszSpaceConst)
            {
               std::string tempQuoted(pszJPQuoted, pszSpaceConst - pszJPQuoted);

               pszJP = Omni::dequoteString (tempQuoted.c_str());

               if (!pszJP)
               {
               }

               if (  pszJP
                  && 2 == sscanf (pszSpaceConst + 1,
                                   "%d %d",
                                   &iColorAdjustRequired,
                                   &iAbsorption)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 2);

                  pMediaTemp = new BCDeviceMedia (0,
                                                  pszJP,
                                                  pszID,
                                                  0,
                                                  iColorAdjustRequired,
                                                  iAbsorption);
               }

               if (pMediaTemp)
               {
                  DOH.setCurrentMedia (pMediaTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_COMMON_NUP
      case PDCCMD_PUSH_CURRENT_NUP:
      {
         char      *pszCommandString = pCmd->getCommandString (false);
         DeviceNUp *pNUpTemp         = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            const char *pszSpaceConst    = 0;
            PSZCRO  pszJPQuoted          = pCmd->getCommandString (false);
            PSZRO   pszJP                = 0;
            int     iSimulationRequired = 0;
            bool    fSimulationRequired = false;

            pszSpaceConst = strchr (pszJPQuoted, ' ');

            if (pszSpaceConst)
            {
               std::string tempQuoted(pszJPQuoted, pszSpaceConst - pszJPQuoted);

               pszJP = Omni::dequoteString (tempQuoted.c_str());

               if (!pszJP)
               {
               }

               if (  pszJP
                  && 1 == sscanf (pszSpaceConst + 1,
                                  "%d",
                                  &iSimulationRequired)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpaceConst + 1, 1);

                  if (iSimulationRequired)
                  {
                     fSimulationRequired = true;
                  }

                  pNUpTemp = new BCDeviceNUp (0,
                                              pszJP,
                                              pszID,
                                              0,
                                              fSimulationRequired);
               }

               if (pNUpTemp)
               {
                  DOH.setCurrentNUp (pNUpTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_ORIENTATION:
      {
         char              *pszCommandString = pCmd->getCommandString (false);
         DeviceOrientation *pOrientationTemp = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace            = 0;
            PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
            PSZRO   pszJP               = 0;
            int     iSimulationRequired = 0;
            bool    fSimulationRequired = false;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                  && 1 == sscanf (pszSpace + 1,
                                  "%d",
                                  &iSimulationRequired)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 1);

                  if (iSimulationRequired)
                  {
                     fSimulationRequired = true;
                  }

                  pOrientationTemp = new BCDeviceOrientation (0,
                                                              pszJP,
                                                              pszID,
                                                              fSimulationRequired);
               }

               if (pOrientationTemp)
               {
                  DOH.setCurrentOrientation (pOrientationTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_COMMON_OUTPUT_BIN
      case PDCCMD_PUSH_CURRENT_OUTPUT_BIN:
      {
         char            *pszCommandString = pCmd->getCommandString (false);
         DeviceOutputBin *pOutputBinTemp = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pOutputBinTemp = new BCDeviceOutputBin (0,
                                                       pszJP,
                                                       pszID,
                                                       0);

               if (pOutputBinTemp)
               {
                  DOH.setCurrentOutputBin (pOutputBinTemp);

                  eCommand = PDCCMD_ACK;
               }

               free ((void *)pszJP);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_PRINT_MODE:
      {
         char            *pszCommandString = pCmd->getCommandString (false);
         DevicePrintMode *pPrintModeTemp   = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace       = 0;
            PSZCRO  pszJPQuoted    = pCmd->getCommandString (false);
            PSZRO   pszJP          = 0;
            int     iPhysicalCount = 0;
            int     iLogicalCount  = 0;
            int     iPlanes        = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                                     && 3 == sscanf (pszSpaceConst + 1,
                                   "%d %d %d",
                                   &iPhysicalCount,
                                   &iLogicalCount,
                                   &iPlanes)
                   )
                {
                   PSZCRO pszID = findSpace (pszSpaceConst + 1, 3);

                  pPrintModeTemp = new BCDevicePrintMode (0,
                                                          pszJP,
                                                          pszID,
                                                          iPhysicalCount,
                                                          iLogicalCount,
                                                          iPlanes);
               }

               if (pPrintModeTemp)
               {
                  DOH.setCurrentPrintMode (pPrintModeTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      case PDCCMD_PUSH_CURRENT_RESOLUTION:
      {
         char             *pszCommandString = pCmd->getCommandString (false);
         DeviceResolution *pResolutionTemp  = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace               = 0;
            PSZCRO  pszJPQuoted            = pCmd->getCommandString (false);
            PSZRO   pszJP                  = 0;
            int     iXRes                  = 0;
            int     iYRes                  = 0;
            int     iXInternalRes          = 0;
            int     iYInternalRes          = 0;
            int     iCapabilities          = 0;
            int     iDestinationBitsPerPel = 0;
            int     iScanlineMultiple      = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                  && 7 == sscanf (pszSpace + 1,
                                  "%d %d %d %d %d %d %d",
                                  &iXRes,
                                  &iYRes,
                                  &iXInternalRes,
                                  &iYInternalRes,
                                  &iCapabilities,
                                  &iDestinationBitsPerPel,
                                  &iScanlineMultiple)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 7);

                  pResolutionTemp = new BCDeviceResolution (0,
                                                            pszJP,
                                                            pszID,
                                                            iXInternalRes,
                                                            iYInternalRes,
                                                            0,
                                                            iCapabilities,
                                                            iDestinationBitsPerPel,
                                                            iScanlineMultiple);
               }

               if (pResolutionTemp)
               {
                  DOH.setCurrentResolution (pResolutionTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_COMMON_SCALING
      case PDCCMD_PUSH_CURRENT_SCALING:
      {
         char          *pszCommandString = pCmd->getCommandString (false);
         DeviceScaling *pScalingTemp  = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace      = 0;
            PSZCRO  pszJPQuoted   = pCmd->getCommandString (false);
            PSZRO   pszJP         = 0;
            int     iMinimumScale = 0;
            int     iMaximumScale = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                  && 2 == sscanf (pszSpace + 1,
                                  "%d %d",
                                  &iMinimumScale,
                                  &iMaximumScale)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 2);

                  pScalingTemp = new BCDeviceScaling (0,
                                                      pszJP,
                                                      pszID,
                                                      0,
                                                      iMinimumScale,
                                                      iMaximumScale);
               }

               if (pScalingTemp)
               {
                  DOH.setCurrentScaling (pScalingTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

#ifdef INCLUDE_JP_COMMON_SHEET_COLLATE
      case PDCCMD_PUSH_CURRENT_SHEET_COLLATE:
      {
         char               *pszCommandString  = pCmd->getCommandString (false);
         DeviceSheetCollate *pSheetCollateTemp = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pSheetCollateTemp = new BCDeviceSheetCollate (0,
                                                             pszJP,
                                                             pszID,
                                                             0);

               if (pSheetCollateTemp)
               {
                  DOH.setCurrentSheetCollate (pSheetCollateTemp);

                  eCommand = PDCCMD_ACK;
               }

               free ((void *)pszJP);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

#ifdef INCLUDE_JP_COMMON_SIDE
      case PDCCMD_PUSH_CURRENT_SIDE:
      {
         char       *pszCommandString = pCmd->getCommandString (false);
         DeviceSide *pSideTemp  = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace            = 0;
            PSZCRO  pszJPQuoted         = pCmd->getCommandString (false);
            PSZRO   pszJP               = 0;
            int     iSimulationRequired = 0;
            bool    fSimulationRequired = false;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                  && 1 == sscanf (pszSpace + 1,
                                  "%d",
                                  &iSimulationRequired)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 1);

                  if (iSimulationRequired)
                  {
                     fSimulationRequired = true;
                  }

                  pSideTemp = new BCDeviceSide (0,
                                                pszJP,
                                                pszID,
                                                0,
                                                fSimulationRequired);
               }

               if (pSideTemp)
               {
                  DOH.setCurrentSide (pSideTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

#ifdef INCLUDE_JP_COMMON_STITCHING
      case PDCCMD_PUSH_CURRENT_STITCHING:
      {
         char            *pszCommandString = pCmd->getCommandString (false);
         DeviceStitching *pStitchingTemp   = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pStitchingTemp = new BCDeviceStitching (0,
                                                       pszJP,
                                                       pszID,
                                                       0);

               if (pStitchingTemp)
               {
                  DOH.setCurrentStitching (pStitchingTemp);

                  eCommand = PDCCMD_ACK;
               }

               free ((void *)pszJP);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_TRAY:
      {
         char       *pszCommandString = pCmd->getCommandString (false);
         DeviceTray *pTrayTemp        = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;
            int     iType       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';

               pszJP = Omni::dequoteString (pszJPQuoted);

               if (!pszJP)
               {
                  *pszSpace = ' ';
               }

               if (  pszJP
                  && 1 == sscanf (pszSpace + 1,
                                  "%d",
                                  &iType)
                  )
               {
                  PSZCRO pszID = findSpace (pszSpace + 1, 1);

                  pTrayTemp = new BCDeviceTray (0,
                                                pszJP,
                                                pszID,
                                                iType,
                                                0);
               }

               if (pTrayTemp)
               {
                  DOH.setCurrentTray (pTrayTemp);

                  eCommand = PDCCMD_ACK;
               }

               if (pszJP)
               {
                  free ((void *)pszJP);
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

#ifdef INCLUDE_JP_COMMON_TRIMMING
      case PDCCMD_PUSH_CURRENT_TRIMMING:
      {
         char           *pszCommandString = pCmd->getCommandString (false);
         DeviceTrimming *pTrimmingTemp    = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char   *pszSpace    = 0;
            PSZCRO  pszJPQuoted = pCmd->getCommandString (false);
            PSZRO   pszJP       = 0;

            pszSpace = strchr (pszJPQuoted, ' ');

            if (pszSpace)
            {
               *pszSpace = '\0';
            }

            pszJP = Omni::dequoteString (pszJPQuoted);

            if (pszJP)
            {
               PSZRO pszID = 0;

               if (pszSpace)
               {
                  pszID = pszSpace + 1;
               }

               pTrimmingTemp = new BCDeviceTrimming (0,
                                                     pszJP,
                                                     pszID,
                                                     0);

               if (pTrimmingTemp)
               {
                  DOH.setCurrentTrimming (pTrimmingTemp);

                  eCommand = PDCCMD_ACK;
               }

               free ((void *)pszJP);
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
#endif

      case PDCCMD_PUSH_CURRENT_GAMMA:
      {
         char        *pszCommandString = pCmd->getCommandString (false);
         DeviceGamma *pGammaTemp       = 0;

         if (  pszCommandString
            && *pszCommandString
            )
         {
            char *pszID       = 0;
            int   iCGamma     = 0;
            int   iMGamma     = 0;
            int   iYGamma     = 0;
            int   iKGamma     = 0;
            int   iCBias      = 0;
            int   iMBias      = 0;
            int   iYBias      = 0;
            int   iKBias      = 0;

            pszID = pszCommandString;

            pszCommandString = strchr (pszID, ' ');

            if (pszCommandString)
            {
               *pszCommandString++ = '\0';

               if (8 == sscanf (pszCommandString,
                                "%d %d %d %d %d %d %d %d",
                                &iCGamma,
                                &iMGamma,
                                &iYGamma,
                                &iKGamma,
                                &iCBias,
                                &iMBias,
                                &iYBias,
                                &iKBias)
                  )
               {
                  pGammaTemp = new BCDeviceGamma (iCGamma,
                                                  iMGamma,
                                                  iYGamma,
                                                  iKGamma,
                                                  iCBias,
                                                  iMBias,
                                                  iYBias,
                                                  iKBias);
               }

               if (pGammaTemp)
               {
                  DOH.setCurrentGamma (pGammaTemp);

                  eCommand = PDCCMD_ACK;
               }
            }
         }

         if (  !pCmd->setCommand (eCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }

      default:
      {
         if (  !pCmd->setCommand (PDCCMD_UNSUPPORTED, pszErrorUnknownCommand)
            || !pCmd->sendCommand (fdS2C)
            )
         {
            goto BUGOUT;
         }
         break;
      }
      }
   }

BUGOUT:

   // Clean up
   if (-1 != fdS2C)
      close (fdS2C);
   if (-1 != fdC2S)
      close (fdC2S);

   if (pszJobProperties)
   {
      free (pszJobProperties);
   }

   if (pbBuffer1)
   {
      shmdt (pbBuffer1);
   }
   if (pbBuffer2)
   {
      shmdt (pbBuffer2);
   }

   delete pCmd;
   delete pResource; pResource = 0;

   if (pszDeviceName)
   {
      free (pszDeviceName);
   }

   if (  HookPoints.pfnFreeInstance
      && hmodDevice
      )
   {
      HookPoints.pfnFreeInstance (hmodDevice, hInstance);
   }

   if (fpError)
   {
      fclose (fpError);
   }

   Omni::terminate ();

   return 0;
}
