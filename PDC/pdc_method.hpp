/*
** Copyright (c) 2003 International Business Machines
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
*/
#ifndef _PDC_METHOD_H
#define _PDC_METHOD_H 1

#include <string>
#include <map>

typedef std::map <std::string, std::string> DriverMap;
typedef std::map <std::string, DriverMap *> DriversMap;

class PDCMethod
{
public:
   virtual                 ~PDCMethod                     () {}

   virtual STRINGPAIRARRAY  pdcEnumerateDevices           () = 0;
   virtual const char      *pdcGetDeviceHandle            (const char *pszDisplayHandle) = 0;

   virtual const void      *pdcOpenDevice                 (const char *pszDeviceHandle) = 0;
   virtual int              pdcCloseDevice                (const void *pvDeviceHandle) = 0;

   virtual int              pdcSetTranslatableLanguage    (const void *pvDeviceHandle,
                                                           const char *pszLanguage) = 0;
   virtual const char      *pdcGetTranslatableLanguage    (const void *pvDeviceHandle) = 0;
   virtual STRINGARRAY      pdcQueryTranslatableLanguages (const void *pvDeviceHandle) = 0;

   virtual const char      *pdcGetPDLInformation          (const void *pvDeviceHandle) = 0;

   virtual const char      *pdcQueryCurrentJobProperties  (const void *pvDeviceHandle) = 0;
   virtual int              pdcSetJobProperties           (const void *pvDeviceHandle,
                                                           const char *pszJobProperties) = 0;
   virtual STRINGARRAY      pdcListJobPropertyKeys        (const void *pvDeviceHandle) = 0;
   virtual STRINGARRAY      pdcListJobPropertyKeyValues   (const void *pvDeviceHandle,
                                                           const char *pszKey) = 0;
   virtual const char      *pdcGetJobProperty             (const void *pvDeviceHandle,
                                                           const char *pszKey) = 0;
   virtual const char      *pdcGetJobPropertyType         (const void *pvDeviceHandle,
                                                           const char *pszKey) = 0;
   virtual STRINGARRAY      pdcTranslateJobProperty       (const void *pvDeviceHandle,
                                                           const char *pszKey,
                                                           const char *pszValue) = 0;

   virtual int              pdcBeginJob                   (const void *pvDeviceHandle) = 0;
   virtual int              pdcStartPage                  (const void *pvDeviceHandle) = 0;
   virtual int              pdcStartPageWithProperties    (const void *pvDeviceHandle,
                                                           const char *pszJobProperties) = 0;
   virtual int              pdcEndPage                    (const void *pvDeviceHandle) = 0;
   virtual int              pdcEndJob                     (const void *pvDeviceHandle) = 0;
   virtual int              pdcAbortPage                  (const void *pvDeviceHandle) = 0;
   virtual int              pdcAbortJob                   (const void *pvDeviceHandle) = 0;
};

#endif /* _PDC_METHOD_H */
